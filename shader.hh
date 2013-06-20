#ifndef SHADER_H
#define SHADER_H

#include <cstdio>
#include <memory>
#include <stdexcept>

#include <glm/glm.hpp>
#include <GLES3/gl3.h>

namespace gl {

template<class T>
void glUniform(GLint __attribute__((unused)) loc, const T __attribute__((unused)) &value) { }

template<>
void glUniform<glm::mat4>(GLint loc, const glm::mat4 &value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

template<>
void glUniform<GLuint>(GLint loc, const GLuint &value) {
    glUniform1i(loc, value);
}

class shader {
    GLuint id;

public:
    shader(GLenum type, const char *src) {
        id = glCreateShader(type);
        if (id == 0)
            return;

        glShaderSource(id, 1, &src, NULL);
        glCompileShader(id);

        GLint compiled;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint len = 0;

            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
            std::unique_ptr<char> msg(new char[len]);
            glGetShaderInfoLog(id, len, NULL, msg.get());

            glDeleteShader(id);
            throw std::logic_error(msg.get());
        }
    }

    const GLuint &get() const { return id; }
    ~shader() { printf("deleting shader\n"); glDeleteShader(id); }
};

template<class T>
class uniform {
    GLuint id;

public:
    uniform(GLuint u) : id(u) { }
    void set(const T &value) { glUniform(id, value); }
};

class program {
    GLuint id;

public:
    program(const char *vshader_src, const char *fshader_src) {
        id = glCreateProgram();
        if (id == 0)
            return;

        shader vshader(GL_VERTEX_SHADER, vshader_src);
        shader fshader(GL_FRAGMENT_SHADER, fshader_src);

        glAttachShader(id, vshader.get());
        glAttachShader(id, fshader.get());
        glLinkProgram(id);

        GLint linked;
        glGetProgramiv(id, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint len = 0;

            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
            std::unique_ptr<char> msg(new char[len]);
            glGetProgramInfoLog(id, len, NULL, msg.get());

            glDeleteProgram(id);
            throw std::logic_error(msg.get());
        }
    }

    ~program() { printf("deleting program\n"); glDeleteProgram(id); }
    void use() const { glUseProgram(id); }

    template<class T>
    uniform<T> uniform(const char *name) {
        return gl::uniform<T>(glGetUniformLocation(id, name));
    }
};

}

#endif
