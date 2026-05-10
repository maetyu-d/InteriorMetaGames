#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <gl/GL.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")

#ifndef WGL_CONTEXT_MAJOR_VERSION_ARB
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#endif

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_RENDERBUFFER 0x8D41
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif

using GLchar = char;
using GLsizeiptr = ptrdiff_t;
using WGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC, HGLRC, const int*);

#define GLPROC(ret, name, ...) using name##Proc = ret (APIENTRY*)(__VA_ARGS__); static name##Proc name
GLPROC(void, glGenVertexArrays, GLsizei, GLuint*);
GLPROC(void, glBindVertexArray, GLuint);
GLPROC(void, glGenBuffers, GLsizei, GLuint*);
GLPROC(void, glBindBuffer, GLenum, GLuint);
GLPROC(void, glBufferData, GLenum, GLsizeiptr, const void*, GLenum);
GLPROC(void, glEnableVertexAttribArray, GLuint);
GLPROC(void, glVertexAttribPointer, GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
GLPROC(GLuint, glCreateShader, GLenum);
GLPROC(void, glShaderSource, GLuint, GLsizei, const GLchar* const*, const GLint*);
GLPROC(void, glCompileShader, GLuint);
GLPROC(void, glGetShaderiv, GLuint, GLenum, GLint*);
GLPROC(void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei*, GLchar*);
GLPROC(GLuint, glCreateProgram, void);
GLPROC(void, glAttachShader, GLuint, GLuint);
GLPROC(void, glLinkProgram, GLuint);
GLPROC(void, glGetProgramiv, GLuint, GLenum, GLint*);
GLPROC(void, glGetProgramInfoLog, GLuint, GLsizei, GLsizei*, GLchar*);
GLPROC(void, glUseProgram, GLuint);
GLPROC(void, glDeleteShader, GLuint);
GLPROC(GLint, glGetUniformLocation, GLuint, const GLchar*);
GLPROC(void, glUniformMatrix4fv, GLint, GLsizei, GLboolean, const GLfloat*);
GLPROC(void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat);
GLPROC(void, glUniform1f, GLint, GLfloat);
GLPROC(void, glUniform1i, GLint, GLint);
GLPROC(void, glActiveTexture, GLenum);
GLPROC(void, glGenFramebuffers, GLsizei, GLuint*);
GLPROC(void, glBindFramebuffer, GLenum, GLuint);
GLPROC(void, glFramebufferTexture2D, GLenum, GLenum, GLenum, GLuint, GLint);
GLPROC(GLenum, glCheckFramebufferStatus, GLenum);
GLPROC(void, glGenRenderbuffers, GLsizei, GLuint*);
GLPROC(void, glBindRenderbuffer, GLenum, GLuint);
GLPROC(void, glRenderbufferStorage, GLenum, GLenum, GLsizei, GLsizei);
GLPROC(void, glFramebufferRenderbuffer, GLenum, GLenum, GLenum, GLuint);
#undef GLPROC

struct Vec3 { float x, y, z; };
struct Vertex { Vec3 p, n, c; float shine; };
struct Mat4 { float m[16]; };
struct Aabb {
    float minX, maxX, minY, maxY, minZ, maxZ;
};

#pragma pack(push, 1)
struct SignalTelemetry {
    uint32_t magic;
    uint32_t version;
    uint32_t byteSize;
    uint32_t sequence;
    float timeSeconds;
    float realDistance;
    float planarDistance;
    float threeDimensionalDistance;
    float playerX, playerY, playerZ;
    float playerVelocityX, playerVelocityY, playerVelocityZ;
    float playerSpeed;
    float playerHorizontalSpeed;
    float playerFacingX, playerFacingY, playerFacingZ;
    float playerMoveDirX, playerMoveDirY, playerMoveDirZ;
    float playerYaw, playerPitch;
    float signalX, signalY, signalZ;
    float visibleTowerZ;
    int32_t falseStartStage;
    int32_t finalStage;
    float signalReached;
    float falseStartFlash;
};
#pragma pack(pop)

static HWND g_hwnd = nullptr;
static HDC g_hdc = nullptr;
static HGLRC g_glrc = nullptr;
static bool g_running = true;
static bool g_focused = true;
static bool g_mouseCaptured = true;
static bool g_fullscreen = false;
static RECT g_windowedRect{};
static DWORD g_windowedStyle = 0;
static int g_width = 1280;
static int g_height = 720;
static int g_fbWidth = 0;
static int g_fbHeight = 0;
static float g_time = 0.0f;
static float g_telemetryFileTimer = 0.0f;
static Vec3 g_lastTelemetryPlayer{};
static bool g_hasTelemetryPlayer = false;
static LARGE_INTEGER g_lastCounter{};
static LARGE_INTEGER g_frequency{};
static Vec3 g_player{0.0f, 8.0f, 420.0f};
static float g_yaw = 3.14159f;
static float g_pitch = -0.05f;
static float g_verticalVelocity = 0.0f;
static bool g_isGrounded = true;
static float g_wrongWaySignal = 0.0f;
static float g_wrongWaySustain = 0.0f;
static float g_signalReached = 0.0f;
static float g_falseStartFlash = 0.0f;
static int g_signalStage = 0;

static GLuint g_sceneProgram = 0;
static GLuint g_postProgram = 0;
static GLuint g_sceneVao = 0;
static GLuint g_sceneVbo = 0;
static GLuint g_lineVao = 0;
static GLuint g_lineVbo = 0;
static GLuint g_quadVao = 0;
static GLuint g_quadVbo = 0;
static GLuint g_fbo = 0;
static GLuint g_colorTex = 0;
static GLuint g_depthRb = 0;
static std::vector<Vertex> g_triangles;
static std::vector<Vertex> g_lines;
static std::vector<Aabb> g_buildingColliders;
static HANDLE g_telemetryMapping = nullptr;
static SignalTelemetry* g_telemetry = nullptr;
static char g_telemetryPath[MAX_PATH] = {};

static constexpr float kTowerZ = -2600.0f;
static constexpr float kStartZ = 420.0f;
static constexpr float kSignalReachDistance = 85.0f;
static constexpr float kFalseStartSpacing = 2600.0f;
static constexpr int kFinalSignalStage = 5;
static constexpr float kPlayerEyeHeight = 5.8f;
static constexpr float kPlayerFootRadius = 1.35f;
static constexpr float kPlayerGroundClearance = 0.38f;
static constexpr float kTerrainMeshStep = 6.5f;
static constexpr uint32_t kTelemetryMagic = 0x4443484Eu; // DCHN

static float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }
static float lerp(float a, float b, float t) { return a + (b - a) * t; }
static float smoothstep(float a, float b, float x) {
    float t = clampf((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
static Vec3 add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vec3 sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3 mul(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
static float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static Vec3 norm(Vec3 v) {
    float l = std::sqrt(std::max(0.000001f, dot(v, v)));
    return {v.x / l, v.y / l, v.z / l};
}

static uint32_t hash2(int x, int z) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(z) * 668265263u;
    h = (h ^ (h >> 13u)) * 1274126177u;
    return h ^ (h >> 16u);
}
static float rand01(int x, int z) {
    return static_cast<float>(hash2(x, z) & 0x00ffffffu) / static_cast<float>(0x01000000u);
}
static float valueNoise(float x, float z) {
    int ix = static_cast<int>(std::floor(x));
    int iz = static_cast<int>(std::floor(z));
    float fx = x - ix;
    float fz = z - iz;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fz = fz * fz * (3.0f - 2.0f * fz);
    return lerp(lerp(rand01(ix, iz), rand01(ix + 1, iz), fx),
                lerp(rand01(ix, iz + 1), rand01(ix + 1, iz + 1), fx), fz);
}
static float fbm(float x, float z) {
    float total = 0.0f, amp = 0.5f, freq = 1.0f;
    for (int i = 0; i < 5; ++i) {
        total += (valueNoise(x * freq, z * freq) * 2.0f - 1.0f) * amp;
        amp *= 0.5f;
        freq *= 2.03f;
    }
    return total;
}
static float progressToTower() {
    float finalTowerZ = kTowerZ - kFalseStartSpacing * static_cast<float>(kFinalSignalStage);
    return clampf((kStartZ - g_player.z) / (kStartZ - finalTowerZ), 0.0f, 1.0f);
}
static float currentTowerZ() {
    return kTowerZ - kFalseStartSpacing * static_cast<float>(g_signalStage);
}
static float finalTowerZ() {
    return kTowerZ - kFalseStartSpacing * static_cast<float>(kFinalSignalStage);
}
static float glitchAmount() {
    float p = progressToTower();
    float pulse = 0.5f + 0.5f * std::sin(g_time * (5.0f + p * 18.0f));
    return smoothstep(0.2f, 1.0f, p) * (0.45f + pulse * 0.55f);
}
static float terrainHeight(float x, float z) {
    float h = fbm(x * 0.012f, z * 0.012f) * 16.0f + fbm(x * 0.04f + 12.0f, z * 0.04f - 8.0f) * 4.5f;
    float warpX = fbm(x * 0.006f + 31.0f, z * 0.006f - 17.0f) * 42.0f;
    float warpZ = fbm(x * 0.007f - 9.0f, z * 0.007f + 25.0f) * 42.0f;
    h += fbm((x + warpX) * 0.022f, (z + warpZ) * 0.022f) * 5.5f;
    h += std::sin((x + warpX) * 0.018f + fbm(x * 0.01f, z * 0.01f) * 3.0f) * 1.4f;
    h -= std::exp(-(x * x) / 760.0f) * 5.5f;
    float g = glitchAmount();
    if (g > 0.12f) {
        float cell = lerp(18.0f, 4.5f, g);
        float block = std::floor((x + z * 0.17f + std::sin(g_time * 2.4f) * 30.0f) / cell);
        h += std::sin(block * 2.1f + g_time * 7.0f) * g * 3.2f;
        h = std::floor(h / lerp(2.8f, 0.65f, g)) * lerp(2.8f, 0.65f, g);
    }
    return h;
}
static Vec3 terrainNormal(float x, float z) {
    float s = 1.5f;
    return norm({terrainHeight(x - s, z) - terrainHeight(x + s, z), 2.0f * s, terrainHeight(x, z - s) - terrainHeight(x, z + s)});
}
static float triangleTerrainHeight(float x, float z) {
    const float step = kTerrainMeshStep;
    float x0 = std::floor(x / step) * step;
    float z0 = std::floor(z / step) * step;
    float u = clampf((x - x0) / step, 0.0f, 1.0f);
    float v = clampf((z - z0) / step, 0.0f, 1.0f);
    float h00 = terrainHeight(x0, z0);
    float h10 = terrainHeight(x0 + step, z0);
    float h01 = terrainHeight(x0, z0 + step);
    float h11 = terrainHeight(x0 + step, z0 + step);
    if (u >= v) {
        return h00 + (h10 - h00) * u + (h11 - h10) * v;
    }
    return h00 + (h11 - h01) * u + (h01 - h00) * v;
}
static float roadSurfaceHeight(float x, float z) {
    const float step = 18.0f;
    float z0 = std::floor(z / step) * step;
    float w = 17.0f + std::sin(z0 * 0.012f) * 2.5f;
    if (std::fabs(x) > w + kPlayerFootRadius) return -100000.0f;
    float u = clampf((x + w) / (w * 2.0f), 0.0f, 1.0f);
    float v = clampf((z - z0) / step, 0.0f, 1.0f);
    float h00 = terrainHeight(-w, z0) + 0.08f;
    float h10 = terrainHeight( w, z0) + 0.08f;
    float h01 = terrainHeight(-w, z0 + step) + 0.08f;
    float h11 = terrainHeight( w, z0 + step) + 0.08f;
    if (u >= v) {
        return h00 + (h10 - h00) * u + (h11 - h10) * v;
    }
    return h00 + (h11 - h01) * u + (h01 - h00) * v;
}
static float renderedSurfaceHeight(float x, float z) {
    return triangleTerrainHeight(x, z);
}
static float playerGroundHeight(float x, float z) {
    float r = kPlayerFootRadius;
    float d = r * 0.72f;
    float h = renderedSurfaceHeight(x, z);
    h = std::max(h, renderedSurfaceHeight(x + r, z));
    h = std::max(h, renderedSurfaceHeight(x - r, z));
    h = std::max(h, renderedSurfaceHeight(x, z + r));
    h = std::max(h, renderedSurfaceHeight(x, z - r));
    h = std::max(h, renderedSurfaceHeight(x + d, z + d));
    h = std::max(h, renderedSurfaceHeight(x - d, z + d));
    h = std::max(h, renderedSurfaceHeight(x + d, z - d));
    h = std::max(h, renderedSurfaceHeight(x - d, z - d));
    return h + kPlayerGroundClearance;
}

static void initTelemetry() {
    g_telemetryMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                            sizeof(SignalTelemetry), "Local\\DeadChannelSignalTelemetry");
    if (g_telemetryMapping) {
        g_telemetry = static_cast<SignalTelemetry*>(
            MapViewOfFile(g_telemetryMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SignalTelemetry)));
    }
    if (g_telemetry) {
        std::memset(g_telemetry, 0, sizeof(SignalTelemetry));
        g_telemetry->magic = kTelemetryMagic;
        g_telemetry->version = 3;
        g_telemetry->byteSize = sizeof(SignalTelemetry);
        g_telemetry->finalStage = kFinalSignalStage;
    }
    g_lastTelemetryPlayer = g_player;
    g_hasTelemetryPlayer = true;

    char exePath[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        char* slash = std::strrchr(exePath, '/');
        char* backslash = std::strrchr(exePath, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (slash) *(slash + 1) = '\0';
        std::snprintf(g_telemetryPath, sizeof(g_telemetryPath), "%sDeadChannelSignalTelemetry.json", exePath);
    }
}

static void writeTelemetryFile(const SignalTelemetry& t) {
    if (!g_telemetryPath[0]) return;
    char tmpPath[MAX_PATH] = {};
    std::snprintf(tmpPath, sizeof(tmpPath), "%s.tmp", g_telemetryPath);
    FILE* f = nullptr;
    if (fopen_s(&f, tmpPath, "wb") != 0 || !f) return;
    std::fprintf(f,
                 "{\n"
                 "  \"magic\": \"DCHN\",\n"
                 "  \"version\": %u,\n"
                 "  \"sequence\": %u,\n"
                 "  \"timeSeconds\": %.3f,\n"
                 "  \"realDistance\": %.3f,\n"
                 "  \"planarDistance\": %.3f,\n"
                 "  \"threeDimensionalDistance\": %.3f,\n"
                 "  \"player\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"playerVelocity\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"playerSpeed\": %.3f,\n"
                 "  \"playerHorizontalSpeed\": %.3f,\n"
                 "  \"playerFacing\": {\"x\": %.5f, \"y\": %.5f, \"z\": %.5f},\n"
                 "  \"playerMoveDirection\": {\"x\": %.5f, \"y\": %.5f, \"z\": %.5f},\n"
                 "  \"playerYaw\": %.5f,\n"
                 "  \"playerPitch\": %.5f,\n"
                 "  \"realSignal\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"visibleTowerZ\": %.3f,\n"
                 "  \"falseStartStage\": %d,\n"
                 "  \"finalStage\": %d,\n"
                 "  \"signalReached\": %.3f,\n"
                 "  \"falseStartFlash\": %.3f\n"
                 "}\n",
                 t.version, t.sequence, t.timeSeconds, t.realDistance, t.planarDistance,
                 t.threeDimensionalDistance, t.playerX, t.playerY, t.playerZ, t.playerVelocityX,
                 t.playerVelocityY, t.playerVelocityZ, t.playerSpeed, t.playerHorizontalSpeed,
                 t.playerFacingX, t.playerFacingY, t.playerFacingZ, t.playerMoveDirX, t.playerMoveDirY,
                 t.playerMoveDirZ, t.playerYaw, t.playerPitch, t.signalX, t.signalY, t.signalZ,
                 t.visibleTowerZ, t.falseStartStage, t.finalStage, t.signalReached, t.falseStartFlash);
    std::fclose(f);
    MoveFileExA(tmpPath, g_telemetryPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

static void updateTelemetry(float dt) {
    float signalX = 0.0f;
    float signalZ = finalTowerZ();
    float signalY = terrainHeight(signalX, signalZ) + 92.0f;
    float dx = g_player.x - signalX;
    float dy = g_player.y - signalY;
    float dz = g_player.z - signalZ;
    float invDt = dt > 0.0001f && g_hasTelemetryPlayer ? 1.0f / dt : 0.0f;
    Vec3 velocity{
        (g_player.x - g_lastTelemetryPlayer.x) * invDt,
        (g_player.y - g_lastTelemetryPlayer.y) * invDt,
        (g_player.z - g_lastTelemetryPlayer.z) * invDt
    };
    Vec3 facing{
        std::sin(g_yaw) * std::cos(g_pitch),
        std::sin(g_pitch),
        std::cos(g_yaw) * std::cos(g_pitch)
    };
    float horizontalSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    Vec3 moveDir{0.0f, 0.0f, 0.0f};
    if (horizontalSpeed > 0.001f) {
        moveDir = {velocity.x / horizontalSpeed, 0.0f, velocity.z / horizontalSpeed};
    }
    SignalTelemetry t{};
    t.magic = kTelemetryMagic;
    t.version = 3;
    t.byteSize = sizeof(SignalTelemetry);
    t.sequence = g_telemetry ? g_telemetry->sequence + 1u : 1u;
    t.timeSeconds = g_time;
    t.planarDistance = std::sqrt(dx * dx + dz * dz);
    t.threeDimensionalDistance = std::sqrt(dx * dx + dy * dy + dz * dz);
    t.realDistance = t.planarDistance;
    t.playerX = g_player.x;
    t.playerY = g_player.y;
    t.playerZ = g_player.z;
    t.playerVelocityX = velocity.x;
    t.playerVelocityY = velocity.y;
    t.playerVelocityZ = velocity.z;
    t.playerSpeed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
    t.playerHorizontalSpeed = horizontalSpeed;
    t.playerFacingX = facing.x;
    t.playerFacingY = facing.y;
    t.playerFacingZ = facing.z;
    t.playerMoveDirX = moveDir.x;
    t.playerMoveDirY = moveDir.y;
    t.playerMoveDirZ = moveDir.z;
    t.playerYaw = g_yaw;
    t.playerPitch = g_pitch;
    t.signalX = signalX;
    t.signalY = signalY;
    t.signalZ = signalZ;
    t.visibleTowerZ = currentTowerZ();
    t.falseStartStage = g_signalStage;
    t.finalStage = kFinalSignalStage;
    t.signalReached = g_signalReached;
    t.falseStartFlash = g_falseStartFlash;
    if (g_telemetry) {
        *g_telemetry = t;
    }
    g_lastTelemetryPlayer = g_player;
    g_hasTelemetryPlayer = true;

    g_telemetryFileTimer += dt;
    if (g_telemetryFileTimer >= 0.25f) {
        g_telemetryFileTimer = 0.0f;
        writeTelemetryFile(t);
    }
}

static void shutdownTelemetry() {
    if (g_telemetry) {
        g_telemetry->sequence += 1u;
        g_telemetry->signalReached = -1.0f;
        UnmapViewOfFile(g_telemetry);
        g_telemetry = nullptr;
    }
    if (g_telemetryMapping) {
        CloseHandle(g_telemetryMapping);
        g_telemetryMapping = nullptr;
    }
}

static Mat4 identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}
static Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            r.m[c * 4 + row] = a.m[0 * 4 + row] * b.m[c * 4 + 0] + a.m[1 * 4 + row] * b.m[c * 4 + 1] + a.m[2 * 4 + row] * b.m[c * 4 + 2] + a.m[3 * 4 + row] * b.m[c * 4 + 3];
    return r;
}
static Mat4 perspective(float fovyDeg, float aspect, float nearZ, float farZ) {
    float f = 1.0f / std::tan(fovyDeg * 0.5f * 3.14159265f / 180.0f);
    Mat4 r{};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (farZ + nearZ) / (nearZ - farZ);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
    return r;
}
static Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = norm(sub(center, eye));
    Vec3 s = norm(cross(f, up));
    Vec3 u = cross(s, f);
    Mat4 r = identity();
    r.m[0] = s.x; r.m[4] = s.y; r.m[8] = s.z;
    r.m[1] = u.x; r.m[5] = u.y; r.m[9] = u.z;
    r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
    r.m[12] = -dot(s, eye);
    r.m[13] = -dot(u, eye);
    r.m[14] = dot(f, eye);
    return r;
}

static void pushTri(Vec3 a, Vec3 b, Vec3 c, Vec3 color, float shine) {
    Vec3 n = norm(cross(sub(b, a), sub(c, a)));
    g_triangles.push_back({a, n, color, shine});
    g_triangles.push_back({b, n, color, shine});
    g_triangles.push_back({c, n, color, shine});
}
static void pushTriSmooth(Vec3 a, Vec3 b, Vec3 c, Vec3 na, Vec3 nb, Vec3 nc, Vec3 color, float shine) {
    g_triangles.push_back({a, na, color, shine});
    g_triangles.push_back({b, nb, color, shine});
    g_triangles.push_back({c, nc, color, shine});
}
static void pushLine(Vec3 a, Vec3 b, Vec3 color, float shine = 0.5f) {
    g_lines.push_back({a, {0, 1, 0}, color, shine});
    g_lines.push_back({b, {0, 1, 0}, color, shine});
}
static void pushCube(Vec3 center, Vec3 scale, Vec3 color, float shine, float yaw = 0.0f) {
    float cy = std::cos(yaw), sy = std::sin(yaw);
    auto tr = [&](float x, float y, float z) {
        return Vec3{center.x + (x * cy + z * sy) * scale.x, center.y + y * scale.y, center.z + (-x * sy + z * cy) * scale.z};
    };
    Vec3 p[8] = {
        tr(-1,-1,-1), tr(1,-1,-1), tr(1,1,-1), tr(-1,1,-1),
        tr(-1,-1, 1), tr(1,-1, 1), tr(1,1, 1), tr(-1,1, 1)
    };
    int f[6][4] = {{0,1,2,3},{5,4,7,6},{4,0,3,7},{1,5,6,2},{3,2,6,7},{4,5,1,0}};
    for (auto& q : f) {
        pushTri(p[q[0]], p[q[1]], p[q[2]], color, shine);
        pushTri(p[q[0]], p[q[2]], p[q[3]], color, shine);
    }
}
static void pushBuilding(Vec3 baseCenter, Vec3 halfSize, Vec3 color, float shine) {
    Vec3 center{baseCenter.x, baseCenter.y + halfSize.y, baseCenter.z};
    pushCube(center, halfSize, color, shine, 0.0f);
    g_buildingColliders.push_back({
        center.x - halfSize.x, center.x + halfSize.x,
        center.y - halfSize.y, center.y + halfSize.y,
        center.z - halfSize.z, center.z + halfSize.z
    });
}
static bool groundedBuildingBase(float x, float z, float sx, float sz, float& baseY) {
    float h0 = renderedSurfaceHeight(x, z);
    float h1 = renderedSurfaceHeight(x - sx, z - sz);
    float h2 = renderedSurfaceHeight(x + sx, z - sz);
    float h3 = renderedSurfaceHeight(x - sx, z + sz);
    float h4 = renderedSurfaceHeight(x + sx, z + sz);
    float low = std::min(std::min(h0, h1), std::min(std::min(h2, h3), h4));
    float high = std::max(std::max(h0, h1), std::max(std::max(h2, h3), h4));
    if (high - low > 5.0f) return false;
    baseY = high;
    return true;
}

static void buildTerrain() {
    const float step = kTerrainMeshStep;
    const int radius = 42;
    float baseX = std::floor(g_player.x / step) * step;
    float baseZ = std::floor(g_player.z / step) * step;
    for (int iz = -radius; iz < radius; ++iz) {
        for (int ix = -radius; ix < radius; ++ix) {
            float x0 = baseX + ix * step, x1 = x0 + step;
            float z0 = baseZ + iz * step, z1 = z0 + step;
            Vec3 p00{x0, terrainHeight(x0, z0), z0};
            Vec3 p10{x1, terrainHeight(x1, z0), z0};
            Vec3 p01{x0, terrainHeight(x0, z1), z1};
            Vec3 p11{x1, terrainHeight(x1, z1), z1};
            float wet = valueNoise(x0 * 0.06f, z0 * 0.06f);
            float rust = valueNoise(x0 * 0.021f + 40.0f, z0 * 0.021f - 14.0f);
            Vec3 c{0.030f + wet * 0.066f + rust * 0.025f, 0.040f + wet * 0.085f + rust * 0.012f, 0.048f + wet * 0.12f};
            float shine = 0.18f + wet * 0.75f;
            Vec3 n00 = terrainNormal(x0, z0);
            Vec3 n10 = terrainNormal(x1, z0);
            Vec3 n01 = terrainNormal(x0, z1);
            Vec3 n11 = terrainNormal(x1, z1);
            pushTriSmooth(p00, p11, p10, n00, n11, n10, c, shine);
            pushTriSmooth(p00, p01, p11, n00, n01, n11, c, shine);
        }
    }
}
static void buildRoad() {
    // The road is shader-painted onto the terrain. Keep this empty so no
    // physical path sheets can float above hills.
}
static void buildPuddles() {
    int base = static_cast<int>(std::floor(g_player.z / 42.0f));
    for (int i = -12; i <= 9; ++i) {
        int cell = base + i;
        float z = cell * 42.0f + (rand01(cell, 71) - 0.5f) * 18.0f;
        float x = (rand01(cell, 133) - 0.5f) * 58.0f;
        float sx = 2.4f + rand01(cell, 91) * 6.2f;
        float sz = 1.2f + rand01(cell, 119) * 3.2f;
        Vec3 c{0.035f, 0.16f + glitchAmount() * 0.08f, 0.20f + glitchAmount() * 0.14f};
        Vec3 under{0.006f, 0.022f, 0.026f};
        float ax = x - sx, az = z - sz;
        float bx = x + sx, bz = z - sz * 0.75f;
        float dx = x - sx * 0.8f, dz = z + sz;
        float ex = x + sx * 1.1f, ez = z + sz * 0.85f;
        Vec3 n = terrainNormal(x, z);
        if (n.y < 0.965f) continue;
        float ya = renderedSurfaceHeight(ax, az);
        float yb = renderedSurfaceHeight(bx, bz);
        float yd = renderedSurfaceHeight(dx, dz);
        float ye = renderedSurfaceHeight(ex, ez);
        float yc = renderedSurfaceHeight(x, z);
        float ym1 = renderedSurfaceHeight((ax + bx) * 0.5f, (az + bz) * 0.5f);
        float ym2 = renderedSurfaceHeight((bx + ex) * 0.5f, (bz + ez) * 0.5f);
        float ym3 = renderedSurfaceHeight((dx + ex) * 0.5f, (dz + ez) * 0.5f);
        float ym4 = renderedSurfaceHeight((ax + dx) * 0.5f, (az + dz) * 0.5f);
        float low = std::min(std::min(std::min(ya, yb), std::min(yd, ye)), std::min(std::min(yc, ym1), std::min(std::min(ym2, ym3), ym4)));
        float high = std::max(std::max(std::max(ya, yb), std::max(yd, ye)), std::max(std::max(yc, ym1), std::max(std::max(ym2, ym3), ym4)));
        if (high - low > 0.11f) continue;
        Vec3 a{ax, ya + 0.025f, az};
        Vec3 b{bx, yb + 0.025f, bz};
        Vec3 d{dx, yd + 0.025f, dz};
        Vec3 e{ex, ye + 0.025f, ez};
        Vec3 au{x - sx * 1.03f, renderedSurfaceHeight(x - sx * 1.03f, z - sz * 1.03f) - 0.020f, z - sz * 1.03f};
        Vec3 bu{x + sx * 1.03f, renderedSurfaceHeight(x + sx * 1.03f, z - sz * 0.78f) - 0.020f, z - sz * 0.78f};
        Vec3 du{x - sx * 0.84f, renderedSurfaceHeight(x - sx * 0.84f, z + sz * 1.03f) - 0.020f, z + sz * 1.03f};
        Vec3 eu{x + sx * 1.13f, renderedSurfaceHeight(x + sx * 1.13f, z + sz * 0.88f) - 0.020f, z + sz * 0.88f};
        pushTri(au, eu, bu, under, 0.25f);
        pushTri(au, du, eu, under, 0.25f);
        pushTri(a, e, b, c, 1.0f);
        pushTri(a, d, e, c, 1.0f);
    }
}
static void buildPylon(float x, float z, float h, Vec3 color) {
    float y = terrainHeight(x, z);
    Vec3 top{x, y + h, z};
    pushLine({x - 4, y, z - 4}, top, color);
    pushLine({x + 4, y, z - 4}, top, color);
    pushLine({x - 4, y, z + 4}, top, color);
    pushLine({x + 4, y, z + 4}, top, color);
    for (int i = 1; i < 9; ++i) {
        float yy = y + h * (i / 9.0f);
        pushLine({x - 4, yy, z - 4}, {x + 4, yy, z + 4}, color);
        pushLine({x + 4, yy, z - 4}, {x - 4, yy, z + 4}, color);
    }
    pushLine({x - 20, y + h * 0.74f, z}, {x + 20, y + h * 0.74f, z}, color);
    pushLine({x, y + h * 0.88f, z - 20}, {x, y + h * 0.88f, z + 20}, color);
}
static void buildProps() {
    int cx = static_cast<int>(std::floor(g_player.x / 64.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 64.0f));
    for (int zc = cz - 8; zc <= cz + 6; ++zc) {
        for (int xc = cx - 7; xc <= cx + 7; ++xc) {
            float r = rand01(xc, zc);
            float x = xc * 64.0f + (rand01(xc + 90, zc) - 0.5f) * 42.0f;
            float z = zc * 64.0f + (rand01(xc, zc + 90) - 0.5f) * 42.0f;
            if (std::fabs(x) < 30.0f) x += x < 0 ? -34.0f : 34.0f;
            if (r < 0.22f) {
                float y = renderedSurfaceHeight(x, z) + 0.35f;
                float yaw = r * 6.28f;
                pushCube({x, y, z}, {3.2f + r * 8.0f, 0.22f, 1.4f + r * 2.5f}, {0.055f, 0.068f, 0.066f}, 0.72f, yaw);
                pushCube({x + 1.8f, y + 0.2f, z - 0.6f}, {0.9f, 0.12f, 0.55f}, {0.09f, 0.16f, 0.17f}, 0.95f, yaw + 0.25f);
            }
            else if (r < 0.38f) buildPylon(x, z, 22.0f + r * 40.0f, {0.25f, 0.34f, 0.37f});
            else if (r < 0.40f) {
                float sx = 13.0f + r * 18.0f;
                float sy = 7.0f + r * 8.0f;
                float sz = 3.2f;
                float y = 0.0f;
                if (!groundedBuildingBase(x, z, sx, sz, y)) continue;
                pushBuilding({x, y, z}, {sx, sy, sz}, {0.075f, 0.086f, 0.088f}, 0.65f);
                if (r < 0.385f) {
                    pushBuilding({x + sx * 0.18f, y + sy * 2.0f, z}, {sx * 0.42f, sy * 0.45f, sz * 0.92f}, {0.060f, 0.070f, 0.070f}, 0.58f);
                }
            }
        }
    }
}
static void buildGroundDetail() {
    int cx = static_cast<int>(std::floor(g_player.x / 24.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 24.0f));
    for (int zc = cz - 10; zc <= cz + 6; ++zc) {
        for (int xc = cx - 8; xc <= cx + 8; ++xc) {
            float r = rand01(xc + 1700, zc - 410);
            float x = xc * 24.0f + (rand01(xc + 17, zc + 29) - 0.5f) * 17.0f;
            float z = zc * 24.0f + (rand01(xc - 31, zc + 43) - 0.5f) * 17.0f;
            if (std::fabs(x) < 11.0f && r > 0.18f) continue;
            float y = renderedSurfaceHeight(x, z) + 0.18f;
            float yaw = rand01(xc, zc) * 6.28318f;
            if (r < 0.18f) {
                float s = 0.25f + rand01(xc + 2, zc) * 0.75f;
                Vec3 c{0.030f + r * 0.12f, 0.034f + r * 0.10f, 0.036f + r * 0.11f};
                pushCube({x, y + s * 0.18f, z}, {s * 1.8f, s * 0.18f, s * 1.1f}, c, 0.55f, yaw);
            } else if (r < 0.30f) {
                float sx = 1.8f + rand01(xc + 8, zc) * 3.5f;
                float sz = 0.35f + rand01(xc, zc + 8) * 1.5f;
                Vec3 c{0.070f, 0.085f + r * 0.11f, 0.088f + r * 0.16f};
                pushCube({x, y + 0.06f, z}, {sx, 0.07f, sz}, c, 0.9f, yaw);
            } else if (r < 0.39f) {
                Vec3 c{0.10f, 0.18f + glitchAmount() * 0.16f, 0.20f + glitchAmount() * 0.22f};
                float h = 0.8f + rand01(xc + 5, zc - 3) * 2.1f;
                pushLine({x, y, z}, {x + std::cos(yaw) * 0.8f, y + h, z + std::sin(yaw) * 0.8f}, c, 0.9f);
                pushLine({x + 0.4f, y, z - 0.2f}, {x + std::cos(yaw + 1.8f) * 0.7f, y + h * 0.72f, z + std::sin(yaw + 1.8f) * 0.7f}, c, 0.9f);
            } else if (r < 0.54f) {
                Vec3 c{0.035f, 0.115f + glitchAmount() * 0.055f, 0.080f + glitchAmount() * 0.035f};
                Vec3 p{x, y + 0.05f, z};
                float curl = yaw;
                for (int k = 0; k < 5; ++k) {
                    float step = 1.0f + rand01(xc + k, zc - k) * 1.7f;
                    Vec3 q{p.x + std::cos(curl) * step, renderedSurfaceHeight(p.x + std::cos(curl) * step, p.z + std::sin(curl) * step) + 0.08f, p.z + std::sin(curl) * step};
                    pushLine(p, q, c, 0.75f);
                    p = q;
                    curl += (rand01(xc + k * 3, zc + k * 5) - 0.5f) * 1.2f;
                }
            }
        }
    }
}
static void buildRuinedStructures() {
    int cx = static_cast<int>(std::floor(g_player.x / 96.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 96.0f));
    for (int zc = cz - 6; zc <= cz + 4; ++zc) {
        for (int xc = cx - 4; xc <= cx + 4; ++xc) {
            float r = rand01(xc + 3000, zc - 2200);
            if (r > 0.15f) continue;
            float x = xc * 96.0f + (rand01(xc + 11, zc + 19) - 0.5f) * 54.0f;
            float z = zc * 96.0f + (rand01(xc - 23, zc + 37) - 0.5f) * 54.0f;
            if (std::fabs(x) < 46.0f) x += x < 0.0f ? -52.0f : 52.0f;
            float y = renderedSurfaceHeight(x, z);
            float yaw = rand01(xc - 7, zc + 5) * 6.28318f;
            if (r < 0.15f) {
                float sx = 20.0f + rand01(xc, zc) * 32.0f;
                float sy = 10.0f + rand01(xc + 1, zc) * 22.0f;
                float sz = 7.0f + rand01(xc + 4, zc - 3) * 10.0f;
                float baseY = 0.0f;
                if (!groundedBuildingBase(x, z, sx, sz, baseY)) continue;
                Vec3 concrete{0.042f, 0.047f, 0.046f};
                pushBuilding({x, baseY, z}, {sx, sy, sz}, concrete, 0.62f);
                if (r < 0.055f) {
                    pushBuilding({x - sx * 0.35f, baseY + sy * 2.0f, z + sz * 0.15f}, {sx * 0.28f, sy * 0.42f, sz * 0.74f}, {0.036f, 0.041f, 0.041f}, 0.56f);
                } else if (r < 0.10f) {
                    pushBuilding({x + sx * 0.18f, baseY + sy * 2.0f, z - sz * 0.10f}, {sx * 0.55f, sy * 0.24f, sz * 0.86f}, {0.038f, 0.044f, 0.044f}, 0.58f);
                }
            }
        }
    }
}
static void buildDistantSilhouettes() {
    int cz = static_cast<int>(std::floor(g_player.z / 180.0f));
    float g = glitchAmount();
    for (int i = -5; i <= 4; ++i) {
        int cell = cz + i;
        for (int side = -1; side <= 1; side += 2) {
            float r = rand01(cell + side * 23, 1200);
            if (r > 0.36f) continue;
            float z = cell * 180.0f + rand01(cell, side * 91) * 95.0f;
            float x = side * (150.0f + rand01(cell, side * 77) * 260.0f);
            float h = 38.0f + rand01(cell + 5, side * 33) * 78.0f;
            float w = 18.0f + rand01(cell + 7, side * 44) * 42.0f;
            float sz = 8.0f + r * 18.0f;
            float y = 0.0f;
            if (!groundedBuildingBase(x, z, w, sz, y)) continue;
            Vec3 dark{0.018f, 0.026f + g * 0.010f, 0.028f + g * 0.016f};
            pushBuilding({x, y, z}, {w, h * 0.5f, sz}, dark, 0.55f);
            if (r < 0.18f) {
                pushBuilding({x + side * w * 0.20f, y + h, z}, {w * 0.52f, h * 0.18f, sz * 0.88f}, {0.014f, 0.022f + g * 0.010f, 0.024f + g * 0.012f}, 0.52f);
            }
        }
    }
}
static void buildBrutalistBlocks() {
    int base = static_cast<int>(std::floor(g_player.z / 220.0f));
    for (int i = -5; i <= 4; ++i) {
        int cell = base + i;
        for (int side = -1; side <= 1; side += 2) {
            float gate = rand01(cell + side * 301, 4040);
            if (gate > 0.42f) continue;
            float z = cell * 220.0f + 70.0f + rand01(cell, side * 409) * 95.0f;
            float x = side * (82.0f + rand01(cell, side * 503) * 130.0f);
            float sx = 20.0f + rand01(cell + 7, side * 601) * 38.0f;
            float sy = (12.0f + rand01(cell + 11, side * 607) * 30.0f) * 2.0f;
            float sz = 10.0f + rand01(cell + 13, side * 613) * 20.0f;
            float y = 0.0f;
            if (!groundedBuildingBase(x, z, sx, sz, y)) {
                y = renderedSurfaceHeight(x, z);
            }
            Vec3 concrete{0.038f, 0.044f, 0.043f};
            pushBuilding({x, y, z}, {sx, sy, sz}, concrete, 0.58f);
            if (gate < 0.18f) {
                pushBuilding({x + side * sx * 0.28f, y + sy * 2.0f, z - sz * 0.12f}, {sx * 0.42f, sy * 0.34f, sz * 0.82f}, {0.030f, 0.036f, 0.036f}, 0.54f);
            }
            Vec3 windowGlow{0.95f, 0.68f + glitchAmount() * 0.08f, 0.18f};
            int rows = 1 + static_cast<int>(rand01(cell + 19, side * 701) * 3.0f);
            int cols = 1 + static_cast<int>(rand01(cell + 23, side * 709) * 4.0f);
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    if (rand01(cell + row * 17, side * 811 + col * 23) < 0.38f) continue;
                    float wx = x + (col - (cols - 1) * 0.5f) * (sx * 0.38f);
                    float wy = y + sy * (0.72f + row * 0.42f);
                    pushCube({wx, wy, z - sz - 0.09f}, {sx * 0.075f, sy * 0.055f, 0.045f}, windowGlow, 1.0f, 0.0f);
                }
            }
        }
    }
}
static void buildAtmosphereDetail() {
    int base = static_cast<int>(std::floor(g_player.z / 18.0f));
    float g = glitchAmount();
    float rainBaseX = std::floor(g_player.x / 16.0f) * 16.0f;
    float rainBaseZ = std::floor(g_player.z / 16.0f) * 16.0f;
    float gust = std::sin(g_time * 0.47f) * 0.65f + std::sin(g_time * 1.13f) * 0.35f;
    for (int i = -14; i <= 14; ++i) {
        int cell = base + i;
        for (int j = 0; j < 7; ++j) {
            float localX = (rand01(cell + j * 31, 802) - 0.5f) * 190.0f;
            float localZ = (rand01(cell + j * 23, 801) - 0.5f) * 260.0f;
            float seed = rand01(cell + j * 17, 803);
            float fall = std::fmod(g_time * (52.0f + seed * 48.0f) + seed * 92.0f, 86.0f);
            float drift = std::fmod(g_time * (8.0f + seed * 11.0f), 16.0f);
            float x = rainBaseX + localX + std::sin(g_time * 0.7f + seed * 9.0f) * 2.5f + gust * (5.0f + seed * 7.0f);
            float z = rainBaseZ + localZ + drift;
            float ground = renderedSurfaceHeight(x, z);
            float y = g_player.y + 46.0f - fall;
            if (y < ground + 2.0f) y += 86.0f;
            float nearFactor = 1.0f - clampf(std::fabs(localZ) / 150.0f, 0.0f, 1.0f);
            float sheet = smoothstep(0.62f, 0.96f, rand01(cell / 3, 940));
            float len = 5.5f + rand01(cell + j * 13, 804) * 11.0f + nearFactor * 6.0f + sheet * 4.0f;
            float slant = 1.4f + gust * 1.2f + seed * 1.0f;
            float fade = (0.34f + nearFactor * 0.16f + sheet * 0.05f) * 0.62f;
            Vec3 c{(0.045f + nearFactor * 0.030f) * fade, (0.18f + g * 0.10f + nearFactor * 0.075f) * fade, (0.22f + g * 0.14f + nearFactor * 0.095f) * fade};
            float wobble = std::sin(g_time * (3.0f + seed * 4.0f) + seed * 20.0f) * 0.55f;
            pushLine({x, y, z}, {x - slant + wobble * 0.5f, y - len, z + 0.55f + nearFactor * 0.7f}, c, 0.45f + nearFactor * 0.08f);
            if (nearFactor > 0.68f && seed > 0.88f) {
                pushLine({x + 0.4f, y - len * 0.25f, z + 0.2f}, {x - slant * 0.55f + wobble * 0.4f, y - len * 0.72f, z + 0.7f}, {c.x * 0.42f, c.y * 0.50f, c.z * 0.56f}, 0.48f);
            }
            if (y - len < ground + 2.8f && seed > 0.82f) {
                float s = 0.55f + seed * 0.9f;
                Vec3 sc{0.045f, 0.21f + g * 0.07f, 0.24f + g * 0.08f};
                pushLine({x - s, ground + 0.16f, z}, {x + s, ground + 0.16f, z}, {sc.x * 0.55f, sc.y * 0.55f, sc.z * 0.55f}, 0.55f);
                pushLine({x, ground + 0.16f, z - s * 0.35f}, {x, ground + 0.16f, z + s * 0.35f}, {sc.x * 0.55f, sc.y * 0.55f, sc.z * 0.55f}, 0.55f);
            }
        }
    }
}
static void buildTower() {
    float towerZ = currentTowerZ();
    buildPylon(0.0f, towerZ, 230.0f, {0.55f, 0.88f, 1.0f});
    float y = terrainHeight(0.0f, towerZ);
    for (int i = 0; i < 8; ++i) {
        float rad = 40.0f + i * 35.0f + std::sin(g_time * 2.0f + i) * 8.0f;
        float yy = y + 50.0f + i * 24.0f;
        Vec3 prev{rad, yy, towerZ};
        for (int s = 1; s <= 112; ++s) {
            float a = s / 112.0f * 6.2831853f;
            Vec3 cur{std::cos(a) * rad, yy + std::sin(a * 3.0f + g_time) * 3.0f, towerZ + std::sin(a) * rad};
            pushLine(prev, cur, {0.08f, 0.75f, 1.0f}, 1.0f);
            prev = cur;
        }
    }
}
static void buildWorld() {
    g_triangles.clear();
    g_lines.clear();
    g_buildingColliders.clear();
    g_triangles.reserve(90000);
    g_lines.reserve(8000);
    buildTerrain();
    buildRoad();
    buildGroundDetail();
    buildProps();
    buildRuinedStructures();
    buildDistantSilhouettes();
    buildBrutalistBlocks();
    buildAtmosphereDetail();
    buildTower();
}

