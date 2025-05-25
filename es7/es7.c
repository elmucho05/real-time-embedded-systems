#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define SHAVING_ITERATIONS 1000
#define PAYING_ITERATIONS 1000
#define NUM_CLI 10
struct gestore_t {
  pthread_mutex_t mutex;
  pthread_cond_t divano;   // coda processi bloccati sul divano, max 4, sennò
                           // fuori
  pthread_cond_t barbiere; // coda dei processi bloccati sul barbi, max 3
  pthread_cond_t cassiere; // coda dei processi bloccati sul cassiere, la
                           // variabile che tiene il numero è booleana
  bool cassiere_libero;
  int num_attivi_divano;
  int num_attivi_barbiere;

} g_gestore;

void initgestore(struct gestore_t *g) {
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&g->mutex, &m_attr);
  pthread_cond_init(&g->divano, &c_attr);
  pthread_cond_init(&g->barbiere, &c_attr);
  pthread_cond_init(&g->cassiere, &c_attr);

  g->num_attivi_barbiere = 0;
  g->num_attivi_divano = 0;
  g->cassiere_libero = true;

  pthread_mutexattr_destroy(&m_attr);
  pthread_condattr_destroy(&c_attr);
}

void *cliente(void *arg) {
  int thread_id = *((int *)arg);
  int iterazioni_barba;
  int iterazioni_paga;
  while (true) {
    pthread_mutex_lock(&g_gestore.mutex);

    printf("Arrived client[%d]\n", thread_id);

    // acquisisco il divano
    while (g_gestore.num_attivi_divano >= 4) {
      // il divano è pieno, devi aspettare
      pthread_cond_wait(&g_gestore.divano, &g_gestore.mutex);
    }
    printf("Client[%d] acquired divano\n", thread_id);
    g_gestore.num_attivi_divano++;
    pthread_mutex_unlock(&g_gestore.mutex);

    // acquisisco il barbiere
    pthread_mutex_lock(&g_gestore.mutex);
    while (g_gestore.num_attivi_barbiere >= 3) {
      pthread_cond_wait(&g_gestore.barbiere, &g_gestore.mutex);
    }
    // who do i have to signal?
    printf("Client[%d] acquired barber\n", thread_id);
    g_gestore.num_attivi_divano--;
    pthread_cond_signal(&g_gestore.divano);
    g_gestore.num_attivi_barbiere++;
    pthread_mutex_unlock(&g_gestore.mutex);

    // ora taglio la barba per SHAVING_ITERATIONS
    iterazioni_barba = SHAVING_ITERATIONS;
    while (iterazioni_barba > 0) {
      iterazioni_barba--;
    }

    printf("Client[%d] finished to the barber\n", thread_id);
    // finito di tagliare la barba
    // ora mi metto in fila in cassa
    // se non è libera, i block myself
    // acquisisco il mutex
    //
    pthread_mutex_lock(&g_gestore.mutex);
    // acquisisco il cassiere
    while (!g_gestore.cassiere_libero) {
      pthread_cond_wait(&g_gestore.cassiere, &g_gestore.mutex);
    }
    printf("Client[%d] acquired cassiere\n", thread_id);
    // now cassiere_libero is true
    g_gestore.cassiere_libero = false;
    // one less active client
    g_gestore.num_attivi_barbiere--;
    pthread_cond_signal(
        &g_gestore.barbiere); // signal that there is a free barber place
    pthread_mutex_unlock(&g_gestore.mutex);

    // now i pay
    iterazioni_paga = PAYING_ITERATIONS;
    while (iterazioni_paga > 0)
      iterazioni_paga--;

    // then i free the cassiere
    pthread_mutex_lock(&g_gestore.mutex);
    g_gestore.cassiere_libero = true;
    pthread_cond_signal(&g_gestore.cassiere); // dii a chi è in fila dopo di te
                                              // che il cassiere è ibero
    printf("Client[%d] payed, and freed cassiere \n", thread_id);
    pthread_mutex_unlock(&g_gestore.mutex);
    printf("Client[%d] FATTO\n", thread_id);
    fflush_unlocked(stdout);
    sleep(1);
  }
}

int main() {

  pthread_t threads[NUM_CLI];
  int thread_ids[NUM_CLI];
  initgestore(&g_gestore);

  for (int i = 0; i < NUM_CLI; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, cliente, &thread_ids[i]);
  }

  for (int i = 0; i < NUM_CLI; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
