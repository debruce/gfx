//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>
// #include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

typedef struct Vertex
{
    glm::vec3 pos;
    glm::vec3 col;
} Vertex;

static const char* vertex_shader_text = R"(
#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 vCol;
in vec3 vPos;
out vec3 color;

void main()
{
    gl_Position = projection * view * model * vec4(vPos, 1.0);
    color = vCol;
}

)";

static const char* fragment_shader_text = R"(
#version 330

in vec3 color;
out vec4 fragment;
void main()
{
    fragment = vec4(color, 1.0);
}

)";

class ShaderProgram {
public:
    GLuint program;
    
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;

    GLint vpos_location;
    GLint vcol_location;
public:
    ShaderProgram()
    {
        const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        modelLoc = glGetUniformLocation(program, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projectionLoc = glGetUniformLocation(program, "projection");

        vpos_location = glGetAttribLocation(program, "vPos");
        vcol_location = glGetAttribLocation(program, "vCol");
    }

    void use()
    {
        glUseProgram(program);
    }

    void setModel(const glm::mat4& model)
    {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    }

    void setView(const glm::mat4& view)
    {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }

    void setProjection(const glm::mat4& projection)
    {
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }

    void setVertexStream()
    {
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));        
    }
};

class VertexBuffer {
    GLuint buffer;
    GLuint array;
    GLuint vpos_location;
    GLuint vcol_location;
    GLuint type;
    size_t len;
public:
    VertexBuffer(const std::vector<Vertex>& points, const GLuint& type, const size_t& len, const GLuint& vpos_location, const GLuint& vcol_location)
        : type(type), len(len), vpos_location(vpos_location), vcol_location(vcol_location)
    {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glGenVertexArrays(1, &array);
        glBindVertexArray(array);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));
    }

    void paint()
    {
        glBindVertexArray(array);
        glDrawArrays(type, 0, len);
    }
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    using namespace std;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);


    // glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vertex), points.data(), GL_STATIC_DRAW);

    // GLuint vertex_array;
    // glGenVertexArrays(1, &vertex_array);
    // glBindVertexArray(vertex_array);

    ShaderProgram shaderProgram;

    std::vector<Vertex> points;
    const float inner = .5;
    const size_t loop_count = 16;
    for (auto i = 0; i <= loop_count; i++) {
        auto angle = M_PI * 2.0 * i / loop_count;
        auto x = cos(angle);
        auto y = sin(angle);
        points.push_back(Vertex{{x, y, -.2f}, {float(i)/loop_count, 1.0, 1.0 - (float(i)/loop_count)}});
        points.push_back(Vertex{{x, y, .2f}, {float(i)/loop_count, 1.0, 1.0 - (float(i)/loop_count)}});
    }
    VertexBuffer loop(points, GL_TRIANGLE_STRIP, points.size(), shaderProgram.vpos_location, shaderProgram.vcol_location);


    std::vector<Vertex> lines;
    lines.push_back(Vertex{{-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    lines.push_back(Vertex{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});

    lines.push_back(Vertex{{0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    lines.push_back(Vertex{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});

    lines.push_back(Vertex{{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}});
    lines.push_back(Vertex{{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    VertexBuffer axes(lines, GL_LINES, 6, shaderProgram.vpos_location, shaderProgram.vcol_location);

    glm::vec3 white{1.0f, 1.0f, 1.0f};
    std::vector<Vertex> box_points;
    box_points.push_back(Vertex{{-1.0f, -1.0f, -1.0f}, white});
    box_points.push_back(Vertex{{-1.0f, +1.0f, -1.0f}, white});
    box_points.push_back(Vertex{{-1.0f, +1.0f, +1.0f}, white});
    box_points.push_back(Vertex{{-1.0f, -1.0f, +1.0f}, white});
    box_points.push_back(Vertex{{-1.0f, -1.0f, -1.0f}, white});

    box_points.push_back(Vertex{{+1.0f, -1.0f, -1.0f}, white});
    box_points.push_back(Vertex{{+1.0f, +1.0f, -1.0f}, white});
    box_points.push_back(Vertex{{+1.0f, +1.0f, +1.0f}, white});
    box_points.push_back(Vertex{{+1.0f, -1.0f, +1.0f}, white});
    box_points.push_back(Vertex{{+1.0f, -1.0f, -1.0f}, white});
    VertexBuffer box(box_points, GL_LINE_STRIP, 10, shaderProgram.vpos_location, shaderProgram.vcol_location);

    std::vector<Vertex> xy_points;
    for (size_t i = 0; i <= 128; i++) {
        auto angle = (M_PI * 2.0 * float(i)) / 128.0;
        xy_points.push_back(Vertex{{cos(angle), sin(angle), .0f}, {1.0f, 1.0f, 0.0f}});        
    }
    VertexBuffer xy_circle(xy_points, GL_LINE_STRIP, 129, shaderProgram.vpos_location, shaderProgram.vcol_location);

    std::vector<Vertex> yz_points;
    for (size_t i = 0; i <= 128; i++) {
        auto angle = (M_PI * 2.0 * float(i)) / 128.0;
        yz_points.push_back(Vertex{{.0f, cos(angle), sin(angle)}, {0.0f, 1.0f, 1.0f}});        
    }
    VertexBuffer yz_circle(yz_points, GL_LINE_STRIP, 129, shaderProgram.vpos_location, shaderProgram.vcol_location);

    std::vector<Vertex> xz_points;
    for (size_t i = 0; i <= 128; i++) {
        auto angle = (M_PI * 2.0 * float(i)) / 128.0;
        xz_points.push_back(Vertex{{cos(angle), 0.0f, sin(angle)}, {1.0f, 0.0f, 1.0f}});        
    }
    VertexBuffer xz_circle(xz_points, GL_LINE_STRIP, 129, shaderProgram.vpos_location, shaderProgram.vcol_location);

    shaderProgram.setVertexStream();

    glm::mat4 view = glm::lookAt(
        glm::vec3({-0.5f, -0.5f, -4.0f}),
        glm::vec3({0.0f, 0.0f, 0.0f}),
        glm::vec3({0.0f, 1.0f, 0.0f})
            );

    while (!glfwWindowShouldClose(window))
    {
        using namespace std;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glm::mat4 projection = glm::perspectiveFov(3.1416f*.333f, 1.0, 1.0, .1f, 10.0f);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.use();

  
        shaderProgram.setView(view);

        shaderProgram.setProjection(projection);

        shaderProgram.setModel(glm::mat4(1.0f));
        axes.paint();
        box.paint();
        // xy_circle.paint();
        // yz_circle.paint();
        // xz_circle.paint();

        auto model = glm::rotate(glm::mat4(1.0f), (GLfloat)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        shaderProgram.setModel(model);
        // axes.paint();
        loop.paint();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
