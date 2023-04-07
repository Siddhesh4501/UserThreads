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
thread* scheduler_thread;
thread* mainThread;

void intiManyToOne();
int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid, void** retval);
int mythread_kill(thread_id tid,int sig);
void mythread_exit(void* ret);



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


void ModifyThreadSignalsMask(){
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
    setitimer(ITIMER_VIRTUAL,&timer,NULL);
}





void setScheduling(int sig){
    if(signal(sig, switchToScheduler) == SIG_ERR){
        exit(1);
    }
    return;
}

void resetScheduling(int sig){
    if(signal(sig, SIG_IGN) == SIG_ERR){
        exit(1);
    }
    return;
}

void switchToScheduler(){

}


void scheduler(){
    resetScheduling(SIGVTALRM);
    if(currThread->state == EXITED)
        removeThread(&sll, currThread);
    if(currThread->state == RUNNING)
        currThread->state = RUNNABLE;
    thread* nextThread = getRunnableThread(&sll);
    if(nextThread == NULL)
       return;
    currThread = nextThread;
    currThread->state = RUNNING;
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
    mythread_exit(NULL);

    resetScheduling(SIGVTALRM);

    currThread->state = EXITED;
    messageWaiters(currThread->waitersTid, currThread->noOfWaiters);
    switchToScheduler();
    return 0;
}


void initialiseContext(sigjmp_buf* context, void* stack, void* fun){
    sigsetjmp(*context, 1);
    (*context)->__jmpbuf[6] = encrypt((long int)stack);
    if(fun)
        (*context)->__jmpbuf[7] = encrypt((long int)fun);
}

void initManyToOne(){

    ModifyThreadSignalsMask();
    sll.back = NULL;
    sll.front = NULL;

    currTid = getpid();

    thread* main_thread = (thread*)malloc(sizeof(thread));
    sigjmp_buf* main_thread_context = (sigjmp_buf*)malloc(sizeof(sigjmp_buf));
    initialiseThread(getNextTid(), NULL, NULL, NULL, RUNNING, main_thread_context);
    addToSLL(&sll,main_thread);
    currThread = main_thread;


    scheduler_thread = (thread*)malloc(sizeof(thread));
    sigjmp_buf* scheduler_thread_context = (sigjmp_buf*)malloc(sizeof(sigjmp_buf));
    void* scheduler_stack = allocateStackSpace(STD_STACK_SIZE, STD_GUARD_SIZE);
    initialiseContext(scheduler_thread_context, scheduler_stack + STD_STACK_SIZE +STD_GUARD_SIZE, scheduler);
    initialiseThread(getNextTid(), scheduler, NULL, NULL, RUNNABLE, scheduler_thread_context);

    setScheduler(SIGVTALRM);
    setTimer(200);
}