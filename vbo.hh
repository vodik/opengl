#ifndef VBO_H
#define VBO_H

#include <cstdio>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <GLES3/gl3.h>

template <class T>
inline size_t vector_size(const std::vector<T> &vec) {
    return sizeof(T) * vec.size();
}

namespace gl {

class vbo {
    struct data {
        GLuint buffer;
        GLint size;
    };

    GLuint id;
    std::vector<struct data> buffers;
    size_t triangles = 0;

    template<class T>
    void append(const std::vector<T> &data, int size) {
        if (triangles == 0)
            triangles = data.size();

        struct data d;
        glGenBuffers(1, &d.buffer);
        glBindBuffer(GL_ARRAY_BUFFER, d.buffer);
        glBufferData(GL_ARRAY_BUFFER, vector_size(data), data.data(), GL_STATIC_DRAW);

        d.size = size;
        buffers.push_back(d);
    }

public:
    vbo() : triangles(0) {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);
    }

    template<class T>
    void add(const std::vector<T> &data) {
        append(data, 1);
    }

    ~vbo() {
        printf("deleting vbo\n");
        for (auto d : buffers) {
            glDeleteBuffers(1, &d.buffer);
        }
        glDeleteVertexArrays(1, &id);
    }

    void draw() const {
        int i = 0;

        for (auto d : buffers) {
            glEnableVertexAttribArray(i);
            glBindBuffer(GL_ARRAY_BUFFER, d.buffer);
            glVertexAttribPointer(i++, d.size, GL_FLOAT, GL_FALSE, 0, nullptr);
        }

        glDrawArrays(GL_TRIANGLES, 0, triangles);
        while (i > -1)
            glDisableVertexAttribArray(i--);
    }
};

template<> void vbo::add<glm::vec2>(const std::vector<glm::vec2> &data) { append(data, 2); }
template<> void vbo::add<glm::vec3>(const std::vector<glm::vec3> &data) { append(data, 3); }

}

#endif
