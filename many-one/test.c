#include<stdio.h>
#include<stdlib.h>
#include "mythread.h"


void* fun1(void* arg){
    if(arg){
        int* k = (int*)arg;
        mythread_kill(*k,2);
        printf("in t2\n");
        while(1){
            printf("in\n");
        }
    }
    while(1){
        printf("hello\n");
    }
}

void* fun2(void* d){
    printf("hello\n");
}

int main(){
    thread_id t1,t2,t3;
    int k = 10;
    int m = 9;
    // printf("hello\n");
    mythread_create(&t1, NULL, fun2, NULL);
    mythread_create(&t3, NULL, fun2, NULL);
    // sleep(1);
    mythread_create(&t2, NULL, fun2, &t1);
    // sleep(1);
    // mythread_create(&t2, NULL, fun1, &k);
    // mythread_create(&t3, NULL, fun1, &m);
    // sleep(1);
    // printf("%d %d \n",t1,t2);
    // mythread_kill(t1,2);
    // sleep(2);
    mythread_join(t1,NULL);
    mythread_join(t2,NULL);
    // mythread_join(t3,NULL);
    // mythread_join(t3,NULL);
    // sleep(1);

    return 0;
}