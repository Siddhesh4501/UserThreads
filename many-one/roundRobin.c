#include "roundRobin.h"


void addToQueue(singlyLL* sll, thread* currthread);
thread* getRunnableThread(singlyLL* sll);
void moveThreadToEnd(singlyLL* sll, thread* currthread, thread* prev);
thread* removeThread(singlyLL* sll, thread* currthread);
thread* getThread(singlyLL* sll, thread_id tid);

void addToSLL(singlyLL* sll, thread* currthread){
    currthread->next = NULL;
    if(sll->front == NULL && sll->back){
        sll->front = currthread;
        sll->back = currthread;
    }
    thread* head = sll->front;
    while(head->next)
        head = head->next;
    head->next = currthread;
}

thread* getRunnableThread(singlyLL* sll){
    thread* head = sll->front;
    thread* prev = NULL;
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
}

thread* removeThread(singlyLL* sll, thread* currthread){
    if((sll->front == sll->back) && (sll->front == currthread)){
        sll->front = NULL;
        sll->back = NULL;
        freeSources(currthread);
        return;
    }
    if(sll->front == currthread){
        sll->front = sll->front->next;
        freeSources(currthread);
        return;
    }
    thread* head = sll->front;
    thread* prev = NULL;
    while(head){
        if(head == currthread){
            prev->next = head->next;
            freeSources(currthread);
        }
        prev = head;
        head = head->next;
    }
    return;
}