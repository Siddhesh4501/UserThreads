#include<stdio.h>
#include<stdlib.h>
#include "mythread.h"


void* fun1(void* arg){
    static int a = 0;
    int* k = (int*)arg;
    // if((*k) == 9){
    //     sleep(2);
    // }
    // printf("Hello world111 %d %d \n",(*k),a++);
    // printf("Hello world\n");
    // fprintf(stdout, "Hello world\n");
    printf("hello\n");
    // fflush(stdout);
    // int* p = (int*)malloc(sizeof(int));
    // *p = 1111;
    // return p;
}

int main(){
    thread_id t1,t2,t3;
    int k = 10;
    int m = 9;

    mythread_create(&t1, NULL, fun1, &k);
    // sleep(1);
    mythread_create(&t2, NULL, fun1, &k);
    mythread_create(&t3, NULL, fun1, &m);
    // sleep(1);
    // printf("%d %d \n",t1,t2);
    mythread_join(t1,NULL);
    mythread_join(t2,NULL);
    // mythread_join(t3,NULL);
    // mythread_join(t3,NULL);
    // sleep(1);

    return 0;
}