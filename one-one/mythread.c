#include<stdio.h>
#include "mythread.h"


int mythread_create(thread_id* tid, void* attr,void*(*funptr)(void*), void* arg);
int mythread_join(thread_id tid,void** retval);
int mythread_kill(thread_id tid,int sig);


