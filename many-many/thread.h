#ifndef mythread
#define mythread
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#define thread_id  unsigned int
#define memsize   unsigned long long int
#define STD_STACK_SIZE 4096
#define STD_GUARD_SIZE 1024
#define SCHED_INTERVAL 200
#define NOOFKERNELTHREADS 2
#define CLONE_FLAGS CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID | CLONE_FILES | CLONE_THREAD | CLONE_FS | CLONE_SIGHAND  | CLONE_VM


typedef struct attr{
    int flag;
    memsize stacksize;
    memsize guardsize;
    void* stackpointer;
    void (*exitfun)();
}attr;

typedef enum thread_state {RUNNING, RUNNABLE, STOPPED, WAITING, EXITED} thread_state;

typedef struct thread{
    thread_id tid;
    void*(*funptr)(void*);
    void* arg;
    attr* thread_attr;
    void* retval;
    thread_state state;
    sigjmp_buf* context;
    int noOfPendingSignals;
    int* pendingSigArr;
    int noOfWaiters;
    thread_id* waitersTid;
    int noOfJoins;
    struct thread *next;
} thread;


typedef struct singlyLL{
    thread *front;
    thread *back;
}singlyLL;



typedef struct kernelThread{
    thread* userThread;
    int threadNo;
    thread_id tid;
} kernelThread;


int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid, void** retval);
int mythread_kill(thread_id tid,int sig);
void mythread_exit(void* ret);






#endif