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
    printf("Thread Exit Failed\n");
    int* a = (int*)malloc(sizeof(int));
    (*a) = 10;
    return (void*)a;
}

void extifun(){
    printf("Exit Function worked\n");
}

void* fun7(void* data){
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
    if((*ret) != 10)
       printf("Thread Exit succeed\n");
       
}

void testExitFun(){
    thread_id tid;
    attr th_attr;
    th_attr.exitfun = extifun;
    th_attr.stacksize = STD_STACK_SIZE;
    th_attr.guardsize = STD_GUARD_SIZE;
    void * retval = malloc(sizeof(int));
    int ret1 = mythread_create(&tid, &th_attr, fun7, NULL);
    int ret2 = mythread_join(tid, retval);
}


#define MAT_SIZE 10

int matrix1[MAT_SIZE][MAT_SIZE];
int matrix2[MAT_SIZE][MAT_SIZE];
int result[MAT_SIZE][MAT_SIZE];
int ans[MAT_SIZE][MAT_SIZE];
thread_id matTid[MAT_SIZE*MAT_SIZE];




typedef struct pair{
    int a,b;
} pair;


void* multiply(void* data){
    pair p = *((pair*) data);
    int sum = 0;
    for(int i=0; i < MAT_SIZE; i++){
        sum+=(matrix1[p.a][i]*matrix2[i][p.b]);
    }
    result[p.a][p.b] = sum;
}



int main(){


    testCreate();
    testJoin();
    testExit();
    testKill();
    testExitFun();


    for(int i = 0; i<MAT_SIZE; i++){
        for(int j=0; j<MAT_SIZE; j++){
            matrix1[i][j] = i*j  + 1;
        }
    }
    for(int i = 0; i<MAT_SIZE; i++){
        for(int j=0; j<MAT_SIZE; j++){
            matrix2[i][j] = i*j;
        }
    }
    int counter = 0;
    for(int i = 0; i<MAT_SIZE; i++){
        for(int j=0; j<MAT_SIZE; j++){
            pair p = {i,j};
            mythread_create(&matTid[counter], NULL, multiply, &p);
            mythread_join(matTid[counter], NULL);
            counter++;
        }
    }

   for(int i=0;i<MAT_SIZE;i++){
     for(int j=0;j<MAT_SIZE;j++){
        for(int k=0;k<MAT_SIZE;k++){
            ans[i][j] += matrix1[i][k] * matrix2[k][j];
        }
     }
   }

   for(int i=0;i<MAT_SIZE;i++){
     for(int j=0;j<MAT_SIZE;j++){
        if(ans[i][j] != result[i][j]){
            printf("Wrong Matrix multiplication\n");
            exit(1);
        }
     }
   }
   printf("Matrix multiplication is Correct\n");

    return 0;
}
