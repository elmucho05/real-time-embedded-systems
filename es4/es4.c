#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define CARTA 0
#define SASS0 1
#define FOBICE 2

char *nomi_mosse[3] = {"carta", "sasso", "forbice"};

struct gestore_t {
  pthread_mutex_t mutex;
  pthread_cond_t waiting_game_start;
  pthread_cond_t waiting_for_all_moves;
  pthread_cond_t waiting_for_game_end;

  int player1_move;
  int player2_move;

  bool game_start;
  bool game_end;

  int num_moves;
} gestore;
void init_gestore(struct gestore_t *g) {
  pthread_condattr_t c_attr;
  pthread_mutexattr_t m_attr;

  pthread_mutexattr_init(&m_attr);
  pthread_condattr_init(&c_attr);

  pthread_mutex_init(&g->mutex, &m_attr);
  pthread_cond_init(&g->waiting_game_start, &c_attr);
  pthread_cond_init(&g->waiting_for_all_moves, &c_attr);
  pthread_cond_init(&g->waiting_for_game_end, &c_attr);

  pthread_mutexattr_destroy(&m_attr);
  pthread_condattr_destroy(&c_attr);
  g->game_end = false;
  g->game_start = false;

  g->player1_move = -1;
  g->player2_move = -1;

  g->num_moves = 0;
}

void *referee() {
  while (true) {

    pthread_mutex_lock(&gestore.mutex);
    // lock the mutex
    printf("[REFEREE] starting the game\n");
    gestore.game_start = true;
    gestore.game_end = false;
    pthread_cond_broadcast(&gestore.waiting_game_start);

    // now i have started the game
    // wait untill both moves have been made
    while (gestore.num_moves != 2) {
      pthread_cond_wait(&gestore.waiting_for_all_moves, &gestore.mutex);
    }

    // all moves have been made, analize them
    int move1 = gestore.player1_move;
    int move2 = gestore.player2_move;

    printf("[REFEREE] analyzing moves : %s VS %s\n", nomi_mosse[move1],
           nomi_mosse[move2]);

    printf("[REFEREE] ending game\n");
    gestore.game_start = false;
    gestore.game_end = true;
    gestore.player1_move = -1;
    gestore.player2_move = -1;
    gestore.num_moves = 0;
    pthread_cond_broadcast(&gestore.waiting_for_game_end);
    pthread_mutex_unlock(&gestore.mutex);

    printf("======================\n");
    sleep(1);
  }
  return NULL;
}
void *player(void *arg) {
  int thread_id = *((int *)arg);
  while (true) {
    pthread_mutex_lock(&gestore.mutex);
    printf("Player[%d] arrived and waiting for game_start\n", thread_id);
    while (!gestore.game_start) {
      pthread_cond_wait(&gestore.waiting_game_start, &gestore.mutex);
    }

    // game started, i have to make a move
    int rand_move = rand() % 2;
    if (thread_id == 1) {
      gestore.player1_move = rand_move;
      printf("Player[1] has chosen %s \n", nomi_mosse[rand_move]);
    } else if (thread_id == 2) {
      printf("Player[2] has chosen %s \n", nomi_mosse[rand_move]);
      gestore.player2_move = rand_move;
    }

    // register one more move
    gestore.num_moves++;
    // who do i need to signal before blocking?
    // referee is waiting for my move
    pthread_cond_signal(&gestore.waiting_for_all_moves);
    // now i have made my move, i will wait fo game to end
    while (!gestore.game_end) {
      printf("Player[%d] is now waiting for game to end\n", thread_id);
      pthread_cond_wait(&gestore.waiting_for_game_end, &gestore.mutex);
    }

    // game end received
    pthread_mutex_unlock(&gestore.mutex);
  }
  return NULL;
}

int main() {
  srand(time(NULL));
  init_gestore(&gestore);
  pthread_t referee_t, p1_t, p2_t;
  int p1_id = 1;
  int p2_id = 2;
  pthread_create(&referee_t, NULL, referee, NULL);
  pthread_create(&p1_t, NULL, player, &p1_id);
  pthread_create(&p2_t, NULL, player, &p2_id);
  pthread_join(referee_t, NULL);
  pthread_join(p1_t, NULL);
  pthread_join(p2_t, NULL);

  return 0;
}
