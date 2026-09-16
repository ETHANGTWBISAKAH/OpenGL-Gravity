#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <random>

#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "glm/glm/glm.hpp"
#include "headerFiles/Shader.h"
#include "headerFiles/VAO.h"
#include "headerFiles/VBO.h"
#include "headerFiles/EBO.h"
using namespace std;
float pi = 3.14159265359;

GLFWwindow* window;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec4 color;
    std::vector<glm::vec3> history;
    float scale;
    float mass;
    float life;
    float maxlife; // Starting total life time
};
float get_random(float min, float max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(min, max); // distrib is a variable
    return distrib(gen);
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    float cameraSpeed = 2.5f * deltaTime;
    // Up down
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    // Left right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

}
float yaw = -90.0f;   // Initialized to -90 degrees so the camera starts facing down the -Z axis
float pitch =  0.0f;  // Horizon level
float lastX =  400.0f;// Center of an 800x600 window
float lastY =  300.0f;
bool firstMouse = true;
void mouseCallBack(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Gk mudeng
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }
}
int main() {
    glfwInit();

    // Version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Use core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window
    float width = 800.0f;
    float height = 600.0f;
    window = glfwCreateWindow(width, height, "OpenGL 3D", NULL, NULL);
    float aspectRatio = width/height;
    if (window == NULL) {
        cout << "Terminated. Failed to create a window." << endl;
        glfwTerminate();
        return -1;
    }

    // Connect
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Terminared. Failed to connect GLAD." << endl;
        return -1;
    }

    // Dots
    float vertices[] = {
        // Front face
        0.1f, 0.1f, 0.1f, 
        0.1f, -0.1f, 0.1f, 
        -0.1f, -0.1f, 0.1f, 
        -0.1f, 0.1f, 0.1f, 

        // Back Face
        0.1f, 0.1f, -0.1f, 
        0.1f, -0.1f, -0.1f, 
        -0.1f, -0.1f, -0.1f, 
        -0.1f, 0.1f, -0.1f, 
    };

    unsigned int indices[] = {
        0, 1, 3, 1, 2, 3, // Front face
        4, 5, 0, 5, 1, 0, // Right face
        7, 6, 4, 6, 5, 4, // Back face
        3, 2, 7, 2, 6, 7, // Left face
        4, 0, 7, 0, 3, 7, // Top face
        1, 5, 2, 5, 6, 2  // Bottom face
    };

    VAO rectVAO;
    rectVAO.bind();

    VBO rectVBO(vertices, sizeof(vertices));
    EBO rectEBO(indices, sizeof(indices));

    rectVAO.linkAttrib(rectVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
    rectVAO.unbind();  

    std::vector<Particle> particles;
    int numParticles = 100;

    Particle blackHole;
    blackHole.velocity = glm::vec3(0.0f);
    blackHole.acceleration = glm::vec3(0.0f);
    blackHole.position = glm::vec3(0.0f,0.0f,0.0f);
    blackHole.color = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    blackHole.mass = 1000.0f;
    blackHole.scale = 5.0f;
    particles.push_back(blackHole);

    // Acceleration
    const float G = 1.0f;
    const float softening = 0.05f;
    const float mass = 1.0f;


    for (int i = 1; i < numParticles; i++) {
        Particle p;

         // Disc
        float ranDistance = 20.0f;
        float ranAngle = get_random(0, 2*pi);
        float effectiveMass = blackHole.mass + (particles.size() * p.mass);
        float v = glm::sqrt(G*effectiveMass/ranDistance);

        glm::vec3 polarCoordinates = glm::vec3(ranDistance * cos(ranAngle), 0.2f, ranDistance * sin(ranAngle));
        glm::vec3 polarPerpendicular = glm::vec3(-1*sin(ranAngle) * ranDistance, 0, cos(ranAngle) * ranDistance);
        

        glm::vec3 directionPerpendicular = glm::normalize(polarPerpendicular);

        glm::vec3 vTotal = directionPerpendicular * v;

        float maxSpeed = 1.0f;
        float heatFactor = glm::clamp(v / maxSpeed, 0.0f, 1.0f);

        p.position = polarCoordinates;
        p.life = get_random(1.0f, 3.0f);
        p.maxlife = p.life;

        p.scale = get_random(1.0f, 2.0f);;
        p.mass = 0.5f;
        p.color = glm::vec4(heatFactor, 0.0f, 1.0f - heatFactor, 1.0f);
        p.velocity = vTotal;
        p.history.push_back(p.position);
        
        particles.push_back(p);
    }   

    unsigned int instancedVBO;
    glGenBuffers(1, &instancedVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    float maxInstances = particles.size() * 16;
    glBufferData(GL_ARRAY_BUFFER, maxInstances * 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // Location 1: position (3vec)
    rectVAO.bind();
    glEnableVertexAttribArray(1); // Janlup!
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glVertexAttribDivisor(1, 1);
    
    // location 2: COlor (4 vec)
    glEnableVertexAttribArray(2); // Ini juga!
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(2, 1);

    // Location 3: scale (1 vec) 
    glEnableVertexAttribArray(3); // Ini juga!
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    rectVAO.unbind();

    std::vector<float> instancedData;
    instancedData.reserve(particles.size() * 20);
    float deltaTime;

    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instancedData.size() * sizeof(float), instancedData.data());
    // Shader

    const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aInstancedPos;\n"
    "layout (location = 2) in vec4 aInstancedColor;\n"
    "layout (location = 3) in float aInstancedScale;\n" // vec1 itu float saja
    "out vec4 particleColor;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "   particleColor = aInstancedColor;\n"
    "   gl_Position = projection * view * model * vec4((aPos.x * aInstancedScale) + aInstancedPos.x, (aPos.y * aInstancedScale) + aInstancedPos.y, (aPos.z * aInstancedScale) + aInstancedPos.z, 1.0);\n"
    "}\n";

    const char* fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 particleColor;\n"
    "void main() {"
    "   FragColor = particleColor;\n"
    "}\n";

    Shader shaderProgram(vertexShaderSource, fragmentShaderSource);

    
    int modelLocation = glGetUniformLocation(shaderProgram.ID, "model");
    int viewLocation = glGetUniformLocation(shaderProgram.ID, "view");
    int projectionLocation = glGetUniformLocation(shaderProgram.ID, "projection");

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    //Alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallBack);

    float lastFrame = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        
        // Clear with black
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // GK mudeng
        // color -> screen, depth -> forget z axis
        // Delta time
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        if (deltaTime > 0.05f) deltaTime = 0.016f;

        lastFrame = currentFrame;

        // Shader
        shaderProgram.use();

        instancedData.clear();

        
        for (Particle& p : particles) {
            p.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        for (Particle& p : particles) {
            for (Particle& j : particles) {
                if (&p == &j) {
                    continue;
                }
                // N body
                glm::vec3 displacementR = j.position - p.position;
                float distanceR = glm::length(displacementR);

                if (distanceR < 0.0001f) continue; // Safeguard! So no errors

                float softenedR = distanceR * distanceR + softening * softening;
                glm::vec3 direction = glm::normalize(displacementR);

                // Accumulate acceleration
                float a = G * j.mass / softenedR;

                p.acceleration += direction * a;
            }
        }


        for (Particle& p : particles) {
            p.life -= deltaTime * 0.5f;
            float colorFade = p.life/p.maxlife;
            
            // Position 
            if (&p != &particles[0]) {
                for (int h = 0; h < p.history.size(); h++) {
                    float fade = (float)h / p.history.size();
                    p.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.05f);
                    if (fade <= 0.8f) {
                        fade = 0.0f;
                    }

                    instancedData.push_back(p.history[h].x);
                    instancedData.push_back(p.history[h].y);
                    instancedData.push_back(p.history[h].z);

                    
                    instancedData.push_back(p.color.r);
                    instancedData.push_back(p.color.g);
                    instancedData.push_back(p.color.b);
                    instancedData.push_back(p.color.a * fade);

                    instancedData.push_back(p.scale * fade);

                }

                p.velocity += p.acceleration * deltaTime;
                p.position += p.velocity * deltaTime;
                float speed = glm::length(p.velocity);
                float heatFactor = glm::clamp(speed / 1.0f, 0.0f, 1.0f);
                p.color = glm::vec4(heatFactor, 0.0f, 1.0f - heatFactor, 1.0f);
                p.history.push_back(p.position);

                // Trail effect
                if (p.history.size() > 15) {
                    p.history.erase(p.history.begin()); // Deletes the oldest entry at index 0
                }

                
                
                // Life
                if (p.position.x > 100.0f || p.position.x < -100.0f) {
                    p.life = 0;
                } else if (p.position.y > 100.0f || p.position.y < -100.0f) {
                    p.life = 0;
                } 
                

                if (p.life <= 0) {
                    
                    float ranDistance = 20.0f;
                    float ranAngle = get_random(0, 2*pi);

                    float v = glm::sqrt(G*blackHole.mass/ranDistance);

                    glm::vec3 polarCoordinates = glm::vec3(ranDistance * cos(ranAngle), 0.2f, ranDistance * sin(ranAngle));
                    glm::vec3 polarPerpendicular = glm::vec3(-1*sin(ranAngle) * ranDistance, 0, cos(ranAngle) * ranDistance);
                    

                    glm::vec3 directionPerpendicular = glm::normalize(polarPerpendicular);

                    glm::vec3 vTotal = directionPerpendicular * v;
                    p.position = polarCoordinates;
                    p.velocity = vTotal;
                    p.life = p.maxlife;
                    
                    p.history.clear();
                    }
                p.color.a = colorFade;
            }
            
            
            instancedData.push_back(p.position.x);
            instancedData.push_back(p.position.y);
            instancedData.push_back(p.position.z);
            instancedData.push_back(p.color.r);
            instancedData.push_back(p.color.g);
            instancedData.push_back(p.color.b);
            instancedData.push_back(p.color.a);
            instancedData.push_back(p.scale);
        }
        // Sends data!
        glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, instancedData.size() * sizeof(float), instancedData.data());
        
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) *  cos(glm::radians(pitch));

        cameraFront = glm::normalize(front);
        // Rotate
        // Camera from where?
        // glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.1f, 5.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // How close you are relative to the object/cube
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        // Radians(45) is the FOV

        // Passes to the gpu
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        rectVAO.bind();
        // vertices!
        int totalInstances = instancedData.size() / 8;
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, totalInstances);

        glfwSwapBuffers(window);
        glfwPollEvents();   

    }
    glfwTerminate();
    return 0;

}