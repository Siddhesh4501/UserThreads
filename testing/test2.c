#include<stdio.h>

void fun(){
    printf("hello");
    return;
}
int main(){

    void (*funptr)() = fun;
    funptr();




    return 0;
}