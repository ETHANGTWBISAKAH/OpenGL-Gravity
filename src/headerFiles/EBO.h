#pragma 1
#include <glad/glad.h>

class EBO {
public:
    unsigned int ID;

    EBO(unsigned int* indices, GLsizeiptr size) {
         // sending the indicies to the EBO!
        glGenBuffers(1, &ID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); 
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    }
    
    void bind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    }
    void unbind() {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};