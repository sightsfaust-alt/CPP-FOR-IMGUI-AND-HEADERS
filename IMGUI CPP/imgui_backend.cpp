// Custom minimal Dear ImGui GLES backend for MenuSZK64.
// See imgui_backend.h for design notes.

#include "imgui_backend.h"

#include <GLES2/gl2.h>
#include <android/log.h>
#include <string.h>
#include <cstddef>  // offsetof

#define LOG_TAG "MenuSZK64-ImGui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace ImGuiBackend
{

static bool         g_initialized = false;
static bool         g_initFailed = false;
static GLuint       g_shaderHandle = 0;
static GLuint       g_vertHandle = 0;
static GLuint       g_fragHandle = 0;
static GLint        g_attribLocProjMtx = -1;
static GLint        g_attribLocPosition = -1;
static GLint        g_attribLocUV = -1;
static GLint        g_attribLocColor = -1;
static GLuint       g_vboHandle = 0;
static GLuint       g_elementsHandle = 0;
static GLuint       g_fontTexture = 0;

// ------------------------------------------------------------------
// Shaders (GLSL ES 1.00 — works on ES2 and ES3 contexts)
// ------------------------------------------------------------------
static const char* kVertexShaderSrc =
    "attribute vec2 Position;\n"
    "attribute vec2 UV;\n"
    "attribute vec4 Color;\n"
    "uniform mat4 ProjMtx;\n"
    "varying vec2 Frag_UV;\n"
    "varying vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "    Frag_UV = UV;\n"
    "    Frag_Color = Color;\n"
    "    gl_Position = ProjMtx * vec4(Position.xy, 0.0, 1.0);\n"
    "}\n";

static const char* kFragmentShaderSrc =
    "precision mediump float;\n"
    "uniform sampler2D Texture;\n"
    "varying vec2 Frag_UV;\n"
    "varying vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
    "}\n";

static bool CheckShader(GLuint handle, const char* desc)
{
    GLint status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1)
        {
            char* buf = new char[logLen];
            glGetShaderInfoLog(handle, logLen, nullptr, buf);
            LOGE("Shader compile failed (%s): %s", desc, buf);
            delete[] buf;
        }
        return false;
    }
    return true;
}

static bool CheckProgram(GLuint handle)
{
    GLint status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLen = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 1)
        {
            char* buf = new char[logLen];
            glGetProgramInfoLog(handle, logLen, nullptr, buf);
            LOGE("Program link failed: %s", buf);
            delete[] buf;
        }
        return false;
    }
    return true;
}

static bool CreateDeviceObjects()
{
    // Shaders
    g_vertHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(g_vertHandle, 1, &kVertexShaderSrc, nullptr);
    glCompileShader(g_vertHandle);
    if (!CheckShader(g_vertHandle, "vertex")) return false;

    g_fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_fragHandle, 1, &kFragmentShaderSrc, nullptr);
    glCompileShader(g_fragHandle);
    if (!CheckShader(g_fragHandle, "fragment")) return false;

    g_shaderHandle = glCreateProgram();
    glAttachShader(g_shaderHandle, g_vertHandle);
    glAttachShader(g_shaderHandle, g_fragHandle);
    glLinkProgram(g_shaderHandle);
    if (!CheckProgram(g_shaderHandle)) return false;

    g_attribLocProjMtx  = glGetUniformLocation(g_shaderHandle, "ProjMtx");
    g_attribLocPosition  = glGetAttribLocation(g_shaderHandle, "Position");
    g_attribLocUV        = glGetAttribLocation(g_shaderHandle, "UV");
    g_attribLocColor     = glGetAttribLocation(g_shaderHandle, "Color");

    // Buffers
    glGenBuffers(1, &g_vboHandle);
    glGenBuffers(1, &g_elementsHandle);

    // Font texture
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    glGenTextures(1, &g_fontTexture);
    glBindTexture(GL_TEXTURE_2D, g_fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_fontTexture);

    // Restore default texture binding (don't leave ours bound)
    glBindTexture(GL_TEXTURE_2D, 0);

    LOGI("Device objects created (font %dx%d)", width, height);
    return true;
}

static bool DoInit()
{
    if (g_initialized) return true;
    if (g_initFailed) { LOGE("DoInit: already failed, skipping"); return false; }

    LOGI("DoInit: creating context...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;              // no imgui.ini (avoid file writes)
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Style: readable on mobile
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.5f);

    if (!CreateDeviceObjects())
    {
        LOGE("CreateDeviceObjects failed — ImGui disabled");
        g_initFailed = true;
        ImGui::DestroyContext();
        return false;
    }

    // Sanity: verify GL objects are valid in this context
    LOGI("DoInit: shader=%u vbo=%u ibo=%u fontTex=%u posLoc=%d",
         g_shaderHandle, g_vboHandle, g_elementsHandle, g_fontTexture,
         g_attribLocPosition);
    if (g_shaderHandle == 0 || g_vboHandle == 0 || g_fontTexture == 0 ||
        g_attribLocPosition < 0)
    {
        LOGE("DoInit: GL objects invalid — no current GL context?");
        g_initFailed = true;
        ImGui::DestroyContext();
        return false;
    }

    g_initialized = true;
    LOGI("ImGui backend initialized");
    return true;
}

bool IsInitialized() { return g_initialized; }

// Human-readable status for on-screen diagnostics (via RW text path).
// Returns a short string describing the backend state.
const char* GetStatusString()
{
    if (g_initFailed) return "ImGui: INIT FAILED";
    if (!g_initialized) return "ImGui: NOT INIT";
    return "ImGui: OK";
}

