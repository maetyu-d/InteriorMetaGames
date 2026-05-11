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
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_STATIC_DRAW 0x88E4
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
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
#undef GLPROC

struct Vec3 { float x, y, z; };
struct IVec3 { int x, y, z; };
struct Vertex { Vec3 p, n, c; float kind; };
struct Mat4 { float m[16]; };

#pragma pack(push, 1)
struct SignalTelemetry {
    uint32_t magic;
    uint32_t version;
    uint32_t byteSize;
    uint32_t sequence;
    float timeSeconds;
    float realDistance;
    float targetDistance;
    float planarDistance;
    float threeDimensionalDistance;
    float playerX, playerY, playerZ;
    float playerVelocityX, playerVelocityY, playerVelocityZ;
    float playerSpeed;
    float playerHorizontalSpeed;
    float playerFacingX, playerFacingY, playerFacingZ;
    float playerMoveDirX, playerMoveDirY, playerMoveDirZ;
    float playerYaw, playerPitch;
    float targetX, targetY, targetZ;
    int32_t targetCellX, targetCellY, targetCellZ;
    int32_t worldSize;
    float signalReached;
};
#pragma pack(pop)

static HWND g_hwnd = nullptr;
static HDC g_hdc = nullptr;
static HGLRC g_glrc = nullptr;
static bool g_running = true;
static bool g_focused = true;
static bool g_mouseCaptured = true;
static bool g_mousePrimed = false;
static int g_mouseIgnoreFrames = 8;
static bool g_mineRequest = false;
static int g_width = 1280, g_height = 720;
static LARGE_INTEGER g_lastCounter{}, g_frequency{};
static float g_time = 0.0f;
static float g_telemetryFileTimer = 0.0f;
static float g_health = 1.0f;
static bool g_reached = false;
static float g_verticalVelocity = 0.0f;
static bool g_grounded = false;
static IVec3 g_mineCell{-1, -1, -1};
static float g_mineProgress = 0.0f;
static float g_mineFlash = 0.0f;
static float g_mineImpact = 0.0f;
static int g_mineChunk = 0;
static IVec3 g_breakCell{-1, -1, -1};
static Vec3 g_breakDir{0.0f, 0.0f, 1.0f};
static float g_breakBurst = 0.0f;
static float g_atomicTime = -1000.0f;
static Vec3 g_atomicOrigin{0.0f, 0.0f, 0.0f};
static Vec3 g_atomicNormal{1.0f, 0.0f, 0.0f};
static bool g_atomicTestKeyWasDown = false;
static bool g_atomicEnded = false;

static GLuint g_program = 0, g_vao = 0, g_vbo = 0;
static GLuint g_atomicVao = 0, g_atomicVbo = 0;
static GLuint g_skyProgram = 0, g_skyVao = 0, g_skyVbo = 0;
static GLuint g_overlayProgram = 0;
static GLuint g_lineVao = 0, g_lineVbo = 0;
static GLint g_uMvp = -1, g_uEye = -1, g_uTime = -1, g_uHealth = -1, g_uGlowPass = -1;
static GLint g_skyTime = -1, g_skyHealth = -1, g_skyResolution = -1, g_skyAtomic = -1;
static GLint g_overlayTime = -1, g_overlayHealth = -1, g_overlayResolution = -1, g_overlayMine = -1, g_overlayAtomic = -1;
static std::vector<Vertex> g_mesh;
static std::vector<Vertex> g_atomicMesh;
static std::vector<Vertex> g_highlight;
static std::vector<Vertex> g_debris;
static bool g_meshDirty = true;
static HANDLE g_telemetryMapping = nullptr;
static SignalTelemetry* g_telemetry = nullptr;
static char g_telemetryPath[MAX_PATH] = {};
static Vec3 g_lastTelemetryPlayer{};
static bool g_hasTelemetryPlayer = false;

static constexpr float kBlockSize = 1.5f;
static constexpr float kEye = 1.55f;
static constexpr float kPlayerRadius = 0.245f;
static constexpr float kGravity = 30.0f;
static constexpr float kMaxFallSpeed = 24.0f;
static constexpr float kDropCommitSpeed = 9.0f;
static constexpr float kAtomicDropSeconds = 2.4f;
static constexpr uint32_t kTelemetryMagic = 0x43575054u; // CWPT
static constexpr float kTelemetryFileInterval = 2.0f;
enum Block : uint8_t { Air = 0, Soft = 1, Rock = 2, Uranium = 3, Target = 4 };
static int g_worldN = 32;
static std::vector<uint8_t> g_blocks;
static std::vector<uint8_t> g_path;
static std::vector<IVec3> g_route;
static IVec3 g_targetCell{16, 0, 16};
static Vec3 g_pos{0.5f, 24.0f + kEye, 0.5f};
static float g_yaw = 0.0f;
static float g_pitch = 0.0f;

