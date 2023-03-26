#include "singlyLL.h"
#include <stdlib.h>
#include <sys/mman.h>

void clearResources(thread* thr){
    if(thr == NULL ) return;
    // printf("munmap %p %d %d \n",thr->thread_attr->stackpointer,thr->thread_attr->stacksize , thr->thread_attr->guardsize);
    // if(thr->thread_attr->stackpointer)
    //     munmap(thr->thread_attr->stackpointer, thr->thread_attr->stacksize + thr->thread_attr->guardsize);
    // printf("in clear\n");
    free(thr->thread_attr);
    free(thr);
    return;
}


void initLL(thread** head){
    (*head) = NULL;
}


void insertInLL(thread** head,thread* newthread){
    // printf("ininsertll %d\n",newthread->tid);
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
    // printf("in delete tid =%d \n",gettid());
    clearResources(curr);
    return;
}

thread* getThreadFromTid(thread* head, thread_id tid){
    int count = 0;
    while(head){
        if(head->tidcopy == tid){
            return head;
        }
        head = head->next;
        // printf("%d %p\n",count++,head);
    }
    return NULL;
}