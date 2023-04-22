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
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set,SIGINT);
    sigdelset(&set,SIGSTOP);
    sigdelset(&set,SIGCONT);
    sigdelset(&set,SIGVTALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}


void setTimer(int duration){
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = duration;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = duration;
    if(setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1)
       printf("error in settimer\n");
      
}

void defaultSigHandler(int signum){
    currThread->state = EXITED;
    mythread_exit(NULL);
    printf("\nSignal Handled %d\n",signum);
}
void setSignalHandlers(){
    signal(SIGTERM, defaultSigHandler);
}

void swapContext(sigjmp_buf* old, sigjmp_buf* new){
    int ret = sigsetjmp(*old, 1);
    if(ret == 0)
        siglongjmp(*new, 1);
}

void setScheduling(int sig){
    if(signal(sig, switchToScheduler) == SIG_ERR)
        exit(1);
}

void resetScheduling(int sig){
    if(signal(sig, SIG_IGN) == SIG_ERR)
        exit(1);
}

void deliverAsynchronousSignal(){
    resetScheduling(SIGVTALRM);
    sigset_t set;
    sigemptyset(&set);
    thread* th = currThread;
    int n = currThread->noOfPendingSignals;
    currThread->noOfPendingSignals = 0;
    for(int i=0; i<n; i++)
        sigaddset(&set, currThread->pendingSigArr[i]);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    for(int i = 0; i < n; i++)
        kill(getpid(), currThread->pendingSigArr[i]);
    ModifyThreadSignalsMask();
    setScheduling(SIGVTALRM);
}

void switchToScheduler(){
    resetScheduling(SIGVTALRM);
    deliverAsynchronousSignal();
    swapContext(currThread->context, schedulerThread->context);
    setScheduling(SIGVTALRM);
}




void scheduler(){
    resetScheduling(SIGVTALRM);  
    if(currThread->state == EXITED){
        removeThread(&sll, currThread);
    }
    if(currThread->state == RUNNING){

        currThread->state = RUNNABLE;
    }
    thread* nextThread = getRunnableThread(&sll);
    if(nextThread == NULL)
       return;

    currThread = nextThread;
    currThread->state = RUNNING;
    setScheduling(SIGVTALRM);
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
    setScheduling(SIGVTALRM);
    currThread->retval = (funptr)(functionarg); 
    void (*exitfun)() = currThread->thread_attr->exitfun;
    if(exitfun)
       exitfun();

    resetScheduling(SIGVTALRM);
    mythread_exit(NULL);
    return 0;
}


void initialiseContext(sigjmp_buf* context, void* stack, void* fun){
    sigsetjmp(*context, 1);
    if(stack)
        (*context)->__jmpbuf[5] = encrypt((long int)(stack));
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
    setSignalHandlers();
    sll.back = NULL;
    sll.front = NULL;

    currTid = getpid();

    mainThread = (thread*)malloc(sizeof(thread));
    initialiseThread(mainThread, getCurrTid(), NULL, NULL, NULL, RUNNING, 1);
    addToSLL(&sll,mainThread);
    currThread = mainThread;


    schedulerThread = (thread*)malloc(sizeof(thread));
    initialiseThread(schedulerThread, getNextTid(), scheduler, NULL, NULL,RUNNABLE, 2);
    
    setScheduling(SIGVTALRM);
    setTimer(1000);
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
    addToSLL(&sll, newthread);
    *tid = getCurrTid();
    sigsetjmp(*(mainThread->context), 1);
    setScheduling(SIGVTALRM);
    return 0;
}



int mythread_join(thread_id tid, void** retval){
    resetScheduling(SIGVTALRM);
    if(tid < 0){
        setScheduling(SIGVTALRM);
       return 1;
    } 
    thread* th = getThread(&sll, tid);
    if(th == NULL){
        setScheduling(SIGVTALRM);
        return 1;
    }
    if(th->state == EXITED){
        if(retval){
            (*retval) = th->retval;
            setScheduling(SIGVTALRM);
            return 0;
        }
    }
    currThread->state = WAITING;
    currThread->noOfJoins++;
    th->noOfWaiters++;
    th->waitersTid = realloc(th->waitersTid, sizeof(thread_id)*(th->noOfWaiters));
    th->waitersTid[th->noOfWaiters-1] = currThread->tid;
    switchToScheduler();
    return 0;
}


void mythread_exit(void* ret){
    resetScheduling(SIGVTALRM);
    currThread->state = EXITED;
    messageWaiters(currThread->waitersTid, currThread->noOfWaiters);
    removeThread(&sll, currThread);
    switchToScheduler();
}


int mythread_kill(thread_id tid,int sig){
    resetScheduling(SIGVTALRM);
    if(sig < 1 || sig > 31)
        return 1;
    if(tid <= 0)
        return 1;


    thread_id currid = getpid();
    if(sig == SIGINT || sig == SIGSTOP || sig == SIGCONT){
        kill(currid, sig);
        setScheduling(SIGVTALRM);
        return 0;
    }
    if(tid == currThread->tid){
        kill(getpid(), sig);
        setScheduling(SIGVTALRM);
        return 0;
    }
    thread* th = getThread(&sll, tid);
    if(th == NULL){
        setScheduling(SIGVTALRM);
        return 1;
    }
    th->noOfPendingSignals++;
    th->pendingSigArr = realloc(th->pendingSigArr, sizeof(int)*(th->noOfPendingSignals));
    th->pendingSigArr[th->noOfPendingSignals-1] = sig;
    setScheduling(SIGVTALRM);
    return 0;
}
