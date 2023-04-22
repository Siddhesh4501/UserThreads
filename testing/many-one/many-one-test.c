#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include"../../many-one/thread.h"


void exitfun(){
    printf("exit - Thread id: %d\n", gettid());
}
void* fun2(void* d){
    printf("Thread Id: %d\n", gettid());
}

int main(){
    thread_id t1,t2,t3;
    int k = 10;
    int m = 9;
    attr at;
    at.exitfun = exitfun;
    at.guardsize = STD_GUARD_SIZE;
    at.stacksize = STD_GUARD_SIZE;

    mythread_create(&t1, NULL, fun2, NULL);
    mythread_create(&t2, NULL, fun2, NULL);
    mythread_create(&t3, &at, fun2, NULL);

    mythread_join(t1, NULL);
    mythread_join(t3, NULL);

    return 0;
}
