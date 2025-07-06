#include "configs.h"        // Globals variables, includes and structs
#include "lab_functions.h"  // Functions defined in the labs

void LoadTextures();  // Function to Load the texture from images
void DrawObject(glm::mat4 model, const char* name,
                int id);  // Function to draw the object in the screen
void sceneObjects(glm::mat4 view, glm::mat4 projection, glm::mat4 T_view,
                  GLuint orangePortalTexture);

#define SPHERE 0
#define BUNNY 1
#define WALL 2
#define PORTALGUN 3
#define FLOOR 4
#define BLUE_PORTAL 10
#define ORANGE_PORTAL 11

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

int main(int argc, char* argv[]) {
  int success = glfwInit();
  if (!success) {
    fprintf(stderr, "ERROR: glfwInit() failed.\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetErrorCallback(ErrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window;
  window =
      glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                       "INF01047 - Trabalho Final Rayan e Gabriel Henrique",
                       glfwGetPrimaryMonitor(), NULL);
  if (!window) {
    glfwTerminate();
    fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
    std::exit(EXIT_FAILURE);
  }

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetKeyCallback(window, KeyCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetCursorPosCallback(window, CursorPosCallback);
  glfwSetScrollCallback(window, ScrollCallback);

  glfwMakeContextCurrent(window);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
  FramebufferSizeCallback(window, SCREEN_WIDTH, SCREEN_HEIGHT);

  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* glversion = glGetString(GL_VERSION);
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  // Framebuffer for the view from blue portal / orange portal texture
  // Tutorial from LearnOpenGL
  GLuint bluePortalViewFramebuffer;
  glGenFramebuffers(1, &bluePortalViewFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, bluePortalViewFramebuffer);

  GLuint orangePortalTexture;
  glGenTextures(1, &orangePortalTexture);
  glBindTexture(GL_TEXTURE_2D, orangePortalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         orangePortalTexture, 0);

  GLuint bluePortalViewRBO;
  glGenRenderbuffers(1, &bluePortalViewRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, bluePortalViewRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH,
                        SCREEN_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, bluePortalViewRBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("blue framebuffer error");
    return -1;
  }

  // Framebuffer for the view from orange portal / blue portal texture
  // Tutorial from LearnOpenGL
  GLuint orangePortalViewFramebuffer;
  glGenFramebuffers(1, &orangePortalViewFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, orangePortalViewFramebuffer);

  GLuint bluePortalTexture;
  glGenTextures(1, &bluePortalTexture);
  glBindTexture(GL_TEXTURE_2D, bluePortalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         bluePortalTexture, 0);

  GLuint orangePortalViewRBO;
  glGenRenderbuffers(1, &orangePortalViewRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, orangePortalViewRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH,
                        SCREEN_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, orangePortalViewRBO);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("orange framebuffer error");
    return -1;
  }

  printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion,
         glslversion);

  LoadShadersFromFiles();

  LoadTextures();

  ObjModel spheremodel("../../data/sphere.obj");
  ComputeNormals(&spheremodel);
  BuildTrianglesAndAddToVirtualScene(&spheremodel);

  ObjModel bunnymodel("../../data/bunny.obj");
  ComputeNormals(&bunnymodel);
  BuildTrianglesAndAddToVirtualScene(&bunnymodel);

  ObjModel planemodel("../../data/plane.obj");
  ComputeNormals(&planemodel);
  BuildTrianglesAndAddToVirtualScene(&planemodel);

  ObjModel gunmodel("../../textures/portalgun/portalgun.obj");
  BuildTrianglesAndAddToVirtualScene(&gunmodel);

  ObjModel wallmodel("../../data/wall.obj");
  BuildTrianglesAndAddToVirtualScene(&wallmodel);

  if (argc > 1) {
    ObjModel model(argv[1]);
    BuildTrianglesAndAddToVirtualScene(&model);
  }

  TextRendering_Init();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glm::vec4 camera_position_c = glm::vec4(0.0f, 2.5f, 5.0f, 1.0f);
  float speed = 5.5f;
  float prev_time = (float)glfwGetTime();

  glm::vec4 bluePortalPosition = glm::vec4(5.0f, 2.0f, 0.0f, 1.0f);
  bool isBluePortalActive = true;

  glm::vec4 orangePortalPosition = glm::vec4(0.0f, 2.0f, -5.0f, 1.0f);
  bool isOrangePortalActive = true;

  while (!glfwWindowShouldClose(window)) {
    glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    if (isBluePortalActive && isOrangePortalActive) {
      // BLUE PORTAL VIEW, APPEARS ON ORANGE PORTAL
      glBindFramebuffer(GL_FRAMEBUFFER, bluePortalViewFramebuffer);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(g_GpuProgramID);

      glm::vec4 blue_portal_looking_at = glm::vec4(0.0f, 2.0f, 0.0f, 1.0f);
      glm::vec4 camera_view_vector =
          blue_portal_looking_at - bluePortalPosition;

      glm::vec4 camera_w_vector =
          -(camera_view_vector / norm(camera_view_vector));
      camera_w_vector.y = 0;

      glm::mat4 view = Matrix_Camera_View(bluePortalPosition,
                                          camera_view_vector, camera_up_vector);
      glm::mat4 T_view = T_Matrix_Camera_View(
          bluePortalPosition, camera_view_vector, camera_up_vector);

      glm::mat4 projection;

      float nearplane = -0.1f;
      float farplane = -70.0f;

      if (g_UsePerspectiveProjection) {
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane,
                                        farplane);
      } else {
        float t = 1.5f * g_CameraDistance / 2.5f;
        float b = -t;
        float r = t * g_ScreenRatio;
        float l = -r;
        projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
      }

      glm::mat4 model = Matrix_Identity();

      glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE,
                         glm::value_ptr(projection));

      sceneObjects(view, projection, T_view, orangePortalTexture);


      // BLUE PORTAL VIEW, APPEARS ON ORANGE PORTAL
      glBindFramebuffer(GL_FRAMEBUFFER, orangePortalViewFramebuffer);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(g_GpuProgramID);

      glm::vec4 orange_portal_looking_at = glm::vec4(0.0f, 2.0f, 0.0f, 1.0f);
      camera_view_vector =
          orange_portal_looking_at - orangePortalPosition;

      camera_w_vector =
          -(camera_view_vector / norm(camera_view_vector));
      camera_w_vector.y = 0;

      view = Matrix_Camera_View(orangePortalPosition,
                                          camera_view_vector, camera_up_vector);
      T_view = T_Matrix_Camera_View(
          orangePortalPosition, camera_view_vector, camera_up_vector);

      if (g_UsePerspectiveProjection) {
        float field_of_view = 3.141592 / 3.0f;
        projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane,
                                        farplane);
      } else {
        float t = 1.5f * g_CameraDistance / 2.5f;
        float b = -t;
        float r = t * g_ScreenRatio;
        float l = -r;
        projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
      }

      model = Matrix_Identity();

      glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE,
                         glm::value_ptr(projection));

      sceneObjects(view, projection, T_view, orangePortalTexture);
    } else {
      // Texturas dos portais nulas
    }

    //// ORIGINAL :
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_GpuProgramID);

    float r = g_CameraDistance;
    float y = r * sin(g_CameraPhi);
    float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
    float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);

    glm::vec4 camera_view_vector = glm::vec4(x, y, z, 0.0f);

    float current_time = (float)glfwGetTime();
    float delta_t = current_time - prev_time;
    prev_time = current_time;

    glm::vec4 camera_w_vector =
        -(camera_view_vector / norm(camera_view_vector));
    camera_w_vector.y = 0;

    if (g_KeyW_Pressed) {
      camera_position_c += -camera_w_vector * speed * delta_t;
    }
    if (g_KeyS_Pressed) {
      camera_position_c += camera_w_vector * speed * delta_t;
    }
    if (g_KeyD_Pressed) {
      glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
      camera_position_c +=
          (upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
    }
    if (g_KeyA_Pressed) {
      glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
      camera_position_c +=
          -(upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
    }

    glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector,
                                        camera_up_vector);
    glm::mat4 T_view = T_Matrix_Camera_View(
        camera_position_c, camera_view_vector, camera_up_vector);

    glm::mat4 projection;

    float nearplane = -0.5f;
    float farplane = -85.0f;

    if (g_UsePerspectiveProjection) {
      float field_of_view = 3.141592 / 3.0f;
      projection =
          Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
    } else {
      float t = 1.5f * g_CameraDistance / 2.5f;
      float b = -t;
      float r = t * g_ScreenRatio;
      float l = -r;
      projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
    }

    sceneObjects(view, projection, T_view, orangePortalTexture);

    glm::mat4 model = Matrix_Identity();

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, bluePortalTexture);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "BluePortalTexture"), 10);

    model = Matrix_Translate(5.0f, 2.5f, 0.0f) * Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(2.5f, 2.5f, 1.0f);
    DrawObject(model, "the_portal", BLUE_PORTAL);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, orangePortalTexture);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "OrangePortalTexture"), 11);

    model = Matrix_Translate(0.0f, 2.5f, -5.0f) * Matrix_Scale(2.5f, 2.5f, 1.0f);
    DrawObject(model, "the_portal", ORANGE_PORTAL);

    // Drawing the portal gun
    model = T_view * Matrix_Translate(1, -1, -2) * Matrix_Scale(1, 1, 1);
    DrawObject(model, "PortalGun", PORTALGUN);

    TextRendering_ShowFramesPerSecond(window);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

