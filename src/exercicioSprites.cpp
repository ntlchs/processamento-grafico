#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <iostream>
#include <fstream>
#include <sstream>

float vertices[] = {
    // pos       // tex
    -0.5f, -0.5f, 0.0f, 0.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
     0.5f,  0.5f, 1.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 1.0f
};

unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

GLuint loadShader(const char* vertPath, const char* fragPath) {
    std::ifstream vFile(vertPath), fFile(fragPath);
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf(); fStream << fFile.rdbuf();
    std::string vCode = vStream.str(), fCode = fStream.str();
    const char* vSrc = vCode.c_str(), *fSrc = fCode.c_str();

    GLuint v = glCreateShader(GL_VERTEX_SHADER), f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(v, 1, &vSrc, nullptr); glCompileShader(v);
    glShaderSource(f, 1, &fSrc, nullptr); glCompileShader(f);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, f);
    glLinkProgram(prog); glDeleteShader(v); glDeleteShader(f);
    return prog;
}

GLuint loadTexture(const char* path) {
    GLuint tex;
    glGenTextures(1, &tex);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    GLenum format = (ch == 4) ? GL_RGBA : GL_RGB;

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return tex;
}

void drawSprite(GLuint shader, GLuint VAO, GLuint texture, glm::vec2 pos, glm::vec2 scale, float rot, glm::mat4 proj) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    model = glm::rotate(model, glm::radians(rot), glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(scale, 1.0f));

    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(800, 600, "Sprites", NULL, NULL);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);
    glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &EBO); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint shader = loadShader("shaders/vertex.glsl", "shaders/fragment.glsl");

    GLuint texFundo   = loadTexture("../assets/sprites/sprite1.png");
    GLuint texArvore  = loadTexture("../assets/sprites/sprite2.png");
    GLuint texPassaro = loadTexture("../assets/sprites/sprite3.png");

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    while (!glfwWindowShouldClose(win)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawSprite(shader, VAO, texFundo,   {400, 300}, {800, 600}, 0.0f, projection);
        drawSprite(shader, VAO, texArvore,  {400, 180}, {300, 200}, 0.0f, projection);
        drawSprite(shader, VAO, texPassaro, {250, 500}, {100, 100}, -10.0f, projection);
        drawSprite(shader, VAO, texPassaro, {550, 520}, {100, 100},  15.0f, projection);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
