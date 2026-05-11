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
    float thirst;
    float waterFuel;
    float jetpackPulse;
    float health;
    int32_t isGrounded;
    float currentGateX, currentGateY, currentGateZ;
    float currentGateDistance;
    float currentGatePlanarDistance;
    float currentGateThreeDimensionalDistance;
    float realGateX, realGateY, realGateZ;
    float realGateDistance;
    float realGatePlanarDistance;
    float realGateThreeDimensionalDistance;
    int32_t gateFound;
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
static constexpr float kSceneRenderScale = 0.72f;
static float g_time = 0.0f;
static float g_telemetryFileTimer = 0.0f;
static Vec3 g_lastTelemetryPlayer{};
static bool g_hasTelemetryPlayer = false;
static LARGE_INTEGER g_lastCounter{};
static LARGE_INTEGER g_frequency{};
static Vec3 g_player{0.0f, 8.0f, 260.0f};
static float g_walkBobPhase = 0.0f;
static float g_yaw = 3.14159f;
static float g_pitch = -0.05f;
static float g_verticalVelocity = 0.0f;
static bool g_isGrounded = true;
static float g_wrongWaySignal = 0.0f;
static float g_wrongWaySustain = 0.0f;
static float g_signalReached = 0.0f;
static float g_falseStartFlash = 0.0f;
static float g_atomicTime = -1000.0f;
static Vec3 g_atomicOrigin{0.0f, 0.0f, 0.0f};
static bool g_atomicTestKeyWasDown = false;
static bool g_atomicEnded = false;
static float g_drinkPulse = 0.0f;
static float g_thirst = 0.0f;
static float g_waterFuel = 0.0f;
static float g_jetpackPulse = 0.0f;
static float g_playerHealth = 1.0f;
static float g_lastWorldX = 999999.0f;
static float g_lastWorldZ = 999999.0f;
static int g_lastWorldStage = -1;
static bool g_worldDirty = true;
static int g_signalStage = 0;

static GLuint g_sceneProgram = 0;
static GLuint g_postProgram = 0;
static GLuint g_sceneVao = 0;
static GLuint g_sceneVbo = 0;
static GLuint g_atomicVao = 0;
static GLuint g_atomicVbo = 0;
static GLuint g_lineVao = 0;
static GLuint g_lineVbo = 0;
static GLuint g_fxLineVao = 0;
static GLuint g_fxLineVbo = 0;
static GLuint g_quadVao = 0;
static GLuint g_quadVbo = 0;
static GLuint g_fbo = 0;
static GLuint g_colorTex = 0;
static GLuint g_ticketTex = 0;
static GLuint g_depthRb = 0;
static std::vector<Vertex> g_triangles;
static std::vector<Vertex> g_atomicTriangles;
static std::vector<Vertex> g_lines;
static std::vector<Vertex> g_frameLines;
static std::vector<Aabb> g_buildingColliders;
static bool g_sceneBuffersDirty = true;
static HANDLE g_telemetryMapping = nullptr;
static SignalTelemetry* g_telemetry = nullptr;
static char g_telemetryPath[MAX_PATH] = {};

struct SceneUniforms {
    GLint mvp = -1;
    GLint view = -1;
    GLint eye = -1;
    GLint time = -1;
    GLint glitch = -1;
    GLint towerAlign = -1;
    GLint figureX = -1;
    GLint towerZ = -1;
};
struct PostUniforms {
    GLint scene = -1;
    GLint ticket = -1;
    GLint time = -1;
    GLint glitch = -1;
    GLint towerAlign = -1;
    GLint wrongWay = -1;
    GLint signalReached = -1;
    GLint falseStart = -1;
    GLint drinkPulse = -1;
    GLint thirst = -1;
    GLint waterFuel = -1;
    GLint jetpackPulse = -1;
    GLint health = -1;
    GLint atomic = -1;
    GLint atomicFade = -1;
    GLint resolution = -1;
};
static SceneUniforms g_sceneUniforms;
static PostUniforms g_postUniforms;

static constexpr float kTowerZ = -680.0f;
static constexpr float kStartZ = 260.0f;
static constexpr float kSignalReachDistance = 82.0f;
static constexpr float kFalseStartSpacing = 760.0f;
static constexpr int kFinalSignalStage = 22;
static constexpr float kPlayerEyeHeight = 5.8f;
static constexpr float kPlayerFootRadius = 1.35f;
static constexpr float kPlayerGroundClearance = 0.38f;
static constexpr float kTerrainMeshStep = 18.0f;
static constexpr float kAtomicDropSeconds = 2.4f;
static constexpr uint32_t kTelemetryMagic = 0x45414754u; // EAGT
static constexpr float kTelemetryFileInterval = 2.0f;

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
static float atomicIntensity() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    if (t < 0.0f) return smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime) * 0.30f;
    float flash = smoothstep(5.0f, 0.0f, t);
    float fluorescentDeath = smoothstep(0.2f, 3.2f, t) * smoothstep(30.0f, 6.0f, t) * 0.95f;
    return clampf(flash + fluorescentDeath, 0.0f, 1.0f);
}
static float atomicEndFade() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    return t <= 9.0f ? 0.0f : smoothstep(9.0f, 17.0f, t);
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
    float pulse = 0.5f + 0.5f * std::sin(g_time * (0.7f + p * 1.8f));
    return 0.025f + smoothstep(0.05f, 1.0f, p) * (0.07f + pulse * 0.05f) + g_falseStartFlash * 0.18f;
}
static float terrainHeight(float x, float z) {
    float tile = std::floor((x + 900.0f) / 18.0f) + std::floor((z + 900.0f) / 18.0f);
    float seam = (std::fmod(std::fabs(x), 18.0f) < 0.34f || std::fmod(std::fabs(z), 18.0f) < 0.34f) ? -0.012f : 0.0f;
    float camber = smoothstep(520.0f, 920.0f, std::fabs(x)) * 0.35f;
    return std::sin(tile * 1.7f) * 0.006f + seam + camber;
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

static Vec3 sightingPositionForStage(int stage) {
    float z = kTowerZ - kFalseStartSpacing * static_cast<float>(stage);
    float lane = static_cast<float>((stage % 5) - 2);
    float x = lane * 72.0f + std::sin(stage * 1.91f) * 22.0f;
    if (stage == 0) x = 0.0f;
    float y = renderedSurfaceHeight(x, z) + 34.0f;
    return {x, y, z};
}

static Vec3 currentSightingPosition() {
    return sightingPositionForStage(g_signalStage);
}
static bool keyDown(int vk);
static Vec3 encounterSightingPosition() {
    Vec3 p = currentSightingPosition();
    return p;
}
static void triggerAtomicBlast() {
    Vec3 gate = currentSightingPosition();
    g_atomicOrigin = {gate.x, renderedSurfaceHeight(gate.x, gate.z), gate.z + 34.0f};
    g_atomicTime = 0.0f;
    g_atomicEnded = false;
    g_falseStartFlash = std::max(g_falseStartFlash, 0.85f);
}
static void updateAtomicBlast(float dt) {
    bool testKeyDown = keyDown('T');
    if (testKeyDown && !g_atomicTestKeyWasDown) triggerAtomicBlast();
    g_atomicTestKeyWasDown = testKeyDown;
    if (g_atomicTime > -100.0f) {
        g_atomicTime += dt;
        float a = atomicIntensity();
        g_falseStartFlash = std::max(g_falseStartFlash, a * 0.78f);
        if (atomicEndFade() >= 0.995f) g_atomicEnded = true;
    }
}

static void initTelemetry() {
    g_telemetryMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                            sizeof(SignalTelemetry), "Local\\EndlessAirportGateTelemetry");
    if (g_telemetryMapping) {
        g_telemetry = static_cast<SignalTelemetry*>(
            MapViewOfFile(g_telemetryMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SignalTelemetry)));
    }
    if (g_telemetry) {
        std::memset(g_telemetry, 0, sizeof(SignalTelemetry));
        g_telemetry->magic = kTelemetryMagic;
        g_telemetry->version = 5;
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
        std::snprintf(g_telemetryPath, sizeof(g_telemetryPath), "%sEndlessAirportGateTelemetry.json", exePath);
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
                 "  \"magic\": \"EAGT\",\n"
                 "  \"version\": %u,\n"
                 "  \"byteSize\": %u,\n"
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
                 "  \"currentGate\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"currentGateDistance\": %.3f,\n"
                 "  \"currentGatePlanarDistance\": %.3f,\n"
                 "  \"currentGateThreeDimensionalDistance\": %.3f,\n"
                 "  \"realGate\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"realGateDistance\": %.3f,\n"
                 "  \"realGatePlanarDistance\": %.3f,\n"
                 "  \"realGateThreeDimensionalDistance\": %.3f,\n"
                 "  \"visibleTowerZ\": %.3f,\n"
                 "  \"falseStartStage\": %d,\n"
                 "  \"finalStage\": %d,\n"
                 "  \"signalReached\": %.3f,\n"
                 "  \"falseStartFlash\": %.3f,\n"
                 "  \"loopDepth\": %.3f,\n"
                 "  \"gateProximity\": %.3f,\n"
                 "  \"walkwayPulse\": %.3f,\n"
                 "  \"onMovingWalkway\": %s,\n"
                 "  \"health\": %.3f,\n"
                 "  \"isGrounded\": %s,\n"
                 "  \"gate47\": {\"stage\": %d, \"finalStage\": %d, \"loopFlash\": %.3f, \"found\": %s}\n"
                 "}\n",
                 t.version, t.byteSize, t.sequence, t.timeSeconds, t.realDistance, t.planarDistance,
                 t.threeDimensionalDistance, t.playerX, t.playerY, t.playerZ, t.playerVelocityX,
                 t.playerVelocityY, t.playerVelocityZ, t.playerSpeed, t.playerHorizontalSpeed,
                 t.playerFacingX, t.playerFacingY, t.playerFacingZ, t.playerMoveDirX, t.playerMoveDirY,
                 t.playerMoveDirZ, t.playerYaw, t.playerPitch, t.signalX, t.signalY, t.signalZ,
                 t.currentGateX, t.currentGateY, t.currentGateZ, t.currentGateDistance,
                 t.currentGatePlanarDistance, t.currentGateThreeDimensionalDistance,
                 t.realGateX, t.realGateY, t.realGateZ, t.realGateDistance,
                 t.realGatePlanarDistance, t.realGateThreeDimensionalDistance,
                 t.visibleTowerZ, t.falseStartStage, t.finalStage, t.signalReached, t.falseStartFlash,
                 t.thirst, t.waterFuel, t.jetpackPulse, t.jetpackPulse > 0.08f ? "true" : "false",
                 t.health, t.isGrounded ? "true" : "false", t.falseStartStage, t.finalStage,
                 t.falseStartFlash, t.gateFound ? "true" : "false");
    std::fclose(f);
    MoveFileExA(tmpPath, g_telemetryPath, MOVEFILE_REPLACE_EXISTING);
}

