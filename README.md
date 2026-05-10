# Dead Channel

Dead Channel is a native C++ first-person walking game set inside abandoned analogue TV landscapes. You move toward a distant transmission tower while corrupted help signals, scanlines, macroblocks, and unstable geometry intensify as you get closer.

The workspace also contains **The White Figure**, a separate first-person walking game using the same Win32 + OpenGL 3.3 technology. It is set in a sundrenched modernist/brutalist Middle Eastern city where a tiny white humanoid silhouette appears impossibly far away on ridges, rooftops, towers, and dunes. Reaching a sighting causes the figure to vanish into heat haze and reappear farther away.

It also contains **The Endless Airport Gate**, a third first-person walking game set inside a surreal terminal where Gate 47 is always visible somewhere in the distance but never nearby. Polished concourses, gate counters, moving walkways, baggage conveyors, escalator-like ramps, glass walls, and repeating overhead architecture create puzzle-like temporal loops.

It also contains **Cube World Path**, a fourth first-person mining game set on a hellish, randomly sized solid cubic voxel world from 32x32x32 through 64x64x64. The player starts on the top face of the cube and must mine down to a target on the opposite bottom face. Only one complete route reaches the target; false mineable branches, unbreakable rock, and toxic uranium veins block false progress.

## Controls

- `WASD` - walk
- `Shift` - move faster
- `Space` - jump
- Left mouse - mine the highlighted block in Cube World Path
- `E` - drink at oasis water in The White Figure
- `Q` - use the water-powered jetpack in The White Figure
- Left mouse - fire weapon in The White Figure
- `Mouse` - look
- `Esc` - release mouse / quit
- `F11` - toggle fullscreen

In The White Figure, the small top-left meters show thirst and jetpack water. As thirst fills, heat haze, glare, and fever tint intensify; drinking at oasis water lowers thirst and refills jetpack water.

After the final colossus sighting, shoot the two eye targets first, then repeatedly shoot the chest target.

## Build

The game is intentionally dependency-free and uses Win32 + a shader-based OpenGL 3.3 renderer. It creates a modern context directly through WGL, loads the required OpenGL entry points itself, renders the world through VAO/VBO streams, and applies the analogue corruption through a framebuffer post-process shader.

### Visual Studio Developer Command Prompt

```bat
build-msvc.bat
```

### MinGW-w64

```powershell
.\build-mingw.ps1
```

The executables are written to:

- `build\DeadChannel.exe`
- `build\TheWhiteFigure.exe`
- `build\EndlessAirportGate.exe`
- `build\CubeWorldPath.exe`

## External Signal Telemetry

Dead Channel does not show the real signal distance to the player, but it publishes it for another process to read.

- Named shared memory: `Local\DeadChannelSignalTelemetry`
- Sidecar JSON file: `build\DeadChannelSignalTelemetry.json`
- `realDistance` is the horizontal world-unit distance from the player to the real final signal, ignoring the false-start towers
- `threeDimensionalDistance` includes height difference
- `playerSpeed` and `playerHorizontalSpeed` expose current movement speed in world units per second
- `playerVelocity` exposes the current per-axis velocity
- `playerFacing`, `playerYaw`, and `playerPitch` expose camera/look direction
- `playerMoveDirection` exposes normalized horizontal movement direction, or zero while still
- `visibleTowerZ`, `falseStartStage`, and `finalStage` expose the current decoy/progression state without changing the in-game display

The White Figure publishes the same hidden-player telemetry contract for external games and tools.

- Named shared memory: `Local\TheWhiteFigureSignalTelemetry`
- Sidecar JSON file: `build\TheWhiteFigureSignalTelemetry.json`
- `realDistance` is the horizontal world-unit distance from the player to the real final colossus sighting, ignoring the false starts
- `player`, `playerVelocity`, `playerSpeed`, `playerHorizontalSpeed`, `playerFacing`, `playerYaw`, `playerPitch`, and `playerMoveDirection` match the Dead Channel fields
- `thirst`, `waterFuel`, `jetpackPulse`, `health`, and `isGrounded` expose the current player survival and movement state
- `colossus` exposes eye-target state, chest-hit progress, required chest hits, and defeat state

The Endless Airport Gate also publishes player telemetry for external games and tools.

- Named shared memory: `Local\EndlessAirportGateTelemetry`
- Sidecar JSON file: `build\EndlessAirportGateTelemetry.json`
- `currentGateDistance` is the horizontal world-unit distance from the player to the currently visible Gate 47
- `realGateDistance` and `realDistance` are the horizontal distance to the final real Gate 47
- `currentGate`, `realGate`, `gate47.stage`, `gate47.finalStage`, and `gate47.found` expose the loop/progression state
- Player position, velocity, speed, facing, move direction, yaw, pitch, and grounded state match the other games

Cube World Path publishes hidden player and target telemetry for external games and tools.

- Named shared memory: `Local\CubeWorldPathTelemetry`
- Sidecar JSON file: `build\CubeWorldPathTelemetry.json`
- `realDistance` and `targetDistance` are the 3D world-unit distance from the player to the true bottom-face target
- `planarDistance` is the horizontal X/Z distance to the target
- `player`, `playerVelocity`, `playerSpeed`, `playerHorizontalSpeed`, `playerFacing`, `playerYaw`, `playerPitch`, and `playerMoveDirection` match the other games
- `target`, `targetCell`, `worldSize`, and `signalReached` expose the generated cube target state without showing it in-game

## Graphics Implementation

- OpenGL 3.3 core-profile context created with WGL
- GLSL world shader for glossy wet terrain, metallic TV props, pylons, and the tower
- Procedural VBO terrain and prop streaming around the player
- Offscreen framebuffer pass with chromatic aberration, scanlines, signal tearing, grain, vignette, and macroblock compression
- Corruption strength is tied to distance from the transmission tower

## The White Figure Graphics

- Uses the same native Win32 OpenGL 3.3 renderer as Dead Channel
- Separate sun-bleached shader palette with pale concrete, sand, blue shaded glass, and limestone forms
- Heavy heat-haze post process with bloom, glare, dust, and horizon shimmer
- Procedural dunes, grounded brutalist blocks, shade columns, plinths, rooftop/perch sightings, and collision-aware buildings

## The Endless Airport Gate Graphics

- Uses the same native Win32 OpenGL 3.3 renderer and framebuffer post-process path
- Separate terminal aesthetic with polished tile floors, glass walls, suspended light bars, airport signage, gate counters, seats, baggage conveyors, moving walkways, and escalator structures
- Gate 47 is procedurally repositioned through looping stages when approached, then becomes reachable after roughly a 15-20 minute route
- Conveyor and walkway bands subtly affect player movement, giving the terminal a different traversal rhythm from the other two games

## Cube World Path

- Uses the same native Win32 OpenGL 3.3 approach with shader-rendered VBO voxel faces
- Generates a solid cube world across the 32x32x32 to 64x64x64 size spectrum, with 50% larger blocks, false mineable branches, and exactly one complete route from the top starting face to the opposite bottom target face
- Brown soft path blocks can be mined; grey rock is unbreakable; glowing green uranium is toxic and unbreakable
- Hellish shader treatment adds charred stone, ember cracks, acidic uranium glow, red fog, and hot target lighting
- The player walks and jumps through opened tunnels while mining forward through the cube
