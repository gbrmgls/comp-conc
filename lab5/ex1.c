#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int n_threads;
int* vetor;

void* acumular (void* arg){
    int id = *(int*) arg;
    long int acumulador = 0;

    for(int i = 0; i < n_threads; i++){
        // Soma todos os elementos do vetor e acumula
        for(int j = 0; j < n_threads; j++){
            acumulador += vetor[j];
        }

        //TODO: aguardar todas as threads terminarem
        // vetor[id] = rand() % 10;
    }

    pthread_exit((void*) acumulador);
    return(NULL);
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Execute: %s <numero de threads>\n", argv[0]);
        return 1;
    }

    n_threads = atoi(argv[1]);
    vetor = malloc(n_threads * sizeof(int));
    long int resultado_thread;

    pthread_t tid[n_threads];
    int ident[n_threads];

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