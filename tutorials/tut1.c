#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

int mails = 0;
pthread_mutex_t mutex;
int primes[4] = {2,3,5,7};

void *routine(void *arg){
    int index = *(int *)arg;
    for(int i=0; i<100000; i++){
        pthread_mutex_lock(&mutex);
        mails++;
        pthread_mutex_unlock(&mutex);
    }

    printf("%d", primes[index]);
    free(arg);//free the memor you allocated dinamically

}

int main(int argc, char* argv[]){
    pthread_t th[4];
    pthread_mutex_init(&mutex,NULL);
    for(int i=0; i<4; i++){
        int *a = malloc(sizeof(int));//but you have to free te momory
        *a = i;//value of a is i
        if(pthread_create(th +i, NULL, &routine, a)!=0){
            perror("Failed to create thread");
            return 1;
        }
        printf("Thread %d has started\n",i);
    }
    for(int i=0; i<4; i++){
        if(pthread_join(th[i], NULL) != 0)
            return 2;
        printf("Thread %d has ended\n", i);
    }

    pthread_mutex_destroy(&mutex);
    printf("Number of mails: %d", mails);

    return 0;
}