static void* loadGlProc(const char* name) {
    void* p = reinterpret_cast<void*>(wglGetProcAddress(name));
    if (!p || p == reinterpret_cast<void*>(0x1) || p == reinterpret_cast<void*>(0x2) || p == reinterpret_cast<void*>(0x3) || p == reinterpret_cast<void*>(-1)) {
        p = reinterpret_cast<void*>(GetProcAddress(GetModuleHandleA("opengl32.dll"), name));
    }
    return p;
}
#define LOAD_GL(name) name = reinterpret_cast<name##Proc>(loadGlProc(#name)); if (!name) return false
static bool loadGlFunctions() {
    LOAD_GL(glGenVertexArrays); LOAD_GL(glBindVertexArray); LOAD_GL(glGenBuffers); LOAD_GL(glBindBuffer);
    LOAD_GL(glBufferData); LOAD_GL(glEnableVertexAttribArray); LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glCreateShader); LOAD_GL(glShaderSource); LOAD_GL(glCompileShader); LOAD_GL(glGetShaderiv); LOAD_GL(glGetShaderInfoLog);
    LOAD_GL(glCreateProgram); LOAD_GL(glAttachShader); LOAD_GL(glLinkProgram); LOAD_GL(glGetProgramiv); LOAD_GL(glGetProgramInfoLog);
    LOAD_GL(glUseProgram); LOAD_GL(glDeleteShader); LOAD_GL(glGetUniformLocation); LOAD_GL(glUniformMatrix4fv);
    LOAD_GL(glUniform3f); LOAD_GL(glUniform1f); LOAD_GL(glUniform1i); LOAD_GL(glActiveTexture);
    LOAD_GL(glGenFramebuffers); LOAD_GL(glBindFramebuffer); LOAD_GL(glFramebufferTexture2D); LOAD_GL(glCheckFramebufferStatus);
    LOAD_GL(glGenRenderbuffers); LOAD_GL(glBindRenderbuffer); LOAD_GL(glRenderbufferStorage); LOAD_GL(glFramebufferRenderbuffer);
    return true;
}
#undef LOAD_GL

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        MessageBoxA(nullptr, log, "Shader compile error", MB_ICONERROR);
    }
    return shader;
}
static GLuint createProgram(const char* vs, const char* fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]{};
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        MessageBoxA(nullptr, log, "Program link error", MB_ICONERROR);
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

