#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<time.h>

int n_threads;
int* vetor;

int bloqueadas_ler = 0;
pthread_mutex_t x_mutex_ler;
pthread_cond_t x_cond_ler;

int bloqueadas_escrever = 0;
pthread_mutex_t x_mutex_escrever;
pthread_cond_t x_cond_escrever;

void barreira_ler(int nthreads, int id){
    pthread_mutex_lock(&x_mutex_ler);

    if(bloqueadas_ler == (n_threads-1)){
        pthread_cond_broadcast(&x_cond_ler);
        bloqueadas_ler = 0;
    } else {
        bloqueadas_ler++;
        pthread_cond_wait(&x_cond_ler, &x_mutex_ler);
    }
    pthread_mutex_unlock(&x_mutex_ler); 
}

void barreira_escrever(int nthreads, int id){
    pthread_mutex_lock(&x_mutex_escrever);

    if(bloqueadas_escrever == (n_threads-1)){
        pthread_cond_broadcast(&x_cond_escrever);
        bloqueadas_escrever = 0;
    } else {
        bloqueadas_escrever++;
        pthread_cond_wait(&x_cond_escrever, &x_mutex_escrever);
    }
    pthread_mutex_unlock(&x_mutex_escrever);
}

void* acumular (void* arg){
    int id = *(int*) arg;
    long int acumulador = 0;

    for(int i = 0; i < n_threads; i++){

        for(int j = 0; j < n_threads; j++){
            acumulador += vetor[j];
        }

        barreira_ler(n_threads, id);

        vetor[id] = rand() % 10;

        barreira_escrever(n_threads, id);
    }

    pthread_exit((void*) acumulador);
    return(NULL);
}

int main(int argc, char* argv[]){

    srand(time(NULL));

    if(argc < 2){
        printf("Execute: %s <numero de threads>\n", argv[0]);
        return 1;
    }

    n_threads = atoi(argv[1]);
    vetor = malloc(n_threads * sizeof(int));
    long int resultado_thread;

    pthread_t tid[n_threads];
    int ident[n_threads];

    pthread_mutex_init(&x_mutex_ler, NULL);
    pthread_cond_init (&x_cond_ler, NULL);

    pthread_mutex_init(&x_mutex_escrever, NULL);
    pthread_cond_init (&x_cond_escrever, NULL);

    // Inicializa o vetor

    for(int i = 0; i < n_threads; i++){
        vetor[i] = rand() % 10;
    }

    for(int i = 0; i < n_threads; i++){
        ident[i] = i;
        if(pthread_create(&tid[i], NULL, acumular, (void *) &ident[i])){
            printf("ERRO NA CRIACAO DA THREAD");
        }
    }

    for(int i = 0; i < n_threads; i++){
        ident[i] = i;
        if(pthread_join(tid[i], (void*) &resultado_thread)){
            printf("ERRO NO JOIN DAS THREADS");
        }

        printf("Resultado da thread %d: %ld\n", i, resultado_thread);
    }

    // free(vetor);

    return 0;
}