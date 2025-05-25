#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#define printf(...)                                                            \
  printf(__VA_ARGS__);                                                         \
  fflush(stdout)
#define thread_idx (abs((int)pthread_self()) % 100)

#define M 4 // numero di Mittenti
#define R 4 // numero di Riceventi
#define N 4 // numero di buste

typedef int T;
struct busta_t {
  T messaggio;
  struct busta_t *next;
};

struct mailbox_t {
  sem_t mutex;

  int num_buste_vuote;
  int num_bloccati_buste[3]; // numero di processi in attesa di byuste vuote
  sem_t s_buste_vuote[3]; // 3 semafori in base alla priortà per i processi in
                          // attesa di buste vuote
  sem_t s_buste_accodate; // semafori per processi riceventi di avere buste da
                          // estrrare
  struct busta_t *head;
  struct busta_t *tail;
} mailbox;

void accoda_busta(struct mailbox_t *m, struct busta_t *busta) {
  if (m->head == NULL || mailbox.tail == NULL) {
    // inserisci in testa
    m->head = busta;
    m->tail = busta;
  } else if (mailbox.head == mailbox.tail) {
    // inserisci in testa
    m->head->next = busta;
    m->tail = busta;
  } else {
    m->tail->next = busta;
    m->tail = busta;
  }
}

struct busta_t *disaccoda_busta(struct mailbox_t *m) {
  if (m->head == NULL)
    return NULL;
  struct busta_t *result = m->head; // dequeue
  m->head = m->head->next;
  return result;
}

void init_mailbox(struct mailbox_t *m) {
  sem_init(&m->mutex, 0, 1);

  for (int priorita = 0; priorita < 3; priorita++) {
    sem_init(&m->s_buste_vuote[priorita], 0, 0);
    m->num_bloccati_buste[priorita] = 0;
  }

  sem_init(&m->s_buste_accodate, 0, 0);
  m->num_buste_vuote = N;
  m->head = NULL;
  m->tail = NULL;
}

void richiedi_busta_vuota(int priorita) {
  sem_wait(&mailbox.mutex);
  if (mailbox.num_buste_vuote > 0) {
    sem_post(&mailbox.s_buste_vuote[priorita]); // preemptive post su quel
                                                // semaforo con quella priorità
    mailbox.num_buste_vuote--;
  } else {
    mailbox.num_bloccati_buste[priorita]++;
  }

  sem_post(&mailbox.mutex);
  sem_wait(&mailbox.s_buste_vuote[priorita]);
}

void riponi_busta_vuota() {
  sem_wait(&mailbox.mutex);

  mailbox.num_buste_vuote++;
  for (int i = 0; i < 3; i++) {
    // wake up in priority order all of the processes waiting for
    // richiedi_busta_vuota
    if (mailbox.num_bloccati_buste[i] > 0) {
      // if there is a blocked process with that priority
      sem_post(&mailbox.s_buste_vuote[i]);
      mailbox.num_bloccati_buste[i]--;
      break;
    }
  }
  sem_post(&mailbox.mutex);
}

void send(T messaggio, int priorita) {
  // wait on the mutex
  // i have to send a message, so first i need to create the message
  // the send it to the mailbox

  // 1. Are there any buste_vuote??
  richiedi_busta_vuota(priorita);
  // 2. Now i create the messagge
  sem_wait(&mailbox.mutex);
  struct busta_t *busta = (struct busta_t *)malloc(sizeof(struct busta_t));
  busta->messaggio = messaggio;
  busta->next = NULL;
  accoda_busta(&mailbox, busta);

  sem_post(&mailbox.s_buste_accodate); // dii a chi sta aspettando messaggi che
                                       // c'è un messaggio
  sem_post(&mailbox.mutex);
}

T receive() {
  sem_wait(&mailbox.s_buste_accodate);
  // now i have a busta accodata, mi hanno svegliato
  //
  // La disaccoda_busta la fai in mutex
  sem_wait(&mailbox.mutex);
  struct busta_t *busta = disaccoda_busta(&mailbox);
  if (busta == NULL) {
    printf("illegal state!\n");
    exit(1);
  }
  sem_post(&mailbox.mutex);
  riponi_busta_vuota();
  T messaggio = busta->messaggio;

  free(busta);
  return messaggio;
}

// ------------------------------------------------------------------------------------------------
// Threads
// ------------------------------------------------------------------------------------------------

void *ricevente(void *arg) {
  while (true) {
    // printf("%d recv> Aspetto un messaggio...\n", thread_idx);

    T messaggio = receive();

    printf("%d recv> Ho ricevuto il messaggio \"%d\"\n", thread_idx, messaggio);

    sleep(rand() % 6);
  }
}

void *mittente(void *arg) {
  while (true) {
    int messaggio = rand() % 100;
    int priorita = rand() % 3;

    // printf("%d send> Invio messaggio %d\n", thread_idx, messaggio);

    send(messaggio, priorita);

    printf("%d send> Messaggio \"%d\" inviato con priorita' \"%d\"!\n",
           thread_idx, messaggio, priorita);

    sleep(rand() % 2);
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  pthread_t t_mittenti[M];
  pthread_t t_riceventi[R];

  init_mailbox(&mailbox);

  for (int m = 0; m < M; m++)
    pthread_create(&t_mittenti[m], NULL, mittente, NULL);

  for (int r = 0; r < R; r++)
    pthread_create(&t_riceventi[r], NULL, ricevente, NULL);

  sleep(60);

  return 0;
}
