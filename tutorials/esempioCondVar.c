#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define USA_SEM

#define SEQUENZA_NESSUNO 0


#define SEQUENZA_A       1
#define SEQUENZA_B       2
#define SEQUENZA_C       3
#define SEQUENZA_D       4
#define SEQUENZA_E       5
#define SEQUENZA_D_o_E   6

#ifdef USA_SEM
struct gestore_t
{
    sem_t mutex;
    sem_t semaforo_a, semaforo_b, semaforo_c, semaforo_d, semaforo_e;
    int bloccati_a, bloccati_b, bloccati_c,bloccati_d, bloccati_e;
    int numero_b_attivi;

    int stato;
}gestore;


void init_gestore(struct gestore_t *g){
    sem_init(&g->mutex,0,1); //il semaforo mutex è sempre inzializzato a 0
    
    sem_init(&g->semaforo_a,0,0);
    sem_init(&g->semaforo_b,0,0);
    sem_init(&g->semaforo_c,0,0);
    sem_init(&g->semaforo_d,0,0);
    sem_init(&g->semaforo_e,0,0);

    g->stato = SEQUENZA_NESSUNO;
    g->bloccati_a = g->bloccati_b = g->bloccati_c = g->bloccati_d = g->bloccati_e = 0;
    g->numero_b_attivi = 0;
}

void startA(struct gestore_t *g){
    sem_wait(&g->mutex);

    //when using SEMAPHORES you ask
    //1. when do i NOT have to block myself?
    //  -when i am in the correct sequence
    // ATTENTION : the blocked_a-- is done by whomever awakes me
    if(g->stato == SEQUENZA_NESSUNO){
        g->stato = SEQUENZA_A;
        sem_post(&g->semaforo_a);
    }else{
        //if i'm not in the correct sequence
        //i will block myself
        g->bloccati_a++;
    }
    
    //first release the mutex
    sem_post(&g->mutex);
    //then block
    sem_wait(&g->semaforo_a);
}
void endA(struct gestore_t *g){
    sem_wait(&g->mutex);

    //who am i responsible to wake up?
    g->stato = SEQUENZA_B;
    while(g->bloccati_b){
        g->bloccati_b--;
        g->numero_b_attivi++;//sono IO quello che aggiorno i contatori di chi sveglio
        sem_post(&g->semaforo_b);
    }
    sem_post(&g->mutex);
}

void startB(struct gestore_t *g){
    sem_wait(&g->mutex);
    //sequenza giusta
    if(g->stato == SEQUENZA_B){
        g->numero_b_attivi++;
        sem_post(&g->semaforo_b);
    }else{
        g->bloccati_b++;
    }

    sem_post(&g->mutex);
    sem_wait(&g->semaforo_b);
}


void svegliaAorC(struct gestore_t *g)
{
  /* chiamata alla fine di B, D o E per controllare se devo svegliare
     qualcuno alla fine di una sequenza */
  if (g->bloccati_a) {
    g->bloccati_a--;
    g->stato = SEQUENZA_A;
    sem_post(&g->semaforo_a);
  } 
  else if (g->bloccati_c) {
    g->bloccati_c--;
    g->stato = SEQUENZA_C;
    sem_post(&g->semaforo_c);
  }else
    g->stato = SEQUENZA_NESSUNO;
  // Nota: non è nella parte else!!! 
  //quando devi decidere se mettere la sequenza A o C, metti
  //come stato --> SEQUENZA_NESSUNO
}


void endB(struct gestore_t *g){
    sem_wait(&g->mutex);
    g->numero_b_attivi--;
    //se sono l'ultimo
    if(g->numero_b_attivi == 0)
        svegliaAorC(g);    
    sem_post(&g->mutex);
}

void startC(struct gestore_t *g){
    sem_wait(&g->mutex);
    //quand è che NON mi devo bloccare?
    if(g->stato == SEQUENZA_NESSUNO){
        //libero tutti i c 
        g->stato = SEQUENZA_C;
        sem_post(&g->semaforo_c);
    }else{
        g->bloccati_c++;
    }
    sem_post(&g->mutex);
    //mi blocco
    sem_wait(&g->semaforo_c);
}
void endC(struct gestore_t *g){
    sem_wait(&g->mutex);
    //who do i have to wake up? E or D
    if(g->bloccati_d){
        //sveglio qualcuno e gli aggiorno i contatori e stati
        g->bloccati_d--;
        g->stato = SEQUENZA_D;
        sem_post(&g->semaforo_d);
    }else if(g->bloccati_e){
        g->bloccati_e--;
        g->stato = SEQUENZA_E;
        sem_post(&g->semaforo_e);
    }else
        g->stato = SEQUENZA_D_o_E;

    sem_post(&g->mutex);
}