static const char* kSceneVs = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aColor;
layout(location=3) in float aShine;
uniform mat4 uMvp;
uniform mat4 uView;
uniform float uTime;
uniform float uGlitch;
out vec3 vWorld;
out vec3 vNormal;
out vec3 vColor;
out float vShine;
void main() {
    vec3 p = aPos;
    float band = floor((p.y + p.z * .025 + uTime * 8.0) / max(1.5, 10.0 - uGlitch * 8.0));
    p.x += sin(band * 5.31 + uTime * 14.0) * uGlitch * 0.8;
    vWorld = p;
    vNormal = normalize(aNormal);
    vColor = aColor;
    vShine = aShine;
    gl_Position = uMvp * vec4(p, 1.0);
}
)GLSL";

static const char* kSceneFs = R"GLSL(
#version 330 core
in vec3 vWorld;
in vec3 vNormal;
in vec3 vColor;
in float vShine;
uniform vec3 uEye;
uniform float uTime;
uniform float uGlitch;
uniform float uTowerAlign;
uniform float uTowerZ;
out vec4 FragColor;
float hash(vec2 p) { return fract(sin(dot(p, vec2(41.7, 289.3))) * 43758.5453); }
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}
float fbm2(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; ++i) {
        v += noise(p) * a;
        p = p * 2.07 + 13.1;
        a *= 0.5;
    }
    return v;
}
float organic(vec2 p) {
    vec2 q = vec2(fbm2(p + vec2(0.0, 4.2)), fbm2(p + vec2(9.1, 1.7)));
    vec2 r = vec2(fbm2(p + q * 2.4 + vec2(3.4, 7.8)), fbm2(p + q * 2.1 + vec2(8.2, 2.6)));
    return fbm2(p + r * 3.2 + q * 1.1);
}
float cellular(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float m = 9.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 g = vec2(float(x), float(y));
            vec2 o = vec2(hash(i + g), hash(i + g + 13.7));
            o = 0.5 + 0.5 * sin(6.2831 * o);
            m = min(m, length(g + o - f));
        }
    }
    return m;
}
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(-0.42, 0.78, 0.34));
    vec3 V = normalize(uEye - vWorld);
    vec3 H = normalize(L + V);
    float diff = max(dot(N, L), 0.0);
    float glitter = hash(floor(vWorld.xz * 0.45)) * 0.35 + 0.65;
    float spec = pow(max(dot(N, H), 0.0), mix(28.0, 140.0, clamp(vShine, 0.0, 1.0))) * vShine * glitter;
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.2);
    float towerPulse = sin(uTime * 7.0 + length(vWorld.xz) * 0.015) * 0.5 + 0.5;
    float towerGlow = exp(-abs(vWorld.x) * 0.006) * exp(-abs(vWorld.z - uTowerZ) * 0.0014);
    vec3 coldSignal = vec3(0.05, 0.78, 1.0);
    vec3 hotSignal = mix(vec3(1.0, 0.88, 0.18), vec3(1.0, 0.12, 0.035), uTowerAlign);
    vec3 signalColor = mix(coldSignal, hotSignal, uTowerAlign);
    float hotCore = pow(towerGlow, 0.68) * uTowerAlign * (0.65 + towerPulse * 1.35);
    vec3 neon = signalColor * (spec * (2.25 + uTowerAlign * 1.4) + rim * 0.22 * vShine + towerGlow * (0.5 + towerPulse) * (0.85 + uTowerAlign * 1.55));
    float wetSheen = pow(max(0.0, 1.0 - abs(N.y)), 2.0) * vShine * 0.18;
    float horizonSignal = exp(-abs(vWorld.y - 18.0) * 0.018) * exp(-abs(vWorld.z + 1800.0) * 0.00055);
    float grit = fbm2(vWorld.xz * 0.12);
    float fine = fbm2(vWorld.xz * 0.85 + vec2(uTime * 0.02, 0.0));
    float roadMask = smoothstep(24.0, 8.0, abs(vWorld.x));
    float colorMax = max(vColor.r, max(vColor.g, vColor.b));
    float colorMin = min(vColor.r, min(vColor.g, vColor.b));
    float grayness = 1.0 - smoothstep(0.010, 0.040, colorMax - colorMin);
    float concreteMask = smoothstep(0.026, 0.045, colorMax) * (1.0 - smoothstep(0.115, 0.155, colorMax)) * grayness * (1.0 - roadMask * 0.6);
    float nonConcrete = 1.0 - concreteMask;
    float crack = 1.0 - smoothstep(0.025, 0.09, abs(noise(vWorld.xz * vec2(0.10, 0.42)) - 0.51));
    float tarLine = 1.0 - smoothstep(0.015, 0.07, abs(fract(vWorld.x * 0.09 + noise(vWorld.xz * 0.025) * 0.2) - 0.5));
    float puddleMask = smoothstep(0.62, 0.88, fbm2(vWorld.xz * 0.055 + vec2(4.0, -9.0))) * roadMask;
    float ripple = sin(length(fract(vWorld.xz * 0.075) - 0.5) * 48.0 - uTime * 5.5) * 0.5 + 0.5;
    float gravel = smoothstep(0.45, 0.95, hash(floor(vWorld.xz * 1.7))) * (1.0 - roadMask);
    float oxide = smoothstep(0.62, 0.98, fbm2(vWorld.xz * 0.032 + vec2(19.0, 3.0)));
    float phosphorDust = smoothstep(0.80, 0.99, fbm2(vWorld.xz * 0.22 + vec2(uTime * 0.025, -7.0)));
    float wetVein = 1.0 - smoothstep(0.018, 0.095, abs(noise(vWorld.xz * vec2(0.18, 0.06) + vec2(2.0, uTime * 0.015)) - 0.5));
    float mineralFleck = smoothstep(0.965, 1.0, hash(floor(vWorld.xz * 5.5))) * (1.0 - roadMask);
    float scanResidue = smoothstep(0.78, 0.99, fbm2(vec2(vWorld.x * 0.75, vWorld.z * 0.035 + uTime * 0.08)));
    float microScratch = 1.0 - smoothstep(0.006, 0.032, abs(fract(vWorld.x * 1.9 + noise(vWorld.xz * 0.18) * 0.45) - 0.5));
    float laminate = smoothstep(0.35, 0.95, fbm2(vWorld.xz * vec2(0.018, 0.11) + vec2(8.0, 2.0)));
    float ghostTile = smoothstep(0.83, 1.0, hash(floor(vWorld.xz * vec2(0.42, 0.24)))) * roadMask;
    float dampEdge = smoothstep(10.0, 36.0, abs(vWorld.x)) * smoothstep(0.22, 0.82, fbm2(vWorld.xz * 0.042));
    float proceduralShine = clamp(vShine + puddleMask * 0.55 + roadMask * 0.12 + wetVein * 0.18 + mineralFleck * 0.28 - crack * 0.18, 0.0, 1.45);
    float bioFilm = smoothstep(0.44, 0.86, organic(vWorld.xz * 0.030 + vec2(4.0, -11.0))) * (1.0 - roadMask * 0.45);
    float lichen = smoothstep(0.58, 0.92, organic(vWorld.xz * 0.095 + vec2(13.0, 5.0))) * smoothstep(0.18, 0.95, 1.0 - abs(N.y));
    float seep = smoothstep(0.50, 0.94, organic(vec2(vWorld.x * 0.040, vWorld.z * 0.010 + vWorld.y * 0.035)));
    float softEdge = smoothstep(0.08, 0.82, organic(vWorld.xz * 0.018 + vec2(30.0, 2.0)));
    float leafRot = smoothstep(0.70, 0.98, organic(vWorld.xz * 0.16 + vec2(-6.0, 22.0))) * (1.0 - roadMask);
    float blackMold = smoothstep(0.68, 0.97, organic(vWorld.xz * 0.052 + vec2(-17.0, 31.0))) * smoothstep(0.35, 1.0, 1.0 - N.y);
    float enamelPeel = smoothstep(0.72, 0.98, organic(vWorld.xz * 0.21 + vec2(44.0, -8.0))) * smoothstep(0.22, 0.88, abs(N.y));
    float rainFlow = smoothstep(0.84, 1.0, sin(vWorld.z * 0.13 + organic(vWorld.xz * 0.035) * 9.0 - uTime * 1.8) * 0.5 + 0.5) * dampEdge;
    float rainRipple = smoothstep(0.78, 0.99, sin(length(fract(vWorld.xz * 0.18 + vec2(uTime * 0.04, -uTime * 0.06)) - 0.5) * 34.0 - uTime * 12.0) * 0.5 + 0.5) * (puddleMask + roadMask * 0.25);
    float tarBloom = smoothstep(0.82, 1.0, organic(vWorld.xz * vec2(0.11, 0.028) + vec2(5.0, uTime * 0.06))) * roadMask;
    float algaeEdge = smoothstep(0.60, 0.96, organic(vWorld.xz * 0.11 + vec2(-40.0, 9.0))) * dampEdge;
    float mycelium = 1.0 - smoothstep(0.040, 0.115, cellular(vWorld.xz * 0.18 + organic(vWorld.xz * 0.025) * 2.0));
    float mossBloom = smoothstep(0.46, 0.88, organic(vWorld.xz * 0.065 + vec2(71.0, -12.0))) * (1.0 - roadMask * 0.35);
    float bruise = smoothstep(0.34, 0.92, organic(vWorld.xz * 0.014 + vec2(-90.0, 4.0)));
    float slime = smoothstep(0.72, 0.98, organic(vWorld.xz * 0.18 + vec2(uTime * 0.018, -uTime * 0.010))) * dampEdge;
    float concreteGrain = fbm2(vWorld.xz * 0.85 + vec2(vWorld.y * 0.045, -vWorld.y * 0.020));
    float aggregate = smoothstep(0.58, 0.95, hash(floor(vWorld.xz * 5.4 + vWorld.y * 0.33)));
    float pitting = smoothstep(0.62, 0.98, organic(vWorld.xz * 0.42 + vec2(vWorld.y * 0.08, -14.0)));
    float panelSeamX = 1.0 - smoothstep(0.020, 0.095, abs(fract(vWorld.x * 0.045) - 0.5));
    float panelSeamY = 1.0 - smoothstep(0.020, 0.090, abs(fract(vWorld.y * 0.075) - 0.5));
    float rainStain = smoothstep(0.48, 0.98, organic(vec2(vWorld.x * 0.070, vWorld.y * 0.045 + vWorld.z * 0.010))) * smoothstep(0.10, 1.0, vWorld.y * 0.030);
    float concreteMold = smoothstep(0.52, 0.98, organic(vWorld.xz * 0.095 + vec2(vWorld.y * 0.026, 12.0)));
    float silt = smoothstep(0.48, 0.96, organic(vWorld.xz * 0.075 + vec2(7.0, 70.0))) * roadMask;
    float bumpH = fbm2(vWorld.xz * 0.34);
    float bumpX = fbm2((vWorld.xz + vec2(0.85, 0.0)) * 0.34) - bumpH;
    float bumpZ = fbm2((vWorld.xz + vec2(0.0, 0.85)) * 0.34) - bumpH;
    vec3 Nb = normalize(N + vec3(-bumpX, 0.0, -bumpZ) * (0.55 + proceduralShine * 0.35));
    float microSpec = pow(max(dot(Nb, H), 0.0), 180.0) * proceduralShine;
    float grazingRain = pow(max(0.0, dot(reflect(-L, Nb), V)), 42.0) * (puddleMask + wetVein * 0.55 + mineralFleck * 0.4);
    float screenBand = smoothstep(0.90, 1.0, sin(vWorld.y * 1.7 + vWorld.z * 0.035 + uTime * 4.0) * 0.5 + 0.5);
    vec3 material = vColor;
    material *= 0.74 + grit * 0.46;
    material = mix(material, vec3(0.010, 0.012, 0.013), roadMask * (0.25 + crack * 0.34 + tarLine * 0.18));
    material = mix(material, vec3(0.025, 0.13, 0.17), puddleMask * (0.48 + ripple * 0.12));
    material = mix(material, vec3(0.075, 0.052, 0.030), oxide * (1.0 - roadMask) * 0.22);
    material += vec3(0.030, 0.040, 0.038) * gravel * 0.32;
    material += signalColor * phosphorDust * (0.025 + uGlitch * 0.055);
    material = mix(material, vec3(0.016, 0.060, 0.073), wetVein * (0.16 + uGlitch * 0.08));
    material += vec3(0.32, 0.52, 0.56) * mineralFleck * (0.08 + vShine * 0.12);
    material += signalColor * scanResidue * uGlitch * 0.035;
    material -= microScratch * roadMask * vec3(0.015, 0.018, 0.018);
    material = mix(material, material * vec3(0.62, 0.74, 0.77), laminate * dampEdge * 0.22);
    material += signalColor * ghostTile * (0.018 + uGlitch * 0.04);
    material += vec3(0.10, 0.16, 0.15) * dampEdge * (1.0 - roadMask) * 0.12;
    material = mix(material, vec3(0.020, 0.070, 0.048), bioFilm * (0.16 + dampEdge * 0.18));
    material = mix(material, vec3(0.030, 0.085, 0.050), lichen * 0.22);
    material = mix(material, vec3(0.015, 0.045, 0.052), seep * dampEdge * 0.28);
    material += vec3(0.055, 0.038, 0.018) * leafRot * 0.24;
    material = mix(material, vec3(0.006, 0.010, 0.009), blackMold * 0.32);
    material = mix(material, vec3(0.078, 0.095, 0.088), enamelPeel * 0.18);
    material = mix(material, vec3(0.055, 0.048, 0.034), silt * 0.20);
    material += vec3(0.030, 0.080, 0.086) * rainFlow * 0.22;
    material += signalColor * rainRipple * 0.025;
    material = mix(material, vec3(0.012, 0.020, 0.018), tarBloom * 0.22);
    material = mix(material, vec3(0.020, 0.095, 0.062), algaeEdge * 0.16);
    material = mix(material, vec3(0.018, 0.090, 0.042), mossBloom * 0.28);
    material = mix(material, vec3(0.060, 0.023, 0.035), bruise * 0.12);
    material += vec3(0.13, 0.19, 0.13) * mycelium * 0.18;
    material += vec3(0.02, 0.11, 0.075) * slime * 0.22;
    vec3 concrete = vec3(0.125, 0.132, 0.124) * (0.72 + concreteGrain * 0.48);
    concrete += vec3(0.135, 0.138, 0.124) * aggregate * 0.12;
    concrete -= vec3(0.026, 0.029, 0.026) * pitting * 0.18;
    concrete -= vec3(0.030, 0.034, 0.031) * max(panelSeamX, panelSeamY) * 0.34;
    concrete -= vec3(0.025, 0.030, 0.027) * rainStain * 0.24;
    concrete = mix(concrete, vec3(0.060, 0.070, 0.064), concreteMold * 0.18);
    material = mix(material, concrete, concreteMask * 0.92);
    material *= 0.92 + softEdge * 0.16;
    material += vec3(0.04, 0.015, 0.004) * smoothstep(0.72, 0.98, fine) * (1.0 - roadMask) * 0.35;
    proceduralShine = clamp(proceduralShine + bioFilm * 0.16 + seep * dampEdge * 0.22 + rainFlow * 0.30 + rainRipple * 0.20 + slime * 0.34 - leafRot * 0.08 - blackMold * 0.10, 0.0, 1.75);
    proceduralShine = clamp(mix(proceduralShine, 0.008, concreteMask), 0.0, 1.75);
    vec3 warm = vec3(0.95, 0.78, 0.52) * diff * (0.08 + uTowerAlign * 0.08);
    vec3 color = material * (0.18 + diff * 0.78) + neon + warm;
    color += signalColor * (wetSheen * nonConcrete + puddleMask * proceduralShine * 0.18);
    color += vec3(0.45, 0.84, 0.95) * puddleMask * pow(max(dot(reflect(-L, N), V), 0.0), 18.0) * 0.5;
    color += signalColor * phosphorDust * (0.08 + uGlitch * 0.16) * (0.35 + rim);
    color += signalColor * mineralFleck * proceduralShine * 0.16 * nonConcrete;
    color += signalColor * scanResidue * uGlitch * (0.05 + rim * 0.12) * nonConcrete;
    color += signalColor * ghostTile * (0.10 + rim * 0.08) * nonConcrete;
    color += vec3(0.55, 0.92, 1.0) * dampEdge * proceduralShine * 0.025 * nonConcrete;
    color += signalColor * microSpec * (0.35 + uGlitch * 0.28) * nonConcrete;
    color += vec3(0.65, 0.95, 1.0) * grazingRain * 0.34 * nonConcrete;
    color += vec3(0.08, 0.32, 0.19) * bioFilm * (0.12 + rim * 0.10);
    color += vec3(0.12, 0.24, 0.12) * lichen * diff * 0.18;
    color += signalColor * seep * dampEdge * (0.04 + uGlitch * 0.06) * nonConcrete;
    color += vec3(0.030, 0.20, 0.18) * rainFlow * (0.14 + rim * 0.18) * nonConcrete;
    color += signalColor * rainRipple * (0.08 + rim * 0.12) * nonConcrete;
    color += signalColor * tarBloom * (0.055 + proceduralShine * 0.08) * nonConcrete;
    color += vec3(0.06, 0.28, 0.15) * algaeEdge * (0.08 + diff * 0.12);
    color += vec3(0.06, 0.25, 0.10) * mossBloom * (0.10 + rim * 0.08);
    color += signalColor * mycelium * (0.030 + uGlitch * 0.045) * nonConcrete;
    color += vec3(0.0, 0.20, 0.13) * slime * proceduralShine * 0.16 * nonConcrete;
    color += vec3(0.030, 0.034, 0.031) * aggregate * concreteMask * 0.08;
    color -= vec3(0.060, 0.065, 0.060) * max(panelSeamX, panelSeamY) * concreteMask * 0.58;
    color -= vec3(0.038, 0.042, 0.038) * pitting * concreteMask * 0.34;
    color -= vec3(0.020, 0.018, 0.014) * blackMold * 0.45;
    color += signalColor * enamelPeel * proceduralShine * 0.045 * nonConcrete;
    color += mix(vec3(0.02, 0.38, 0.55), vec3(0.9, 0.16, 0.025), uTowerAlign) * screenBand * uGlitch * 0.09 * nonConcrete;
    color -= crack * roadMask * vec3(0.018, 0.020, 0.018);
    color += mix(vec3(0.02, 0.30, 0.42), vec3(0.62, 0.12, 0.04), uTowerAlign) * horizonSignal * (0.10 + uGlitch * 0.25);
    color += mix(vec3(0.02, 0.42, 0.78), vec3(1.0, 0.20, 0.045), uTowerAlign) * uGlitch * towerPulse * (0.14 + uTowerAlign * 0.18);
    color += hotSignal * hotCore * 1.35;
    vec3 matteConcrete = material * (0.42 + diff * 0.56) + warm * 0.16;
    matteConcrete += vec3(0.030, 0.034, 0.031) * aggregate * 0.10;
    matteConcrete += vec3(0.040, 0.052, 0.054) * (0.22 + uGlitch * 0.08);
    matteConcrete -= vec3(0.024, 0.028, 0.026) * rainStain * 0.20;
    matteConcrete -= vec3(0.032, 0.036, 0.034) * max(panelSeamX, panelSeamY) * 0.25;
    matteConcrete -= vec3(0.020, 0.023, 0.020) * pitting * 0.16;
    matteConcrete = max(matteConcrete, vec3(0.0));
    color = mix(color, matteConcrete, concreteMask);
    float dist = length(uEye - vWorld);
    vec3 fogColor = mix(vec3(0.006, 0.010, 0.014), mix(vec3(0.035, 0.115, 0.155), vec3(0.18, 0.045, 0.025), uTowerAlign), uGlitch);
    float fog = smoothstep(90.0, mix(980.0, 520.0, uGlitch), dist);
    float milk = smoothstep(35.0, mix(420.0, 260.0, uGlitch), dist) * 0.18;
    color = mix(color, fogColor, fog * 0.86);
    color = mix(color, fogColor + vec3(0.020, 0.036, 0.040), milk);
    color = max(color, vec3(0.0));
    FragColor = vec4(color, 1.0);
}
)GLSL";

