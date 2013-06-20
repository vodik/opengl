#include "sdl2.hh"

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GLES3/gl3.h>

#include "vbo.hh"
#include "shader.hh"
#include "texture.hh"

const char *vshader_src =
    "#version 140                                    \n"
    "                                                \n"
    "uniform mat4 MVP;                               \n"
    "                                                \n"
    "in vec2 in_UV;                                  \n"
    "in vec3 in_Position;                            \n"
    "out vec2 UV;                                    \n"
    "                                                \n"
    "void main() {                                   \n"
    "    gl_Position = MVP * vec4(in_Position, 1.0); \n"
    "    UV = in_UV;                                 \n"
    "}                                               \n";

const char *fshader_src =
    "#version 140                                    \n"
    "precision mediump float;                        \n"
    "                                                \n"
    "uniform sampler2D texture;                      \n"
    "                                                \n"
    "in vec2 UV;                                     \n"
    "out vec4 color;                                 \n"
    "                                                \n"
    "void main() {                                   \n"
    "    color = texture2D(texture, UV);             \n"
    "}                                               \n";

static const std::vector<glm::vec3> g_vertex_buffer_data = {
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f)
};

static const std::vector<glm::vec2> g_uv_buffer_data = {
    glm::vec2(0.000059f, 1.0f-0.000004f),
    glm::vec2(0.000103f, 1.0f-0.336048f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(1.000023f, 1.0f-0.000013f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.999958f, 1.0f-0.336064f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.336024f, 1.0f-0.671877f),
    glm::vec2(0.667969f, 1.0f-0.671889f),
    glm::vec2(1.000023f, 1.0f-0.000013f),
    glm::vec2(0.668104f, 1.0f-0.000013f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.000059f, 1.0f-0.000004f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(0.336098f, 1.0f-0.000071f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(0.336024f, 1.0f-0.671877f),
    glm::vec2(1.000004f, 1.0f-0.671847f),
    glm::vec2(0.999958f, 1.0f-0.336064f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.668104f, 1.0f-0.000013f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(0.667979f, 1.0f-0.335851f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(0.668104f, 1.0f-0.000013f),
    glm::vec2(0.336098f, 1.0f-0.000071f),
    glm::vec2(0.000103f, 1.0f-0.336048f),
    glm::vec2(0.000004f, 1.0f-0.671870f),
    glm::vec2(0.336024f, 1.0f-0.671877f),
    glm::vec2(0.000103f, 1.0f-0.336048f),
    glm::vec2(0.336024f, 1.0f-0.671877f),
    glm::vec2(0.335973f, 1.0f-0.335903f),
    glm::vec2(0.667969f, 1.0f-0.671889f),
    glm::vec2(1.000004f, 1.0f-0.671847f),
    glm::vec2(0.667979f, 1.0f-0.335851f)
};

int main(void)
{
    sdl::sdl sdl(3, 1);
    sdl::window window = sdl.new_window("Tutorial");

    SDL_GL_SetSwapInterval(1);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // shader stuff
    gl::program shader(vshader_src, fshader_src);
    auto shader_mvp = shader.uniform<glm::mat4>("MVP");
    auto shader_tex = shader.uniform<GLuint>("texture");

    // VBO stuff
    gl::vbo vbo;
    vbo.add(g_vertex_buffer_data);
    vbo.add(g_uv_buffer_data);

    // texture stuff
    gl::texture texture = image::from_bitmap("uvtemplate.bmp");

    // project stuff
    glm::mat4 projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(4, 3, 3),
                                 glm::vec3(0, 0, 0),
                                 glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    // event loop
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYUP)
                done = true;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture.bind(GL_TEXTURE0);

        shader.use();
        shader_mvp.set(mvp);
        shader_tex.set(0);

        vbo.draw();

        window.swap();
        SDL_Delay(100);
    }

    return 0;
}
