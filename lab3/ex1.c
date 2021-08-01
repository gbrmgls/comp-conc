#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

long long int tam_vetor;
int n_threads;
int *vetor;
int maior;
double start, finish, elapsed;

double t_sequencial = 0;
double t_s = 0;
double t_p = 0;

void* prod_matrizes(void* arg){
    int ident = *(int*) arg;
    // calcular intervalo da matriz pra somar
    // somar
    pthread_exit(NULL); // retornar soma
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
        if(pthread_create(&tid[i], NULL, prod_matrizes, (void *) &ident[i])){
            printf("ERRO NA CRIACAO DA THREAD");
        }
    }

    for(int i = 0; i < n_threads; i++){
        ident[i] = i;
        if(pthread_join(tid[i], NULL)){
            printf("ERRO NO JOIN DAS THREADS");
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de execucao concorrente: %lf\n", elapsed);
    t_p += elapsed;

    // Verificacao sequencial
    GET_TIME(start);

    for(int i = 0; i < tam_vetor; i++){
        if(vetor[i] > maior){
            maior = vetor[i];
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de execucao sequencial: %lf\n", elapsed);
    t_sequencial += elapsed;

    // Teste


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