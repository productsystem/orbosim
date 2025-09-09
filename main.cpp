//#include "imgui/imgui.h"
//#include "imgui/imgui_impl_glfw.h"
//#include "imgui/imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include "camera.h"
#include "shader.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

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
    float radius;
    bool isSun;
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
}

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, int sectorCount, int stackCount) {
    float x, y, z, xy;
    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    // vertices
    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = glm::pi<float>() / 2 - i * stackStep; // from pi/2 to -pi/2
        xy = cosf(stackAngle);
        z = sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal (same as position for a unit sphere)
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }

    // indices
    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}


void updatePhysics(std::vector<Body>& bodies, float dt) {
    const float G = 1.0f;
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor + capture
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Setup Dear ImGui context
    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls

    //// Setup Dear ImGui style
    //ImGui::StyleColorsDark();

    //// Setup Platform/Renderer backends
    //ImGui_ImplGlfw_InitForOpenGL(window, true);
    //ImGui_ImplOpenGL3_Init("#version 460");




    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // build shaders
    Shader objectShader("object.vs", "object.fs");
    Shader lightShader("light.vs", "light.fs");

    // generate sphere mesh
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateSphere(vertices, indices, 36, 18);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    // physics bodies
    std::vector<Body> bodies;
    bodies.push_back({ glm::vec3(0.0f), glm::vec3(0.0f), 1000.0f, 1.5f, true, glm::vec3(1.0f, 1.0f, 0.6f) });       // central massive body
    float r = 5.0f;
    float sunMass = 1000.0f;
    float v = sqrt(1.0f * sunMass / r);

    bodies.push_back({
        glm::vec3(r, 0.0f, 0.0f),   // position at +X
        glm::vec3(0.0f, v, 0.0f),   // velocity in +Y for circular orbit
        1.0f,
        0.5f,
        false,
        glm::vec3(0.2f, 0.3f, 1.0f)
        });


    // render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // Start ImGui frame
        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();
        //ImGui::Begin("Simulation Controls");

        //static float G = 1.0f;       // gravity
        //static float earthMass = 1.0f;
        //static float sunMass = 1000.0f;

        //ImGui::SliderFloat("Gravity G", &G, 0.01f, 10.0f);
        //ImGui::SliderFloat("Earth Mass", &earthMass, 0.1f, 50.0f);
        //ImGui::SliderFloat("Sun Mass", &sunMass, 100.0f, 5000.0f);

        //ImGui::End();

        updatePhysics(bodies, deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup projection & view matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // --- Draw all bodies (planets / Earth) ---
        objectShader.use();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // Lighting uniforms
        objectShader.setVec3("lightPos", glm::vec3(0.0f, 0.0f, 0.0f)); // Sun at center
        objectShader.setVec3("viewPos", camera.Position);
        objectShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);

        for (auto& b : bodies) {
            if (b.isSun) continue; // skip Sun here

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, b.pos);
            model = glm::scale(model, glm::vec3(b.radius));
            objectShader.setMat4("model", model);

            objectShader.setVec3("objectColor", b.color); // give Earth, Mars, etc. unique colors

            glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
        }

        // --- Draw the Sun ---
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, glm::vec3(0.0f, 0.0f, 0.0f));
        sunModel = glm::scale(sunModel, glm::vec3(2.0f)); // make it bigger
        lightShader.setMat4("model", sunModel);
        lightShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.8f)); // warm glow

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);
        // Render ImGui
       /* ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());*/


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
  /*  ImGui_ImplOpenGL3_Shutdown();*//*
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();*/


    glfwTerminate();
    return 0;
}
