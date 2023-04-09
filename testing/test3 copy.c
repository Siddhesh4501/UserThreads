#include<stdio.h>
#include<stdlib.h>
#include "many-one/roundRobin.h"
#include "many-one/thread.h"

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
    printSLL(sll);



    return 0;
}