#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include"../../many-many/thread.h"


void* fun1(void* d){
    printf("Function 1 %d\n",gettid());
}
void* fun2(void* d){
    printf("Function 2 %d\n",gettid());
}

int main(){
    thread_id t1,t2;

    mythread_create(&t1, NULL, fun2, NULL);
    mythread_create(&t2, NULL, fun1, NULL);
   
    mythread_join(t1, NULL);
    mythread_join(t2, NULL);

    return 0;
}

