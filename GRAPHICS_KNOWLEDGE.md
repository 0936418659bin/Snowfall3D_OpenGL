# Tài liệu Học Graphics - Các Phép Chiếu & Kiến Thức Cơ Bản

## I. CÁC PHÉP CHIẾU (PROJECTION)

### 1. **Perspective Projection (Phép chiếu Perspective)**

**Định nghĩa**: Hiển thị 3D như góc nhìn con người - vật xa nhỏ hơn, vật gần lớn hơn.

**Công thức (GLM)**:
```cpp
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),           // Field of View (FOV) - độ rộng tầm nhìn
    (float)width / (float)height,  // Aspect ratio - tỷ lệ khung hình
    0.1f,                          // Near plane - mặt phẳng gần (z-min)
    100.0f                         // Far plane - mặt phẳng xa (z-max)
);
```

**Tính chất**:
- Vật phía xa có kích thước nhỏ hơn (giống thực tế)
- Thường dùng cho game, mô phỏng
- Tạo hiệu ứng sâu độc lập

**Ứng dụng trong project**:
```cpp
// Trong main.cpp render loop
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
    (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
```

### 2. **Orthographic Projection (Phép chiếu Orthographic)**

**Định nghĩa**: Hiển thị 3D mà không có hiệu ứng perspective - vật xa/gần cùng kích thước.

**Công thức (GLM)**:
```cpp
glm::mat4 projection = glm::ortho(
    -width/2.0f,   // Left (x-min)
    width/2.0f,    // Right (x-max)
    -height/2.0f,  // Bottom (y-min)
    height/2.0f,   // Top (y-max)
    0.1f,          // Near plane
    100.0f         // Far plane
);
```

**Tính chất**:
- Không có "nhỏ xa, lớn gần"
- Dùng cho CAD, UI, map view từ trên xuống
- Tạo góc nhìn như bản đồ

**Toggle trong project** (phím O):
```cpp
// Thêm vào processInput()
if (curO && !prevO) {
    useOrthographic = !useOrthographic;
    std::cout << "Projection: " << (useOrthographic ? "Orthographic" : "Perspective") << "\n";
}
```

---

## II. COORDINATE SYSTEMS (HỆ TỌA ĐỘ)

### **Các bước biến đổi (Transformation Pipeline)**

```
Local Space (Model Space)
        ↓ (Model Matrix)
World Space
        ↓ (View Matrix)
Camera Space (View Space)
        ↓ (Projection Matrix)
Clip Space
        ↓ (Perspective Divide)
Normalized Device Coordinates (NDC): [-1, 1]
        ↓ (Viewport Transform)
Screen Space: [0, width] × [0, height]
```

### **Công thức tính toán**:
```glsl
// Trong vertex shader
gl_Position = projection * view * model * vec4(position, 1.0);

// Hoặc từng bước:
vec4 worldPos = model * vec4(position, 1.0);
vec4 viewPos = view * worldPos;
vec4 clipPos = projection * viewPos;
```

### **Matrix math trong GLM**:
```cpp
// Model matrix - dịch chuyển, quay, scale
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(x, y, z));
model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
model = glm::scale(model, glm::vec3(scale, scale, scale));

// View matrix - camera position/direction
glm::mat4 view = glm::lookAt(
    glm::vec3(0.0f, 5.0f, 10.0f),  // Camera position
    glm::vec3(0.0f, 0.0f, 0.0f),   // Look at (target)
    glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
);

// Projection matrix
glm::mat4 projection = glm::perspective(...);

// Kết hợp
glm::mat4 mvp = projection * view * model;
```

---

## III. CAMERA & VIEW MATRIX

### **View Matrix - Ma trận nhìn**

Định nghĩa: Biến đổi tất cả vật thể sao cho camera ở gốc tọa độ.

**Công thức GLM**:
```cpp
glm::mat4 view = glm::lookAt(
    eye,      // Vị trí camera
    center,   // Điểm camera nhìn đến
    up        // Hướng lên của camera (thường là +Y)
);
```

**Ví dụ trong project**:
```cpp
glm::mat4 view = camera.GetViewMatrix();
// Hoặc tính trực tiếp:
// view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
```

### **Camera Movement - Di chuyển camera**

