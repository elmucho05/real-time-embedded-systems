#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N 10

#define _printf(...)                                                           \
  printf(__VA_ARGS__);                                                         \
  fflush(stdout)

struct pasticceria_t {
  sem_t spazio_disponibile;      // 10
  sem_t aspettando_torte_pronte; // dove il commesso aspetta
  sem_t aspettando_clienti; // dove il commeesso aspetta che ci siano richieste
  sem_t cliente_aspetta_che_venga_servito; // dove i clienti aspettano che il
                                           // commesso venda la torta

} g_pasticceria;

void pasticceria_init(struct pasticceria_t *p) {
  sem_init(&p->spazio_disponibile, 0, N);
  sem_init(&p->aspettando_torte_pronte, 0, 0);
  sem_init(&p->aspettando_clienti, 0, 0);
  sem_init(&p->cliente_aspetta_che_venga_servito, 0, 0);
}

void cuoco_inizio_torta(struct pasticceria_t *p) {
  sem_wait(&p->spazio_disponibile); // decrementa spazioe spazio_disponibile
}

void cuoco_fine_torta(struct pasticceria_t *p) {
  sem_post(&p->aspettando_torte_pronte); // incrementa spazio_disponibile
}

void commesso_prendo_torta(struct pasticceria_t *p) {
  sem_wait(&p->aspettando_clienti);      // prima aspetta per i clienti
  sem_wait(&p->aspettando_torte_pronte); // poi aspetta per le torte
}

void commesso_vendo_torta(struct pasticceria_t *p) {
  sem_post(&p->cliente_aspetta_che_venga_servito); // dii che c'è una consegna,
                                                   // vendi torta
  sem_post(&p->spazio_disponibile); // dii al cuoco che puoi preapare altre
  // incrementa spazio_disponibile
}

void cliente_acquisto(struct pasticceria_t *p) {
  sem_post(&p->aspettando_clienti); // dii al commesso di vendermi una torta e
                                    // poi aspetta
  sem_wait(&p->cliente_aspetta_che_venga_servito); // one more client waiting
}

// ------------------------------------------------------------------------------------------------

void *cuoco(void *arg) {
  while (1) {
    cuoco_inizio_torta(&g_pasticceria);

    int torte_da_preparare;
    sem_getvalue(&g_pasticceria.spazio_disponibile,
                 &torte_da_preparare); // May not be thread-safe
    _printf("cuoco> Ho preparato la torta %d \n", torte_da_preparare);

    cuoco_fine_torta(&g_pasticceria);

    sleep(rand() % 2);
  }
}

void *commesso(void *arg) {
  while (1) {
    commesso_prendo_torta(&g_pasticceria);

    _printf("commesso> Vendo la torta...\n");

    commesso_vendo_torta(&g_pasticceria);

    sleep(rand() % 2);
  }
}

void *un_cliente(void *arg) {
  while (1) {
    _printf("cliente> Acquisto una torta...\n");

    cliente_acquisto(&g_pasticceria);

    _printf("cliente> Torta acquistata\n");

    sleep(rand() % 2);
  }
}

int main(int argc, char *argv[]) {
  pthread_t thread_cuoco;
  pthread_t thread_commesso;
  pthread_t thread_cliente;

  pasticceria_init(&g_pasticceria);

  pthread_create(&thread_cuoco, NULL, cuoco, NULL);
  pthread_create(&thread_commesso, NULL, commesso, NULL);
  pthread_create(&thread_cliente, NULL, un_cliente, NULL);

  pthread_join(thread_cuoco, NULL);
  pthread_join(thread_commesso, NULL);
  pthread_join(thread_cliente, NULL);

  return 0;
}