static void updateTelemetry(float dt) {
    Vec3 realSignal = sightingPositionForStage(kFinalSignalStage);
    Vec3 currentGate = currentSightingPosition();
    float signalX = realSignal.x;
    float signalY = realSignal.y;
    float signalZ = realSignal.z;
    float dx = g_player.x - signalX;
    float dy = g_player.y - signalY;
    float dz = g_player.z - signalZ;
    float currentDx = g_player.x - currentGate.x;
    float currentDy = g_player.y - currentGate.y;
    float currentDz = g_player.z - currentGate.z;
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
    t.version = 5;
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
    t.currentGateX = currentGate.x;
    t.currentGateY = currentGate.y;
    t.currentGateZ = currentGate.z;
    t.currentGatePlanarDistance = std::sqrt(currentDx * currentDx + currentDz * currentDz);
    t.currentGateThreeDimensionalDistance = std::sqrt(currentDx * currentDx + currentDy * currentDy + currentDz * currentDz);
    t.currentGateDistance = t.currentGatePlanarDistance;
    t.realGateX = signalX;
    t.realGateY = signalY;
    t.realGateZ = signalZ;
    t.realGateDistance = t.realDistance;
    t.realGatePlanarDistance = t.planarDistance;
    t.realGateThreeDimensionalDistance = t.threeDimensionalDistance;
    t.visibleTowerZ = currentTowerZ();
    t.falseStartStage = g_signalStage;
    t.finalStage = kFinalSignalStage;
    t.signalReached = g_signalReached;
    t.falseStartFlash = g_falseStartFlash;
    t.thirst = g_thirst;
    t.waterFuel = g_waterFuel;
    t.jetpackPulse = g_jetpackPulse;
    t.health = g_playerHealth;
    t.isGrounded = g_isGrounded ? 1 : 0;
    t.gateFound = g_signalReached >= 0.98f ? 1 : 0;
    if (g_telemetry) {
        *g_telemetry = t;
    }
    g_lastTelemetryPlayer = g_player;
    g_hasTelemetryPlayer = true;

    g_telemetryFileTimer += dt;
    if (g_telemetryFileTimer >= kTelemetryFileInterval) {
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
static void pushAtomicTri(Vec3 a, Vec3 b, Vec3 c, Vec3 color, float shine) {
    Vec3 n = norm(cross(sub(b, a), sub(c, a)));
    g_atomicTriangles.push_back({a, n, color, shine});
    g_atomicTriangles.push_back({b, n, color, shine});
    g_atomicTriangles.push_back({c, n, color, shine});
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
static void pushFrameLine(Vec3 a, Vec3 b, Vec3 color, float shine = 0.5f) {
    g_frameLines.push_back({a, {0, 1, 0}, color, shine});
    g_frameLines.push_back({b, {0, 1, 0}, color, shine});
}
static void pushAtomicBillboard(Vec3 center, float width, float height, Vec3 color, float shine) {
    Vec3 toPlayer = norm(sub(g_player, center));
    Vec3 right = norm(cross({0.0f, 1.0f, 0.0f}, toPlayer));
    Vec3 up{0.0f, 1.0f, 0.0f};
    Vec3 r = mul(right, width * 0.5f);
    Vec3 u = mul(up, height * 0.5f);
    Vec3 a = sub(sub(center, r), u);
    Vec3 b = add(sub(center, u), r);
    Vec3 c = add(add(center, r), u);
    Vec3 d = add(sub(center, r), u);
    pushAtomicTri(a, b, c, color, shine);
    pushAtomicTri(a, c, d, color, shine);
}
static void pushAtomicRing(Vec3 center, float radius, Vec3 color, float shine, int steps = 128) {
    Vec3 prev{center.x + radius, center.y, center.z};
    for (int i = 1; i <= steps; ++i) {
        float a = static_cast<float>(i) / static_cast<float>(steps) * 6.2831853f;
        Vec3 cur{center.x + std::cos(a) * radius, center.y, center.z + std::sin(a) * radius};
        pushFrameLine(prev, cur, color, shine);
        prev = cur;
    }
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
static Vec3 buildingHazeColor(Vec3 color, float x, float z) {
    float dx = x - g_player.x;
    float dz = z - g_player.z;
    float d = std::sqrt(dx * dx + dz * dz);
    float t = smoothstep(820.0f, 2050.0f, d);
    Vec3 haze{0.38f, 0.39f, 0.39f};
    return {lerp(color.x, haze.x, t), lerp(color.y, haze.y, t), lerp(color.z, haze.z, t)};
}
static void pushBuilding(Vec3 baseCenter, Vec3 halfSize, Vec3 color, float shine) {
    Vec3 center{baseCenter.x, baseCenter.y + halfSize.y, baseCenter.z};
    color = buildingHazeColor(color, baseCenter.x, baseCenter.z);
    shine *= 1.0f - smoothstep(820.0f, 2050.0f, std::sqrt((baseCenter.x - g_player.x) * (baseCenter.x - g_player.x) + (baseCenter.z - g_player.z) * (baseCenter.z - g_player.z))) * 0.82f;
    Vec3 foundationColor{color.x * 0.70f, color.y * 0.70f, color.z * 0.70f};
    pushCube({baseCenter.x, baseCenter.y - 1.75f, baseCenter.z}, {halfSize.x * 1.035f, 1.85f, halfSize.z * 1.035f}, foundationColor, shine * 0.55f, 0.0f);
    pushCube(center, halfSize, color, shine, 0.0f);
    g_buildingColliders.push_back({
        center.x - halfSize.x, center.x + halfSize.x,
        center.y - halfSize.y, center.y + halfSize.y,
        center.z - halfSize.z, center.z + halfSize.z
    });
}
static bool groundedBuildingBase(float x, float z, float sx, float sz, float& baseY) {
    float low = 1.0e9f;
    float high = -1.0e9f;
    for (int iz = -2; iz <= 2; ++iz) {
        for (int ix = -2; ix <= 2; ++ix) {
            float px = x + sx * (static_cast<float>(ix) * 0.5f);
            float pz = z + sz * (static_cast<float>(iz) * 0.5f);
            float h = renderedSurfaceHeight(px, pz);
            low = std::min(low, h);
            high = std::max(high, h);
        }
    }
    float allowedSlope = clampf(9.0f + std::min(sx, sz) * 0.08f, 9.0f, 18.0f);
    if (high - low > allowedSlope) return false;
    baseY = low - 0.25f;
    return true;
}

static void buildTerrain() {
    const float step = kTerrainMeshStep;
    const int radiusX = 42;
    const int radiusZ = 86;
    float baseX = std::floor(g_player.x / step) * step;
    float baseZ = std::floor(g_player.z / step) * step;
    for (int iz = -radiusZ; iz < radiusZ; ++iz) {
        for (int ix = -radiusX; ix < radiusX; ++ix) {
            float x0 = baseX + ix * step, x1 = x0 + step;
            float z0 = baseZ + iz * step, z1 = z0 + step;
            Vec3 p00{x0, terrainHeight(x0, z0), z0};
            Vec3 p10{x1, terrainHeight(x1, z0), z0};
            Vec3 p01{x0, terrainHeight(x0, z1), z1};
            Vec3 p11{x1, terrainHeight(x1, z1), z1};
            float lane = smoothstep(250.0f, 520.0f, std::fabs(x0));
            float check = std::fmod(std::fabs(std::floor(x0 / 18.0f) + std::floor(z0 / 18.0f)), 2.0f);
            float wave = 0.5f + 0.5f * std::sin((x0 + z0) * 0.018f + g_time * 0.35f);
            Vec3 c{0.18f + check * 0.030f + wave * 0.10f, 0.16f + check * 0.025f, 0.25f + wave * 0.18f};
            c = {lerp(c.x, 0.08f, lane), lerp(c.y, 0.12f, lane), lerp(c.z, 0.22f, lane)};
            Vec3 n{0.0f, 1.0f, 0.0f};
            pushTriSmooth(p00, p11, p10, n, n, n, c, 0.72f);
            pushTriSmooth(p00, p01, p11, n, n, n, c, 0.72f);
        }
    }
}
static void pushGateText47(Vec3 c, float scale, Vec3 color) {
    float x = c.x, y = c.y, z = c.z;
    float s = scale;
    pushLine({x - 7*s, y + 5*s, z}, {x - 7*s, y - 5*s, z}, color, 1.0f);
    pushLine({x - 11*s, y + 0.5f*s, z}, {x - 3*s, y + 0.5f*s, z}, color, 1.0f);
    pushLine({x - 3*s, y + 5*s, z}, {x - 3*s, y - 5*s, z}, color, 1.0f);
    pushLine({x + 2*s, y + 5*s, z}, {x + 10*s, y + 5*s, z}, color, 1.0f);
    pushLine({x + 10*s, y + 5*s, z}, {x + 4*s, y - 5*s, z}, color, 1.0f);
}
static void pushDigitGlyph(int digit, Vec3 c, float s, Vec3 color) {
    float x = c.x, y = c.y, z = c.z;
    auto seg = [&](float ax, float ay, float bx, float by) {
        pushLine({x + ax * s, y + ay * s, z}, {x + bx * s, y + by * s, z}, color, 1.0f);
    };
    if (digit == 0) { seg(-3, 5, 3, 5); seg(-3, -5, 3, -5); seg(-3, 5, -3, -5); seg(3, 5, 3, -5); }
    else if (digit == 1) { seg(0, 5, 0, -5); seg(-2, 3, 0, 5); seg(-2, -5, 2, -5); }
    else if (digit == 2) { seg(-3, 5, 3, 5); seg(3, 5, 3, 0); seg(3, 0, -3, 0); seg(-3, 0, -3, -5); seg(-3, -5, 3, -5); }
    else if (digit == 3) { seg(-3, 5, 3, 5); seg(3, 5, 3, -5); seg(-2, 0, 3, 0); seg(-3, -5, 3, -5); }
    else if (digit == 4) { seg(-3, 5, -3, 0); seg(-3, 0, 3, 0); seg(3, 5, 3, -5); }
    else if (digit == 5) { seg(3, 5, -3, 5); seg(-3, 5, -3, 0); seg(-3, 0, 3, 0); seg(3, 0, 3, -5); seg(3, -5, -3, -5); }
    else if (digit == 6) { seg(3, 5, -3, 5); seg(-3, 5, -3, -5); seg(-3, 0, 3, 0); seg(3, 0, 3, -5); seg(3, -5, -3, -5); }
    else if (digit == 7) { seg(-3, 5, 3, 5); seg(3, 5, -1, -5); }
    else if (digit == 8) { seg(-3, 5, 3, 5); seg(-3, -5, 3, -5); seg(-3, 5, -3, -5); seg(3, 5, 3, -5); seg(-3, 0, 3, 0); }
    else { seg(3, -5, 3, 5); seg(3, 5, -3, 5); seg(-3, 5, -3, 0); seg(-3, 0, 3, 0); seg(3, 0, 3, -5); }
}
static void pushArrowGlyph(Vec3 c, float s, int dir, Vec3 color) {
    float z = c.z;
    if (dir == 0) {
        pushLine({c.x, c.y + 7*s, z}, {c.x, c.y - 7*s, z}, color, 1.0f);
        pushLine({c.x, c.y - 7*s, z}, {c.x - 5*s, c.y - 2*s, z}, color, 1.0f);
        pushLine({c.x, c.y - 7*s, z}, {c.x + 5*s, c.y - 2*s, z}, color, 1.0f);
    } else if (dir < 0) {
        pushLine({c.x + 8*s, c.y, z}, {c.x - 8*s, c.y, z}, color, 1.0f);
        pushLine({c.x - 8*s, c.y, z}, {c.x - 2*s, c.y + 5*s, z}, color, 1.0f);
        pushLine({c.x - 8*s, c.y, z}, {c.x - 2*s, c.y - 5*s, z}, color, 1.0f);
    } else {
        pushLine({c.x - 8*s, c.y, z}, {c.x + 8*s, c.y, z}, color, 1.0f);
        pushLine({c.x + 8*s, c.y, z}, {c.x + 2*s, c.y + 5*s, z}, color, 1.0f);
        pushLine({c.x + 8*s, c.y, z}, {c.x + 2*s, c.y - 5*s, z}, color, 1.0f);
    }
}
static void pushGateNumber(Vec3 c, int number, float s, Vec3 color) {
    int tens = (number / 10) % 10;
    int ones = number % 10;
    pushDigitGlyph(tens, {c.x - 5.0f * s, c.y, c.z}, s, color);
    pushDigitGlyph(ones, {c.x + 5.0f * s, c.y, c.z}, s, color);
}
static void pushDirectionSign(Vec3 c, int arrowDir, int gateNo, Vec3 faceColor, Vec3 lineColor) {
    pushCube(c, {52.0f, 15.0f, 2.5f}, faceColor, 0.78f);
    pushArrowGlyph({c.x - 24.0f, c.y, c.z - 3.2f}, 1.35f, arrowDir, lineColor);
    pushGateNumber({c.x + 14.0f, c.y, c.z - 3.2f}, gateNo, 1.10f, lineColor);
}
static void pushDepartureBoard(Vec3 c, int primaryGate, int decoyGate, Vec3 accent) {
    Vec3 face{0.005f, 0.010f, 0.026f};
    pushCube(c, {104.0f, 34.0f, 3.2f}, face, 0.92f);
    pushCube({c.x, c.y + 25.0f, c.z - 3.8f}, {96.0f, 3.0f, 0.8f}, accent, 1.0f);
    pushGateNumber({c.x - 62.0f, c.y + 8.0f, c.z - 4.2f}, primaryGate, 1.45f, {0.15f, 1.0f, 0.84f});
    pushGateNumber({c.x + 44.0f, c.y + 8.0f, c.z - 4.2f}, decoyGate, 1.15f, {1.0f, 0.16f, 0.92f});
    pushArrowGlyph({c.x - 10.0f, c.y + 8.0f, c.z - 4.2f}, 1.4f, primaryGate == 47 ? 0 : (primaryGate < decoyGate ? -1 : 1), {0.95f, 0.95f, 0.18f});
    for (int row = 0; row < 4; ++row) {
        float y = c.y - 7.0f - row * 8.0f;
        float jitter = static_cast<float>((primaryGate + decoyGate + row * 11) % 19);
        pushLine({c.x - 90.0f, y, c.z - 4.2f}, {c.x - 20.0f + jitter, y, c.z - 4.2f}, {0.12f, 0.70f, 0.82f}, 0.8f);
        pushLine({c.x + 10.0f, y, c.z - 4.2f}, {c.x + 88.0f - jitter, y, c.z - 4.2f}, {0.90f, 0.12f, 0.70f}, 0.8f);
    }
}
static void pushRouteBand(float x0, float x1, float z, Vec3 color, bool broken) {
    float y = 0.52f;
    int segments = broken ? 6 : 1;
    for (int i = 0; i < segments; ++i) {
        float a = i / static_cast<float>(segments);
        float b = (i + 0.64f) / static_cast<float>(segments);
        float xa = lerp(x0, x1, a);
        float xb = lerp(x0, x1, std::min(b, 1.0f));
        pushLine({xa, y, z}, {xb, y, z}, color, 1.0f);
        pushLine({xa, y, z + 3.2f}, {xb, y, z + 3.2f}, color, 1.0f);
    }
}
static void pushFloorArrow(Vec3 c, float s, int dir, Vec3 color) {
    float z = c.z;
    if (dir == 0) {
        pushLine({c.x, c.y, z + 13*s}, {c.x, c.y, z - 13*s}, color, 1.0f);
        pushLine({c.x, c.y, z - 13*s}, {c.x - 8*s, c.y, z - 4*s}, color, 1.0f);
        pushLine({c.x, c.y, z - 13*s}, {c.x + 8*s, c.y, z - 4*s}, color, 1.0f);
    } else if (dir < 0) {
        pushLine({c.x + 13*s, c.y, z}, {c.x - 13*s, c.y, z}, color, 1.0f);
        pushLine({c.x - 13*s, c.y, z}, {c.x - 4*s, c.y, z + 8*s}, color, 1.0f);
        pushLine({c.x - 13*s, c.y, z}, {c.x - 4*s, c.y, z - 8*s}, color, 1.0f);
    } else {
        pushLine({c.x - 13*s, c.y, z}, {c.x + 13*s, c.y, z}, color, 1.0f);
        pushLine({c.x + 13*s, c.y, z}, {c.x + 4*s, c.y, z + 8*s}, color, 1.0f);
        pushLine({c.x + 13*s, c.y, z}, {c.x + 4*s, c.y, z - 8*s}, color, 1.0f);
    }
}
static void addColliderBox(Vec3 center, Vec3 half) {
    g_buildingColliders.push_back({center.x - half.x, center.x + half.x, center.y - half.y, center.y + half.y, center.z - half.z, center.z + half.z});
}
static void buildConcourseShell() {
    int base = static_cast<int>(std::floor(g_player.z / 220.0f));
    Vec3 wall{0.20f, 0.18f, 0.32f};
    Vec3 glass{0.10f, 0.34f, 0.42f};
    Vec3 ceiling{0.30f, 0.25f, 0.42f};
    for (int i = -9; i <= 8; ++i) {
        float z = (base + i) * 220.0f;
        pushCube({-335.0f, 40.0f, z}, {22.0f, 40.0f, 116.0f}, wall, 0.04f);
        pushCube({335.0f, 40.0f, z}, {22.0f, 40.0f, 116.0f}, wall, 0.04f);
        pushCube({-309.0f, 42.0f, z}, {3.0f, 22.0f, 72.0f}, glass, 0.26f);
        pushCube({309.0f, 42.0f, z}, {3.0f, 22.0f, 72.0f}, glass, 0.26f);
        pushCube({0.0f, 77.0f + std::sin((base + i) * 0.7f) * 1.2f, z}, {340.0f, 5.5f, 118.0f}, ceiling, 0.08f);
            pushCube({0.0f, 58.0f, z - 108.0f}, {330.0f, 4.0f, 4.0f}, {0.25f, 0.26f, 0.26f}, 0.03f);
            pushCube({0.0f, 58.0f, z + 108.0f}, {330.0f, 4.0f, 4.0f}, {0.25f, 0.26f, 0.26f}, 0.03f);
        for (int lane = -2; lane <= 2; ++lane) {
            float lx = lane * 118.0f;
            pushCube({lx, 69.5f, z - 52.0f}, {42.0f, 0.7f, 3.2f}, {0.15f + lane * 0.10f, 0.95f, 0.70f - lane * 0.08f}, 0.95f);
            pushCube({lx, 69.5f, z + 52.0f}, {42.0f, 0.7f, 3.2f}, {0.95f - lane * 0.08f, 0.18f + lane * 0.12f, 0.82f}, 0.95f);
        }
        for (int k = -3; k <= 3; ++k) {
            float x = k * 86.0f;
            Vec3 beam{0.24f, 0.25f, 0.25f};
            pushCube({x, 41.0f, z - 92.0f}, {5.2f, 40.0f, 5.2f}, beam, 0.04f);
            pushCube({x, 41.0f, z + 92.0f}, {5.2f, 40.0f, 5.2f}, beam, 0.04f);
        }
        addColliderBox({-335.0f, 40.0f, z}, {22.0f, 40.0f, 116.0f});
        addColliderBox({335.0f, 40.0f, z}, {22.0f, 40.0f, 116.0f});
    }
}
static void buildMovingWalkway(float x, float z, float len, int dir) {
    Vec3 belt = dir > 0 ? Vec3{0.018f, 0.105f, 0.140f} : Vec3{0.130f, 0.070f, 0.028f};
    Vec3 rubber{0.018f, 0.020f, 0.022f};
    Vec3 chrome{0.42f, 0.43f, 0.41f};
    Vec3 glow = dir > 0 ? Vec3{0.22f, 0.95f, 1.00f} : Vec3{1.00f, 0.72f, 0.22f};
    pushCube({x, 0.18f, z}, {29.5f, 0.16f, len + 7.0f}, rubber, 0.18f);
    pushCube({x, 0.46f, z}, {22.5f, 0.20f, len}, belt, 0.74f);
    pushCube({x - 27.0f, 2.2f, z}, {1.0f, 2.0f, len}, chrome, 0.18f);
    pushCube({x + 27.0f, 2.2f, z}, {1.0f, 2.0f, len}, chrome, 0.18f);
    pushCube({x - 24.7f, 3.4f, z}, {0.65f, 0.40f, len}, glow, 0.92f);
    pushCube({x + 24.7f, 3.4f, z}, {0.65f, 0.40f, len}, glow, 0.92f);
    pushCube({x, 0.78f, z - len + 4.0f}, {24.0f, 0.32f, 1.8f}, chrome, 0.24f);
    pushCube({x, 0.78f, z + len - 4.0f}, {24.0f, 0.32f, 1.8f}, chrome, 0.24f);
    for (int i = -4; i <= 4; ++i) {
        float p = z + i * 24.0f;
        if (p < z - len + 11.0f || p > z + len - 11.0f) continue;
        float center = p + dir * 2.2f;
        pushCube({x - 5.2f, 0.96f, center}, {1.25f, 0.075f, 9.0f}, glow, 0.98f, 0.62f * dir);
        pushCube({x + 5.2f, 0.96f, center}, {1.25f, 0.075f, 9.0f}, glow, 0.98f, -0.62f * dir);
        pushCube({x, 0.94f, p - dir * 6.5f}, {13.0f, 0.055f, 0.75f}, {0.02f, 0.02f, 0.024f}, 0.30f);
    }
    for (int i = -6; i <= 6; ++i) {
        float p = z + i * 17.0f;
        if (p < z - len + 4.0f || p > z + len - 4.0f) continue;
        Vec3 tick = (i & 1) ? glow : Vec3{0.15f, 0.17f, 0.17f};
        pushLine({x - 20.0f, 1.05f, p}, {x + 20.0f, 1.05f, p + dir * 3.5f}, tick, 0.95f);
    }
}
static void buildConveyorsAndEscalators() {
    int base = static_cast<int>(std::floor(g_player.z / 360.0f));
    for (int i = -4; i <= 4; ++i) {
        int cell = base + i;
        float z = cell * 360.0f + 40.0f;
        int dir = (cell & 1) ? 1 : -1;
        buildMovingWalkway(-82.0f, z, 116.0f, dir);
        buildMovingWalkway(82.0f, z + 154.0f, 116.0f, -dir);
    }
}
static void pushMazeWall(Vec3 center, Vec3 half, Vec3 color, float shine) {
    pushCube(center, half, color, shine);
    addColliderBox(center, half);
}
static float mazeLaneX(int lane) {
    return (static_cast<float>(lane) - 2.0f) * 72.0f;
}
static int mazePathLane(int cell) {
    float wave = std::sin(static_cast<float>(cell) * 0.73f) + std::sin(static_cast<float>(cell) * 0.31f + 1.7f) * 0.62f;
    int lane = static_cast<int>(std::floor((wave + 1.62f) / 3.24f * 5.0f));
    return static_cast<int>(clampf(static_cast<float>(lane), 0.0f, 4.0f));
}
static void buildMazePartitions() {
    int base = static_cast<int>(std::floor(g_player.z / 160.0f));
    Vec3 heavy{0.13f, 0.09f, 0.23f};
    Vec3 dark{0.06f, 0.04f, 0.11f};
    Vec3 lit{0.80f, 0.12f, 0.92f};
    for (int i = -12; i <= 12; ++i) {
        int cell = base + i;
        float z = cell * 160.0f;
        float phase = rand01(cell, 4701);
        int lane = cell == 0 ? 2 : mazePathLane(cell);
        int nextLane = cell == -1 ? 2 : mazePathLane(cell + 1);
        int prevLane = cell == 1 ? 2 : mazePathLane(cell - 1);
        float gapX = mazeLaneX(lane);
        float leftEnd = gapX - 44.0f;
        float rightStart = gapX + 44.0f;
        if (leftEnd > -282.0f) {
            float cx = (-282.0f + leftEnd) * 0.5f;
            float hx = std::max(8.0f, (leftEnd + 282.0f) * 0.5f);
            pushMazeWall({cx, 30.0f, z + 18.0f}, {hx, 30.0f, 8.0f}, heavy, 0.30f);
        }
        if (rightStart < 282.0f) {
            float cx = (rightStart + 282.0f) * 0.5f;
            float hx = std::max(8.0f, (282.0f - rightStart) * 0.5f);
            pushMazeWall({cx, 30.0f, z + 18.0f}, {hx, 30.0f, 8.0f}, heavy, 0.30f);
        }

        int leftLane = std::min(lane, nextLane);
        int rightLane = std::max(lane, nextLane);
        for (int l = 0; l < 5; ++l) {
            float lx = mazeLaneX(l);
            bool connector = l >= leftLane && l <= rightLane;
            if (!connector && rand01(cell + l * 37, 6029) < 0.68f) {
                pushMazeWall({lx + 36.0f, 25.0f, z - 54.0f}, {7.0f, 25.0f, 54.0f}, dark, 0.24f);
            }
        }

        int decoyLane = (lane + 2 + (cell & 1)) % 5;
        if (decoyLane == lane) decoyLane = (lane + 3) % 5;
        float decoyX = mazeLaneX(decoyLane);
        pushMazeWall({decoyX, 24.0f, z - 96.0f}, {46.0f, 24.0f, 6.5f}, {0.08f, 0.04f, 0.16f}, 0.32f);
        if (rand01(cell + 313, 7781) < 0.55f) {
            pushMazeWall({decoyX - 46.0f, 23.0f, z - 48.0f}, {7.0f, 23.0f, 44.0f}, {0.10f, 0.04f, 0.18f}, 0.28f);
            pushMazeWall({decoyX + 46.0f, 23.0f, z - 48.0f}, {7.0f, 23.0f, 44.0f}, {0.10f, 0.04f, 0.18f}, 0.28f);
        }

        pushLine({gapX - 24.0f, 2.2f, z + 9.0f}, {gapX + 24.0f, 2.2f, z + 9.0f}, lit, 1.0f);
        pushLine({gapX, 3.0f, z + 9.0f}, {gapX, 54.0f, z + 9.0f}, {0.12f, 0.95f, 1.0f}, 1.0f);
        int laneDelta = nextLane - lane;
        int honestDir = laneDelta < 0 ? -1 : (laneDelta > 0 ? 1 : 0);
        int liarDir = -honestDir;
        if (honestDir == 0) liarDir = (cell & 1) ? -1 : 1;
        bool lying = rand01(cell + 911, 1203) < 0.34f;
        int signDir = lying ? liarDir : honestDir;
        int signGate = lying ? 34 + (cell * 7 + 19) % 26 : 47;
        Vec3 signFace = lying ? Vec3{0.05f, 0.02f, 0.14f} : Vec3{0.01f, 0.035f, 0.055f};
        Vec3 signLine = lying ? Vec3{1.0f, 0.20f, 0.72f} : Vec3{0.20f, 1.0f, 0.90f};
        pushDirectionSign({gapX, 44.0f, z - 1.0f}, signDir, signGate, signFace, signLine);
        pushFloorArrow({gapX, 0.45f, z - 48.0f}, 1.45f, signDir, signLine);
        float nextX = mazeLaneX(nextLane);
        pushRouteBand(gapX, nextX, z - 74.0f, lying ? Vec3{1.0f, 0.10f, 0.80f} : Vec3{0.10f, 1.0f, 0.86f}, lying);
        if (cell % 4 == 0) {
            int decoyGate = 31 + (cell * 13 + g_signalStage * 5) % 36;
            pushDepartureBoard({0.0f, 52.0f, z - 116.0f}, 47, decoyGate, lying ? Vec3{1.0f, 0.12f, 0.80f} : Vec3{0.10f, 0.95f, 1.0f});
        }
        if (prevLane != lane) {
            float prevX = mazeLaneX(prevLane);
            pushLine({prevX, 0.55f, z - 132.0f}, {gapX, 0.55f, z - 24.0f}, {0.20f, 0.95f, 1.0f}, 0.9f);
        }
    }
}
static void buildGateCluster(float z, int idx) {
    float side = (idx % 2 == 0) ? -1.0f : 1.0f;
    float x = side * (220.0f + (idx % 3) * 18.0f);
    Vec3 counter{0.22f, 0.25f, 0.27f};
    Vec3 sign{0.02f, 0.05f, 0.07f};
    Vec3 amber{1.0f, 0.95f, 0.12f};
    pushCube({x, 10.0f, z}, {58.0f, 10.0f, 14.0f}, {0.18f, 0.10f, 0.28f}, 0.30f);
    pushCube({x, 29.0f, z - 13.0f}, {64.0f, 12.0f, 3.0f}, {0.01f, 0.02f, 0.04f}, 0.75f);
    int gateNo = 34 + (idx * 7) % 28;
    if ((idx + g_signalStage) % 5 == 0) gateNo = 47;
    if (gateNo == 47) pushGateText47({x, 27.0f, z - 13.5f}, 1.15f, amber);
    int arrowDir = x < 0.0f ? -1 : 1;
    bool decoy = gateNo != 47 && rand01(idx + 41, 909) < 0.46f;
    int shownGate = decoy ? 47 : gateNo;
    Vec3 dirColor = decoy ? Vec3{1.0f, 0.10f, 0.95f} : Vec3{0.18f, 0.95f, 1.0f};
    pushDirectionSign({x - arrowDir * 78.0f, 34.0f, z - 8.0f}, arrowDir, shownGate, {0.00f, 0.02f, 0.05f}, dirColor);
    for (int s = -2; s <= 2; ++s) {
        pushCube({x + s * 18.0f, 4.0f, z + 34.0f}, {5.2f, 0.55f, 5.0f}, {0.08f, 0.085f, 0.085f}, 0.12f);
        pushCube({x + s * 18.0f, 6.2f, z + 38.0f}, {4.8f, 2.0f, 0.45f}, {0.13f, 0.14f, 0.14f}, 0.10f);
    }
    addColliderBox({x, 10.0f, z}, {58.0f, 10.0f, 14.0f});
}
static void buildGate47() {
    Vec3 p = encounterSightingPosition();
    Vec3 sign{0.005f, 0.006f, 0.020f};
    Vec3 glow{1.0f, 0.10f, 0.95f};
    pushCube({p.x, p.y, p.z}, {72.0f, 18.0f, 3.0f}, sign, 0.48f);
    pushGateText47({p.x, p.y, p.z - 4.0f}, 2.35f, glow);
    pushCube({p.x, 8.0f, p.z + 30.0f}, {64.0f, 8.0f, 18.0f}, {0.18f, 0.21f, 0.24f}, 0.22f);
    for (int i = -4; i <= 4; ++i) {
        float t = (i + 4) / 8.0f;
        pushLine({p.x + i * 14.0f, 1.0f, p.z + 58.0f}, {p.x + i * 14.0f, 64.0f, p.z - 24.0f}, {0.15f + t * 0.85f, 0.85f - t * 0.35f, 1.0f}, 1.0f);
    }
}
static void buildAtomicBlastFx() {
    g_atomicTriangles.clear();
    g_frameLines.clear();
    if (g_atomicTime < 0.0f || g_atomicTime > kAtomicDropSeconds + 34.0f) return;
    float t = g_atomicTime - kAtomicDropSeconds;
    Vec3 base = g_atomicOrigin;
    float ground = renderedSurfaceHeight(base.x, base.z);
    base.y = ground;
    if (t < 0.0f) {
        float drop = smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime);
        Vec3 bomb{base.x, ground + 720.0f * (1.0f - drop) + 12.0f, base.z};
        pushAtomicBillboard(bomb, 96.0f, 220.0f, {1.8f, 1.75f, 1.20f}, 1.0f);
        pushFrameLine({bomb.x, bomb.y + 240.0f, bomb.z}, bomb, {1.35f, 1.0f, 0.45f}, 1.0f);
        return;
    }
    float flash = smoothstep(3.8f, 0.0f, t);
    float bloom = smoothstep(0.1f, 3.0f, t) * smoothstep(27.0f, 5.0f, t);
    float distantBeat = 1.0f - smoothstep(3.4f, 6.2f, t);
    if (distantBeat > 0.001f) {
        Vec3 farBase{base.x + 180.0f, ground + 520.0f, base.z - 980.0f};
        float farFire = (230.0f + t * 42.0f) * smoothstep(4.4f, 0.08f, t);
        float farRise = smoothstep(0.0f, 5.0f, t);
        if (farFire > 1.0f) {
            pushAtomicBillboard(farBase, farFire * 2.8f, farFire * 1.75f, {1.95f, 1.45f, 0.38f}, 1.0f);
            pushAtomicBillboard({farBase.x, farBase.y + farFire * 0.18f, farBase.z}, farFire * 1.45f, farFire * 1.15f, {1.0f, 0.98f, 0.84f}, 1.0f);
        }
        float stemH = 210.0f + farRise * 520.0f;
        float stemW = 56.0f + farRise * 150.0f;
        for (int i = 0; i < 3; ++i) {
            float fi = static_cast<float>(i);
            float a = fi * 1.8f + g_time * 0.10f;
            Vec3 p{farBase.x + std::cos(a) * stemW * 0.18f, farBase.y + 120.0f + stemH * (0.10f + fi * 0.095f), farBase.z + std::sin(a) * stemW * 0.18f};
            pushAtomicBillboard(p, stemW * (1.5f + fi * 0.10f), stemW * 1.7f, {0.90f, 0.86f, 0.74f}, 0.72f);
        }
        float capW = 320.0f + farRise * 720.0f;
        float capY = farBase.y + 520.0f + farRise * 260.0f;
        for (int i = 0; i < 5; ++i) {
            float seed = rand01(9000 + i, 51);
            float a = static_cast<float>(i) / 18.0f * 6.2831853f + seed * 0.4f;
            Vec3 p{farBase.x + std::cos(a) * capW * (0.18f + seed * 0.32f),
                   capY + std::sin(a * 2.0f) * 48.0f,
                   farBase.z + std::sin(a) * capW * (0.08f + seed * 0.18f)};
            pushAtomicBillboard(p, 110.0f + seed * 180.0f, 82.0f + seed * 130.0f, {0.78f, 0.75f, 0.66f}, 0.66f);
        }
        for (int i = 0; i < 2; ++i) {
            float rr = 160.0f + i * 72.0f + t * 96.0f;
            pushAtomicRing({farBase.x, ground + 92.0f + i * 1.5f, farBase.z}, rr, {1.0f, 0.88f, 0.42f}, 0.9f, 48);
        }
    }
    float shock = 540.0f + t * 430.0f;
    if (t < 14.0f) {
        pushAtomicRing({base.x, ground + 1.0f, base.z}, shock, {1.7f, 1.55f, 0.55f}, 1.0f, 72);
        pushAtomicRing({base.x, ground + 7.0f, base.z}, shock * 0.66f, {0.35f, 1.45f, 1.6f}, 1.0f, 56);
        pushAtomicRing({base.x, ground + 13.0f, base.z}, shock * 0.44f, {1.65f, 0.16f, 1.35f}, 1.0f, 48);
    }
    float fire = (700.0f + t * 135.0f) * smoothstep(9.0f, 0.12f, t);
    if (fire > 1.0f) {
        pushAtomicBillboard({base.x, ground + fire * 0.28f, base.z}, fire * 5.8f, fire * 1.28f, {1.75f, 1.55f, 0.35f}, 1.0f);
        pushAtomicBillboard({base.x, ground + fire * 0.50f, base.z}, fire * 2.9f, fire * 1.68f, {1.15f, 1.55f, 1.50f}, 1.0f);
        pushAtomicBillboard({base.x, ground + fire * 0.36f, base.z}, fire * 4.2f, fire * 1.10f, {1.65f, 0.26f, 1.25f}, 1.0f);
    }
    float rise = smoothstep(0.0f, 16.0f, t);
    float columnH = 360.0f + rise * 1150.0f;
    float columnW = 150.0f + rise * 520.0f;
    for (int i = 0; i < 6; ++i) {
        float fi = static_cast<float>(i);
        float a = fi * 1.55f + g_time * 0.20f;
        Vec3 p{base.x + std::cos(a) * columnW * (0.12f + fi * 0.010f),
               ground + columnH * (0.05f + fi * 0.040f),
               base.z + std::sin(a) * columnW * 0.26f};
        float s = columnW * (0.65f + fi * 0.025f);
        Vec3 c = (i % 3 == 0) ? Vec3{1.0f, 1.0f, 0.95f} : ((i % 3 == 1) ? Vec3{0.35f, 1.0f, 1.0f} : Vec3{1.0f, 0.20f, 0.95f});
        c = {c.x + flash * 0.4f, c.y + flash * 0.4f, c.z + flash * 0.4f};
        pushAtomicBillboard(p, s * 2.5f, s * 1.4f, c, 1.0f);
    }
    for (int i = 0; i < 18; ++i) {
        float seed = rand01(i + 8000, 17);
        float a = seed * 6.2831853f;
        float r = (160.0f + seed * 1600.0f) * bloom;
        Vec3 p{base.x + std::cos(a) * r, ground + 20.0f + rand01(i, 44) * 760.0f * bloom, base.z + std::sin(a) * r * 0.55f};
        Vec3 c = i % 2 ? Vec3{0.95f, 1.0f, 1.0f} : Vec3{1.0f, 0.28f, 0.95f};
        pushAtomicBillboard(p, 30.0f + seed * 90.0f, 18.0f + seed * 64.0f, c, 0.85f);
    }
}
static void buildAirportDetails() {
    int base = static_cast<int>(std::floor(g_player.z / 180.0f));
    for (int i = -8; i <= 8; ++i) {
        int cell = base + i;
        float z = cell * 180.0f;
        buildGateCluster(z, cell);
        for (int lane = -2; lane <= 2; ++lane) {
            float x = lane * 92.0f + std::sin(cell * 0.8f + lane) * 8.0f;
            pushCube({x, 21.0f, z + 78.0f}, {18.0f, 0.9f, 2.2f}, {0.45f, 0.95f, 0.75f}, 0.70f);
            pushLine({x - 10.0f, 20.0f, z + 78.0f}, {x - 10.0f, 72.0f, z + 78.0f}, {0.80f, 0.20f, 1.0f}, 1.0f);
            pushLine({x + 10.0f, 20.0f, z + 78.0f}, {x + 10.0f, 72.0f, z + 78.0f}, {0.10f, 0.95f, 1.0f}, 1.0f);
        }
        if (cell % 4 == 0) {
            pushCube({0.0f, 3.0f, z - 50.0f}, {95.0f, 3.0f, 9.0f}, {0.08f, 0.09f, 0.10f}, 0.82f);
            pushGateNumber({-34.0f, 8.2f, z - 60.0f}, 47, 1.15f, {0.90f, 0.15f, 1.0f});
            pushFloorArrow({42.0f, 6.4f, z - 50.0f}, 1.0f, (cell & 1) ? -1 : 1, {0.20f, 1.0f, 0.90f});
            pushDepartureBoard({0.0f, 36.0f, z - 76.0f}, 47, 60 + ((cell * 3) % 30), {1.0f, 0.90f, 0.12f});
            for (int b = -5; b <= 5; ++b) {
                float bx = b * 17.0f + std::sin(g_time * 1.8f + b) * 3.0f;
                pushCube({bx, 7.6f, z - 50.0f}, {5.0f, 4.0f, 3.2f}, {0.02f + (b&1)*0.30f, 0.06f, 0.09f + (b&1)*0.20f}, 0.38f);
            }
        }
    }
}
static bool shouldRebuildWorld() {
    float dx = g_player.x - g_lastWorldX;
    float dz = g_player.z - g_lastWorldZ;
    return g_worldDirty || g_signalStage != g_lastWorldStage || (dx * dx + dz * dz) > 96.0f * 96.0f;
}
static void buildWorld() {
    g_triangles.clear();
    g_lines.clear();
    g_buildingColliders.clear();
    g_triangles.reserve(76000);
    g_lines.reserve(9000);
    buildTerrain();
    buildConcourseShell();
    buildConveyorsAndEscalators();
    buildMazePartitions();
    buildAirportDetails();
    buildGate47();
    g_lastWorldX = g_player.x;
    g_lastWorldZ = g_player.z;
    g_lastWorldStage = g_signalStage;
    g_worldDirty = false;
    g_sceneBuffersDirty = true;
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
uniform float uFigureX;
uniform float uTowerZ;
out vec4 FragColor;
float hash(vec2 p) { return fract(sin(dot(p, vec2(41.7, 289.3))) * 43758.5453); }
float noise(vec2 p) {
    vec2 i = floor(p); vec2 f = fract(p); f = f*f*(3.0-2.0*f);
    float a = hash(i), b = hash(i+vec2(1,0)), c = hash(i+vec2(0,1)), d = hash(i+vec2(1,1));
    return mix(mix(a,b,f.x), mix(c,d,f.x), f.y);
}
float fbm(vec2 p) { float v=0.0; float a=0.55; for(int i=0;i<3;i++){ v += noise(p)*a; p=p*2.07+7.1; a*=0.46; } return v; }
vec3 spectrum(float x) {
    return 0.55 + 0.45 * cos(6.28318 * (vec3(0.00, 0.33, 0.67) + x));
}
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(-0.58, 0.74, 0.34));
    vec3 V = normalize(uEye - vWorld);
    vec3 H = normalize(L + V);
    float diff = max(dot(N, L), 0.0);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.5);
    float dist = length(uEye - vWorld);

    float grain = fbm(vWorld.xz * 0.12 + vec2(vWorld.y * 0.015, 0.0));
    float fine = noise(vWorld.xz * 0.85 + vec2(vWorld.y * 0.04, 13.0));
    float pores = noise(vWorld.xy * 1.10 + vWorld.z * 0.04);
    float stains = noise(vWorld.xz * 0.025 + vec2(12.0, 4.0));
    float panelX = 1.0 - smoothstep(0.018, 0.075, abs(fract(vWorld.x * 0.042) - 0.5));
    float panelY = 1.0 - smoothstep(0.018, 0.075, abs(fract(vWorld.y * 0.060) - 0.5));
    float floorGridX = 1.0 - smoothstep(0.020, 0.090, abs(fract(vWorld.x / 18.0) - 0.5));
    float floorGridZ = 1.0 - smoothstep(0.020, 0.090, abs(fract(vWorld.z / 18.0) - 0.5));
    float broadPanel = max(
        1.0 - smoothstep(0.030, 0.135, abs(fract(vWorld.x / 72.0) - 0.5)),
        1.0 - smoothstep(0.030, 0.135, abs(fract(vWorld.z / 72.0) - 0.5))
    );
    float maxc = max(vColor.r, max(vColor.g, vColor.b));
    float minc = min(vColor.r, min(vColor.g, vColor.b));
    float redColossus = smoothstep(0.84, 0.98, vColor.r) * (1.0 - smoothstep(0.08, 0.20, vColor.g + vColor.b));
    float concrete = smoothstep(0.38, 0.66, maxc) * (1.0 - smoothstep(0.16, 0.36, maxc-minc));
    float floorLike = smoothstep(0.92, 0.995, N.y);
    float blueBelt = (1.0 - smoothstep(0.035, 0.070, vColor.r)) * smoothstep(0.075, 0.130, vColor.g) * smoothstep(0.105, 0.170, vColor.b);
    float amberBelt = smoothstep(0.095, 0.160, vColor.r) * smoothstep(0.050, 0.095, vColor.g) * (1.0 - smoothstep(0.045, 0.085, vColor.b));
    float beltMat = clamp(max(blueBelt, amberBelt) * floorLike, 0.0, 1.0);

    float chromaField = fbm(vWorld.xz * 0.018 + vec2(uTime * 0.06, -uTime * 0.035));
    vec3 spectral = spectrum(chromaField + vWorld.y * 0.006 + uGlitch * 0.8);
    vec3 material = mix(vColor, spectral, 0.42 + floorLike * 0.20) * (0.78 + grain * 0.35 + fine * 0.12);
    material = mix(material, spectral * (0.58 + grain * 0.45 + pores * 0.18), concrete * 0.58);
    material -= vec3(0.05, 0.025, 0.075) * max(panelX, panelY) * concrete;
    material += spectral * max(floorGridX, floorGridZ) * floorLike * 0.18;
    material -= vec3(0.035, 0.028, 0.045) * broadPanel * floorLike * (1.0 - beltMat) * 0.55;
    material -= vec3(0.03, 0.00, 0.05) * smoothstep(0.52, 1.0, stains) * concrete;
    material += spectrum(pores + uTime * 0.04) * smoothstep(0.58, 0.92, pores) * concrete * 0.10;
    float beltRibs = 1.0 - smoothstep(0.08, 0.22, abs(fract(vWorld.z * 0.28 + uTime * 1.35) - 0.5));
    float beltFine = 1.0 - smoothstep(0.03, 0.10, abs(fract(vWorld.x * 0.70) - 0.5));
    vec3 beltGlow = mix(vec3(0.05, 0.85, 1.0), vec3(1.0, 0.55, 0.08), amberBelt);
    material = mix(material, vColor * (0.55 + grain * 0.16) + beltGlow * (beltRibs * 0.38 + beltFine * 0.08), beltMat);

    float spec = pow(max(dot(N, H), 0.0), 86.0) * vShine;
    float strip = exp(-abs(fract((vWorld.z + 52.0) / 104.0) - 0.5) * 18.0);
    float stripX = smoothstep(0.92, 0.45, abs(fract((vWorld.x + 59.0) / 118.0) - 0.5));
    float lampFalloff = strip * stripX * smoothstep(72.0, 0.0, abs(vWorld.y - 69.5));
    float underShadow = smoothstep(0.0, 0.45, 1.0 - N.y) * 0.18 + smoothstep(26.0, 0.0, vWorld.y) * 0.16;
    vec3 lamp = mix(vec3(0.15, 0.95, 1.0), vec3(1.0, 0.15, 0.95), smoothstep(0.1, 0.9, fract(vWorld.z / 220.0 + uTime * 0.025)));
    vec3 cool = spectrum(vWorld.x * 0.006 + uTime * 0.05) * 0.42;
    vec3 bounce = spectrum(vWorld.z * 0.004 + 0.2) * max(N.y, 0.0) * 0.22;
    float contact = smoothstep(18.0, 0.0, vWorld.y) * (1.0 - floorLike) * 0.18;
    float ceilingFade = smoothstep(55.0, 82.0, vWorld.y) * 0.10;
    float depthShade = smoothstep(120.0, 760.0, dist) * 0.12;
    vec3 color = material * (0.16 + diff * 0.62 + lampFalloff * 0.48 - underShadow) + lamp * spec * (0.18 + lampFalloff * 0.34) + cool * rim * 0.10 + bounce;
    color *= 1.0 - contact - ceilingFade - depthShade;
    color += lamp * floorLike * lampFalloff * 0.055;
    color += vec3(3.4, 0.08, 0.0) * redColossus * (0.95 + rim * 0.55);

    float figureAura = exp(-abs(vWorld.z - uTowerZ) * 0.0016) * exp(-abs(vWorld.x - uFigureX) * 0.0024);
    color += spectrum(uTime * 0.12 + vWorld.x * 0.002) * figureAura * (0.16 + uTowerAlign * 0.55);
    color += vec3(1.0, 0.12, 0.95) * uTowerAlign * rim * 0.22;

    vec3 fog = mix(vec3(0.08, 0.02, 0.16), vec3(0.10, 0.35, 0.45), clamp((vWorld.y + 20.0) / 180.0, 0.0, 1.0));
    float lowAir = smoothstep(90.0, 0.0, abs(vWorld.y - uEye.y));
    float haze = smoothstep(420.0, 1600.0, dist) * 0.58;
    haze += smoothstep(820.0, 2300.0, dist) * lowAir * 0.08;
    haze = clamp(haze, 0.0, 0.68);
    haze *= 1.0 - redColossus * 0.82;
    color = mix(color, fog, haze);
    color += vec3(0.04, 0.16, 0.18) * floorLike * smoothstep(320.0, 40.0, dist) * (1.0 - beltMat) * 0.18;
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
uniform sampler2D uTicket;
uniform vec3 uResolution;
uniform float uTime;
uniform float uGlitch;
uniform float uTowerAlign;
uniform float uWrongWay;
uniform float uSignalReached;
uniform float uFalseStart;
uniform float uDrinkPulse;
uniform float uThirst;
uniform float uWaterFuel;
uniform float uJetpackPulse;
uniform float uHealth;
uniform float uAtomic;
uniform float uAtomicFade;
out vec4 FragColor;
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x), mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}
float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }
vec3 scene(vec2 uv) { return texture(uScene, clamp(uv, vec2(0.001), vec2(0.999))).rgb; }
vec3 spectrum(float x) {
    return 0.5 + 0.5 * cos(6.28318 * (vec3(0.00, 0.33, 0.67) + x));
}
float sdSegment(vec2 p, vec2 a, vec2 b) {
    vec2 pa = p - a;
    vec2 ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}
