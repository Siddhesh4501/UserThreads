#include "singlyLL.h"
#include <stdlib.h>
#include <sys/mman.h>

void clearResources(thread* thr){
    if(thr == NULL ) return;
    munmap(thr->thread_attr->stackpointer, thr->thread_attr->stacksize + thr->thread_attr->guardsize);
    free(thr->thread_attr);
    free(thr);
    return;
}


void insertInLL(thread** head,thread* newthread){
    newthread->next = NULL;
    if((*head) == NULL){
        *head = newthread;
    }
    thread* curr = (*head);
    while(curr->next)
        curr = curr->next;
    curr->next = newthread;
    return;
}


void deleteFromLL(thread** head, thread_id tid){
    thread* curr = (*head);
    thread* prev = NULL;
    while(curr && curr->tid != tid){
        prev = curr;
        curr = curr->next;
    }
    if(curr == NULL) return;
    if(prev == NULL){
        *head = curr->next;
        clearResources(curr);
    }
    prev->next = curr->next;
    clearResources(curr);
    return;
}