static float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }
static float smoothstep(float a, float b, float x) {
    float t = clampf((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
static Vec3 add(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vec3 sub(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3 mul(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
static float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 cross(Vec3 a, Vec3 b) { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
static float length(Vec3 v) { return std::sqrt(std::max(0.0f, dot(v, v))); }
static Vec3 norm(Vec3 v) { float l = std::sqrt(std::max(0.000001f, dot(v, v))); return {v.x/l, v.y/l, v.z/l}; }
static float wrapAngle(float a) {
    const float pi = 3.14159265f;
    while (a > pi) a -= pi * 2.0f;
    while (a < -pi) a += pi * 2.0f;
    return a;
}
static float worldHalf() { return static_cast<float>(g_worldN) * kBlockSize * 0.5f; }
static int idx(int x, int y, int z) { return x + y * g_worldN + z * g_worldN * g_worldN; }
static bool inBounds(int x, int y, int z) { return x >= 0 && y >= 0 && z >= 0 && x < g_worldN && y < g_worldN && z < g_worldN; }
static uint32_t hash3(int x, int y, int z) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + static_cast<uint32_t>(z) * 2246822519u;
    h = (h ^ (h >> 13u)) * 1274126177u;
    return h ^ (h >> 16u);
}
static float rand01(int x, int y, int z) { return static_cast<float>(hash3(x, y, z) & 0x00ffffffu) / static_cast<float>(0x01000000u); }
static bool sameCell(IVec3 a, IVec3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
static Vec3 cellCenter(int x, int y, int z) {
    float half = worldHalf();
    return {
        (static_cast<float>(x) + 0.5f) * kBlockSize - half,
        (static_cast<float>(y) + 0.5f) * kBlockSize - half,
        (static_cast<float>(z) + 0.5f) * kBlockSize - half
    };
}
static IVec3 worldToCell(Vec3 p) {
    float half = worldHalf();
    return {
        static_cast<int>(std::floor((p.x + half) / kBlockSize)),
        static_cast<int>(std::floor((p.y + half) / kBlockSize)),
        static_cast<int>(std::floor((p.z + half) / kBlockSize))
    };
}
static bool solid(uint8_t b) { return b == Soft || b == Rock || b == Uranium; }
static float atomicIntensity() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    if (t < 0.0f) return smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime) * 0.34f;
    float flash = smoothstep(6.0f, 0.0f, t);
    float aftermath = smoothstep(0.2f, 4.0f, t) * smoothstep(40.0f, 7.0f, t) * 0.92f;
    return clampf(flash + aftermath, 0.0f, 1.0f);
}
static float atomicEndFade() {
    if (g_atomicTime < 0.0f) return 0.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    return t <= 10.0f ? 0.0f : smoothstep(10.0f, 16.0f, t);
}

static Mat4 identity() { Mat4 r{}; r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.0f; return r; }
static Mat4 multiply(Mat4 a, Mat4 b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c) for (int row = 0; row < 4; ++row)
        r.m[c*4+row] = a.m[row]*b.m[c*4] + a.m[4+row]*b.m[c*4+1] + a.m[8+row]*b.m[c*4+2] + a.m[12+row]*b.m[c*4+3];
    return r;
}
static Mat4 perspective(float fovy, float aspect, float zn, float zf) {
    float f = 1.0f / std::tan(fovy * 0.5f * 3.14159265f / 180.0f);
    Mat4 r{}; r.m[0] = f / aspect; r.m[5] = f; r.m[10] = (zf + zn) / (zn - zf); r.m[11] = -1.0f; r.m[14] = 2.0f * zf * zn / (zn - zf); return r;
}
static Mat4 lookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 f = norm(sub(target, eye)), s = norm(cross(f, up)), u = cross(s, f);
    Mat4 r = identity();
    r.m[0]=s.x; r.m[4]=s.y; r.m[8]=s.z;
    r.m[1]=u.x; r.m[5]=u.y; r.m[9]=u.z;
    r.m[2]=-f.x; r.m[6]=-f.y; r.m[10]=-f.z;
    r.m[12] = -dot(s, eye); r.m[13] = -dot(u, eye); r.m[14] = dot(f, eye);
    return r;
}
static Vec3 forward() {
    float cp = std::cos(g_pitch);
    return norm({std::sin(g_yaw) * cp, std::sin(g_pitch), std::cos(g_yaw) * cp});
}
static bool footBlocked(Vec3 p);
static bool keyDown(int vk);

static void initTelemetry() {
    g_telemetryMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                            sizeof(SignalTelemetry), "Local\\CubeWorldPathTelemetry");
    if (g_telemetryMapping) {
        g_telemetry = static_cast<SignalTelemetry*>(
            MapViewOfFile(g_telemetryMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SignalTelemetry)));
    }
    if (g_telemetry) {
        std::memset(g_telemetry, 0, sizeof(SignalTelemetry));
        g_telemetry->magic = kTelemetryMagic;
        g_telemetry->version = 1;
        g_telemetry->byteSize = sizeof(SignalTelemetry);
    }
    g_lastTelemetryPlayer = g_pos;
    g_hasTelemetryPlayer = true;

    char exePath[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        char* slash = std::strrchr(exePath, '/');
        char* backslash = std::strrchr(exePath, '\\');
        if (backslash && (!slash || backslash > slash)) slash = backslash;
        if (slash) *(slash + 1) = '\0';
        std::snprintf(g_telemetryPath, sizeof(g_telemetryPath), "%sCubeWorldPathTelemetry.json", exePath);
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
                 "  \"magic\": \"CWPT\",\n"
                 "  \"version\": %u,\n"
                 "  \"sequence\": %u,\n"
                 "  \"timeSeconds\": %.3f,\n"
                 "  \"realDistance\": %.3f,\n"
                 "  \"targetDistance\": %.3f,\n"
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
                 "  \"target\": {\"x\": %.3f, \"y\": %.3f, \"z\": %.3f},\n"
                 "  \"targetCell\": {\"x\": %d, \"y\": %d, \"z\": %d},\n"
                 "  \"worldSize\": %d,\n"
                 "  \"signalReached\": %.3f\n"
                 "}\n",
                 t.version, t.sequence, t.timeSeconds, t.realDistance, t.targetDistance, t.planarDistance,
                 t.threeDimensionalDistance, t.playerX, t.playerY, t.playerZ, t.playerVelocityX,
                 t.playerVelocityY, t.playerVelocityZ, t.playerSpeed, t.playerHorizontalSpeed,
                 t.playerFacingX, t.playerFacingY, t.playerFacingZ, t.playerMoveDirX, t.playerMoveDirY,
                 t.playerMoveDirZ, t.playerYaw, t.playerPitch, t.targetX, t.targetY, t.targetZ,
                 t.targetCellX, t.targetCellY, t.targetCellZ, t.worldSize, t.signalReached);
    std::fclose(f);
    MoveFileExA(tmpPath, g_telemetryPath, MOVEFILE_REPLACE_EXISTING);
}

static void updateTelemetry(float dt) {
    Vec3 target = cellCenter(g_targetCell.x, g_targetCell.y, g_targetCell.z);
    float dx = g_pos.x - target.x;
    float dy = g_pos.y - target.y;
    float dz = g_pos.z - target.z;
    float invDt = dt > 0.0001f && g_hasTelemetryPlayer ? 1.0f / dt : 0.0f;
    Vec3 velocity{
        (g_pos.x - g_lastTelemetryPlayer.x) * invDt,
        (g_pos.y - g_lastTelemetryPlayer.y) * invDt,
        (g_pos.z - g_lastTelemetryPlayer.z) * invDt
    };
    Vec3 facing = forward();
    float horizontalSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    Vec3 moveDir{0.0f, 0.0f, 0.0f};
    if (horizontalSpeed > 0.001f) moveDir = {velocity.x / horizontalSpeed, 0.0f, velocity.z / horizontalSpeed};

    SignalTelemetry t{};
    t.magic = kTelemetryMagic;
    t.version = 1;
    t.byteSize = sizeof(SignalTelemetry);
    t.sequence = g_telemetry ? g_telemetry->sequence + 1u : 1u;
    t.timeSeconds = g_time;
    t.planarDistance = std::sqrt(dx * dx + dz * dz);
    t.threeDimensionalDistance = std::sqrt(dx * dx + dy * dy + dz * dz);
    t.realDistance = t.threeDimensionalDistance;
    t.targetDistance = t.threeDimensionalDistance;
    t.playerX = g_pos.x;
    t.playerY = g_pos.y;
    t.playerZ = g_pos.z;
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
    t.targetX = target.x;
    t.targetY = target.y;
    t.targetZ = target.z;
    t.targetCellX = g_targetCell.x;
    t.targetCellY = g_targetCell.y;
    t.targetCellZ = g_targetCell.z;
    t.worldSize = g_worldN;
    t.signalReached = g_reached ? 1.0f : 0.0f;
    if (g_telemetry) *g_telemetry = t;
    g_lastTelemetryPlayer = g_pos;
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

static void generateWorld() {
    g_worldN = 32 + static_cast<int>(timeGetTime() % 33u);
    g_blocks.assign(static_cast<size_t>(g_worldN) * g_worldN * g_worldN, Rock);
    g_path.assign(static_cast<size_t>(g_worldN) * g_worldN * g_worldN, 0);
    g_route.clear();
    int x = g_worldN / 2, z = g_worldN / 2;
    auto markPathCell = [&](int px, int py, int pz) {
        if (!inBounds(px, py, pz) || !inBounds(px, py + 1, pz)) return;
        g_path[idx(px, py, pz)] = 1;
        g_path[idx(px, py + 1, pz)] = 1;
        g_blocks[idx(px, py, pz)] = Soft;
        g_blocks[idx(px, py + 1, pz)] = Soft;
    };
    auto carvePath = [&](int px, int py, int pz, int radius) {
        for (int oz = -radius; oz <= radius; ++oz) {
            for (int ox = -radius; ox <= radius; ++ox) {
                if (ox * ox + oz * oz > radius * radius + radius) continue;
                markPathCell(px + ox, py, pz + oz);
            }
        }
    };
    for (int y = g_worldN - 2; y >= 0; --y) {
        if (y < g_worldN - 3) {
            float r = rand01(x + y * 3, z - y * 5, y);
            if (r < 0.30f) x += (rand01(y, x, z) < 0.5f ? -1 : 1);
            else if (r < 0.58f) z += (rand01(z, y, x) < 0.5f ? -1 : 1);
            x = std::max(3, std::min(g_worldN - 4, x));
            z = std::max(3, std::min(g_worldN - 4, z));
        }
        IVec3 p{x, y, z};
        g_route.push_back(p);
        int mainRadius = (rand01(x + y * 11, y, z + 7) > 0.72f) ? 1 : 0;
        if (g_worldN > 48 && rand01(x, y * 3, z + 31) > 0.88f) mainRadius = 2;
        carvePath(x, y, z, mainRadius);
        if (y < g_worldN - 2) {
            IVec3 prev = g_route[g_route.size() - 2];
            while (prev.x != x) {
                prev.x += prev.x < x ? 1 : -1;
                int r = (rand01(prev.x, prev.y * 5, prev.z) > 0.68f) ? 1 : 0;
                carvePath(prev.x, prev.y, prev.z, r);
                g_route.push_back(prev);
            }
            while (prev.z != z) {
                prev.z += prev.z < z ? 1 : -1;
                int r = (rand01(prev.x + 19, prev.y, prev.z * 5) > 0.68f) ? 1 : 0;
                carvePath(prev.x, prev.y, prev.z, r);
                g_route.push_back(prev);
            }
        }
    }
    g_targetCell = g_route.back();
    g_blocks[idx(g_targetCell.x, g_targetCell.y, g_targetCell.z)] = Target;
    g_blocks[idx(g_targetCell.x, g_targetCell.y + 1, g_targetCell.z)] = Target;
    auto markFalseCell = [&](int px, int py, int pz) {
        if (!inBounds(px, py, pz) || !inBounds(px, py + 1, pz)) return false;
        if (g_path[idx(px, py, pz)] == 1 || g_path[idx(px, py + 1, pz)] == 1) return false;
        g_path[idx(px, py, pz)] = 2;
        g_path[idx(px, py + 1, pz)] = 2;
        g_blocks[idx(px, py, pz)] = Soft;
        g_blocks[idx(px, py + 1, pz)] = Soft;
        return true;
    };
    auto carveFalse = [&](int px, int py, int pz, int radius) {
        bool any = false;
        for (int oz = -radius; oz <= radius; ++oz) {
            for (int ox = -radius; ox <= radius; ++ox) {
                if (ox * ox + oz * oz > radius * radius + radius) continue;
                any = markFalseCell(px + ox, py, pz + oz) || any;
            }
        }
        return any;
    };
    IVec3 surfaceStart = g_route.front();
    int surfaceFalseCount = 3 + (g_worldN > 44 ? 1 : 0);
    int surfaceDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    int surfaceDirOffset = static_cast<int>(rand01(g_worldN, 17, 13) * 4.0f) & 3;
    for (int s = 0; s < surfaceFalseCount; ++s) {
        int dirIndex = (surfaceDirOffset + s) & 3;
        int dx = surfaceDirs[dirIndex][0], dz = surfaceDirs[dirIndex][1];
        int bendDx = surfaceDirs[(dirIndex + 1 + (s & 1) * 2) & 3][0];
        int bendDz = surfaceDirs[(dirIndex + 1 + (s & 1) * 2) & 3][1];
        int len = 4 + static_cast<int>(rand01(s, g_worldN, 91) * (4.0f + static_cast<float>(g_worldN - 32) * 4.0f / 32.0f));
        IVec3 end = surfaceStart;
        for (int step = 1; step <= len; ++step) {
            IVec3 next = end;
            if (step < len / 2) { next.x += dx; next.z += dz; }
            else { next.x += (step & 1) ? dx : bendDx; next.z += (step & 1) ? dz : bendDz; }
            int r = (step > 1 && rand01(s * 17, step, g_worldN) > 0.52f) ? 1 : 0;
            if (g_worldN > 50 && step > 2 && rand01(s, step * 9, g_worldN) > 0.90f) r = 2;
            if (!carveFalse(next.x, surfaceStart.y, next.z, r)) break;
            end = next;
        }
        int capRadius = 1 + (g_worldN > 52 && (s & 1) ? 1 : 0);
        int capX = end.x + ((s & 1) ? bendDx : dx);
        int capZ = end.z + ((s & 1) ? bendDz : dz);
        uint8_t cap = (s % 2 == 0) ? Rock : Uranium;
        for (int oz = -capRadius; oz <= capRadius; ++oz) for (int ox = -capRadius; ox <= capRadius; ++ox) {
            if (ox * ox + oz * oz > capRadius * capRadius + capRadius) continue;
            int cx = capX + ox, cz = capZ + oz;
            if (inBounds(cx, surfaceStart.y, cz) && g_path[idx(cx, surfaceStart.y, cz)] != 1) {
                g_blocks[idx(cx, surfaceStart.y, cz)] = cap;
                if (inBounds(cx, surfaceStart.y + 1, cz)) g_blocks[idx(cx, surfaceStart.y + 1, cz)] = cap;
            }
        }
    }
    int falsePathCount = 8 + (g_worldN - 32) * 10 / 32;
    for (int f = 0; f < falsePathCount && !g_route.empty(); ++f) {
        int anchorIndex = 4 + static_cast<int>(rand01(f, 33, g_worldN) * static_cast<float>(std::max(1, static_cast<int>(g_route.size()) - 8)));
        anchorIndex = std::min(anchorIndex, static_cast<int>(g_route.size()) - 1);
        IVec3 p = g_route[anchorIndex];
        int dx = rand01(f, 12, 3) < 0.5f ? -1 : 1;
        int dz = rand01(f, 14, 9) < 0.5f ? -1 : 1;
        bool xFirst = rand01(f, 20, 5) < 0.5f;
        int lenA = 3 + static_cast<int>(rand01(f, 44, 2) * (g_worldN == 32 ? 4.0f : 8.0f));
        int lenB = 1 + static_cast<int>(rand01(f, 48, 7) * 4.0f);
        IVec3 end = p;
        for (int step = 0; step < lenA + lenB; ++step) {
            IVec3 next = end;
            if ((step < lenA) == xFirst) next.x += dx; else next.z += dz;
            if (!inBounds(next.x, next.y, next.z) || !inBounds(next.x, next.y + 1, next.z)) break;
            if (g_path[idx(next.x, next.y, next.z)] == 1 && step > 0) break;
            end = next;
            int r = (step > 1 && rand01(f, step * 23, end.y) > 0.58f) ? 1 : 0;
            if (g_worldN > 54 && step > 3 && rand01(f + 7, step, end.x + end.z) > 0.92f) r = 2;
            carveFalse(end.x, end.y, end.z, r);
        }
        int capX = end.x + (xFirst ? 0 : dx);
        int capZ = end.z + (xFirst ? dz : 0);
        int capRadius = (rand01(f, end.y, 4) > 0.60f) ? 1 : 0;
        uint8_t cap = (f % 3 == 0) ? Uranium : Rock;
        for (int oz = -capRadius; oz <= capRadius; ++oz) for (int ox = -capRadius; ox <= capRadius; ++ox) {
            if (ox * ox + oz * oz > capRadius * capRadius + capRadius) continue;
            int cx = capX + ox, cz = capZ + oz;
            if (inBounds(cx, end.y, cz) && g_path[idx(cx, end.y, cz)] != 1) {
                g_blocks[idx(cx, end.y, cz)] = cap;
                if (inBounds(cx, end.y + 1, cz)) g_blocks[idx(cx, end.y + 1, cz)] = cap;
            }
        }
    }
    int veinCount = 28 + (g_worldN - 32) * 44 / 32;
    for (int v = 0; v < veinCount; ++v) {
        int vx = 2 + static_cast<int>(rand01(v, 1, 3) * (g_worldN - 4));
        int vy = 2 + static_cast<int>(rand01(v, 9, 5) * (g_worldN - 4));
        int vz = 1 + static_cast<int>(rand01(v, 7, 8) * (g_worldN - 2));
        int axis = static_cast<int>(rand01(v, 4, 11) * 3.0f);
        int lenv = 3 + static_cast<int>(rand01(v, 12, 2) * (6.0f + static_cast<float>(g_worldN - 32) * 4.0f / 32.0f));
        for (int i = 0; i < lenv; ++i) {
            int px = vx + (axis == 0 ? i : 0), py = vy + (axis == 1 ? i : 0), pz = vz + (axis == 2 ? i : 0);
            if (inBounds(px, py, pz) && !g_path[idx(px, py, pz)]) g_blocks[idx(px, py, pz)] = Uranium;
        }
    }
    IVec3 start = g_route.front();
    g_pos = {cellCenter(start.x, start.y, start.z).x, worldHalf() + kEye, cellCenter(start.x, start.y, start.z).z};
    g_yaw = 3.14159265f;
    g_pitch = -0.55f;
    g_verticalVelocity = 0.0f;
    g_grounded = true;
    g_atomicTime = -1000.0f;
    g_atomicTestKeyWasDown = false;
    g_atomicEnded = false;
}

static bool rayCell(IVec3& out, uint8_t& block);
static void addFace(Vec3 p, int face, Vec3 color, float kind) {
    float h = kBlockSize * 0.5f;
    static const Vec3 n[6] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    Vec3 q[6][4] = {
        {{h,-h,-h},{h,-h,h},{h,h,h},{h,h,-h}},
        {{-h,-h,h},{-h,-h,-h},{-h,h,-h},{-h,h,h}},
        {{-h,h,-h},{h,h,-h},{h,h,h},{-h,h,h}},
        {{-h,-h,h},{h,-h,h},{h,-h,-h},{-h,-h,-h}},
        {{h,-h,h},{-h,-h,h},{-h,h,h},{h,h,h}},
        {{-h,-h,-h},{h,-h,-h},{h,h,-h},{-h,h,-h}}
    };
    int order[6] = {0,2,1,0,3,2};
    for (int i = 0; i < 6; ++i) {
        Vec3 v = add(p, q[face][order[i]]);
        g_mesh.push_back({v, n[face], color, kind + static_cast<float>(face) * 0.03125f});
    }
}
static void addAtomicFace(Vec3 p, Vec3 u, Vec3 v, Vec3 n, Vec3 color, float kind) {
    g_atomicMesh.push_back({p, n, color, kind});
    g_atomicMesh.push_back({add(add(p, u), v), n, color, kind});
    g_atomicMesh.push_back({add(p, u), n, color, kind});
    g_atomicMesh.push_back({p, n, color, kind});
    g_atomicMesh.push_back({add(p, v), n, color, kind});
    g_atomicMesh.push_back({add(add(p, u), v), n, color, kind});
}
static void addAtomicCube(Vec3 c, Vec3 u, Vec3 v, Vec3 w, float half, Vec3 color, float kind) {
    u = mul(norm(u), half);
    v = mul(norm(v), half);
    w = mul(norm(w), half);
    Vec3 p000 = sub(sub(sub(c, u), v), w);
    Vec3 p001 = add(sub(sub(c, u), v), w);
    Vec3 p010 = sub(add(sub(c, u), v), w);
    Vec3 p011 = add(add(sub(c, u), v), w);
    Vec3 p100 = sub(sub(add(c, u), v), w);
    Vec3 p101 = add(sub(add(c, u), v), w);
    Vec3 p110 = sub(add(add(c, u), v), w);
    Vec3 p111 = add(add(add(c, u), v), w);
    addAtomicFace(p100, sub(p101, p100), sub(p110, p100), norm(u), color, kind + 0.00000f);
    addAtomicFace(p001, sub(p000, p001), sub(p011, p001), mul(norm(u), -1.0f), color, kind + 0.03125f);
    addAtomicFace(p010, sub(p110, p010), sub(p011, p010), norm(v), color, kind + 0.06250f);
    addAtomicFace(p001, sub(p101, p001), sub(p000, p001), mul(norm(v), -1.0f), color, kind + 0.09375f);
    addAtomicFace(p101, sub(p001, p101), sub(p111, p101), norm(w), color, kind + 0.12500f);
    addAtomicFace(p000, sub(p100, p000), sub(p010, p000), mul(norm(w), -1.0f), color, kind + 0.15625f);
}
static void addHighlightLine(Vec3 a, Vec3 b, Vec3 color, float kind) {
    g_highlight.push_back({a, {0, 1, 0}, color, kind});
    g_highlight.push_back({b, {0, 1, 0}, color, kind});
}
static void addDebrisLine(Vec3 a, Vec3 b, Vec3 color, float kind) {
    g_debris.push_back({a, {0, 1, 0}, color, kind});
    g_debris.push_back({b, {0, 1, 0}, color, kind});
}
static void addDebrisBox(Vec3 c, Vec3 u, Vec3 v, Vec3 w, float half, Vec3 color, float kind) {
    Vec3 p[8] = {
        sub(sub(sub(c, mul(u, half)), mul(v, half)), mul(w, half)),
        add(sub(sub(c, mul(v, half)), mul(w, half)), mul(u, half)),
        add(add(sub(c, mul(w, half)), mul(u, half)), mul(v, half)),
        add(sub(add(c, mul(v, half)), mul(w, half)), mul(u, -half)),
        add(sub(sub(c, mul(u, half)), mul(v, half)), mul(w, half)),
        add(add(sub(c, mul(v, half)), mul(u, half)), mul(w, half)),
        add(add(add(c, mul(u, half)), mul(v, half)), mul(w, half)),
        add(add(sub(c, mul(u, half)), mul(v, half)), mul(w, half))
    };
    int e[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    for (auto& edge : e) addDebrisLine(p[edge[0]], p[edge[1]], color, kind);
}
static Vec3 facePoint(Vec3 p, int axis, float sign, float u, float v, float s) {
    if (axis == 0) return {p.x + sign * s, p.y + u * s, p.z + v * s};
    if (axis == 1) return {p.x + u * s, p.y + sign * s, p.z + v * s};
    return {p.x + u * s, p.y + v * s, p.z + sign * s};
}
static void triggerAtomicBlast() {
    float half = worldHalf();
    IVec3 start = g_route.empty() ? IVec3{g_worldN / 2, g_worldN - 2, g_worldN / 2} : g_route.front();
    Vec3 startWorld = cellCenter(start.x, start.y, start.z);
    int choice = static_cast<int>(rand01(static_cast<int>(g_time * 10.0f) + g_worldN, start.x, start.z) * 4.0f) & 3;
    float insetA = (rand01(choice + start.x, g_worldN, 41) - 0.5f) * half * 0.70f;
    float y = clampf(startWorld.y - half * (0.18f + rand01(choice, start.y, 52) * 0.46f), -half * 0.72f, half * 0.52f);
    if (choice == 0) {
        g_atomicNormal = {1, 0, 0};
        g_atomicOrigin = {half + 0.10f, y, clampf(startWorld.z + insetA, -half * 0.70f, half * 0.70f)};
    } else if (choice == 1) {
        g_atomicNormal = {-1, 0, 0};
        g_atomicOrigin = {-half - 0.10f, y, clampf(startWorld.z + insetA, -half * 0.70f, half * 0.70f)};
    } else if (choice == 2) {
        g_atomicNormal = {0, 0, 1};
        g_atomicOrigin = {clampf(startWorld.x + insetA, -half * 0.70f, half * 0.70f), y, half + 0.10f};
    } else {
        g_atomicNormal = {0, 0, -1};
        g_atomicOrigin = {clampf(startWorld.x + insetA, -half * 0.70f, half * 0.70f), y, -half - 0.10f};
    }
    g_atomicTime = 0.0f;
    g_mineFlash = std::max(g_mineFlash, 0.55f);
    g_mineImpact = std::max(g_mineImpact, 0.50f);
}
static void updateAtomicBlast(float dt) {
    bool testKeyDown = keyDown('T');
    if (testKeyDown && !g_atomicTestKeyWasDown) triggerAtomicBlast();
    g_atomicTestKeyWasDown = testKeyDown;
    if (g_atomicTime > -100.0f) {
        g_atomicTime += dt;
        float a = atomicIntensity();
        if (a > 0.02f) {
            g_mineFlash = std::max(g_mineFlash, a * 0.82f);
            g_mineImpact = std::max(g_mineImpact, a * 0.75f);
        }
    }
}
static void addAtomicRing(Vec3 center, Vec3 u, Vec3 v, float radius, Vec3 color, float kind, int steps = 64) {
    Vec3 prev = add(center, mul(u, radius));
    for (int i = 1; i <= steps; ++i) {
        float a = static_cast<float>(i) / static_cast<float>(steps) * 6.2831853f;
        Vec3 cur = add(center, add(mul(u, std::cos(a) * radius), mul(v, std::sin(a) * radius)));
        addDebrisLine(prev, cur, color, kind);
        prev = cur;
    }
}
static void buildAtomicBlastSolids() {
    g_atomicMesh.clear();
    if (g_atomicTime < 0.0f || g_atomicTime > kAtomicDropSeconds + 44.0f) return;
    Vec3 n = norm(g_atomicNormal);
    Vec3 u = std::fabs(n.y) < 0.85f ? norm(cross({0,1,0}, n)) : Vec3{1,0,0};
    Vec3 v = norm(cross(n, u));
    Vec3 impact = g_atomicOrigin;
    float t = g_atomicTime - kAtomicDropSeconds;
    if (t < 0.0f) {
        float drop = smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime);
        Vec3 start = add(add(impact, mul(n, 58.0f)), mul(v, 32.0f));
        Vec3 pos = add(mul(start, 1.0f - drop), mul(add(impact, mul(n, 1.5f)), drop));
        addAtomicCube(pos, u, v, n, 2.1f, {1.0f, 0.18f, 0.012f}, 4.0f);
        addAtomicCube(add(pos, mul(n, -3.0f)), u, v, n, 1.35f, {0.24f, 0.025f, 0.010f}, 1.0f);
        return;
    }
    float blast = smoothstep(0.0f, 2.2f, t) * smoothstep(20.0f, 4.0f, t);
    float flash = smoothstep(3.2f, 0.0f, t);
    float bloom = smoothstep(0.2f, 5.0f, t) * smoothstep(19.0f, 6.0f, t);
    float fireR = 14.0f + t * 8.8f;
    for (int i = 0; i < 132; ++i) {
        float seed = rand01(i + 4100, 3, 9);
        float a = seed * 6.2831853f;
        float r = fireR * (0.15f + rand01(i, 4, 8) * 0.88f);
        float out = (3.5f + rand01(i, 7, 12) * 28.0f) * (0.70f + blast * 1.75f);
        Vec3 p = add(impact, add(mul(n, out), add(mul(u, std::cos(a) * r), mul(v, std::sin(a) * r * 0.78f))));
        float s = (2.4f + seed * 7.8f) * (1.0f + flash * 1.45f);
        Vec3 c = seed > 0.55f ? Vec3{1.0f, 0.26f + flash * 0.45f, 0.014f} : Vec3{0.55f, 0.055f, 0.010f};
        addAtomicCube(p, u, v, n, s, c, seed > 0.55f ? 4.0f : 1.0f);
    }
    float cloud = smoothstep(0.8f, 10.0f, t);
    for (int layer = 0; layer < 8; ++layer) {
        int count = 22 + layer * 10;
        float layerOut = (14.0f + layer * 9.0f + cloud * 86.0f);
        float layerR = (14.0f + layer * 12.0f + cloud * 66.0f);
        for (int i = 0; i < count; ++i) {
            float seed = rand01(4300 + layer * 97 + i, 6, 2);
            float a = static_cast<float>(i) / static_cast<float>(count) * 6.2831853f + seed * 0.65f + g_time * 0.025f;
            Vec3 p = add(impact, add(mul(n, layerOut + seed * 8.0f),
                         add(mul(u, std::cos(a) * layerR * (0.55f + seed * 0.55f)),
                             mul(v, std::sin(a) * layerR * 0.72f + (static_cast<float>(layer) - 1.2f) * 4.5f))));
            float s = 3.3f + seed * 10.0f + cloud * (3.2f + layer * 1.6f);
            Vec3 ash = layer < 2 ? Vec3{0.46f, 0.10f + bloom * 0.08f, 0.030f} : Vec3{0.18f, 0.055f, 0.030f};
            ash = {ash.x + flash * 0.30f, ash.y + flash * 0.14f, ash.z + flash * 0.035f};
            addAtomicCube(p, u, v, n, s, ash, layer < 2 ? 1.0f : 2.0f);
        }
    }
    for (int i = 0; i < 76; ++i) {
        float seed = rand01(4700 + i, 14, 5);
        float a = seed * 6.2831853f;
        Vec3 p = add(impact, add(mul(n, 4.0f + seed * 12.0f), add(mul(u, std::cos(a) * (10.0f + seed * 22.0f)), mul(v, std::sin(a) * (8.0f + seed * 18.0f)))));
        addAtomicCube(p, u, v, n, 0.9f + seed * 1.5f, {0.04f, 0.90f, 0.035f}, 3.0f);
    }
}
static void buildAtomicBlastLines() {
    if (g_atomicTime < 0.0f || g_atomicTime > kAtomicDropSeconds + 44.0f) return;
    Vec3 n = norm(g_atomicNormal);
    Vec3 u = std::fabs(n.y) < 0.85f ? norm(cross({0,1,0}, n)) : Vec3{1,0,0};
    Vec3 v = norm(cross(n, u));
    Vec3 impact = g_atomicOrigin;
    float y = 1.0f;
    float t = g_atomicTime - kAtomicDropSeconds;
    if (t < 0.0f) {
        float drop = smoothstep(0.0f, kAtomicDropSeconds, g_atomicTime);
        Vec3 start = add(add(impact, mul(n, 58.0f)), mul(v, 32.0f));
        Vec3 pos = add(mul(start, 1.0f - drop), mul(add(impact, mul(n, 1.5f)), drop));
        float pulse = 0.65f + 0.35f * std::sin(g_time * 22.0f);
        addDebrisBox(pos, u, v, n, 1.35f, {1.0f, 0.26f + pulse * 0.20f, 0.02f}, 1.0f);
        addDebrisLine(start, pos, {0.72f, 0.12f, 0.02f}, 1.0f);
        addDebrisLine(add(pos, mul(u, -2.5f)), add(pos, mul(u, 2.5f)), {1.0f, 0.70f, 0.08f}, 1.0f);
        return;
    }
    float blast = smoothstep(0.0f, 2.4f, t) * smoothstep(19.5f, 4.2f, t);
    float flash = smoothstep(3.0f, 0.0f, t);
    float radius = (22.0f + t * 23.0f) * (1.0f + blast * 1.55f);
    Vec3 hot{1.0f + flash * 0.6f, 0.19f + flash * 0.50f, 0.015f};
    Vec3 ember{0.95f, 0.10f + blast * 0.18f, 0.010f};
    addAtomicRing(add(impact, mul(n, 0.4f)), u, v, radius, hot, 1.0f, 72);
    addAtomicRing(add(impact, mul(n, 0.8f)), u, v, radius * 0.62f, {0.55f, 0.04f, 0.008f}, 1.0f, 52);
    for (int i = 0; i < 190; ++i) {
        float seed = rand01(i + 2000, 5, 9);
        float a = seed * 6.2831853f;
        float r = radius * (0.15f + rand01(i, 8, 12) * 0.92f);
        Vec3 faceP = add(impact, add(mul(u, std::cos(a) * r), mul(v, std::sin(a) * r)));
        float len = (8.0f + rand01(i, 17, 4) * 52.0f) * (0.70f + blast * 1.60f);
        addDebrisLine(faceP, add(faceP, mul(n, len)), i % 3 ? ember : hot, 1.0f);
    }
    float cloudRise = smoothstep(0.5f, 9.0f, t);
    for (int i = 0; i < 150; ++i) {
        float seed = rand01(i + 3000, 6, 2);
        float a = seed * 6.2831853f + g_time * 0.035f;
        float layer = rand01(i, 21, 7);
        float out = (18.0f + cloudRise * 86.0f) * (0.35f + layer);
        float side = (8.0f + cloudRise * 54.0f) * (rand01(i, 31, 4) - 0.5f);
        Vec3 p = add(impact, add(mul(n, out), add(mul(u, std::cos(a) * (7.0f + layer * 22.0f)), mul(v, side + std::sin(a) * (4.0f + layer * 10.0f)))));
        float s = (1.2f + seed * 3.8f) * (1.0f + cloudRise * 1.1f);
        Vec3 c = layer > 0.62f ? Vec3{0.30f, 0.08f, 0.025f} : Vec3{0.75f, 0.19f, 0.025f};
        c = {c.x + flash * 0.55f, c.y + flash * 0.25f, c.z + flash * 0.06f};
        addDebrisBox(p, u, v, n, s, c, 1.0f);
    }
}
static void rebuildHighlight() {
    g_highlight.clear();
    g_debris.clear();
    IVec3 c{}; uint8_t b = Air;
    bool hasTarget = rayCell(c, b) && b != Air && b != Target;
    float pulse = 0.75f + 0.25f * std::sin(g_time * 9.0f);
    if (hasTarget) {
        float mining = sameCell(c, g_mineCell) ? g_mineProgress : 0.0f;
        Vec3 color = b == Soft ? Vec3{1.0f, 0.30f + 0.48f * mining + 0.18f * pulse, 0.025f + 0.12f * mining} : (b == Uranium ? Vec3{0.70f + 0.25f * pulse, 1.0f, 0.02f} : Vec3{1.0f, 0.02f, 0.01f});
        float h = kBlockSize * (0.5005f + 0.0035f * pulse + 0.0060f * mining + 0.0030f * g_mineFlash);
        Vec3 p = cellCenter(c.x, c.y, c.z);
        Vec3 v[8] = {
            {p.x-h,p.y-h,p.z-h},{p.x+h,p.y-h,p.z-h},{p.x+h,p.y+h,p.z-h},{p.x-h,p.y+h,p.z-h},
            {p.x-h,p.y-h,p.z+h},{p.x+h,p.y-h,p.z+h},{p.x+h,p.y+h,p.z+h},{p.x-h,p.y+h,p.z+h}
        };
        int e[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
        for (auto& edge : e) addHighlightLine(v[edge[0]], v[edge[1]], color, static_cast<float>(b));
        if (b == Soft && mining > 0.02f) {
        Vec3 toEye = norm(sub(g_pos, p));
        int axis = 0;
        if (std::fabs(toEye.y) > std::fabs(toEye.x) && std::fabs(toEye.y) > std::fabs(toEye.z)) axis = 1;
        else if (std::fabs(toEye.z) > std::fabs(toEye.x)) axis = 2;
        float sign = axis == 0 ? (toEye.x >= 0.0f ? 1.0f : -1.0f) : (axis == 1 ? (toEye.y >= 0.0f ? 1.0f : -1.0f) : (toEye.z >= 0.0f ? 1.0f : -1.0f));
        float s = h * 0.985f;
        Vec3 crackColor{0.82f, 0.12f + 0.34f * mining + 0.08f * g_mineImpact, 0.012f + 0.045f * g_mineFlash};
        int count = 3 + static_cast<int>(mining * 10.0f);
        for (int i = 0; i < count; ++i) {
            float a = rand01(c.x + i * 7, c.y + 17, c.z + 3) * 2.0f - 1.0f;
            float d = rand01(c.x + i * 13, c.y + 5, c.z + 29) * 2.0f - 1.0f;
            float len = (0.12f + 0.36f * rand01(c.x + i, c.y + i * 3, c.z + 11)) * (0.15f + mining);
            float wobble = rand01(i, c.y, c.z) - 0.5f;
            Vec3 a0 = facePoint(p, axis, sign, a * 0.72f, d * 0.72f, s);
            Vec3 a1 = facePoint(p, axis, sign, (a + len * wobble) * 0.92f, (d + len) * 0.92f, s);
            addHighlightLine(a0, a1, crackColor, static_cast<float>(b));
        }
        int chips = 3 + static_cast<int>(mining * 8.0f + g_mineImpact * 4.0f);
        Vec3 chipColor{0.82f, 0.24f + 0.22f * mining, 0.030f + 0.055f * g_mineFlash};
        for (int i = 0; i < chips; ++i) {
            float a = rand01(c.x + i * 31, c.y + 9, c.z + 2) * 1.6f - 0.8f;
            float d = rand01(c.x + i * 17, c.y + 4, c.z + 23) * 1.6f - 0.8f;
            float kick = (0.08f + 0.26f * rand01(c.x, c.y + i, c.z)) * (mining + g_mineImpact * 0.7f);
            Vec3 p0 = facePoint(p, axis, sign, a, d, s * 1.01f);
            Vec3 p1 = facePoint(p, axis, sign, a + kick * (rand01(i, 2, c.z) - 0.5f), d + kick, s * (1.01f + kick * 0.36f));
            addHighlightLine(p0, p1, chipColor, static_cast<float>(b));
        }
        }
    }
    if (g_breakBurst > 0.001f && inBounds(g_breakCell.x, g_breakCell.y, g_breakCell.z)) {
        Vec3 p = cellCenter(g_breakCell.x, g_breakCell.y, g_breakCell.z);
        Vec3 n = norm(g_breakDir);
        Vec3 tangent = norm(std::fabs(n.y) < 0.85f ? cross(n, {0,1,0}) : cross(n, {1,0,0}));
        Vec3 bitangent = norm(cross(tangent, n));
        Vec3 burstColor{0.82f, 0.34f + 0.20f * g_breakBurst, 0.050f};
        int debris = 32 + static_cast<int>(g_breakBurst * 38.0f);
        for (int i = 0; i < debris; ++i) {
            float a = rand01(g_breakCell.x + i * 17, g_breakCell.y, g_breakCell.z) * 2.0f - 1.0f;
            float b2 = rand01(g_breakCell.x, g_breakCell.y + i * 13, g_breakCell.z) * 2.0f - 1.0f;
            float speed = 0.30f + 1.45f * rand01(i, g_breakCell.y, g_breakCell.z);
            float age = 1.0f - g_breakBurst;
            Vec3 start = add(add(p, mul(n, kBlockSize * 0.52f)), add(mul(tangent, a * kBlockSize * 0.44f), mul(bitangent, b2 * kBlockSize * 0.44f)));
            Vec3 dir = norm(add(n, add(mul(tangent, a * 0.62f), mul(bitangent, b2 * 0.62f))));
            Vec3 end = add(start, mul(dir, speed * (0.25f + age * 0.95f)));
            end.y -= age * age * 0.55f;
            addDebrisLine(start, end, burstColor, 1.0f);
        }
    }
    buildAtomicBlastLines();
}
static Vec3 blockColor(uint8_t b, int x, int y, int z) {
    float r = rand01(x, y, z);
    if (b == Soft) return {0.94f + r * 0.18f, 0.34f + r * 0.11f, 0.070f + r * 0.035f};
    if (b == Uranium) return {0.10f, 1.0f, 0.035f};
    if (b == Target) return {1.0f, 0.22f, 0.025f};
    return {0.006f + r * 0.015f, 0.012f + r * 0.018f, 0.040f + r * 0.040f};
}
static void rebuildMesh() {
    g_mesh.clear();
    g_mesh.reserve(90000);
    int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
    for (int z = 0; z < g_worldN; ++z) for (int y = 0; y < g_worldN; ++y) for (int x = 0; x < g_worldN; ++x) {
        uint8_t b = g_blocks[idx(x,y,z)];
        if (b == Air) continue;
        Vec3 c = blockColor(b, x, y, z);
        Vec3 p = cellCenter(x, y, z);
        for (int f = 0; f < 6; ++f) {
            int nx = x + dirs[f][0], ny = y + dirs[f][1], nz = z + dirs[f][2];
            if (!inBounds(nx, ny, nz) || g_blocks[idx(nx, ny, nz)] == Air) addFace(p, f, c, static_cast<float>(b));
        }
    }
    g_meshDirty = true;
}

static const char* kVs = R"GLSL(
#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec3 aColor;
layout(location=3) in float aKind;
uniform mat4 uMvp;
out vec3 vWorld;
out vec3 vNormal;
out vec3 vColor;
out float vKind;
void main(){vWorld=aPos;vNormal=aNormal;vColor=aColor;vKind=aKind;gl_Position=uMvp*vec4(aPos,1.0);}
)GLSL";
static const char* kFs = R"GLSL(
#version 330 core
in vec3 vWorld; in vec3 vNormal; in vec3 vColor; in float vKind;
uniform vec3 uEye; uniform float uTime; uniform float uHealth; uniform float uGlowPass;
out vec4 FragColor;
float hash(vec3 p){return fract(sin(dot(p,vec3(41.7,289.3,17.1)))*43758.5453);}
float hash1(float p){return fract(sin(p*91.7)*43758.5453);}
float noise(vec3 p){
    vec3 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float a=hash(i), b=hash(i+vec3(1,0,0)), c=hash(i+vec3(0,1,0)), d=hash(i+vec3(1,1,0));
    float e=hash(i+vec3(0,0,1)), f1=hash(i+vec3(1,0,1)), g=hash(i+vec3(0,1,1)), h=hash(i+vec3(1,1,1));
    return mix(mix(mix(a,b,f.x),mix(c,d,f.x),f.y),mix(mix(e,f1,f.x),mix(g,h,f.x),f.y),f.z);
}
float fbm(vec3 p){float v=0.0,a=0.55;for(int i=0;i<4;i++){v+=noise(p)*a;p=p*2.13+4.7;a*=0.48;}return v;}
void main(){
    vec3 N=normalize(vNormal), L=normalize(vec3(-0.35,0.48,0.25)), V=normalize(uEye-vWorld), H=normalize(L+V);
    float diff=max(dot(N,L),0.0), rim=pow(1.0-max(dot(N,V),0.0),2.4);
    float blockKind=floor(vKind+0.001);
    float isSoft=1.0-smoothstep(0.45,0.55,abs(blockKind-1.0));
    float isRock=1.0-smoothstep(0.45,0.55,abs(blockKind-2.0));
    float isUranium=1.0-smoothstep(0.45,0.55,abs(blockKind-3.0));
    float isTarget=1.0-smoothstep(0.45,0.55,abs(blockKind-4.0));
    float faceId=floor(fract(vKind)*32.0+0.5);
    vec3 cell=fract(vWorld/1.5+0.5);
    vec3 faceCoord = faceId < 2.5 ? cell.yzx : (faceId < 4.5 ? cell.xzy : cell.xyz);
    float edge=max(max(1.0-smoothstep(0.018,0.095,min(cell.x,1.0-cell.x)),1.0-smoothstep(0.018,0.095,min(cell.y,1.0-cell.y))),1.0-smoothstep(0.018,0.095,min(cell.z,1.0-cell.z)));
    float innerEdge=max(1.0-smoothstep(0.16,0.24,min(faceCoord.x,1.0-faceCoord.x)),1.0-smoothstep(0.16,0.24,min(faceCoord.y,1.0-faceCoord.y)));
    float grain=fbm(vWorld*3.8)*0.36 + hash(floor(vWorld*23.0))*0.10;
    float micro=fbm(vWorld*13.5+N*3.7);
    float grit=hash(floor(vWorld*34.0+faceId))*0.18;
    float ash=fbm(vWorld*0.45+vec3(0.0,uTime*0.06,0.0));
    float strata=0.5+0.5*sin(vWorld.y*9.5+noise(vWorld*0.8)*5.0+faceId);
    float lamina=0.5+0.5*sin(faceCoord.y*44.0+noise(vWorld*2.4)*8.0+faceId*1.7);
    float deepBand=0.5+0.5*sin(vWorld.y*2.7+fbm(vWorld*0.33)*9.0+faceId*2.0);
    float bandMask=smoothstep(0.24,0.88,deepBand);
    vec2 plateCell=floor(faceCoord.xy*5.0+noise(vWorld*0.9)*2.0);
    vec2 plateLocal=fract(faceCoord.xy*5.0+noise(vWorld*0.9)*2.0)-0.5;
    float plateId=hash(vec3(plateCell,faceId));
    float plateEdge=1.0-smoothstep(0.41,0.48,max(abs(plateLocal.x),abs(plateLocal.y))+0.04*sin(plateLocal.x*17.0+plateId*8.0));
    float plateShade=0.82+0.32*plateId;
    float lockPlate=step(0.42,fract(faceCoord.x*2.0))*step(0.42,fract(faceCoord.y*2.0));
    float lockSeam=max(1.0-smoothstep(0.018,0.060,abs(fract(faceCoord.x*2.0)-0.5)),1.0-smoothstep(0.018,0.060,abs(fract(faceCoord.y*2.0)-0.5)));
    float lockRune=step(0.82,noise(vec3(faceCoord.xy*10.0,faceId*2.0)+floor(vWorld*0.15)))*isRock;
    float softChip=smoothstep(0.50,0.96,noise(vWorld*12.0+faceId))*isSoft;
    float softPebble=step(0.70,noise(vWorld*20.0+N*2.5))*smoothstep(0.20,0.86,micro)*isSoft;
    float softDustPattern=smoothstep(0.25,0.92,fbm(vec3(faceCoord.xy*8.0,faceId)+vWorld*0.08))*isSoft;
    float uraniumCrystal=step(0.62,noise(vec3(faceCoord.xy*12.0,faceId)+vWorld*0.6))*isUranium;
    float uraniumGrid=max(1.0-smoothstep(0.015,0.055,abs(fract(faceCoord.x*5.0)-0.5)),1.0-smoothstep(0.015,0.055,abs(fract(faceCoord.y*5.0)-0.5)))*isUranium;
    float sediment=0.5+0.5*sin(faceCoord.y*92.0+fbm(vWorld*4.2)*7.0+faceId*5.0);
    float scorchedLine=smoothstep(0.72,0.98,sediment)*smoothstep(0.10,0.92,faceCoord.y);
    float hotNeedle=smoothstep(0.965,0.998,noise(vec3(faceCoord.xy*18.0,faceId)+vWorld*0.22+vec3(0.0,uTime*0.08,0.0)))*(1.0-edge);
    float blister=smoothstep(0.72,0.96,fbm(vWorld*7.5+N*4.0))*smoothstep(0.36,0.92,micro)*(1.0-edge);
    float fossil=sin(faceCoord.x*55.0+noise(vWorld*3.1)*8.0)*sin(faceCoord.y*38.0+faceId);
    fossil=smoothstep(0.72,0.98,fossil)*(0.45+0.55*noise(vWorld*8.0));
    float pock=step(0.73,noise(vWorld*6.5+faceId*3.1));
    float pits=smoothstep(0.58,0.94,noise(vWorld*18.0+N*5.0))*smoothstep(0.22,0.80,micro);
    float veinTrace=abs(fract(faceCoord.x*4.0+faceCoord.y*7.0+noise(vWorld*1.6)*1.9)-0.5);
    float hairline=1.0-smoothstep(0.010,0.048,veinTrace);
    float quartz=step(0.955,noise(vWorld*9.0+faceId*4.3))*smoothstep(0.25,0.85,micro);
    float salt=step(0.982,hash(floor(vWorld*47.0+faceId*11.0)));
    float rubbed=pow(max(0.0,dot(reflect(-L,N),V)),18.0)*(0.06+0.05*micro)*(1.0-edge);
    float cracks=max(max(step(0.72,noise(vWorld*2.8+N*7.0+faceId)), step(0.88,hash(floor(vWorld*13.0+faceId)))), hairline*smoothstep(0.35,0.82,ash));
    float verticalSoot=smoothstep(0.65,0.98,faceCoord.y)*smoothstep(0.18,0.0,abs(faceCoord.x-0.5));
    float slag=smoothstep(0.56,0.96,ash)*smoothstep(0.25,0.85,strata);
    float dustShelf=smoothstep(0.03,0.16,faceCoord.y)*smoothstep(0.72,0.18,faceCoord.y)*(0.45+0.55*noise(vWorld*5.5));
    float molten=hairline*smoothstep(0.66,0.96,noise(vWorld*1.9+vec3(0.0,uTime*0.05,0.0)))*(0.65+0.35*sin(uTime*2.2+vWorld.y*3.0));
    float lavaPocket=smoothstep(0.78,0.98,fbm(vWorld*1.15+vec3(0.0,uTime*0.035,0.0)))*smoothstep(0.30,0.92,ash)*(1.0-edge);
    float radioactiveDust=smoothstep(0.72,0.98,noise(vWorld*7.0+vec3(uTime*0.1,0.0,0.0)))*smoothstep(2.6,3.3,blockKind);
    vec3 ember=vec3(1.55,0.25,0.025)*(cracks*(0.10+0.13*strata)+molten*0.42);
    float faceLight = mix(0.72, 1.18, max(N.y,0.0)) * mix(0.82, 1.06, step(4.5,faceId));
    float relief=(lamina-0.5)*0.18+(micro-0.5)*0.16+plateEdge*0.09-pits*0.10;
    float bevelLight=clamp(diff+relief,0.0,1.4);
    float roughLight=0.12+bevelLight*(0.53+0.12*micro)+grain+grit;
    roughLight *= mix(1.0,0.36,isRock);
    roughLight *= mix(1.0,1.42,isSoft);
    roughLight *= mix(1.0,1.24,isUranium);
    vec3 mineralTint=mix(vec3(0.92,0.86,0.78),vec3(1.25,0.68,0.36),smoothstep(0.42,0.92,lamina))*0.14;
    mineralTint += vec3(0.12,0.045,0.020)*bandMask;
    vec3 c=vColor*roughLight*faceLight+vColor*mineralTint+vec3(0.42,0.065,0.018)*rim+ember;
    c *= 1.0-edge*0.38-innerEdge*0.16-pock*0.12-pits*0.18-verticalSoot*0.28-slag*0.22;
    c *= (0.92+0.22*lamina+0.13*micro)*plateShade;
    c=mix(c,c*vec3(0.12,0.18,0.34)+vec3(0.002,0.006,0.025),isRock*0.96);
    c=mix(c,c+vec3(0.52,0.18,0.030)*(0.45+0.55*softDustPattern),isSoft*0.72);
    c=mix(c,c*vec3(0.25,1.55,0.28)+vec3(0.015,0.42,0.020),isUranium*0.82);
    c += vec3(1.10,0.10,0.012)*edge*cracks*(0.12+0.32*isSoft);
    c += vec3(0.65,0.075,0.012)*hairline*smoothstep(0.60,0.96,ash)*(0.12+0.28*isSoft);
    c += vec3(0.08,0.055,0.040)*pits*(1.0-edge)*(0.08+0.32*isSoft);
    c += vec3(0.12,0.045,0.020)*plateEdge*(0.10+0.30*plateId)*isSoft;
    c -= vec3(0.12,0.15,0.22)*lockSeam*isRock*1.25;
    c += vec3(0.020,0.045,0.115)*lockPlate*isRock*0.48;
    c += vec3(0.020,0.090,0.220)*lockRune*isRock*0.30;
    c += vec3(0.42,0.15,0.030)*softChip*isSoft*0.44;
    c += vec3(0.34,0.20,0.090)*softPebble*isSoft*0.22;
    c += vec3(0.05,1.20,0.08)*uraniumCrystal*isUranium*0.70;
    c += vec3(0.02,0.85,0.04)*uraniumGrid*isUranium*0.44;
    c -= vec3(0.055,0.030,0.020)*scorchedLine*(0.28+0.42*ash);
    c += vec3(1.20,0.16,0.015)*hotNeedle*0.11;
    c -= vec3(0.060,0.034,0.018)*blister*0.26;
    c += vec3(0.28,0.12,0.040)*blister*rim*0.20;
    c += vec3(0.11,0.070,0.035)*fossil*0.075;
    c += vec3(0.42,0.30,0.16)*quartz*(0.03+0.15*isSoft+0.05*diff);
    c += vec3(0.20,0.15,0.08)*salt*(0.02+0.08*isSoft);
    c += vec3(0.44,0.24,0.10)*dustShelf*(0.04+0.12*isSoft);
    c += vec3(0.30,0.14,0.055)*rubbed*(0.15+0.85*isSoft);
    c += vec3(1.45,0.18,0.018)*lavaPocket*(0.04+0.22*rim)*(0.35+0.75*isSoft);
    float uranium=smoothstep(2.7,3.2,blockKind);
    float uraniumPulse=0.70+0.38*sin(uTime*5.0+vWorld.x*2.0)+0.18*noise(vWorld*4.0+uTime);
    vec3 glow=vec3(0.0);
    glow += uranium*vec3(0.60,3.6,0.06)*uraniumPulse;
    glow += radioactiveDust*vec3(0.22,1.40,0.04);
    glow += uraniumCrystal*vec3(0.12,1.80,0.04);
    glow += uraniumGrid*vec3(0.05,0.90,0.03);
    glow += hotNeedle*vec3(0.35,0.035,0.004);
    glow += lavaPocket*vec3(0.24,0.026,0.004);
    c += uranium*vec3(0.85,3.0,0.06)*uraniumPulse;
    c += radioactiveDust*vec3(0.26,1.1,0.04);
    c += uranium*vec3(0.15,0.65,0.02)*rim*1.4;
    float target=smoothstep(3.7,4.2,blockKind);
    vec3 targetGlow=target*vec3(2.7,0.42,0.04)*(0.80+0.35*sin(uTime*4.0));
    glow += targetGlow;
    c += targetGlow*1.10;
    float lavaLight=smoothstep(22.0,0.0,abs(vWorld.y+26.0))*0.26;
    c += vec3(1.05,0.15,0.025)*lavaLight*(0.55+0.45*noise(vWorld*0.14+uTime*0.1));
    glow += vec3(0.65,0.075,0.010)*lavaPocket*0.18;
    c += vec3(0.45,0.05,0.01)*pow(max(0.0,1.0-length(faceCoord.xy-vec2(0.5))*1.8),2.0)*cracks*0.16;
    c += vec3(0.22,0.035,0.006)*slag*(0.4+0.6*rim);
    float dist=length(uEye-vWorld);
    vec3 fog=mix(vec3(0.080,0.010,0.004),vec3(0.24,0.025,0.008),1.0-uHealth);
    float furnace=smoothstep(10.0,70.0,dist)*(0.08+0.08*sin(uTime*1.7+dist*0.18));
    float smokeVeil=smoothstep(12.0,82.0,dist)*(0.58+0.24*fbm(vec3(vWorld.xz*0.035,uTime*0.045)));
    float lowMiasma=smoothstep(0.0,22.0,dist)*smoothstep(18.0,-42.0,vWorld.y)*(0.35+0.20*noise(vec3(vWorld.xz*0.07,uTime*0.08)));
    float emberFog=smoothstep(8.0,72.0,dist)*(0.35+0.65*smoothstep(0.65,0.98,fbm(vec3(vWorld.xz*0.052,uTime*0.12))));
    c=mix(c,fog,smoothstep(26.0,88.0,dist)*0.76);
    c=mix(c,vec3(0.105,0.022,0.010),smokeVeil*0.24);
    c=mix(c,vec3(0.24,0.055,0.010),lowMiasma*0.18);
    c += vec3(0.16,0.030,0.005)*emberFog*0.12;
    c=mix(c,c+vec3(0.20,0.035,0.004),furnace);
    c += vec3(0.18,0.012,0.002)*(1.0-uHealth)*rim;
    c *= 1.0 + smoothstep(0.72,1.0,1.0-uHealth)*0.25;
    if(uGlowPass>0.5){
        FragColor=vec4(glow,0.34+uranium*0.26+target*0.26);
        return;
    }
    c = c/(c+vec3(0.72));
    c = pow(max(c,vec3(0.0)),vec3(0.82));
    FragColor=vec4(c,1.0);
}
)GLSL";

static const char* kSkyVs = R"GLSL(
#version 330 core
layout(location=0) in vec2 aPos;
out vec2 vUv;
void main(){
    vUv = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

static const char* kSkyFs = R"GLSL(
#version 330 core
in vec2 vUv;
uniform float uTime;
uniform float uHealth;
uniform float uAtomic;
uniform vec3 uResolution;
out vec4 FragColor;
float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123);}
float star(vec2 p){vec2 f=fract(p)-0.5;return smoothstep(0.34,0.0,length(f))*step(0.994,hash(floor(p)));}
float noise(vec2 p){
    vec2 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float a=hash(i), b=hash(i+vec2(1,0)), c=hash(i+vec2(0,1)), d=hash(i+vec2(1,1));
    return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}
