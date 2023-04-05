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
#include "mythread.h"
#include "locks.h"
#include "singlyLL.h"

thread* head;
lock_t lock_var;
lock_t lock_for_init;



int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);
void mythread_exit(void* ret); 
int initialiseThreadObject(thread* newthread, void*(*funptr)(void*), void* arg, void* thread_attr);
void initThreadStructures();


void initThreadStructures(){
    lock_init(&lock_var);

    lock_lock(&lock_var);
    initLL(&head);
    lock_unlock(&lock_var);

    thread* newthread = (thread*)malloc(sizeof(thread));
    if(newthread == NULL){
        perror("Error in creating main thread\n");
        return;
    }
    initialiseThreadObject(newthread, NULL, NULL, NULL);
    newthread->tid = getpid();
    newthread->tidcopy = newthread->tid;

    lock_lock(&lock_var);
    insertInLL(&head, newthread);
    lock_unlock(&lock_var);

    return;
}

int wrapper(void* arg){
    thread* newthread = (thread*)arg;
    void* functionarg = newthread->arg;
    void*(*funptr)(void*) = newthread->funptr;
    newthread->retval = (funptr)(functionarg); 
    void (*exitfun)() = newthread->thread_attr->exitfun;
    if(exitfun)
       exitfun();
    mythread_exit(NULL);
    return 0;
}


void* allocateStackSpace(memsize stacksize, memsize guardsize){
    void* mem;
    mem = malloc(stacksize + guardsize);
    if(mem == NULL){
        perror("malloc failed in stack\n");
        return NULL;
    }
    return mem;

}

int initialiseThreadObject(thread* newthread, void*(*funptr)(void*), void* arg, void* thread_attr){
    newthread->thread_attr = (attr*)malloc(sizeof(attr));
    if(newthread->thread_attr == NULL){
        perror("Error in creating thread attribute\n");
        return 1;
    }
    newthread->funptr = funptr;
    newthread->arg = arg;
    newthread->next = NULL;

    if(thread_attr == NULL){
        attr* thattr = newthread->thread_attr;
        thattr->exitfun = NULL;
        thattr->flag = 0;
        thattr->stacksize = STD_STACK_SIZE;
        thattr->guardsize = STD_GUARD_SIZE;
    }
    else{
        memcpy(newthread->thread_attr, thread_attr, sizeof(attr));
    }
    newthread->thread_attr->stackpointer = allocateStackSpace(newthread->thread_attr->stacksize, newthread->thread_attr->guardsize);
    if(newthread->thread_attr->stackpointer == NULL){
        perror("Error in allocating memory for stack\n");
        return 1;
    }
    return 0;

}

int mythread_create(thread_id* tid, void* thread_attr,void*(*funptr)(void*), void* arg){
    static int is_first_thread = 1; //
    
    if(is_first_thread){
        lock_lock(&lock_for_init);
        
        is_first_thread = 0;
        initThreadStructures();
        
        lock_unlock(&lock_for_init);
    }
    else{

        thread* newthread = (thread*)malloc(sizeof(thread));
        if(newthread == NULL){
            return 1;
        }
        if(initialiseThreadObject(newthread, funptr, arg, thread_attr) !=0 )
        return 1;

        thread_id cloneid = clone(wrapper, newthread->thread_attr->stackpointer + newthread->thread_attr->stacksize + newthread->thread_attr->guardsize, CLONE_FLAGS, newthread, &(newthread->tid), NULL, &(newthread->tid));
        // printf("clone %d\n",cloneid);
        if(cloneid == -1){
            perror("clone failed\n");
            return 1;
        }
        newthread->tid = cloneid;
        newthread->tidcopy = cloneid;
        lock_lock(&lock_var);
        insertInLL(&head,newthread);
        lock_unlock(&lock_var);

        *tid = cloneid;
        return 0;

    }

}

int mythread_join(thread_id tid, void** retval){
    if(tid <= 0)
       return 1;
    lock_lock(&lock_var);//No need
    thread* currentthread = getThreadFromTid(head, tid);
    lock_unlock(&lock_var);
    if(currentthread == NULL){
        return 1;
    }
    while(currentthread && currentthread->tid == tid){
    }
    if(retval != NULL){
        (*retval) = currentthread->retval;
    }
    return 0;
}

int mythread_kill(thread_id tid, int sig){
    if(sig < 1 || sig > 31)
        return 1;
    if(tid <= 0)
        return 1;


    thread_id currid = gettid();
    if(sig == SIGINT || sig == SIGSTOP || sig == SIGCONT){
        lock_lock(&lock_var);
        killToAllThreads(&head, currid, sig);
        lock_unlock(&lock_var);
    }
    if(sig == SIGINT){
        lock_lock(&lock_var);
        deleteFromLL(&head, currid);
        lock_unlock(&lock_var); 
    }
    tgkill(getpid(), currid, sig);
    return 0;
}



void mythread_exit(void* ret){

    thread_id tid = gettid();
    lock_lock(&lock_var);
    thread* currentthread = getThreadFromTid(head, tid);
    lock_unlock(&lock_var);
    if(currentthread == NULL){
        return;
    }
    if(ret != NULL){
        currentthread->retval = ret;
    }

    lock_lock(&lock_var);
    deleteFromLL(&head, tid);
    lock_unlock(&lock_var);
    tgkill(getpid(), gettid(), SIGINT);
}