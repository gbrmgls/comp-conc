#include <stdio.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <pthread.h>
#include "timer.h"

#define WIDTH 600
#define HEIGHT 600
#define MAX_ITERATIONS 10000
#define N_THREADS 12

typedef struct mtx_coords {
    int x;
    int y;
} t_coords;

typedef struct window_plot {
    double zoom;
    double XOffset;
    double YOffset;
    int width, height;
} t_plot;

GLubyte PixelBuffer[WIDTH * HEIGHT * 3];
t_coords FractalMatrixLocation;
t_plot WindowMatrixPlot = {0.001, -1.25, 0.041, WIDTH, HEIGHT};
pthread_mutex_t lock;
double start, finish, elapsed;

void nextMatrixLocation() {
    int x = FractalMatrixLocation.x + 1;
    int y = FractalMatrixLocation.y;
    if(x == WIDTH) {
        x = 0;
        y++;
    }
    FractalMatrixLocation.x = x;
    FractalMatrixLocation.y = y;
}

double map(double value, double min, double max, double floor, double ceil) {
    return floor + (ceil - floor) * ((value - min) / (max - min));
}

void display(GLFWwindow *window) {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);
    glfwSwapBuffers(window);
}

void makePixel(int x, int y, int r, int g, int b, GLubyte* pixels, int width, int height) {
    if (0 <= x && x < width && 0 <= y && y < height) {
        int position = (x + y * width) * 3;
        pixels[position] = r;
        pixels[position + 1] = g;
        pixels[position + 2] = b;
    }
}

void mandelbrotSequencial() {
    /* Render here */
        for(int x = 0; x < WindowMatrixPlot.width; x++) {
            for(int y = 0; y < WindowMatrixPlot.height; y++) {
                double a = map(x, 0, WindowMatrixPlot.width, -2.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset, 0.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset);
                double b = map(y, 0, WindowMatrixPlot.height, -1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset, 1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset);
                
                double ca = a;
                double cb = b;

                int n = 0;

                while (n < MAX_ITERATIONS) {
                    double aa = a*a - b*b;
                    double bb = 2 * a * b;

                    a = aa + ca;
                    b = bb + cb;
                    if(a + b > 16 || a + b < -16) {
                        break;
                    }
                    n++;
                }

                double bright = map(n, 0, MAX_ITERATIONS, 0, 1);
                bright = map(sqrt(bright), 0, 1, 0, 255);
                if(n == MAX_ITERATIONS) {
                    bright = 0;
                }
                makePixel(
                    x, y, 
                    bright, bright, bright,
                    PixelBuffer, WIDTH, HEIGHT
                );
            }
        }
}

void* calculaPixel(void* arg) {
    while(FractalMatrixLocation.y < HEIGHT) {
        pthread_mutex_lock(&lock);
        nextMatrixLocation();
        pthread_mutex_unlock(&lock);
        // printf("x = %d e y = %d\n", FractalMatrixLocation.x, FractalMatrixLocation.y);
        double a = map(FractalMatrixLocation.x, 0, WindowMatrixPlot.width, -2.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset, 0.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.XOffset);
        double b = map(FractalMatrixLocation.y, 0, WindowMatrixPlot.height, -1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset, 1.5 * WindowMatrixPlot.zoom + WindowMatrixPlot.YOffset);
        
        double ca = a;
        double cb = b;

        int n = 0;

        while (n < MAX_ITERATIONS) {
            double aa = a*a - b*b;
            double bb = 2 * a * b;

            a = aa + ca;
            b = bb + cb;
            if(a + b > 16 || a + b < -16) {
                break;
            }
            n++;
        }

        double bright = map(n, 0, MAX_ITERATIONS, 0, 1);
        bright = map(sqrt(bright), 0, 1, 0, 255);
        if(n == MAX_ITERATIONS) {
            bright = 0;
        }
        makePixel(
            FractalMatrixLocation.x, FractalMatrixLocation.y, 
            bright, bright, bright,
            PixelBuffer, WIDTH, HEIGHT
        );
    }
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t threads[N_THREADS];
    pthread_mutex_init(&lock, NULL);

    GLFWwindow* window;
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "Mandelbrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // double zoom = 0.001;
    // double XOffset = -1.25;
    // double YOffset = 0.041;
    int frame = 1;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Setup View
        // int width, height;
        FractalMatrixLocation.x = -1;
        FractalMatrixLocation.y = 0;

        glfwGetFramebufferSize(window, &WindowMatrixPlot.width, &WindowMatrixPlot.height);
        glViewport(0, 0, WindowMatrixPlot.width, WindowMatrixPlot.height);

        GET_TIME(start);
        /* Render here */
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
        // for(int x = 0; x < width; x++) {
        //     for(int y = 0; y < height; y++) {
        //         double a = map(x, 0, width, -2.5 * zoom + XOffset, 0.5 * zoom + XOffset);
        //         double b = map(y, 0, height, -1.5 * zoom + YOffset, 1.5 * zoom + YOffset);
                
        //         double ca = a;
        //         double cb = b;

        //         int n = 0;

        //         while (n < MAX_ITERATIONS) {
        //             double aa = a*a - b*b;
        //             double bb = 2 * a * b;

        //             a = aa + ca;
        //             b = bb + cb;
        //             if(a + b > 16 || a + b < -16) {
        //                 break;
        //             }
        //             n++;
        //         }

        //         double bright = map(n, 0, MAX_ITERATIONS, 0, 1);
        //         bright = map(sqrt(bright), 0, 1, 0, 255);
        //         if(n == MAX_ITERATIONS) {
        //             bright = 0;
        //         }
        //         makePixel(
        //             x, y, 
        //             bright, bright, bright,
        //             PixelBuffer, WIDTH, HEIGHT
        //         );
        //     }
        // }
        GET_TIME(finish);
        elapsed = finish - start;
        printf("Frame %d --> ", frame++);
        // printf("Tempo de execucao sequencial: %lf\n", elapsed);
        printf("Tempo de execucao concorrente: %lf\n", elapsed);

        glClear(GL_COLOR_BUFFER_BIT);

        // Drawing
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        // printf("FRAME FIM --> zoom %lf\n", WindowMatrixPlot.zoom);
        // break;
        WindowMatrixPlot.zoom *= 0.75;
        if(frame == 11) break;
    }

    pthread_mutex_destroy(&lock);
    glfwTerminate();
    return 0;
}