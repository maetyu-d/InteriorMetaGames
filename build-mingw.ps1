$ErrorActionPreference = "Stop"
New-Item -ItemType Directory -Force -Path build | Out-Null
g++ -std=c++17 -O2 src\DeadChannel.cpp -o build\DeadChannel.exe -lopengl32 -lgdi32 -luser32 -lwinmm
g++ -std=c++17 -O2 src\TheWhiteFigure.cpp -o build\TheWhiteFigure.exe -lopengl32 -lgdi32 -luser32 -lwinmm
g++ -std=c++17 -O2 src\EndlessAirportGate.cpp -o build\EndlessAirportGate.exe -lopengl32 -lgdi32 -luser32 -lwinmm
g++ -std=c++17 -O2 src\CubeWorldPath.cpp -o build\CubeWorldPath.exe -lopengl32 -lgdi32 -luser32 -lwinmm
