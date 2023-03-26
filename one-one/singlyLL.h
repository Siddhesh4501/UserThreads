#ifndef singlyll
#define singlyll

#include "mythread.h"


void clearResources(thread* thr);
void initLL(thread** head);
void insertInLL(thread** head,thread* newthread);
thread* getThreadFromTid(thread* head, thread_id tid);
void deleteFromLL(thread** head, thread_id tid);

#endif