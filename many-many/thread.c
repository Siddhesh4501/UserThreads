#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <linux/futex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include"thread.h"
#include"roundRobin.h"
#include "encrypt.h"
#include "locks.h"


singlyLL sll;
thread_id currTid;
kernelThread currThreads[NOOFKERNELTHREADS];
int currThreadsAlarmStatus[NOOFKERNELTHREADS];
// thread* currThread;
thread* schedulerThread;
// thread* mainThread;
lock_t sched_lock;
lock_t create_thread_lock;

int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid, void** retval);
int mythread_kill(thread_id tid,int sig);
void mythread_exit(void* ret);
void switchToScheduler();

void* allocateStackSpace(memsize stacksize, memsize guardsize){
    void* mem;
    mem = malloc(stacksize + guardsize);
    if(mem == NULL){
        perror("malloc failed in stack\n");
        return NULL;
    }
    return mem;

}

thread_id getNextTid(){
    return ++currTid;
}
thread_id getCurrTid(){
    return currTid;
}


void ModifyThreadSignalsMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGINT);
    sigdelset(&set,SIGSTOP);
    sigdelset(&set,SIGCONT);
    sigdelset(&set,SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}


void setTimer(int duration){
    // printf("in set timer\n");
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = duration;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = duration;
    if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1)
       printf("error in settimer\n");
      
}

void setHandler(int sig){
    if(signal(sig, switchToScheduler) == SIG_ERR)
            exit(1);
}

void swapContext(sigjmp_buf* old, sigjmp_buf* new){
    // printf("in swap context\n");
    int ret = sigsetjmp(*old, 1);
    if(ret == 0)
        siglongjmp(*new, 1);
    // printf("in swap context1\n");
    // printf("after swap context long jump\n");
}

void setScheduling(int sig){
    // printf("in setscheduler\n");
    if(gettid() == getpid()) return;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThreadsAlarmStatus[i] = 1;
            break;
        }
    }
}

void resetScheduling(int sig){
    // printf("in reset scheduler\n");
    if(gettid() == getpid()) return;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThreadsAlarmStatus[i] = 0;
            break;
        }
    }
    
}

void deliverAsynchronousSignal(){
    resetScheduling(SIGVTALRM);
    sigset_t set;
    sigemptyset(&set);
    
    thread* currThread = NULL;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThread = currThreads[i].userThread;
            break;
        }
    }
    int n = currThread->noOfPendingSignals;
    for(int i = 0; i < n; i++){
        sigaddset(&set, currThread->pendingSigArr[i]);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        kill(getpid(), currThread->pendingSigArr[i]);
        sigdelset(&set, currThread->pendingSigArr[i]);
    }
    ModifyThreadSignalsMask();
    setScheduling(SIGVTALRM);
}

void switchToScheduler(){
    if(gettid() == getpid()) return;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            if(currThreadsAlarmStatus[i] == 0) return;
            else break;
        }
    }
    printf("in switch to sched\n");

    printf("on timer interrupt %d\n",gettid());
    thread* currThread = NULL;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThread = currThreads[i].userThread;
            break;
        }
    }
    if(currThread == NULL){
        printf("error %d\n",gettid());
        exit(0);
    }
    resetScheduling(SIGVTALRM);
    // printf("in switch to scheduler\n");
    // printf("switch to shed %p %p\n",currThread,schedulerThread);
    deliverAsynchronousSignal();
    swapContext(currThread->context, schedulerThread->context);
    setScheduling(SIGVTALRM);

}




void scheduler(){
    if(gettid() == getpid()) return;
    lock_lock(&sched_lock);
    printf("in scheduler %d\n",gettid());
    resetScheduling(SIGVTALRM);  
    // printSLL(sll);
    thread* currThread = NULL;
    int i;
    for(i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThread = currThreads[i].userThread;
            break;
        }
    }
    printf("after in scheduler\n");

    if(currThread){
        if(currThread->state == EXITED){
            // printf("exited called\n");
            // removeThread(&sll, currThread);
        }
        if(currThread->state == RUNNING){
            // printf("runningg called\n");

            currThread->state = RUNNABLE;
        }
    }
    // printSLL(sll);
    thread* nextThread = NULL;
    while(nextThread == NULL){
        printf("after in scheduler1\n");
        nextThread = getRunnableThread(&sll);
    }

    currThread = nextThread;
    currThread->state = RUNNING;
    currThreads[i].userThread = currThread;
    // printf("in sched %p\n",currThread);
    setScheduling(SIGVTALRM);
    lock_unlock(&sched_lock);
    siglongjmp(*(currThread->context),1);
}




void messageWaiters(thread_id* waitersTid, int n){
    for(int i = 0; i < n; i++){
        thread_id tid = waitersTid[i];
        thread* th = getThread(&sll, tid);
        th->noOfJoins--;
        if(th->noOfJoins == 0){
            th->state = RUNNABLE;
        }
    }
}


