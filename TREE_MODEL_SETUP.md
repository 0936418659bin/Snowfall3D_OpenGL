# Tích hợp Assimp - Hướng dẫn sử dụng mô hình cây 3D

## Tổng quan

Dự án đã được tích hợp với **Assimp (Open Asset Importer Library)** để load và render mô hình cây 3D từ các file:
- `.obj`
- `.fbx` 
- `.dae`
- `.blend`
- và nhiều format khác

Thay vì sử dụng cây được tạo thủ công bằng procedural geometry, bạn có thể import mô hình cây đẹp từ các website miễn phí hoặc tự tạo.

## Cách sử dụng

### 1. **Tìm kiếm mô hình cây 3D**

Các website có mô hình miễn phí:
- **Sketchfab** (https://sketchfab.com) - Tìm "tree" → Filter `FREE` → Download OBJ/FBX
- **CGTrader Free** (https://www.cgtrader.com/free-3d-models/plants/trees)
- **TurboSquid Free** (https://www.turbosquid.com/Search/3D-Models/free/trees)
- **Poly Haven** (https://polyhaven.com/models?category=Plants)
- **Free3D** (https://free3d.com/3d-models/trees)

**Gợi ý**: Chọn các mô hình có **geometry đơn giản** (dưới 10,000 vertices) để render nhanh hơn khi dùng instancing với 180 cây.

### 2. **Đặt file model vào thư mục project**

Tạo thư mục `assets` tại thư mục gốc của project (cùng cấp với `CMakeLists.txt`):

```
E:\OpenGL\GiuaKi\
  ├── CMakeLists.txt
  ├── include/
  ├── src/
  ├── shaders/
  └── assets/
      └── tree.obj    ← Đặt file model ở đây
```

**Các tên file hỗ trợ (theo thứ tự ưu tiên)**:
- `tree.obj`
- `assets/tree.obj`
- `../assets/tree.obj`

### 3. **Build và chạy**

Chương trình sẽ tự động:
- Tìm file `tree.obj` (hoặc các vị trí khác)
- Load mô hình bằng Assimp
- Render 180 instances của cây này trên terrain

```bash
cd build
cmake --build . --config Release -- /m
.\bin\Release\Snowfall3D.exe
```

Nếu tìm thấy mô hình:
```
Loaded tree model: assets/tree.obj (XXXXX indices)
```

Nếu không tìm thấy:
```
Note: Tree model file not found. Using procedural trees.
```

## Cấu hình Assimp

Hàm `LoadTreeModel()` sử dụng các post-processing flags:

```cpp
aiProcess_Triangulate      // Chuyển đổi tất cả faces thành triangles
aiProcess_FlipWindingOrder // Điều chỉnh winding order cho OpenGL
aiProcess_CalcTangentSpace // Tính toán tangent/bitangent (optional)
```

Có thể mở rộng thêm:
- `aiProcess_RemoveRedundantMaterials` - Loại bỏ materials trùng lặp
- `aiProcess_OptimizeMeshes` - Tối ưu meshes
- `aiProcess_SortByPType` - Sắp xếp theo primitive type

## Chi tiết kỹ thuật

### Hàm LoadTreeModel()

```cpp
bool Vegetation::LoadTreeModel(const std::string &modelPath)
```

**Tham số:**
- `modelPath`: Đường dẫn tới file model (`.obj`, `.fbx`, v.v.)

**Trả về:**
- `true`: Load thành công
- `false`: Lỗi khi load

**Quá trình hoạt động:**
1. Assimp đọc file model
2. Trích xuất tất cả vertices và indices từ tất cả meshes
3. Upload lên GPU (VBO + EBO)
4. Setup VAO với instance attributes (model matrices + seeds)
5. Enable flag `useLoadedModel = true`

### Rendering

Khi `useLoadedModel == true`, hàm `Render()` sử dụng:

```cpp
glDrawElementsInstanced(GL_TRIANGLES, modelVertCount, GL_UNSIGNED_INT, 0, (GLsizei)treeInstances.size());
```

**Kết quả:**
- Render 180 instances của mô hình cây
- Mỗi instance có:
  - Vị trí khác nhau trên terrain
  - Model matrix được truyền qua instance attribute
  - Per-instance seed cho wind sway

## Tối ưu hiệu suất

### 1. **Giảm độ phức tạp của model**

- Dùng Blender để decimate meshes (giảm vertices)
- Target: 2,000-5,000 vertices per tree
- Lý do: 180 cây × 5,000 verts = 900,000 verts/frame (reasonable)

### 2. **Texture baking**

- Bake lighting vào vertex colors (thay vì dùng shaders phức tạp)
- Bake normal maps vào model trực tiếp

### 3. **LOD (Level of Detail)**

Có thể mở rộng để:
- Load model LOD cao gần camera
- Dùng procedural geometry LOD thấp ở xa

## Mở rộng trong tương lai

### 1. **Leaf texture**
- Thêm texture UV cho lá cây
- Sử dụng texture alpha-blending trong shader

### 2. **Multiple tree types**
```cpp
vegetation.LoadTreeModel("assets/tree_oak.obj");    // Oak tree
vegetation.LoadTreeModel("assets/tree_pine.obj");   // Pine tree
vegetation.LoadTreeModel("assets/tree_birch.obj");  // Birch tree
```

### 3. **Skeleton animation**
- Load rigged models với bones
- Animate branches bằng wind force

### 4. **Shadow mapping**
- Generate depth map từ sun direction
- Render tree shadows lên terrain

## Troubleshooting

| Vấn đề | Giải pháp |
|--------|----------|
| Model không hiển thị | Kiểm tra file path, format file (.obj, .fbx valid?) |
| Model quá nhỏ/lớn | Scale model trong Blender trước export |
| Model xoay sai hướng | Thêm `aiProcess_ConvertToLeftHanded` nếu cần |
| Performance kém | Giảm số vertices của model hoặc giảm số instances |
| Lỗi load Assimp | Đảm bảo Assimp DLL có trong build folder |

## Ví dụ file `.obj` đơn giản

Nếu muốn tạo tree model tối giản trong Blender:

1. Tạo UV Sphere (trunk)
2. Tạo vài Cone (branches)
3. Combine all objects
4. Export as `.obj`

Kích thước khuyến nghị: 2-3 units cao, 0.5 units rộng (sẽ được scale bởi `t0.scale` trong Vegetation.cpp)

## Liên hệ

- **Assimp Docs**: https://assimp.org/
- **Assimp GitHub**: https://github.com/assimp/assimp
- **OpenGL + Assimp Tutorial**: https://learnopengl.com/Model-Loading/Assimp

