#version 450

// Atrybuty wierzchołków
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

// Atrybuty instancji (macierz modelu i dodatkowy atrybut)
layout(location = 3) in mat4 instanceModel;
layout(location = 7) in vec4 cubeColor; // np. kolor instancji lub inna informacja

// Uniform Buffer dla globalnych danych (widok, projekcja)
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

// Wyjścia do fragment shadera
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    // Transformacja pozycji wierzchołka: model -> view -> projection
    gl_Position = ubo.proj * ubo.view * instanceModel * vec4(inPosition, 1.0);

    // Przekazanie koloru i współrzędnych tekstury do fragment shadera
    fragColor = vec4(cubeColor.x, cubeColor.y, cubeColor.z, cubeColor.w); // Kolor instancji jako modulacja (opcjonalnie)
    fragTexCoord = inTexCoord;
}