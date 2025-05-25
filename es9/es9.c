#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
 * This program  has 5 producer threads and one consumer
 * There is a buffer which is a char[6] where the last char is 0, like \0 in
 * strings. Basically the exercise asks to build the buffer in a certain way
 * - each caracheter of the buffer is written by a thread
 *   - e.g. buff[0] is written by thread[0] so thread with index 0
 *   - ...buff[5] is 0 to simulte string terminator
 *
 * Consumer gets activated only when all the threads have written to the
 * buffer, so it can be read by the consumer
 *
 * */
typedef struct {
  char message[6];    // 5 elements where 6 is 0, like \0 for strings
  sem_t producers[5]; // there are 5 producesrs, each with their own semaphore
  sem_t mutex;        // for mutual exclusion inside this resource
  sem_t consumer; // where consumer is blocked and can be activated when message
                  // is ready
  char iterations; // variabile che permette di far stampare un messaggio
                   // diverso ogni volta (sommo questo) "padding" a ciascun
                   // carattere della stringa ad ogni iterazione, così avrò, ad
                   // esempio abcde, bcdef, cdefg, ..
  int n_caratter_depositati;
  int stop; // per stoppare il consumer
} buf_t;
buf_t global_buf;

char *shared_message = "abcde"; // this is a sample of message

void buf_init(buf_t *buf) {
  buf->iterations = 0;
  buf->n_caratter_depositati = 0;
  buf->message[5] =
      0; // l'ultimo elemento del messaggio è uno 0, così so che ho finito
  sem_init(&buf->mutex, 0, 1);

  // now i have to initialize all of the other semapores for each producer
  for (int i = 0; i < 5; i++) {
    sem_init(&buf->producers[i], 0, 0);
  }
}
void *producer(void *arg) {
  int thread_idx = *(int *)arg;

  // since each thread wirtes at it's position, so thread0 writes mess[0].
  // ogni thread prende il suo caratter da shared_message
  char thread_car = shared_message[thread_idx];

  while (1) {
    // i have to write at the messagge[thread_idx];
    global_buf.message[thread_idx] =
        (thread_car +
         (global_buf
              .iterations)); // praticamete, la prima volta scrivo abcde, poi
                             // scriverò bcdef, poi cdefg, per far vedere che ad
                             // ogni giro scrivo qualcosa diverso.
    // Il valore della ioterations lo cambia il consumer
    // ATTENZIONE : vedi che in questo caso mi serve mutua esclusione perché
    // ogni thread scrive in una porzione diversa del buffe , nella sua
    printf("thread[%d] prende '%c'\n", thread_idx, thread_car);

    // ora comincio a modificare le variabili comuni
    // devo dire che nel buffer c'è un carattere in più
    // E se il nuero è 5, segnalare al consumer questo
    sem_wait(&global_buf.mutex);
    global_buf.n_caratter_depositati++;

    //    printf("Num_caratteri depositati %d\n",
    //    global_buf.n_caratter_depositati);
    // who do i need to wake up?
    if (global_buf.n_caratter_depositati == 5) {
      sem_post(&global_buf.consumer);
    }

    // now i can release the mutex because i have to block
    sem_post(&global_buf.mutex);
    sem_wait(
        &global_buf
             .producers[thread_idx]); // dopo aver depositato il mio messaggio,
                                      // mi metto in attesa sul mio semfaroro,
                                      // sarà il consumer a doermi svegliare
    sleep(1);
  }
  pthread_exit(0);
}

void *consumer(void *arg) {
  // il consumer by default è bloccato sul suo semfaroro
  //
  while (1) {
    sem_wait(&global_buf.consumer);
    // now he has woken up, means all producers have written to the buffer
    // he has to read, and update the variables
    sem_wait(&global_buf.mutex);

    // read the message
    printf("Il messaggio depositato è '%s'\n", global_buf.message);
    // resetto il messaggio
    global_buf.n_caratter_depositati = 0;
    // who do i need to wake up?
    //   all the producers
    for (int i = 0; i < 5; i++) {
      sem_post(&global_buf.producers[i]);
    }

    // i have to also reset that offeset of caracters
    global_buf.iterations =
        (global_buf.iterations + 1) % 26; // ensure value stays between 0 and 25

    // now i can release the mutex
    sem_post(&global_buf.mutex);
  }
  pthread_exit(0);
}
int main(int argc, char *argv[]) {
  pthread_attr_t a;
  pthread_t threads[6];
  int consumer_ids[5] = {0, 1, 2, 3, 4};
  int current_consumer = 0;

  buf_init(&global_buf);
  for (int i = 0; i < 6; i++) {

    if (i == 0)
      pthread_create(&threads[i], &a, consumer,
                     NULL); // il primo processo è il lettore, consumer
    else {
      // è necessario fare un array per memorizzare gli indici dei thread,
      // poiché, se si passasse semplicemente il puntatore a producer_idx,
      // questo risulterebbe nella condivisione di tale puntatore da parte di
      // tutti i thread, causando un comportamento non voluto!

      // usando un array è garantito che ciascun thread prenderà il puntatore
      // all'elemento di tale array, risultando in un'associazione univoca
      // thread => id.

      pthread_create(
          &threads[current_consumer], &a, producer,
          (void *)&consumer_ids[current_consumer]); // gli altri sono
                                                    // produttori, ai quali
                                                    // assegno un indice
      current_consumer++;
    }
  }

  for (int i = 0; i < 6; i++) {

    int *retval;
    pthread_join(threads[i], (void **)&retval);
  }

  return 0;
}
