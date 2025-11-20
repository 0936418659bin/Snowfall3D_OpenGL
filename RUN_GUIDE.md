# Snowfall3D - Hướng dẫn chạy chương trình

## Yêu cầu hệ thống

- **Windows 10/11** (64-bit)
- **Visual Studio 2022** Community/Professional hoặc mới hơn
- **CMake** 3.10+
- **vcpkg** (đã cài sẵn, toolchain file: `C:/vcpkg/scripts/buildsystems/vcpkg.cmake`)
- **Graphics**: GPU hỗ trợ OpenGL 3.3+

## Dependencies (tự động cài qua vcpkg)

- **GLFW 3.3**: Windowing & Input
- **GLM**: Math library
- **GLAD**: OpenGL loader
- **Assimp 6.0**: Model loading (OBJ, FBX, DAE, etc.)
- **OpenGL 3.3 Core**: Graphics API

## Hướng dẫn build & run

### 1. **Clone/Mở project**

```bash
cd e:\OpenGL\GiuaKi
```

### 2. **Build project (lần đầu hoặc sau khi thay đổi source)**

#### **Windows - Visual Studio 2022**

```bash
cd build

# Tạo build files (nếu build folder chưa tồn tại)
cmake -G "Visual Studio 17 2022" -A x64 ..

# Build
cmake --build . --config Release -- /m

# Hoặc sử dụng MSBuild trực tiếp
msbuild .\Snowfall3D.vcxproj /p:Configuration=Release /m
```

Lưu ý: vcpkg toolchain sẽ tự động tải & build dependencies (GLFW, GLM, Assimp, etc.)

### 3. **Chạy chương trình**

```bash
# Release build
.\bin\Release\Snowfall3D.exe

# Hoặc từ thư mục gốc project
cd ..\
.\build\bin\Release\Snowfall3D.exe
```

## Cấu trúc thư mục

```
Snowfall3D/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files (.h)
│   ├── *.h                 # Classes: Camera, Terrain, Vegetation, etc.
│   └── external/glad/      # GLAD OpenGL loader
├── src/                    # Source files (.cpp)
│   ├── main.cpp            # Entry point & render loop
│   ├── *.cpp               # Implementation files
├── shaders/                # GLSL shaders
│   ├── *.vert              # Vertex shaders
│   ├── *.frag              # Fragment shaders
├── assets/                 # 3D models (OBJ, FBX, etc.)
│   └── tree.obj            # Test tree model (Assimp-loaded)
├── build/                  # Build output (created by CMake)
│   └── bin/Release/        # Executable & runtime files
│       ├── Snowfall3D.exe  # Main executable
│       ├── shaders/        # Copied shader files (auto-copied by CMake)
│       └── assets/         # Copied model files (auto-copied by CMake)
└── README.md / RUN_GUIDE.md
```

## In-game Controls

### **Camera & Movement**
- `WASD` - Move forward/left/backward/right
- `Space` - Move up
- `Left Shift` - Move down
- `Mouse Move` - Look around
- `Scroll Wheel` - Zoom in/out

### **Weather & Environment**
- `P` - Pause/resume weather
- `R` - Cycle precipitation mode (Snow → Mix → Rain → Snow)
- `[` / `]` - Decrease/increase weather intensity
- `J` / `L` - Adjust wind direction left/right
- `I` / `K` - Adjust particle emission rate
- `Z` / `X` - Quick increase/decrease snowfall

### **Rendering & Display**
- `O` - Toggle projection (Perspective ↔ Orthographic)
- `H` - Toggle on-screen stats (FPS, particle count, etc.)
- `T` - Advance time by +1 hour
- `Y` - Toggle day/night cycle (12h ↔ 0h)
- `B` - Toggle auto time progression

### **Misc**
- `ESC` - Exit program

## Troubleshooting

### **Build fails with "Could not find assimp"**
- Assimp not installed via vcpkg
- Solution: Run `vcpkg install assimp:x64-windows`

### **"Cannot find vcpkg.cmake"**
- vcpkg path mismatch in CMakeLists.txt (line 3)
- Update: `set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")`

