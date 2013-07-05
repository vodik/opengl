#include <cstdio>
#include <cassert>
#include <memory>
#include <stdexcept>

#include <glm/glm.hpp>
#include <GLES3/gl3.h>

namespace gl {

class texture {
    GLuint id;
    std::unique_ptr<unsigned char> data;

public:
    texture(unsigned width, unsigned height, std::unique_ptr<unsigned char> data) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data.get());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    texture(texture &&other) {
        printf("moving texture\n");
        data = std::move(other.data);
        id = other.id;
        other.id = 0;
    }

    texture &operator=(texture &&other) {
        if (this != &other) {
            printf("moving texture\n");
            data = std::move(other.data);
            id = other.id;
            other.id = 0;
        }

        return *this;
    }

    ~texture() {
        glDeleteTextures(1, &id);
    }

    void bind(GLenum texture) const {
        glActiveTexture(texture);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    const GLuint &get() const { return id; }
};

}

namespace image {

gl::texture from_bitmap(const char *path) {
    /* unsigned char header[54]; */
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    unsigned char header[54];
    FILE *fp = fopen(path, "rb");

    fread(header, 1, 54, fp);

    assert(header[0] == 'B');
    assert(header[1] == 'M');
    assert(*(int *)(header + 0x1E) == 0);
    assert(*(int *)(header + 0x1C) == 24);

    dataPos   = *(unsigned int*)(header + 0x0A);
    imageSize = *(unsigned int*)(header + 0x22);
    width     = *(unsigned int*)(header + 0x12);
    height    = *(unsigned int*)(header + 0x16);

    auto data = std::unique_ptr<unsigned char>(new unsigned char [imageSize]);
    fread(data.get(), 1, imageSize, fp);
    fclose(fp);

    return gl::texture(width, height, std::move(data));
}

}

/* namespace gl { */

/* class texture { */

/* public: */
/*     texture(const image::bitmap &bitmap) { */
/*     } */
/* } */

/* } */