float fbm(vec2 p){
    float v=0.0, a=0.52;
    for(int i=0;i<5;i++){v+=noise(p)*a;p=p*2.04+vec2(8.1,3.7);a*=0.50;}
    return v;
}
void main(){
    vec2 uv=vUv;
    vec2 q=uv-0.5;
    q.x*=uResolution.x/max(uResolution.y,1.0);
    float reached=uResolution.z;
    float danger=1.0-uHealth;
    float atomic=clamp(uAtomic,0.0,1.0);
    vec3 zenith=vec3(0.018,0.003,0.010);
    vec3 high=vec3(0.070,0.010,0.012);
    vec3 low=vec3(0.56,0.055,0.006);
    vec3 col=mix(low, zenith, smoothstep(0.02,0.96,uv.y));
    col=mix(col, high, exp(-abs(uv.y-0.54)*4.5)*0.42);

    float smokeA=fbm(vec2(uv.x*2.35,uv.y*1.70)+vec2(uTime*0.018,-uTime*0.010));
    float smokeB=fbm(vec2(uv.x*6.20+smokeA*1.8,uv.y*3.10)-vec2(uTime*0.035,uTime*0.018));
    float smokeC=fbm(vec2(uv.x*11.0+smokeB*2.0,uv.y*5.4+smokeA)-vec2(uTime*0.070,uTime*0.032));
    float shelf=sin(uv.y*34.0+smokeA*5.0+uTime*0.18)*0.5+0.5;
    float smoke=smoothstep(0.34,0.90,smokeA*0.62+smokeB*0.42+smokeC*0.24);
    float storm=fbm(vec2(atan(q.y,q.x)*1.8+uTime*0.025,length(q)*3.6-smokeA*0.8));
    col=mix(col, vec3(0.030,0.010,0.010), smoke*(0.38+0.26*smoothstep(0.25,0.90,uv.y)));
    col -= vec3(0.030,0.012,0.010)*shelf*smoke*smoothstep(0.35,0.98,uv.y)*0.32;
    col=mix(col,vec3(0.015,0.005,0.008),smoothstep(0.66,0.98,storm)*0.18*smoothstep(0.28,1.0,uv.y));

    float horizon=exp(-abs(uv.y-0.18)*9.0);
    float heat=0.56+0.44*sin(uTime*0.65+uv.x*18.0+smokeA*3.2);
    col += vec3(1.20,0.16,0.018)*horizon*(0.15+0.13*heat);
    col += vec3(3.4,0.74,0.080)*atomic*(0.44+0.96*horizon);
    col = mix(col, vec3(1.0,0.78,0.30), atomic*smoothstep(0.92,0.12,uv.y)*0.52);
    col += vec3(0.45,0.035,0.006)*exp(-dot(q-vec2(0.0,-0.28),q-vec2(0.0,-0.28))*3.8)*(0.50+0.30*danger);
    float farFurnace=exp(-dot(q-vec2(0.0,-0.42),q-vec2(0.0,-0.42))*7.0);
    float farRift=pow(max(0.0,1.0-abs(q.x+sin(uv.y*11.0+uTime*0.10)*0.045)*12.0),2.2)*smoothstep(0.03,0.38,uv.y)*(1.0-smoothstep(0.52,0.88,uv.y));
    float crown=pow(max(0.0,1.0-length(q-vec2(0.0,0.24))*1.05),4.0)*smoothstep(0.20,0.92,uv.y);
    col += vec3(1.35,0.14,0.010)*farFurnace*(0.10+0.08*smokeB);
    col += vec3(1.05,0.09,0.010)*farRift*(0.12+0.08*sin(uTime*0.9+smokeC*4.0));
    col += vec3(0.30,0.035,0.018)*crown*(0.08+0.16*storm);
    float shaft=pow(max(0.0,1.0-abs(q.x+sin(uv.y*7.0+uTime*0.12)*0.08)*3.0),3.2)*smoothstep(0.18,0.95,uv.y);
    float shaft2=pow(max(0.0,1.0-abs(q.x-0.38+sin(uv.y*9.0-uTime*0.08)*0.12)*4.2),2.8)*smoothstep(0.10,0.82,uv.y);
    float shaft3=pow(max(0.0,1.0-abs(q.x+0.52+sin(uv.y*5.0+uTime*0.06)*0.10)*5.2),2.4)*smoothstep(0.08,0.72,uv.y);
    col += vec3(0.34,0.045,0.010)*shaft*(0.10+0.18*smokeA);
    col += vec3(0.24,0.032,0.008)*(shaft2+shaft3)*(0.07+0.12*smokeB);
    float caustic=sin((uv.x+smokeA*0.12)*42.0+uTime*0.55)*sin((uv.y-smokeB*0.08)*31.0-uTime*0.32);
    col += vec3(0.11,0.026,0.006)*smoothstep(0.72,1.0,caustic)*horizon*0.16;

    vec2 cell=floor(vec2(uv.x*uResolution.x*0.18, uv.y*uResolution.y*0.18 - uTime*24.0));
    float ember=step(0.992,hash(cell));
    float emberShape=1.0-smoothstep(0.0,0.55,length(fract(vec2(uv.x*uResolution.x*0.18, uv.y*uResolution.y*0.18 - uTime*24.0))-0.5));
    col += vec3(1.0,0.22,0.035)*ember*emberShape*(0.20+0.45*smoothstep(0.10,0.82,uv.y));

    float ash=step(0.955,hash(floor(vec2(uv.x*uResolution.x*0.055+uTime*2.0,uv.y*uResolution.y*0.055-uTime*7.0))));
    float ashFine=step(0.985,hash(floor(vec2(uv.x*uResolution.x*0.22-uTime*5.0,uv.y*uResolution.y*0.22-uTime*17.0))));
    float highCinders=star(vec2(uv.x*uResolution.x*0.055+uTime*0.8,uv.y*uResolution.y*0.055-uTime*3.0))*smoothstep(0.48,1.0,uv.y);
    col += vec3(0.12,0.075,0.055)*ash*(0.08+0.10*smoke);
    col += vec3(0.18,0.11,0.075)*ashFine*(0.04+0.08*smoothstep(0.25,1.0,uv.y));
    col += vec3(1.00,0.22,0.035)*highCinders*0.20;
    float fallout=step(0.90-atomic*0.20,hash(floor(vec2(uv.x*uResolution.x*0.18+uTime*8.0,uv.y*uResolution.y*0.18-uTime*28.0))));
    col += vec3(1.35,0.28,0.050)*fallout*atomic*(0.24+0.32*smoothstep(0.05,0.8,uv.y));

    float vignette=smoothstep(0.54,1.16,length(q));
    float murk=smoothstep(0.0,0.62,uv.y)*(1.0-smoothstep(0.70,1.0,uv.y))*smoothstep(0.44,0.92,smoke);
    col=mix(col,vec3(0.055,0.014,0.010),murk*0.22);
    vec3 grade=mix(vec3(col.r*1.12,col.g*0.88,col.b*0.78),vec3(col.r*0.92,col.g*1.02,col.b*1.08),smoothstep(0.58,1.0,uv.y));
    col=mix(col,grade,0.18);
    col*=1.0-vignette*0.43;
    col+=vec3(1.35,0.18,0.020)*atomic*(1.0-vignette*0.25)*0.30;
    col=mix(col, col+vec3(0.28,0.045,0.006), danger*0.32);
    col=mix(col, vec3(0.78,0.13,0.015), reached*(0.18+0.16*horizon));
    col=col/(col+vec3(0.86));
    col=pow(max(col,vec3(0.0)),vec3(0.86));
    FragColor=vec4(col,1.0);
}
)GLSL";

