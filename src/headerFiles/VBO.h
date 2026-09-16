#pragma once
#include <glad/glad.h>
#include <iostream>
class VBO {
public:
    unsigned int ID;
    
    // A VBO is a small package that has said vertices so it is processed fast
    // Think of it as a bag that has the 3 dots which has an ID, weight of the box (size of data)
    // How many items (dots) and ect

    VBO(float* vertices, GLsizeiptr size) {
        glGenBuffers(1, &ID); // Makes 1 buffer ID 
        glBindBuffer(GL_ARRAY_BUFFER, ID); // Selects the VBO as the active buffer
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); // copies the array into the VBO
    }
    void bind() {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }
    void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
};