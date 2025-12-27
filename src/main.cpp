#include <iostream>
#include <string>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader/shader.h>
#include <fftw/fftw3.h>
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

float test_func(float x){
    return x + (float)std::pow(x, 2);
}

fftw_complex *fft_test(int N){
    fftw_complex *in, *out;
    fftw_plan p;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int x = 0; x < N; x++){
        in[x][0] = test_func(float(x/float(N)));
    }

    fftw_execute(p);

    fftw_destroy_plan(p);
    fftw_free(in); 
    return out;
}

int main()
{
    int num_frequencies = 10;

    fftw_complex *test;
    test = fft_test(num_frequencies);
    for (int i = 0; i < 10; i++){
        for (int j = 0; j<2; j++){
            printf("%f ", test[j][i]);
        }
        printf("\n");
    }

    GLFWwindow* window;
    if (!init_window(window)) {
        close(window);
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader myShader(vertexCodeString, fragmentCodeString);

    //dodecahedron
    const float gr = (1 + std::sqrt(5)) / 2;
    float vertices[(num_frequencies + 1) * 3];
    unsigned int indices[num_frequencies * 2];
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[2] = 0;
    for (int i = 0; i < 10; i++){
        vertices[(i+1)*3] = test[i][0];
        vertices[(i+1)*3+1] = test[i][1];
        vertices[(i+1)*3+2] = 0;

        indices[i*2] = i;
        indices[i*2+1] = i+1;
    }
    

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

        glEnable(GL_DEPTH_TEST);

        //view matrix
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        int viewLocation = glGetUniformLocation(myShader.ID, "view");
        glUniformMatrix4fv(viewLocation, 1, false, glm::value_ptr(view));

        //projection matrix
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), (float)(WINDOW_WIDTH / WINDOW_HEIGHT), 0.1f, 100.0f);
        int projLocation = glGetUniformLocation(myShader.ID, "projection");
        glUniformMatrix4fv(projLocation, 1, false, glm::value_ptr(projection));

        //model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 1.0f));
        int modelLocation = glGetUniformLocation(myShader.ID, "model");
        glUniformMatrix4fv(modelLocation, 1, false, glm::value_ptr(model));

        //render triangles
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glDrawElements(GL_LINES, sizeof(indices)/sizeof(*indices), GL_UNSIGNED_INT, 0);

        //check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    close(window);
    return 0;
}

