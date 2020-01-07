#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <pthread.h>
#include "x86_function.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int COORDS_NUM = 100;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main() {\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main() {\n"
"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

int initializeGLAD(){
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }
  return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void genVertexShader(unsigned *vertex_shader) {
  *vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(*vertex_shader, 1, &vertexShaderSource, NULL);
  glCompileShader(*vertex_shader);
  // Check if compilation was successful
  int  success;
  char infoLog[512];
  glGetShaderiv(*vertex_shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(*vertex_shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
  }
}

void genFragmentShader(unsigned *fragment_shader) {
  *fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(*fragment_shader, 1, &fragmentShaderSource, NULL);
  glCompileShader(*fragment_shader);
  // Check if compilation was successful
  int  success;
  char infoLog[512];
  glGetShaderiv(*fragment_shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(*fragment_shader, 512, NULL, infoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
  }
}

void compileShaderProgram(unsigned *shader_program, unsigned *vertex_shader, unsigned *fragment_shader) {
  *shader_program = glCreateProgram();
  // Link shaders with a program
  glAttachShader(*shader_program, *vertex_shader);
  glAttachShader(*shader_program, *fragment_shader);
  glLinkProgram(*shader_program);
  // Check if linking was successful
  int  success;
  char infoLog[512];
  glGetProgramiv(*shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*shader_program, 512, NULL, infoLog);
    printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
  }

}

void processInput(GLFWwindow *window){
  if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);
}

void setCursorPos(GLFWwindow* window, float *vertices) {
  double xpos, ypos;
  int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  for(;;){
    if (state == GLFW_PRESS) {
      glfwGetCursorPos(window, &xpos, &ypos);
      *vertices++ = (float) xpos;
      *vertices = SCR_HEIGHT - (float) ypos;
      return;
    }
  }
}

// Converts 2 coords of pixel array into normal device coordinates
void pix2NDC(float *arr, unsigned pointsNumber) {
  unsigned i = 0;
  for (; i < pointsNumber; ++i) {
    *arr = (*arr) * 2 / SCR_WIDTH - 1;
    ++arr;
    *arr =(*arr) * 2 / SCR_HEIGHT - 1;
    ++arr;
  }
}

int main() {
  // Set up GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
      "Bezier curve", NULL, NULL);
  if(window == NULL){
    printf("Can't create window\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if(initializeGLAD() == -1)
    return -1;

  glViewport(0,0, SCR_WIDTH, SCR_HEIGHT);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  float base_vertices[] = {
    50, 50,
    50, 500,
    400, 500,
    750, 500,
    500, 100
  };

  float vertices[COORDS_NUM] ;

  x86_function(vertices, base_vertices, COORDS_NUM);

  pix2NDC(vertices, COORDS_NUM / 2);

  // Create and compile shaders
  unsigned vertex_shader;
  genVertexShader(&vertex_shader);
  unsigned fragment_shader;
  genFragmentShader(&fragment_shader);

  // Create and compile shader program
  unsigned shader_program;
  compileShaderProgram(&shader_program, &vertex_shader, &fragment_shader);

  // Delete shaders, we don't need them anymore
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Bind the Vertex Array Object first, then bind and set vertex buffer(s),
  // and then configure vertex attributes(s).
  unsigned VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // Creates and initializes a buffer object's data store
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Define an array of generic vertex attribute data
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Note that this is allowed, the call to glVertexAttribPointer
  // registered VBO as the vertex attribute's bound vertex buffer
  // object so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't
  // accidentally modify this VAO, but this rarely happens. Modifying other
  // VAOs requires a call to glBindVertexArray anyways so we generally don't
  // unbind VAOs (nor VBOs) when it's not directly necessary.
  glBindVertexArray(0);

  //pthread_t callback_thr; // thread for inputs
  //float *vert_pointers[2];
  //vert_pointers[0] = vertices;
  //vert_pointers[1] = base_vertices;

  //pthread_create(&callback_thr, NULL, getInputVertices, vert_pointers);
  //pthread_join(callback_thr, NULL);

  // Render loop
  while(!glfwWindowShouldClose(window)) {

    // Read inputs
    processInput(window);

    // Clear colors
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw triangle
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_STRIP, 0,COORDS_NUM / 2);

    // Last things for render loop
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwTerminate();
  return 0;
}
