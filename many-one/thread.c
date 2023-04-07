#define _GNU_SOURCE
#include"thread.h"
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


sigset_t set;

singlyLL que;

thread_id currTid;






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

void switchToScheduler();
void scheduler();
void setScheduler(int sig){
    if(signal(sig, switchToScheduler) == SIG_ERR){
        exit(1);
    }
    return;
}




void intiManyToOne(){

    ModifyThreadSignalsMask();
    que.back = NULL;
    que.front = NULL;

    currTid = getpid();

    thread* main_thread = (thread*)malloc(sizeof(thread));
    sigjmp_buf main_thread_context = (sigjmp_buf*)malloc(sizeof(sigjmp_buf));
    // void* main_thread_stack = allocateStackSpace(STD_STACK_SIZE, STD_GUARD_SIZE);
    // initialiseContext(main_thread_context, main_thread_stack, NULL);
    initialiseThread(getNextTid(), NULL, NULL, NULL, RUNNING, main_thread_context);





    thread* scheduler_thread = (thread*)malloc(sizeof(thread));
    sigjmp_buf scheduler_thread_context = (sigjmp_buf*)malloc(sizeof(sigjmp_buf));
    void* scheduler_stack = allocateStackSpace(STD_STACK_SIZE, STD_GUARD_SIZE);
    initialiseContext(scheduler_thread_context, scheduler_stack, scheduler);
    initialiseThread(getNextTid(), scheduler, NULL, NULL, RUNNABLE, scheduler_thread_context);

    setScheduler(SIGVTALRM);
    setTimer(200);

}