// True when ImGui wants mouse input (touch is over an ImGui window/widget).
// Used by the touch hook to decide whether to swallow a touch instead of
// forwarding it to the game. Safe to call from the touch thread: the flag
// is written by the render thread during NewFrame, reads are single-word.
// May lag by one frame — acceptable for hit-testing.
bool WantsMouseCapture()
{
    if (!g_initialized || g_initFailed) return false;
    return ImGui::GetIO().WantCaptureMouse;
}

void ProcessTouch(int state, int touchId, float x, float y)
{
    if (!g_initialized) return;
    ImGuiIO& io = ImGui::GetIO();
    // We only track the first touch as mouse (menu is single-touch).
    // state: 2 = down, 3 = move, 1 = up
    if (touchId == 0 || touchId == 1)
    {
        if (state == 2) // down
        {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, true);
        }
        else if (state == 3) // move
        {
            io.AddMousePosEvent(x, y);
        }
        else if (state == 1) // up
        {
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, false);
        }
    }
}

bool NewFrame(int displayW, int displayH, float deltaTime)
{
    if (!DoInit()) return false;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)displayW, (float)displayH);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.DeltaTime = deltaTime > 0.0f ? deltaTime : (1.0f / 60.0f);

    ImGui::NewFrame();
    return true;
}

// ------------------------------------------------------------------
// Render with FULL GL state save/restore.
// The game expects its GL state untouched after we return.
// ------------------------------------------------------------------
void Render()
{
    if (!g_initialized) return;
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data)
    {
        LOGE("Render: draw_data is NULL!");
        return;
    }
    if (draw_data->CmdListsCount == 0)
    {
        // Only log once to avoid spam — helps diagnose empty frames
        static bool loggedEmpty = false;
        if (!loggedEmpty) { loggedEmpty = true; LOGI("Render: CmdListsCount == 0 (no draw commands)"); }
        return;
    }

    // Avoid rendering when minimized
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    // ---- BACKUP GL STATE ----
    GLint last_program = 0;
    GLint last_texture = 0;
    GLint last_array_buffer = 0;
    GLint last_element_array_buffer = 0;
    GLint last_viewport[4] = {0, 0, 0, 0};
    GLint last_scissor_box[4] = {0, 0, 0, 0};
    GLenum last_blend_src_rgb = 0, last_blend_dst_rgb = 0;
    GLenum last_blend_src_alpha = 0, last_blend_dst_alpha = 0;
    GLenum last_blend_equation_rgb = 0, last_blend_equation_alpha = 0;
    GLboolean last_enable_blend = GL_FALSE;
    GLboolean last_enable_cull_face = GL_FALSE;
    GLboolean last_enable_depth_test = GL_FALSE;
    GLboolean last_enable_scissor_test = GL_FALSE;
    GLboolean last_enable_stencil_test = GL_FALSE;

    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    last_enable_blend = glIsEnabled(GL_BLEND);
    last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);

    // ---- SETUP RENDER STATE ----
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);

    glViewport(0, 0, fb_width, fb_height);

    // Orthographic projection
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho[4][4] =
    {
        { 2.0f/(R-L), 0.0f,        0.0f, 0.0f },
        { 0.0f,       2.0f/(T-B),  0.0f, 0.0f },
        { 0.0f,       0.0f,       -1.0f, 0.0f },
        { (R+L)/(L-R), (T+B)/(B-T), 0.0f, 1.0f },
    };

    glUseProgram(g_shaderHandle);
    glUniform1i(glGetUniformLocation(g_shaderHandle, "Texture"), 0);
    glUniformMatrix4fv(g_attribLocProjMtx, 1, GL_FALSE, &ortho[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, g_vboHandle);
    glEnableVertexAttribArray((GLuint)g_attribLocPosition);
    glEnableVertexAttribArray((GLuint)g_attribLocUV);
    glEnableVertexAttribArray((GLuint)g_attribLocColor);
    glVertexAttribPointer((GLuint)g_attribLocPosition, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
    glVertexAttribPointer((GLuint)g_attribLocUV, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
    glVertexAttribPointer((GLuint)g_attribLocColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

    // ---- RENDER COMMAND LISTS ----
    ImVec2 clip_off = draw_data->DisplayPos;
    ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        glBindBuffer(GL_ARRAY_BUFFER, g_vboHandle);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert),
                     (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_elementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx),
                     (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
                    clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Flip Y for GL scissor coordinates
                    glScissor((int)clip_rect.x,
                              (int)(fb_height - clip_rect.w),
                              (int)(clip_rect.z - clip_rect.x),
                              (int)(clip_rect.w - clip_rect.y));
                    glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                    glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                                   sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                                   (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
                }
            }
        }
    }

    // ---- RESTORE GL STATE ----
    glDisableVertexAttribArray((GLuint)g_attribLocPosition);
    glDisableVertexAttribArray((GLuint)g_attribLocUV);
    glDisableVertexAttribArray((GLuint)g_attribLocColor);
    glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                        last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], last_scissor_box[2], last_scissor_box[3]);
}

void Shutdown()
{
    if (!g_initialized) return;
    glDeleteBuffers(1, &g_vboHandle);
    glDeleteBuffers(1, &g_elementsHandle);
    glDeleteTextures(1, &g_fontTexture);
    glDetachShader(g_shaderHandle, g_vertHandle);
    glDetachShader(g_shaderHandle, g_fragHandle);
    glDeleteShader(g_vertHandle);
    glDeleteShader(g_fragHandle);
    glDeleteProgram(g_shaderHandle);
    g_initialized = false;
    ImGui::DestroyContext();
}

} // namespace ImGuiBackend
