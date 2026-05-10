# Dead Channel

Dead Channel is a native C++ first-person walking game set inside abandoned analogue TV landscapes. You move toward a distant transmission tower while corrupted help signals, scanlines, macroblocks, and unstable geometry intensify as you get closer.

The workspace also contains **The White Figure**, a separate first-person walking game using the same Win32 + OpenGL 3.3 technology. It is set in a sundrenched modernist/brutalist Middle Eastern city where a tiny white humanoid silhouette appears impossibly far away on ridges, rooftops, towers, and dunes. Reaching a sighting causes the figure to vanish into heat haze and reappear farther away.

It also contains **The Endless Airport Gate**, a third first-person walking game set inside a surreal terminal where Gate 47 is always visible somewhere in the distance but never nearby. Polished concourses, gate counters, moving walkways, baggage conveyors, escalator-like ramps, glass walls, and repeating overhead architecture create puzzle-like temporal loops.

## Controls

- `WASD` - walk
- `Shift` - move faster
- `Space` - jump
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
