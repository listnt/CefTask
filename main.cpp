#include "src/app/app.hpp"
#include "src/backend/backend.hpp"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>

UserData *userData;
App *app;

bool initWindow() {

  EGLConfig config;
  EGLint num_config = 0;
  EGLint attribute_list[] = {EGL_RED_SIZE,  1, EGL_GREEN_SIZE, 1,
                             EGL_BLUE_SIZE, 1, EGL_NONE};
  EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

  if (WinCreate(userData, "test") != EGL_TRUE) {
    std::cerr << "Error creating window" << std::endl;
    return false;
  }

  userData->eglDisplay = eglGetDisplay(userData->eglNativeDisplay);
  eglInitialize(userData->eglDisplay, NULL, NULL);

  eglChooseConfig(userData->eglDisplay, attribute_list, &config, 1,
                  &num_config);

  userData->eglContext = eglCreateContext(userData->eglDisplay, config,
                                          EGL_NO_CONTEXT, contextAttribs);

  userData->eglSurface = eglCreateWindowSurface(
      userData->eglDisplay, config, userData->eglNativeWindow, NULL);

  eglMakeCurrent(userData->eglDisplay, userData->eglSurface,
                 userData->eglSurface, userData->eglContext);

  return true;
}

GLuint CompileShader(GLenum type, FILE *shaderFile) {
  fseek(shaderFile, 0, SEEK_END);
  long fsize = ftell(shaderFile);
  fseek(shaderFile, 0, SEEK_SET); /* same as rewind(f); */

  char *shaderSrc = (char *)malloc(fsize + 1);
  fread(shaderSrc, fsize, 1, shaderFile);
  fclose(shaderFile);

  shaderSrc[fsize] = '\0';

  // use the string, then ...

  // printf("[DEBUG] compiling shader: %d\nsource:\n%s\n", type, shaderSrc);
  GLint compiled;

  // Create the shader object
  GLuint shader = glCreateShader(type);

  if (shader == 0) {
    free(shaderSrc);
    return 0;
  }

  // Load the shader source
  glShaderSource(shader, 1, &shaderSrc, nullptr);

  // Compile the shader
  glCompileShader(shader);

  // Check the compile status
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (!compiled) {
    GLint infoLen = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {
      char *infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));

      glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
      printf("Error compiling shader:\n%s\n", infoLog);

      free(infoLog);
    }

    glDeleteShader(shader);
    free(shaderSrc);
    return 0;
  }

  free(shaderSrc);
  return shader;
}

int linkProgram(const GLuint programObject) {
  GLint linked;

  // Link the program
  glLinkProgram(programObject);

  // Check the link status
  glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

  if (!linked) {
    GLint infoLen = 0;

    glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1) {
      const auto infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));

      glGetProgramInfoLog(programObject, infoLen, nullptr, infoLog);
      printf("Error linking program:\n%s\n", infoLog);

      free(infoLog);
    }

    glDeleteProgram(programObject);
    return GL_FALSE;
  }

  return GL_TRUE;
}

GLuint initShaders(const char *vs, const char *fs) {
  FILE *vShaderFile = fopen(vs, "rb");
  FILE *fragmentShaderFile = fopen(fs, "rb");

  auto vertexShader = CompileShader(GL_VERTEX_SHADER, vShaderFile);
  auto fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderFile);
  if (vertexShader == 0 || fragmentShader == 0) {
    return 0;
  }

  // Create the program object
  const GLuint programObject = glCreateProgram();
  if (programObject == 0)
    return 0;
  glAttachShader(programObject, vertexShader);
  glAttachShader(programObject, fragmentShader);

  linkProgram(programObject);
  glUseProgram(programObject);

  return programObject;
}

void Draw() {
  while (userInterrupt(userData) == GL_FALSE) {
    CefDoMessageLoopWork();

    app->Render();
  }

  delete app;
}

int init(const unsigned int width, const unsigned int height) {
  std::cout << "starting OpenGL app" << std::endl;

  userData->WindowWidth = 640;
  userData->WindowHeight = 480;
  userData->keyFunc = NULL;

  if (!initWindow()) {
    return GL_FALSE;
  }

  const auto programObject = initShaders("./shaders/.vert", "./shaders/.frag");
  userData->programObject = programObject;

  glUseProgram(userData->programObject);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glEnable(GL_DEPTH_TEST); // GL_BLEND | GL_CULL_FACE

  app = new App();
  app->initScene();

  Draw();

  return GL_TRUE;
}

int main(int argc, char *argv[]) {
  userData = static_cast<UserData *>(malloc(sizeof(UserData)));

  userData->argc = argc;
  userData->argv = argv;

  std::cout << "start" << std::endl;
  CefMainArgs main_args(argc, argv);

  int exit_code = CefExecuteProcess(main_args, nullptr, nullptr);
  if (exit_code >= 0) {
    std::cout << "child process exited" << std::endl;
    return exit_code;
  }

  if (!init(640, 480)) {
    return EXIT_FAILURE;
  }

  std::cout << "end" << std::endl;

  return 1;
}