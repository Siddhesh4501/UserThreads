#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>
#include "mythread.h"
#include "locks.h"
#include "singlyLL.h"

thread* head;
lock_t lock_var;




int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);
void initThreadStructures();


void* wrapper(void* arg){

    thread* newthread = (thread*)arg;

    void*(*funptr)(void*) = newthread->funptr;
    void* arg = newthread->arg;
    funptr(arg);
    void (*exitfun)() = newthread->thread_attr->exitfun;
    exitfun();
    deleteFromLL(&head, (thread_id)gettid());
}

int mythread_create(thread_id* tid, void* thread_attr,void*(*funptr)(void*), void* arg){
    static int is_first_thread = 1;
    if(is_first_thread){
        initThreadStructures();
        is_first_thread = 0;
    }
    lock_lock(&lock_var);


    thread* newthread = (thread*)malloc(sizeof(thread));

    if(newthread == NULL){
        return -1;
    }
    newthread->thread_attr = (attr*)malloc(sizeof(attr));

    newthread->funptr = funptr;
    newthread->arg = arg;
    newthread->thread_attr = thread_attr;
    newthread->next = NULL;

    if(newthread->thread_attr == NULL){
        attr* thattr = newthread->thread_attr;
        thattr->exitfun = NULL;
        thattr->flag = 0;
        thattr->stacksize = STD_STACK_SIZE;
        thattr->guardsize = STD_GUARD_SIZE;
    }
    else{
        memcpy(newthread->thread_attr, thread_attr, sizeof(attr));
    }
    newthread->thread_attr->stackpointer = mmap(NULL, newthread->thread_attr->stacksize + newthread->thread_attr->guardsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    thread_id cloneid = clone(wrapper, newthread->thread_attr->stackpointer, CLONE_FLAGS, newthread, &newthread->tid, NULL, &newthread->tid);
    newthread->tid = cloneid;

    insertInLL(&head,newthread);
    *tid = cloneid;

    lock_unlock(&lock_var);
}