static const char* kOverlayFs = R"GLSL(
#version 330 core
in vec2 vUv;
uniform float uTime;
uniform float uHealth;
uniform float uMine;
uniform float uAtomic;
uniform vec3 uResolution;
out vec4 FragColor;
float hash(vec2 p){return fract(sin(dot(p,vec2(269.5,183.3)))*43758.5453123);}
float lineHash(float p){return fract(sin(p*313.17)*43758.5453);}
float noise(vec2 p){
    vec2 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float a=hash(i), b=hash(i+vec2(1,0)), c=hash(i+vec2(0,1)), d=hash(i+vec2(1,1));
    return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}
float fbm(vec2 p){float v=0.0,a=0.55;for(int i=0;i<4;i++){v+=noise(p)*a;p=p*2.08+vec2(4.2,7.1);a*=0.50;}return v;}
void main(){
    vec2 uv=vUv;
    vec2 q=uv-0.5;
    q.x*=uResolution.x/max(uResolution.y,1.0);
    float danger=1.0-uHealth;
    float reached=uResolution.z;
    float mine=clamp(uMine,0.0,1.0);
    float atomic=clamp(uAtomic,0.0,1.0);
    float drift=fbm(vec2(uv.x*2.0+uTime*0.018,uv.y*1.4-uTime*0.025));
    float veil=smoothstep(0.44,0.95,drift)*smoothstep(0.02,0.78,uv.y);
    float lowerSmoke=exp(-abs(uv.y-0.12)*6.8)*(0.45+0.55*fbm(vec2(uv.x*4.0-uTime*0.045,uv.y*2.0)));
    vec2 emberGrid=vec2(uv.x*uResolution.x*0.13+uTime*3.2,uv.y*uResolution.y*0.13-uTime*19.0);
    vec2 emberLocal=fract(emberGrid)-0.5;
    float ember=step(0.989,hash(floor(emberGrid)))*(1.0-smoothstep(0.0,0.42,length(emberLocal)));
    vec2 ashGrid=vec2(uv.x*uResolution.x*0.055-uTime*1.1,uv.y*uResolution.y*0.055-uTime*8.4);
    float ash=step(0.955,hash(floor(ashGrid)))*(0.45+0.55*noise(ashGrid));
    float pulse=0.5+0.5*sin(uTime*1.15+drift*4.0);
    float vignette=smoothstep(0.48,1.06,length(q));
    float furnaceHalo=exp(-dot(q-vec2(0.0,-0.30),q-vec2(0.0,-0.30))*5.5)*(0.50+0.50*pulse);
    float scratch=0.0;
    for(int i=0;i<4;i++){
        float lane=lineHash(float(i)*19.7+floor(uTime*0.12));
        float x=lane*1.8-0.9+sin(uTime*0.11+float(i))*0.04;
        float y=fract(uv.y*1.7+lineHash(float(i))*0.6-uTime*(0.025+0.012*float(i)));
        scratch+=smoothstep(0.012,0.0,abs(q.x-x))*smoothstep(0.0,0.22,y)*smoothstep(1.0,0.72,y);
    }
    float heatRipple=sin((uv.y+drift*0.05)*180.0+uTime*6.0)*sin((uv.x-drift*0.04)*48.0-uTime*1.2);
    float emberBloom=smoothstep(0.015,0.0,length(emberLocal))*step(0.989,hash(floor(emberGrid)));
    float grain=hash(floor(uv*uResolution.xy+uTime*17.0))*0.5+hash(floor(uv*uResolution.xy*0.5-uTime*5.0))*0.5;
    vec3 color=vec3(0.17,0.035,0.008)*veil*0.30;
    color+=vec3(0.42,0.055,0.006)*lowerSmoke*(0.10+0.08*pulse);
    float mineBeat=pow(mine,0.55)*(0.55+0.45*sin(uTime*38.0));
    float biteRing=smoothstep(0.25,0.0,abs(length(q)-0.24-mine*0.12))*mine;
    float debrisSpray=step(0.975-mine*0.020,hash(floor(vec2(uv.x*uResolution.x*0.34+uTime*20.0,uv.y*uResolution.y*0.34-uTime*44.0))))*(1.0-smoothstep(0.05,0.68,length(q)));
    color+=vec3(1.15,0.22,0.035)*ember*(0.18+0.18*danger+0.55*mine);
    color+=vec3(1.00,0.18,0.025)*emberBloom*(0.12+0.18*danger+0.45*mine);
    color+=vec3(0.11,0.085,0.065)*ash*0.055;
    color+=vec3(0.16,0.018,0.004)*vignette*(0.22+0.18*danger);
    color+=vec3(0.32,0.055,0.008)*reached*lowerSmoke*0.10;
    color+=vec3(0.34,0.052,0.008)*furnaceHalo*0.055;
    color+=vec3(2.2,0.48,0.060)*atomic*(0.24+0.42*furnaceHalo)*(1.0-vignette*0.16);
    color+=vec3(0.95,0.14,0.018)*mine*(0.060+0.085*pulse+0.080*mineBeat)*(1.0-vignette*0.35);
    color+=vec3(1.25,0.23,0.040)*biteRing*0.065;
    color+=vec3(1.00,0.34,0.10)*debrisSpray*(0.030+0.075*mine);
    color+=vec3(0.20,0.080,0.030)*scratch*0.070;
    color+=vec3(0.065,0.017,0.005)*smoothstep(0.68,1.0,heatRipple)*lowerSmoke*0.10;
    color+=vec3(1.25,0.18,0.018)*smoothstep(0.48,1.0,heatRipple)*atomic*0.18;
    color+=vec3(grain)*0.010*(0.65+danger*0.8+atomic*3.8);
    float blastRing=smoothstep(0.055,0.0,abs(length(q)-0.18-uAtomic*0.58))*atomic;
    float secondRing=smoothstep(0.040,0.0,abs(length(q)-0.46-uAtomic*0.34))*atomic;
    color+=vec3(2.2,0.62,0.12)*blastRing*0.42;
    color+=vec3(1.4,0.24,0.035)*secondRing*0.24;
    float alpha=clamp(veil*0.065+lowerSmoke*0.060+ember*0.16+emberBloom*0.22+ash*0.035+scratch*0.045+furnaceHalo*0.025+mine*0.075+biteRing*0.12+debrisSpray*0.035+vignette*(0.09+danger*0.06)+atomic*0.34+blastRing*0.30+secondRing*0.18,0.0,0.86);
    float endFade=clamp(uResolution.z,0.0,1.0);
    color=mix(color,vec3(0.0),endFade);
    alpha=mix(alpha,1.0,endFade);
    FragColor=vec4(color,alpha);
}
)GLSL";

