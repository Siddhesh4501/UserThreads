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
#include "mythread.h"
#include "locks.h"
#include "singlyLL.h"

thread* head;
lock_t lock_var;




int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);
int initialiseThreadObject(thread* newthread, void*(*funptr)(void*), void* arg, void* thread_attr);
void initThreadStructures();




void initThreadStructures(){
    // printf("ininitthread\n");
    
    lock_init(&lock_var);

    lock_lock(&lock_var);
    initLL(&head);
    lock_unlock(&lock_var);

    thread* newthread = (thread*)malloc(sizeof(thread));
    if(newthread == NULL)
       exit(100);
    initialiseThreadObject(newthread, NULL, NULL, NULL);
    // printf("initthreadfinish\n");
    newthread->tid = getpid();
    newthread->tidcopy = newthread->tid;

    lock_lock(&lock_var);
    insertInLL(&head, newthread);
    lock_unlock(&lock_var);

    return;

}

int wrapper(void* arg){
    // printf("in wrapper12323\n");


    // lock_lock(&lock_var);
    thread* newthread = (thread*)arg;
    
    
    void* functionarg = newthread->arg;

    void*(*funptr)(void*) = newthread->funptr;
    // printf("%p\n",funptr);
    newthread->retval = (funptr)(functionarg); 
    // printf("%d\n",*((int*)retval));
    void (*exitfun)() = newthread->thread_attr->exitfun;
    if(exitfun)
       exitfun();
    // printf("calling function\n"); 
    // printf("hello in wrapper\n");
    lock_lock(&lock_var);
    deleteFromLL(&head, gettid());
    lock_unlock(&lock_var);
    return 0;
}


void* allocateStackSpace(memsize stacksize, memsize guardsize){
    void* mem;
    mem = mmap(NULL, stacksize + guardsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS , -1, 0);
    if(mem == MAP_FAILED){
        return NULL;
    }
    // printf("in allocate stack %p \n",mem);
    return mem;

}

int initialiseThreadObject(thread* newthread, void*(*funptr)(void*), void* arg, void* thread_attr){
    newthread->thread_attr = (attr*)malloc(sizeof(attr));
    // printf("%d \n",sizeof(attr));
    if(newthread->thread_attr == NULL)
       return 1;
    newthread->funptr = funptr;
    newthread->arg = arg;
    newthread->next = NULL;

    if(thread_attr == NULL){
        // printf("%p\n",newthread->thread_attr);
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
        exit(1);
        return 1;
    }
        // printf("intinitialisthreadobject\n");
    return 0;

}



int mythread_create(thread_id* tid, void* thread_attr,void*(*funptr)(void*), void* arg){
    // printf("in mythread create\n");


    static int is_first_thread = 1;

    if(is_first_thread){
        is_first_thread = 0;
        initThreadStructures();
    }


    thread* newthread = (thread*)malloc(sizeof(thread));
    if(newthread == NULL){
        return 1;
    }
    if(initialiseThreadObject(newthread, funptr, arg, thread_attr) !=0 )
       return 1;


    // printf("%p\n",newthread->thread_attr->stackpointer);
    thread_id cloneid = clone(wrapper, newthread->thread_attr->stackpointer + newthread->thread_attr->stacksize + newthread->thread_attr->guardsize, CLONE_FLAGS, newthread, &(newthread->tid), NULL, &(newthread->tid));
    if(cloneid == -1){
        printf("thread failed\n");
        exit(1);
    }

    newthread->tid = cloneid;
    newthread->tidcopy = cloneid;
    lock_lock(&lock_var);
    insertInLL(&head,newthread);
    lock_unlock(&lock_var);
    *tid = cloneid;
    return 0;

}

int mythread_join(thread_id tid, void** retval){
    if(tid <= 0)
       return 1;
    lock_lock(&lock_var);
    thread* currentthread = getThreadFromTid(head, tid);
    lock_unlock(&lock_var);
    if(currentthread == NULL){
        // printf("in mythread join %d \n",tid);
        return 1;
    }
    while(currentthread->tid == tid){
        lock_lock(&lock_var);
        // printf("fdfd\n");
        lock_unlock(&lock_var);
    }
    // waitpid(tid, NULL, __WALL);
    if(retval != NULL){
        (*retval) = currentthread->retval;
    }
    return 0;
}