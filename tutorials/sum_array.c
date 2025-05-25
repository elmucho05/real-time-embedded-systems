#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
/*
    this simple program sums the elements of the array
    - half of the sum is done my a thread
    - half by another
*/

int primes[10] = {2,3,5,7,11,13,17,19,23,29};

void *routine(void *arg){
    int index = *(int*)arg;
    int sum = 0;
    for(int i=0; i<5;i++){
        sum+=primes[index+i];//lo puoi fare anche così
    }
    printf("Local sum : %d\n", sum);
    *(int*)arg = sum;
    return arg;
}

int main(int argc, char *argv[]){

    pthread_t th[2];
    int i;
    for(i=0; i<2; i++){
        int *a = malloc(sizeof(int));
        *a = i*5;//gli dai o 0 o 5 come parametro
        if(pthread_create(&th[i], NULL, &routine, a)!=0){
            perror("Failed to create thread");
        }
    }
    int globalSum = 0;
    for(i=0;i<2;i++){
        int *r;
        if(pthread_join(th[i], (void*)&r)!=0){//put the return value to r
            perror("Failed to join thread");
        }
        globalSum +=*r;
        free(r);
    }

    printf("Global sum: %d\n", globalSum);
    return 0;
}