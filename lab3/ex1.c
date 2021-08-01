#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

long long int tam_vetor;
int n_threads;
int *vetor;
int conc_maior = -1;
int conc_menor = 100;
int seq_maior = -1;
int seq_menor = 100;
double start, finish, elapsed;

double t_sequencial = 0;
double t_s = 0;
double t_p = 0;

typedef struct {
  float menor;
  float maior;
} thread_data;

void* verifica_vetor(void* arg){
    int ident = *(int*) arg;

    thread_data* thread_resultado;
    thread_resultado = (thread_data*) malloc(sizeof(thread_data));
    thread_resultado->maior = -1;
    thread_resultado->menor = 100;

    long long int inicio = (tam_vetor/n_threads) * ident;
    long long int final;
    if(ident == n_threads - 1){
        final = tam_vetor;
    } else {
        final = (tam_vetor/n_threads) + inicio;
    }
    
    for(int i = inicio; i < final; i++){
        if(vetor[i] > thread_resultado->maior){
            thread_resultado->maior = vetor[i];
        }
        if(vetor[i] < thread_resultado->menor) {
            thread_resultado->menor = vetor[i];
        }
    }

    pthread_exit((void*) thread_resultado);
    return(NULL);
}

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Execute: %s <tamanho da matriz> <numero de threads>", argv[0]);
        return 1;
    }

    tam_vetor = atoll(argv[1]);
    n_threads = atoi(argv[2]);
    
    // Inicializacao das estruturas
    GET_TIME(start);

    vetor = malloc(tam_vetor * sizeof(*vetor));

    pthread_t tid[n_threads];
    int ident[n_threads];
    thread_data* thread_resultado;
    
    for(int i = 0; i < tam_vetor; i++){
        vetor[i] = i % 10;
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de inicializacao: %lf\n", elapsed);
    t_sequencial += elapsed;
    t_s += elapsed;

    // Criacao, execucao e termino das threads de verificacao
    GET_TIME(start);

    for(int i = 0; i < n_threads; i++){
        ident[i] = i;
        if(pthread_create(&tid[i], NULL, verifica_vetor, (void *) &ident[i])){
            printf("ERRO NA CRIACAO DA THREAD");
        }
    }

    for(int i = 0; i < n_threads; i++){
        ident[i] = i;
        if(pthread_join(tid[i], (void**) &thread_resultado)){
            printf("ERRO NO JOIN DAS THREADS");
        }

        if(thread_resultado->maior > conc_maior){
            conc_maior = thread_resultado->maior;
        }
        if(thread_resultado->menor < conc_menor){
            conc_menor = thread_resultado->menor;
        }

        free(thread_resultado);
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de execucao concorrente: %lf\n", elapsed);
    t_p += elapsed;

    // Verificacao sequencial
    GET_TIME(start);

    for(int i = 0; i < tam_vetor; i++){
        if(vetor[i] > seq_maior){
            seq_maior = vetor[i];
        }
        if(vetor[i] < seq_menor) {
            seq_menor = vetor[i];
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de execucao sequencial: %lf\n", elapsed);
    t_sequencial += elapsed;

    // Teste

    // if((seq_maior == conc_maior) && (seq_menor == conc_menor)){
    //     puts("Passou no teste.");
    // }


    // Limpeza de espaco alocado do vetor
    GET_TIME(start);

    free(vetor);

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de limpeza: %lf\n", elapsed);
    t_sequencial += elapsed;
    t_s += elapsed;
    printf("Speed up: %lf\n", (t_sequencial/(t_s + t_p)));

    return 0;
}