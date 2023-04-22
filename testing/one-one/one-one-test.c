#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include"../../one-one/mythread.h"


void* fun1(void* data){
    int a = 10;
    return;
}

void* fun2(void* data){
	if(data != NULL)
	    printf("Thread id : %d,  Data  : %d\n", gettid(), (int*) data);
	else
	    printf("Thread id : %d\n", gettid());
}

void* fun3(void* data){
    int* a = (int*)malloc(sizeof(int));
    (*a) = 10;
    return (void*)a;
}

void* fun4(void* data){
    while(1){
    }
}
void* fun5(void* data){
    int tid = *((int*)data);
    mythread_kill(tid, SIGABRT);
}

void* fun6(void* data){
    mythread_exit(NULL);
    int* a = (int*)malloc(sizeof(int));
    (*a) = 10;
    return (void*)a;
}


void testCreate(void){
    thread_id ids[5];
    int succ = 0, fail= 0;
    for(int i = 0; i < 5; i++){
        int ret = mythread_create(&ids[i], NULL, fun1, NULL);
        if(ret == 0) succ++;
        else         fail++;
    }
    printf("Thread Create testing Success %d Failure %d\n",succ,fail);
}

void testJoin(void){
    thread_id ids[5];
    void * retval = malloc(sizeof(int));
    int succ = 0, fail= 0;
    for(int i = 0; i < 5; i++){
        int ret1 = mythread_create(&ids[i], NULL, fun3, NULL);
        int ret2 = mythread_join(ids[i],&retval);
        if(ret2 == 0){
            int* ret = (int*)retval;
            if((*ret) == 10) succ++;
        }
    }
    printf("Thread Join testing Success %d Failure %d\n",succ,fail);
}

void testKill(void){
       thread_id tid[2];
       int ret1 = mythread_create(&tid[0],NULL, fun4, NULL);
       int ret2 = mythread_create(&tid[1],NULL, fun5, &tid[0]);
       mythread_join(tid[0], NULL);
       mythread_join(tid[1], NULL);
       printf("Thread Kill succeed\n");
}


void testExit(void){
    thread_id tid;
    void * retval = malloc(sizeof(int));
    int ret1 = mythread_create(&tid,NULL, fun6, NULL);
    int ret2 = mythread_join(tid, retval);
    int* ret = (int*)retval;
    if((*ret) == 10)
       printf("Thread Exit failed\n");
    else
       printf("Thread Exit succeed\n");
       
    // pr
}

int main(){

    int data = 10;





    // thread_id t1,t2,t3;
    // mythread_create(&t1, NULL, fun2, NULL);
    // mythread_create(&t2, NULL, fun2, data);
    // mythread_create(&t3, NULL, fun2, NULL);

    // sleep(1);
    // mythread_kill(t3, SIGINT);
    // sleep(2);

    // mythread_join(t1,NULL);
    // mythread_join(t2,NULL);
    // mythread_join(t3,NULL);


    testCreate();
    testJoin();
    testExit();
    testKill();

    return 0;
}
