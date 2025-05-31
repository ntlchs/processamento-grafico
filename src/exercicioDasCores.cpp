#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <cmath>

const int WIDTH = 800, HEIGHT = 600;
const int ROWS = 6, COLS = 8;
const float tolerance = 0.3f;
float dMax = sqrt(3.0f);
int score = 100;

struct Rectangle {
    glm::vec2 position;
    glm::vec3 color;
    bool removed;
};

Rectangle grid[ROWS][COLS];
GLuint shaderProgram;

const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
uniform mat4 projection;
uniform mat4 model;
void main()
{
    gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
}
)";

const GLchar *fragmentShaderSource = R"(
#version 400
uniform vec4 inputColor;
out vec4 color;
void main()
{
    color = inputColor;
}
)";

GLuint setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createRectangle() {
    float vertices[] = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    return vao;
}

float colorDistance(glm::vec3 c1, glm::vec3 c2) {
    return sqrt(pow(c1.r - c2.r, 2) + pow(c1.g - c2.g, 2) + pow(c1.b - c2.b, 2));
}

void eliminateSimilars(int x, int y) {
    glm::vec3 targetColor = grid[y][x].color;
    grid[y][x].removed = true;

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (!grid[i][j].removed && colorDistance(grid[i][j].color, targetColor) / dMax <= tolerance) {
                grid[i][j].removed = true;
                score -= 1;
            }
        }
    }
    std::cout << "Score: " << score << std::endl;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int col = xpos / (WIDTH / COLS);
        int row = ypos / (HEIGHT / ROWS);

        if (!grid[row][col].removed) {
            eliminateSimilars(col, row);
        }
    }
}

int main() {
    srand(time(0));

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo das cores", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    shaderProgram = setupShader();
    GLuint vao = createRectangle();

    glm::mat4 projection = glm::ortho(0.0f, float(WIDTH), float(HEIGHT), 0.0f, -1.0f, 1.0f);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            grid[i][j].position = glm::vec2((j + 0.5f) * (WIDTH / COLS), (i + 0.5f) * (HEIGHT / ROWS));
            grid[i][j].color = glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);
            grid[i][j].removed = false;
        }
    }

    while (!glfwWindowShouldClose(window)) {
        bool allRemoved = true;
        for (int i = 0; i < ROWS && allRemoved; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (!grid[i][j].removed) {
                    allRemoved = false;
                    break;
                }
            }
        }

        if (allRemoved) {
            std::cout << "Resetting game. Final Score: " << score << std::endl;
            score = 100;
            for (int i = 0; i < ROWS; ++i) {
                for (int j = 0; j < COLS; ++j) {
                    grid[i][j].position = glm::vec2((j + 0.5f) * (WIDTH / COLS), (i + 0.5f) * (HEIGHT / ROWS));
                    grid[i][j].color = glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);
                    grid[i][j].removed = false;
                }
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(vao);

        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (!grid[i][j].removed) {
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(grid[i][j].position, 0.0f));
                    model = glm::scale(model, glm::vec3(WIDTH / COLS, HEIGHT / ROWS, 1.0f));
                    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
                    glUniform4f(glGetUniformLocation(shaderProgram, "inputColor"), grid[i][j].color.r, grid[i][j].color.g, grid[i][j].color.b, 1.0f);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "Final Score: " << score << std::endl;
    glfwTerminate();
    return 0;
}
