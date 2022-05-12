#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "Defines.hpp"
#include "Helpers.hpp"

constexpr uint32_t VIEWER_WIDTH = 500u;
constexpr uint32_t VIEWER_HEIGHT = 500u;

enum
{
    PROGRAM_DEFAULT = 0,
    PROGRAM_SSQ     = 1, // Screen Space Quad
    PROGRAM_QUAD    = 2,
    PROGRAM_COUNT
};

enum
{
    FRAMEBUFFER_DEFAULT = 0,
    FRAMEBUFFER_COUNT
};

enum
{
    TEXTURE_RENDER_TARGET = 0,
    TEXTURE_BACKBUFFER    = 1,
    TEXTURE_COUNT
};

enum
{
    VERTEXARRAY_TRIANGLE = 0,
    VERTEXARRAY_QUAD     = 1,
    VERTEXARRAY_COUNT
};

enum
{
    BUFFER_TRIANGLE_VERTEX = 0,
    BUFFER_QUAD            = 1,
    BUFFER_COUNT
};

enum
{
    ATTRIB_POS = 1,
    ATTRIB_NORM = 2,
    ATTRIB_UV = 4,
};

struct AppManager
{
    int blendEq = 0;
    float clearColor[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    bool showAlpha = false;
} g_app;

struct OpenGLManager
{
    GLuint programs[PROGRAM_COUNT];
    GLuint framebuffers[FRAMEBUFFER_COUNT];
    GLuint textures[TEXTURE_COUNT];
    GLuint vertexArrays[VERTEXARRAY_COUNT];
    GLuint buffers[BUFFER_COUNT];
} g_gl;

void createVao(GLuint &vaoHandle, GLuint &vboHandle, size_t size, const void *data, uint16_t attribMask)
{
    glGenVertexArrays(1, &vaoHandle);
    glGenBuffers(1, &vboHandle);

    glBindVertexArray(vaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(12));

    if (attribMask & ATTRIB_POS)
    {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)(0));
    }

    size_t stride = (attribMask & ATTRIB_POS) ? sizeof(float) * 3 : 0;
    stride += (attribMask & ATTRIB_NORM)       ? sizeof(float) * 3 : 0;
    stride += (attribMask & ATTRIB_UV)         ? sizeof(float) * 2 : 0;

    size_t currentAttribIdx = 0;
    size_t offset = 0;

    if (attribMask & ATTRIB_POS)
    {
        glEnableVertexAttribArray(currentAttribIdx);
        glVertexAttribPointer(currentAttribIdx, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
        offset += sizeof(float) * 3;
        currentAttribIdx++;
    }

    if (attribMask & ATTRIB_NORM)
    {
        glEnableVertexAttribArray(currentAttribIdx);
        glVertexAttribPointer(currentAttribIdx, 3, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
        offset += sizeof(float) * 3;
        currentAttribIdx++;
    }

    if (attribMask & ATTRIB_UV)
    {
        glEnableVertexAttribArray(currentAttribIdx);
        glVertexAttribPointer(currentAttribIdx, 2, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
        offset += sizeof(float) * 2;
        currentAttribIdx++;
    }

    glBindVertexArray(0u);
    glBindBuffer(GL_ARRAY_BUFFER, 0u);
}

void init()
{
    g_gl.programs[PROGRAM_DEFAULT] = createProgram("../src/shaders/shader.vert", "../src/shaders/shader.frag",
                                                   "default-shader-program");

    g_gl.programs[PROGRAM_SSQ] = createProgram("../src/shaders/ssq.vert", "../src/shaders/ssq.frag",
                                               "ssq-shader-program");

    g_gl.programs[PROGRAM_QUAD] = createProgram("../src/shaders/quad.vert", "../src/shaders/quad.frag",
                                               "quad-shader-program");

    constexpr std::array<float, 9> vertices{
        -0.5f,
        -0.5f,
        0.0f,
        0.5f,
        -0.5f,
        0.0f,
        0.0f,
        0.5f,
        0.0f,
    };

    createVao(g_gl.vertexArrays[VERTEXARRAY_TRIANGLE], g_gl.buffers[BUFFER_TRIANGLE_VERTEX],
              sizeof(float) * vertices.size(), (void *)vertices.data(), ATTRIB_POS);

    glGenFramebuffers(1, &g_gl.framebuffers[FRAMEBUFFER_DEFAULT]);
    glBindFramebuffer(GL_FRAMEBUFFER, g_gl.framebuffers[FRAMEBUFFER_DEFAULT]);

    glGenTextures(1, &g_gl.textures[TEXTURE_RENDER_TARGET]);
    glBindTexture(GL_TEXTURE_2D, g_gl.textures[TEXTURE_RENDER_TARGET]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VIEWER_WIDTH, VIEWER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_gl.textures[TEXTURE_RENDER_TARGET], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        EXIT("Invalid framebuffer object!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };

    createVao(g_gl.vertexArrays[VERTEXARRAY_QUAD], g_gl.buffers[BUFFER_QUAD],
        sizeof(quadVertices), (void*)quadVertices, ATTRIB_POS | ATTRIB_UV);
}

void render()
{
    // First Pass
    if (g_app.showAlpha)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, g_gl.framebuffers[FRAMEBUFFER_DEFAULT]);
    }

    glClearColor(g_app.clearColor[0], g_app.clearColor[1], g_app.clearColor[2], g_app.clearColor[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    if (g_app.blendEq == 0)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    else
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(g_gl.programs[PROGRAM_DEFAULT]);
    glBindVertexArray(g_gl.vertexArrays[VERTEXARRAY_TRIANGLE]);

    set_uni_vec3(g_gl.programs[PROGRAM_DEFAULT], "u_translation", {-0.3f, 0.0f, 0.0f});
    set_uni_vec3(g_gl.programs[PROGRAM_DEFAULT], "u_color", {1.0f, 0.0f, 0.0f});
    set_uni_float(g_gl.programs[PROGRAM_DEFAULT], "u_alpha", 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    set_uni_vec3(g_gl.programs[PROGRAM_DEFAULT], "u_translation", {0.3f, 0.0f, 0.0f});
    set_uni_vec3(g_gl.programs[PROGRAM_DEFAULT], "u_color", {1.0f, 0.0f, 0.0f});
    set_uni_float(g_gl.programs[PROGRAM_DEFAULT], "u_alpha", 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0u);
    glUseProgram(0u);
    glDisable(GL_BLEND);

    if (g_app.showAlpha)
    {
        // Second Pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g_gl.programs[PROGRAM_QUAD]);
        glBindTexture(GL_TEXTURE_2D, g_gl.textures[TEXTURE_RENDER_TARGET]);
        glBindVertexArray(g_gl.vertexArrays[VERTEXARRAY_QUAD]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0u);
    }
}

void release()
{
    glDeleteVertexArrays(VERTEXARRAY_COUNT, g_gl.vertexArrays);
    glDeleteBuffers(BUFFER_COUNT, g_gl.buffers);
}

void gui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Gui"))
    {
        ImGui::RadioButton("Default", &g_app.blendEq, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Separate", &g_app.blendEq, 1);
        ImGui::SliderFloat4("Clear Color", g_app.clearColor, 0.0f, 1.0f);
        ImGui::Checkbox("Show Alpha", &g_app.showAlpha);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the Window
    GLFWwindow *window = glfwCreateWindow(
        VIEWER_WIDTH, VIEWER_HEIGHT,
        "Blending Demo", nullptr, nullptr);
    if (window == nullptr)
    {
        LOG("=> Failure <=\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // glfwSetKeyCallback(window, &keyboardCallback);
    // glfwSetCursorPosCallback(window, &mouseMotionCallback);
    // glfwSetMouseButtonCallback(window, &mouseButtonCallback);
    // glfwSetScrollCallback(window, &mouseScrollCallback);
    // glfwSetWindowSizeCallback(window, &resizeCallback);

    // Load OpenGL functions
    LOG("Loading {OpenGL}\n");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG("gladLoadGLLoader failed\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    LOG("-- Begin -- Init\n");
    init();
    LOG("-- End -- Init\n");

    // glViewport(0, 0, VIEWER_WIDTH, VIEWER_HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        render();

        gui();

        glfwSwapBuffers(window);
    }

    release();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}