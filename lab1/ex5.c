#include <stdio.h>
#include <pthread.h>

#define NTHREADS 2

int quadrados[10000];

void* dobrar (void* arg) {
    int entrada = * (int*) arg;
    int INICIO;
    int FIM;
    if(entrada == 0) {
        INICIO = 0;
        FIM = 4999;
    } else if(entrada == 1) {
        INICIO = 5000;
        FIM = 9999;
    }

    for(int i=INICIO; i<=FIM; i++){
        printf("[%d] %d ao quadrado eh %d\n", i, i%10, quadrados[i] * quadrados[i]);
        quadrados[i] = quadrados[i] * quadrados[i];
    }

    pthread_exit(NULL);
}

int main (void) {
    pthread_t tid[NTHREADS];
    int ident[NTHREADS];
    char mostrar_vetor;

    for(int i=0; i<=9999; i++){
        quadrados[i] = i%10;
    }

    for(int i=0; i<NTHREADS; i++){
        ident[i] = i;
        if(pthread_create(&tid[i], NULL, dobrar, (void*) &ident[i]))
            printf("ERRO NA CRIACAO DA THREAD\n");
    }

    for(int i=0; i<NTHREADS; i++){
        if(pthread_join(tid[i], NULL))
            printf("ERRO NO JOIN DAS THREADS\n");
    }

    printf("mostrar vetor? (y/n): ");
    scanf("%c", &mostrar_vetor);

    if(mostrar_vetor == 'y' || mostrar_vetor == 'Y'){
        printf("[");
        for(int i=0; i<9999; i++){
            printf("%d, ", quadrados[i]);
        }
        printf("%d]\n", quadrados[9999]);

    }

    pthread_exit(NULL);
    return 0;
}