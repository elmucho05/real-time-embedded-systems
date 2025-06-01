#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N 5

struct gestore_t {
  sem_t mutex;
  sem_t waiting_for_all_players;
  sem_t waiting_for_referee_start;
  sem_t waiting_for_result;
  sem_t waiting_for_player_to_finish;
  int num_arrivati_partenza;
  int num_arrivati_finish;

  bool game_start;
  bool game_end;

  int primo, ultimo;

} global_gestore;

void init_gestore(struct gestore_t *g) {
  sem_init(&g->mutex, 0, 1);
  sem_init(&g->waiting_for_all_players, 0, 0);
  sem_init(&g->waiting_for_referee_start, 0, 0);
  sem_init(&g->waiting_for_player_to_finish, 0, 0);
  sem_init(&g->waiting_for_result, 0, 0);

  g->num_arrivati_partenza = 0;
  g->num_arrivati_finish = 0;

  g->game_end = false;
  g->game_start = false;

  g->primo = -1;
  g->ultimo = -1;
}
void arbitro_attendicorridori(struct gestore_t *g) {

  // now aribtro starts
  printf("[ARBITRO] started \n");
  sem_wait(&g->mutex);

  // when do i have to stop?
  // when the number of players at the start is 0
  if (g->num_arrivati_partenza == 0) {
    sem_post(&g->mutex);
    sem_wait(&g->waiting_for_all_players);
  }

  sem_post(&g->mutex);

  // now someone has signaled me
}
void arbitro_via(struct gestore_t *g) {}

void arbitro_risultato(struct gestore_t *g, int *primo, int *secondo) {}

void corridore_attendi_via(struct gestore_t *g, int numercorridore) {}

void corridore_arrivo(struct gestore_t *g, int numercorridore) {}
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
  init_gestore(&global_gestore);
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