int wrapper(){
    thread* currThread = NULL;
    for(int i = 0; i < NOOFKERNELTHREADS; i++){
        if(currThreads[i].tid == gettid()){
            currThread = currThreads[i].userThread;
            break;
        }
    }
    void*(*funptr)(void*) = currThread->funptr;
    void* functionarg = currThread->arg;
    // printf("%p %p %p\n",funptr,functionarg,currThread);
    setScheduling(SIGVTALRM);
    // printf("in wrapper\n");
    currThread->retval = (funptr)(functionarg); 
    // void (*exitfun)() = currThread->thread_attr->exitfun;
    // if(exitfun)
    //    exitfun();

    resetScheduling(SIGVTALRM);
    printf("after executing function\n");
    // mythread_exit(NULL);
    switchToScheduler();
    
    return 0;
}

int kernelThreadsWrapper(void* arg){
    resetScheduling(SIGVTALRM);
    static int currNoOfThreads = 0;
    kernelThread* kernelThread = arg;
    currThreads[currNoOfThreads].userThread = NULL;
    currThreads[currNoOfThreads].tid = gettid();
    currThreads[currNoOfThreads].threadNo = currNoOfThreads;
    currNoOfThreads++;
    // printf("in kernel thread wrapper\n");
    // wrapper();
    setTimer(1000);
    scheduler();
    // while(1);
    return 0;
}


void initialiseContext(sigjmp_buf* context, void* stack, void* fun){
    // printf("in initialsie1111\n");
    sigsetjmp(*context, 1);
    if(stack){
        (*context)->__jmpbuf[5] = encrypt((long int)(stack - sizeof(int)));
        (*context)->__jmpbuf[6] = (*context)->__jmpbuf[5];
    }
    if(fun)
        (*context)->__jmpbuf[7] = encrypt((long int)fun);
}

int initialiseThread(thread* th, pid_t tid, void* fun, void* arg, void* thread_attr, thread_state state, int threadType){
    
    if(threadType != 1){
        th->thread_attr = (attr*)malloc(sizeof(attr));
        if(th->thread_attr == NULL){
            perror("Error in creating thread attribute\n");
            return 1;
        }
        if(thread_attr == NULL){
            attr* thattr = th->thread_attr;
            thattr->exitfun = NULL;
            thattr->flag = 0;
            thattr->stacksize = STD_STACK_SIZE;
            thattr->guardsize = STD_GUARD_SIZE;
        }
        else{
            memcpy(th->thread_attr, thread_attr, sizeof(attr));
        }
        th->thread_attr->stackpointer = allocateStackSpace(th->thread_attr->stacksize, th->thread_attr->guardsize);
        if(th->thread_attr->stackpointer == NULL){
            perror("Error in allocating memory for stack\n");
            return 1;
        }
    }
    sigjmp_buf* newthread_context = (sigjmp_buf*)malloc(sizeof(sigjmp_buf));
    // printf("in intit\n");
    if(threadType == 1)
        initialiseContext(newthread_context, NULL, NULL);
    else if (threadType == 2)
        initialiseContext(newthread_context, th->thread_attr->stackpointer + th->thread_attr->stacksize + th->thread_attr->guardsize, scheduler);
    else
        initialiseContext(newthread_context, th->thread_attr->stackpointer + th->thread_attr->stacksize + th->thread_attr->guardsize, wrapper);

    th->tid = tid;
    th->funptr = fun;
    th->arg = arg;
    th->state = state;
    th->context = newthread_context;
    th->noOfPendingSignals = 0;
    th->pendingSigArr = NULL;
    th->noOfWaiters = 0;
    th->waitersTid = NULL;
    th->noOfJoins = 0;
    th->next = NULL;
    return 0;
}




void initManyToMany(){
    ModifyThreadSignalsMask();
    sll.back = NULL;
    sll.front = NULL;

    currTid = getpid();
    lock_init(&sched_lock);
    lock_init(&create_thread_lock);
    // mainThread = (thread*)malloc(sizeof(thread));
    // initialiseThread(mainThread, getCurrTid(), NULL, NULL, NULL, RUNNING, 1);
    // addToSLL(&sll,mainThread);
  
    schedulerThread = (thread*)malloc(sizeof(thread));
    initialiseThread(schedulerThread, getNextTid(), scheduler, NULL, NULL, RUNNABLE, 2);

    for(int i=0; i < NOOFKERNELTHREADS; i++){
        kernelThread* kernelThread = malloc(sizeof(kernelThread));
        // initialiseThread(kernelThread, getNextTid(), wrapper, NULL, NULL, RUNNING, 2);
         
        void* kernelStackSpace = allocateStackSpace(STD_STACK_SIZE, STD_GUARD_SIZE);
        thread_id cloneid = clone(kernelThreadsWrapper, kernelStackSpace + STD_STACK_SIZE + STD_GUARD_SIZE, CLONE_FLAGS, kernelThread, NULL, NULL, NULL);
        currThreadsAlarmStatus[i] = 0;
        // currThreads[i].id = cloneid;
        // currThreads[i].thread = kernelThread;
        // currThreads[i].threadNo = i;
    }
    // lock_lock(&lock_var);
    // insertInLL(&head,newthread);
    // lock_unlock(&lock_var);

    




    printf("exiting intimany-many");
    // printf("main thread add %p, scheduler add %p\n",mainThread,schedulerThread);
    setHandler(SIGVTALRM);
    setScheduling(SIGVTALRM);
    // setTimer(1000);
}




