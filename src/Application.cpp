#include "Application.h"

Application::Application(Window *wnd_ptr)
    :   m_window(wnd_ptr) {
    Init();
}

Application::Application() {
    m_window = new Window();
    Init();
}

Application::~Application() {
    glfwTerminate();
}

void Application::Init() {
    m_cube = new Object("assets/objfiles/cube.obj");
    m_shader = new Shader("assets/shaders/basic.vs", "assets/shaders/basic.fs");
    m_camera = new Camera();

    // 創建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // 初始化 ImGui 的 GLFW 繫結
    ImGui_ImplGlfw_InitForOpenGL(m_window->getWindow(), true);

    // 初始化 ImGui 的 OpenGL3 繫結
    ImGui_ImplOpenGL3_Init("#version 450");

    // 建立字體圖集
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // 設置 ImGui 的風格（可選）
    ImGui::StyleColorsDark();
}

void Application::drawTest() {
    m_shader->bind();
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(m_camera->fov), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 1000.0f);
    view       = m_camera->getLookAt();
    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);
    glm::mat4 model = glm::mat4(1.0f);
    m_shader->setMat4("model", model);
    if(0){
        GLuint vao, vbo;
        std::vector<float> data;
        for(int i = 0; i < 40; ++i) {
            data.push_back(i); // x
            data.push_back(0); // y
            data.push_back(0); // z
            data.push_back(1); // r
            data.push_back(0); // g
            data.push_back(0); // b
        }
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, 40);
    }
    m_shader->unbind();
}

void Application::processCamera(float delta_time) {
    if(m_window->isPressed(GLFW_KEY_W)){
        m_camera->updateInput(FORWARD, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_S)){
        m_camera->updateInput(BACKWARD, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_A)){
        m_camera->updateInput(LEFT, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_D)){
        m_camera->updateInput(RIGHT, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_K)){
        m_camera->updateInput(RPITCH, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_J)){
        m_camera->updateInput(PITCH, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_L)){
        m_camera->updateInput(YAW, delta_time);
    }
    if(m_window->isPressed(GLFW_KEY_H)){
        m_camera->updateInput(RYAW, delta_time);
    }
}

void Application::processInput() {
    if(m_window->isPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(m_window->getWindow(), true);
    }
}

void Application::run() {
    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    // C++ 17 is needed
    std::vector<std::string> files;
    std::string path = "assets/Vector";
    int current_item = 0;
    int trace_step = 50;
    for(const auto & entry : std::filesystem::directory_iterator(path)) {
        std::string file = entry.path().string();
        // if operating system is windows
        #ifdef _WIN32
            std::size_t found = file.find_last_of("\\");
        #endif
        // if operating system is linux
        #ifdef __linux__
            std::size_t found = file.find_last_of("/");
        #endif
        file = file.substr(found + 1);
        if(file == "Images")    continue;
        files.push_back(file);
    }
    std::sort(files.begin(), files.end());

    char **items = new char*[files.size()];
    for(int i = 0; i < files.size(); ++i) {
        items[i] = new char[files[i].size() + 1];
        strcpy(items[i], files[i].c_str());
    }

    LIC lic("1.vec");
    m_camera->Position = glm::vec3(35, 35, 150.0f);
    while(!m_window->shouldClose()) {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // glViewport(0, 0, m_window->getWidth()/2, m_window->getHeight()/2);
        m_window->setClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        m_window->clear();

        processCamera(deltaTime);
        processInput();

        glViewport(0, 0, m_window->getWidth(), m_window->getHeight());
        {
            m_shader->bind();
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 projection = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(m_camera->fov), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 1000.0f);
            view       = m_camera->getLookAt();
            m_shader->setMat4("view", view);
            m_shader->setMat4("projection", projection);
            glm::mat4 model = glm::mat4(1.0f);
            m_shader->setMat4("model", model);
            lic.draw();
            m_shader->unbind();
        }

        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            {
                // create a imgui slider
                ImGui::Begin("LIC");
                ImGui::SliderInt("Filter K", &trace_step, 1, 50);
                // create a combo for select which file
                ImGui::Combo("Vector Field", &current_item, items, files.size());
                if(ImGui::Button("Load")) {
                    lic = LIC(files[current_item], trace_step);
                    float z = lic.resolution.y * 150.f / 70.f;
                    m_camera->Position = glm::vec3(lic.resolution.x / 2.0, lic.resolution.y / 2.0, z);
                }
                ImGui::End();
            }
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // drawTest();
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_window->swapBuffers();
    }
}