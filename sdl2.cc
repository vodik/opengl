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

const char *vshader_src =
    "#version 140                                    \n"
    "                                                \n"
    "in vec3 in_Color;                               \n"
    "in vec3 in_Position;                            \n"
    "out vec3 ex_Color;                              \n"
    "uniform mat4 MVP;                               \n"
    "                                                \n"
    "void main() {                                   \n"
    "    gl_Position = MVP * vec4(in_Position, 1.0); \n"
    "    ex_Color = in_Color;                        \n"
    "}                                               \n";

const char *fshader_src =
    "#version 140                                    \n"
    "                                                \n"
    "precision mediump float;                        \n"
    "in vec3 ex_Color;                               \n"
    "out vec4 out_Color;                             \n"
    "                                                \n"
    "void main() {                                   \n"
    "    out_Color = vec4(ex_Color, 0.0);            \n"
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

static const std::vector<GLfloat> g_color_buffer_data = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
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
    GLuint id;
    std::vector<GLuint> buffers;
    size_t triangles = 0;

public:
    vbo(const std::vector<GLfloat> &data) {
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        triangles = data.size();
        add(data);
    }

    void add(const std::vector<GLfloat> &data) {
        assert(triangles == data.size());

        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, vector_size(data), data.data(), GL_STATIC_DRAW);
        buffers.push_back(buffer);
    }

    ~vbo() {
        printf("deleting vbo\n");
        glDeleteBuffers(buffers.size(), buffers.data());
        glDeleteVertexArrays(1, &id);
    }

    void draw() const {
        int i = 0;

        for (GLuint buffer : buffers) {
            glEnableVertexAttribArray(i);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glVertexAttribPointer(i++, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
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

    gl::vbo vbo(g_vertex_buffer_data);
    vbo.add(g_color_buffer_data);

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

        program.use();
        program.uniform_set(mvp_id, mvp);

        vbo.draw();
        window.swap();
        SDL_Delay(10);
    }

    return 0;
}
