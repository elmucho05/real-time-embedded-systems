#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Let A, B, C, D, and E be the procedures that a set of processes P1, P2, ..., PN can invoke,
 * and which must be executed respecting the following synchronization constraints:
 * 1. there are only two possible sequences of procedure executions,
 *    which are mutually exclusive:
 *    - the first sequence requires that procedure A is executed first,
 *      followed exclusively by one or more concurrent activations of procedure B
 *    - the second sequence consists of the execution of procedure C,
 *      followed exclusively by the execution of procedure D or E
 * 2. once one of the two sequences is completed, a new sequence can be started again.
 *
 * Allowed sequences:
 *  IDLE  -> A|B
 *  A     -> B
 *  B     -> B|IDLE
 *  C     -> D|E
 *  D     -> IDLE
 *  E     -> IDLE
 */

 // enum current_state
 // {
 //   STATE_IDLE,
 //   STATE_A,
 //   STATE_B,
 //   STATE_C,
 //   STATE_D,
 //   STATE_E
 // } current_state;
 // enum current_state g_current_state = STATE_IDLE;

sem_t g_sem_IDLE;
sem_t g_sem_A;
sem_t g_sem_B;
sem_t g_sem_C;
sem_t g_sem_D;
sem_t g_sem_E;

void sleep_random()
{
  struct timespec t;
  t.tv_sec = 0;
  t.tv_nsec = (rand() % 100 + 1) * 1000000;
  nanosleep(&t, NULL);
}

char get_letter(char c)
{
  int r = rand();
  if (r % 2 == 0)
    return (char)toupper(c);
  return (char)tolower(c);
}

void* proc_IDLE(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_IDLE);

    int r = rand();
    // Move to state A: unlock A's semaphore 
    if (r % 2 == 0)
    {
      sem_post(&g_sem_A);
    }
    // Move to state C: unlock C's semaphore
    else
    {
      sem_post(&g_sem_C);
    }
  }
}

void* proc_A(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_A);

    char c = get_letter('A');
    printf("%c", c);
    sleep_random();
  
    // Move to state B: unlock B's semaphore
    sem_post(&g_sem_B);
  }
}

void* proc_B(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_B);

    char c;
    int r;
    do {
      c = get_letter('B');
      printf("%c", c);
      sleep_random();

      r = rand();
    } while (r % 2 == 0); // Keep on state B

    // Move to state IDLE: unlock IDLE's semaphore
    sem_post(&g_sem_IDLE);
  }
}

void* proc_C(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_C);

    char c = get_letter('C');
    printf("%c", c);
    sleep_random();

    // Move to state D: unlock D's semaphore
    int r = rand();
    if (r % 2 == 0)
    {
      sem_post(&g_sem_B);
    }
    // Or move to state E
    else
    {
      sem_post(&g_sem_E);
    }
  }
}

void* proc_D(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_D);

    char c = get_letter('D');
    printf("%c", c);
    sleep_random();

    // Move to state IDLE: unlock IDLE's semaphore
    sem_post(&g_sem_IDLE);
  }
}

void* proc_E(void* arg)
{
  while (true)
  {
    sem_wait(&g_sem_E);

    char c = get_letter('E');
    printf("%c", c);
    sleep_random();

    // Move to state IDLE: unlock IDLE's semaphore
    sem_post(&g_sem_IDLE);
  }
}


int main()
{
  srand(time(NULL));

  sem_init(&g_sem_IDLE, 0, 1);  // Inizialmente disponibile
  sem_init(&g_sem_A, 0, 0);     // Inizialmente NON disponibile
  sem_init(&g_sem_B, 0, 0);     // Inizialmente NON disponibile
  sem_init(&g_sem_C, 0, 0);     // Inizialmente NON disponibile
  sem_init(&g_sem_D, 0, 0);     // Inizialmente NON disponibile
  sem_init(&g_sem_E, 0, 0);     // Inizialmente NON disponibile

  pthread_t threads[6];
  pthread_create(&threads[0], NULL, proc_IDLE, NULL);
  pthread_create(&threads[1], NULL, proc_A, NULL);
  pthread_create(&threads[2], NULL, proc_B, NULL);
  pthread_create(&threads[3], NULL, proc_C, NULL);
  pthread_create(&threads[4], NULL, proc_D, NULL);
  pthread_create(&threads[5], NULL, proc_E, NULL);
  for (int i = 0; i < 6; i++)
    pthread_detach(threads[i]);

  sleep(3);

  sem_destroy(&g_sem_IDLE);
  sem_destroy(&g_sem_A);
  sem_destroy(&g_sem_B);
  sem_destroy(&g_sem_C);
  sem_destroy(&g_sem_D);
  sem_destroy(&g_sem_E);

  return 0;
}