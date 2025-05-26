#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N 5

/*
 *Il sistema è modellato con un thread per ogni corridore, ed un thread per
l'arbitro. Il corridore arriva al punto di partenza ed aspetta il via
dell'arbitro. Quando l'arbitro da il via, il corridore corre ed arriva al
traguardo. L'arbitro arriva ed attende che tutti i corridori si siano
posizionati. Dopo di che da il via alla gara, e si mette in attesa dell'arrivo
dell'ultimo corridore. Non appena l'ultimo corridore arriva, l'arbitro comunica
il primo e l'ultimo classificato della gara.
 * */
struct gestore_t {
  pthread_mutex_t mutex;
  pthread_cond_t attendi_via;
  pthread_cond_t attendi_corridori;
  pthread_cond_t attendi_risultato;
  pthread_cond_t attendi_fine_dei_corridori;
  bool gara_start;
  bool gara_over;

  int corridori_arrivati_al_traguardo;
  int corridori_arrivati_alla_partenza;

  int active_corridori;
  int blocked_corridori;
  int ultimo, primo; // for the result, who is the first and who is the last

} global_gestore;

void init_gestore(struct gestore_t *g) {
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&g->mutex, &m_attr);
  pthread_cond_init(&g->attendi_corridori, &c_attr);
  pthread_cond_init(&g->attendi_risultato, &c_attr);
  pthread_cond_init(&g->attendi_via, &c_attr);
  pthread_cond_init(&g->attendi_fine_dei_corridori, &c_attr);
  g->gara_start = false;
  g->gara_over = false;
  g->corridori_arrivati_al_traguardo = 0;
  g->corridori_arrivati_alla_partenza = 0;
  g->active_corridori = 0;

  g->primo = -1;
  g->ultimo = -1;
  g->blocked_corridori = 0;
  pthread_condattr_destroy(&c_attr);
  pthread_mutexattr_destroy(&m_attr);
}

void arbitro_attendicorridori(struct gestore_t *g) {
  printf("[ARBITRO] started \n");
  pthread_mutex_lock(&g->mutex);

  while (g->corridori_arrivati_alla_partenza != N ||
         g->blocked_corridori != N) {
    // referee blocks

    printf("[ARBITRO] in attesa di corridori alla partenza\n");
    pthread_cond_wait(&g->attendi_corridori, &g->mutex);
  }

  pthread_mutex_unlock(&g->mutex);
}

void arbitro_via(struct gestore_t *g) {
  // print PRONTIII
  printf("[ARBITRO] PRONTI? VIA...\n");
  pthread_mutex_lock(&g->mutex);
  // i have to put a mutex because i'm modifing a variable
  g->gara_start = true;
  pthread_cond_broadcast(&g->attendi_via);
  pthread_mutex_unlock(&g->mutex);
  // broadcast to all waiting corridori
}

void arbitro_risultato(struct gestore_t *g, int *primo, int *ultimo) {
  pthread_mutex_lock(&g->mutex);
  // now i have to block in the condition where i wait for guys to finish
  while (g->active_corridori || g->corridori_arrivati_al_traguardo < N) {
    pthread_cond_wait(&g->attendi_fine_dei_corridori, &g->mutex);
  }
  printf("[ARBITRO] analizzo i risultati\n");

  printf("[ARBITRO] PRIMO : %d\n", *(primo));
  printf("[ARBITRO] ULTIMO : %d\n", *(ultimo));

  g->ultimo = -1;
  g->primo = -1;
  g->active_corridori = 0;
  g->gara_over = true;
  g->gara_start = false;
  g->corridori_arrivati_al_traguardo = 0;
  g->corridori_arrivati_alla_partenza = 0;
  g->blocked_corridori = 0;
  pthread_mutex_unlock(&g->mutex);
}

void corridore_attendi_via(struct gestore_t *g, int numerocorridore) {
  // record that a corridere has arrived
  printf("Corridore[%d] has arrived at the pista\n", numerocorridore);

  // acquire the mutex
  pthread_mutex_lock(&g->mutex);
  g->corridori_arrivati_alla_partenza++;
  g->blocked_corridori++;

  // who do i have to wake up before blocking?
  if (g->corridori_arrivati_alla_partenza == N)
    pthread_cond_signal(&g->attendi_corridori);

  while (!g->gara_start) {
    printf("Corridore[%d] is now waiting for the referee to start\n",
           numerocorridore);

    printf("Corridore[%d] printing blocked_corridori = %d\n", numerocorridore,
           g->blocked_corridori);
    pthread_cond_wait(&g->attendi_via, &g->mutex);
  }
  pthread_mutex_unlock(&g->mutex);
}

void corridore_arrivo(struct gestore_t *g, int numerocorridore) {
  // first starts going

  usleep((rand() % 1000000) + 500000); // 0.5-1.5 seconds
  printf("Corridore[%d] RUNNING\n", numerocorridore);
  pthread_mutex_lock(&g->mutex);
  printf("Corridore[%d] HAS FINISHED\n", numerocorridore);
  g->corridori_arrivati_al_traguardo++;
  if (g->corridori_arrivati_al_traguardo == 1) {
    g->primo = numerocorridore;
  } else if (g->corridori_arrivati_al_traguardo == 5) {
    g->ultimo = numerocorridore;
  }

  printf("Corridore[%d] printing corridori_arrivati_al_traguardo = %d\n",
         numerocorridore, g->corridori_arrivati_al_traguardo);
  // before blocking, who do i have to wake up?
  if (g->corridori_arrivati_al_traguardo == N) {
    pthread_cond_signal(&g->attendi_fine_dei_corridori);
  }
  // now i have to block myself
  pthread_mutex_unlock(&g->mutex);
}

void *arbitro() {

  while (1) {
    arbitro_attendicorridori(&global_gestore);
    arbitro_via(&global_gestore);
    arbitro_risultato(&global_gestore, &global_gestore.primo,
                      &global_gestore.ultimo);

    printf("============");
    sleep(2);
  }

  pthread_exit(NULL);
}
void *corridore(void *arg) {
  int thread_id = *((int *)arg);
  while (1) {
    corridore_attendi_via(&global_gestore, thread_id);
    corridore_arrivo(&global_gestore, thread_id);
    sleep(1);
  }
  pthread_exit(NULL);
}
int main(int argc, char *argv[]) {
  pthread_t corridori[N];
  int corridori_ids[N];

  pthread_attr_t a;
  pthread_attr_init(&a);
  pthread_t arb;
  if (pthread_create(&arb, &a, arbitro, NULL) != 0) {
    printf("Error creating referee\n");
  }
  for (int i = 0; i < N; i++) {
    corridori_ids[i] = i;
    pthread_create(&corridori[i], NULL, corridore, &corridori_ids[i]);
  }

  for (int i = 0; i < N; i++) {
    pthread_join(corridori[i], NULL);
  }
  pthread_join(arb, NULL);
  return EXIT_SUCCESS;
}
