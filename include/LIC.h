#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_set>
#include <utility>

/*
    method to write data to a bmp file: 
        https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
*/

class LIC {
public:
    LIC(std::string file, int trace_step = 50);
    void draw();
    glm::ivec2 resolution;
private:
    std::vector<unsigned char> image; // use this to store bmp image
    std::vector<float> data; // use this to store data for drawing on the window (OpenGL) [x, y, 0, r, g, b]
    std::vector<std::vector<glm::vec2>> vField; // store vector field data
    std::vector<std::vector<glm::vec2>> vFieldExpanded; // store expanded vector field data
    std::vector<std::vector<float>> noise;
    std::vector<std::vector<float>> lic;
    GLuint vao, vbo;
    glm::vec2 stepSize;
    int m_trace_step;
    void initGL();
    void loadFile(std::string file);
    void expandField();
    glm::vec2 interpolate(glm::vec2 pos);
    float trace(double x, double y);
    void generateNoise();
    void generateLIC();
    void writeData();
    void writeToFile(); // write to bmp file
    void RK2();
};