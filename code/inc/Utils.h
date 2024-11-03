//
// Created by Radek on 28.10.2024.
//

#ifndef UTILS_H
#define UTILS_H
#include <fstream>
#include <vector>

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

glm::vec4 generateRandomColor() {

    float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // Losowa wartość dla czerwonego
    float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);// Losowa wartość dla zielonego
    float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); // Losowa wartość dla niebieskiego
    float a = 1.0f; // Pełna przezroczystość

    return glm::vec4(r, g, b, a); // Zwraca kolor w formacie RGBA
}

#endif //UTILS_H
