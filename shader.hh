#ifndef SHADER_H
#define SHADER_H

#include <cstdio>
#include <glm/glm.hpp>
#include <GLES3/gl3.h>

namespace gl {

class entity {
protected:
    GLuint id;

public:
    const GLuint &get() const { return id; }
};

class shader : public entity {
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
            if (len > 1) {
                char *msg = new char[len];
                glGetShaderInfoLog(id, len, NULL, msg);

                fprintf(stderr, "error compiling shader: %s\n", msg);
                delete [] msg;
            }

            glDeleteShader(id);
        }
    }

    ~shader() { printf("deleting shader\n"); glDeleteShader(id); }
};

template<class T>
void glUniform(GLint loc, T &value) { }

template<>
void glUniform<glm::mat4>(GLint loc, glm::mat4 &value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
}

class program : public entity {
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
            if (len > 1) {
                char *msg = new char[len];
                glGetProgramInfoLog(id, len, NULL, msg);

                fprintf(stderr, "error linking program: %s\n", msg);
            }

            glDeleteProgram(id);
        }
    }

    ~program() { printf("deleting program\n"); glDeleteProgram(id); }
    void use() const { glUseProgram(id); }

    int uniform_get(const char *name) {
        return glGetUniformLocation(id, name);
    }

    template<class T>
    void uniform_set(int id, T &value) {
        glUniform(id, value);
    }
};

}

#endif