static void logShader(GLuint obj, bool program) {
    GLint n = 0;
    if (program) glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &n); else glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &n);
    if (n > 1) {
        std::vector<char> log(static_cast<size_t>(n) + 1);
        GLsizei out = 0;
        if (program) glGetProgramInfoLog(obj, n, &out, log.data()); else glGetShaderInfoLog(obj, n, &out, log.data());
        MessageBoxA(g_hwnd, log.data(), "CubeWorldPath shader", MB_OK | MB_ICONERROR);
    }
}
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, nullptr); glCompileShader(s);
    GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok); if (!ok) logShader(s, false); return s;
}
static GLuint createProgram(const char* vs, const char* fs) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vs), f = compileShader(GL_FRAGMENT_SHADER, fs), p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f); glLinkProgram(p);
    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok); if (!ok) logShader(p, true);
    glDeleteShader(v); glDeleteShader(f); return p;
}
static bool loadGl() {
#define LOAD(name) name = reinterpret_cast<name##Proc>(wglGetProcAddress(#name)); if(!name) return false
    LOAD(glGenVertexArrays); LOAD(glBindVertexArray); LOAD(glGenBuffers); LOAD(glBindBuffer); LOAD(glBufferData);
    LOAD(glEnableVertexAttribArray); LOAD(glVertexAttribPointer); LOAD(glCreateShader); LOAD(glShaderSource); LOAD(glCompileShader);
    LOAD(glGetShaderiv); LOAD(glGetShaderInfoLog); LOAD(glCreateProgram); LOAD(glAttachShader); LOAD(glLinkProgram); LOAD(glGetProgramiv);
    LOAD(glGetProgramInfoLog); LOAD(glUseProgram); LOAD(glDeleteShader); LOAD(glGetUniformLocation); LOAD(glUniformMatrix4fv);
    LOAD(glUniform3f); LOAD(glUniform1f);
#undef LOAD
    return true;
}