### **Shaders not loading**
- Shader files must be in `build/bin/Release/shaders/`
- CMake POST_BUILD copies them automatically
- If missing, manually: `cp -r shaders build/bin/Release/`

### **Models not loading (Assimp error)**
- Model files must be in `build/bin/Release/assets/`
- CMake POST_BUILD copies them automatically
- Add `.obj`/`.fbx` files to `assets/` folder before build

### **Runtime crashes with "gladLoadGLLoader failed"**
- GPU doesn't support OpenGL 3.3+
- Check `glxinfo` or use GPU-Z to verify GPU capabilities

### **Very low FPS**
- Reduce particle count: Press `[` to decrease intensity
- Reduce tree instancing: Edit `vegetation.Generate()` in main.cpp (change `treeCount` parameter)
- Enable V-Sync if supported by driver

## Project Features

### **Graphics Systems**
- **Instanced Rendering**: 180 trees rendered with GPU instancing (efficient)
- **Procedural Vegetation**: Branching trees with wind sway
- **Billboarded Leaves**: Camera-facing foliage cards with per-leaf animation
- **Terrain**: Perlin noise-based height map with dynamic snow accumulation
- **Skybox**: Full 360° sky dome with time-of-day lighting
- **Clouds**: Tiled billboard clouds with coverage control

### **Particle System**
- **Snowfall/Rain/Mix**: Three precipitation modes
- **Wind Physics**: Particles affected by wind direction & strength
- **Terrain Collision**: Particles accumulate on ground

### **Model Loading**
- **Assimp Integration**: Load `.obj`, `.fbx`, `.dae`, `.blend` (with plugins)
- **GPU Instancing**: Same model rendered multiple times efficiently

## Performance Tips

1. **Reduce particle count** if FPS < 30:
   - Press `[` multiple times to lower intensity
   
2. **Reduce tree instancing**:
   - Edit `src/main.cpp` line ~115: `vegetation.Generate(terrain, 1200, 180);`
   - Change `180` to smaller number (e.g., `60` for fewer trees)
   
3. **Use simpler models**:
   - Replace `assets/tree.obj` with a lower-poly version
   - Recommended: 2,000–5,000 vertices per tree
   
4. **Enable GPU acceleration**:
   - Use dedicated GPU (NVIDIA/AMD)
   - Disable CPU-integrated graphics if dual-GPU system

## Adding Custom Tree Models

1. Find/create a tree model (`.obj`, `.fbx`, `.dae`)
2. Place in `assets/` folder (e.g., `assets/my_tree.obj`)
3. Edit `src/main.cpp` around line ~120:
   ```cpp
   vegetation.LoadTreeModel("assets/my_tree.obj");
   ```
4. Rebuild: `cmake --build build --config Release -- /m`
5. Run & enjoy!

**Recommended sources**:
- Sketchfab (https://sketchfab.com) - Search "tree", filter FREE
- Poly Haven (https://polyhaven.com/models)
- TurboSquid Free (https://www.turbosquid.com/Search/3D-Models/free)

## Build Configurations

### **Debug Build** (slower, more info)
```bash
cmake --build build --config Debug -- /m
```

### **Release Build** (fast, optimized)
```bash
cmake --build build --config Release -- /m
```

## File Locations After Build

After successful build, the executable and assets are organized as:

```
build/
├── bin/
│   ├── Debug/
│   │   ├── Snowfall3D.exe
│   │   ├── shaders/        ← Shader files (auto-copied)
│   │   ├── assets/         ← Model files (auto-copied)
│   │   └── *.dll           ← Runtime dependencies (GLFW, Assimp, etc.)
│   └── Release/
│       ├── Snowfall3D.exe  ← Use this
│       ├── shaders/
│       ├── assets/
│       └── *.dll
```

## Next Steps

- Customize tree colors/scale in `src/Vegetation.cpp`
- Add new weather effects in `src/ParticleSystem.cpp`
- Modify terrain generation in `src/Terrain.cpp`
- Adjust lighting in `src/Light.cpp`
- Edit shaders in `shaders/` for visual effects

Enjoy Snowfall3D! ❄️

