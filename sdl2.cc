#define GL3_PROTOTYPES 1
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <vector>

#include <errno.h>
#include <err.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GLES3/gl3.h>

#include "shader.hh"
#include "texture.hh"

const char *vshader_src =
    "#version 140                                    \n"
    "                                                \n"
    "in vec2 in_UV;                                  \n"
    "in vec3 in_Position;                            \n"
    "out vec2 UV;                                    \n"
    "                                                \n"
    "uniform mat4 MVP;                               \n"
    "                                                \n"
    "void main() {                                   \n"
    "    gl_Position = MVP * vec4(in_Position, 1.0); \n"
    "    UV = in_UV;                                 \n"
    "}                                               \n";

const char *fshader_src =
    "#version 140                                    \n"
    "precision mediump float;                        \n"
    "                                                \n"
    "in vec2 UV;                                     \n"
    "out vec4 color;                                 \n"
    "                                                \n"
    "uniform sampler2D tex;                          \n"
    "                                                \n"
    "void main() {                                   \n"
    "    color = texture2D(tex, UV);                 \n"
    "}                                               \n";

static const std::vector<GLfloat> g_vertex_buffer_data = {
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
};

static const std::vector<GLfloat> g_uv_buffer_data = {
    0.000059f, 1.0f-0.000004f,
    0.000103f, 1.0f-0.336048f,
    0.335973f, 1.0f-0.335903f,
    1.000023f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.336024f, 1.0f-0.671877f,
    0.667969f, 1.0f-0.671889f,
    1.000023f, 1.0f-0.000013f,
    0.668104f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.000059f, 1.0f-0.000004f,
    0.335973f, 1.0f-0.335903f,
    0.336098f, 1.0f-0.000071f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.336024f, 1.0f-0.671877f,
    1.000004f, 1.0f-0.671847f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.668104f, 1.0f-0.000013f,
    0.335973f, 1.0f-0.335903f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.668104f, 1.0f-0.000013f,
    0.336098f, 1.0f-0.000071f,
    0.000103f, 1.0f-0.336048f,
    0.000004f, 1.0f-0.671870f,
    0.336024f, 1.0f-0.671877f,
    0.000103f, 1.0f-0.336048f,
    0.336024f, 1.0f-0.671877f,
    0.335973f, 1.0f-0.335903f,
    0.667969f, 1.0f-0.671889f,
    1.000004f, 1.0f-0.671847f,
    0.667979f, 1.0f-0.335851f
};

/* {{{ ERROR HANDLING */
static void sdldie(const char *fmt, ...)
{
    const char *sdlerror = SDL_GetError();
    char *msg = nullptr;
    va_list ap;

    va_start(ap, fmt);
    vasprintf(&msg, fmt, ap);
    va_end(ap);

    fprintf(stderr, "%s: %s: %s\n",
            program_invocation_short_name, msg, sdlerror);

    SDL_Quit();
    exit(EXIT_FAILURE);
}

static void sdlcheck(const char *fmt, ...)
{
    const char *sdlerror = SDL_GetError();
    if (sdlerror[0] != '\0') {
        char *msg = nullptr;
        va_list ap;

        va_start(ap, fmt);
        vasprintf(&msg, fmt, ap);
        va_end(ap);

        fprintf(stderr, "%s: %s: %s\n",
                program_invocation_short_name, msg, sdlerror);

        free(msg);
        SDL_ClearError();
    }
}
/* }}} */

template <class T>
inline size_t vector_size(const std::vector<T> &vec) {
    return sizeof(T) * vec.size();
}

namespace sdl { /* {{{ */

class sdl {
public:
    sdl(int major, int minor) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            sdldie("unable to initialize sdl");

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    }

    ~sdl() {
        printf("sdl quitting\n");
        SDL_Quit();
    }
};

class window {
    SDL_Window *ptr;
    SDL_GLContext glcontext;

public:
    window(const char *title) {
        ptr = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, 512, 512,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!ptr)
            sdldie("unable to create window");

        glcontext = SDL_GL_CreateContext(ptr);
        sdlcheck("error creating context");
    }

    ~window() {
        printf("sdl cleanup\n");
        SDL_GL_DeleteContext(glcontext);
        SDL_DestroyWindow(ptr);
    }

    void swap() const { SDL_GL_SwapWindow(ptr); }
    SDL_Window *get() const { return ptr; }
};

} /* }}} */

namespace gl { /* {{{ */

class vbo {
    struct data {
        GLuint buffer;
        GLint size;
    };

    GLuint id;
    std::vector<struct data> buffers;
    size_t triangles = 0;

public:
    vbo(const std::vector<GLfloat> &data, int size) {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        triangles = data.size() / size;
        add(data, size);
    }

    void add(const std::vector<GLfloat> &data, int size) {
        assert(triangles == data.size() / size);

        struct data d;
        glGenBuffers(1, &d.buffer);
        glBindBuffer(GL_ARRAY_BUFFER, d.buffer);
        glBufferData(GL_ARRAY_BUFFER, vector_size(data), data.data(), GL_STATIC_DRAW);

        d.size = size;
        buffers.push_back(d);
    }

    ~vbo() {
        printf("deleting vbo\n");
        /* glDeleteBuffers(buffers.size(), buffers.data()); */
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

} /* }}} */

int main(void)
{
    sdl::sdl sdl(3, 1);
    sdl::window window("Tutorial");

    SDL_GL_SetSwapInterval(1);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    gl::program program(vshader_src, fshader_src);
    int mvp_id = program.uniform_get("MVP");
    int tex_id = program.uniform_get("tex");

    gl::vbo vbo(g_vertex_buffer_data, 3);
    vbo.add(g_uv_buffer_data, 2);

    image::bitmap texture("uvtemplate.bmp");

    glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(4, 3, 3),
                                 glm::vec3(0, 0, 0),
                                 glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYUP)
                done = true;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture.bind();

        program.use();
        program.uniform_set(mvp_id, mvp);
        program.uniform_set(tex_id, 0);

        vbo.draw();
        window.swap();
        SDL_Delay(10);
    }

    return 0;
}
