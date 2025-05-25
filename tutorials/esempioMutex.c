#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <string.h> 
#include <unistd.h> 

#define USA_MUTEX
#define SEQUENZA_NESSUNO 0
#define SEQUENZA_A       1
#define SEQUENZA_B       2
#define SEQUENZA_C       3
#define SEQUENZA_D       4
#define SEQUENZA_E       5
#define SEQUENZA_D_o_E   6

pthread_t tid[2]; //due thread
int counter; 

pthread_mutex_t mutex;

void *body(void *arg){
    pthread_mutex_lock(&mutex);
    unsigned long i = 0;
    counter +=1;
    printf("\n Job %d has started\n", counter); 
    for (i = 0; i < (0xFFFFFFFF); i++) 
    ; 

    printf("\n Job %d has finished\n", counter); 

    pthread_mutex_unlock(&mutex);
  
}


int main(){
    int i=0;
    int error;

    if(pthread_mutex_init(&mutex, NULL) != NULL){
        printf("\n mutex init failed \n");
        return 1;
    }

    while(i<2){
        error = pthread_create(&(tid[i]), NULL, &body, NULL);
        if(error != 0){
            printf("\nThread cannot be created:[%s]", strerror(error));
        }
        i++;
    }

    pthread_join(tid[0],NULL);
    pthread_join(tid[1], NULL);
    pthread_mutex_destroy(&mutex);
    return 0;
}