#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define THREADS 5

typedef struct {
  int num_votanti;
  int quorum;
  pthread_mutex_t vote_mutex;
  pthread_cond_t waiting_for_quorum;
  int tot_voti_pro, tot_voti_contro;
} urna_t;

urna_t global_urn;

void urn_init(urna_t *urn) {
  pthread_mutexattr_t m_attr;
  pthread_condattr_t c_attr;

  urn->num_votanti = THREADS;
  urn->tot_voti_pro = 0;
  urn->tot_voti_contro = 0;

  double half = ((double)urn->num_votanti) / 2;
  urn->quorum = (int)ceil(half); // intero superiore

  if (urn->num_votanti % 2 == 0) {
    printf("Ci deve essere un numero DISPARI DI VOTANTI");
    exit(-1);
  }

  if (pthread_mutexattr_init(&m_attr) < 0) {
    perror("pthread_mutexattr_init: ");
    exit(-1);
  }
  if (pthread_condattr_init(&c_attr) < 0) {
    perror("pthread_condattr_init: ");
    exit(-1);
  }
  if (pthread_mutex_init(&urn->vote_mutex, &m_attr) < 0) {
    perror("pthread_mutex_init: ");
    exit(-1);
  }
  if (pthread_cond_init(&urn->waiting_for_quorum, &c_attr) < 0) {
    perror("pthread_cond_mutex_init: ");
    exit(-1);
  }
}

void vota(int arg) {
  pthread_mutex_lock(&global_urn.vote_mutex);

  if (arg == 0)
    global_urn.tot_voti_pro++;
  if (arg == 1)
    global_urn.tot_voti_contro++;
  pthread_mutex_unlock(&global_urn.vote_mutex);
  // segnalo a tutti i vtanti aspettando il quorum che io ho votato
  pthread_cond_broadcast(&global_urn.waiting_for_quorum);
}

int risultato() {
  pthread_mutex_lock(&global_urn.vote_mutex);
  int vince_pro = global_urn.tot_voti_pro >= global_urn.quorum;
  int vince_contro = global_urn.tot_voti_contro >= global_urn.quorum;
  int result = -1;

  // ATTENZIONE LAVORO CON COND_VAR, devo fare il while
  while (vince_pro == 0 && vince_contro == 0) {
    // se non ho ancora un vincitore
    pthread_cond_wait(&global_urn.waiting_for_quorum, &global_urn.vote_mutex);
    // check the conditions again, you have tyo say who who
    vince_pro = global_urn.tot_voti_pro >= global_urn.quorum;
    vince_contro = global_urn.tot_voti_contro >= global_urn.quorum;
  }

  if (vince_pro > 0)
    result = vince_contro;
  else
    result = vince_contro;
  pthread_mutex_unlock(&global_urn.vote_mutex);
  return result;
}

void *thread() {
  int thread_id = pthread_self();
  int voto = rand() % 2;
  printf("\nSto andando a votare %d !\n", thread_id);
  vota(voto);
  printf("Thread %d ha votato %d \n", thread_id, voto);
  if (voto == risultato()) {
    printf("Ho vinto %d!\n", thread_id);
  } else
    printf("Ho perso %d!\n", thread_id);
  pthread_exit(0);
}
int main() {
  srand(time(NULL));
  pthread_attr_t a;
  pthread_t threads[THREADS];

  pthread_attr_init(&a);

  urn_init(&global_urn);

  for (int i = 0; i < THREADS; i++) {
    pthread_create(&threads[i], &a, thread, NULL);
  }

  for (int i = 0; i < THREADS; i++) {
    int *retval;
    pthread_join(threads[i], (void **)&retval);
  }
  return 0;
}
