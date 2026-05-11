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
struct Sniper {
    int id;
    Vec3 pos;
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
    int32_t leftEyeDestroyed;
    int32_t rightEyeDestroyed;
    int32_t chestHits;
    int32_t requiredChestHits;
    int32_t colossusDefeated;
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
static float g_drinkPulse = 0.0f;
static float g_thirst = 0.35f;
static float g_waterFuel = 0.65f;
static float g_jetpackPulse = 0.0f;
static float g_weaponPulse = 0.0f;
static float g_hitPulse = 0.0f;
static float g_damagePulse = 0.0f;
static float g_tracerPulse = 0.0f;
static Vec3 g_tracerStart{};
static Vec3 g_tracerEnd{};
static float g_enemyTracerPulse = 0.0f;
static Vec3 g_enemyTracerStart{};
static Vec3 g_enemyTracerEnd{};
static float g_incomingDirX = 0.0f;
static float g_incomingDirY = 0.0f;
static float g_playerHealth = 1.0f;
static float g_sniperTimer = 0.0f;
static bool g_fireRequested = false;
static float g_atomicTime = -1000.0f;
static Vec3 g_atomicOrigin{0.0f, 0.0f, 0.0f};
static bool g_atomicTestKeyWasDown = false;
static bool g_atomicEnded = false;
static bool g_leftEyeDestroyed = false;
static bool g_rightEyeDestroyed = false;
static int g_chestHits = 0;
static bool g_colossusDefeated = false;
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
static GLuint g_depthRb = 0;
static std::vector<Vertex> g_triangles;
static std::vector<Vertex> g_atomicTriangles;
static std::vector<Vertex> g_lines;
static std::vector<Vertex> g_frameLines;
static std::vector<Aabb> g_buildingColliders;
static std::vector<Sniper> g_snipers;
static std::vector<int> g_killedSnipers;
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
    GLint weaponPulse = -1;
    GLint hitPulse = -1;
    GLint damagePulse = -1;
    GLint health = -1;
    GLint incomingDir = -1;
    GLint atomic = -1;
    GLint atomicFade = -1;
    GLint resolution = -1;
};
static SceneUniforms g_sceneUniforms;
static PostUniforms g_postUniforms;

static constexpr float kTowerZ = -1850.0f;
static constexpr float kStartZ = 420.0f;
static constexpr float kSignalReachDistance = 150.0f;
static constexpr float kFalseStartSpacing = 1350.0f;
static constexpr int kFinalSignalStage = 10;
static constexpr int kRequiredChestHits = 12;
static constexpr float kPlayerEyeHeight = 5.8f;
static constexpr float kPlayerFootRadius = 1.35f;
static constexpr float kPlayerGroundClearance = 0.38f;
static constexpr float kTerrainMeshStep = 22.0f;
static constexpr float kAtomicDropSeconds = 3.0f;
static constexpr uint32_t kTelemetryMagic = 0x57464847u; // WFHG
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
static bool keyDown(int vk);
static float atomicIntensity() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    if (t < 0.0f) return smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime) * 0.22f;
    float flash = smoothstep(6.5f, 0.0f, t);
    float furnace = smoothstep(0.3f, 4.2f, t) * smoothstep(42.0f, 7.0f, t) * 0.92f;
    return clampf(flash + furnace, 0.0f, 1.0f);
}
static float atomicEndFade() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    return t <= 12.0f ? 0.0f : smoothstep(12.0f, 19.0f, t);
}
static float atomicFinalConvulsion() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    return smoothstep(9.5f, 14.0f, t) * (1.0f - smoothstep(18.5f, 22.0f, t));
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
    return 0.05f + smoothstep(0.05f, 1.0f, p) * (0.16f + pulse * 0.10f);
}
static float terrainHeight(float x, float z) {
    float warpX = fbm(x * 0.0038f + 31.0f, z * 0.0038f - 17.0f) * 85.0f;
    float warpZ = fbm(x * 0.0042f - 9.0f, z * 0.0042f + 25.0f) * 85.0f;
    float dunes = std::sin((x + warpX) * 0.010f + (z + warpZ) * 0.004f) * 6.2f;
    dunes += std::sin((x - warpX) * 0.021f + z * 0.012f) * 2.6f;
    float h = fbm(x * 0.006f, z * 0.006f) * 13.0f + fbm((x + warpX) * 0.018f, (z + warpZ) * 0.018f) * 4.0f + dunes;
    h -= std::exp(-(x * x) / 1150.0f) * 3.0f;
    h += smoothstep(240.0f, 760.0f, std::fabs(x)) * 5.0f;
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

static Vec3 sightingPositionForStage(int stage) {
    float z = kTowerZ - kFalseStartSpacing * static_cast<float>(stage);
    float side = rand01(stage * 17 + 31, 913) < 0.5f ? -1.0f : 1.0f;
    float x = side * (34.0f + rand01(stage * 41 + 7, 1201) * 290.0f);
    if (stage % 4 == 0) x *= 0.18f;
    if (stage == 0) x = 0.0f;
    float y = renderedSurfaceHeight(x, z);
    float perch = rand01(stage * 89 + 3, 771);
    if (perch > 0.55f) y += 22.0f + rand01(stage + 9, 441) * 70.0f;
    else if (perch > 0.28f) y += 9.0f + rand01(stage + 13, 443) * 18.0f;
    return {x, y, z};
}

static Vec3 currentSightingPosition() {
    return sightingPositionForStage(g_signalStage);
}
static Vec3 encounterSightingPosition() {
    Vec3 p = currentSightingPosition();
    float base = renderedSurfaceHeight(p.x, p.z);
    if (p.y > base + 12.0f) p.y += 0.25f;
    return p;
}
static float colossusScaleFor(Vec3 p) {
    float distance = std::sqrt((g_player.x - p.x) * (g_player.x - p.x) + (g_player.z - p.z) * (g_player.z - p.z));
    return clampf(distance * 0.030f, 48.0f, 135.0f);
}
static void colossusTargets(Vec3& leftEye, Vec3& rightEye, Vec3& chest, float& eyeRadius, float& chestRadius) {
    Vec3 p = encounterSightingPosition();
    float s = colossusScaleFor(p);
    leftEye = {p.x - s * 0.075f, p.y + s * 3.66f, p.z - s * 0.16f};
    rightEye = {p.x + s * 0.075f, p.y + s * 3.66f, p.z - s * 0.16f};
    chest = {p.x, p.y + s * 2.28f, p.z - s * 0.17f};
    eyeRadius = s * 0.13f;
    chestRadius = s * 0.24f;
}

static void initTelemetry() {
    g_telemetryMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                            sizeof(SignalTelemetry), "Local\\TheWhiteFigureSignalTelemetry");
    if (g_telemetryMapping) {
        g_telemetry = static_cast<SignalTelemetry*>(
            MapViewOfFile(g_telemetryMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SignalTelemetry)));
    }
    if (g_telemetry) {
        std::memset(g_telemetry, 0, sizeof(SignalTelemetry));
        g_telemetry->magic = kTelemetryMagic;
        g_telemetry->version = 4;
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
        std::snprintf(g_telemetryPath, sizeof(g_telemetryPath), "%sTheWhiteFigureSignalTelemetry.json", exePath);
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
                 "  \"magic\": \"WFHG\",\n"
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
                 "  \"visibleTowerZ\": %.3f,\n"
                 "  \"falseStartStage\": %d,\n"
                 "  \"finalStage\": %d,\n"
                 "  \"signalReached\": %.3f,\n"
                 "  \"falseStartFlash\": %.3f,\n"
                 "  \"thirst\": %.3f,\n"
                 "  \"waterFuel\": %.3f,\n"
                 "  \"jetpackPulse\": %.3f,\n"
                 "  \"jetpackActive\": %s,\n"
                 "  \"health\": %.3f,\n"
                 "  \"isGrounded\": %s,\n"
                 "  \"colossus\": {\"leftEyeDestroyed\": %s, \"rightEyeDestroyed\": %s, \"chestHits\": %d, \"requiredChestHits\": %d, \"defeated\": %s}\n"
                 "}\n",
                 t.version, t.byteSize, t.sequence, t.timeSeconds, t.realDistance, t.planarDistance,
                 t.threeDimensionalDistance, t.playerX, t.playerY, t.playerZ, t.playerVelocityX,
                 t.playerVelocityY, t.playerVelocityZ, t.playerSpeed, t.playerHorizontalSpeed,
                 t.playerFacingX, t.playerFacingY, t.playerFacingZ, t.playerMoveDirX, t.playerMoveDirY,
                 t.playerMoveDirZ, t.playerYaw, t.playerPitch, t.signalX, t.signalY, t.signalZ,
                 t.visibleTowerZ, t.falseStartStage, t.finalStage, t.signalReached, t.falseStartFlash,
                 t.thirst, t.waterFuel, t.jetpackPulse, t.jetpackPulse > 0.08f ? "true" : "false",
                 t.health, t.isGrounded ? "true" : "false", t.leftEyeDestroyed ? "true" : "false",
                 t.rightEyeDestroyed ? "true" : "false", t.chestHits, t.requiredChestHits,
                 t.colossusDefeated ? "true" : "false");
    std::fclose(f);
    MoveFileExA(tmpPath, g_telemetryPath, MOVEFILE_REPLACE_EXISTING);
}

