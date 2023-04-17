#include<stdio.h>
#include<stdlib.h>
#include "thread.h"


// void* fun1(void* arg){
//     if(arg){
//         int* k = (int*)arg;
//         mythread_kill(*k,2);
//         printf("in t2\n");
//         while(1){
//             printf("in\n");
//         }
//     }
//     while(1){
//         printf("hello\n");
//     }
// }

void* fun1(void* d){
    // mythread_exit(NULL);
    printf("helloworld!!!!!!!!!!!! %d\n",gettid());
}
void* fun2(void* d){
    // mythread_exit(NULL);
    printf("hello!!!!!!!!!!!! %d\n",gettid());
}

int main(){
    thread_id t1,t2,t3;
    int k = 10;
    int m = 9;
    printf("hello %d\n",gettid());
    mythread_create(&t1, NULL, fun2, NULL);
    mythread_create(&t2, NULL, fun1, NULL);
    // printf("%d \n",t1);
    // mythread_create(&t2, NULL, fun2, NULL);
   
    // while(1){
    //     // printf("hello\n");
    // }
    // mythread_join(t1, NULL);
    // mythread_join(t2, NULL);
    while(1);
    return 0;
}