```cpp
// Trong Camera::ProcessKeyboard()
if (direction == FORWARD)
    position += front * speed;
else if (direction == BACKWARD)
    position -= front * speed;
else if (direction == LEFT)
    position -= right * speed;
else if (direction == RIGHT)
    position += right * speed;
else if (direction == UP)
    position += up * speed;
else if (direction == DOWN)
    position -= up * speed;
```

---

## IV. NORMAL MAP & NORMAL TRANSFORMATION

### **Normal Mapping - Lập bản đồ pháp tuyến**

**Mục đích**: Tạo chi tiết bề mặt mà không tăng số đỉnh (vertex).

**Cách hoạt động**:
1. Lưu trữ normal vector của pixel trong texture (RGB = XYZ)
2. Trong fragment shader, lấy normal từ texture
3. Tính lighting với normal này thay vì normal từ geometry

**Công thức GLSL**:
```glsl
#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;

uniform sampler2D normalMap;
uniform sampler2D diffuseMap;

void main() {
    // Sample normal từ texture
    vec3 norm = texture(normalMap, TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);  // Convert [0,1] -> [-1,1]
    
    // Tạo TBN matrix (Tangent-Binormal-Normal)
    vec3 N = normalize(Normal);
    vec3 T = normalize(Tangent);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    
    // Transform normal sang world space
    norm = normalize(TBN * norm);
    
    // Dùng norm cho lighting
    float diff = max(dot(norm, lightDir), 0.0);
    // ... lighting calculation
}
```

### **Recalculating Normals in Shaders**:

```glsl
// Tính approximate normal từ position
vec3 normal = normalize(cross(dFdx(fragPos), dFdy(fragPos)));

// Hoặc normalize per-vertex
vec3 normal = normalize(mat3(model) * vertexNormal);
```

---

## V. TRANSFORMATION & MATRIX OPERATIONS

### **Các phép toán ma trận phổ biến**:

```cpp
// 1. Dịch chuyển (Translation)
glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));

// 2. Quay (Rotation) - quanh trục Z
glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 0, 1));

// 3. Phóng to/Thu nhỏ (Scale)
glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, sz));

// 4. Kết hợp
glm::mat4 transform = trans * rot * scale;  // Áp dụng scale → rot → trans
```

**Lưu ý**: Thứ tự quan trọng! `A * B` khác `B * A`

### **Inverse & Transpose**:

```cpp
// Nghịch đảo (Inverse) - phục hồi transformation
glm::mat4 inv = glm::inverse(model);

// Chuyển vị (Transpose) - dùng cho normal transformation
glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
```

---

## VI. LIGHTING & SHADING

### **Phong Lighting Model**:

```glsl
// Ambient - ánh sáng môi trường
vec3 ambient = ambientStrength * lightColor;

// Diffuse - ánh sáng khuếch tán (phụ thuộc góc nhìn)
vec3 norm = normalize(normal);
vec3 lightDir = normalize(lightPos - fragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

// Specular - ánh sáng phản chiếu (sáng bóng)
vec3 viewDir = normalize(viewPos - fragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = specularStrength * spec * lightColor;

// Kết hợp
vec3 result = (ambient + diffuse + specular) * objectColor;
```

### **Normal Transformation**:

```glsl
// SAIIII:
vec3 normal = normalize(mat3(model) * normal);

// ĐÚNG (khi model có non-uniform scale):
vec3 normal = normalize(mat3(transpose(inverse(model))) * normal);
```

---

## VII. INSTANCING - RENDER INSTANCE RÁT NHIỀU OBJECT

**Mục đích**: Render 1000+ object cùng model nhưng vị trí/màu khác nhau.

### **Cách 1: Instanced Rendering (GPU)**

```glsl
// Vertex shader
layout(location = 4) in mat4 instanceModel;  // Per-instance model matrix
layout(location = 8) in vec3 instanceColor;  // Per-instance color

void main() {
    gl_Position = projection * view * instanceModel * vec4(position, 1.0);
    fragColor = instanceColor;
}
```

```cpp
// C++ code
glBindBuffer(GL_ARRAY_BUFFER, instanceMatrixVBO);
for (int i = 0; i < 4; i++) {
    glEnableVertexAttribArray(4 + i);
    glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), 
                         (void*)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(4 + i, 1);  // One per instance
}

// Draw
glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
```

