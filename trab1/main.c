#include <stdio.h>
#include <GLFW/glfw3.h>
#include <math.h>

#define WIDTH 600
#define HEIGHT 600
#define MAX_ITERATIONS 10000

GLubyte PixelBuffer[WIDTH * HEIGHT * 3];

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

double map(double value, double min, double max, double floor, double ceil) {
    return floor + (ceil - floor) * ((value - min) / (max - min));
}

int main(void)
{
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

    double zoom = 0.001;
    double XOffset = -1.25;
    double YOffset = 0.041;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Setup View
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        /* Render here */
        for(int x = 0; x < width; x++) {
            for(int y = 0; y < height; y++) {
                double a = map(x, 0, width, -2.5 * zoom + XOffset, 0.5 * zoom + XOffset);
                double b = map(y, 0, height, -1.5 * zoom + YOffset, 1.5 * zoom + YOffset);
                
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

        glClear(GL_COLOR_BUFFER_BIT);

        // Drawing
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
        printf("FRAME FIM --> zoom %lf\n", zoom);
        // break;
        zoom *= 0.75;
    }

    glfwTerminate();
    return 0;
}