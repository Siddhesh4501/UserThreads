#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include"../../one-one/mythread.h"


void* fun2(void* data){
	if(data != NULL)
	    printf("Thread id : %d,  Data  : %d\n", gettid(), (int*) data);
	else
	    printf("Thread id : %d\n", gettid());
}


int main(){

    int data = 10;

    thread_id t1,t2,t3;
    mythread_create(&t1, NULL, fun2, NULL);
    mythread_create(&t2, NULL, fun2, data);
    mythread_create(&t3, NULL, fun2, NULL);

    sleep(1);
    mythread_kill(t3, SIGINT);
    sleep(2);

    mythread_join(t1,NULL);
    mythread_join(t2,NULL);
    mythread_join(t3,NULL);

    return 0;
}
