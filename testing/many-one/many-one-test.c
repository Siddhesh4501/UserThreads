#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include"../../many-one/thread.h"


void* fun1(void* data){
    int a = 10;
}

// void* fun2(void* data){
// 	if(data != NULL)
// 	    printf("Thread id : %d,  Data  : %d\n", gettid(), (int*) data);
// 	else
// 	    printf("Thread id : %d\n", gettid());
// }

void* fun3(void* data){
    int* a = (int*)malloc(sizeof(int));
    (*a) = 10;
    return (void*)a;
}

void* fun4(void* data){
    while(1);
}
void* fun5(void* data){
    int tid = *((int*)data);
    mythread_kill(tid, SIGTERM);
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
    int succ = 0, fail= 0;
    for(int i = 0; i < 5; i++){
        int ret1 = mythread_create(&ids[i], NULL, fun3, NULL);
        int ret2 = mythread_join(ids[i], NULL);
        if(ret2 == 0){
            succ++;
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

    testCreate();
    testJoin();
    testExit();
    testKill();

    return 0;
}
