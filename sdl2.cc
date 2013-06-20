#define GL3_PROTOTYPES 1
#include <cstdlib>
#include <cstdio>
#include <vector>

#include <errno.h>
#include <err.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GLES3/gl3.h>

#include "shader.hh"

// An array of 3 vectors which represents 3 vertices
static const std::vector<GLfloat> g_vertex_buffer_data = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
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

namespace sdl {

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

}

namespace gl {

class vbo {
    GLuint id;
    GLuint buffer;

public:
    vbo(const std::vector<GLfloat> &data) {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, vector_size(data), data.data(), GL_STATIC_DRAW);
    }

    ~vbo() {
        printf("deleting vbo\n");
        glDeleteBuffers(1, &buffer);
        glDeleteVertexArrays(1, &id);
    }

    const GLuint &data() const { return buffer; }

    void draw() const {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
    }
};

}

static const gl::program init(void)
{
    const char *vshader_src =
        "#version 100                 \n"
        "uniform mat4 MVP;            \n"
        "attribute vec4 vPosition;    \n"
        "void main() {                \n"
        "    gl_Position = MVP * vPosition; \n"
        "}                            \n";

    const char *fshader_src =
        "#version 100                                 \n"
        "precision mediump float;                     \n"
        "void main() {                                \n"
        "    gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
        "}                                            \n";

    return gl::program(vshader_src, fshader_src);
}

static void draw(gl::program &program, gl::vbo &vbo,
                 glm::mat4 &mvp, int id)
{
    glClear(GL_COLOR_BUFFER_BIT);

    program.use();
    program.uniform_set(id, mvp);

    vbo.draw();
}

int main(void)
{
    sdl::sdl sdl(3, 1);
    sdl::window window("Tutorial");

    SDL_GL_SetSwapInterval(1);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    window.swap();

    glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(4, 3, 3),
                                 glm::vec3(0, 0, 0),
                                 glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    gl::program program = init();
    auto uniform = program.uniform_get("MVP");

    gl::vbo vbo(g_vertex_buffer_data);

    SDL_Event event;
    bool done = false;
    while (!done)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                done = true;
        }

        draw(program, vbo, mvp, uniform);

        window.swap();
        SDL_Delay(10);
    }

    return 0;
}
