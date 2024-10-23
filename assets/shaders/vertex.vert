#version 450

layout(location = 0) in vec3 inPosition; // Pozycje wierzchołków
layout(location = 1) in vec2 texCoords;    // Kolor wierzchołków

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

void main() {
    // Przekształcenie pozycji wierzchołka
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
}