#include<stdio.h>
#include<stdlib.h>
#include "roundRobin.h"
#include "thread.h"

int main(){
    singlyLL sll;
    sll.front = NULL;
    sll.back = NULL;
    thread *t1 = (thread*)malloc(sizeof(thread));
    t1->tid = 100;
    thread *t2 = (thread*)malloc(sizeof(thread));
    t2->tid = 101;
    thread *t3 = (thread*)malloc(sizeof(thread));
    t3->tid = 102;

    addToSLL(&sll,t1);
    addToSLL(&sll,t2);
    moveThreadToEnd(&sll,t1,NULL);
    removeThread(&sll, t2);
    printSLL(sll);
    // printSLL(sll);



    return 0;
}