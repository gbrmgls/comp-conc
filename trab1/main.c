#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

#define WIDTH 600
#define HEIGHT 600
// #define MAX_ITERATIONS 1000000
// #define N_THREADS 12

// Tipo que armazena as coordenadas de um pixel
typedef struct mtx_coords {
    int x;
    int y;
} t_coords;

// Tipo que armazena configurações do plot da fractal na janela
typedef struct window_plot {
    double zoom;
    double XOffset;
    double YOffset;
    int width, height;
} t_plot;

GLubyte PixelBuffer[WIDTH * HEIGHT * 3];

// Matrizes para testar a corretude da forma concorrente do programa
int mtxtestseq[WIDTH * HEIGHT * 3];
int mtxtestconc[WIDTH * HEIGHT * 3];

// Atribuindo configurações do programa
t_plot WindowMatrixPlot = {0.001, -1.2506, 0.041, WIDTH, HEIGHT};
int N_THREADS;
int MAX_ITERATIONS;
double MAX_BRIGHT_LENGTH;

// Variável que armazena o andamento do programa na matriz
int mtxposition;
pthread_mutex_t lock;

double start, finish, elapsed;

// Recebe o "andamento" da matriz e define a coordenada de onde está
void nextMatrixLocation(int mtxposition, t_coords* fractalMatrixLocation) {
    int x = mtxposition % WIDTH;
    int y = mtxposition / WIDTH;

    fractalMatrixLocation->x = x;
    fractalMatrixLocation->y = y;
}

// Recebe um valor pertencente a um intervalo [min,max] e retorna o valor transformado
// para o intervalo [floor,ceil]
double map(double value, double min, double max, double floor, double ceil) {
    return floor + (ceil - floor) * ((value - min) / (max - min));
}

// "Pinta" os pixels, definidos em PixelBuffer, na janela do programa
void display(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);
    glfwSwapBuffers(window);
}

// Atribui ao pixel na posição (x,y) a cor [r,g,b]
void makePixel(int x, int y, int r, int g, int b, GLubyte* pixels, int width, int height) {
    if (0 <= x && x < width && 0 <= y && y < height) {
        int position = (x + y * width) * 3;
        pixels[position] = r;
        pixels[position + 1] = 0;
        pixels[position + 2] = b;
    }
}


// Função que monta a fractal de mandelbrot na forma sequencial
void mandelbrotSequencial() {
    for(int x = 0; x < WindowMatrixPlot.width; x++) {
        for(int y = 0; y < WindowMatrixPlot.height; y++) {
            double a = map(x, 0, WindowMatrixPlot.width,
                -2.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset,
                0.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset);
            double b = map(y, 0, WindowMatrixPlot.height,
                -1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset,
                1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset);
            
            double ca = a;
            double cb = b;

            int n = 0;

            while (n < MAX_ITERATIONS) {
                double aa = a*a - b*b;
                double bb = 2 * a * b;

                a = aa + ca;
                b = bb + cb;
                if(a*a + b*b > 4) {
                    break;
                }
                n++;
            }
            
            // Controla o brilho de cada pixel, dependendo do número de iterações
            double bright = map(n, 0, MAX_ITERATIONS, 0, MAX_BRIGHT_LENGTH);
            bright = map(sqrt(bright), 0, MAX_BRIGHT_LENGTH, 0, 255);
            if(n == MAX_ITERATIONS) {
                bright = 0;
            }

            // Pinta os pixels na janela
            // makePixel(
            //     x, y, 
            //     bright, bright, bright,
            //     PixelBuffer, WIDTH, HEIGHT
            // );

            // Monta a matriz de pixels da forma sequencial para testar no final
            int position = (x + y * WIDTH) * 3;
            mtxtestseq[position] = bright;
            mtxtestseq[position + 1] = bright;
            mtxtestseq[position + 2] = bright;
        }
    }
}

