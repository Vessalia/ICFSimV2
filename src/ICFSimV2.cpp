#include <iostream>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Constants.h"
#include "DoubleBuffer.h"
#include "Material.h"

Uint32 lastUpdateTime = SDL_GetTicks();
float deltaTime;

SDL_GLContext gContext;
SDL_Window* gWindow = nullptr;

GLuint VAO;

std::unique_ptr<Shader> impulseShader;
std::unique_ptr<Material> impulseMaterial;

std::unique_ptr<DoubleBuffer> velBuffer;

bool init();
void draw(float deltaTime);
void close();
void handleMovement(SDL_Event* event, float dt);

int main()
{
    if (!init())
    {
        std::cout << "Failed to initialize!" << std::endl;
        return 1;
    }

    bool quit = false;
    bool turning = false;
    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }

            if (e.type == SDL_MOUSEMOTION)
            {
                handleMovement(&e, 1 / 60.f);
            }
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        deltaTime = (SDL_GetTicks() - lastUpdateTime) / 1000.f;
        draw(1 / 60.0);
        lastUpdateTime = SDL_GetTicks();

        SDL_GL_SwapWindow(gWindow);
    }

    close();

    return 0;
}

static void GLErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
{
    auto const src_str = [source]()
    {
        switch (source)
        {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        }
        return "";
    }();

    auto const type_str = [type]()
    {
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        }
        return "";
    }();

    auto const severity_str = [severity]()
    {
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        }
        return "";
    }();
    std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize. SDL Error: " << SDL_GetError() << "\n" << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

    gWindow = SDL_CreateWindow("LearnOpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }

    gContext = SDL_GL_CreateContext(gWindow);

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL.\n" << std::endl;
        return false;
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLErrorCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    impulseShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/impulse.frag.glsl");
    impulseMaterial = std::make_unique<Material>(*impulseShader);
    impulseMaterial->pushUniform("invR", 1 / BRUSH_RADIUS);

    velBuffer = std::make_unique<DoubleBuffer>(TEX_RES_X, TEX_RES_Y);

    return true;
}

bool doImpulse = false;
int prevMouseX = 0;
int prevMouseY = 0;
void handleMovement(SDL_Event* event, float dt)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    y = SCREEN_HEIGHT - y;
    if (event->type == SDL_MOUSEMOTION && event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        float dx = x - prevMouseX;
        float dy = y - prevMouseY;
        float valX = (float)x / SCREEN_WIDTH;
        float valY = (float)y / SCREEN_HEIGHT;

        glm::vec2 pos(valX, valY);
        float forceX = std::min(std::max(5.f / SCREEN_WIDTH, -1.f), 1.f);
        float forceY = std::min(std::max(5.f / SCREEN_HEIGHT, -1.f), 1.f);
        glm::vec4 force(forceX, forceY, 0, 0);
        impulseMaterial->pushUniform("impulsePos", pos);
        impulseMaterial->pushUniform("Fdt", force);

        prevMouseX = x;
        prevMouseY = y;
        doImpulse = true;
    }
}

void draw(DoubleBuffer& buffer, Material& material)
{
    buffer.getWrite().use();
    material.use();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void draw(float deltaTime)
{
    // Impulse
    if (doImpulse)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "impulse step");
        impulseMaterial->pushUniform("base", velBuffer->getRead().getFrameTexture());
        draw(*velBuffer, *impulseMaterial);
        velBuffer->swap();
        glPopDebugGroup();

        doImpulse = false;
    }

    velBuffer->getRead().copyToBackbuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void close()
{
    glDeleteVertexArrays(1, &VAO);

    SDL_GL_DeleteContext(gContext);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    IMG_Quit();
}