void startD(struct gestore_t *g){
    sem_wait(&g->mutex);
    if(g->stato == SEQUENZA_D_o_E){
        g->stato = SEQUENZA_D;
        sem_post(&g->semaforo_d);
    }else{
        g->bloccati_d++;
    }
    sem_post(&g->mutex);
    sem_wait(&g->semaforo_d);
}

void endD(struct gestore_t *g){
    sem_wait(&g->mutex);
    //who do i have to wake up, A or C
    svegliaAorC(g);
    sem_post(&g->mutex);
}

void startE(struct gestore_t *g){
    sem_wait(&g->mutex);

    if(g->stato == SEQUENZA_D_o_E){
        g->stato = SEQUENZA_E;
        sem_post(&g->semaforo_e);
    }else{
        g->bloccati_e++;
    }

    sem_post(&g->mutex);
    sem_wait(&g->semaforo_e);
}

void endE(struct gestore_t *g){
    sem_wait(&g->mutex);
    svegliaAorC(g);
    sem_post(&g->mutex);
}

#endif

#ifdef USA_MUTEX
struct gestore_t
{
    pthread_mutex_t mutex;
    pthread_cond_t condizione_a,condizione_b, condizione_c,condizione_d,condizione_e;
    int bloccati_a, bloccati_b, bloccati_c,bloccati_d, bloccati_e;
    int numbero_processi_b;
    int stato;
}gestore;




void init_gestore(struct gestore_t *g){
    pthread_mutexattr_t my_attr;
    pthread_condattr_t condition_attr;

    pthread_mutexattr_init(&my_attr);
    pthread_condattr_init(&condition_attr);

    pthread_mutex_init(&g->mutex, &my_attr);
    pthread_cond_init(&g->condizione_a, &condition_attr);
    pthread_cond_init(&g->condizione_b, &condition_attr);
    pthread_cond_init(&g->condizione_c, &condition_attr);
    pthread_cond_init(&g->condizione_d, &condition_attr);
    pthread_cond_init(&g->condizione_e, &condition_attr);

    pthread_condattr_destroy(&condition_attr);
    pthread_mutexattr_destroy(&my_attr);
    
    g->bloccati_a = g->bloccati_b = g->bloccati_c = g->bloccati_d = g->bloccati_e = 0;
    g->numbero_processi_b = 0;
    g->stato = SEQUENZA_NESSUNO;
}

void startA(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    
//do i need to block myself?
    while(g->stato != SEQUENZA_NESSUNO){
        g->bloccati_a++;
        pthread_cond_wait(&g->condizione_a, &g->mutex);
        g->bloccati_a--;
    }

    //now i am actually executing
    g->stato = SEQUENZA_A;
    pthread_mutex_unlock(&g->mutex);
}
void endA(struct gestore_t *g)
{
  pthread_mutex_lock(&g->mutex);
  
  //now the sequence is B
  g->stato = SEQUENZA_B;
  //who do i need to wake up?
    if(g->bloccati_b)
        pthread_cond_broadcast(&g->condizione_b);
    
  pthread_mutex_unlock(&g->mutex);
}

void startB(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    
    //do i need to block myself?
    while(g->stato!=SEQUENZA_B){
        g->bloccati_b++;
        pthread_cond_wait(&g->condizione_b, &g->mutex);
        g->bloccati_b--;
    }
    
    g->numbero_processi_b++;
    pthread_mutex_unlock(&g->mutex);
}
void svegliaAorC(struct gestore_t *g)
{
  /* chiamata alla fine di B, D o E per controllare se devo svegliare
     qualcuno alla fine di una sequenza */
  if (g->bloccati_a) {
    pthread_cond_signal(&g->condizione_a);
  } 
  else if (g->bloccati_c) {
    pthread_cond_signal(&g->condizione_c);
  }

  // Nota: non è nella parte else!!! 
  //quando devi decidere se mettere la sequenza A o C, metti
  //come stato --> SEQUENZA_NESSUNO
  g->stato = SEQUENZA_NESSUNO;
}

