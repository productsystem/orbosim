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
Camera camera(glm::vec3(0.0f, 0.0f, 25.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

const int NUM_ANTS = 500;
const int GRID_SIZE = 128;
float pheromone[GRID_SIZE][GRID_SIZE] = { 0.0f };
float PHEROMONE_DECAY = 0.995f;

struct Ant {
    glm::vec2 pos;
    glm::vec2 vel;
    bool carryingFood;
};

std::vector<Ant> ants;
std::vector<glm::vec3> particlePositions;
std::vector<glm::vec3> particleColors;

glm::vec2 homePos(-10.0f, 0.0f);
glm::vec2 foodPos(10.0f, 0.0f);
float PHEROMONE_DEPOSIT = 1.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0,0,width,height); }
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll((float)yoffset); }
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse) { lastX=xpos; lastY=ypos; firstMouse=false; }
    float xoffset = xpos-lastX;
    float yoffset = -(ypos-lastY);
    lastX=xpos; lastY=ypos;
    if(cameraMode) camera.ProcessMouseMovement(xoffset,yoffset);
}
void processInput(GLFWwindow* window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS) glfwSetWindowShouldClose(window,true);
    if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS) camera.ProcessKeyboard(FORWARD,deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS) camera.ProcessKeyboard(BACKWARD,deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS) camera.ProcessKeyboard(LEFT,deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS) camera.ProcessKeyboard(RIGHT,deltaTime);
    if(glfwGetKey(window, GLFW_KEY_TAB)==GLFW_PRESS && !tabPressed) {
        cameraMode=!cameraMode;
        glfwSetInputMode(window,GLFW_CURSOR,cameraMode?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);
        tabPressed=true;
    }
    if(glfwGetKey(window, GLFW_KEY_TAB)==GLFW_RELEASE) tabPressed=false;
}

void initAnts() {
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> distVel(-0.02f,0.02f);

    for(int i=0;i<NUM_ANTS;i++){
        ants.push_back({homePos, glm::vec2(distVel(rng), distVel(rng)), false});
        particlePositions.push_back(glm::vec3(homePos,0.0f));
        particleColors.push_back(glm::vec3(1.0f,1.0f,0.0f));
    }
}

void updateAnts(float dt) {
    for(auto &ant : ants) {
        glm::vec2 target = ant.carryingFood ? homePos : foodPos;
        glm::vec2 dir = target - ant.pos;

        ant.vel += glm::vec2(((float)rand()/RAND_MAX-0.5f), ((float)rand()/RAND_MAX-0.5f))*0.02f;
        if(glm::length(dir)>0.01f) ant.vel += glm::normalize(dir)*0.01f;

        float speed = glm::length(ant.vel);
        if(speed>0.1f) ant.vel = ant.vel/speed*0.1f;

        ant.pos += ant.vel * dt * 30.0f;
        if(ant.pos.x>20.0f) ant.pos.x=-20.0f;
        if(ant.pos.x<-20.0f) ant.pos.x=20.0f;
        if(ant.pos.y>20.0f) ant.pos.y=-20.0f;
        if(ant.pos.y<-20.0f) ant.pos.y=20.0f;
        int gx = (int)((ant.pos.x+20.0f)/40.0f*GRID_SIZE);
        int gy = (int)((ant.pos.y+20.0f)/40.0f*GRID_SIZE);
        gx=glm::clamp(gx,0,GRID_SIZE-1);
        gy=glm::clamp(gy,0,GRID_SIZE-1);
        /*for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++)
                pheromone[i][j] *= PHEROMONE_DECAY;*/
        pheromone[gx][gy] += PHEROMONE_DEPOSIT;

        if(glm::length(dir)<0.5f) ant.carryingFood = !ant.carryingFood;
    }
    /*for (int i = 0; i < GRID_SIZE; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            pheromone[i][j] *= PHEROMONE_DECAY;*/

    for(int i=0;i<GRID_SIZE;i++)
        for(int j=0;j<GRID_SIZE;j++)
            pheromone[i][j]*=PHEROMONE_DECAY;
    for(size_t i=0;i<ants.size();i++){
        int gx = (int)((ants[i].pos.x+20.0f)/40.0f*GRID_SIZE);
        int gy = (int)((ants[i].pos.y+20.0f)/40.0f*GRID_SIZE);
        gx=glm::clamp(gx,0,GRID_SIZE-1);
        gy=glm::clamp(gy,0,GRID_SIZE-1);
        float p = glm::clamp(pheromone[gx][gy]/10.0f,0.0f,1.0f);
        particleColors[i]=glm::vec3(1.0f,1.0f-p,0.0f);
        particlePositions[i]=glm::vec3(ants[i].pos,0.0f);
    }
    /*for (size_t i = 0; i < ants.size(); i++) {
        int gx = (int)((ants[i].pos.x + 20.0f) / 40.0f * GRID_SIZE);
        int gy = (int)((ants[i].pos.y + 20.0f) / 40.0f * GRID_SIZE);
        gx = glm::clamp(gx, 0, GRID_SIZE - 1);
        gy = glm::clamp(gy, 0, GRID_SIZE - 1);
        float p = glm::clamp(pheromone[gx][gy] / 10.0f, 0.0f, 1.0f);*/
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,6);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Ant Simulation",NULL,NULL);
    if(!window){ std::cout<<"Failed to create GLFW window\n"; glfwTerminate(); return -1;}
    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cerr<<"Failed to initialize GLAD\n"; return -1;}
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io=ImGui::GetIO(); (void)io;
    io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 460");

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);

    Shader pointShader("point.vs","point.fs");

    unsigned int VAO,VBO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
    float pointVertex[]={0.0f,0.0f,0.0f};
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(pointVertex),pointVertex,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    unsigned int instanceVBO,colorVBO;
    glGenBuffers(1,&instanceVBO);
    glGenBuffers(1,&colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1,1);
    glBindBuffer(GL_ARRAY_BUFFER,colorVBO);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2,1);

    initAnts();
    while(!glfwWindowShouldClose(window)){
        float currentFrame=glfwGetTime();
        deltaTime=currentFrame-lastFrame;
        lastFrame=currentFrame;

        processInput(window);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Simulation Controls",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
        ImGui::Text("FPS: %.1f",1.0f/deltaTime);
        ImGui::End();

        updateAnts(deltaTime);
        glBindBuffer(GL_ARRAY_BUFFER,instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,particlePositions.size()*sizeof(glm::vec3),particlePositions.data(),GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,colorVBO);
        glBufferData(GL_ARRAY_BUFFER,particleColors.size()*sizeof(glm::vec3),particleColors.data(),GL_DYNAMIC_DRAW);

        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection=glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH/(float)SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 view=camera.GetViewMatrix();

        pointShader.use();
        pointShader.setMat4("projection",projection);
        pointShader.setMat4("view",view);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_POINTS,0,1,particlePositions.size());

        ImGui::Begin("Poi");
        ImGui::Text("Home: (%.1f, %.1f)", homePos.x, homePos.y);
        ImGui::Text("Food: (%.1f, %.1f)", foodPos.x, foodPos.y);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