void* calculaPixel(void* arg) {
    t_coords FractalMatrixLocation;
    int threadPosition;
    while(1) {
        // Soma uma unidade ao "andamento" da matriz e informa qual posição a thread deve calular
        // impedindo que outra thread some o andamento antes da thread ser informada.
        pthread_mutex_lock(&lock);
        mtxposition++;
        threadPosition = mtxposition;
        pthread_mutex_unlock(&lock);

        nextMatrixLocation(threadPosition, &FractalMatrixLocation);
        if(threadPosition >= WIDTH * HEIGHT) break;

        double a = map(FractalMatrixLocation.x, 0, WindowMatrixPlot.width,
            -2.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset,
            0.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset);
        double b = map(FractalMatrixLocation.y, 0, WindowMatrixPlot.height,
            -1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset,
            1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset);
        
        double ca = a;
        double cb = b;

        int n = 0;

        while (n < MAX_ITERATIONS) {
            double aa = a*a - b*b;
            double bb = 2 * a * b;

            a = aa + ca;
            b = bb + cb;
            if(a*a + b*b > 4) {
                break;
            }
            n++;
        }

        // Controla o brilho de cada pixel, dependendo do número de iterações
        double bright = map(n, 0, MAX_ITERATIONS, 0, MAX_BRIGHT_LENGTH);
        bright = map(sqrt(bright), 0, MAX_BRIGHT_LENGTH, 0, 255);
        if(n == MAX_ITERATIONS) {
            bright = 0;
        }

        // Pinta os pixels na janela
        makePixel(
            FractalMatrixLocation.x, FractalMatrixLocation.y, 
            bright, bright, bright,
            PixelBuffer, WIDTH, HEIGHT
        );

        // Monta a matriz de pixels da forma concorrente para testar no final
        int position = (FractalMatrixLocation.x + FractalMatrixLocation.y * WIDTH) * 3;
        mtxtestconc[position] = bright;
        mtxtestconc[position + 1] = bright;
        mtxtestconc[position + 2] = bright;
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    if(argc < 3){
        printf("Execute: %s <numero maximo de iteracoes> <numero de threads>", argv[0]);
        return 1;
    }

    N_THREADS = atoi(argv[2]);
    MAX_ITERATIONS = atoi(argv[1]);

    MAX_BRIGHT_LENGTH = MAX_ITERATIONS < 1000 ? 1 : 10000.0/MAX_ITERATIONS;

    pthread_t threads[N_THREADS];
    pthread_mutex_init(&lock, NULL);

    GLFWwindow* window;
    // Inicializando a biblioteca
    if (!glfwInit())
        return -1;

    // Criando a janela e seu contexto OpenGL
    window = glfwCreateWindow(WIDTH, HEIGHT, "Mandelbrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Cria o contexto atual da janela
    glfwMakeContextCurrent(window);

    int frame = 1;

    GET_TIME(start);
    while (!glfwWindowShouldClose(window))
    {
        // Configuração da visualização
        mtxposition = -1;
        glfwGetFramebufferSize(window, &WindowMatrixPlot.width, &WindowMatrixPlot.height);
        glViewport(0, 0, WindowMatrixPlot.width, WindowMatrixPlot.height);

        for(int i = 0; i < N_THREADS; i++) {
            if(pthread_create(&threads[i], NULL, calculaPixel, NULL)) {
                printf("Erro pthread_create");
                return 1;
            }
        }

        for (int i = 0; i < N_THREADS; i++) {
            if(pthread_join(threads[i], NULL)) {
                printf("Erro pthread_join");
                return 1;
            }
        }
        // mandelbrotSequencial();

        frame++;
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenhando
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);

        // Funções necesárias para o funcionamento da biblioteca que desenha os pixels
        glfwSwapBuffers(window);
        glfwPollEvents();

        WindowMatrixPlot.zoom *= 0.75;
        if(frame == 100) break;
    }
    GET_TIME(finish);
    elapsed = finish - start;

    // printf("Tempo de execucao sequencial: %lf\n", elapsed);
    printf("Tempo de execucao concorrente: %lf\n", elapsed);

    int deuRuim = 0;
    for(int i = 0; i < WIDTH * HEIGHT * 3; i++) {
        if(mtxtestconc[i] != mtxtestseq[i]) {
            deuRuim = 1;
            printf("deu ruim\n");
            printf("pixel %d %d\n", (i/3)%WIDTH, (i/3)/WIDTH);
            printf("mtxtestconc[i] = %d, mtxtestseq[i] = %d\n", mtxtestconc[i], mtxtestseq[i]);
            break;
        }
    }
    if(!deuRuim) printf("Deu bom\n");

    pthread_mutex_destroy(&lock);
    glfwTerminate();
    return 0;
}