**Lợi ích**:
- Giảm 90% draw calls
- GPU xử lý instancing, CPU chỉ submit 1 lệnh
- 180 cây trong project được render với 1 draw call

### **Cách 2: Vertex Attributes Divisor**

```cpp
// Setup instance data (per-instance scale, color, etc.)
std::vector<glm::vec3> instancePositions;
std::vector<float> instanceScales;

glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(glm::vec3), 
             instancePositions.data(), GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(5);
glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
glVertexAttribDivisor(5, 1);  // Per-instance

glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
glBufferData(GL_ARRAY_BUFFER, instanceScales.size() * sizeof(float), 
             instanceScales.data(), GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(6);
glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
glVertexAttribDivisor(6, 1);  // Per-instance
```

---

## VIII. DEPTH TEST & Z-FIGHTING

### **Depth Test**:
```cpp
glEnable(GL_DEPTH_TEST);           // Bật depth test
glDepthFunc(GL_LESS);               // Vẽ nếu z < zbuffer
glDepthMask(GL_TRUE);               // Ghi vào depth buffer
glClear(GL_DEPTH_BUFFER_BIT);       // Xóa depth buffer
```

### **Z-Fighting** (hai surface gần nhau, flickering):
```cpp
// Cách 1: Polygon Offset
glEnable(GL_POLYGON_OFFSET_FILL);
glPolygonOffset(1.0f, 1.0f);

// Cách 2: Tăng near/far plane separation
glm::perspective(45.0f, aspect, 0.01f, 1000.0f);  // More range
```

---

## IX. BLENDING (MIX COLORS)

```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Transparency

// Công thức: Final = SrcColor * SrcAlpha + DstColor * (1 - SrcAlpha)
```

---

## X. FIELD OF VIEW (FOV) & ASPECT RATIO

```cpp
// FOV impact
// FOV = 45° → zoom level bình thường
// FOV = 90° → rộng (wide angle)
// FOV = 20° → hẹp (telephoto)

glm::mat4 proj = glm::perspective(glm::radians(fov), width/height, near, far);

// Aspect ratio impact
// 16:9 (1.777) → sử dụng đúng tỷ lệ khung hình
// 1:1 (1.0) → vuông, stretch x-axis
```

---

## XI. PHÉP QUAY EULER vs QUATERNION

### **Euler Angles** (Yaw-Pitch-Roll):
```cpp
glm::vec3 eulerAngles(pitch, yaw, roll);
glm::quat q = glm::quat(eulerAngles);
glm::mat4 rotMatrix = glm::toMat4(q);

// Vấn đề: Gimbal lock (khi pitch = 90°)
```

### **Quaternion** (tốt hơn):
```cpp
glm::quat q = glm::angleAxis(glm::radians(angle), glm::vec3(0, 1, 0));
glm::mat4 rot = glm::toMat4(q);

// Lợi ích: Mịn hơn, không gimbal lock, dễ interpolate (slerp)
```

---

## TÓME TRONG PROJECT

| Khái niệm | File | Ví dụ |
|-----------|------|-------|
| Perspective | main.cpp:L193 | `glm::perspective(camera.Zoom, ...)` |
| View Matrix | Camera.h | `GetViewMatrix()` |
| Model Matrix | Vegetation.cpp:L345 | `glm::translate() * glm::rotate() * glm::scale()` |
| Instancing | Vegetation.cpp:L370+ | Instance matrix VBO + glVertexAttribDivisor |
| Lighting | vegetation.frag | Phong model (ambient + diffuse + specular) |
| Normal Transform | vegetation.vert | `mat3(model) * normal` |
| Depth Test | main.cpp:L88 | `glEnable(GL_DEPTH_TEST)` |
| Blending | main.cpp:L90 | `glBlendFunc(GL_SRC_ALPHA, ...)` |

---

## THAM KHẢO THÊM

- **GLM Math Library**: glm.g-truc.net
- **LearnOpenGL**: learnopengl.com
- **Khoa Graphics CSDN**: www.csdn.net
- **Computer Graphics by Foley & van Dam**: Sách kinh điển