static bool initRenderer() {
    if (!loadGl()) return false;
    g_program = createProgram(kVs, kFs);
    g_skyProgram = createProgram(kSkyVs, kSkyFs);
    g_overlayProgram = createProgram(kSkyVs, kOverlayFs);
    g_uMvp = glGetUniformLocation(g_program, "uMvp");
    g_uEye = glGetUniformLocation(g_program, "uEye");
    g_uTime = glGetUniformLocation(g_program, "uTime");
    g_uHealth = glGetUniformLocation(g_program, "uHealth");
    g_uGlowPass = glGetUniformLocation(g_program, "uGlowPass");
    g_skyTime = glGetUniformLocation(g_skyProgram, "uTime");
    g_skyHealth = glGetUniformLocation(g_skyProgram, "uHealth");
    g_skyResolution = glGetUniformLocation(g_skyProgram, "uResolution");
    g_skyAtomic = glGetUniformLocation(g_skyProgram, "uAtomic");
    g_overlayTime = glGetUniformLocation(g_overlayProgram, "uTime");
    g_overlayHealth = glGetUniformLocation(g_overlayProgram, "uHealth");
    g_overlayMine = glGetUniformLocation(g_overlayProgram, "uMine");
    g_overlayResolution = glGetUniformLocation(g_overlayProgram, "uResolution");
    g_overlayAtomic = glGetUniformLocation(g_overlayProgram, "uAtomic");
    const float skyQuad[] = {
        -1.0f,-1.0f,  1.0f,-1.0f,  1.0f, 1.0f,
        -1.0f,-1.0f,  1.0f, 1.0f, -1.0f, 1.0f
    };
    glGenVertexArrays(1, &g_skyVao); glGenBuffers(1, &g_skyVbo);
    glBindVertexArray(g_skyVao); glBindBuffer(GL_ARRAY_BUFFER, g_skyVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyQuad), skyQuad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),reinterpret_cast<void*>(0));
    glGenVertexArrays(1, &g_vao); glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao); glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,kind)));
    glGenVertexArrays(1, &g_atomicVao); glGenBuffers(1, &g_atomicVbo);
    glBindVertexArray(g_atomicVao); glBindBuffer(GL_ARRAY_BUFFER, g_atomicVbo);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,kind)));
    glGenVertexArrays(1, &g_lineVao); glGenBuffers(1, &g_lineVbo);
    glBindVertexArray(g_lineVao); glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,p)));
    glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,n)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,c)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,kind)));
    glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE); glCullFace(GL_BACK);
    generateWorld(); rebuildMesh();
    return true;
}

