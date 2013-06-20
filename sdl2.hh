#ifndef SDL_H
#define SDL_H

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

namespace sdl {

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

    window(window &&other) {
        printf("moving window\n");
        ptr = other.ptr;
        glcontext = other.glcontext;
        other.ptr = nullptr;
    }

    window &operator=(window &&other) {
        if (this != &other) {
            printf("moving window\n");
            ptr = other.ptr;
            glcontext = other.glcontext;
            other.ptr = nullptr;
        }

        return *this;
    }

    ~window() {
        printf("sdl cleanup\n");
        if (ptr) {
            SDL_GL_DeleteContext(glcontext);
            SDL_DestroyWindow(ptr);
        }
    }

    void swap() const { SDL_GL_SwapWindow(ptr); }
    SDL_Window *get() const { return ptr; }
};

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

    window new_window(const char *title) { return std::move(window(title)); }
};

}

#endif
