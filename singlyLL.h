#include "mythread.h"


void clearResources(thread* thr);


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