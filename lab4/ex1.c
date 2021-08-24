#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define n_threads 4

pthread_mutex_t lock;
pthread_cond_t cond1;
pthread_cond_t cond2;
int count = 0;

void * thread1(void * arg) {
    pthread_mutex_lock(&lock);
    
    if(count == 0){
        pthread_cond_wait(&cond1, &lock);
        count++;
    }
    
    puts("Fique a vontade.");
    
    if(count == 3) {
        pthread_cond_signal(&cond2);
    }

    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

void * thread2(void * arg) {
    puts("Seja bem-vindo!");
    sleep(1);
    pthread_mutex_lock(&lock);
    count++;
    pthread_cond_broadcast(&cond1);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

void * thread3(void * arg) {
    pthread_mutex_lock(&lock);
    
    if(count < 3){
        pthread_cond_wait(&cond2, &lock);
    }

    puts("Volte sempre!");
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

void * thread4(void * arg) {
    pthread_mutex_lock(&lock);
    if(count == 0){
        pthread_cond_wait(&cond1, &lock);
        count++;
    }
    puts("Sente-se por favor.");

    if(count == 3) {
        pthread_cond_signal(&cond2);
    }

    pthread_mutex_unlock(&lock); 
    pthread_exit(NULL);  
}

int main() {
    pthread_t threads[n_threads];
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init (&cond1, NULL);
    pthread_cond_init (&cond2, NULL);
    pthread_create(&threads[1], NULL, thread2, NULL);
    pthread_create(&threads[0], NULL, thread1, NULL);
    pthread_create(&threads[3], NULL, thread4, NULL);
    pthread_create(&threads[2], NULL, thread3, NULL);

    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);
}