static bool keyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }
static void updateMouse() {
    if (!g_focused || !g_mouseCaptured) return;
    RECT rc; GetClientRect(g_hwnd, &rc);
    POINT center{(rc.right-rc.left)/2,(rc.bottom-rc.top)/2}; ClientToScreen(g_hwnd,&center);
    POINT p; GetCursorPos(&p);
    if (!g_mousePrimed || g_mouseIgnoreFrames > 0) {
        SetCursorPos(center.x, center.y);
        g_mousePrimed = true;
        if (g_mouseIgnoreFrames > 0) --g_mouseIgnoreFrames;
        return;
    }
    int dx = p.x - center.x, dy = p.y - center.y;
    if (std::abs(dx) > (rc.right - rc.left) / 4 || std::abs(dy) > (rc.bottom - rc.top) / 4) {
        SetCursorPos(center.x, center.y);
        return;
    }
    if (dx || dy) { g_yaw = wrapAngle(g_yaw + dx * 0.0022f); g_pitch = clampf(g_pitch - dy * 0.0020f, -1.45f, 1.45f); SetCursorPos(center.x, center.y); }
}
static bool playerFitsRadius(Vec3 p, float r) {
    float foot = p.y - kEye + 0.16f;
    float head = p.y + 0.12f;
    Vec3 samples[10] = {
        {p.x-r,foot,p.z},{p.x+r,foot,p.z},{p.x,foot,p.z-r},{p.x,foot,p.z+r},{p.x,foot,p.z},
        {p.x-r,head,p.z},{p.x+r,head,p.z},{p.x,head,p.z-r},{p.x,head,p.z+r},{p.x,head,p.z}
    };
    for (Vec3 s : samples) {
        IVec3 c = worldToCell(s);
        if (inBounds(c.x,c.y,c.z) && solid(g_blocks[idx(c.x,c.y,c.z)])) return false;
    }
    return true;
}
static bool playerFits(Vec3 p) {
    return playerFitsRadius(p, kPlayerRadius);
}
static bool playerFitsVertical(Vec3 p) {
    return playerFitsRadius(p, kPlayerRadius * 0.34f);
}
static bool footBlocked(Vec3 p) {
    float y = p.y - kEye - 0.040f;
    float r = kPlayerRadius * 0.42f;
    Vec3 samples[5] = {
        {p.x,y,p.z},{p.x-r,y,p.z},{p.x+r,y,p.z},{p.x,y,p.z-r},{p.x,y,p.z+r}
    };
    for (Vec3 s : samples) {
        IVec3 c = worldToCell(s);
        if (inBounds(c.x,c.y,c.z) && solid(g_blocks[idx(c.x,c.y,c.z)])) return true;
    }
    return false;
}
static bool onGround() {
    return footBlocked(g_pos);
}
static void tryMove(Vec3 d) {
    Vec3 p = g_pos;
    Vec3 axes[3] = {{d.x,0,0},{0,d.y,0},{0,0,d.z}};
    for (Vec3 a : axes) {
        float mag = length(a);
        int steps = std::max(1, static_cast<int>(std::ceil(mag / 0.08f)));
        Vec3 step = mul(a, 1.0f / static_cast<float>(steps));
        for (int i = 0; i < steps; ++i) {
            Vec3 n = add(p, step);
            bool vertical = std::fabs(a.y) > 0.0f;
            bool fits = vertical ? playerFitsVertical(n) : playerFits(n);
            if (fits) {
                p = n;
            } else {
                if (vertical) g_verticalVelocity = 0.0f;
                break;
            }
        }
    }
    g_pos = p;
}
static bool rayCell(IVec3& out, uint8_t& block) {
    Vec3 f = forward();
    for (float t = 0.0f; t < 8.5f; t += 0.06f) {
        Vec3 p = add(g_pos, mul(f, t));
        IVec3 c = worldToCell(p);
        if (!inBounds(c.x,c.y,c.z)) continue;
        uint8_t b = g_blocks[idx(c.x,c.y,c.z)];
        if (b != Air) { out = c; block = b; return true; }
    }
    return false;
}
static void resetMining(float dt) {
    g_mineProgress = std::max(0.0f, g_mineProgress - dt * 2.7f);
    if (g_mineProgress <= 0.0f) {
        g_mineCell = {-1, -1, -1};
        g_mineChunk = 0;
    }
}
static void updateMining(float dt) {
    g_mineFlash = std::max(0.0f, g_mineFlash - dt * 4.6f);
    g_mineImpact = std::max(0.0f, g_mineImpact - dt * 5.8f);
    g_breakBurst = std::max(0.0f, g_breakBurst - dt * 2.45f);
    bool held = keyDown(VK_LBUTTON);
    if (!held) { resetMining(dt); return; }
    IVec3 c{}; uint8_t b = Air;
    if (!rayCell(c, b)) { resetMining(dt); return; }
    if (b == Soft) {
        if (!sameCell(c, g_mineCell)) {
            g_mineCell = c;
            g_mineProgress = std::max(0.0f, g_mineProgress * 0.35f);
            g_mineChunk = static_cast<int>(g_mineProgress * 8.0f);
            g_mineImpact = std::max(g_mineImpact, 0.45f);
        }
        float bite = 0.82f + 0.44f * std::max(0.0f, std::sin(g_time * 34.0f)) + 0.18f * rand01(c.x, c.y, c.z);
        g_mineProgress = clampf(g_mineProgress + dt * bite * 1.28f, 0.0f, 1.0f);
        int chunk = static_cast<int>(g_mineProgress * 8.0f);
        if (chunk > g_mineChunk) {
            g_mineChunk = chunk;
            g_mineImpact = 1.0f;
            g_mineFlash = std::max(g_mineFlash, 0.34f + 0.08f * static_cast<float>(chunk));
        } else {
            g_mineImpact = std::max(g_mineImpact, 0.32f + 0.56f * g_mineProgress);
        }
        if (g_mineProgress >= 1.0f) {
            Vec3 center = cellCenter(c.x, c.y, c.z);
            bool minedBelowFeet = std::fabs(g_pos.x - center.x) < kBlockSize * 0.58f && std::fabs(g_pos.z - center.z) < kBlockSize * 0.58f && center.y < g_pos.y && center.y > g_pos.y - kEye - kBlockSize * 0.90f;
            g_breakCell = c;
            g_breakDir = norm(sub(g_pos, center));
            g_breakBurst = 1.0f;
            g_blocks[idx(c.x,c.y,c.z)] = Air;
            if (minedBelowFeet) {
                g_grounded = false;
                g_verticalVelocity = std::min(g_verticalVelocity, -kDropCommitSpeed);
            }
            g_mineProgress = 0.0f;
            g_mineFlash = 1.0f;
            g_mineImpact = 1.0f;
            g_mineChunk = 0;
            g_mineCell = {-1, -1, -1};
            rebuildMesh();
        }
    } else if (b == Uranium) {
        g_mineCell = c;
        g_mineChunk = 0;
        g_mineProgress = std::max(0.0f, g_mineProgress - dt * 2.0f);
        g_mineImpact = std::max(g_mineImpact, 0.65f);
        g_health = clampf(g_health - dt * 0.42f, 0.0f, 1.0f);
    } else {
        g_mineCell = c;
        g_mineChunk = 0;
        g_mineProgress = std::max(0.0f, g_mineProgress - dt * 1.6f);
        g_mineImpact = std::max(g_mineImpact, 0.70f);
        g_mineFlash = std::max(g_mineFlash, 0.22f);
    }
}
static void update(float dt) {
    updateMouse();
    updateAtomicBlast(dt);
    float endFade = atomicEndFade();
    if (endFade >= 0.995f) {
        g_atomicEnded = true;
        g_mineRequest = false;
        updateTelemetry(dt);
        return;
    }
    bool wasGrounded = g_grounded;
    Vec3 f = forward();
    Vec3 flat = norm({f.x, 0.0f, f.z});
    Vec3 right = norm(cross(flat, {0,1,0}));
    Vec3 move{};
    if (keyDown('W')) move = add(move, flat);
    if (keyDown('S')) move = sub(move, flat);
    if (keyDown('D')) move = add(move, right);
    if (keyDown('A')) move = sub(move, right);
    if (length(move) > 0.001f) move = norm(move);
    float speed = keyDown(VK_SHIFT) ? 6.2f : 3.6f;
    tryMove(mul(move, speed * dt));
    g_grounded = onGround();
    if (g_grounded && keyDown(VK_SPACE)) {
        g_verticalVelocity = 5.4f;
        g_grounded = false;
    } else if (g_grounded) {
        g_verticalVelocity = 0.0f;
    }
    if (!g_grounded) {
        if (wasGrounded && g_verticalVelocity > -kDropCommitSpeed) g_verticalVelocity = -kDropCommitSpeed;
        g_verticalVelocity -= kGravity * dt;
        g_verticalVelocity = std::max(g_verticalVelocity, -kMaxFallSpeed);
        tryMove({0.0f, g_verticalVelocity * dt, 0.0f});
        g_grounded = onGround();
        if (g_grounded && g_verticalVelocity < 0.0f) g_verticalVelocity = 0.0f;
    }
    updateMining(dt);
    g_mineRequest = false;
    for (int z = std::max(0, worldToCell(g_pos).z - 1); z <= std::min(g_worldN - 1, worldToCell(g_pos).z + 1); ++z)
        for (int y = std::max(0, worldToCell(g_pos).y - 1); y <= std::min(g_worldN - 1, worldToCell(g_pos).y + 1); ++y)
            for (int x = std::max(0, worldToCell(g_pos).x - 1); x <= std::min(g_worldN - 1, worldToCell(g_pos).x + 1); ++x)
                if (g_blocks[idx(x,y,z)] == Uranium && length(sub(cellCenter(x,y,z), g_pos)) < 2.05f) g_health = clampf(g_health - dt * 0.10f, 0.0f, 1.0f);
    IVec3 pc = worldToCell(g_pos);
    if (inBounds(pc.x,pc.y,pc.z) && g_blocks[idx(pc.x,pc.y,pc.z)] == Target) g_reached = true;
    if (pc.y <= 1 && std::abs(pc.x - g_targetCell.x) <= 1 && std::abs(pc.z - g_targetCell.z) <= 1) g_reached = true;
    if (g_health <= 0.0f || g_pos.y < -worldHalf() - 12.0f) {
        g_health = 1.0f;
        IVec3 start = g_route.front();
        g_pos = {cellCenter(start.x, start.y, start.z).x, worldHalf() + kEye, cellCenter(start.x, start.y, start.z).z};
        g_verticalVelocity = 0.0f;
        g_grounded = true;
        g_yaw = 3.14159265f;
        g_pitch = -0.55f;
    }
    updateTelemetry(dt);
}

