#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader/shader.h>
#include <fftw/fftw3.h>
#include <cmath>
#include <stdio.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std; 

const char* WINDOW_NAME = "OPENGL WINDOW";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;


const char *vertexCodeString = "./shaders/shader.vert";
const char *fragmentCodeString = "./shaders/shader.frag";

class Circle
{
public:
    glm::vec3 position;
    float radius;

    Circle(float x, float y, float z, float radius) 
    {
        position = glm::vec3(x, y, z);
        this->radius = radius;
    }
};

// -------------------- CircleRenderer (ALL FIXED) --------------------
class CircleRenderer
{
public:
    CircleRenderer() = default;

    ~CircleRenderer()
    {
        if (VAO)         glDeleteVertexArrays(1, &VAO);
        if (meshVBO)     glDeleteBuffers(1, &meshVBO);
        if (instanceVBO) glDeleteBuffers(1, &instanceVBO);
        if (myShader.ID) glDeleteProgram(myShader.ID);
    }

    void init(int w, int h, const char* vsFile, const char* fsFile)
    {
        windowWidth  = w;
        windowHeight = h;

        myShader.init(vsFile, fsFile);
        setupCircleMesh();
        setupInstanceBuffer();
        updateProjection(windowWidth, windowHeight);
    }

    void setCircles(const std::vector<Circle>& c)
    {
        circles = c;
        updateInstanceBuffer();
    }

    // Call this if the window is resized
    void onResize(int w, int h)
    {
        windowWidth  = (w > 0) ? w : 1;
        windowHeight = (h > 0) ? h : 1;
        updateProjection(windowWidth, windowHeight);
    }

    void draw()
    {
        if (!myShader.ID || !VAO) return;

        myShader.use();
        glBindVertexArray(VAO);

        glDrawArraysInstanced(
            GL_LINE_STRIP,
            0,
            vertexCount,
            static_cast<GLsizei>(circles.size())
        );
    }

private:
    void setupCircleMesh()
    {
        std::vector<float> verts;
        verts.reserve((SEGMENT_NUMBER + 1) * 3);

        float twoPi = 2.0f * static_cast<float>(M_PI);

        for (int i = 0; i <= SEGMENT_NUMBER; i++)
        {
            float a = twoPi * i / SEGMENT_NUMBER;
            verts.push_back(std::cos(a));
            verts.push_back(std::sin(a));
            verts.push_back(0.0f);
        }

        vertexCount = static_cast<int>(verts.size() / 3);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &meshVBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     verts.size() * sizeof(float),
                     verts.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    void setupInstanceBuffer()
    {
        glGenBuffers(1, &instanceVBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // allocate 0 for now; real size in updateInstanceBuffer()
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // Attribute 1: Circle.position (vec3)
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Circle), (void*)offsetof(Circle, position)
        );
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);

        // Attribute 2: Circle.radius (float)
        glVertexAttribPointer(
            2, 1, GL_FLOAT, GL_FALSE,
            sizeof(Circle), (void*)offsetof(Circle, radius)
        );
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
    }

    void updateInstanceBuffer()
    {
        // Upload your circles directly (NO CircleInstanceGPU, no copying)
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     circles.size() * sizeof(Circle),
                     circles.data(),
                     GL_DYNAMIC_DRAW);
    }

    void updateProjection(int w, int h)
    {
        float aspect = static_cast<float>(h) / static_cast<float>(w);

        glm::mat4 proj = glm::ortho(-1.0f, 1.0f,
                                    -aspect, aspect);

        myShader.use();
        GLint loc = glGetUniformLocation(myShader.ID, "projection");
        if (loc != -1)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
        }
    }

private:
    GLuint VAO = 0;
    GLuint meshVBO = 0;
    GLuint instanceVBO = 0;
    Shader myShader;

    int vertexCount = 0;
    int windowWidth = 1;
    int windowHeight = 1;

    std::vector<Circle> circles;

    static constexpr int SEGMENT_NUMBER = 120;
};

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
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            glViewport(0, 0, w, h);

        }
    }


    return success;
}

void close_window(GLFWwindow* &window) 
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

void storeCode()
{
    //idk what this code does but imma just store it

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
        close_window(window);
        //return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader myShader;
    myShader.init(vertexCodeString, fragmentCodeString);

    //dodecahedron
    const float gr = (1 + std::sqrt(5)) / 2;
    float vertices[(num_frequencies + 1) * 3];
    unsigned int indices[num_frequencies * 2];
    vertices[0] = 0;
    vertices[1] = 0;
    vertices[2] = 0;
    for (int i = 0; i < 10; i++){
        vertices[(i+1)*3] = test[i][0] + vertices[(i+1)*3 - 2];
        vertices[(i+1)*3+1] = test[i][1] + vertices[(i+1)*3 - 1];
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

    close_window(window);

    //mine!!!
    /*
       Shader myShader(vertexCodeString, fragmentCodeString);
    auto circle = drawCircle(0, 0, 0, 0.6f, 120);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 circle.size() * sizeof(float),
                 circle.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);
    */
    //return 0;
}

std::vector<float> drawCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides )
{
    float aspect = (float)WINDOW_HEIGHT / WINDOW_WIDTH;
    std::vector<float> circle;
    float twoPi = 2.0f * M_PI;
    int segments = 1 + numberOfSides;

    for (int i = 0; i <= segments; i++)
    {
        float angle = twoPi * i / segments;
        circle.push_back(radius * cos(angle) * aspect);
        circle.push_back(radius * sin(angle));
        circle.push_back(0.0f);
    }

    return circle;
}

int main()
{
    GLFWwindow* window;

    if (!init_window(window)) {
        close_window(window);
        return -1;
    }
    
   
    CircleRenderer renderer;
    
    renderer.init(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        vertexCodeString,
        fragmentCodeString
    );
    glfwSetWindowUserPointer(window, &renderer);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    renderer.onResize(w, h);


    std::vector<Circle> circles;

    const int NUM_CIRCLES = 150;

    // random generators
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> posX(-0.9f, 0.9f);
    std::uniform_real_distribution<float> posY(-0.9f, 0.9f);
    std::uniform_real_distribution<float> radiusDist(0.02f, 0.06f);

    circles.clear();
    circles.reserve(NUM_CIRCLES);

    for (int i = 0; i < NUM_CIRCLES; ++i)
    {
        float r = radiusDist(rng);

        circles.emplace_back(
            posX(rng),
            posY(rng),
            0.0f,
            r
        );
    }

    renderer.setCircles(circles);

    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* win, int w, int h)
        {
            glViewport(0, 0, w, h);

            auto* renderer =
                static_cast<CircleRenderer*>(
                    glfwGetWindowUserPointer(win)
                );

            if (renderer)
                renderer->onResize(w, h);
        }
    );



    while(!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.draw();

        //check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    close_window(window);
    glfwTerminate();

    return 0;
}

