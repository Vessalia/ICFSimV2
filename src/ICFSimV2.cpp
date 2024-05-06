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

bool doImpulse = false;
bool addInk = false;
int prevMouseX = 0;
int prevMouseY = 0;

SDL_GLContext gContext;
SDL_Window* gWindow = nullptr;

GLuint VAO;

std::unique_ptr<Shader> impulseShader;
std::unique_ptr<Material> impulseMaterial;

std::unique_ptr<Shader> advectShader;
std::unique_ptr<Material> advectMaterial;

std::unique_ptr<Shader> boundaryShader;
std::unique_ptr<Material> boundaryMaterial;

std::unique_ptr<Shader> divShader;
std::unique_ptr<Material> divMaterial;

std::unique_ptr<Shader> psiShader;
std::unique_ptr<Material> psiMaterial;

std::unique_ptr<Shader> gradShader;
std::unique_ptr<Material> gradMaterial;

std::unique_ptr<DoubleBuffer> velBuffer;
std::unique_ptr<DoubleBuffer> inkBuffer;
std::unique_ptr<Framebuffer> divFB;
std::unique_ptr<DoubleBuffer> psiBuffer;

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

            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_LCTRL)
                {
                    addInk = true;
                }
            }
            
            if (e.type == SDL_KEYUP)
            {
                if (e.key.keysym.sym == SDLK_LCTRL)
                {
                    addInk = false;
                }
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    advectShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/advect.frag.glsl");
    advectMaterial = std::make_unique<Material>(*advectShader);
    advectMaterial->pushUniform("invDim", INV_DIM);
    advectMaterial->pushUniform("rdx", 1 / DX);

    boundaryShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/boundary.frag.glsl");
    boundaryMaterial = std::make_unique<Material>(*boundaryShader);

    divShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/div.frag.glsl");
    divMaterial = std::make_unique<Material>(*divShader);
    divMaterial->pushUniform("halfrdx", 0.5f / DX);

    psiShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/jacobi.frag.glsl");
    psiMaterial = std::make_unique<Material>(*psiShader);
    psiMaterial->pushUniform("alpha", -DX * DX);
    psiMaterial->pushUniform("rBeta", 0.25f);

    gradShader = std::make_unique<Shader>("shaders/screen.vert.glsl", "shaders/grad.frag.glsl");
    gradMaterial = std::make_unique<Material>(*gradShader);
    gradMaterial->pushUniform("halfrdx", 0.5f / DX);

    velBuffer = std::make_unique<DoubleBuffer>(TEX_RES_X, TEX_RES_Y);
    inkBuffer = std::make_unique<DoubleBuffer>(TEX_RES_X, TEX_RES_Y);
    divFB = std::make_unique<Framebuffer>(TEX_RES_X, TEX_RES_Y);
    psiBuffer = std::make_unique<DoubleBuffer>(TEX_RES_X, TEX_RES_Y);

    return true;
}

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
        float forceX = std::min(std::max(dx / SCREEN_WIDTH, -1.f), 1.f);
        float forceY = std::min(std::max(dy / SCREEN_HEIGHT, -1.f), 1.f);
        glm::vec4 force(forceX, forceY, 0, 0);
        impulseMaterial->pushUniform("impulsePos", pos);
        impulseMaterial->pushUniform("Fdt", force);

        prevMouseX = x;
        prevMouseY = y;
        doImpulse = true;
    }
}

void draw(Framebuffer& framebuffer, Material& material)
{
    framebuffer.use();
    material.use();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void draw(DoubleBuffer& buffer, Material& material)
{
    draw(buffer.getWrite(), material);
}

void draw(float deltaTime)
{
    // Advect
    boundaryMaterial->pushUniform("scale", -1);
    boundaryMaterial->pushUniform("x", velBuffer->getRead().getFrameTexture());
    draw(*velBuffer, *boundaryMaterial);
    velBuffer->swap();

    advectMaterial->pushUniform("u", velBuffer->getRead().getFrameTexture());
    advectMaterial->pushUniform("x", velBuffer->getRead().getFrameTexture());
    advectMaterial->pushUniform("dissipation", VELOCITY_DISSIPATION);
    advectMaterial->pushUniform("dt", deltaTime);
    draw(*velBuffer, *advectMaterial);
    velBuffer->swap();

    boundaryMaterial->pushUniform("scale", 0);
    boundaryMaterial->pushUniform("x", inkBuffer->getRead().getFrameTexture());
    draw(*inkBuffer, *boundaryMaterial);
    inkBuffer->swap();

    advectMaterial->pushUniform("u", velBuffer->getRead().getFrameTexture());
    advectMaterial->pushUniform("x", inkBuffer->getRead().getFrameTexture());
    advectMaterial->pushUniform("dissipation", INK_LONGEVITY);
    advectMaterial->pushUniform("dt", deltaTime);
    draw(*inkBuffer, *advectMaterial);
    inkBuffer->swap();

    // Impulse
    if (doImpulse)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "impulse step");
        impulseMaterial->pushUniform("base", velBuffer->getRead().getFrameTexture());
        draw(*velBuffer, *impulseMaterial);
        velBuffer->swap();

        if (addInk)
        {
            impulseMaterial->pushUniform("base", inkBuffer->getRead().getFrameTexture());
            impulseMaterial->pushUniform("Fdt", INK_COLOUR);
            draw(*inkBuffer, *impulseMaterial);
            inkBuffer->swap();
        }

        doImpulse = false;
        glPopDebugGroup();
    }

    divMaterial->pushUniform("map", velBuffer->getRead().getFrameTexture());
    draw(*divFB, *divMaterial);

    boundaryMaterial->pushUniform("scale", 1);
    psiMaterial->pushUniform("b", divFB->getFrameTexture());

    for (int i = 0; i < NUM_POISSON_STEPS; ++i)
    {
        boundaryMaterial->pushUniform("x", psiBuffer->getRead().getFrameTexture());
        draw(*psiBuffer, *boundaryMaterial);
        psiBuffer->swap();

        psiMaterial->pushUniform("x", psiBuffer->getRead().getFrameTexture());
        draw(*psiBuffer, *psiMaterial);
        psiBuffer->swap();
    }

    boundaryMaterial->pushUniform("x", velBuffer->getRead().getFrameTexture());
    boundaryMaterial->pushUniform("scale", -1);
    draw(*velBuffer, *boundaryMaterial);
    velBuffer->swap();

    gradMaterial->pushUniform("mapP", psiBuffer->getRead().getFrameTexture());
    gradMaterial->pushUniform("mapW", velBuffer->getRead().getFrameTexture());
    draw(*velBuffer, *gradMaterial);
    velBuffer->swap();

    velBuffer->getRead().copyToBackbuffer(glm::vec4(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT));
    inkBuffer->getRead().copyToBackbuffer(glm::vec4(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT));
    divFB->copyToBackbuffer(glm::vec4(0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
    psiBuffer->getRead().copyToBackbuffer(glm::vec4(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2));
}

void close()
{
    glDeleteVertexArrays(1, &VAO);

    SDL_GL_DeleteContext(gContext);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    IMG_Quit();
}