static void render() {
    float atomic = atomicIntensity();
    float endFade = atomicEndFade();
    glViewport(0,0,g_width,g_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glUseProgram(g_skyProgram);
    glUniform1f(g_skyTime, g_time);
    glUniform1f(g_skyHealth, g_health);
    glUniform1f(g_skyAtomic, atomic);
    glUniform3f(g_skyResolution, static_cast<float>(g_width), static_cast<float>(g_height), g_reached ? 1.0f : 0.0f);
    glBindVertexArray(g_skyVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    Vec3 fwd = forward();
    Vec3 flat = norm({fwd.x, 0.0f, fwd.z});
    Vec3 right = norm(cross(flat, {0,1,0}));
    float shock = atomic * (0.075f + 0.095f * std::sin(g_time * 17.0f) * std::sin(g_time * 41.0f));
    float shake = g_mineImpact * (0.003f + 0.0035f * g_mineProgress) + g_mineFlash * 0.003f + shock;
    Vec3 renderPos = add(g_pos, add(mul(right, std::sin(g_time * 91.0f) * shake), Vec3{0.0f, std::cos(g_time * 77.0f) * shake * (0.45f + atomic * 4.0f), 0.0f}));
    Mat4 view = lookAt(renderPos, add(renderPos, fwd), {0,1,0});
    Mat4 proj = perspective(72.0f + g_mineImpact * 0.225f + g_mineFlash * 0.325f + atomic * 18.0f, static_cast<float>(g_width) / std::max(1, g_height), 0.04f, 220.0f);
    Mat4 mvp = multiply(proj, view);
    glUseProgram(g_program);
    glUniformMatrix4fv(g_uMvp, 1, GL_FALSE, mvp.m);
    glUniform3f(g_uEye, g_pos.x, g_pos.y, g_pos.z);
    glUniform1f(g_uTime, g_time);
    glUniform1f(g_uHealth, g_health);
    glUniform1f(g_uGlowPass, 0.0f);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    if (g_meshDirty) {
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_mesh.size() * sizeof(Vertex)), g_mesh.data(), GL_DYNAMIC_DRAW);
        g_meshDirty = false;
    }
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_mesh.size()));
    buildAtomicBlastSolids();
    if (!g_atomicMesh.empty()) {
        glBindVertexArray(g_atomicVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_atomicVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_atomicMesh.size() * sizeof(Vertex)), g_atomicMesh.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_atomicMesh.size()));
    }
    glBindVertexArray(g_vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glUniform1f(g_uGlowPass, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_mesh.size()));
    if (!g_atomicMesh.empty()) {
        glBindVertexArray(g_atomicVao);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(g_atomicMesh.size()));
        glBindVertexArray(g_vao);
    }
    glUniform1f(g_uGlowPass, 0.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(g_overlayProgram);
    glUniform1f(g_overlayTime, g_time);
    glUniform1f(g_overlayHealth, g_health);
    glUniform1f(g_overlayMine, std::max(std::max(g_mineProgress, g_mineFlash * 0.95f), g_mineImpact * 0.70f));
    glUniform1f(g_overlayAtomic, atomic);
    glUniform3f(g_overlayResolution, static_cast<float>(g_width), static_cast<float>(g_height), endFade);
    glBindVertexArray(g_skyVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    rebuildHighlight();
    glUseProgram(g_program);
    glUniformMatrix4fv(g_uMvp, 1, GL_FALSE, mvp.m);
    glUniform3f(g_uEye, g_pos.x, g_pos.y, g_pos.z);
    glUniform1f(g_uTime, g_time);
    glUniform1f(g_uHealth, g_health);
    glUniform1f(g_uGlowPass, 0.0f);
    if (!g_highlight.empty()) {
        glDisable(GL_CULL_FACE);
        glLineWidth(1.6f + g_mineProgress * 2.2f + g_mineFlash * 1.2f + g_mineImpact * 0.9f);
        glBindVertexArray(g_lineVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_highlight.size() * sizeof(Vertex)), g_highlight.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_highlight.size()));
        glEnable(GL_CULL_FACE);
    }
    if (!g_debris.empty()) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glLineWidth(1.7f + g_breakBurst * 2.6f);
        glBindVertexArray(g_lineVao);
        glBindBuffer(GL_ARRAY_BUFFER, g_lineVbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(g_debris.size() * sizeof(Vertex)), g_debris.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(g_debris.size()));
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }
    SwapBuffers(g_hdc);
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CLOSE: g_running = false; PostQuitMessage(0); return 0;
    case WM_SIZE: g_width = std::max(1, static_cast<int>(LOWORD(lp))); g_height = std::max(1, static_cast<int>(HIWORD(lp))); return 0;
    case WM_SETFOCUS: g_focused = true; g_mousePrimed = false; g_mouseIgnoreFrames = 8; return 0;
    case WM_KILLFOCUS: g_focused = false; g_mousePrimed = false; g_mouseIgnoreFrames = 8; return 0;
    case WM_LBUTTONDOWN: g_mouseCaptured = true; ShowCursor(FALSE); g_mineRequest = true; return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) {
            if (g_mouseCaptured) { g_mouseCaptured = false; ShowCursor(TRUE); }
            else { g_running = false; PostQuitMessage(0); }
        }
        return 0;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}
static bool createContext() {
    WNDCLASSA wc{}; wc.style = CS_OWNDC; wc.lpfnWndProc = wndProc; wc.hInstance = GetModuleHandle(nullptr); wc.lpszClassName = "CubeWorldPathWindow"; wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);
    RECT r{0,0,g_width,g_height}; AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
    g_hwnd = CreateWindowA("CubeWorldPathWindow", "Cube World Path - Modern OpenGL 3.3", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, r.right-r.left, r.bottom-r.top, nullptr, nullptr, wc.hInstance, nullptr);
    g_hdc = GetDC(g_hwnd);
    PIXELFORMATDESCRIPTOR pfd{}; pfd.nSize=sizeof(pfd); pfd.nVersion=1; pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER; pfd.iPixelType=PFD_TYPE_RGBA; pfd.cColorBits=32; pfd.cDepthBits=24; pfd.cStencilBits=8;
    int pf = ChoosePixelFormat(g_hdc, &pfd); SetPixelFormat(g_hdc, pf, &pfd);
    HGLRC temp = wglCreateContext(g_hdc); wglMakeCurrent(g_hdc, temp);
    auto createAttribs = reinterpret_cast<WGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (createAttribs) {
        int attribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,3,WGL_CONTEXT_MINOR_VERSION_ARB,3,WGL_CONTEXT_PROFILE_MASK_ARB,WGL_CONTEXT_CORE_PROFILE_BIT_ARB,0};
        g_glrc = createAttribs(g_hdc, nullptr, attribs);
        wglMakeCurrent(nullptr, nullptr); wglDeleteContext(temp); wglMakeCurrent(g_hdc, g_glrc);
    } else g_glrc = temp;
    ShowCursor(FALSE);
    RECT client{};
    GetClientRect(g_hwnd, &client);
    POINT center{(client.right - client.left) / 2, (client.bottom - client.top) / 2};
    ClientToScreen(g_hwnd, &center);
    SetCursorPos(center.x, center.y);
    QueryPerformanceFrequency(&g_frequency); QueryPerformanceCounter(&g_lastCounter);
    return g_glrc != nullptr;
}
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    timeBeginPeriod(1);
    if (!createContext() || !initRenderer()) return 1;
    initTelemetry();
    MSG msg{};
    while (g_running) {
        while (PeekMessage(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        float dt = static_cast<float>(now.QuadPart - g_lastCounter.QuadPart) / static_cast<float>(g_frequency.QuadPart);
        g_lastCounter = now; dt = clampf(dt, 0.0f, 0.05f); g_time += dt;
        update(dt); render();
    }
    shutdownTelemetry();
    timeEndPeriod(1);
    return 0;
}