int mythread_create(thread_id* tid, void* thread_attr,void*(*funptr)(void*), void* arg){
    resetScheduling(SIGVTALRM);
    static int is_first_thread = 1; 
    lock_lock(&create_thread_lock);
    if(is_first_thread){
        // printf("in critical sections\n");
        is_first_thread = 0;
        initManyToMany();
    }
    lock_unlock(&create_thread_lock);
    thread* newthread = (thread*)malloc(sizeof(thread));
    if(newthread == NULL)
        return 1;
    if(initialiseThread(newthread, getNextTid(), funptr, arg, thread_attr, RUNNABLE, 0) !=0 )
       return 1;
    // printf("in thread create %p\n",newthread);
    addToSLL(&sll, newthread);
    // printSLL(sll);
    *tid = newthread->tid;
    printf("step1\n");
    // sigsetjmp(*(mainThread->context), 1);
    // setScheduling(SIGVTALRM);
    return 0;
}



// int mythread_join(thread_id tid, void** retval){
//     resetScheduling(SIGVTALRM);
//     if(tid < 0){
//         setScheduling(SIGVTALRM);
//        return 1;
//     } 
//     thread* th = getThread(&sll, tid);
//         // printf("in join\n");     
//     if(th == NULL){
//         setScheduling(SIGVTALRM);
//         return 1;
//     }
//     if(th->state == EXITED){
//         if(retval){
//             (*retval) = th->retval;
//             setScheduling(SIGVTALRM);
//             return 0;
//         }
//     }
//     thread* currThread = NULL;
//     for(int i = 0; i < NOOFKERNELTHREADS; i++){
//         if(currThreads[i].tid == gettid()){
//             currThread = currThreads[i].userThread;
//             break;
//         }
//     }
//     currThread->state = WAITING;
//     currThread->noOfJoins++;
//     th->noOfWaiters++;
//     th->waitersTid = realloc(th->waitersTid, sizeof(thread_id)*(th->noOfWaiters));
//     th->waitersTid[th->noOfWaiters-1] = currThread->tid;
//     switchToScheduler();
//     return 0;
// }


// void mythread_exit(void* ret){
//     resetScheduling(SIGVTALRM);
//     thread* currThread = NULL;
//     for(int i = 0; i < NOOFKERNELTHREADS; i++){
//         if(currThreads[i].tid == gettid()){
//             currThread = currThreads[i].userThread;
//             break;
//         }
//     }
//     currThread->state = EXITED;
//     messageWaiters(currThread->waitersTid, currThread->noOfWaiters);
//     removeThread(&sll, currThread);
// }


// int mythread_kill(thread_id tid,int sig){
//     resetScheduling(SIGVTALRM);
//     if(sig < 1 || sig > 31)
//         return 1;
//     if(tid <= 0)
//         return 1;

//     thread* currThread = NULL;
//     for(int i = 0; i < NOOFKERNELTHREADS; i++){
//         if(currThreads[i].tid == gettid()){
//             currThread = currThreads[i].userThread;
//             break;
//         }
//     }
//     thread_id currid = getpid();
//     if(sig == SIGINT || sig == SIGSTOP || sig == SIGCONT){
//         kill(currid, sig);
//         setScheduling(SIGVTALRM);
//         return 0;
//     }
//     if(tid == currThread->tid){
//         // raise(sig);
//         kill(getpid(), sig);
//         setScheduling(SIGVTALRM);
//         return 0;
//     }
//     thread* th = getThread(&sll, tid);
//     if(th == NULL){
//         setScheduling(SIGVTALRM);
//         return 1;
//     }
//     th->noOfPendingSignals++;
//     th->pendingSigArr = realloc(th->pendingSigArr, sizeof(int)*(th->noOfPendingSignals));
//     th->pendingSigArr[th->noOfPendingSignals-1] = sig;
//     setScheduling(SIGVTALRM);
//     return 0;
// }
