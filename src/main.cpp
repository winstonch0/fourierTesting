#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace std; 

const char* WINDOW_NAME = "OPENGL WINDOW";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width = WINDOW_WIDTH, int height = WINDOW_HEIGHT)
{
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

bool init_window(GLFWwindow* &window)
{
    bool success = true;
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        success = false;
    }
    else 
    {
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);

        if (window == NULL)
        {
            cout << "Failed to open GLFW window" << endl;
            success = false;
        }
        else 
        {
            glfwMakeContextCurrent(window);
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                cout << "Failed to initialize GLAD" << endl;
                success = false;
            }
            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        }
    }


    return success;
}

void close(GLFWwindow* &window) 
{
    glfwDestroyWindow(window);
    window = NULL;
    glfwTerminate();
}

int main()
{
    GLFWwindow* window;

    if (!init_window(window)) {
        close(window);
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    while(!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    close(window);
    return 0;
}