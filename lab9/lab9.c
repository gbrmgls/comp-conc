#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N_PRODUTORES 4
#define N_CONSUMIDORES 4
#define BUFFER_SIZE 10

sem_t mutexProd, mutexCons, mutexBufferCheio, slotsVazios;
int buffer[BUFFER_SIZE];

void retiraTodos (int* elementos, int id) {
  int indice = 0;

  // Espera o buffer encher
  sem_wait(&mutexBufferCheio);
  
  // Secao critica de consumidores
  sem_wait(&mutexCons);

  // Incrementa indice do item retirado
  for(int i=0; i<BUFFER_SIZE; i++) {
    elementos[i] = buffer[indice];
    indice = (indice + 1) % BUFFER_SIZE;
  }
  
  sem_post(&mutexCons);

  // Imprime itens consumidos
  printf("consumidor %d retirando - inicio\n", id);
  for(int i=0; i<BUFFER_SIZE; i++) {
    printf("%d\n", elementos[i]);
  }
  printf("consumidor %d retirando - fim\n", id);

  // Sinaliza o esvaziamento do buffer
  for(int i=0; i<BUFFER_SIZE; i++) {
    sem_post(&slotsVazios);
  }
}

void insereUm (int item, int id) {
  // Indice estatico para manter o andamento de slots do buffer a serem preenchidos
  // ja que essa informacao eh acumulativa
  static int indice = 0;

  // Verifica se tem espaco no buffer
  sem_wait(&slotsVazios);

  // Secao critica de produtores
  sem_wait(&mutexProd);

  // Imprime item inserido
  printf("produtor %d inseriu %d\n", id, item);

  buffer[indice] = item;
  indice++;

  sem_post(&mutexProd);

  // Sinaliza o prrenchimento total do buffer
  if(indice == BUFFER_SIZE) {
    indice = 0;
    sem_post(&mutexBufferCheio);
  }
}

// Tarefa produtora
void *produtor(void * arg) {
  int elemento = 0;
  int id = *(int*)arg;
  
  while(1) {
    insereUm(elemento+id, id);
    elemento++;
  }
  
  pthread_exit(NULL);
}

// Tarefa consumidora
void *consumidor(void * arg) {
  int* elementos;
  int id = *(int*)arg;
  
  elementos = (int*) malloc(sizeof(int) * BUFFER_SIZE);

  while(1) {
    retiraTodos(elementos, id);
  }
  
  pthread_exit(NULL);
}

int main() {
  int N_THREADS = N_PRODUTORES + N_CONSUMIDORES;
  pthread_t tid[N_THREADS];
  int *id[N_THREADS];

  // Preenche vetor com ids indexados das threads
  for(int i=0; i<N_THREADS; i++) {
    (id[i] = malloc(sizeof(int))) == NULL;
    *id[i] = i+1;
  }

  // Inicia semaforos
  sem_init(&mutexCons, 0, 1);
  sem_init(&mutexProd, 0, 1);
  sem_init(&mutexBufferCheio, 0, 0);
  sem_init(&slotsVazios, 0, BUFFER_SIZE);

  // Inicia threads produtoras
  for(int i=0; i<N_PRODUTORES; i++) {
    pthread_create(&tid[i], NULL, produtor, (void *)id[i]);
  }

  // Inicia threads consumidoras
  for(int i=0; i<N_CONSUMIDORES; i++) {
    pthread_create(&tid[i], NULL, consumidor, (void *)id[i]);
  }

  // Espera todas as threads terminarem
  for(int i=0; i<N_THREADS; i++) {
    pthread_join(tid[i], NULL);
  } 
  
  pthread_exit(NULL);
  return 0;
}