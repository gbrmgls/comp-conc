#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define n_threads 4

int messages_left = 2; // Total de mensagens entre t2 e t3
int t1t4_flag = 0; // Flag para decidir qual mensagem mostrar em t1t4

sem_t mutex; 
sem_t sem_t1t4, sem_t3; 

void* t1t4(void* args) {
  sem_wait(&sem_t1t4);

  // Secao critica
  sem_wait(&mutex);
  
  // Checa flag para mostrar cada mensagem
  if(t1t4_flag == 0) {
    printf("Fique a vontade.\n");
    t1t4_flag++;
  }
  else {
    printf("Sente-se por favor.\n");
  }

  messages_left--;

  // Se nao faltar nenhuma mensagem, liberar t3
  if(messages_left == 0){
    sem_post(&sem_t3);
  }
  
  sem_post(&mutex);

  pthread_exit(NULL);
}

void* t2(void* args) {
  printf("Seja bem-vindo!\n");

  // Libera as duas execucoes de t1t4
  sem_post(&sem_t1t4);
  sem_post(&sem_t1t4);

  pthread_exit(NULL);
}

void* t3(void* args) {
  // Espera semaforo de t3
  sem_wait(&sem_t3);

  printf("Volte sempre!\n");
  
  pthread_exit(NULL);
}

int main() {
  pthread_t tid[n_threads];

  sem_init(&mutex, 0, 1);
  sem_init(&sem_t1t4, 0, 0);
  sem_init(&sem_t3, 0, 0);

  pthread_create(&tid[0], NULL, t1t4, NULL);
  pthread_create(&tid[1], NULL, t2, NULL);
  pthread_create(&tid[2], NULL, t3, NULL);
  pthread_create(&tid[3], NULL, t1t4, NULL);

  for (int i = 0; i < n_threads; i++) {
    pthread_join(tid[i], NULL);
  } 

  pthread_exit(NULL);
}