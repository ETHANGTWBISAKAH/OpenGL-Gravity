#pragma once
#include <glad/glad.h>
#include "VBO.h"

// A VAO are the rules that the VBO wants the handler (GPU) to comply.
// Like a package labelled Fragile wants to be handeled carefully

class VAO {
public:
    unsigned int ID;

    VAO() {
        glGenVertexArrays(1, &ID); // Generate VAO ID
    }
    void bind() {
        glBindVertexArray(ID); // Bind VAO first to record configuration
    }
    void unbind() {
        glBindVertexArray(0);
    }
    void linkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
        VBO.bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);
        VBO.unbind();
    }
};


