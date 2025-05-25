#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <memory.h>

#define PLAYER_MOVE_NONE -1
#define PLAYER_MOVE_ROCK 0
#define PLAYER_MOVE_PAPER 1
#define PLAYER_MOVE_SCISSORS 2
static const char* PLAYER_MOVE_NAMES[] = { "ROCK", "PAPER", "SCISSORS" };

sem_t g_sem_players_are_ready;

pthread_mutex_t g_mutex;
pthread_cond_t g_cv_players_are_done;

sem_t g_sem_players_are_done;
sem_t g_sem_game_start;
sem_t g_sem_game_over;

bool g_player1_ready = false;
bool g_player2_ready = false;

int g_player_1_move = PLAYER_MOVE_NONE;
int g_player_2_move = PLAYER_MOVE_NONE;

void* proc_player(void* arg)
{
  int player_id = (*(int*)arg);

  while (true)
  {
    // 1. Signal the referee i'm ready
    printf("[player_%d] ready to play!\n", player_id);
    if (player_id == 1)
      g_player1_ready = true;
    else if (player_id == 2)
      g_player2_ready = true;

    if (g_player1_ready && g_player2_ready)
      sem_post(&g_sem_players_are_ready);

    // 2. Waiting for the referee to start
    printf("[player_%d] Waiting for the referee to start...\n", player_id);
    sem_wait(&g_sem_game_start);

    // 3. ROCK-PAPER-SCISSORS
    int random = rand() % 3;

    pthread_mutex_lock(&g_mutex);
    if (player_id == 1)
    {
      g_player_1_move = random;
      printf("[player_1] %s!\n", PLAYER_MOVE_NAMES[g_player_1_move]);
    }
    else if (player_id == 2)
    {
      g_player_2_move = random;
      printf("[player_2] %s!\n", PLAYER_MOVE_NAMES[g_player_2_move]);
    }
    pthread_cond_signal(&g_cv_players_are_done);
    pthread_mutex_unlock(&g_mutex);


    // 4. Waiting for the referee to terminate the game
    printf("[player_%d] Waiting for the referee to terminate the game...\n", player_id);
    sem_wait(&g_sem_game_over);
  }
}

void* proc_referee(void* arg)
{
  while (true)
  {
    // 1. Waiting all players are ready
    printf("[REFEREE] Waiting all players are ready...\n");
    sem_wait(&g_sem_players_are_ready);

    // 2. Start the game
    printf("[REFEREE] All players are ready! The game is starting soon...\n");
    sleep(1);
    sem_post(&g_sem_game_start);
    sem_post(&g_sem_game_start);

    // 3. Waiting all players have made their move and terminate the game
    pthread_mutex_lock(&g_mutex);
    while (g_player_1_move == PLAYER_MOVE_NONE || g_player_2_move == PLAYER_MOVE_NONE)
    {
      printf("[REFEREE] Waiting all players have made their move...\n");
      pthread_cond_wait(&g_cv_players_are_done, &g_mutex);
    }
    pthread_mutex_unlock(&g_mutex);

    printf("[REFEREE] All players are done! %s vs %s: ",
      PLAYER_MOVE_NAMES[g_player_1_move],
      PLAYER_MOVE_NAMES[g_player_2_move]
    );
    if (g_player_1_move == g_player_2_move)
      printf("DRAW!\n");
    else if (g_player_1_move == PLAYER_MOVE_ROCK && g_player_2_move == PLAYER_MOVE_PAPER)
      printf("player 2 wins!\n");
    else if (g_player_1_move == PLAYER_MOVE_ROCK && g_player_2_move == PLAYER_MOVE_SCISSORS)
      printf("player 1 wins!\n");
    else if (g_player_1_move == PLAYER_MOVE_PAPER && g_player_2_move == PLAYER_MOVE_ROCK)
      printf("player 1 wins!\n");
    else if (g_player_1_move == PLAYER_MOVE_PAPER && g_player_2_move == PLAYER_MOVE_SCISSORS)
      printf("player 2 wins!\n");
    else if (g_player_1_move == PLAYER_MOVE_SCISSORS && g_player_2_move == PLAYER_MOVE_ROCK)
      printf("player 2 wins!\n");
    else if (g_player_1_move == PLAYER_MOVE_SCISSORS && g_player_2_move == PLAYER_MOVE_PAPER)
      printf("player 1 wins!\n");


    printf("[REFEREE] Terminate the game.\n");
    printf("\n ========================= \n");
    sleep(1);

    g_player1_ready = false;
    g_player2_ready = false;
    g_player_1_move = PLAYER_MOVE_NONE;
    g_player_2_move = PLAYER_MOVE_NONE;

    sem_post(&g_sem_game_over);
    sem_post(&g_sem_game_over);
  }
}

int main()
{
  srand(time(NULL));

  pthread_mutex_init(&g_mutex, NULL);
  pthread_cond_init(&g_cv_players_are_done, NULL);

  sem_init(&g_sem_players_are_ready, 0, 0);
  sem_init(&g_sem_players_are_done, 0, 0);
  sem_init(&g_sem_game_start, 0, 0);
  sem_init(&g_sem_game_over, 0, 0);

  int player_1_id = 1;
  int player_2_id = 2;
  pthread_t player_1;
  pthread_t player_2;
  pthread_t referee;
  pthread_create(&player_1, NULL, proc_player, &player_1_id);
  pthread_create(&player_2, NULL, proc_player, &player_2_id);
  pthread_create(&referee, NULL, proc_referee, NULL);
  pthread_join(player_1, NULL);
  pthread_join(player_2, NULL);
  pthread_join(referee, NULL);

  pthread_mutex_destroy(&g_mutex);
  pthread_cond_destroy(&g_cv_players_are_done);

  sem_destroy(&g_sem_players_are_ready);
  sem_destroy(&g_sem_players_are_done);
  sem_destroy(&g_sem_game_start);
  sem_destroy(&g_sem_game_over);

  return 0;
}