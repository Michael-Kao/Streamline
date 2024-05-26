#include "LIC.h"

LIC::LIC(std::string file, int trace_step) 
    : resolution(glm::vec2(0, 0)) , m_trace_step(trace_step){
    loadFile(file);
}

void LIC::loadFile(std::string file) {
    std::string path = "assets/Vector/" + file;
    std::ifstream f(path);
    if(!f.is_open()) {
        std::cout << "Failed to open file: " << file << std::endl;
        return;
    }
    else {
        std::string res_x, res_y;
        f >> res_x >> res_y;
        resolution.x = std::stoi(res_x);
        resolution.y = std::stoi(res_y);
        vField.resize(resolution.x, std::vector<glm::vec2>(resolution.y));
        for(int i = 0; i < resolution.x; ++i) {
            for(int j = 0; j < resolution.y; ++j) {
                f >> vField[i][j].x >> vField[i][j].y;
                // std::string x, y;
                // f >> x >> y;
                // vField[i][j].x = std::stof(x);
                // vField[i][j].y = std::stof(y);
            }
        }
        std::cout << "Resolution: " << resolution.x << " " << resolution.y << std::endl;
        std::cout << "Data size: " << vField.size() << "," << vField[0].size() << std::endl;
        f.close();

        expandField();
        generateNoise();
        generateLIC();
        initGL();
    }
}

void LIC::generateNoise() {
    noise.resize(vFieldExpanded.size(), std::vector<float>(vFieldExpanded[0].size()));
    srand(time(0));
    for(int i = 0; i < noise.size(); ++i) {
        for(int j = 0; j < noise[i].size(); ++j) {
            noise[i][j] = (rand() % 100) / 100.0f;
        }
    }
    std::cout << "Noise size: " << noise.size() << "," << noise[0].size() << std::endl;
    std::cout << "-------------\n";
}

void LIC::expandField() {
    int WIDTH = 700;
    int HEIGHT = 700;
    vFieldExpanded.resize(WIDTH, std::vector<glm::vec2>(HEIGHT));
    stepSize.x = (float)resolution.x / WIDTH;
    stepSize.y = (float)resolution.y / HEIGHT;
    for(int i = 0; i < WIDTH; ++i) {
        for(int j = 0; j < HEIGHT; ++j) {
            float x = i * stepSize.x;
            float y = j * stepSize.y;

            vFieldExpanded[i][j] = interpolate(glm::vec2(x, y));
        }
    }
}

glm::vec2 LIC::interpolate(glm::vec2 pos) {
    float dx = pos.x - (int)pos.x;
    float dy = pos.y - (int)pos.y;
    if((int)pos.x == resolution.x - 1 && (int)pos.y == resolution.y - 1) {
        return vField[(int)pos.x][(int)pos.y];
    }
    else if((int)pos.x == resolution.x - 1) {
        glm::vec2 v00 = vField[(int)pos.x][(int)pos.y];
        glm::vec2 v01 = vField[(int)pos.x][(int)pos.y + 1];
        return v00 + dy * (v01 - v00);
    }
    else if((int)pos.y == resolution.y - 1) {
        glm::vec2 v00 = vField[(int)pos.x][(int)pos.y];
        glm::vec2 v10 = vField[(int)pos.x + 1][(int)pos.y];
        return v00 + dx * (v10 - v00);
    }
    else {
        glm::vec2 v00 = vField[(int)pos.x][(int)pos.y];
        glm::vec2 v01 = vField[(int)pos.x][(int)pos.y + 1];
        glm::vec2 v10 = vField[(int)pos.x + 1][(int)pos.y];
        glm::vec2 v11 = vField[(int)pos.x + 1][(int)pos.y + 1];
        glm::vec2 x00 = v00 + dx * (v10 - v00);
        glm::vec2 x01 = v01 + dx * (v11 - v01);
        return x00 + dy * (x01 - x00);
    }
    return {};
}

void LIC::generateLIC() {
    lic.resize(vFieldExpanded.size(), std::vector<float>(vFieldExpanded[0].size()));
    for(int i = 0; i < lic.size(); ++i) {
        for(int j = 0; j < lic[i].size(); ++j) {
            float color = trace(i, j);
            lic[i][j] = color / (float)m_trace_step;
        }
    }

    writeData();
}

float LIC::trace(double x, double y) {
    int width = vFieldExpanded.size();
    int height = vFieldExpanded[0].size();
    int ix, iy;
    float noise_color = 0;
    double next_x, next_y;
    int step = m_trace_step;

    while(step >= 0)
    {
        ix = int(x + width ) % width;
        iy = int(y + height) % height;

        noise_color += noise[ix][iy];

        assert(ix >= 0 && iy >= 0);

        int n = 1;
        while(true) { 
            next_x = x + n * vFieldExpanded[ix][iy].x;
            next_y = y + n * vFieldExpanded[ix][iy].y;
            if(int(next_x + width) % width == ix && int(next_y + height) % height == iy) {
                n += 1;
            } else {
                break;
            }
        }


        x = next_x;
        y = next_y;

        step --;
    }

    return noise_color;
}

void LIC::writeData() {
    for(int i = 0; i < lic.size(); ++i) {
        for(int j = 0; j < lic[i].size(); ++j) {
            data.push_back(i * stepSize.x);
            data.push_back(j * stepSize.y);
            data.push_back(0);
            data.push_back(lic[i][j]);
            data.push_back(lic[i][j]);
            data.push_back(lic[i][j]);
        }
    }
}

void LIC::initGL() {
    assert(data.size() > 0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void LIC::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, data.size() / 6);
}

void LIC::RK2() {
    // float h = 0.1;
    // assert(vField.size() > 0);
    // std::vector<std::vector<bool>> visited(vField.size(), std::vector<bool>(vField[0].size(), false));
    // for(int i = 0; i < vField.size(); ++i) {
    //     for(int j = 0; j < vField[0].size(); ++j) {
    //         std::vector<glm::vec2> curLine;
    //         std::unordered_set<std::pair<int, int>> curLineVisited;
    //         int maxPoints = 1000;
    //         for(int k = 0; k < maxPoints; ++k) {
    //             if(visited[i][j])   break;
    //             curLineVisited.insert( { i, j } );

    //             // RK2
    //             glm::vec2 K1 = 
    //         }
    //     }
    // }
}