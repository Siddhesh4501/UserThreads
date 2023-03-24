#include<stdio.h>




#define thread_id  unsigned int
#define memsize   unsigned long long int

typedef struct thread{
    thread_id tid;
    void*(*funptr)(void*);
    void* arg;
    memsize stacksize;
    memsize guardsize;
    void* stackpointer;
    struct thread * next;
    void*(*exitfun)(void*);
    int flags;
    void* retval;
} thread;



int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);
