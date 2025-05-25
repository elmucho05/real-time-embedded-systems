#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define my_printf(x, ...)                                                      \
  printf(x, ##__VA_ARGS__);                                                    \
  fflush(stdout)
#define do_something() usleep(rand() % 1000 + 1000)
/*
 * ProcA e B lavorano sulla stessa variabile ma non c'è bisongo di escluderle
 * perché lavorano su aree diverso
 *
 * */

struct handler_t {
  sem_t mutex;
  sem_t sa, sb, sr;
  int ca, // number of blocked A
      cb, // number of blocked B
      cr; // number of blocked R

  int na, // number of running A
      nb, // number of running B
      nr; // number of running R
  //
} g_handler;

void handler_init(struct handler_t *handler) {
  // mutex
  sem_init(&handler->mutex, 0, 1);

  // private semaphores
  sem_init(&handler->sa, 0, 0);
  sem_init(&handler->sb, 0, 0);
  sem_init(&handler->sr, 0, 0);

  // handler vars
  handler->ca = 0;
  handler->cb = 0;
  handler->cr = 0;

  handler->na = 0;
  handler->nb = 0;
  handler->nr = 0;
}

void StartProcA() {
  sem_wait(&g_handler.mutex);
  // When can ProcA run?
  // if there are no active Resets of other A threads running
  // A processes have to be mutually exclusive
  if (g_handler.nr == 0 && g_handler.na == 0) {
    sem_post(&g_handler.sa); // preemptive post
    g_handler.na++;
  } else {
    // A should block but first update the var and release the mutex
    g_handler.ca++;
  }

  sem_post(&g_handler.mutex);
  sem_wait(&g_handler.sa);
}
void EndProcA() {
  sem_wait(&g_handler.mutex);
  // ProcA is no more running, i decrement the counter of running instances
  g_handler.na--;

  // does A have to wake up someone?
  // if i'm the last one
  if (g_handler.na == 0) {
    if (g_handler.nb == 0 && g_handler.cb > 0) {
      // Wakes up any blocked resets _only_if_ there's no B running
      g_handler.cr--;
      g_handler.nr++;
      sem_post(&g_handler.sr);
    } else if (g_handler.ca > 0) {
      // otherwise wake up another A, since A and B operate in the
      // same var, cannot wake each other22k
      g_handler.ca--;
      g_handler.na++;
      sem_post(&g_handler.sa);
    }
  }

  sem_post(&g_handler.mutex);
}

void StartProcB() {
  sem_wait(&g_handler.mutex);
  if (g_handler.nr == 0 && g_handler.nb == 0) {
    // if there are no active Resets or other B processes, then i run
    sem_post(&g_handler.sb); // preemptive post
    g_handler.nb++;
  } else {
    g_handler.cb++;
  }

  sem_post(&g_handler.mutex);
  sem_wait(&g_handler.sb);
}
void EndProcB() {
  sem_wait(&g_handler.mutex);
  // now b is no longer running
  g_handler.nb--;
  if (g_handler.nb == 0) {
    // if there are no more running A, and there ARE blocked R, activate an R
    if (g_handler.na == 0 && g_handler.cr > 0) {
      g_handler.cr--;
      g_handler.nr++;
      sem_post(&g_handler.sr);
    } else if (g_handler.cb > 0) {
      // unlock another blocked b
      g_handler.cb--;
      g_handler.nb++;
      sem_post(&g_handler.sb);
    }
  }

  sem_post(&g_handler.mutex);
}

void StartReset() {
  sem_wait(&g_handler.mutex);
  // when can i run?
  if (g_handler.na == 0 && g_handler.nb == 0 && g_handler.nr == 0) {
    sem_post(&g_handler.sr); // preemptive post
    g_handler.nr++;
  } else {
    g_handler.cr++; // record that reset has been suspended
  }

  sem_post(&g_handler.mutex);
  sem_wait(&g_handler.sr);
}

void EndReset() {
  sem_wait(&g_handler.mutex);
  g_handler.nr--;
  if (g_handler.nr == 0) {
    if (g_handler.cr > 0) { // if i'm the last one, give priority to blocked R
      g_handler.cr--;
      g_handler.nr++;
      sem_post(&g_handler.sr);
    } else {
      if (g_handler.ca > 0) {
        g_handler.ca--;
        g_handler.na++;
        sem_post(&g_handler.sa);
      }
      if (g_handler.cb > 0) {
        g_handler.cb--;
        g_handler.nb++;
        sem_post(&g_handler.sb);
      }
    }
  }
  sem_post(&g_handler.mutex);
}
void ProcA(int thread_idx) {
  StartProcA();

  my_printf("<%dA", thread_idx);
  do_something();
  my_printf("A%d>", thread_idx);

  EndProcA();
}

void ProcB(int thread_idx) {
  StartProcB();

  my_printf("<%dB", thread_idx);
  do_something();
  my_printf("B%d>", thread_idx);

  EndProcB();
}

void Reset(int thread_idx) {
  StartReset();

  my_printf("<%dR", thread_idx);
  do_something();
  my_printf("R%d>", thread_idx);

  EndReset();
}

void *thread_body(void *arg) {
  int thread_idx = *((int *)arg);
  while (1) {
    int r = rand();

    // my_printf("|%d->%s|", thread_idx, (r%3 ==0  ? "A" : (r%3==1 ? "B" :
    // "R")));

    if (r % 3 == 0)
      ProcA(thread_idx);
    else if (r % 3 == 1)
      ProcB(thread_idx);
    else if (r % 3 == 2)
      Reset(thread_idx);
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  const int k_num_threads = 10;
  pthread_t my_threads[k_num_threads];

  handler_init(&g_handler);

  // stores threads id in an array for debugging
  int thread_ids[k_num_threads];
  for (int i = 0; i < k_num_threads; i++)
    thread_ids[i] = i;

  // creates and starts threads
  for (int i = 0; i < k_num_threads; i++) {
    if (pthread_create(&my_threads[i], NULL, thread_body,
                       (void *)&thread_ids[i]) != 0) {
      perror("pthread_create() error\n");
      return 1;
    }
  }

  sleep(60);

  return 0;
}
