#include<stdio.h>
#include<stdlib.h>

int main(){

    int *a = malloc(sizeof(int));
    *a = 5;
    printf("%d",*a);

    return 0;
}