float glyphLine(vec2 p, float x, float y, float s, int glyph) {
    vec2 q = (p - vec2(x, y)) / s;
    float d = 9.0;
    if (glyph == 0) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.24, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.30, 0.00)));
        d = min(d, sdSegment(q, vec2(-0.35, -0.45), vec2(0.26, -0.45)));
        d = min(d, sdSegment(q, vec2(0.24, 0.45), vec2(0.35, 0.12)));
        d = min(d, sdSegment(q, vec2(0.30, 0.00), vec2(0.38, -0.34)));
    } else if (glyph == 1) {
        d = min(d, sdSegment(q, vec2(-0.36, -0.45), vec2(-0.05, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.05, 0.45), vec2(0.36, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.22, -0.05), vec2(0.20, -0.05)));
    } else if (glyph == 2) {
        d = min(d, sdSegment(q, vec2(-0.34, 0.45), vec2(-0.34, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.34, 0.45), vec2(0.22, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.34, 0.02), vec2(0.18, 0.02)));
        d = min(d, sdSegment(q, vec2(0.22, 0.45), vec2(0.36, 0.18)));
        d = min(d, sdSegment(q, vec2(0.18, 0.02), vec2(0.38, -0.45)));
    } else if (glyph == 3) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.12, 0.45)));
        d = min(d, sdSegment(q, vec2(0.12, 0.45), vec2(0.37, 0.18)));
        d = min(d, sdSegment(q, vec2(0.37, 0.18), vec2(0.37, -0.18)));
        d = min(d, sdSegment(q, vec2(0.37, -0.18), vec2(0.12, -0.45)));
        d = min(d, sdSegment(q, vec2(0.12, -0.45), vec2(-0.35, -0.45)));
    } else if (glyph == 4) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(0.35, 0.45), vec2(0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, -0.45)));
    } else if (glyph == 5) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(0.35, 0.45), vec2(0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, -0.45), vec2(0.35, -0.45)));
    } else if (glyph == 6) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.22, 0.00)));
        d = min(d, sdSegment(q, vec2(0.35, 0.45), vec2(0.35, 0.18)));
        d = min(d, sdSegment(q, vec2(0.22, 0.00), vec2(0.35, -0.45)));
    } else if (glyph == 7) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(0.35, 0.45), vec2(0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.25, 0.00)));
    } else if (glyph == 8) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(0.35, 0.45), vec2(0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.35, 0.00)));
        d = min(d, sdSegment(q, vec2(-0.35, -0.45), vec2(0.35, -0.45)));
    } else if (glyph == 9) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.20, 0.00)));
    } else if (glyph == 10) {
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(-0.35, -0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.45), vec2(0.35, 0.45)));
        d = min(d, sdSegment(q, vec2(-0.35, 0.00), vec2(0.18, 0.00)));
        d = min(d, sdSegment(q, vec2(-0.35, -0.45), vec2(0.35, -0.45)));
    }
    return 1.0 - smoothstep(0.055, 0.095, d);
}
float ticketWord(vec2 p, int word) {
    float a = 0.0;
    if (word == 0) {
        a = max(a, glyphLine(p, -0.74, 0.68, 0.128, 0));
        a = max(a, glyphLine(p, -0.57, 0.68, 0.128, 5));
        a = max(a, glyphLine(p, -0.40, 0.68, 0.128, 1));
        a = max(a, glyphLine(p, -0.23, 0.68, 0.128, 2));
        a = max(a, glyphLine(p, -0.06, 0.68, 0.128, 3));
        a = max(a, glyphLine(p,  0.11, 0.68, 0.128, 7));
        a = max(a, glyphLine(p,  0.28, 0.68, 0.128, 4));
        a = max(a, glyphLine(p,  0.45, 0.68, 0.128, 8));
    } else {
        a = max(a, glyphLine(p, -0.34, 0.43, 0.165, 8));
        a = max(a, glyphLine(p, -0.09, 0.43, 0.165, 1));
        a = max(a,  glyphLine(p,  0.16, 0.43, 0.165, 6));
        a = max(a, glyphLine(p,  0.41, 0.43, 0.165, 10));
    }
    return a;
}
int blockGlyphMask(int glyph, int row) {
    if (glyph == 0) {
        if (row == 0) return 30; if (row == 1) return 17; if (row == 2) return 17; if (row == 3) return 30; if (row == 4) return 17; if (row == 5) return 17; return 30;
    } else if (glyph == 1) {
        if (row == 0) return 14; if (row == 1) return 17; if (row == 2) return 17; if (row == 3) return 31; if (row == 4) return 17; if (row == 5) return 17; return 17;
    } else if (glyph == 2) {
        if (row == 0) return 30; if (row == 1) return 17; if (row == 2) return 17; if (row == 3) return 30; if (row == 4) return 20; if (row == 5) return 18; return 17;
    } else if (glyph == 3) {
        if (row == 0) return 30; if (row == 1) return 17; if (row == 2) return 17; if (row == 3) return 17; if (row == 4) return 17; if (row == 5) return 17; return 30;
    } else if (glyph == 4) {
        if (row == 0) return 17; if (row == 1) return 25; if (row == 2) return 21; if (row == 3) return 19; if (row == 4) return 17; if (row == 5) return 17; return 17;
    } else if (glyph == 5) {
        if (row == 0) return 14; if (row == 1) return 17; if (row == 2) return 17; if (row == 3) return 17; if (row == 4) return 17; if (row == 5) return 17; return 14;
    } else if (glyph == 6) {
        if (row == 0) return 31; if (row == 1) return 4; if (row == 2) return 4; if (row == 3) return 4; if (row == 4) return 4; if (row == 5) return 4; return 4;
    } else if (glyph == 7) {
        if (row == 0) return 31; if (row == 1) return 4; if (row == 2) return 4; if (row == 3) return 4; if (row == 4) return 4; if (row == 5) return 4; return 31;
    } else if (glyph == 8) {
        if (row == 0) return 14; if (row == 1) return 17; if (row == 2) return 16; if (row == 3) return 23; if (row == 4) return 17; if (row == 5) return 17; return 14;
    } else if (glyph == 9) {
        if (row == 0) return 4; if (row == 1) return 12; if (row == 2) return 20; if (row == 3) return 31; if (row == 4) return 4; if (row == 5) return 4; return 4;
    } else if (glyph == 10) {
        if (row == 0) return 31; if (row == 1) return 1; if (row == 2) return 2; if (row == 3) return 4; if (row == 4) return 8; if (row == 5) return 8; return 8;
    }
    return 0;
}
float blockGlyph(vec2 p, float x, float y, float s, int glyph) {
    vec2 q = (p - vec2(x, y)) / s + vec2(2.5, 3.5);
    if (q.x < 0.0 || q.x >= 5.0 || q.y < 0.0 || q.y >= 7.0) return 0.0;
    int col = int(floor(q.x));
    int row = int(floor(q.y));
    int mask = blockGlyphMask(glyph, row);
    int bit = 1 << (4 - col);
    float on = (mask & bit) != 0 ? 1.0 : 0.0;
    vec2 cell = fract(q) - 0.5;
    float body = step(max(abs(cell.x), abs(cell.y)), 0.49);
    return on * body;
}
float ticketBlockText(vec2 p, int word) {
    float a = 0.0;
    if (word == 0) {
        a = max(a, blockGlyph(p, -0.82, 0.54, 0.055, 0));
        a = max(a, blockGlyph(p, -0.61, 0.54, 0.055, 5));
        a = max(a, blockGlyph(p, -0.40, 0.54, 0.055, 1));
        a = max(a, blockGlyph(p, -0.19, 0.54, 0.055, 2));
        a = max(a, blockGlyph(p,  0.02, 0.54, 0.055, 3));
        a = max(a, blockGlyph(p,  0.23, 0.54, 0.055, 7));
        a = max(a, blockGlyph(p,  0.44, 0.54, 0.055, 4));
        a = max(a, blockGlyph(p,  0.65, 0.54, 0.055, 8));
    } else if (word == 1) {
        a = max(a, blockGlyph(p, -0.68, 0.04, 0.095, 8));
        a = max(a, blockGlyph(p, -0.31, 0.04, 0.095, 1));
        a = max(a, blockGlyph(p,  0.06, 0.04, 0.095, 6));
        a = max(a, blockGlyph(p,  0.43, 0.04, 0.095, 7));
    } else {
        a = max(a, blockGlyph(p, -0.36, -0.56, 0.150, 9));
        a = max(a, blockGlyph(p,  0.36, -0.56, 0.150, 10));
    }
    return a;
}
void main() {
    vec2 uv = vUv;
    vec2 px = 1.0 / uResolution.xy;
    vec2 center = uv - 0.5;
    vec2 pixel = vUv * uResolution.xy;
    float loopDepth = smoothstep(0.12, 1.0, uThirst);
    float atomic = clamp(uAtomic, 0.0, 1.0);
    float whiteFade = clamp(uAtomicFade, 0.0, 1.0);
    float ripple = noise(center * vec2(5.0, 7.0) + vec2(uTime * 0.20, -uTime * 0.13));
    float tunnel = sin(length(center) * 34.0 - uTime * 2.2 + ripple * 5.5);
    float lineJitter = (hash(vec2(floor(uv.y * uResolution.y * 0.5), floor(uTime * 9.0))) - 0.5) * 0.0065;
    uv += normalize(center + vec2(0.0001)) * tunnel * (0.006 + loopDepth * 0.018 + uTowerAlign * 0.014 + atomic * 0.060);
    uv.x += lineJitter * (0.35 + uGlitch * 2.6 + loopDepth * 1.2 + atomic * 8.0);

    vec2 chroma = normalize(center + vec2(0.0001)) * (0.0025 + loopDepth * 0.007 + uTowerAlign * 0.010);
    vec3 base = scene(uv);
    vec3 color = vec3(scene(uv + chroma).r, base.g, scene(uv - chroma).b);
    float softGlare = smoothstep(0.62, 1.0, luma(color));
    vec3 bloom = scene(uv + vec2(px.x * 4.0, 0.0)) + scene(uv - vec2(px.x * 4.0, 0.0));
    bloom += scene(uv + vec2(0.0, px.y * 4.0)) + scene(uv - vec2(0.0, px.y * 4.0));
    bloom += scene(uv + chroma * 3.0) + scene(uv - chroma * 3.0);
    bloom *= 0.1667;
    color = mix(color, bloom * spectrum(uTime * 0.04 + ripple), softGlare * 0.42);
    color += spectrum(uTime * 0.08 + length(center) * 1.7) * softGlare * (0.20 + uTowerAlign * 0.24);

    vec3 sky = mix(vec3(0.05, 0.00, 0.16), vec3(0.00, 0.38, 0.48), smoothstep(0.18, 1.0, uv.y));
    float empty = 1.0 - smoothstep(0.015, 0.075, luma(color));
    color = mix(color, sky, empty * 0.88);

    float dust = noise(uv * vec2(5.0, 2.8) + vec2(uTime * 0.018, -uTime * 0.012));
    color = mix(color, spectrum(dust + uTime * 0.03), smoothstep(0.25, 0.92, dust) * 0.16);
    float glare = pow(max(0.0, 1.0 - length(center * vec2(1.2, 0.85))), 2.4);
    float fluorescentPulse = 0.92 + 0.035 * sin(uTime * 17.0) + 0.020 * sin(uTime * 43.0);
    color += spectrum(uTime * 0.11) * glare * 0.18 * fluorescentPulse;
    float whiteCore = pow(max(0.0, 1.0 - length(center * vec2(1.0, 0.72))), 2.1);
    color += vec3(2.8, 2.3, 0.78) * atomic * (0.58 + whiteCore * 1.75);
    color += spectrum(uTime * 0.30 + length(center) * 2.0) * atomic * 1.25;
    color = mix(color, vec3(1.0, 0.94, 0.52), atomic * 0.44);
    color += spectrum(uTime * 0.25 + uv.y) * uFalseStart * (0.42 + 0.24 * sin(uTime * 18.0));
    color = mix(color, spectrum(uTime * 0.12 + 0.4), uSignalReached * 0.22);
    color = mix(color, color + spectrum(uTime * 0.08 + 0.2), uDrinkPulse * 0.42);
    color += spectrum(uTime * 0.10 + length(center)) * uDrinkPulse * pow(max(0.0, 1.0 - length(center) * 1.6), 2.0) * 0.35;
    color += spectrum(uTime * 0.18 + uv.x) * uJetpackPulse * pow(max(0.0, 1.0 - abs(center.y + 0.34) * 2.2), 2.0) * (0.48 + 0.35 * sin(uTime * 24.0));
    color = mix(color, color * spectrum(uTime * 0.035 + loopDepth) + spectrum(loopDepth + 0.3) * 0.08, loopDepth * 0.38);

    float grain = hash(uv * uResolution.xy + floor(uTime * 24.0)) - 0.5;
    color += grain * 0.026;
    float scan = sin(pixel.y * 3.14159 + sin(uTime * 4.0) * 0.8) * 0.5 + 0.5;
    color *= 0.91 + scan * 0.09;
    color = color / (color + vec3(0.62));
    color = pow(max(color, vec3(0.0)), vec3(0.78));
    float tone = luma(color);
    color = mix(color * 0.92, color + vec3(0.035, 0.020, 0.050), smoothstep(0.08, 0.72, tone));
    color += vec3(0.018, 0.030, 0.035) * smoothstep(0.18, 0.86, uv.y);
    color *= spectrum(uv.x * 0.4 + uv.y * 0.2 + uTime * 0.025) * 0.35 + vec3(0.88);
    float vignette = smoothstep(0.55, 1.04, length(center));
    color *= 1.0 - vignette * 0.34;
    color = mix(vec3(luma(color)), color, 1.45);
    color = mix(color, vec3(0.0), whiteFade);

    float ticketLife = 1.0 - smoothstep(9.0, 15.0, uTime);
    if (ticketLife > 0.001) {
        vec2 ticketUv = (vUv - vec2(0.50, 0.31)) / vec2(0.72, 0.34) + vec2(0.5);
        ticketUv.y = 1.0 - ticketUv.y;
        float card = step(0.0, ticketUv.x) * step(ticketUv.x, 1.0) * step(0.0, ticketUv.y) * step(ticketUv.y, 1.0);
        vec3 ticket = texture(uTicket, clamp(ticketUv, vec2(0.0), vec2(1.0))).rgb;
        float shadow = step(0.0, ticketUv.x + 0.025) * step(ticketUv.x + 0.025, 1.0) * step(0.0, ticketUv.y - 0.035) * step(ticketUv.y - 0.035, 1.0);
        color = mix(color, vec3(0.0, 0.0, 0.0), shadow * (1.0 - card) * ticketLife * 0.34);
        color = mix(color, ticket, card * ticketLife);
    }
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
static void cacheUniformLocations() {
    g_sceneUniforms.mvp = glGetUniformLocation(g_sceneProgram, "uMvp");
    g_sceneUniforms.view = glGetUniformLocation(g_sceneProgram, "uView");
    g_sceneUniforms.eye = glGetUniformLocation(g_sceneProgram, "uEye");
    g_sceneUniforms.time = glGetUniformLocation(g_sceneProgram, "uTime");
    g_sceneUniforms.glitch = glGetUniformLocation(g_sceneProgram, "uGlitch");
    g_sceneUniforms.towerAlign = glGetUniformLocation(g_sceneProgram, "uTowerAlign");
    g_sceneUniforms.figureX = glGetUniformLocation(g_sceneProgram, "uFigureX");
    g_sceneUniforms.towerZ = glGetUniformLocation(g_sceneProgram, "uTowerZ");

    g_postUniforms.scene = glGetUniformLocation(g_postProgram, "uScene");
    g_postUniforms.ticket = glGetUniformLocation(g_postProgram, "uTicket");
    g_postUniforms.time = glGetUniformLocation(g_postProgram, "uTime");
    g_postUniforms.glitch = glGetUniformLocation(g_postProgram, "uGlitch");
    g_postUniforms.towerAlign = glGetUniformLocation(g_postProgram, "uTowerAlign");
    g_postUniforms.wrongWay = glGetUniformLocation(g_postProgram, "uWrongWay");
    g_postUniforms.signalReached = glGetUniformLocation(g_postProgram, "uSignalReached");
    g_postUniforms.falseStart = glGetUniformLocation(g_postProgram, "uFalseStart");
    g_postUniforms.drinkPulse = glGetUniformLocation(g_postProgram, "uDrinkPulse");
    g_postUniforms.thirst = glGetUniformLocation(g_postProgram, "uThirst");
    g_postUniforms.waterFuel = glGetUniformLocation(g_postProgram, "uWaterFuel");
    g_postUniforms.jetpackPulse = glGetUniformLocation(g_postProgram, "uJetpackPulse");
    g_postUniforms.health = glGetUniformLocation(g_postProgram, "uHealth");
    g_postUniforms.atomic = glGetUniformLocation(g_postProgram, "uAtomic");
    g_postUniforms.atomicFade = glGetUniformLocation(g_postProgram, "uAtomicFade");
    g_postUniforms.resolution = glGetUniformLocation(g_postProgram, "uResolution");
}
static void createTicketTexture() {
    constexpr int w = 768;
    constexpr int h = 384;
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HDC dc = CreateCompatibleDC(nullptr);
    HBITMAP bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HGDIOBJ oldBmp = SelectObject(dc, bmp);

    HBRUSH paper = CreateSolidBrush(RGB(244, 235, 194));
    RECT full{0, 0, w, h};
    FillRect(dc, &full, paper);
    DeleteObject(paper);

    HBRUSH band = CreateSolidBrush(RGB(18, 12, 34));
    RECT topBand{0, 0, w, 72};
    FillRect(dc, &topBand, band);
    DeleteObject(band);

    HPEN linePen = CreatePen(PS_SOLID, 4, RGB(28, 22, 42));
    HGDIOBJ oldPen = SelectObject(dc, linePen);
    MoveToEx(dc, 0, 302, nullptr);
    LineTo(dc, w, 302);
    for (int x = 590; x < w; x += 18) {
        MoveToEx(dc, x, 96, nullptr);
        LineTo(dc, x + 10, 284);
    }
    SelectObject(dc, oldPen);
    DeleteObject(linePen);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(246, 239, 174));
    HFONT titleFont = CreateFontA(44, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, "Arial");
    HGDIOBJ oldFont = SelectObject(dc, titleFont);
    RECT titleRect{34, 12, w - 34, 66};
    DrawTextA(dc, "BOARDING PASS", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SetTextColor(dc, RGB(20, 14, 36));
    HFONT labelFont = CreateFontA(50, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(dc, labelFont);
    RECT labelRect{44, 104, 580, 174};
    DrawTextA(dc, "BOARDING GATE", -1, &labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HFONT gateFont = CreateFontA(172, 0, 0, 0, FW_BLACK, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH | FF_SWISS, "Arial Black");
    SelectObject(dc, gateFont);
    RECT gateRect{40, 158, 438, 326};
    DrawTextA(dc, "47", -1, &gateRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HFONT smallFont = CreateFontA(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH | FF_SWISS, "Arial");
    SelectObject(dc, smallFont);
    RECT smallRect{44, 314, 560, 360};
    DrawTextA(dc, "TERMINAL LOOP / FIND GATE 47", -1, &smallRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    std::vector<unsigned char> rgba(w * h * 4);
    const unsigned char* bgra = static_cast<const unsigned char*>(bits);
    for (int i = 0; i < w * h; ++i) {
        rgba[i * 4 + 0] = bgra[i * 4 + 2];
        rgba[i * 4 + 1] = bgra[i * 4 + 1];
        rgba[i * 4 + 2] = bgra[i * 4 + 0];
        rgba[i * 4 + 3] = 255;
    }

    SelectObject(dc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(labelFont);
    DeleteObject(gateFont);
    DeleteObject(smallFont);
    SelectObject(dc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dc);

    glGenTextures(1, &g_ticketTex);
    glBindTexture(GL_TEXTURE_2D, g_ticketTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
static void resizeFramebuffer() {
    int targetW = std::max(1, static_cast<int>(g_width * kSceneRenderScale));
    int targetH = std::max(1, static_cast<int>(g_height * kSceneRenderScale));
    if (targetW == g_fbWidth && targetH == g_fbHeight && g_fbo) return;
    g_fbWidth = targetW;
    g_fbHeight = targetH;
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
    cacheUniformLocations();
    createTicketTexture();
    setupVertexStream(g_sceneVao, g_sceneVbo);
    setupVertexStream(g_atomicVao, g_atomicVbo);
    setupVertexStream(g_lineVao, g_lineVbo);
    setupVertexStream(g_fxLineVao, g_fxLineVbo);
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
    float g = glitchAmount();
    float atomic = atomicIntensity();
    float whiteFade = atomicEndFade();
    if (whiteFade >= 0.995f) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, g_width, g_height);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(g_hdc);
        return;
    }
    if (shouldRebuildWorld()) buildWorld();
    float cp = std::cos(g_pitch);
    Vec3 dir{std::sin(g_yaw) * cp, std::sin(g_pitch), std::cos(g_yaw) * cp};
    Vec3 towerFocus = currentSightingPosition();
    float towerDot = dot(norm(dir), norm(sub(towerFocus, g_player)));
    float towerAlign = smoothstep(0.985f, 0.999f, towerDot);
    towerAlign *= 0.82f + 0.18f * (0.5f + 0.5f * std::sin(g_time * 3.0f));
    Vec3 right = norm(cross(norm({dir.x, 0.0f, dir.z}), {0, 1, 0}));
    float shake = atomic * (0.75f + 1.05f * std::sin(g_time * 31.0f) * std::sin(g_time * 71.0f));
    Vec3 eye = add(g_player, add(mul(right, std::sin(g_time * 91.0f) * shake), {0.0f, std::cos(g_time * 77.0f) * shake * 0.62f, 0.0f}));
    Mat4 view = lookAt(eye, add(eye, dir), {0, 1, 0});
    Mat4 proj = perspective(66.0f + g * 2.5f + atomic * 20.0f, static_cast<float>(g_width) / std::max(1, g_height), 0.08f, 4800.0f);
    Mat4 mvp = multiply(proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glViewport(0, 0, g_fbWidth, g_fbHeight);
    glClearColor(0.66f, 0.78f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_sceneProgram);
    glUniformMatrix4fv(g_sceneUniforms.mvp, 1, GL_FALSE, mvp.m);
    glUniformMatrix4fv(g_sceneUniforms.view, 1, GL_FALSE, view.m);
    glUniform3f(g_sceneUniforms.eye, g_player.x, g_player.y, g_player.z);
    glUniform1f(g_sceneUniforms.time, g_time);
    glUniform1f(g_sceneUniforms.glitch, g);
    glUniform1f(g_sceneUniforms.towerAlign, towerAlign);
    glUniform1f(g_sceneUniforms.figureX, towerFocus.x);
    glUniform1f(g_sceneUniforms.towerZ, towerFocus.z);

    glDisable(GL_CULL_FACE);
    glBindVertexArray(g_sceneVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_sceneVbo);
    if (g_sceneBuffersDirty) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_triangles.size() * sizeof(Vertex)), g_triangles.data(), GL_DYNAMIC_DRAW);
    }
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_triangles.size()));
    buildAtomicBlastFx();
    if (!g_atomicTriangles.empty()) {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBindVertexArray(g_atomicVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_atomicVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_atomicTriangles.size() * sizeof(Vertex)), g_atomicTriangles.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_atomicTriangles.size()));
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(g_sceneVao);
    }

    glLineWidth(1.35f + g * 1.2f);
    glBindVertexArray(g_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
    if (g_sceneBuffersDirty) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_lines.size() * sizeof(Vertex)), g_lines.data(), GL_DYNAMIC_DRAW);
        g_sceneBuffersDirty = false;
    }
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_lines.size()));
    if (!g_frameLines.empty()) {
        glBindVertexArray(g_fxLineVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_fxLineVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_frameLines.size() * sizeof(Vertex)), g_frameLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_frameLines.size()));
    }
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_width, g_height);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_postProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_colorTex);
    glUniform1i(g_postUniforms.scene, 0);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, g_ticketTex);
    glUniform1i(g_postUniforms.ticket, 1);
    glActiveTexture(GL_TEXTURE0);
    glUniform1f(g_postUniforms.time, g_time);
    glUniform1f(g_postUniforms.glitch, g);
    glUniform1f(g_postUniforms.towerAlign, towerAlign);
    glUniform1f(g_postUniforms.wrongWay, g_wrongWaySignal);
    glUniform1f(g_postUniforms.signalReached, g_signalReached);
    glUniform1f(g_postUniforms.falseStart, g_falseStartFlash);
    glUniform1f(g_postUniforms.drinkPulse, g_drinkPulse);
    glUniform1f(g_postUniforms.thirst, g_thirst);
    glUniform1f(g_postUniforms.waterFuel, g_waterFuel);
    glUniform1f(g_postUniforms.jetpackPulse, g_jetpackPulse);
    glUniform1f(g_postUniforms.health, g_playerHealth);
    glUniform1f(g_postUniforms.atomic, atomic);
    glUniform1f(g_postUniforms.atomicFade, whiteFade);
    glUniform3f(g_postUniforms.resolution, static_cast<float>(g_width), static_cast<float>(g_height), 0.0f);
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
    float footY = position.y - kPlayerEyeHeight - 0.25f;
    float headY = position.y + 0.45f;
    float r = kPlayerFootRadius + 0.35f;
    for (const Aabb& b : g_buildingColliders) {
        if (headY < b.minY || footY > b.maxY) continue;
        if (position.x + r < b.minX || position.x - r > b.maxX) continue;
        if (position.z + r < b.minZ || position.z - r > b.maxZ) continue;
        return true;
    }
    return false;
}
static float clampedWorldX(float x) {
    return clampf(x, -305.0f, 305.0f);
}
static float clampedWorldZ(float z) {
    return clampf(z, finalTowerZ() - 120.0f, 620.0f);
}
static bool canOccupyHorizontal(float x, float z, float maxStepUp) {
    float currentGround = playerGroundHeight(g_player.x, g_player.z);
    float ground = playerGroundHeight(x, z);
    if (ground > currentGround + maxStepUp) return false;
    Vec3 probe{x, ground + kPlayerEyeHeight, z};
    return !collidesWithBuilding(probe);
}
static bool tryMoveHorizontalStep(float dx, float dz, float maxStepUp) {
    bool moved = false;
    if (std::fabs(dx) > 0.0001f) {
        float nx = clampedWorldX(g_player.x + dx);
        if (canOccupyHorizontal(nx, g_player.z, maxStepUp)) {
            g_player.x = nx;
            moved = true;
        }
    }
    if (std::fabs(dz) > 0.0001f) {
        float nz = clampedWorldZ(g_player.z + dz);
        if (canOccupyHorizontal(g_player.x, nz, maxStepUp)) {
            g_player.z = nz;
            moved = true;
        }
    }
    return moved;
}
static bool tryMoveHorizontal(float dx, float dz, float maxStepUp = 2.8f) {
    float dist = std::sqrt(dx * dx + dz * dz);
    int steps = std::max(1, static_cast<int>(std::ceil(dist / 0.58f)));
    bool moved = false;
    float sx = dx / static_cast<float>(steps);
    float sz = dz / static_cast<float>(steps);
    for (int i = 0; i < steps; ++i) {
        moved = tryMoveHorizontalStep(sx, sz, maxStepUp) || moved;
    }
    return moved;
}
static void resolvePlayerPenetration() {
    for (int pass = 0; pass < 4; ++pass) {
        float ground = playerGroundHeight(g_player.x, g_player.z);
        float footY = ground + kPlayerGroundClearance - 0.25f;
        float headY = ground + kPlayerEyeHeight + 0.45f;
        float r = kPlayerFootRadius + 0.38f;
        bool resolved = false;
        for (const Aabb& b : g_buildingColliders) {
            if (headY < b.minY || footY > b.maxY) continue;
            float overlapLeft = (g_player.x + r) - b.minX;
            float overlapRight = b.maxX - (g_player.x - r);
            float overlapBack = (g_player.z + r) - b.minZ;
            float overlapFront = b.maxZ - (g_player.z - r);
            if (overlapLeft <= 0.0f || overlapRight <= 0.0f || overlapBack <= 0.0f || overlapFront <= 0.0f) continue;
            float pushX = overlapLeft < overlapRight ? -overlapLeft : overlapRight;
            float pushZ = overlapBack < overlapFront ? -overlapBack : overlapFront;
            if (std::fabs(pushX) < std::fabs(pushZ)) {
                g_player.x = clampedWorldX(g_player.x + pushX);
            } else {
                g_player.z = clampedWorldZ(g_player.z + pushZ);
            }
            resolved = true;
            break;
        }
        if (!resolved) break;
    }
}
static float movingWalkwayVelocity() {
    float localZ = std::fmod(g_player.z + 100000.0f, 360.0f);
    int dirCell = (static_cast<int>(std::floor(g_player.z / 360.0f)) & 1) ? 1 : -1;
    auto band = [](float d, float halfWidth, float feather) {
        return 1.0f - smoothstep(halfWidth - feather, halfWidth, std::fabs(d));
    };
    float left = band(g_player.x + 82.0f, 26.0f, 8.0f) * smoothstep(284.0f, 298.0f, localZ);
    float right = band(g_player.x - 82.0f, 26.0f, 8.0f) * smoothstep(78.0f, 92.0f, localZ) * (1.0f - smoothstep(196.0f, 210.0f, localZ));
    return (left * static_cast<float>(dirCell) + right * static_cast<float>(-dirCell)) * 20.0f;
}
static void update(float dt) {
    updateMouse();
    updateAtomicBlast(dt);
    if (g_atomicEnded) {
        updateTelemetry(dt);
        return;
    }
    if (g_playerHealth <= 0.0f) {
        g_playerHealth = 1.0f;
        g_player = {0.0f, 8.0f, 260.0f};
        g_verticalVelocity = 0.0f;
        g_walkBobPhase = 0.0f;
        g_worldDirty = true;
    }
    float speed = keyDown(VK_SHIFT) ? 34.0f : 18.0f;
    Vec3 forward{std::sin(g_yaw), 0.0f, std::cos(g_yaw)};
    Vec3 right{std::cos(g_yaw), 0.0f, -std::sin(g_yaw)};
    Vec3 move{0, 0, 0};
    if (keyDown('W')) move = add(move, forward);
    if (keyDown('S')) move = sub(move, forward);
    if (keyDown('D')) move = sub(move, right);
    if (keyDown('A')) move = add(move, right);
    float len = std::sqrt(move.x * move.x + move.z * move.z);
    Vec3 sighting = currentSightingPosition();
    Vec3 flatToTower = norm(sub({sighting.x, g_player.y, sighting.z}, g_player));
    Vec3 flatLook = norm(forward);
    float lookAway = clampf(-dot(flatLook, flatToTower), 0.0f, 1.0f);
    float targetWrongLook = smoothstep(0.18f, 0.88f, lookAway);
    float targetWrongMove = 0.0f;
    if (len > 0.001f) {
        move = mul(move, 1.0f / len);
        float away = clampf(-dot(move, flatToTower), 0.0f, 1.0f);
        targetWrongMove = smoothstep(0.15f, 0.85f, away);
        tryMoveHorizontal(move.x * speed * dt, move.z * speed * dt);
    }
    float conveyorVelocity = movingWalkwayVelocity();
    if (std::fabs(conveyorVelocity) > 0.05f) {
        tryMoveHorizontal(0.0f, conveyorVelocity * dt, 3.5f);
        g_jetpackPulse = clampf(g_jetpackPulse + dt * (1.6f + std::fabs(conveyorVelocity) * 0.08f), 0.0f, 1.0f);
    } else {
        g_jetpackPulse = clampf(g_jetpackPulse - dt * 2.0f, 0.0f, 1.0f);
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
    g_player.x = clampedWorldX(g_player.x);
    g_player.z = clampedWorldZ(g_player.z);
    float towerDistance = std::sqrt((g_player.x - sighting.x) * (g_player.x - sighting.x) + (g_player.z - sighting.z) * (g_player.z - sighting.z));
    if (towerDistance < kSignalReachDistance) {
        if (g_signalStage >= kFinalSignalStage) {
            g_signalReached = clampf(g_signalReached + dt * 0.45f, 0.0f, 1.0f);
            g_falseStartFlash = clampf(g_falseStartFlash + dt * 0.35f, 0.0f, 1.0f);
        } else {
            ++g_signalStage;
            g_falseStartFlash = 1.0f;
            g_signalReached = 0.0f;
            g_wrongWaySignal = 0.0f;
            g_wrongWaySustain = 0.0f;
            g_player.z = clampedWorldZ(g_player.z + 118.0f);
            float landingGround = playerGroundHeight(g_player.x, g_player.z);
            if (collidesWithBuilding({g_player.x, landingGround + kPlayerEyeHeight, g_player.z})) {
                int cell = static_cast<int>(std::floor(g_player.z / 160.0f));
                int lane = cell == 0 ? 2 : mazePathLane(cell);
                float safeX = clampedWorldX(mazeLaneX(lane));
                if (canOccupyHorizontal(safeX, g_player.z, 100.0f)) {
                    g_player.x = safeX;
                }
            }
            g_worldDirty = true;
        }
    } else {
        g_signalReached = clampf(g_signalReached - dt * 0.10f, 0.0f, 1.0f);
    }
    resolvePlayerPenetration();
    g_falseStartFlash = clampf(g_falseStartFlash - dt * 0.55f, 0.0f, 1.0f);
    g_thirst = clampf(static_cast<float>(g_signalStage) / static_cast<float>(kFinalSignalStage), 0.0f, 1.0f);
    g_waterFuel = clampf(1.0f - towerDistance / 900.0f, 0.0f, 1.0f);
    g_drinkPulse = clampf(g_falseStartFlash, 0.0f, 1.0f);
    float walkAmount = clampf(len + std::fabs(conveyorVelocity) / 20.0f, 0.0f, 1.0f);
    g_walkBobPhase += dt * lerp(2.2f, 7.0f, walkAmount);
    float bob = std::sin(g_walkBobPhase) * lerp(0.018f, 0.070f, walkAmount);
    float groundY = playerGroundHeight(g_player.x, g_player.z) + kPlayerEyeHeight;
    if (g_isGrounded && keyDown(VK_SPACE)) {
        g_verticalVelocity = 13.5f;
        g_isGrounded = false;
    }
    if (!g_isGrounded) {
        g_verticalVelocity -= 32.0f * dt;
        g_player.y += g_verticalVelocity * dt;
        g_player.y = std::min(g_player.y, groundY + 60.0f);
        if (g_player.y <= groundY) {
            g_player.y = groundY;
            g_verticalVelocity = 0.0f;
            g_isGrounded = true;
        }
    } else {
        float targetY = groundY + bob;
        if (g_player.y < groundY - 0.05f) {
            g_player.y = groundY;
        } else {
            g_player.y = lerp(g_player.y, targetY, 1.0f - std::exp(-dt * 13.0f));
        }
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
        if (!g_mouseCaptured) {
            g_mouseCaptured = true;
            ShowCursor(FALSE);
        }
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
    wc.lpszClassName = "EndlessAirportGateWindow";
    RegisterClassA(&wc);

    RECT rect{0, 0, g_width, g_height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowA("EndlessAirportGateWindow", "The Endless Airport Gate - Modern OpenGL 3.3", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                           nullptr, nullptr, instance, nullptr);
    if (!g_hwnd || !createModernContext(g_hwnd) || !initRenderer()) {
        MessageBoxA(nullptr, "Could not initialize a shader-based OpenGL renderer.", "The Endless Airport Gate", MB_ICONERROR);
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




