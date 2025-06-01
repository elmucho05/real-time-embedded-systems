#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
 * Una pasticceria produce e vende al dettaglio delle torte. La pasticceria è
gestita da un cuoco, che cucina le torte con arte, e da un commesso, che prende
le torte prodotte, le incarta e le vende al dettaglio.

Il pasticcere evita di
iniziare a produrre torte se in pasticceria ci sono piu' di N torte invendute. I
clienti acquistano una torta alla volta. La vendita di una torta da parte del
commesso coincide con l'acquisto da parte del cliente.

Il sistema è modellato
tramite un thread per il cuoco, uno per il commesso, ed uno per ogni cliente. Il
numero di clienti non è specificato, e non è importante il loro ordine di
accodamento. */
#define MAX_CLIENTI 10
#define MAX_TORTE 5
struct pasticceria {
  // selling
  sem_t mutex;
  sem_t torte_disp;     // where commesso will block if there are no more cakes
  sem_t clienti_attesa; // whre commesso will block if there are no lcients
  sem_t spazio_disponibile; // whre cuoco blocks if there are too many cakes
  sem_t torta_acquistata;   // clients wait for cake sold confirmation
  int torte_disponibili;
  int active_commessi;
  int clients_waiting;

} g_pasticceria;

void init_pasticceria(struct pasticceria *p) {
  sem_init(&p->mutex, 0, 1);
  sem_init(&p->torte_disp, 0, 0);
  sem_init(&p->clienti_attesa, 0, 0);

  sem_init(&p->spazio_disponibile, 0, MAX_TORTE);
  sem_init(&p->torta_acquistata, 0, 0);
  p->torte_disponibili = 0;
  p->clients_waiting = 0;
}

void cuoco_inizio_torta(struct pasticceria *p) {
  sem_wait(&p->spazio_disponibile); // decremente number of cakes
}

void cuoco_fine_torta(struct pasticceria *p) {
  sem_post(&p->torte_disp); // incremente number of cakes
}

void commesso_prendo_torta(struct pasticceria *p) {
  sem_wait(&p->clienti_attesa); // wait for clients
  sem_wait(&p->torte_disp);     // decrement number of cakes
}

void commesso_vendo_torta(struct pasticceria *p) {
  sem_post(&p->torta_acquistata); // increment torta acquistata, so say that now
                                  // you can buy
  sem_post(&p->spazio_disponibile); // increment the avaiable space
}

void cliente_acquisto(struct pasticceria *p, int thread_id) {
  sem_post(&p->clienti_attesa); // di al commesso che ci sono, increment clienti
                                // che possono stare in attesa
  sem_wait(&p->torta_acquistata); // e aspetta di acquistare la torta, torta
                                  // acquistata, so now you have bought, set
                                  // that to 0
}

void *cliente(void *arg) {
  int thread_id = *((int *)arg);
  while (1) {
    printf("cliente> Acquisto una torta...\n");

    cliente_acquisto(&g_pasticceria, thread_id);

    printf("cliente> Torta acquistata\n");

    sleep(rand() % 2);
  }
}
void *commesso(void *arg) {
  while (1) {
    commesso_prendo_torta(&g_pasticceria);

    printf("commesso> Vendo la torta...\n");

    commesso_vendo_torta(&g_pasticceria);

    sleep(rand() % 2);
  }

  return NULL;
}

void *chef(void *arg) {

  while (1) {
    cuoco_inizio_torta(&g_pasticceria);

    int torte_da_preparare;
    sem_getvalue(&g_pasticceria.spazio_disponibile,
                 &torte_da_preparare); // May not be thread-safe
    printf("cuoco> Torte rimanenti %d \n", torte_da_preparare);

    cuoco_fine_torta(&g_pasticceria);

    sleep(rand() % 2);
  }
}
int main(int argc, char *argv[]) {
  pthread_t comm, cuoco;
  pthread_t clienti[MAX_CLIENTI];
  int client_ids[MAX_CLIENTI];

  init_pasticceria(&g_pasticceria);
  pthread_create(&cuoco, NULL, chef, NULL);
  pthread_create(&comm, NULL, commesso, NULL);

  for (int i = 0; i < MAX_CLIENTI; i++) {
    client_ids[i] = i;
    pthread_create(&clienti[i], NULL, cliente, &client_ids[i]);
  }

  for (int i = 0; i < MAX_CLIENTI; i++) {
    pthread_join(clienti[i], NULL);
  }
  pthread_join(cuoco, NULL);
  pthread_join(comm, NULL);

  return 0;
}
