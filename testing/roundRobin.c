#include "roundRobin.h"


void addToSLL(singlyLL* sll, thread* currthread);
thread* getRunnableThread(singlyLL* sll);
void moveThreadToEnd(singlyLL* sll, thread* currthread, thread* prev);
thread* getThread(singlyLL* sll, thread_id tid);
void removeThread(singlyLL* sll, thread* currthread);

void addToSLL(singlyLL* sll, thread* currthread){
    currthread->next = NULL;
    if(sll->front == NULL){
        sll->front = currthread;
        sll->back = currthread;
        return;
    }
    thread* head = sll->front;
    while(head->next)
        head = head->next;
    head->next = currthread;
    sll->back->next = currthread;
    sll->back = currthread;
}

thread* getRunnableThread(singlyLL* sll){
    thread* head = sll->front;
    thread* prev = NULL;
    printf("getrunn %p\n",head);
    while(head){
        if(head->state == RUNNABLE){
            moveThreadToEnd(sll, head, prev);
            return head;
        }
        prev = head;
        head = head->next;
    }
    return NULL;
}

thread* getThread(singlyLL* sll, thread_id tid){
    thread* head = sll->front;
    while(head){
        if(head->tid == tid)
          return head;
    }
    return NULL;
}

void moveThreadToEnd(singlyLL* sll, thread* currthread, thread* prev){
    if(sll->front == sll->back) return;
    if(currthread == sll->back) return;
    if(currthread == sll->front)
        sll->front = sll->front->next;
    else
       prev->next = currthread->next;
    sll->back->next = currthread;
    sll->back = currthread;
    currthread->next = NULL;
    // printf("in move end %p\n",sll->front);
}

void removeThread(singlyLL* sll, thread* currthread){
    printf("remove thread called\n");
    if((sll->front == sll->back) && (sll->front == currthread)){
        sll->front = NULL;
        sll->back = NULL;
        // freeSources(currthread);
        return;
    }
    if(sll->front == currthread){
        sll->front = sll->front->next;
        // freeSources(currthread);
        return;
    }
    thread* head = sll->front;
    thread* prev = NULL;
    while(head){
        if(head == currthread){
            prev->next = head->next;
            if(head->next == sll->back)
                sll->back = prev;
            // freeSources(currthread);
        }
        prev = head;
        head = head->next;
    }
    printSLL(*sll);
    return;
}

void printSLL(singlyLL sll){
    thread *head = sll.front;
    while(head != NULL){
        printf("threadId: %d\n",head->tid);
        head = head->next;
    }
    printf("head %p\n",head);
}