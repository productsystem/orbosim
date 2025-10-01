#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <random>
#include "camera.h"
#include "shader.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
bool cameraMode = false;
bool tabPressed = false;

const int NUMBODIES = 1000;


Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f; 
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

struct Body {
    glm::vec3 pos;
    glm::vec3 vel;
    float mass;
    glm::vec3 color;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = -(ypos - lastY);

    lastX = xpos;
    lastY = ypos;
    if(cameraMode)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed)
    {
        cameraMode = !cameraMode;
        if (cameraMode)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        tabPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
    {
        tabPressed = false;
    }
        
}

void updatePhysics(std::vector<Body>& bodies, float dt, float Ga) {
    const float G = Ga;
    std::vector<glm::vec3> forces(bodies.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = 0; j < bodies.size(); j++) {
            if (i == j) continue;
            glm::vec3 dir = bodies[j].pos - bodies[i].pos;
            float dist = glm::length(dir);
            float f = G * bodies[i].mass * bodies[j].mass / (dist * dist + 1e-5f);
            forces[i] += f * glm::normalize(dir);
        }
    }

    for (size_t i = 0; i < bodies.size(); i++) {
        glm::vec3 accel = forces[i] / bodies[i].mass;
        bodies[i].vel += accel * dt;
        bodies[i].pos += bodies[i].vel * dt;
    }
}

int main() {
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Orbital Mechanics", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    Shader pointShader("point.vs", "point.fs");

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    float pointVertex[] = { 0.0f, 0.0f, 0.0f };
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertex), pointVertex, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int instanceVBO, colorVBO;
    glGenBuffers(1, &instanceVBO);
    glGenBuffers(1, &colorVBO);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);


    std::vector<Body> bodies;
    bodies.reserve(NUMBODIES);

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distPos(-50.0f, 50.0f);
    std::uniform_real_distribution<float> distVel(-0.1f, 0.1f);
    std::uniform_real_distribution<float> distMass(0.5f, 2.0f);
    std::uniform_real_distribution<float> distColor(0.0f, 1.0f);

    for (int i = 0; i < NUMBODIES; i++) {
        bodies.push_back({
            glm::vec3(distPos(rng), distPos(rng), distPos(rng)),
            glm::vec3(distVel(rng), distVel(rng), distVel(rng)),
            distMass(rng),
            glm::vec3(distColor(rng), distColor(rng), distColor(rng))
            });
    }

    std::vector<glm::vec3> instancePositions(bodies.size());
    std::vector<glm::vec3> instanceColors(bodies.size());


    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 300.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, SCR_HEIGHT));
        ImGui::Begin("Simulation Controls", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
        float fps = 1.0f / deltaTime;
        ImGui::Text("FPS: %.1f", fps);


        static float G = 1.0f;

        ImGui::SliderFloat("Gravity G", &G, 0.01f, 10.0f);

        ImGui::End();

        updatePhysics(bodies, deltaTime, G);

        for (size_t i = 0; i < bodies.size(); i++) {
            instancePositions[i] = bodies[i].pos;
            instanceColors[i] = bodies[i].color;
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, bodies.size() * sizeof(glm::vec3), instancePositions.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glBufferData(GL_ARRAY_BUFFER, bodies.size() * sizeof(glm::vec3), instanceColors.data(), GL_DYNAMIC_DRAW);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        pointShader.use();
        pointShader.setMat4("projection", projection);
        pointShader.setMat4("view", view);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS, 0, 1, bodies.size());
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();


    glfwTerminate();
    return 0;
}
