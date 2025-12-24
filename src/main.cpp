#include <iostream>
#include <string>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader/shader.h>
#include <cmath>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std; 

const char* WINDOW_NAME = "OPENGL WINDOW";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;


const char *vertexCodeString = "./shaders/shader.vert";
const char *fragmentCodeString = "./shaders/shader.frag";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
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

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    GLFWwindow* window;
    if (!init_window(window)) {
        close(window);
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader myShader(vertexCodeString, fragmentCodeString);

    //dodecahedron
    const float gr = (1 + std::sqrt(5)) / 2;
    float vertices[] = {
        0, 1, gr,
        0, -1, gr,
        0, 1, -gr,
        0, -1, -gr,
        1, gr, 0,
        -1, gr, 0,
        1, -gr, 0,
        -1, -gr, 0,
        gr, 0, 1,
        gr, 0, -1,
        -gr, 0, 1,
        -gr, 0, -1,
        0, 0, 0
    };
    unsigned int indices[] = { 
    4,5,0,
    4,5,2,
    6,7,1,
    6,7,3,
    0,1,8,
    0,1,10,
    2,3,9,
    2,3,11,
    8,9,4,
    8,9,6,
    10,11,5,
    10,11,7,
    4,0,8,
    4,2,9,
    5,0,10,
    5,2,11,
    6,1,8,
    6,3,9,
    7,1,10,
    7,3,11
    }; 
    
    glm::vec3 objPositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    //vertex buffer object
    unsigned int VBO;
    glGenBuffers(1, &VBO); 

    //vertex array object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO); 

    //Element buffer object
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        //rendering
        //clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //activate shader
        myShader.use();

        //render triangles
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        int viewLocation = glGetUniformLocation(myShader.ID, "view");
        glUniformMatrix4fv(viewLocation, 1, false, glm::value_ptr(view));

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float)(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 100.0f);
        int projLocation = glGetUniformLocation(myShader.ID, "projection");
        glUniformMatrix4fv(projLocation, 1, false, glm::value_ptr(projection));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        for (int i = 0; i < 10; i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, objPositions[i]);
            model = glm::rotate(model, glm::radians((float)(30 * i))+(float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f)); 
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            int modelLocation = glGetUniformLocation(myShader.ID, "model");
            glUniformMatrix4fv(modelLocation, 1, false, glm::value_ptr(model));

            glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(*indices), GL_UNSIGNED_INT, 0);
        }

        //check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    close(window);
    return 0;
}

