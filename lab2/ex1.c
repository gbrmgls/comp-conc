#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

int tam_matriz;
int n_threads;
int **matriz_a;
int **matriz_b;
int **matriz_c;
int **matriz_c_sequencial;
double start, finish, elapsed;

double t_sequencial = 0;
double t_s = 0;
double t_p = 0;

void* prod_matrizes(void* arg){
    int ident = *(int*) arg;
    for(int i = ident; i < tam_matriz; i+=n_threads){
        for(int j = 0; j < tam_matriz; j++){
            matriz_c[i][j] = 0;
            for(int k = 0; k < tam_matriz; k++){
                matriz_c[i][j] += matriz_a[i][k] * matriz_b[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Execute: %s <tamanho da matriz> <numero de threads>", argv[0]);
        return 1;
    }

    tam_matriz = atoi(argv[1]);
    n_threads = atoi(argv[2]);
    
    // Inicializacao das estruturas
    GET_TIME(start);

    matriz_a = malloc(tam_matriz * sizeof(*matriz_a));
    matriz_b = malloc(tam_matriz * sizeof(*matriz_b));
    matriz_c = malloc(tam_matriz * sizeof(*matriz_c));
    matriz_c_sequencial = malloc(tam_matriz * sizeof(*matriz_c_sequencial));

    pthread_t tid[n_threads];
    int ident[n_threads];
    int index_teste = 0;
    
    for(int i = 0; i < tam_matriz; i++){
        matriz_a[i] = malloc(tam_matriz * sizeof(*matriz_a[i]));
    }

    for(int i = 0; i < tam_matriz; i++){
        matriz_b[i] = malloc(tam_matriz * sizeof(*matriz_b[i]));
    }

    for(int i = 0; i < tam_matriz; i++){
        matriz_c[i] = malloc(tam_matriz * sizeof(*matriz_c[i]));
    }

    for(int i = 0; i < tam_matriz; i++){
        matriz_c_sequencial[i] = malloc(tam_matriz * sizeof(*matriz_c_sequencial[i]));
    }


    for(int i = 0; i < tam_matriz; i++){
        for(int j = 0; j < tam_matriz; j++){
            matriz_a[i][j] = 3;
        }
    }

    for(int i = 0; i < tam_matriz; i++){
        for(int j = 0; j < tam_matriz; j++){
            matriz_b[i][j] = 5;
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de inicializacao: %lf\n", elapsed);
    t_sequencial += elapsed;
    t_s += elapsed;

    // Criacao, execucao e termino das threads de multiplicacao
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
    printf("Tempo de multiplicacao concorrente: %lf\n", elapsed);
    t_p += elapsed;

    // Multiplicacao sequencial
    GET_TIME(start);


    for(int i = 0; i < tam_matriz; i++){
        for(int j = 0; j < tam_matriz; j++){
            matriz_c_sequencial[i][j] = 0;
            for(int k = 0; k < tam_matriz; k++){
                matriz_c_sequencial[i][j] += matriz_a[i][k] * matriz_b[k][j];
            }
        }
    }

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de multiplicacao sequencial: %lf\n", elapsed);
    t_sequencial += elapsed;

    // Teste

    // for(int i = 0; i < tam_matriz; i++){
    //     for(int j = 0; j < tam_matriz; j++){
    //        if(matriz_c[i][j] == matriz_c_sequencial[i][j]){
    //            index_teste++;
    //        }
    //     }
    // }

    // if(index_teste == tam_matriz * tam_matriz){
    //     puts("Passou no teste.");
    // }
    // else{
    //     puts("não passou no teste.");
    // }

    // Limpeza de espaco alocado das matrizes
    GET_TIME(start);

    for(int i = 0; i < tam_matriz; i++){
        free(matriz_a[i]);
        free(matriz_b[i]);
        free(matriz_c[i]);
        free(matriz_c_sequencial[i]);
    }

    free(matriz_a);
    free(matriz_b);
    free(matriz_c);
    free(matriz_c_sequencial);

    GET_TIME(finish);
    elapsed = finish - start;
    printf("Tempo de limpeza: %lf\n", elapsed);
    t_sequencial += elapsed;
    t_s += elapsed;

    printf("Speed up: %lf\n", (t_sequencial/(t_s + t_p)));
    
    pthread_exit(NULL);

    return 0;
}