void endB(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    //one less active b
    g->numbero_processi_b--;
    //am i the last active one?
    if(g->numbero_processi_b == 0){
        svegliaAorC(g);
    }
    pthread_mutex_unlock(&g->mutex);
}

void startC(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    //means sequence C was chosen
    //but first, do i have to block myself
    while(g->stato != SEQUENZA_NESSUNO){
        g->bloccati_c++;//block
        pthread_cond_wait(&g->condizione_c, &g->mutex);//wait on condition c
        g->bloccati_c--;
    } 

    g->stato = SEQUENZA_C;
    pthread_mutex_unlock(&g->mutex);
}

void endC(struct gestore_t *g){
    //who do i need to wake up next? sequenza D oppure E, quindi 
    //setta lo status a DoE
    pthread_mutex_lock(&g->mutex);
    if(g->bloccati_d){
        pthread_cond_signal(&g->condizione_d);
    }else if(g->bloccati_e){
        pthread_cond_signal(&g->condizione_e);
    }    
    g->stato = SEQUENZA_D_o_E;
    pthread_mutex_unlock(&g->mutex);
}

void startD(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    //significa che mi hanno sbloccato

    //ma prima mi chiedo, IO MI DEVO BLOCCARE?
    while(g->stato!=SEQUENZA_D_o_E){
        g->bloccati_d++;
        pthread_cond_wait(&g->condizione_d, &g->mutex);
        g->bloccati_d--;
    }

    g->stato = SEQUENZA_D;
    pthread_mutex_unlock(&g->mutex);
}
void endD(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);

    //who do i need to signal?
    svegliaAorC(g);

    pthread_mutex_unlock(&g->mutex);

}

void startE(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    while(g->stato != SEQUENZA_D_o_E){
        g->bloccati_e++;
        pthread_cond_wait(&g->condizione_e,&g->mutex);
        g->bloccati_e--;
    }

    g->stato = SEQUENZA_E;
    pthread_mutex_unlock(&g->mutex);
}
void endE(struct gestore_t *g){
    pthread_mutex_lock(&g->mutex);
    svegliaAorC(g);
    pthread_mutex_unlock(&g->mutex);
}
#endif
void pausetta(void)
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand()%10+1)*1000000;
  nanosleep(&t,NULL);
}


void *A(void *arg)
{
  for (;;) {
    startA(&gestore);
    putchar(*(char *)arg);
    endA(&gestore);
    pausetta();
  }
  return 0;
}

void *B(void *arg)
{
  for (;;) {
    startB(&gestore);
 
    
    putchar(*(char *)arg);
    endB(&gestore);
    pausetta();
  }
  return 0;
}

void *C(void *arg)
{
  for (;;) {
    startC(&gestore);
    putchar(*(char *)arg);
    endC(&gestore);
    pausetta();
  }
  return 0;
}

void *D(void *arg)
{
  for (;;) {
    startD(&gestore);
    putchar(*(char *)arg);
    endD(&gestore);
    pausetta();
  }
  return 0;
}

void *E(void *arg)
{
  for (;;) {
    startE(&gestore);
    putchar(*(char *)arg);
    endE(&gestore);
    pausetta();
  }
  return 0;
}

int main(){
    pthread_attr_t attributo_thread;
    pthread_t mio_thread;

    init_gestore(&gestore);
    srand(555);

    pthread_attr_init(&attributo_thread);
    pthread_attr_setdetachstate(&attributo_thread, PTHREAD_CREATE_DETACHED);
    
    pthread_create(&mio_thread, &attributo_thread, A, (void *)"a");
    pthread_create(&mio_thread, &attributo_thread, A, (void *)"A");
    
    pthread_create(&mio_thread, &attributo_thread, B, (void *)"b");
    pthread_create(&mio_thread, &attributo_thread, B, (void *)"B");
    pthread_create(&mio_thread, &attributo_thread, B, (void *)"x");
    
    pthread_create(&mio_thread, &attributo_thread, C, (void *)"c");
    pthread_create(&mio_thread, &attributo_thread, C, (void *)"C");

    pthread_create(&mio_thread, &attributo_thread, D, (void *)"d");
    pthread_create(&mio_thread, &attributo_thread, D, (void *)"D");

    pthread_create(&mio_thread, &attributo_thread, E, (void *)"e");
    pthread_create(&mio_thread, &attributo_thread, E, (void *)"E");

    


    pthread_attr_destroy(&attributo_thread);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(5);
  
    return 0;
}