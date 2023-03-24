#include<stdio.h>

#define thread_id  unsigned int
#define memsize   unsigned long long int
#define STD_STACK_SIZE 4096
#define STD_GUARD_SIZE 1024


#define CLONE_FLAGS CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID | CLONE_FILES | CLONE_THREAD | CLONE_FS | CLONE_SIGHAND | CLONE_IO | CLONE_PARENT_SETTID | CLONE_VM

// typedef struct funwitharg{
//     void*(*funptr)(void*);
//     void* arg;

// }funwitharg;

typedef struct attr{
    int flag;
    memsize stacksize;
    memsize guardsize;
    void* stackpointer;
    void (*exitfun)();
}attr;


typedef struct thread{
    thread_id tid;
    void*(*funptr)(void*);
    void* arg;
    struct thread * next;
    void* retval;
    attr* thread_attr;
} thread;



void initThreadStructures();

int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);