static const char* kPostVs = R"GLSL(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUv;
out vec2 vUv;
void main() {
    vUv = aUv;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

static const char* kPostFs = R"GLSL(
#version 330 core
in vec2 vUv;
uniform sampler2D uScene;
uniform vec3 uResolution;
uniform float uTime;
uniform float uGlitch;
uniform float uTowerAlign;
uniform float uWrongWay;
uniform float uSignalReached;
uniform float uFalseStart;
out vec4 FragColor;
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
vec3 sampleScene(vec2 uv) { return texture(uScene, clamp(uv, vec2(0.001), vec2(0.999))).rgb; }
float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }
float softNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}
float softOrganic(vec2 p) {
    vec2 q = vec2(softNoise(p + vec2(2.0, 7.0)), softNoise(p + vec2(11.0, 3.0)));
    return softNoise(p + q * 3.0);
}
void main() {
    vec2 uv = vUv;
    float scan = sin((uv.y * uResolution.y + uTime * 12.0) * 3.14159);
    float wrongSurge = pow(uWrongWay, 0.75);
    float signalHeat = clamp(uGlitch + uTowerAlign * 0.55 + wrongSurge * 2.6, 0.0, 3.2);
    float roll = fract(uTime * (0.035 + signalHeat * 0.055));
    float rollBand = smoothstep(0.03, 0.0, abs(fract(uv.y - roll) - 0.5));
    float band = floor(uv.y * mix(80.0, 22.0, clamp(signalHeat, 0.0, 1.0)));
    float tear = (hash(vec2(band, floor(uTime * (18.0 + uTowerAlign * 22.0 + wrongSurge * 90.0)))) - 0.5) * signalHeat * (0.042 + wrongSurge * 0.22);
    uv.x += tear * smoothstep(0.15, 0.95, hash(vec2(band, 7.0))) + rollBand * signalHeat * 0.018;
    vec2 center = uv - 0.5;
    float lineLock = pow(max(0.0, 1.0 - length(center * vec2(1.0, 1.25)) * 2.0), 3.0) * uTowerAlign;
    uv += center * dot(center, center) * (0.08 + lineLock * 0.035);
    uv.x += sin(uv.y * 80.0 + uTime * 21.0) * lineLock * 0.006;
    float block = mix(220.0, 36.0, clamp(signalHeat, 0.0, 1.0));
    vec2 buv = floor(uv * block) / block;
    vec2 sampleUv = mix(uv, buv, clamp(signalHeat, 0.0, 1.0) * (0.42 + wrongSurge * 0.55));
    float ca = 0.0015 + signalHeat * 0.0065 + wrongSurge * 0.018;
    vec3 color;
    color.r = sampleScene(sampleUv + vec2(ca, 0.0)).r;
    color.g = sampleScene(sampleUv).g;
    color.b = sampleScene(sampleUv - vec2(ca, 0.0)).b;
    vec3 ghost = sampleScene(sampleUv + vec2(0.016 + signalHeat * 0.018, 0.0)) * vec3(0.08, 0.28, 0.35);
    ghost += sampleScene(sampleUv - vec2(0.026 + uTowerAlign * 0.02, 0.0)) * vec3(0.25, 0.06, 0.035) * uTowerAlign;
    color += ghost * (0.35 + signalHeat * 0.28);
    vec3 sky = mix(vec3(0.004, 0.006, 0.012), vec3(0.020, 0.060, 0.083), smoothstep(0.05, 0.95, uv.y));
    sky += mix(vec3(0.02, 0.15, 0.22), vec3(0.55, 0.12, 0.025), uTowerAlign) * pow(max(0.0, 1.0 - abs(uv.x - 0.5) * 2.7), 6.0) * (0.35 + signalHeat);
    float star = step(0.9965, hash(floor(uv * uResolution.xy * vec2(0.18, 0.11))));
    sky += vec3(0.35, 0.74, 0.9) * star * smoothstep(0.0, 0.55, 1.0 - uv.y) * (0.18 + uGlitch * 0.22);
    float empty = 1.0 - smoothstep(0.012, 0.08, dot(color, vec3(0.299, 0.587, 0.114)));
    color = mix(color, sky, empty * 0.9);
    vec3 bloom = vec3(0.0);
    vec3 halo = vec3(0.0);
    vec2 px = 1.0 / uResolution.xy;
    vec3 blur4 = sampleScene(sampleUv + vec2(px.x, 0.0)) + sampleScene(sampleUv - vec2(px.x, 0.0));
    blur4 += sampleScene(sampleUv + vec2(0.0, px.y)) + sampleScene(sampleUv - vec2(0.0, px.y));
    vec3 sharpen = color * 5.0 - blur4;
    color = mix(color, sharpen, 0.055 + signalHeat * 0.025);
    for (int i = 1; i <= 7; ++i) {
        float r = float(i) * (1.85 + signalHeat * 3.1);
        float threshold = mix(0.22, 0.16, clamp(signalHeat, 0.0, 1.0));
        float w = 1.0 / (1.0 + float(i) * 0.42);
        bloom += max(sampleScene(sampleUv + vec2(px.x * r, 0.0)) - threshold, 0.0) * w;
        bloom += max(sampleScene(sampleUv - vec2(px.x * r, 0.0)) - threshold, 0.0) * w;
        bloom += max(sampleScene(sampleUv + vec2(0.0, px.y * r)) - threshold, 0.0) * w;
        bloom += max(sampleScene(sampleUv - vec2(0.0, px.y * r)) - threshold, 0.0) * w;
        bloom += max(sampleScene(sampleUv + vec2(px.x * r, px.y * r)) - threshold * 1.15, 0.0) * w * 0.65;
        bloom += max(sampleScene(sampleUv + vec2(-px.x * r, px.y * r)) - threshold * 1.15, 0.0) * w * 0.65;
        bloom += max(sampleScene(sampleUv + vec2(px.x * r, -px.y * r)) - threshold * 1.15, 0.0) * w * 0.45;
        bloom += max(sampleScene(sampleUv + vec2(-px.x * r, -px.y * r)) - threshold * 1.15, 0.0) * w * 0.45;
        if (i <= 4) {
            float hr = float(i) * (10.0 + signalHeat * 8.0);
            float hw = 1.0 / (1.0 + float(i) * 0.75);
            halo += max(sampleScene(sampleUv + vec2(px.x * hr, 0.0)) - 0.12, 0.0) * hw;
            halo += max(sampleScene(sampleUv - vec2(px.x * hr, 0.0)) - 0.12, 0.0) * hw;
            halo += max(sampleScene(sampleUv + vec2(0.0, px.y * hr)) - 0.12, 0.0) * hw * 0.65;
            halo += max(sampleScene(sampleUv - vec2(0.0, px.y * hr)) - 0.12, 0.0) * hw * 0.65;
        }
    }
    color += bloom * mix(vec3(0.055, 0.088, 0.108), vec3(0.125, 0.060, 0.030), uTowerAlign) * (1.05 + signalHeat * 0.65);
    color += halo * mix(vec3(0.030, 0.075, 0.095), vec3(0.105, 0.045, 0.020), uTowerAlign) * (0.58 + signalHeat * 0.32);
    float shaft = pow(max(0.0, 1.0 - abs(center.x) * 2.6), 6.0) * smoothstep(0.82, 0.05, uv.y);
    float shaftNoise = hash(floor(vec2(uv.x * 36.0 + uTime * 2.0, uv.y * 18.0)));
    color += mix(vec3(0.025, 0.17, 0.23), vec3(0.62, 0.13, 0.025), uTowerAlign) * shaft * (0.10 + signalHeat * 0.24) * (0.55 + shaftNoise * 0.45);
    vec2 e = px * vec2(1.5, 1.5);
    float edge = abs(luma(sampleScene(sampleUv + vec2(e.x, 0.0))) - luma(sampleScene(sampleUv - vec2(e.x, 0.0))));
    edge += abs(luma(sampleScene(sampleUv + vec2(0.0, e.y))) - luma(sampleScene(sampleUv - vec2(0.0, e.y))));
    color += mix(vec3(0.02, 0.62, 0.85), vec3(1.0, 0.18, 0.035), uTowerAlign) * edge * (0.18 + signalHeat * 0.25);
    vec3 lockColor = mix(vec3(0.05, 0.75, 1.0), vec3(1.0, 0.28, 0.04), uTowerAlign);
    color += lockColor * lineLock * (0.18 + uTowerAlign * 0.45);
    color += vec3(1.0, 0.82, 0.18) * pow(max(0.0, 1.0 - abs(center.y) * 18.0), 2.0) * uTowerAlign * 0.13;
    float noise = hash(uv * uResolution.xy + floor(uTime * 60.0));
    color += (noise - 0.5) * (0.05 + signalHeat * 0.2);
    float analogMottle = hash(floor(uv * uResolution.xy / vec2(5.0, 3.0)) + floor(uTime * 24.0));
    color += (analogMottle - 0.5) * vec3(0.020, 0.035, 0.040) * (0.35 + signalHeat);
    float organicBloom = softNoise(uv * vec2(9.0, 5.0) + vec2(uTime * 0.035, -uTime * 0.02));
    organicBloom = smoothstep(0.22, 0.92, organicBloom);
    color += mix(vec3(0.0, 0.08, 0.06), vec3(0.16, 0.035, 0.012), uTowerAlign) * organicBloom * (0.035 + signalHeat * 0.035);
    float lensDrop = softNoise(uv * vec2(22.0, 14.0) + vec2(-uTime * 0.025, uTime * 0.018));
    lensDrop = smoothstep(0.82, 0.99, lensDrop) * smoothstep(0.10, 0.55, length(center));
    vec2 smearUv = sampleUv + vec2(0.0, lensDrop * (0.012 + signalHeat * 0.010));
    color = mix(color, sampleScene(smearUv) + vec3(0.02, 0.08, 0.10) * lensDrop, lensDrop * 0.20);
    float wetGlass = softNoise(uv * vec2(4.5, 12.0) + vec2(uTime * 0.018, uTime * 0.055));
    float verticalRun = smoothstep(0.68, 0.98, wetGlass) * smoothstep(0.02, 0.42, uv.y);
    float runDistort = verticalRun * (0.0035 + signalHeat * 0.0025);
    vec3 runSample = sampleScene(sampleUv + vec2(sin(uv.y * 32.0 + uTime) * runDistort, runDistort * 2.8));
    color = mix(color, runSample, verticalRun * 0.16);
    color += vec3(0.015, 0.065, 0.075) * verticalRun * (0.16 + signalHeat * 0.10);
    float rainHaze = smoothstep(0.10, 0.95, softNoise(vec2(uv.x * 2.8 + uTime * 0.025, uv.y * 7.0 - uTime * 0.12)));
    color += mix(vec3(0.010, 0.052, 0.062), vec3(0.090, 0.030, 0.014), uTowerAlign) * rainHaze * (0.052 + signalHeat * 0.045);
    float fungalVeil = smoothstep(0.54, 0.98, softOrganic(uv * vec2(3.2, 5.8) + vec2(uTime * 0.012, -uTime * 0.018)));
    float vein = 1.0 - smoothstep(0.018, 0.075, abs(softOrganic(uv * vec2(18.0, 9.0) + vec2(4.0, uTime * 0.03)) - 0.5));
    color += mix(vec3(0.0, 0.045, 0.028), vec3(0.105, 0.024, 0.010), uTowerAlign) * fungalVeil * (0.025 + signalHeat * 0.025);
    color += mix(vec3(0.02, 0.14, 0.09), vec3(0.16, 0.035, 0.012), uTowerAlign) * vein * (0.012 + signalHeat * 0.018);
    float breath = sin(uTime * 0.85 + softNoise(vec2(uTime * 0.07, 2.0)) * 2.0) * 0.5 + 0.5;
    color += mix(vec3(0.020, 0.070, 0.080), vec3(0.12, 0.035, 0.014), uTowerAlign) * breath * (0.045 + signalHeat * 0.045);
    float spectral = sin((uv.x + uv.y * 0.37) * 44.0 + uTime * 0.9) * 0.5 + 0.5;
    spectral *= smoothstep(0.15, 0.85, softNoise(uv * vec2(6.0, 3.0) + vec2(2.0, uTime * 0.03)));
    color += mix(vec3(0.0, 0.10, 0.13), vec3(0.20, 0.045, 0.012), uTowerAlign) * spectral * (0.018 + signalHeat * 0.024);
    float gray = luma(color);
    vec3 cctvTone = vec3(gray * 0.72, gray * 0.96, gray * 0.86);
    float exposurePulse = 0.96 + 0.035 * sin(uTime * 1.7) + 0.018 * sin(uTime * 7.3);
    color = mix(color, cctvTone, 0.18);
    color *= exposurePulse;
    float cctvRows = sin(uv.y * uResolution.y * 0.62 + floor(uTime * 15.0) * 0.7) * 0.5 + 0.5;
    color *= 0.965 + cctvRows * 0.030;
    vec2 cblock = floor(uv * uResolution.xy / vec2(16.0, 12.0));
    float blockNoise = hash(cblock + floor(uTime * 8.0));
    color += (blockNoise - 0.5) * vec3(0.018, 0.030, 0.026) * (0.24 + signalHeat * 0.18 + wrongSurge * 1.4);
    float wrongBand = step(0.44, hash(vec2(floor(uv.y * 42.0), floor(uTime * (18.0 + wrongSurge * 32.0)))));
    color += vec3(0.08, 0.42, 0.48) * wrongBand * wrongSurge * (0.24 + signalHeat * 0.12);
    color.r += wrongSurge * 0.16 * sin(uv.y * 120.0 + uTime * 29.0);
    color.gb -= wrongSurge * 0.085 * sin(uv.x * 92.0 - uTime * 23.0);
    color = mix(color, vec3(hash(floor(uv * uResolution.xy * 0.08 + uTime * 25.0))), wrongSurge * 0.18);
    color += mix(vec3(0.10, 0.38, 0.42), vec3(1.0, 0.42, 0.08), uTowerAlign) * uSignalReached * (0.24 + 0.12 * sin(uTime * 9.0));
    color += vec3(0.25, 0.95, 1.0) * uFalseStart * (0.55 + 0.25 * sin(uTime * 35.0));
    color = mix(color, vec3(luma(color)) + vec3(0.14, 0.30, 0.28), uSignalReached * 0.22 + uFalseStart * 0.35);
    vec3 bleach = color * color * (3.0 - 2.0 * color);
    color = mix(color, bleach, 0.13 + signalHeat * 0.045);
    float filmWeave = sin(uv.y * 940.0 + uTime * 19.0 + hash(vec2(floor(uTime * 12.0), 3.0)) * 6.28);
    color += filmWeave * vec3(0.010, 0.016, 0.018) * (0.22 + signalHeat * 0.25);
    color *= 0.88 + scan * (0.035 + signalHeat * 0.055);
    color += mix(vec3(0.0, 0.35, 0.55), vec3(0.75, 0.16, 0.025), uTowerAlign) * pow(max(0.0, 1.0 - length(center) * 1.4), 2.5) * 0.2;
    float grille = sin(uv.x * uResolution.x * 3.14159);
    color *= 0.965 + 0.035 * grille;
    float slot = mod(floor(uv.x * uResolution.x), 3.0);
    vec3 phosphor = slot < 1.0 ? vec3(1.10, 0.92, 0.88) : (slot < 2.0 ? vec3(0.88, 1.08, 0.92) : vec3(0.88, 0.94, 1.12));
    color *= mix(vec3(1.0), phosphor, 0.16 + signalHeat * 0.05);
    color += vec3(0.18, 0.65, 0.78) * rollBand * (0.05 + signalHeat * 0.16);
    color += vec3(0.04, 0.18, 0.20) * pow(max(0.0, 1.0 - abs(uv.y - 0.56) * 4.0), 8.0) * (0.12 + signalHeat * 0.15);
    float cornerBleed = pow(max(0.0, length(center) - 0.32), 2.0);
    color += mix(vec3(0.015, 0.075, 0.090), vec3(0.16, 0.032, 0.014), uTowerAlign) * cornerBleed * (0.13 + signalHeat * 0.12);
    float ganzfeld = smoothstep(0.20, 0.82, rainHaze) * (0.10 + signalHeat * 0.05);
    color = mix(color, mix(vec3(0.045, 0.100, 0.115), vec3(0.145, 0.060, 0.035), uTowerAlign), ganzfeld);
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(0.88, 0.92, 1.0));
    color *= vec3(0.92, 1.03, 1.12);
    color *= 1.0 - smoothstep(0.55, 0.9, length(center));
    FragColor = vec4(color, 1.0);
}
)GLSL";

