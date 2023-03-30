#include "singlyLL.h"
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>

void clearResources(thread* thr){
    if(thr == NULL ) return;
    if(thr->thread_attr->stackpointer){
        free(thr->thread_attr->stackpointer);
    }
    free(thr->thread_attr);
    free(thr);
    return;
}


void initLL(thread** head){
    (*head) = NULL;
}


void insertInLL(thread** head,thread* newthread){
    newthread->next = NULL;
    if((*head) == NULL){
        (*head) = newthread;
        return;
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
        (*head) = curr->next;
        clearResources(curr);
        return;
    }
    prev->next = curr->next;
    clearResources(curr);
    return;
}

thread* getThreadFromTid(thread* head, thread_id tid){
    while(head){
        if(head->tidcopy == tid){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

void killToAllThreads(thread** head, thread_id tid, int sig){
    thread* curr = (*head);
    while(curr){
        if(curr->tid != tid){
            if(sig == SIGINT)
               deleteFromLL(head, curr->tid);
           tgkill(getpid(), curr->tid, sig);
        }
        curr = curr->next;
    }
}