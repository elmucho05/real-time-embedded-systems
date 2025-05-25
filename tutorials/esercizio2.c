#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond_AB;
pthread_cond_t cond_Reset;
int reset_attivi;
int ab_attivi;
int reset_bloccati;
int ab_bloccati;

void myinit(void){
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&mutex, &m_attr);
    pthread_cond_init(&cond_AB, &c_attr);

    reset_attivi = ab_attivi = reset_bloccati = ab_bloccati = 0;
}
void pausetta(void){
    struct timespec t ;
    t.tv_sec = 0;
    t.tv_nsec = (rand()%10+1)*1000000;
    nanosleep(&t,NULL);
}

void startProcA(){
    pthread_mutex_lock(&mutex);
    while(reset_attivi || reset_bloccati){//se ci sono reset attivi o bloccati, mi blocco
        //setto prima le variabili
        ab_bloccati++;
        pthread_cond_wait(&cond_AB, &mutex);
        ab_bloccati--;
    }

    //ora che mi sono liberato, comincio ad eseguire
    ab_attivi++;
    pthread_mutex_unlock(&mutex);
}
void endProcA(){
    pthread_mutex_lock(&mutex);

    ab_attivi--;
    //who is my responsability to wake up?
    //priority to resets
    //and if i'm the last one, otherwise i have starvation of B 
    if(reset_bloccati && ab_attivi==0){
        pthread_cond_signal(&cond_Reset);
    }

    pthread_mutex_unlock(&mutex);
}
void startProcR(){
    pthread_mutex_lock(&mutex);
    //do i block myself
    while(ab_attivi){//when there is A or B executing
        reset_bloccati++;
        pthread_cond_wait(&cond_Reset,&mutex);
        reset_bloccati--;
    }
    reset_attivi++;
    pthread_mutex_unlock(&mutex);
}
void endProcR(){
    //one less active R
    pthread_mutex_lock(&mutex);
    reset_attivi--;
    if(ab_bloccati){//non c'è bisogno di controllare se è l'ultimo
        //RESET ha sempre la priorità,
        //se controlli anche se è l'ultimo, hai starvation degli AB 
        pthread_cond_broadcast(&cond_AB);
    }

    pthread_mutex_unlock(&mutex);
}


#define BUSY 1000000
#define CYCLE 50
void myprint(char *s){
    int i,j;
    fprintf(stderr,"[");
    for(j=0; j<CYCLE; j++){
        fprintf(stderr, s);
        for(i=0; i<BUSY; i++); //fa un ciclo per 1000000   
    }
    fprintf(stderr, "]");
}

//------------------------ //
// Simulazione accesso risorse condivise //
void ProcA(){
    myprint("+");
}
void ProcB(){
    myprint("-");
}
void ProcR(){
    myprint(".");
}

void *PA(void *arg){
    for(;;){//while(true)
        fprintf(stderr,"\nA");
        startProcA();//come un mutex
        ProcA(); //sumula un accedi alla risorsa A
        endProcA();
        fprintf(stderr,"a\n");
    }
}
void *PR(void *arg){
    for(;;){
        fprintf(stderr,"arrivato un R");
        startProcR();
        ProcR();
        endProcR();
        fprintf(stderr,"r");
        pausetta();//se non la metto, arriva sempre lui prima

    }   
}

int main(){
    pthread_attr_t my_attr;
    pthread_t my_thread;

    myinit(); //inizializza mutexes e altre var
    srand(555);//inizializza i numeri casuali

    pthread_attr_init(&my_attr);
    pthread_attr_setdetachstate(&my_attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&my_thread,&my_attr, PA, NULL);
    pthread_create(&my_thread,&my_attr, PR,NULL);

    pthread_attr_destroy(&my_attr);

    sleep(5);


    return 0;
}