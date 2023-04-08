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


sigset_t set;
singlyLL sll;
thread_id currTid;
thread* currThread;
thread* schedulerThread;
thread* mainThread;

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


void swapContext(sigjmp_buf* old, sigjmp_buf* new){
    printf("in swap context\n");
    int ret = sigsetjmp(*old, 1);
    if(ret == 0)
        siglongjmp(*new, 1);
    printf("after swap context long jump\n");
}

void setScheduling(int sig){
    // printf("in setscheduler\n");
    if(signal(SIGVTALRM, switchToScheduler) == SIG_ERR)
        exit(1);
}

void resetScheduling(int sig){
    // printf("in reset scheduler\n");
    if(signal(sig, SIG_IGN) == SIG_ERR)
        exit(1);
}
void switchToScheduler(){
    resetScheduling(SIGVTALRM);
    // printf("in switch to scheduler\n");
    // printf("switch to shed %p %p\n",currThread,schedulerThread);
    swapContext(currThread->context, schedulerThread->context);
    setScheduling(SIGVTALRM);
}




void scheduler(){
    // printf("in scheduler!!!!!!!!!!!!!!!!!!!\n");
    resetScheduling(SIGVTALRM);
    
    // printf("in scheduler\n");
    if(currThread->state == EXITED)
        removeThread(&sll, currThread);
    if(currThread->state == RUNNING)
        currThread->state = RUNNABLE;
    // printSLL(sll);
    thread* nextThread = getRunnableThread(&sll);

    if(nextThread == NULL)
       return;

    currThread = nextThread;
    currThread->state = RUNNING;
    setScheduling(SIGVTALRM);
    printf("in sched %p\n",currThread);
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
    void*(*funptr)(void*) = currThread->funptr;
    void* functionarg = currThread->arg;
    // printf("%p %p %p\n",funptr,functionarg,currThread);
    setScheduling(SIGVTALRM);
    // printf("in wrapper\n");
    currThread->retval = (funptr)(functionarg); 
    void (*exitfun)() = currThread->thread_attr->exitfun;
    if(exitfun)
       exitfun();
    // mythread_exit(NULL);

    resetScheduling(SIGVTALRM);

    currThread->state = EXITED;
    messageWaiters(currThread->waitersTid, currThread->noOfWaiters);
    // removeThread(&sll, currThread);
    switchToScheduler();
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




void initManyToOne(){
    ModifyThreadSignalsMask();
    sll.back = NULL;
    sll.front = NULL;

    currTid = getpid();

    mainThread = (thread*)malloc(sizeof(thread));
    initialiseThread(mainThread, getCurrTid(), NULL, NULL, NULL, RUNNING, 1);
    addToSLL(&sll,mainThread);
    // // printf("step2\n");
    currThread = mainThread;
    // printf("currthread %p\n",currThread);


    schedulerThread = (thread*)malloc(sizeof(thread));
    // // printf("sched add %p\n",schedulerThread);
    initialiseThread(schedulerThread, getNextTid(), scheduler, NULL, NULL,RUNNABLE, 2);
    
    printf("main thread add %p, scheduler add %p\n",mainThread,schedulerThread);
    setScheduling(SIGVTALRM);
    setTimer(2000);
}




int mythread_create(thread_id* tid, void* thread_attr,void*(*funptr)(void*), void* arg){
    resetScheduling(SIGVTALRM);
    static int is_first_thread = 1; 
    
    if(is_first_thread){
        is_first_thread = 0;
        initManyToOne();
    }
    thread* newthread = (thread*)malloc(sizeof(thread));
    if(newthread == NULL)
        return 1;
    if(initialiseThread(newthread, getNextTid(), funptr, arg, thread_attr, RUNNABLE, 0) !=0 )
       return 1;
    // printf("step1\n");
    printf("in thread create %p\n",newthread);
    addToSLL(&sll, newthread);
    // printSLL(sll);
    *tid = getCurrTid();
    setScheduling(SIGVTALRM);
    return 0;
}