static void setupVertexStream(GLuint& vao, GLuint& vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, p)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, n)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, c)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, shine)));
}
static void resizeFramebuffer() {
    if (g_width == g_fbWidth && g_height == g_fbHeight && g_fbo) return;
    g_fbWidth = g_width;
    g_fbHeight = g_height;
    if (!g_fbo) {
        glGenFramebuffers(1, &g_fbo);
        glGenTextures(1, &g_colorTex);
        glGenRenderbuffers(1, &g_depthRb);
    }
    glBindTexture(GL_TEXTURE_2D, g_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_fbWidth, g_fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depthRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_fbWidth, g_fbHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_colorTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_depthRb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
static bool initRenderer() {
    if (!loadGlFunctions()) return false;
    g_sceneProgram = createProgram(kSceneVs, kSceneFs);
    g_postProgram = createProgram(kPostVs, kPostFs);
    setupVertexStream(g_sceneVao, g_sceneVbo);
    setupVertexStream(g_lineVao, g_lineVbo);
    float quad[] = {
        -1, -1, 0, 0,  1, -1, 1, 0,  1,  1, 1, 1,
        -1, -1, 0, 0,  1,  1, 1, 1, -1,  1, 0, 1
    };
    glGenVertexArrays(1, &g_quadVao);
    glGenBuffers(1, &g_quadVbo);
    glBindVertexArray(g_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    resizeFramebuffer();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return true;
}

static void render() {
    resizeFramebuffer();
    buildWorld();
    float g = glitchAmount();
    float cp = std::cos(g_pitch);
    Vec3 dir{std::sin(g_yaw) * cp, std::sin(g_pitch), std::cos(g_yaw) * cp};
    float towerZ = currentTowerZ();
    Vec3 towerFocus{0.0f, terrainHeight(0.0f, towerZ) + 118.0f, towerZ};
    float towerDot = dot(norm(dir), norm(sub(towerFocus, g_player)));
    float towerAlign = smoothstep(0.965f, 0.997f, towerDot);
    towerAlign *= 0.72f + 0.28f * (0.5f + 0.5f * std::sin(g_time * 18.0f));
    Mat4 view = lookAt(g_player, add(g_player, dir), {0, 1, 0});
    Mat4 proj = perspective(68.0f + g * 8.0f, static_cast<float>(g_width) / std::max(1, g_height), 0.08f, 4300.0f);
    Mat4 mvp = multiply(proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glViewport(0, 0, g_width, g_height);
    glClearColor(0.004f + g * 0.02f, 0.007f, 0.012f + g * 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_sceneProgram);
    glUniformMatrix4fv(glGetUniformLocation(g_sceneProgram, "uMvp"), 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(glGetUniformLocation(g_sceneProgram, "uView"), 1, GL_FALSE, view.m);
    glUniform3f(glGetUniformLocation(g_sceneProgram, "uEye"), g_player.x, g_player.y, g_player.z);
    glUniform1f(glGetUniformLocation(g_sceneProgram, "uTime"), g_time);
    glUniform1f(glGetUniformLocation(g_sceneProgram, "uGlitch"), g);
    glUniform1f(glGetUniformLocation(g_sceneProgram, "uTowerAlign"), towerAlign);
    glUniform1f(glGetUniformLocation(g_sceneProgram, "uTowerZ"), towerZ);

    glDisable(GL_CULL_FACE);
    glBindVertexArray(g_sceneVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_sceneVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_triangles.size() * sizeof(Vertex)), g_triangles.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_triangles.size()));

    glLineWidth(1.35f + g * 1.2f);
    glBindVertexArray(g_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_lines.size() * sizeof(Vertex)), g_lines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_lines.size()));
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_postProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_colorTex);
    glUniform1i(glGetUniformLocation(g_postProgram, "uScene"), 0);
    glUniform1f(glGetUniformLocation(g_postProgram, "uTime"), g_time);
    glUniform1f(glGetUniformLocation(g_postProgram, "uGlitch"), g);
    glUniform1f(glGetUniformLocation(g_postProgram, "uTowerAlign"), towerAlign);
    glUniform1f(glGetUniformLocation(g_postProgram, "uWrongWay"), g_wrongWaySignal);
    glUniform1f(glGetUniformLocation(g_postProgram, "uSignalReached"), g_signalReached);
    glUniform1f(glGetUniformLocation(g_postProgram, "uFalseStart"), g_falseStartFlash);
    glUniform3f(glGetUniformLocation(g_postProgram, "uResolution"), static_cast<float>(g_width), static_cast<float>(g_height), 0.0f);
    glBindVertexArray(g_quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
    SwapBuffers(g_hdc);
}

static void updateMouse() {
    if (!g_focused || !g_mouseCaptured) return;
    RECT rc;
    GetClientRect(g_hwnd, &rc);
    POINT center{(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
    ClientToScreen(g_hwnd, &center);
    POINT p;
    GetCursorPos(&p);
    int dx = p.x - center.x;
    int dy = p.y - center.y;
    if (dx || dy) {
        g_yaw += dx * 0.0022f;
        g_pitch = clampf(g_pitch - dy * 0.0020f, -1.45f, 1.45f);
        SetCursorPos(center.x, center.y);
    }
}
static bool keyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }
static bool collidesWithBuilding(Vec3 position) {
    float footY = position.y - kPlayerEyeHeight;
    float headY = position.y + 0.25f;
    float r = kPlayerFootRadius;
    for (const Aabb& b : g_buildingColliders) {
        if (headY < b.minY || footY > b.maxY) continue;
        if (position.x + r < b.minX || position.x - r > b.maxX) continue;
        if (position.z + r < b.minZ || position.z - r > b.maxZ) continue;
        return true;
    }
    return false;
}
static void update(float dt) {
    updateMouse();
    float speed = keyDown(VK_SHIFT) ? 34.0f : 18.0f;
    Vec3 forward{std::sin(g_yaw), 0.0f, std::cos(g_yaw)};
    Vec3 right{std::cos(g_yaw), 0.0f, -std::sin(g_yaw)};
    Vec3 move{0, 0, 0};
    if (keyDown('W')) move = add(move, forward);
    if (keyDown('S')) move = sub(move, forward);
    if (keyDown('D')) move = sub(move, right);
    if (keyDown('A')) move = add(move, right);
    float len = std::sqrt(move.x * move.x + move.z * move.z);
    float towerZ = currentTowerZ();
    Vec3 flatToTower = norm(sub({0.0f, g_player.y, towerZ}, g_player));
    Vec3 flatLook = norm(forward);
    float lookAway = clampf(-dot(flatLook, flatToTower), 0.0f, 1.0f);
    float targetWrongLook = smoothstep(0.18f, 0.88f, lookAway);
    float targetWrongMove = 0.0f;
    if (len > 0.001f) {
        move = mul(move, 1.0f / len);
        float away = clampf(-dot(move, flatToTower), 0.0f, 1.0f);
        targetWrongMove = smoothstep(0.15f, 0.85f, away);
        Vec3 next = add(g_player, mul(move, speed * dt));
        next.x = clampf(next.x, -900.0f, 900.0f);
        next.z = clampf(next.z, finalTowerZ() - 120.0f, 620.0f);
        float currentGround = playerGroundHeight(g_player.x, g_player.z);
        float nextGround = playerGroundHeight(next.x, next.z);
        next.y = nextGround + kPlayerEyeHeight;
        if (nextGround < currentGround + 2.8f && !collidesWithBuilding(next)) {
            g_player.x = next.x;
            g_player.z = next.z;
        }
    }
    float targetWrongWay = clampf(targetWrongMove * 0.72f + targetWrongLook * 0.62f, 0.0f, 1.0f);
    if (targetWrongWay > 0.05f) {
        g_wrongWaySustain = clampf(g_wrongWaySustain + dt * (0.34f + targetWrongWay * 0.34f), 0.0f, 1.0f);
    } else {
        g_wrongWaySustain = clampf(g_wrongWaySustain - dt * 0.70f, 0.0f, 1.0f);
    }
    float sustainedWrongWay = targetWrongWay * (0.18f + g_wrongWaySustain * 0.82f);
    float response = sustainedWrongWay > g_wrongWaySignal ? 4.0f : 2.6f;
    g_wrongWaySignal += (sustainedWrongWay - g_wrongWaySignal) * clampf(dt * response, 0.0f, 1.0f);
    g_player.x = clampf(g_player.x, -900.0f, 900.0f);
    g_player.z = clampf(g_player.z, finalTowerZ() - 120.0f, 620.0f);
    float towerDistance = std::sqrt(g_player.x * g_player.x + (g_player.z - towerZ) * (g_player.z - towerZ));
    if (towerDistance < kSignalReachDistance) {
        if (g_signalStage < kFinalSignalStage) {
            ++g_signalStage;
            g_falseStartFlash = 1.0f;
            g_wrongWaySignal = 0.0f;
            g_wrongWaySustain = 0.0f;
        } else {
            g_signalReached = clampf(g_signalReached + dt * 0.45f, 0.0f, 1.0f);
        }
    } else {
        g_signalReached = clampf(g_signalReached - dt * 0.10f, 0.0f, 1.0f);
    }
    g_falseStartFlash = clampf(g_falseStartFlash - dt * 0.55f, 0.0f, 1.0f);
    float bob = std::sin(g_time * 7.0f) * (len > 0.001f ? 0.09f : 0.025f);
    float groundY = playerGroundHeight(g_player.x, g_player.z) + kPlayerEyeHeight;
    if (g_isGrounded && keyDown(VK_SPACE)) {
        g_verticalVelocity = 13.5f;
        g_isGrounded = false;
    }
    if (!g_isGrounded) {
        g_verticalVelocity -= 32.0f * dt;
        g_player.y += g_verticalVelocity * dt;
        if (g_player.y <= groundY) {
            g_player.y = groundY;
            g_verticalVelocity = 0.0f;
            g_isGrounded = true;
        }
    } else {
        g_player.y = groundY + bob;
    }
}

static void toggleFullscreen() {
    g_fullscreen = !g_fullscreen;
    if (g_fullscreen) {
        g_windowedStyle = GetWindowLong(g_hwnd, GWL_STYLE);
        GetWindowRect(g_hwnd, &g_windowedRect);
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);
        SetWindowLong(g_hwnd, GWL_STYLE, g_windowedStyle & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(g_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    } else {
        SetWindowLong(g_hwnd, GWL_STYLE, g_windowedStyle);
        SetWindowPos(g_hwnd, nullptr, g_windowedRect.left, g_windowedRect.top,
                     g_windowedRect.right - g_windowedRect.left,
                     g_windowedRect.bottom - g_windowedRect.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}
static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE:
        g_width = std::max(1, static_cast<int>(LOWORD(lp)));
        g_height = std::max(1, static_cast<int>(HIWORD(lp)));
        return 0;
    case WM_SETFOCUS:
        g_focused = true;
        if (g_mouseCaptured) ShowCursor(FALSE);
        return 0;
    case WM_KILLFOCUS:
        g_focused = false;
        ShowCursor(TRUE);
        return 0;
    case WM_LBUTTONDOWN:
        g_mouseCaptured = true;
        ShowCursor(FALSE);
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) {
            if (g_mouseCaptured) {
                g_mouseCaptured = false;
                ShowCursor(TRUE);
            } else {
                g_running = false;
            }
        }
        if (wp == VK_F11) toggleFullscreen();
        return 0;
    case WM_CLOSE:
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}

static bool createModernContext(HWND hwnd) {
    g_hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pf = ChoosePixelFormat(g_hdc, &pfd);
    if (!pf || !SetPixelFormat(g_hdc, pf, &pfd)) return false;

    HGLRC temp = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, temp);
    auto createContextAttribs = reinterpret_cast<WGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (createContextAttribs) {
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        g_glrc = createContextAttribs(g_hdc, nullptr, attribs);
    }
    if (!g_glrc) g_glrc = temp;
    wglMakeCurrent(nullptr, nullptr);
    if (g_glrc != temp) wglDeleteContext(temp);
    return g_glrc && wglMakeCurrent(g_hdc, g_glrc);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd) {
    WNDCLASSA wc{};
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "DeadChannelWindow";
    RegisterClassA(&wc);

    RECT rect{0, 0, g_width, g_height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowA("DeadChannelWindow", "Dead Channel - Modern OpenGL 3.3", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                           nullptr, nullptr, instance, nullptr);
    if (!g_hwnd || !createModernContext(g_hwnd) || !initRenderer()) {
        MessageBoxA(nullptr, "Could not initialize a shader-based OpenGL renderer.", "Dead Channel", MB_ICONERROR);
        return 1;
    }
    buildWorld();
    initTelemetry();

    ShowWindow(g_hwnd, showCmd);
    ShowCursor(FALSE);
    QueryPerformanceFrequency(&g_frequency);
    QueryPerformanceCounter(&g_lastCounter);
    timeBeginPeriod(1);

    MSG msg{};
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = static_cast<float>(now.QuadPart - g_lastCounter.QuadPart) / static_cast<float>(g_frequency.QuadPart);
        g_lastCounter = now;
        dt = clampf(dt, 0.0f, 0.05f);
        g_time += dt;
        update(dt);
        updateTelemetry(dt);
        render();
        Sleep(1);
    }

    shutdownTelemetry();
    timeEndPeriod(1);
    if (g_glrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(g_glrc);
    }
    if (g_hdc) ReleaseDC(g_hwnd, g_hdc);
    return 0;
}
