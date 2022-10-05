#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

// Константы
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float PI=3.14159;
const float points[]={
        // фигура
        3.19f,  5.64f,
       -2.76f,  5.46f,
        4.76f, -2.89f,
       -3.09f, -4.03f,
       // координатные оси
       -12.0f,   0.0f,
        12.0f,   0.0f,
         0.0f, -12.0f,
         0.0f,  12.0f

};
// цвета
const float ORANGE[]={1.0f, 0.5f, 0.2f, 1.0f};
const float GREEN[]={0.0f, 0.5f, 0.2f, 1.0f};
 // просто единичная матрица
  const float identity_matrix[]={
        1.0f,     0.0f,   0.0f,     0.0f,
        0.0f,     1.0f,   0.0f,     0.0f,
        0.0f,     0.0f,   1.0f,     0.0f,
        0.0f,     0.0f,   0.0f,     1.0f

};
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform mat4 tm;\n"
    "uniform mat4 rotate;\n"
    "uniform mat4 scale_matrix;\n"
    "uniform mat4 shift_matrix;\n"
    "uniform mat4 mirr_matrix;\n"
    "void main()\n"
    "{\n"
       "gl_Position =tm*mirr_matrix*scale_matrix*shift_matrix*rotate*vec4(aPos.x, aPos.y, 0, 1);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 col;\n"
    "void main()\n"
    "{\n"
    "   FragColor = col;\n"
    "}\n\0";



#endif // CONSTANTS_H_INCLUDED