void sceneObjects(glm::mat4 view, glm::mat4 projection, glm::mat4 T_view,
                  GLuint orangePortalTexture) {
  glm::mat4 model = Matrix_Identity();

  glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE,
                     glm::value_ptr(projection));

  // Drawing the sphere
  model = Matrix_Translate(-1.0f, 2.0f, 0.0f) * Matrix_Rotate_Z(0.6f) *
          Matrix_Rotate_X(0.2f) *
          Matrix_Rotate_Y(g_AngleY + (float)glfwGetTime() * 0.1f);
  DrawObject(model, "the_sphere", SPHERE);

  // Drawing the bunny
  model = Matrix_Translate(1.0f, 2.0f, 0.0f) *
          Matrix_Rotate_X(g_AngleX + (float)glfwGetTime() * 0.1f);
  DrawObject(model, "the_bunny", BUNNY);

  // Higher walls
  model =
      Matrix_Translate(-30.0f, 0.0f, -30.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0f, 0.0f, 30.0f) *
          Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, 0.0f, -30.0f) *
          Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-3.141592f) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  // Floors
  model = Matrix_Translate(-30.0, 0.0f, -10.0f) *
          Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0, -20.0f, 10.0f) *
          Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0, 0.0f, 30.0f) *
          Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  // Lower walls

  model = Matrix_Translate(-30.0f, -20.0f, -10.0f) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0f, -20.0f, 30.0f) *
          Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, -20.0f, -30.0f) *
          Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, -20.0f, 10.0f) * Matrix_Rotate_Y(-3.141592f) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);
}

void LoadTextures() {
  LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");
  LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif");
  LoadTextureImage("../../data/portalwall.png");
  LoadTextureImage("../../textures/portalgun/textures/portalgun_col.jpg");
}

void DrawObject(glm::mat4 model, const char* name, int id) {
  glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));

  glUniform1i(g_object_id_uniform, id);

  DrawVirtualObject(name);
}