static void updateTelemetry(float dt) {
    Vec3 realSignal = sightingPositionForStage(kFinalSignalStage);
    float signalX = realSignal.x;
    float signalY = realSignal.y;
    float signalZ = realSignal.z;
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
    t.version = 4;
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
    t.thirst = g_thirst;
    t.waterFuel = g_waterFuel;
    t.jetpackPulse = g_jetpackPulse;
    t.health = g_playerHealth;
    t.isGrounded = g_isGrounded ? 1 : 0;
    t.leftEyeDestroyed = g_leftEyeDestroyed ? 1 : 0;
    t.rightEyeDestroyed = g_rightEyeDestroyed ? 1 : 0;
    t.chestHits = g_chestHits;
    t.requiredChestHits = kRequiredChestHits;
    t.colossusDefeated = g_colossusDefeated ? 1 : 0;
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
static void addSniper(int id, Vec3 pos);
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
    Vec3 haze{0.82f, 0.66f, 0.44f};
    return {lerp(color.x, haze.x, t), lerp(color.y, haze.y, t), lerp(color.z, haze.z, t)};
}
static void pushBuilding(Vec3 baseCenter, Vec3 halfSize, Vec3 color, float shine) {
    Vec3 center{baseCenter.x, baseCenter.y + halfSize.y, baseCenter.z};
    color = buildingHazeColor(color, baseCenter.x, baseCenter.z);
    shine *= 1.0f - smoothstep(820.0f, 2050.0f, std::sqrt((baseCenter.x - g_player.x) * (baseCenter.x - g_player.x) + (baseCenter.z - g_player.z) * (baseCenter.z - g_player.z))) * 0.82f;
    Vec3 foundationColor{color.x * 0.78f, color.y * 0.76f, color.z * 0.72f};
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
    const int radius = 82;
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
            float sand = valueNoise(x0 * 0.018f, z0 * 0.018f);
            float stone = valueNoise(x0 * 0.061f + 40.0f, z0 * 0.061f - 14.0f);
            Vec3 c{0.58f + sand * 0.18f + stone * 0.06f, 0.48f + sand * 0.14f + stone * 0.04f, 0.33f + sand * 0.08f};
            float shine = 0.05f + stone * 0.12f;
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
    for (int zc = cz - 12; zc <= cz + 10; ++zc) {
        for (int xc = cx - 10; xc <= cx + 10; ++xc) {
            float r = rand01(xc, zc);
            float x = xc * 64.0f + (rand01(xc + 90, zc) - 0.5f) * 42.0f;
            float z = zc * 64.0f + (rand01(xc, zc + 90) - 0.5f) * 42.0f;
            if (std::fabs(x) < 30.0f) x += x < 0 ? -34.0f : 34.0f;
            if (r < 0.16f) {
                float y = renderedSurfaceHeight(x, z) + 0.18f;
                float yaw = r * 6.28f;
                Vec3 limestone{0.62f, 0.57f, 0.46f};
                pushCube({x, y, z}, {5.0f + r * 10.0f, 0.18f, 1.0f + r * 3.0f}, limestone, 0.08f, yaw);
            }
            else if (r < 0.25f) {
                float y = renderedSurfaceHeight(x, z);
                float h = 9.0f + r * 22.0f;
                Vec3 shade{0.50f, 0.47f, 0.39f};
                pushCube({x, y + h * 0.5f, z}, {0.42f, h * 0.5f, 0.42f}, shade, 0.05f);
                pushCube({x, y + h + 0.18f, z}, {5.8f, 0.18f, 5.8f}, {0.68f, 0.63f, 0.52f}, 0.10f);
            }
            else if (r < 0.30f) {
                float sx = 24.0f + r * 28.0f;
                float sy = 18.0f + r * 18.0f;
                float sz = 8.0f + r * 7.0f;
                float y = 0.0f;
                if (!groundedBuildingBase(x, z, sx, sz, y)) continue;
                pushBuilding({x, y, z}, {sx, sy, sz}, {0.58f, 0.55f, 0.48f}, 0.07f);
                if (r < 0.385f) {
                    pushBuilding({x + sx * 0.18f, y + sy * 2.0f, z}, {sx * 0.42f, sy * 0.45f, sz * 0.92f}, {0.64f, 0.60f, 0.50f}, 0.06f);
                }
            }
        }
    }
}
static void buildGroundDetail() {
    int cx = static_cast<int>(std::floor(g_player.x / 24.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 24.0f));
    for (int zc = cz - 16; zc <= cz + 12; ++zc) {
        for (int xc = cx - 12; xc <= cx + 12; ++xc) {
            float r = rand01(xc + 1700, zc - 410);
            float x = xc * 24.0f + (rand01(xc + 17, zc + 29) - 0.5f) * 17.0f;
            float z = zc * 24.0f + (rand01(xc - 31, zc + 43) - 0.5f) * 17.0f;
            if (std::fabs(x) < 11.0f && r > 0.18f) continue;
            float y = renderedSurfaceHeight(x, z) + 0.18f;
            float yaw = rand01(xc, zc) * 6.28318f;
            if (r < 0.18f) {
                float s = 0.25f + rand01(xc + 2, zc) * 0.75f;
                Vec3 c{0.54f + r * 0.18f, 0.47f + r * 0.12f, 0.32f + r * 0.08f};
                pushCube({x, y + s * 0.18f, z}, {s * 1.8f, s * 0.18f, s * 1.1f}, c, 0.08f, yaw);
            } else if (r < 0.30f) {
                float sx = 1.8f + rand01(xc + 8, zc) * 3.5f;
                float sz = 0.35f + rand01(xc, zc + 8) * 1.5f;
                Vec3 c{0.64f, 0.56f + r * 0.06f, 0.40f + r * 0.04f};
                pushCube({x, y + 0.06f, z}, {sx, 0.07f, sz}, c, 0.10f, yaw);
            } else if (r < 0.39f) {
                Vec3 c{0.52f, 0.45f + glitchAmount() * 0.04f, 0.32f + glitchAmount() * 0.03f};
                float h = 0.8f + rand01(xc + 5, zc - 3) * 2.1f;
                pushLine({x, y, z}, {x + std::cos(yaw) * 0.8f, y + h, z + std::sin(yaw) * 0.8f}, c, 0.9f);
                pushLine({x + 0.4f, y, z - 0.2f}, {x + std::cos(yaw + 1.8f) * 0.7f, y + h * 0.72f, z + std::sin(yaw + 1.8f) * 0.7f}, c, 0.9f);
            } else if (r < 0.54f) {
                Vec3 c{0.44f, 0.37f + glitchAmount() * 0.03f, 0.24f + glitchAmount() * 0.02f};
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
    for (int zc = cz - 12; zc <= cz + 10; ++zc) {
        for (int xc = cx - 8; xc <= cx + 8; ++xc) {
            float r = rand01(xc + 3000, zc - 2200);
            if (r > 0.035f) continue;
            float x = xc * 96.0f + (rand01(xc + 11, zc + 19) - 0.5f) * 54.0f;
            float z = zc * 96.0f + (rand01(xc - 23, zc + 37) - 0.5f) * 54.0f;
            if (std::fabs(x) < 46.0f) x += x < 0.0f ? -52.0f : 52.0f;
            float y = renderedSurfaceHeight(x, z);
            float yaw = rand01(xc - 7, zc + 5) * 6.28318f;
            if (r < 0.15f) {
                float sx = 48.0f + rand01(xc, zc) * 78.0f;
                float sy = 36.0f + rand01(xc + 1, zc) * 76.0f;
                float sz = 22.0f + rand01(xc + 4, zc - 3) * 42.0f;
                float baseY = 0.0f;
                if (!groundedBuildingBase(x, z, sx, sz, baseY)) continue;
                Vec3 concrete{0.54f, 0.52f, 0.45f};
                pushBuilding({x, baseY, z}, {sx, sy, sz}, concrete, 0.08f);
                if (r < 0.055f) {
                    pushBuilding({x - sx * 0.35f, baseY + sy * 2.0f, z + sz * 0.15f}, {sx * 0.28f, sy * 0.42f, sz * 0.74f}, {0.48f, 0.47f, 0.41f}, 0.06f);
                } else if (r < 0.10f) {
                    pushBuilding({x + sx * 0.18f, baseY + sy * 2.0f, z - sz * 0.10f}, {sx * 0.55f, sy * 0.24f, sz * 0.86f}, {0.58f, 0.55f, 0.47f}, 0.07f);
                }
            }
        }
    }
}
static void buildDistantSilhouettes() {
    int cz = static_cast<int>(std::floor(g_player.z / 180.0f));
    float g = glitchAmount();
    for (int i = -12; i <= 10; ++i) {
        int cell = cz + i;
        for (int side = -1; side <= 1; side += 2) {
            float r = rand01(cell + side * 23, 1200);
            if (r > 0.055f) continue;
            float z = cell * 180.0f + rand01(cell, side * 91) * 95.0f;
            float x = side * (150.0f + rand01(cell, side * 77) * 260.0f);
            float h = 88.0f + rand01(cell + 5, side * 33) * 170.0f;
            float w = 44.0f + rand01(cell + 7, side * 44) * 92.0f;
            float sz = 22.0f + r * 58.0f;
            float y = 0.0f;
            if (!groundedBuildingBase(x, z, w, sz, y)) continue;
            Vec3 dark{0.42f, 0.39f + g * 0.02f, 0.33f + g * 0.02f};
            pushBuilding({x, y, z}, {w, h * 0.5f, sz}, dark, 0.06f);
            if (rand01(cell + side * 331, 5077) < 0.42f) {
                addSniper(cell * 10 + side + 10000, {x, y + h + 1.0f, z - sz * 0.25f});
            }
            if (r < 0.07f) {
                pushBuilding({x + side * w * 0.20f, y + h, z}, {w * 0.52f, h * 0.18f, sz * 0.88f}, {0.36f, 0.34f + g * 0.02f, 0.29f + g * 0.02f}, 0.52f);
            }
        }
    }
}
static void buildBrutalistBlocks() {
    int base = static_cast<int>(std::floor(g_player.z / 220.0f));
    for (int i = -12; i <= 10; ++i) {
        int cell = base + i;
        for (int side = -1; side <= 1; side += 2) {
            float gate = rand01(cell + side * 301, 4040);
            if (gate > 0.055f) continue;
            float z = cell * 220.0f + 70.0f + rand01(cell, side * 409) * 115.0f;
            float x = side * (120.0f + rand01(cell, side * 503) * 220.0f);
            float sx = 48.0f + rand01(cell + 7, side * 601) * 86.0f;
            float sy = (34.0f + rand01(cell + 11, side * 607) * 76.0f) * 1.65f;
            float sz = 26.0f + rand01(cell + 13, side * 613) * 56.0f;
            float y = 0.0f;
            if (!groundedBuildingBase(x, z, sx, sz, y)) continue;
            Vec3 concrete{0.56f, 0.54f, 0.47f};
            pushBuilding({x, y, z}, {sx, sy, sz}, concrete, 0.07f);
            if (rand01(cell + side * 419, 6089) < 0.55f) {
                addSniper(cell * 10 + side + 20000, {x + side * sx * 0.22f, y + sy * 2.0f + 1.0f, z - sz * 0.25f});
            }
            if (gate < 0.055f) {
                pushBuilding({x + side * sx * 0.28f, y + sy * 2.0f, z - sz * 0.12f}, {sx * 0.42f, sy * 0.34f, sz * 0.82f}, {0.50f, 0.48f, 0.42f}, 0.06f);
            }
        }
    }
}
static void buildAtmosphereDetail() {
    int base = static_cast<int>(std::floor(g_player.z / 34.0f));
    float gust = std::sin(g_time * 0.23f) * 0.55f + std::sin(g_time * 0.61f) * 0.25f;
    for (int i = -16; i <= 16; ++i) {
        int cell = base + i;
        for (int j = 0; j < 5; ++j) {
            float seed = rand01(cell + j * 17, 803);
            float x = g_player.x + (rand01(cell + j * 31, 802) - 0.5f) * 310.0f + gust * seed * 20.0f;
            float z = g_player.z + (rand01(cell + j * 23, 801) - 0.5f) * 420.0f;
            float ground = renderedSurfaceHeight(x, z);
            float y = ground + 1.0f + seed * 9.0f;
            float shimmer = std::sin(g_time * (1.0f + seed * 2.0f) + seed * 40.0f) * 1.6f;
            Vec3 c{0.78f, 0.68f, 0.48f};
            float len = 9.0f + seed * 24.0f;
            pushLine({x - len * 0.5f, y, z}, {x + len * 0.5f, y + shimmer * 0.15f, z + 1.0f}, {c.x * 0.14f, c.y * 0.12f, c.z * 0.08f}, 0.10f);
        }
    }
}
static bool oasisSpot(int cx, int cz, Vec3& center, float& radius) {
    float gate = rand01(cx + 9100, cz - 7400);
    bool starterOasis = (cx == 0 && cz == 0) || (cx == 0 && cz == -1);
    if (!starterOasis && gate > 0.34f) return false;
    float x = cx * 300.0f + (rand01(cx + 19, cz + 31) - 0.5f) * 135.0f;
    float z = cz * 300.0f + (rand01(cx - 29, cz + 47) - 0.5f) * 135.0f;
    if (starterOasis) {
        x = cx == 0 && cz == 0 ? 42.0f : -58.0f;
        z = cx == 0 && cz == 0 ? 230.0f : -185.0f;
    }
    if (std::fabs(x) < 34.0f) x += x < 0.0f ? -48.0f : 48.0f;
    radius = starterOasis ? 145.0f : 68.0f + rand01(cx + 53, cz - 61) * 76.0f;
    float y = renderedSurfaceHeight(x, z);
    float low = y;
    float high = y;
    for (int i = 0; i < 8; ++i) {
        float a = i / 8.0f * 6.2831853f;
        float h = renderedSurfaceHeight(x + std::cos(a) * radius * 0.78f, z + std::sin(a) * radius * 0.78f);
        low = std::min(low, h);
        high = std::max(high, h);
    }
    if (!starterOasis && high - low > 16.0f) return false;
    center = {x, low + 0.055f, z};
    return true;
}
static float nearestOasisDistance(float& radiusOut) {
    int cx = static_cast<int>(std::floor(g_player.x / 300.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 300.0f));
    float best = 1.0e9f;
    radiusOut = 0.0f;
    for (int zc = cz - 3; zc <= cz + 3; ++zc) {
        for (int xc = cx - 3; xc <= cx + 3; ++xc) {
            Vec3 c{};
            float r = 0.0f;
            if (!oasisSpot(xc, zc, c, r)) continue;
            float dx = g_player.x - c.x;
            float dz = g_player.z - c.z;
            float d = std::sqrt(dx * dx + dz * dz);
            if (d < best) {
                best = d;
                radiusOut = r;
            }
        }
    }
    return best;
}
static void buildOases() {
    int cx = static_cast<int>(std::floor(g_player.x / 300.0f));
    int cz = static_cast<int>(std::floor(g_player.z / 300.0f));
    for (int zc = cz - 4; zc <= cz + 4; ++zc) {
        for (int xc = cx - 4; xc <= cx + 4; ++xc) {
            Vec3 c{};
            float r = 0.0f;
            if (!oasisSpot(xc, zc, c, r)) continue;
            Vec3 water{0.02f, 0.78f, 0.95f};
            Vec3 shore{0.54f, 0.43f, 0.20f};
            const int seg = 14;
            float stretchX = 1.45f + rand01(xc + 101, zc - 303) * 1.55f;
            float stretchZ = 0.72f + rand01(xc - 707, zc + 111) * 0.55f;
            float yaw = rand01(xc + 41, zc + 73) * 6.2831853f;
            float cy = std::cos(yaw), sy = std::sin(yaw);
            auto ovalPoint = [&](float a, float scale) {
                float lx = std::cos(a) * r * stretchX * scale;
                float lz = std::sin(a) * r * stretchZ * scale;
                float px = c.x + lx * cy + lz * sy;
                float pz = c.z - lx * sy + lz * cy;
                return Vec3{px, renderedSurfaceHeight(px, pz) + 0.06f, pz};
            };
            Vec3 prevInner = ovalPoint(0.0f, 0.42f);
            Vec3 prevOuter = ovalPoint(0.0f, 1.0f);
            for (int i = 1; i <= seg; ++i) {
                float a = i / static_cast<float>(seg) * 6.2831853f;
                Vec3 inner = ovalPoint(a, 0.42f);
                Vec3 mid = ovalPoint(a, 0.82f);
                Vec3 outer = ovalPoint(a, 1.0f);
                pushTri(c, prevInner, inner, water, 1.0f);
                pushTri(prevInner, mid, inner, water, 0.95f);
                pushTri(prevInner, prevOuter, mid, {water.x * 0.80f, water.y * 0.90f, water.z * 0.86f}, 0.62f);
                pushTri(prevOuter, outer, mid, shore, 0.08f);
                pushLine(prevOuter, outer, {0.36f, 0.72f, 0.58f}, 0.18f);
                prevInner = inner;
                prevOuter = outer;
            }
        }
    }
}
static bool shouldRebuildWorld() {
    float dx = g_player.x - g_lastWorldX;
    float dz = g_player.z - g_lastWorldZ;
    return g_worldDirty || g_signalStage != g_lastWorldStage || (dx * dx + dz * dz) > 120.0f * 120.0f;
}
static void buildTower() {
    Vec3 p = encounterSightingPosition();
    float base = renderedSurfaceHeight(p.x, p.z);
    if (p.y > base + 12.0f) {
        pushBuilding({p.x, base, p.z}, {42.0f, (p.y - base) * 0.5f, 32.0f}, {0.58f, 0.55f, 0.48f}, 0.05f);
    }
    Vec3 bodyRed{1.0f, 0.025f, 0.0f};
    Vec3 white{1.0f, 0.98f, 0.86f};
    Vec3 glow{1.0f, 0.12f, 0.02f};
    float distance = std::sqrt((g_player.x - p.x) * (g_player.x - p.x) + (g_player.z - p.z) * (g_player.z - p.z));
    float s = clampf(distance * 0.030f, 48.0f, 135.0f);
    if (g_colossusDefeated) {
        float gy = renderedSurfaceHeight(p.x, p.z) + 2.0f;
        pushCube({p.x, gy + s * 0.22f, p.z}, {s * 1.28f, s * 0.22f, s * 0.30f}, bodyRed, 0.85f, 0.28f);
        pushCube({p.x - s * 0.82f, gy + s * 0.18f, p.z + s * 0.18f}, {s * 0.62f, s * 0.16f, s * 0.18f}, white, 0.32f, -0.42f);
        pushCube({p.x + s * 0.88f, gy + s * 0.16f, p.z - s * 0.14f}, {s * 0.70f, s * 0.15f, s * 0.18f}, white, 0.32f, 0.55f);
        pushCube({p.x - s * 1.25f, gy + s * 0.14f, p.z - s * 0.28f}, {s * 0.20f, s * 0.16f, s * 0.20f}, white, 0.28f, 0.18f);
        pushCube({p.x + s * 1.18f, gy + s * 0.12f, p.z + s * 0.32f}, {s * 0.30f, s * 0.11f, s * 0.18f}, bodyRed, 0.70f, -0.72f);
        for (int i = 0; i < 8; ++i) {
            float a = i / 8.0f * 6.2831853f;
            float r = s * (0.35f + 0.16f * i);
            Vec3 a0{p.x + std::cos(a) * r, gy + s * 0.06f, p.z + std::sin(a) * r};
            Vec3 b0{p.x + std::cos(a + 0.35f) * (r + s * 0.35f), gy + s * 0.05f, p.z + std::sin(a + 0.35f) * (r + s * 0.35f)};
            pushLine(a0, b0, glow, 0.65f);
        }
        return;
    }
    pushCube({p.x, p.y + s * 1.95f, p.z}, {s * 0.22f, s * 1.34f, s * 0.12f}, bodyRed, 1.0f);
    pushCube({p.x, p.y + s * 3.55f, p.z}, {s * 0.20f, s * 0.24f, s * 0.14f}, white, 0.24f);
    pushCube({p.x, p.y + s * 2.05f, p.z + 0.2f}, {s * 0.46f, s * 1.70f, s * 0.030f}, {0.90f, 0.05f, 0.0f}, 0.75f);
    pushLine({p.x - s * 0.18f, p.y + s * 0.72f, p.z}, {p.x - s * 0.58f, p.y, p.z}, white, 0.16f);
    pushLine({p.x + s * 0.18f, p.y + s * 0.72f, p.z}, {p.x + s * 0.58f, p.y, p.z}, white, 0.16f);
    pushLine({p.x - s * 0.22f, p.y + s * 2.52f, p.z}, {p.x - s * 0.92f, p.y + s * 1.76f, p.z}, white, 0.16f);
    pushLine({p.x + s * 0.22f, p.y + s * 2.52f, p.z}, {p.x + s * 0.92f, p.y + s * 1.76f, p.z}, white, 0.16f);
    pushLine({p.x, p.y + s * 3.95f, p.z}, {p.x, p.y + s * 5.15f, p.z}, glow, 0.10f);
    pushLine({p.x - s * 1.2f, p.y + s * 2.65f, p.z}, {p.x + s * 1.2f, p.y + s * 2.65f, p.z}, glow, 0.10f);
    for (int i = 0; i < 6; ++i) {
        float h = s * (3.2f + i * 0.65f);
        float w = s * (0.6f + i * 0.34f);
        pushLine({p.x - w, p.y + h, p.z}, {p.x + w, p.y + h + std::sin(g_time + i) * 0.5f, p.z}, {0.90f, 0.84f, 0.68f}, 0.08f);
    }
    if (g_signalStage >= kFinalSignalStage && !g_colossusDefeated) {
        Vec3 leftEye{}, rightEye{}, chest{};
        float eyeR = 0.0f, chestR = 0.0f;
        colossusTargets(leftEye, rightEye, chest, eyeR, chestR);
        Vec3 targetRed{1.0f, 0.0f, 0.0f};
        Vec3 targetWhite{1.0f, 0.90f, 0.66f};
        if (!g_leftEyeDestroyed) {
            pushCube(leftEye, {eyeR, eyeR * 0.65f, eyeR * 0.28f}, targetRed, 1.0f);
            pushLine({leftEye.x - eyeR * 1.6f, leftEye.y, leftEye.z}, {leftEye.x + eyeR * 1.6f, leftEye.y, leftEye.z}, targetWhite, 1.0f);
        }
        if (!g_rightEyeDestroyed) {
            pushCube(rightEye, {eyeR, eyeR * 0.65f, eyeR * 0.28f}, targetRed, 1.0f);
            pushLine({rightEye.x - eyeR * 1.6f, rightEye.y, rightEye.z}, {rightEye.x + eyeR * 1.6f, rightEye.y, rightEye.z}, targetWhite, 1.0f);
        }
        if (g_leftEyeDestroyed && g_rightEyeDestroyed) {
            float pulse = 0.72f + 0.28f * std::sin(g_time * 10.0f);
            pushCube(chest, {chestR, chestR * 0.78f, chestR * 0.32f}, {1.0f, 0.10f * pulse, 0.0f}, 1.0f);
            pushLine({chest.x - chestR * 1.8f, chest.y, chest.z}, {chest.x + chestR * 1.8f, chest.y, chest.z}, targetWhite, 1.0f);
            pushLine({chest.x, chest.y - chestR * 1.4f, chest.z}, {chest.x, chest.y + chestR * 1.4f, chest.z}, targetWhite, 1.0f);
        }
    }
}
static void triggerAtomicBlast() {
    Vec3 p = sightingPositionForStage(kFinalSignalStage);
    float ground = renderedSurfaceHeight(p.x, p.z);
    g_atomicOrigin = {p.x + 140.0f, ground, p.z + 90.0f};
    g_atomicTime = 0.0f;
    g_atomicEnded = false;
    g_damagePulse = std::max(g_damagePulse, 0.70f);
    g_hitPulse = std::max(g_hitPulse, 0.55f);
}
static void updateAtomicBlast(float dt) {
    bool testKeyDown = keyDown('T');
    if (testKeyDown && !g_atomicTestKeyWasDown) triggerAtomicBlast();
    g_atomicTestKeyWasDown = testKeyDown;
    if (g_atomicTime > -100.0f) {
        g_atomicTime += dt;
        float a = atomicIntensity();
        g_damagePulse = std::max(g_damagePulse, a * 0.72f);
        g_hitPulse = std::max(g_hitPulse, a * 0.55f);
        if (atomicEndFade() >= 0.995f) g_atomicEnded = true;
    }
}
static void buildAtomicBlastFx() {
    g_atomicTriangles.clear();
    if (g_atomicTime < 0.0f || g_atomicTime > kAtomicDropSeconds + 44.0f) return;
    float t = g_atomicTime - kAtomicDropSeconds;
    Vec3 base = g_atomicOrigin;
    float ground = renderedSurfaceHeight(base.x, base.z);
    base.y = ground;
    if (t < 0.0f) {
        float drop = smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime);
        Vec3 bomb{base.x, ground + 1900.0f * (1.0f - drop) + 28.0f, base.z};
        pushAtomicBillboard(bomb, 90.0f, 220.0f, {1.45f, 1.12f, 0.08f}, 1.0f);
        pushFrameLine({bomb.x, bomb.y + 260.0f, bomb.z}, bomb, {1.45f, 0.92f, 0.02f}, 1.0f);
        return;
    }
    float flash = smoothstep(4.6f, 0.0f, t);
    float bloom = smoothstep(0.1f, 3.2f, t) * smoothstep(36.0f, 6.0f, t);
    float rise = smoothstep(0.0f, 18.0f, t);
    float shock = 620.0f + t * 420.0f;
    if (t < 18.0f) {
        pushAtomicRing({base.x, ground + 3.0f, base.z}, shock, {1.55f, 1.06f, 0.04f}, 1.0f);
        pushAtomicRing({base.x, ground + 7.0f, base.z}, shock * 0.58f, {1.35f, 0.72f, 0.02f}, 0.9f, 96);
    }
    float fire = (620.0f + t * 120.0f) * smoothstep(9.0f, 0.2f, t);
    if (fire > 2.0f) {
        pushAtomicBillboard({base.x, ground + fire * 0.34f, base.z}, fire * 4.5f, fire * 1.35f, {1.65f, 1.02f, 0.02f}, 1.0f);
        pushAtomicBillboard({base.x, ground + fire * 0.58f, base.z}, fire * 2.3f, fire * 1.45f, {1.35f, 1.24f, 0.22f}, 1.0f);
    }
    float stemH = (620.0f + rise * 1400.0f);
    float stemW = (190.0f + rise * 420.0f);
    for (int i = 0; i < 16; ++i) {
        float fi = static_cast<float>(i);
        float h = stemH * (0.06f + fi * 0.062f);
        float a = fi * 1.7f + g_time * 0.12f;
        Vec3 p{base.x + std::cos(a) * stemW * (0.08f + fi * 0.020f), ground + h, base.z + std::sin(a) * stemW * 0.14f};
        float s = stemW * (0.50f + fi * 0.035f);
        Vec3 c{0.95f + flash * 0.46f, 0.76f + flash * 0.34f, 0.26f + flash * 0.05f};
        pushAtomicBillboard(p, s * 2.3f, s * 1.7f, c, 0.65f);
    }
    float capY = ground + 1250.0f + rise * 900.0f;
    float capW = 1450.0f + rise * 2400.0f;
    float capH = 440.0f + rise * 500.0f;
    for (int ring = 0; ring < 4; ++ring) {
        int count = 14 + ring * 9;
        float rf = 0.10f + ring * 0.13f;
        for (int i = 0; i < count; ++i) {
            float a = static_cast<float>(i) / static_cast<float>(count) * 6.2831853f + ring * 0.37f + g_time * 0.018f;
            float seed = rand01(5000 + ring * 41 + i, 777);
            Vec3 p{base.x + std::cos(a) * capW * rf * (0.80f + seed * 0.38f),
                   capY + std::sin(a * 2.0f + seed) * capH * 0.12f,
                   base.z + std::sin(a) * capW * rf * 0.42f};
            Vec3 c = ring < 2 ? Vec3{1.05f, 0.84f, 0.28f} : Vec3{0.70f, 0.58f, 0.30f};
            c = {c.x + flash * 0.42f + bloom * 0.25f, c.y + flash * 0.34f + bloom * 0.18f, c.z + flash * 0.04f};
            pushAtomicBillboard(p, capH * (0.74f + seed * 0.34f), capH * (0.50f + seed * 0.24f), c, 0.55f);
        }
    }
    for (int i = 0; i < 90; ++i) {
        float seed = rand01(i + 6100, 33);
        float a = seed * 6.2831853f;
        float r = (420.0f + seed * 2200.0f) * bloom;
        Vec3 p{base.x + std::cos(a) * r, ground + 80.0f + rand01(i, 44) * 1200.0f * bloom, base.z + std::sin(a) * r * 0.55f};
        pushAtomicBillboard(p, 45.0f + seed * 130.0f, 30.0f + seed * 90.0f, {1.22f, 0.82f, 0.08f}, 0.7f);
    }
}
static void buildWorld() {
    g_triangles.clear();
    g_lines.clear();
    g_buildingColliders.clear();
    g_snipers.clear();
    g_triangles.reserve(50000);
    g_lines.reserve(2200);
    buildTerrain();
    buildRoad();
    buildRuinedStructures();
    buildDistantSilhouettes();
    buildBrutalistBlocks();
    buildOases();
    buildTower();
    float testZ = g_player.z - 360.0f;
    float testY = renderedSurfaceHeight(-70.0f, testZ);
    pushBuilding({-70.0f, testY, testZ}, {42.0f, 44.0f, 32.0f}, {0.56f, 0.54f, 0.47f}, 0.06f);
    addSniper(900000 + g_signalStage, {-70.0f, testY + 89.0f, testZ - 8.0f});
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
    float heat = sin(p.z * 0.018 + uTime * 1.7) * sin(p.x * 0.011 - uTime * 0.9);
    p.x += heat * uGlitch * 0.10 * smoothstep(2.0, 90.0, p.y);
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
float fbm(vec2 p) { float v=0.0; float a=0.5; for(int i=0;i<5;i++){ v += noise(p)*a; p=p*2.03+7.1; a*=0.5; } return v; }
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(vec3(-0.58, 0.74, 0.34));
    vec3 V = normalize(uEye - vWorld);
    vec3 H = normalize(L + V);
    float diff = max(dot(N, L), 0.0);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 2.5);
    float dist = length(uEye - vWorld);

    float grain = fbm(vWorld.xz * 0.12 + vec2(vWorld.y * 0.015, 0.0));
    float fine = fbm(vWorld.xz * 0.85 + vec2(vWorld.y * 0.04, 13.0));
    float panelX = 1.0 - smoothstep(0.018, 0.075, abs(fract(vWorld.x * 0.042) - 0.5));
    float panelY = 1.0 - smoothstep(0.018, 0.075, abs(fract(vWorld.y * 0.060) - 0.5));
    float maxc = max(vColor.r, max(vColor.g, vColor.b));
    float minc = min(vColor.r, min(vColor.g, vColor.b));
    float redColossus = smoothstep(0.84, 0.98, vColor.r) * (1.0 - smoothstep(0.08, 0.20, vColor.g + vColor.b));
    float concrete = smoothstep(0.38, 0.66, maxc) * (1.0 - smoothstep(0.16, 0.36, maxc-minc));
    float sand = smoothstep(0.46, 0.82, vColor.r) * smoothstep(0.24, 0.52, vColor.b);

    vec3 material = vColor * (0.82 + grain * 0.30 + fine * 0.08);
    material = mix(material, vec3(0.60, 0.57, 0.49) * (0.75 + grain * 0.34), concrete * 0.75);
    material -= vec3(0.08, 0.075, 0.060) * max(panelX, panelY) * concrete;
    material += vec3(0.16, 0.12, 0.05) * sand * smoothstep(0.58, 1.0, fine) * 0.26;

    float spec = pow(max(dot(N, H), 0.0), 72.0) * vShine;
    vec3 sun = vec3(1.00, 0.80, 0.48);
    vec3 sky = vec3(0.34, 0.55, 0.78);
    vec3 bounce = vec3(0.72, 0.50, 0.25) * max(N.y, 0.0) * 0.18;
    vec3 color = material * (0.26 + diff * 0.92) + sun * spec * 0.55 + sky * rim * 0.12 + bounce;
    color += vec3(3.4, 0.08, 0.0) * redColossus * (0.95 + rim * 0.55);

    float figureAura = exp(-abs(vWorld.z - uTowerZ) * 0.0016) * exp(-abs(vWorld.x - uFigureX) * 0.0024);
    color += vec3(1.0, 0.88, 0.58) * figureAura * (0.035 + uTowerAlign * 0.18);
    color += vec3(1.0, 0.96, 0.78) * uTowerAlign * rim * 0.08;

    vec3 fog = mix(vec3(0.90, 0.70, 0.43), vec3(0.62, 0.75, 0.88), clamp((vWorld.y + 20.0) / 180.0, 0.0, 1.0));
    float lowAir = smoothstep(90.0, 0.0, abs(vWorld.y - uEye.y));
    float haze = smoothstep(180.0, 1180.0, dist) * (0.74 + uGlitch * 0.48);
    haze += smoothstep(360.0, 1700.0, dist) * lowAir * 0.24;
    haze = clamp(haze, 0.0, 0.94);
    haze *= 1.0 - redColossus * 0.82;
    color = mix(color, fog, haze);
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
uniform float uDrinkPulse;
uniform float uThirst;
uniform float uWaterFuel;
uniform float uJetpackPulse;
uniform float uWeaponPulse;
uniform float uHitPulse;
uniform float uDamagePulse;
uniform float uHealth;
uniform float uAtomic;
uniform float uAtomicFade;
uniform vec3 uIncomingDir;
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
void main() {
    vec2 uv = vUv;
    vec2 px = 1.0 / uResolution.xy;
    vec2 center = uv - 0.5;
    float horizon = smoothstep(0.10, 0.76, uv.y) * smoothstep(1.05, 0.18, uv.y);
    float heatLow = smoothstep(0.72, 0.12, uv.y);
    float n1 = noise(vec2(uv.x * 18.0 + uTime * 0.55, uv.y * 5.0 - uTime * 0.15));
    float n2 = noise(vec2(uv.x * 52.0 - uTime * 1.2, uv.y * 13.0 + uTime * 0.28));
    float n3 = noise(vec2(uv.x * 8.0 - uTime * 0.22, uv.y * 2.4 + uTime * 0.08));
    float thirstHeat = smoothstep(0.12, 1.0, uThirst);
    float atomic = clamp(uAtomic, 0.0, 1.0);
    float endFade = clamp(uAtomicFade, 0.0, 1.0);
    float shimmer = (n1 - 0.5) * 0.024 + (n2 - 0.5) * 0.008 + (n3 - 0.5) * 0.018;
    shimmer *= heatLow * (1.70 + uGlitch * 3.6 + uTowerAlign * 1.45 + thirstHeat * 4.5 + atomic * 12.0);
    uv.x += shimmer;
    uv.y += sin(uv.x * 80.0 + uTime * 2.1) * heatLow * (0.0023 + atomic * 0.018);
    uv += center * atomic * (0.030 + 0.026 * sin(uTime * 29.0));

    vec3 color = scene(uv);
    vec3 mirage = scene(uv + vec2(shimmer * 2.2, -0.010 * heatLow));
    color = mix(color, mirage, heatLow * (0.10 + uGlitch * 0.10 + thirstHeat * 0.22));
    float softGlare = smoothstep(0.54, 1.0, luma(color));
    color += vec3(0.30, 0.22, 0.10) * softGlare * (0.14 + thirstHeat * 0.08);

    vec3 sky = mix(vec3(0.96, 0.72, 0.42), vec3(0.42, 0.70, 0.95), smoothstep(0.18, 1.0, uv.y));
    float empty = 1.0 - smoothstep(0.015, 0.075, luma(color));
    color = mix(color, sky, empty * 0.92);
    float sun = pow(max(0.0, 1.0 - length((uv - vec2(0.78, 0.82)) * vec2(1.0, 1.25))), 9.0);
    color += vec3(1.0, 0.76, 0.34) * sun * 1.35;
    color += vec3(0.95, 0.74, 0.45) * horizon * (0.15 + uGlitch * 0.36 + thirstHeat * 0.30);
    float blastCore = pow(max(0.0, 1.0 - length(center * vec2(1.0, 0.78))), 2.0);
    color += vec3(3.0, 2.05, 0.10) * atomic * (0.48 + horizon * 1.45 + blastCore * 0.95);
    color = mix(color, vec3(1.0, 0.92, 0.22), atomic * 0.48);

    float dust = noise(uv * vec2(5.0, 2.8) + vec2(uTime * 0.018, -uTime * 0.012));
    color = mix(color, vec3(0.88, 0.71, 0.48), smoothstep(0.25, 0.92, dust) * 0.18);
    float glare = pow(max(0.0, 1.0 - length(center * vec2(1.2, 0.85))), 2.4);
    color += vec3(0.78, 0.50, 0.22) * glare * 0.12;
    color += vec3(1.0, 0.94, 0.72) * uFalseStart * (0.26 + 0.12 * sin(uTime * 18.0));
    color = mix(color, vec3(0.98, 0.90, 0.70), uSignalReached * 0.18);
    color = mix(color, color + vec3(0.04, 0.20, 0.24), uDrinkPulse * 0.34);
    color += vec3(0.05, 0.34, 0.36) * uDrinkPulse * pow(max(0.0, 1.0 - length(center) * 1.6), 2.0);
    color += vec3(0.06, 0.30, 0.40) * uJetpackPulse * pow(max(0.0, 1.0 - abs(center.y + 0.34) * 2.2), 2.0) * (0.35 + 0.25 * sin(uTime * 24.0));
    color += vec3(0.70, 0.52, 0.24) * uWeaponPulse * pow(max(0.0, 1.0 - length(center * vec2(1.7, 1.0))), 12.0);
    color += vec3(0.15, 0.75, 0.62) * uHitPulse * pow(max(0.0, 1.0 - length(center * vec2(1.2, 1.2))), 6.0);
    color += vec3(0.85, 0.02, 0.0) * uDamagePulse * pow(smoothstep(0.20, 0.78, length(center)), 1.7);
    vec2 incoming = normalize(uIncomingDir.xy + vec2(0.0001));
    float directionalEdge = smoothstep(0.35, 0.96, dot(normalize(center + vec2(0.0001)), incoming));
    directionalEdge *= smoothstep(0.18, 0.72, length(center));
    color += vec3(1.0, 0.04, 0.0) * directionalEdge * uDamagePulse * 0.52;
    float fever = smoothstep(0.55, 1.0, uThirst);
    color = mix(color, color * vec3(1.16, 0.90, 0.70) + vec3(0.10, 0.025, 0.0), fever * 0.34);

    float grain = hash(uv * uResolution.xy + floor(uTime * 24.0)) - 0.5;
    color += grain * (0.010 + atomic * 0.050);
    color = color / (color + vec3(0.88));
    color = pow(color, vec3(0.86, 0.90, 1.0));
    color *= vec3(1.12, 1.02, 0.90);
    color *= 1.0 - smoothstep(0.65, 1.05, length(center));
    vec2 pixel = vUv * uResolution.xy;
    vec2 boxMin = vec2(24.0, uResolution.y - 42.0);
    vec2 boxMax = vec2(184.0, uResolution.y - 24.0);
    vec2 fillMin = boxMin + vec2(3.0);
    vec2 fillMax = vec2(mix(fillMin.x, boxMax.x - 3.0, uThirst), boxMax.y - 3.0);
    float insideOuter = step(boxMin.x, pixel.x) * step(pixel.x, boxMax.x) * step(boxMin.y, pixel.y) * step(pixel.y, boxMax.y);
    float insideInner = step(fillMin.x, pixel.x) * step(pixel.x, boxMax.x - 3.0) * step(fillMin.y, pixel.y) * step(pixel.y, boxMax.y - 3.0);
    float insideFill = step(fillMin.x, pixel.x) * step(pixel.x, fillMax.x) * step(fillMin.y, pixel.y) * step(pixel.y, fillMax.y);
    float frame = insideOuter * (1.0 - insideInner);
    float fill = insideFill;
    vec3 meterColor = mix(vec3(0.10, 0.45, 0.52), vec3(1.0, 0.28, 0.05), thirstHeat);
    color = mix(color, vec3(0.025, 0.020, 0.015), insideInner * 0.68);
    color = mix(color, vec3(0.95, 0.90, 0.76), frame * 0.95);
    color = mix(color, meterColor, fill * 0.96);
    vec2 fuelMin = vec2(24.0, uResolution.y - 68.0);
    vec2 fuelMax = vec2(184.0, uResolution.y - 52.0);
    vec2 fuelFillMin = fuelMin + vec2(3.0);
    vec2 fuelFillMax = vec2(mix(fuelFillMin.x, fuelMax.x - 3.0, uWaterFuel), fuelMax.y - 3.0);
    float fuelOuter = step(fuelMin.x, pixel.x) * step(pixel.x, fuelMax.x) * step(fuelMin.y, pixel.y) * step(pixel.y, fuelMax.y);
    float fuelInner = step(fuelFillMin.x, pixel.x) * step(pixel.x, fuelMax.x - 3.0) * step(fuelFillMin.y, pixel.y) * step(pixel.y, fuelMax.y - 3.0);
    float fuelFill = step(fuelFillMin.x, pixel.x) * step(pixel.x, fuelFillMax.x) * step(fuelFillMin.y, pixel.y) * step(pixel.y, fuelFillMax.y);
    color = mix(color, vec3(0.018, 0.024, 0.026), fuelInner * 0.70);
    color = mix(color, vec3(0.80, 0.92, 0.94), fuelOuter * (1.0 - fuelInner) * 0.95);
    color = mix(color, vec3(0.08, 0.55, 0.72), fuelFill * 0.96);
    vec2 healthMin = vec2(uResolution.x - 184.0, uResolution.y - 42.0);
    vec2 healthMax = vec2(uResolution.x - 24.0, uResolution.y - 24.0);
    vec2 healthFillMin = healthMin + vec2(3.0);
    vec2 healthFillMax = vec2(mix(healthFillMin.x, healthMax.x - 3.0, uHealth), healthMax.y - 3.0);
    float healthOuter = step(healthMin.x, pixel.x) * step(pixel.x, healthMax.x) * step(healthMin.y, pixel.y) * step(pixel.y, healthMax.y);
    float healthInner = step(healthFillMin.x, pixel.x) * step(pixel.x, healthMax.x - 3.0) * step(healthFillMin.y, pixel.y) * step(pixel.y, healthMax.y - 3.0);
    float healthFill = step(healthFillMin.x, pixel.x) * step(pixel.x, healthFillMax.x) * step(healthFillMin.y, pixel.y) * step(pixel.y, healthFillMax.y);
    color = mix(color, vec3(0.028, 0.014, 0.012), healthInner * 0.70);
    color = mix(color, vec3(0.92, 0.82, 0.70), healthOuter * (1.0 - healthInner) * 0.95);
    color = mix(color, vec3(0.82, 0.06, 0.035), healthFill * 0.96);
    float cross = (1.0 - smoothstep(0.45, 1.15, abs(pixel.x - uResolution.x * 0.5))) * smoothstep(5.0, 8.0, abs(pixel.y - uResolution.y * 0.5));
    cross += (1.0 - smoothstep(0.45, 1.15, abs(pixel.y - uResolution.y * 0.5))) * smoothstep(5.0, 8.0, abs(pixel.x - uResolution.x * 0.5));
    color = mix(color, vec3(0.82, 0.78, 0.66), clamp(cross, 0.0, 1.0) * 0.22);
    color = mix(color, vec3(0.0), endFade);
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
    g_postUniforms.weaponPulse = glGetUniformLocation(g_postProgram, "uWeaponPulse");
    g_postUniforms.hitPulse = glGetUniformLocation(g_postProgram, "uHitPulse");
    g_postUniforms.damagePulse = glGetUniformLocation(g_postProgram, "uDamagePulse");
    g_postUniforms.health = glGetUniformLocation(g_postProgram, "uHealth");
    g_postUniforms.atomic = glGetUniformLocation(g_postProgram, "uAtomic");
    g_postUniforms.atomicFade = glGetUniformLocation(g_postProgram, "uAtomicFade");
    g_postUniforms.incomingDir = glGetUniformLocation(g_postProgram, "uIncomingDir");
    g_postUniforms.resolution = glGetUniformLocation(g_postProgram, "uResolution");
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
    cacheUniformLocations();
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
    g_frameLines.reserve(4);
    resizeFramebuffer();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return true;
}

static void render() {
    resizeFramebuffer();
    if (shouldRebuildWorld()) buildWorld();
    float g = glitchAmount();
    float atomic = atomicIntensity();
    float endFade = atomicEndFade();
    float finalShake = atomicFinalConvulsion();
    float cp = std::cos(g_pitch);
    Vec3 dir{std::sin(g_yaw) * cp, std::sin(g_pitch), std::cos(g_yaw) * cp};
    Vec3 towerFocus = currentSightingPosition();
    float towerDot = dot(norm(dir), norm(sub(towerFocus, g_player)));
    float towerAlign = smoothstep(0.985f, 0.999f, towerDot);
    towerAlign *= 0.82f + 0.18f * (0.5f + 0.5f * std::sin(g_time * 3.0f));
    Vec3 right = norm(cross(norm({dir.x, 0.0f, dir.z}), {0, 1, 0}));
    float shake = atomic * (0.55f + 0.75f * std::sin(g_time * 23.0f) * std::sin(g_time * 47.0f));
    shake += finalShake * (1.9f + 1.5f * std::sin(g_time * 37.0f) * std::sin(g_time * 83.0f));
    Vec3 renderEye = add(g_player, add(mul(right, std::sin(g_time * 91.0f) * shake), {0.0f, std::cos(g_time * 77.0f) * shake * 0.55f, 0.0f}));
    Mat4 view = lookAt(renderEye, add(renderEye, dir), {0, 1, 0});
    Mat4 proj = perspective(66.0f + g * 2.5f + atomic * 18.0f + finalShake * 12.0f, static_cast<float>(g_width) / std::max(1, g_height), 0.08f, 6200.0f);
    Mat4 mvp = multiply(proj, view);

    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glViewport(0, 0, g_width, g_height);
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
    g_frameLines.clear();
    buildAtomicBlastFx();

    glBindVertexArray(g_sceneVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_sceneVbo);
    if (g_sceneBuffersDirty) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_triangles.size() * sizeof(Vertex)), g_triangles.data(), GL_DYNAMIC_DRAW);
    }
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_triangles.size()));
    if (!g_atomicTriangles.empty()) {
        glBindVertexArray(g_atomicVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_atomicVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_atomicTriangles.size() * sizeof(Vertex)), g_atomicTriangles.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_atomicTriangles.size()));
    }

    glLineWidth(1.35f + g * 1.2f);
    glBindVertexArray(g_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
    if (g_sceneBuffersDirty) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_lines.size() * sizeof(Vertex)), g_lines.data(), GL_DYNAMIC_DRAW);
        g_sceneBuffersDirty = false;
    }
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_lines.size()));

    if (g_tracerPulse > 0.01f) {
        Vec3 tracerColor{1.0f, 0.78f * g_tracerPulse, 0.24f * g_tracerPulse};
        g_frameLines.push_back({g_tracerStart, {0, 1, 0}, tracerColor, 1.0f});
        g_frameLines.push_back({g_tracerEnd, {0, 1, 0}, tracerColor, 1.0f});
    }
    if (g_enemyTracerPulse > 0.01f) {
        Vec3 enemyColor{1.0f, 0.04f * g_enemyTracerPulse, 0.0f};
        g_frameLines.push_back({g_enemyTracerStart, {0, 1, 0}, enemyColor, 1.0f});
        g_frameLines.push_back({g_enemyTracerEnd, {0, 1, 0}, enemyColor, 1.0f});
    }
    if (!g_frameLines.empty()) {
        glBindVertexArray(g_fxLineVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_fxLineVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_frameLines.size() * sizeof(Vertex)), g_frameLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_frameLines.size()));
    }
    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(g_postProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_colorTex);
    glUniform1i(g_postUniforms.scene, 0);
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
    glUniform1f(g_postUniforms.weaponPulse, g_weaponPulse);
    glUniform1f(g_postUniforms.hitPulse, g_hitPulse);
    glUniform1f(g_postUniforms.damagePulse, g_damagePulse);
    glUniform1f(g_postUniforms.health, g_playerHealth);
    glUniform1f(g_postUniforms.atomic, clampf(atomic + finalShake * 0.85f, 0.0f, 1.0f));
    glUniform1f(g_postUniforms.atomicFade, endFade);
    glUniform3f(g_postUniforms.incomingDir, g_incomingDirX, g_incomingDirY, 0.0f);
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
static bool sniperKilled(int id) {
    return std::find(g_killedSnipers.begin(), g_killedSnipers.end(), id) != g_killedSnipers.end();
}
static void addSniper(int id, Vec3 pos) {
    if (sniperKilled(id)) return;
    g_snipers.push_back({id, pos});
    Vec3 red{1.0f, 0.02f, 0.0f};
    Vec3 glow{1.0f, 0.18f, 0.04f};
    pushCube({pos.x, pos.y + 5.2f, pos.z}, {3.2f, 5.4f, 1.30f}, red, 1.0f);
    pushCube({pos.x, pos.y + 11.6f, pos.z}, {2.10f, 1.55f, 1.05f}, red, 1.0f);
    pushLine({pos.x - 7.0f, pos.y + 7.2f, pos.z}, {pos.x + 7.0f, pos.y + 7.2f, pos.z}, glow, 1.0f);
    pushLine({pos.x, pos.y + 12.8f, pos.z}, {pos.x, pos.y + 28.0f, pos.z}, glow, 1.0f);
    pushLine({pos.x - 5.0f, pos.y + 12.2f, pos.z}, {pos.x + 5.0f, pos.y + 12.2f, pos.z}, glow, 1.0f);
}
static Vec3 lookDirection() {
    float cp = std::cos(g_pitch);
    return {std::sin(g_yaw) * cp, std::sin(g_pitch), std::cos(g_yaw) * cp};
}
static bool rayHitsSphere(Vec3 origin, Vec3 dir, Vec3 center, float radius, float& tHit) {
    Vec3 oc = sub(origin, center);
    float b = dot(oc, dir);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0f) return false;
    float t = -b - std::sqrt(h);
    if (t < 0.0f) t = -b + std::sqrt(h);
    if (t < 0.0f) return false;
    tHit = t;
    return true;
}
static bool fireAtColossus(Vec3 dir) {
    if (g_signalStage < kFinalSignalStage || g_colossusDefeated) return false;
    Vec3 leftEye{}, rightEye{}, chest{};
    float eyeR = 0.0f, chestR = 0.0f;
    colossusTargets(leftEye, rightEye, chest, eyeR, chestR);
    float t = 0.0f;
    if (!g_leftEyeDestroyed && rayHitsSphere(g_player, dir, leftEye, eyeR, t)) {
        g_leftEyeDestroyed = true;
        g_hitPulse = 1.0f;
        g_tracerEnd = leftEye;
        g_worldDirty = true;
        return true;
    }
    if (!g_rightEyeDestroyed && rayHitsSphere(g_player, dir, rightEye, eyeR, t)) {
        g_rightEyeDestroyed = true;
        g_hitPulse = 1.0f;
        g_tracerEnd = rightEye;
        g_worldDirty = true;
        return true;
    }
    if (g_leftEyeDestroyed && g_rightEyeDestroyed && rayHitsSphere(g_player, dir, chest, chestR, t)) {
        ++g_chestHits;
        g_hitPulse = 1.0f;
        g_tracerEnd = chest;
        if (g_chestHits >= kRequiredChestHits) {
            g_colossusDefeated = true;
            g_signalReached = 1.0f;
        }
        g_worldDirty = true;
        return true;
    }
    return false;
}
static void fireWeapon() {
    g_weaponPulse = 1.0f;
    Vec3 dir = norm(lookDirection());
    g_tracerStart = add(g_player, mul(dir, 3.8f));
    g_tracerEnd = add(g_player, mul(dir, 720.0f));
    g_tracerPulse = 1.0f;
    if (fireAtColossus(dir)) return;
    int bestIndex = -1;
    float bestT = 1.0e9f;
    for (int i = 0; i < static_cast<int>(g_snipers.size()); ++i) {
        Vec3 to = sub(g_snipers[i].pos, g_player);
        float t = dot(to, dir);
        if (t < 0.0f || t > 2800.0f) continue;
        Vec3 closest = add(g_player, mul(dir, t));
        float miss = std::sqrt(dot(sub(g_snipers[i].pos, closest), sub(g_snipers[i].pos, closest)));
        float allowance = 4.0f + t * 0.0025f;
        if (miss < allowance && t < bestT) {
            bestT = t;
            bestIndex = i;
        }
    }
    if (bestIndex >= 0) {
        g_tracerEnd = g_snipers[bestIndex].pos;
        g_killedSnipers.push_back(g_snipers[bestIndex].id);
        g_snipers.erase(g_snipers.begin() + bestIndex);
        g_hitPulse = 1.0f;
        g_worldDirty = true;
    }
}
static void updateSnipers(float dt) {
    g_sniperTimer -= dt;
    if (g_sniperTimer > 0.0f || g_snipers.empty()) return;
    g_sniperTimer = 1.6f + rand01(static_cast<int>(g_time * 10.0f), g_signalStage + 91) * 2.1f;
    float best = 1.0e9f;
    int bestIndex = -1;
    for (int i = 0; i < static_cast<int>(g_snipers.size()); ++i) {
        float d = std::sqrt(dot(sub(g_snipers[i].pos, g_player), sub(g_snipers[i].pos, g_player)));
        if (d < best) {
            best = d;
            bestIndex = i;
        }
    }
    if (bestIndex >= 0 && best < 2600.0f) {
        g_enemyTracerStart = add(g_snipers[bestIndex].pos, {0.0f, 4.0f, 0.0f});
        g_enemyTracerEnd = g_player;
        g_enemyTracerPulse = 1.0f;
        Vec3 toSniper = norm(sub(g_snipers[bestIndex].pos, g_player));
        Vec3 forward = norm(lookDirection());
        Vec3 right{std::cos(g_yaw), 0.0f, -std::sin(g_yaw)};
        g_incomingDirX = clampf(dot(toSniper, right), -1.0f, 1.0f);
        g_incomingDirY = clampf(dot(toSniper, forward), -1.0f, 1.0f);
        float hitChance = clampf(0.72f - best * 0.00018f, 0.20f, 0.66f);
        if (rand01(static_cast<int>(g_time * 17.0f), g_snipers[bestIndex].id) < hitChance) {
            g_playerHealth = clampf(g_playerHealth - 0.10f, 0.0f, 1.0f);
            g_damagePulse = 1.0f;
        }
    }
}
static void update(float dt) {
    updateMouse();
    updateAtomicBlast(dt);
    if (g_atomicEnded) {
        updateTelemetry(dt);
        return;
    }
    if (g_fireRequested) {
        fireWeapon();
        g_fireRequested = false;
    }
    updateSnipers(dt);
    g_weaponPulse = clampf(g_weaponPulse - dt * 6.0f, 0.0f, 1.0f);
    g_hitPulse = clampf(g_hitPulse - dt * 3.0f, 0.0f, 1.0f);
    g_damagePulse = clampf(g_damagePulse - dt * 2.8f, 0.0f, 1.0f);
    g_tracerPulse = clampf(g_tracerPulse - dt * 8.0f, 0.0f, 1.0f);
    g_enemyTracerPulse = clampf(g_enemyTracerPulse - dt * 5.5f, 0.0f, 1.0f);
    if (g_playerHealth <= 0.0f) {
        g_playerHealth = 1.0f;
        g_damagePulse = 1.0f;
        g_player = {0.0f, 8.0f, 420.0f};
        g_verticalVelocity = 0.0f;
        g_worldDirty = true;
    }
    bool wantsJetpack = keyDown('Q') && g_waterFuel > 0.015f;
    float speed = keyDown(VK_SHIFT) ? 34.0f : 18.0f;
    if (wantsJetpack) speed *= 2.35f;
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
    float towerDistance = std::sqrt((g_player.x - sighting.x) * (g_player.x - sighting.x) + (g_player.z - sighting.z) * (g_player.z - sighting.z));
    if (towerDistance < kSignalReachDistance) {
        if (g_signalStage < kFinalSignalStage) {
            ++g_signalStage;
            g_falseStartFlash = 1.0f;
            g_signalReached = 0.0f;
            g_wrongWaySignal = 0.0f;
            g_wrongWaySustain = 0.0f;
        } else {
            g_signalReached = g_colossusDefeated ? 1.0f : clampf(g_signalReached + dt * 0.20f, 0.0f, 0.82f);
        }
    } else {
        if (!g_colossusDefeated) g_signalReached = clampf(g_signalReached - dt * 0.10f, 0.0f, 1.0f);
    }
    g_falseStartFlash = clampf(g_falseStartFlash - dt * 0.55f, 0.0f, 1.0f);
    g_thirst = clampf(g_thirst + dt * 0.006f, 0.0f, 1.0f);
    float oasisRadius = 0.0f;
    float oasisDistance = nearestOasisDistance(oasisRadius);
    if (oasisDistance < oasisRadius + 4.0f && keyDown('E')) {
        g_thirst = clampf(g_thirst - dt * 0.70f, 0.0f, 1.0f);
        g_waterFuel = clampf(g_waterFuel + dt * 0.55f, 0.0f, 1.0f);
        g_drinkPulse = clampf(g_drinkPulse + dt * 3.0f, 0.0f, 1.0f);
    } else {
        g_drinkPulse = clampf(g_drinkPulse - dt * 1.8f, 0.0f, 1.0f);
    }
    float bob = std::sin(g_time * 7.0f) * (len > 0.001f ? 0.09f : 0.025f);
    float groundY = playerGroundHeight(g_player.x, g_player.z) + kPlayerEyeHeight;
    bool jetpackActive = wantsJetpack;
    if (g_isGrounded && keyDown(VK_SPACE)) {
        g_verticalVelocity = 13.5f;
        g_isGrounded = false;
    }
    if (jetpackActive) {
        g_isGrounded = false;
        g_verticalVelocity = clampf(g_verticalVelocity + dt * 54.0f, -18.0f, 34.0f);
        g_waterFuel = clampf(g_waterFuel - dt * 0.031f, 0.0f, 1.0f);
        g_jetpackPulse = clampf(g_jetpackPulse + dt * 4.0f, 0.0f, 1.0f);
    } else {
        g_jetpackPulse = clampf(g_jetpackPulse - dt * 2.4f, 0.0f, 1.0f);
    }
    if (!g_isGrounded) {
        g_verticalVelocity -= (jetpackActive ? 18.0f : 32.0f) * dt;
        g_player.y += g_verticalVelocity * dt;
        g_player.y = std::min(g_player.y, groundY + 390.0f);
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
        if (g_mouseCaptured) {
            g_fireRequested = true;
        } else {
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
    wc.lpszClassName = "TheWhiteFigureWindow";
    RegisterClassA(&wc);

    RECT rect{0, 0, g_width, g_height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowA("TheWhiteFigureWindow", "The White Figure - Modern OpenGL 3.3", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                           nullptr, nullptr, instance, nullptr);
    if (!g_hwnd || !createModernContext(g_hwnd) || !initRenderer()) {
        MessageBoxA(nullptr, "Could not initialize a shader-based OpenGL renderer.", "The White Figure", MB_ICONERROR);
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




