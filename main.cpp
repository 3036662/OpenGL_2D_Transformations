#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <iostream>

#include <constants.h>
#include <font_arial.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);


int main()
{

    // glfw: инициализация и конфигурирование
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfw: создание окна
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glad: загрузка всех указателей на OpenGL-функции
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    /***************************************************************/
    // создать буфер для точек (vertex bufer)
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*16, &points, GL_STATIC_DRAW);
    // Vertex Array Object
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    // аттрибуты данных:  индекс аттрибута=0, количество аттрибутов=2, тип аттрибута= double
    // необходимость нормализации = false, смещение между наборами аттрибутов =2, смещение до первого аттрибута=0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    /********************************************************************/
    // матрицы трансформаций
    // трансформируется только координата w
    float left{-12},right{12},bottom{-9},top{9};
    float scale{1};
    (right-left)>=(top-bottom) ? scale=(right-left)/2.0f : scale=(top-bottom)/2.0f;
    float projection_matrix[16];
    std::copy(std::begin(identity_matrix),std::end(identity_matrix),std::begin(projection_matrix));
    projection_matrix[15]=scale;

    // матрица поворота
    float rotation_matrix[16];
    std::copy(std::begin(identity_matrix),std::end(identity_matrix),std::begin(rotation_matrix));

    // матрица масштабирования
    float scale_matrix[16];
    std::copy(std::begin(identity_matrix),std::end(identity_matrix),std::begin(scale_matrix));

    // матррица сдвига
    float shift_matrix[16];
    std::copy(std::begin(identity_matrix),std::end(identity_matrix),std::begin(shift_matrix));

    // матрица отражения
    float mirror_matrix[16];
    std::copy(std::begin(identity_matrix),std::end(identity_matrix),std::begin(mirror_matrix));

    /********************************************************************/
    // шейдеры
    // вершинный шейдер
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // проверка на ошибки
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // фрагментный шейдер
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // проверка на ошибки
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // связать шейдеры в программу
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // проверка на ошибки
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    /********************************************************************/
    // создать интерфейс
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io=ImGui::GetIO();(void)io;
    //io.Fonts->AddFontFromFileTTF("fonts/arial.ttf",10,NULL,io.Fonts->GetGlyphRangesCyrillic());// добавить кириллицу
    io.Fonts->AddFontFromMemoryCompressedTTF(ArialFont_compressed_data,ArialFont_compressed_size,10.0f,NULL,io.Fonts->GetGlyphRangesCyrillic());
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // переменные хранят значения преобразований
    float rotate_degr{0.0f}; // угол поворота в градусах
    float user_scale{1.0f}; // коэф. масштабирования
    float user_shift_x{0.0f}; // сдвиг по х
    float user_shift_y{0.0f}; // сдвиг по y
    bool mirr_x{false},mirr_y{false}; // отражение по х и y
    // получить id uniform щейдера
    int rotateLoc=glGetUniformLocation(shaderProgram, "rotate");
    int scaleLoc=glGetUniformLocation(shaderProgram, "scale_matrix");
    int shiftLoc=glGetUniformLocation(shaderProgram, "shift_matrix");
    int mirrorLoc=glGetUniformLocation(shaderProgram, "mirr_matrix");
    // отправить матрицу проекции в вершинный шейдер
    int modelLoc = glGetUniformLocation(shaderProgram, "tm");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, projection_matrix);
    int colorLoc=glGetUniformLocation(shaderProgram,"col"); // матрица цвета для фрагментного щейдера

     /********************************************************************/
    // Главный цикл рендеринга
    while (!glfwWindowShouldClose(window))
    {
        // Обработка ввода
        processInput(window);
		// Выполнение рендеринга
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        //gui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Трансформации");
        ImGui::SliderFloat("Повернуть", &rotate_degr, 0.0f, 360.0f);
        ImGui::SliderFloat("Масштаб", &user_scale, 0.0f, 3.0f);
        ImGui::SliderFloat("Сдвиг_x",&user_shift_x,-5.0f,5.0f);
        ImGui::SliderFloat("Сдвиг_y",&user_shift_y,-5.0f,5.0f);
        ImGui::Checkbox("Отражение относ y",&mirr_y);
        ImGui::Checkbox("Отражение относ x",&mirr_x);
        (mirr_y) ? mirror_matrix[0]=-1 : mirror_matrix[0]=1;
        (mirr_x) ? mirror_matrix[5]=-1 : mirror_matrix[5]=1;
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glUseProgram(shaderProgram);
        glUniform4fv(colorLoc, 1, ORANGE);
        // поворот
        rotation_matrix[0]=cos(rotate_degr*PI/180);
        rotation_matrix[5]=rotation_matrix[0];
        rotation_matrix[4]=sin(rotate_degr*PI/180);
        rotation_matrix[1]=-rotation_matrix[4];
        // масштабирование
        scale_matrix[0]=user_scale;
        scale_matrix[5]=user_scale;
        // сдвиг
        shift_matrix[12]=user_shift_x;
        shift_matrix[13]=user_shift_y;
        // передать матрицы в щейдер
        glUniformMatrix4fv(rotateLoc,1,GL_FALSE,rotation_matrix);
        glUniformMatrix4fv(scaleLoc,1,GL_FALSE,scale_matrix);
        glUniformMatrix4fv(shiftLoc,1,GL_FALSE,shift_matrix);
        glUniformMatrix4fv(mirrorLoc,1,GL_FALSE,mirror_matrix);
        //нарисовать фигуру из массива точек
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        // координатные оси
        // пользователь не может влиять на оси, поэтому шейдеру передаются единичные матрицы
        // можно было бы использовать другой шейдер для  оптимизации
        glUniformMatrix4fv(rotateLoc,1,GL_FALSE,identity_matrix);
        glUniformMatrix4fv(scaleLoc,1,GL_FALSE,identity_matrix);
        glUniformMatrix4fv(shiftLoc,1,GL_FALSE,identity_matrix);
        glUniform4fv(colorLoc, 1, GREEN);
        glDrawArrays(GL_LINES, 4, 4);
        // glfw: обмен содержимым front- и back-буферов. Отслеживание событий ввода/вывода
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // завершить выполнени gui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: завершение, освобождение всех ранее задействованных GLFW-ресурсов
    glfwTerminate();
    return 0;
}


// Обработка всех событий ввода: запрос GLFW о нажатии/отпускании клавиш на клавиатуре в текущем кадре и соответствующая обработка данных событий
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: всякий раз, когда изменяются размеры окна (пользователем или операционной системой), вызывается данная callback-функция
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // Убеждаемся, что окно просмотра соответствует новым размерам окна.
    //std::cout<<"Window changed"<<std::endl;
    glViewport(0, 0, width, height);
}
