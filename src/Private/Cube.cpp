#include "Public/Cube.h"
#include <glad/glad.h>

Cube& Cube::GetInstance()
{
    static Cube instance;
    return instance;
}

void Cube::Draw(Shader& Shader)
{
    Shader.Use();
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

Cube::~Cube()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

Cube::Cube()
{
    float vertices[] =
    {
        //    Position       |       Normal         |   Texture
        // back face
        -1.0f, -1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // top-right
        -1.0f, -1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // bottom-left
        -1.0f,  1.0f, -1.0f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // top-left
        // front face                                 
        -1.0f, -1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // bottom-right
         1.0f,  1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // top-right
         1.0f,  1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // top-right
        -1.0f,  1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f, // top-left
        -1.0f, -1.0f,  1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // bottom-left
        // left face                                  
        -1.0f,  1.0f,  1.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-right
        -1.0f,  1.0f, -1.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // top-left
        -1.0f, -1.0f, -1.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  1.0f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-right
        // right face                                 
         1.0f,  1.0f,  1.0f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
         1.0f, -1.0f, -1.0f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
         1.0f,  1.0f, -1.0f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // top-right         
         1.0f, -1.0f, -1.0f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // bottom-right
         1.0f,  1.0f,  1.0f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // top-left
         1.0f, -1.0f,  1.0f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // bottom-left     
        // bottom face                                
        -1.0f, -1.0f, -1.0f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // top-left
         1.0f, -1.0f,  1.0f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // bottom-left
         1.0f, -1.0f,  1.0f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // bottom-left
        -1.0f, -1.0f,  1.0f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // bottom-right
        -1.0f, -1.0f, -1.0f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // top-right
        // top face                                   
        -1.0f,  1.0f, -1.0f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // top-left
         1.0f,  1.0f , 1.0f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // bottom-right
         1.0f,  1.0f, -1.0f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // top-right     
         1.0f,  1.0f,  1.0f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f, -1.0f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // top-left
        -1.0f,  1.0f,  1.0f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f  // bottom-left        
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
