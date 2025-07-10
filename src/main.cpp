#include "collision.h"      // Collision functions
#include "configs.h"        // Globals variables, includes and structs
#include "lab_functions.h"  // Functions defined in the labs

void LoadTextures();  // Function to Load the texture from images
void DrawObject(glm::mat4 model, const char* name,
                int id);  // Function to draw the object in the screen
void sceneObjects(glm::mat4 view, glm::mat4 projection,
                  glm::mat4 T_view);  // Function to draw the map
                                      // without the portals
void UpdatePortalPosition(glm::vec4 colision_point, glm::vec4 surface_normal,
                          int portal_color);
void MovePlayerToPortal(glm::vec4* camera, glm::mat4 portal_transform,
                        int portal_color);
glm::vec4 calculateBezierCurve(float t, glm::vec4 point_a, glm::vec4 point_b,
                               glm::vec4 point_c, glm::vec4 point_d);

#define SPHERE 0
#define BUNNY 1
#define WALL 2
#define PORTALGUN 3
#define FLOOR 4
#define PLAYER 5
#define CUBE 6
#define CROSSHAIR 7
#define LEVEL_COMPLETE 8
#define BLUE_PORTAL 10
#define ORANGE_PORTAL 11
#define GOURAUD_SHADING_RED 20
#define GOURAUD_SHADING_GREEN 21
#define DOOR 30
#define BUTTON 31
#define PLATFORM 32
#define BOX 33

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

glm::vec4 bluePortalPosition = glm::vec4(1000.0f, 1000.0f, 1000.0f, 1000.0f);
glm::mat4 bluePortalRotation = Matrix_Rotate_Y(-M_PI_2);
glm::vec4 bluePortalLooksAt = glm::vec4(0.0f, 2.0f, 0.0f, 1.0f);
int bluePortalSeesDirection = SOUTH;
bool isBluePortalActive = false;

glm::vec4 orangePortalPosition = glm::vec4(1000.0f, 1000.0f, 1000.0f, 1.0f);
glm::mat4 orangePortalRotation = Matrix_Identity();
glm::vec4 orangePortalLooksAt = glm::vec4(0.0f, 2.0f, 0.0f, 1.0f);
int orangePortalSeesDirection = WEST;
bool isOrangePortalActive = false;

glm::vec4 boxPosition = glm::vec4(0.0f, 14.1f, 0.75f, 1.0f);
bool isFloorButtonPressed = false;

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
  window = glfwCreateWindow(
      1920, 1080, "INF01047 - Trabalho Final Rayan e Gabriel Henrique",
      glfwGetPrimaryMonitor(), NULL);

  if (!window) {
    glfwTerminate();
    fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
    std::exit(EXIT_FAILURE);
  }

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetKeyCallback(window, KeyCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetCursorPosCallback(window, CursorPosCallback);
  glfwSetScrollCallback(window, ScrollCallback);

  glfwMakeContextCurrent(window);

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
  FramebufferSizeCallback(window, width, height);

  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* glversion = glGetString(GL_VERSION);
  const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  // Framebuffer for the view from blue portal / orange portal texture
  // Based on a tutorial from LearnOpenGL (https://learnopengl.com/Advanced-OpenGL/Framebuffers)
  GLuint bluePortalViewFramebuffer;
  glGenFramebuffers(1, &bluePortalViewFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, bluePortalViewFramebuffer);

  GLuint orangePortalTexture;
  glGenTextures(1, &orangePortalTexture);
  glBindTexture(GL_TEXTURE_2D, orangePortalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         orangePortalTexture, 0);

  GLuint bluePortalViewRBO;
  glGenRenderbuffers(1, &bluePortalViewRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, bluePortalViewRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         bluePortalTexture, 0);

  GLuint orangePortalViewRBO;
  glGenRenderbuffers(1, &orangePortalViewRBO);
  glBindRenderbuffer(GL_RENDERBUFFER, orangePortalViewRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
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
  ComputeNormals(&gunmodel);
  BuildTrianglesAndAddToVirtualScene(&gunmodel);

  ObjModel cubemodel("../../data/the_cube.obj");
  ComputeNormals(&cubemodel);
  BuildTrianglesAndAddToVirtualScene(&cubemodel);

  ObjModel wallmodel("../../data/wall.obj");
  ComputeNormals(&wallmodel);
  BuildTrianglesAndAddToVirtualScene(&wallmodel);
  SetWallsInfo();

  ObjModel doormodel("../../data/door/EDITOR_door.obj");
  BuildTrianglesAndAddToVirtualScene(&doormodel);

  ObjModel doormodelopen("../../data/door/EDITOR_door_open.obj");
  BuildTrianglesAndAddToVirtualScene(&doormodelopen);

  ObjModel buttonmodel("../../data/Button/button.obj");
  BuildTrianglesAndAddToVirtualScene(&buttonmodel);

  ObjModel platformmodel("../../data/Platform/light_rail_platform.obj");
  BuildTrianglesAndAddToVirtualScene(&platformmodel);

  ObjModel boxmodel("../../data/Weighted Cube/weightedcube.obj");
  BuildTrianglesAndAddToVirtualScene(&boxmodel);

  if (argc > 1) {
    ObjModel model(argv[1]);
    BuildTrianglesAndAddToVirtualScene(&model);
  }

  TextRendering_Init();

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glm::vec4 camera_position_c = glm::vec4(0.0f, 2.5f, 25.0f, 1.0f);
  float speed = 5.5f;
  float prev_time = (float)glfwGetTime();

  while (!glfwWindowShouldClose(window)) {
    glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    float nearplane = -0.5f;
    float farplane = -85.0f;

    float current_time = (float)glfwGetTime();
    float delta_t = current_time - prev_time;
    prev_time = current_time;

    // Defining the points for the moving platform's Bézier curve
    glm::vec4 pointA = glm::vec4(27.75f, -0.2f, -9.0f, 1.0f);
    glm::vec4 pointB = glm::vec4(5.91f, 5.79f, -8.95f, 1.0f);
    glm::vec4 pointC = glm::vec4(21.79f, 10.36f, -4.79f, 1.0f);
    glm::vec4 pointD = glm::vec4(0.0f, 13.0f, -3.5f, 1.0f);

    glm::vec4 bezierPoint = calculateBezierCurve(
        (sin(0.2 * glfwGetTime()) + 1) / 2, pointA, pointB, pointC, pointD);

    // Moving platform model
    g_bezierModel =
        Matrix_Translate(bezierPoint.x, bezierPoint.y, bezierPoint.z) *
        Matrix_Scale(0.025f, 0.025f, 0.025f);

    BoundingBox platform = GetBoundingBoxObject("platform");
    
    // Platform position change since last frame
    g_MovingPlatformDelta =
        glm::vec4((g_MovingPlatform.max.x + g_MovingPlatform.min.x) / 2.0f,
                  (g_MovingPlatform.max.y + g_MovingPlatform.min.y) / 2.0f,
                  (g_MovingPlatform.max.z + g_MovingPlatform.min.z) / 2.0f, 0);
    g_MovingPlatform = {g_bezierModel * platform.min,
                        g_bezierModel * platform.max};
    g_MovingPlatformDelta =
        glm::vec4((g_MovingPlatform.max.x + g_MovingPlatform.min.x) / 2.0f -
                      g_MovingPlatformDelta.x,
                  (g_MovingPlatform.max.y + g_MovingPlatform.min.y) / 2.0f -
                      g_MovingPlatformDelta.y,
                  (g_MovingPlatform.max.z + g_MovingPlatform.min.z) / 2.0f -
                      g_MovingPlatformDelta.z,
                  0);

    if (isBluePortalActive && isOrangePortalActive) {
      // DRAWING THE BLUE PORTAL VIEW, APPEARS ON ORANGE PORTAL
      glBindFramebuffer(GL_FRAMEBUFFER, bluePortalViewFramebuffer);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(g_GpuProgramID);

      // Portal's look-at camera
      glm::vec4 camera_view_vector = bluePortalLooksAt - bluePortalPosition;
      glm::vec4 camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

      glm::mat4 view = Matrix_Camera_View(bluePortalPosition,
                                          camera_view_vector, camera_up_vector);
      glm::mat4 T_view = T_Matrix_Camera_View(
          bluePortalPosition, camera_view_vector, camera_up_vector);

      glm::mat4 projection;

      if (g_UsePerspectiveProjection) {
        float field_of_view = M_PI / 3.0f;
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

      sceneObjects(view, projection, T_view);

      glActiveTexture(GL_TEXTURE10);
      glBindTexture(GL_TEXTURE_2D, bluePortalTexture);
      glUniform1i(glGetUniformLocation(g_GpuProgramID, "BluePortalTexture"),
                  10);

      glm::mat4 modelOrangePortal =
          Matrix_Translate(orangePortalPosition.x, orangePortalPosition.y,
                           orangePortalPosition.z) *
          orangePortalRotation * Matrix_Scale(2.5f, 2.5f, 1.0f);
      DrawObject(modelOrangePortal, "the_portal", ORANGE_PORTAL);

      model = Matrix_Translate(camera_position_c.x, camera_position_c.y,
                               camera_position_c.z) *
              Matrix_Rotate_Y(g_CameraTheta + M_PI_2);
      DrawObject(model, "the_bunny", BUNNY);

      // DRAWING THE BLUE PORTAL VIEW, APPEARS ON ORANGE PORTAL
      glBindFramebuffer(GL_FRAMEBUFFER, orangePortalViewFramebuffer);
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glUseProgram(g_GpuProgramID);

      // Portal's look-at camera
      camera_view_vector = orangePortalLooksAt - orangePortalPosition;

      view = Matrix_Camera_View(orangePortalPosition, camera_view_vector,
                                camera_up_vector);
      T_view = T_Matrix_Camera_View(orangePortalPosition, camera_view_vector,
                                    camera_up_vector);

      if (g_UsePerspectiveProjection) {
        float field_of_view = M_PI / 3.0f;
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

      sceneObjects(view, projection, T_view);

      glActiveTexture(GL_TEXTURE10);
      glBindTexture(GL_TEXTURE_2D, bluePortalTexture);
      glUniform1i(glGetUniformLocation(g_GpuProgramID, "BluePortalTexture"),
                  10);

      glm::mat4 modelBluePortal =
          Matrix_Translate(bluePortalPosition.x, bluePortalPosition.y,
                           bluePortalPosition.z) *
          bluePortalRotation * Matrix_Scale(2.5f, 2.5f, 1.0f);
      DrawObject(modelBluePortal, "the_portal", BLUE_PORTAL);

      glActiveTexture(GL_TEXTURE11);
      glBindTexture(GL_TEXTURE_2D, orangePortalTexture);
      glUniform1i(glGetUniformLocation(g_GpuProgramID, "OrangePortalTexture"),
                  11);

      model = Matrix_Translate(camera_position_c.x, camera_position_c.y,
                               camera_position_c.z) *
              Matrix_Rotate_Y(g_CameraTheta + M_PI_2);
      DrawObject(model, "the_bunny", BUNNY);
    }

    // DRAWING THE PLAYER'S VIEW
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_GpuProgramID);

    // Player free camera definition
    float r = g_CameraDistance;
    float y = r * sin(g_CameraPhi);
    float z = r * cos(g_CameraPhi) * cos(g_CameraTheta);
    float x = r * cos(g_CameraPhi) * sin(g_CameraTheta);

    glm::vec4 camera_view_vector = glm::vec4(x, y, z, 0.0f);
    //    glm::vec4
    camera_up_vector = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    glm::vec4 camera_w_vector =
        -(camera_view_vector / norm(camera_view_vector));
    camera_w_vector.y = 0;

    //  -=-=-=-=-=-=-=-=-=-=-=-=-=- USER INPUT HANDLING
    //  -=-=-=-=-=-=-=-=-=-=-=-=-=-
    glm::vec4 cam_temp = camera_position_c;

    if (g_KeyW_Pressed) {
      cam_temp = camera_position_c;
      cam_temp += -camera_w_vector * speed * delta_t;
      if (!CheckCollisionPointWalls(cam_temp)) {
        camera_position_c = cam_temp;
      }
    }
    if (g_KeyS_Pressed) {
      cam_temp = camera_position_c;
      cam_temp += camera_w_vector * speed * delta_t;
      CheckCollisionPointWalls(cam_temp);
      if (!CheckCollisionPointWalls(cam_temp)) {
        camera_position_c = cam_temp;
      }
    }
    if (g_KeyD_Pressed) {
      cam_temp = camera_position_c;
      glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
      cam_temp += (upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
      CheckCollisionPointWalls(cam_temp);
      if (!CheckCollisionPointWalls(cam_temp)) {
        camera_position_c = cam_temp;
      }
    }
    if (g_KeyA_Pressed) {
      cam_temp = camera_position_c;
      glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
      cam_temp += -(upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
      CheckCollisionPointWalls(cam_temp);
      if (!CheckCollisionPointWalls(cam_temp)) {
        camera_position_c = cam_temp;
      }
    }

    if (g_LeftMouseButtonPressed && !g_HoldingBox) {
      std::pair<glm::vec4, glm::vec4> collisionResult =
          CheckCollisionLineToWalls(camera_position_c, camera_view_vector);

      UpdatePortalPosition(collisionResult.first, collisionResult.second,
                           BLUE_PORTAL);
      isBluePortalActive = true;
    }

    if (g_RightMouseButtonPressed && !g_HoldingBox) {
      std::pair<glm::vec4, glm::vec4> collisionResult =
          CheckCollisionLineToWalls(camera_position_c, camera_view_vector);

      UpdatePortalPosition(collisionResult.first, collisionResult.second,
                           ORANGE_PORTAL);

      isOrangePortalActive = true;
    }

    BoundingBox player = GetBoundingBoxObject("the_bunny");
    glm::mat4 modelPlayer =
        Matrix_Translate(camera_position_c.x, camera_position_c.y,
                         camera_position_c.z) *
        Matrix_Rotate_Y(g_CameraTheta + M_PI_2);

    player = {modelPlayer * player.min, modelPlayer * player.max};

    if (CheckCollisionAABBtoAABB(player, g_MovingPlatform)) {
      camera_position_c = camera_position_c + g_MovingPlatformDelta;
      g_OnMovingPlatform = true;
      g_PlayerOnGround = true;
    } else {
      g_OnMovingPlatform = false;
      g_PlayerOnGround = false;
    }

    BoundingBox fixed_platform = GetBoundingBoxObject("platform");
    glm::mat4 platform_trasform = Matrix_Translate(0.0f, 13.0f, 0.0f) *
                                  Matrix_Scale(0.025f, 0.025f, 0.025f);
    fixed_platform = {platform_trasform * fixed_platform.min,
                      platform_trasform * fixed_platform.max};

    player.min.y = player.min.y - speed * delta_t;
    g_PlayerOnGround = CheckCollisionPlayerFloor(player) ||
                       CheckCollisionAABBtoAABB(player, fixed_platform) ||
                       g_OnMovingPlatform;
    if (!g_PlayerOnGround && !g_Jumping && !g_OnMovingPlatform) {
      camera_position_c.y = camera_position_c.y - speed * delta_t;
      g_PlayerOnGround = false;
    }

    if (g_Space_Pressed && !g_Jumping && g_PlayerOnGround) {
      g_Jumping = true;
      g_Jump_Time = 0.6;
    }

    if (g_Jump_Time > 0.1) {
      camera_position_c =
          camera_position_c + glm::vec4(0, speed * delta_t, 0, 0);
      g_Jump_Time = g_Jump_Time - delta_t;
    } else {
      if (g_Jump_Time > 0.0) {
        g_Jump_Time = g_Jump_Time - delta_t;
      } else {
        g_Jumping = false;
      }
    }

    //  -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector,
                                        camera_up_vector);
    glm::mat4 T_view = T_Matrix_Camera_View(
        camera_position_c, camera_view_vector, camera_up_vector);

    glm::mat4 projection;

    if (g_UsePerspectiveProjection) {
      float field_of_view = M_PI / 3.0f;
      projection =
          Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
    } else {
      float t = 1.5f * g_CameraDistance / 2.5f;
      float b = -t;
      float r = t * g_ScreenRatio;
      float l = -r;
      projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
    }

    sceneObjects(view, projection, T_view);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, bluePortalTexture);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "BluePortalTexture"), 10);

    glm::mat4 modelBluePortal =
        Matrix_Translate(bluePortalPosition.x, bluePortalPosition.y,
                         bluePortalPosition.z) *
        bluePortalRotation * Matrix_Scale(2.5f, 2.5f, 1.0f);
    DrawObject(modelBluePortal, "the_portal", BLUE_PORTAL);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, orangePortalTexture);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "OrangePortalTexture"),
                11);

    glm::mat4 modelOrangePortal =
        Matrix_Translate(orangePortalPosition.x, orangePortalPosition.y,
                         orangePortalPosition.z) *
        orangePortalRotation * Matrix_Scale(2.5f, 2.5f, 1.0f);
    DrawObject(modelOrangePortal, "the_portal", ORANGE_PORTAL);

    if (CheckCollisionPlayerPortal(camera_position_c, modelBluePortal)) {
      MovePlayerToPortal(&camera_position_c, modelOrangePortal, ORANGE_PORTAL);
    }

    if (CheckCollisionPlayerPortal(camera_position_c, modelOrangePortal)) {
      MovePlayerToPortal(&camera_position_c, modelBluePortal, BLUE_PORTAL);
    }

    // Drawing the portal gun
    glClear(GL_DEPTH_BUFFER_BIT);
    glm::mat4 model = T_view * Matrix_Translate(0.4, -0.3, -0.8) *
                      Matrix_Scale(0.3, 0.3, 0.3);
    DrawObject(model, "PortalGun", PORTALGUN);

    // Drawing the crosshair
    model = T_view * Matrix_Translate(0.0, 0.0, -2.5) *
            Matrix_Scale(0.015, 0.015, 0.015);
    DrawObject(model, "the_sphere", CROSSHAIR);

    // Checking Collision with the button

    player = GetBoundingBoxObject("the_bunny");
    modelPlayer = Matrix_Translate(camera_position_c.x, camera_position_c.y,
                                   camera_position_c.z) *
                  Matrix_Rotate_Y(g_CameraTheta + M_PI_2);
    player = {modelPlayer * player.min, modelPlayer * player.max};

    glm::mat4 buttonModel = Matrix_Translate(-20.0f, 0.1f, 25.0f) *
                            Matrix_Scale(0.03f, 0.005f, 0.005f);
    BoundingBox buttonBB = GetBoundingBoxObject("button");
    buttonBB = {buttonModel * buttonBB.min, buttonModel * buttonBB.max};

    BoundingBox boxBB = GetBoundingBoxObject("wcube_rdmobj00");
    glm::mat4 cubeModel =
        Matrix_Translate(boxPosition.x, boxPosition.y, boxPosition.z) *
        Matrix_Scale(0.05f, 0.05f, 0.05f);
    boxBB = {cubeModel * boxBB.min, cubeModel * boxBB.max};

    isFloorButtonPressed = CheckCollisionAABBtoAABB(player, buttonBB) ||
                           CheckCollisionAABBtoAABB(boxBB, buttonBB);

    // Checking if player has ended the level
    glm::mat4 modelDoor = Matrix_Translate(0.0f, 3.0f, -26.5f) *
                          Matrix_Rotate_Z(M_PI_2) *
                          Matrix_Scale(0.05f, 0.05f, 0.05f);

    BoundingBox doorBB = GetBoundingBoxObject("body.dmx/door.dmx.mesh_0");
    doorBB = {modelDoor * doorBB.min, modelDoor * doorBB.max};

    g_EndGame =
        CheckCollisionAABBtoAABB(player, doorBB) && isFloorButtonPressed;

    // Logic for grabbing the box
    if (g_KeyE_Toggled) {
      bool validDistance = norm(boxPosition - camera_position_c) <=
                           1.0f * norm(camera_view_vector);
      if (validDistance) {
        g_HoldingBox = true;
      }
      if (!validDistance && !g_HoldingBox) {
        g_KeyE_Toggled = false;
      }
    } else {
      g_HoldingBox = false;
    }

    if (!g_HoldingBox) {
      if (boxPosition.y < GetCurrentFloorY(boxPosition)) {
        boxPosition.y =
            GetCurrentFloorY(boxPosition) + 0.1f * boxBB.max.y / 2.0f;
      }
    }
    if (g_HoldingBox) {
      boxPosition = camera_position_c + camera_view_vector;
      boxBB = GetBoundingBoxObject("wcube_rdmobj00");
      if (boxPosition.y < GetCurrentFloorY(camera_position_c)) {
        boxPosition.y =
            GetCurrentFloorY(camera_position_c) + 0.1f * boxBB.max.y / 2.0f;
      }

    } else {
      boxBB = GetBoundingBoxObject("wcube_rdmobj00");
      cubeModel =
          Matrix_Translate(boxPosition.x, boxPosition.y, boxPosition.z) *
          Matrix_Scale(0.05f, 0.05f, 0.05f);
      boxBB = {cubeModel * boxBB.min, cubeModel * boxBB.max};

      // Box physics
      if (CheckCollisionAABBtoAABB(boxBB, g_MovingPlatform)) {
        boxPosition = boxPosition + g_MovingPlatformDelta;
        g_BoxOnMovingPlatform = true;
        g_BoxOnGround = true;
      } else {
        g_BoxOnMovingPlatform = false;
        g_BoxOnGround = false;
      }

      BoundingBox fixed_platform = GetBoundingBoxObject("platform");
      glm::mat4 platform_trasform = Matrix_Translate(0.0f, 13.0f, 0.0f) *
                                    Matrix_Scale(0.025f, 0.025f, 0.025f);
      fixed_platform = {platform_trasform * fixed_platform.min,
                        platform_trasform * fixed_platform.max};

      boxBB.min.y = boxBB.min.y - 2.0f * delta_t;
      g_BoxOnGround = CheckCollisionPlayerFloor(boxBB) ||
                      CheckCollisionAABBtoAABB(boxBB, fixed_platform) ||
                      g_BoxOnMovingPlatform;
      if (!g_BoxOnGround && !g_BoxOnMovingPlatform) {
        boxPosition.y = boxPosition.y - speed * delta_t;
        g_BoxOnGround = false;
      }
    }

    if (g_EndGame) {
      glClear(GL_DEPTH_BUFFER_BIT);
      model = T_view * Matrix_Translate(-5.0, -5.0, -5.5) *
              Matrix_Scale(10.0, 10.0, 1.0);
      DrawObject(model, "cube", LEVEL_COMPLETE);
    }

    TextRendering_ShowFramesPerSecond(window);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwTerminate();

  return 0;
}

// Portal teleportation
void MovePlayerToPortal(glm::vec4* camera, glm::mat4 portal_transform,
                        int portal_color) {
  if (!isBluePortalActive || !isOrangePortalActive) {
    return;
  }

  BoundingBox portalPoints = GetBoundingBoxObject("the_portal");

  BoundingBox portal = {portal_transform * portalPoints.min,
                        portal_transform * portalPoints.max};

  glm::vec4 portal_normal = GetNormal(portal);

  glm::vec4 new_position =
      glm::vec4((portal.max.x + portal.min.x) / 2.0f,
                (portal.max.y + portal.min.y) / 2.0f,
                (portal.max.z + portal.min.z) / 2.0f, 1.0f);

  if (new_position.y < camera->y) {
    new_position.y = portal.max.y;
  }

  *camera = new_position + 2.0f * (portal_normal / norm(portal_normal));

  // Change player's orientation after leaving a portal
  if (portal_color == BLUE_PORTAL) {
    switch (bluePortalSeesDirection) {
      case NORTH:
        g_CameraTheta = M_PI;
        break;

      case SOUTH:
        g_CameraTheta = 0;
        break;

      case EAST:
        g_CameraTheta = M_PI_2;
        break;

      case WEST:
        g_CameraTheta = -M_PI_2;
        break;
    }
  } else {
    switch (orangePortalSeesDirection) {
      case NORTH:
        g_CameraTheta = M_PI;
        break;

      case SOUTH:
        g_CameraTheta = 0;
        break;

      case EAST:
        g_CameraTheta = M_PI_2;
        break;

      case WEST:
        g_CameraTheta = -M_PI_2;
        break;
    }
  }
}

// Drawing scene objects seen in the three cameras
void sceneObjects(glm::mat4 view, glm::mat4 projection, glm::mat4 T_view) {
  glm::mat4 model = Matrix_Identity();

  glUniformMatrix4fv(g_view_uniform, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(g_projection_uniform, 1, GL_FALSE,
                     glm::value_ptr(projection));

  model = Matrix_Translate(0.0f, 8.0f, -30.0f);
  if (isFloorButtonPressed)
    DrawObject(model, "the_sphere", GOURAUD_SHADING_GREEN);
  else
    DrawObject(model, "the_sphere", GOURAUD_SHADING_RED);

  // moving platfmorm
  DrawObject(g_bezierModel, "platform", PLATFORM);

  // Fixed platform
  model = Matrix_Translate(0.0f, 13.0f, 0.0f) *
          Matrix_Scale(0.025f, 0.025f, 0.025f);
  DrawObject(model, "platform", PLATFORM);

  // Button
  model = Matrix_Translate(-20.0f, 0.35f, 25.0f) *
          Matrix_Scale(0.035f, 0.035f, 0.035f);

  DrawObject(model, "button", BUTTON);

  // Box
  model = Matrix_Translate(boxPosition.x, boxPosition.y, boxPosition.z) *
          Matrix_Scale(0.05f, 0.05f, 0.05f);
  DrawObject(model, "wcube_rdmobj00", BOX);

  // Exit door
  model = Matrix_Translate(0.0f, 3.0f, -26.5f) * Matrix_Rotate_Z(M_PI_2) *
          Matrix_Scale(0.05f, 0.05f, 0.05f);

  if (!isFloorButtonPressed) {
    DrawObject(model, "body.dmx/door.dmx.mesh_0", DOOR);
  } else {
    DrawObject(model, "body.dmx/door_open.dmx.mesh_0", DOOR);
  }

  // Higher walls
  model =
      Matrix_Translate(-30.0f, 0.0f, -30.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, 0.0f, -30.0f) * Matrix_Rotate_Y(-M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-M_PI) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  // Floors
  model = Matrix_Translate(-30.0, 0.0f, -10.0f) * Matrix_Rotate_X(-M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", FLOOR);

  model = Matrix_Translate(-30.0, -20.0f, 10.0f) * Matrix_Rotate_X(-M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", FLOOR);

  model = Matrix_Translate(-30.0, 0.0f, 30.0f) * Matrix_Rotate_X(-M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", FLOOR);

  // Ceilings
  model = Matrix_Translate(-30.0, 20.0f, -10.0f) * Matrix_Rotate_X(M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0, 20.0f, 10.0f) * Matrix_Rotate_X(M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0, 20.0f, -30.0f) * Matrix_Rotate_X(M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  // Lower walls

  model = Matrix_Translate(-30.0f, -20.0f, -10.0f) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(-30.0f, -20.0f, 30.0f) * Matrix_Rotate_Y(M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, -20.0f, -30.0f) * Matrix_Rotate_Y(-M_PI_2) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);

  model = Matrix_Translate(30.0f, -20.0f, 10.0f) * Matrix_Rotate_Y(M_PI) *
          Matrix_Scale(10.0f, 10.0f, 0.0f);
  DrawObject(model, "the_wall", WALL);
}

void LoadTextures() {
  LoadTextureImage("../../data/door/EDITOR_door.png");
  LoadTextureImage("../../data/Button/button.png");
  LoadTextureImage("../../data/portalwall.png");
  LoadTextureImage("../../textures/portalgun/textures/portalgun_col.jpg");
  LoadTextureImage("../../data/Platform/light_rail_platform.png");
  LoadTextureImage("../../data/Weighted Cube/metal_box.png");
  LoadTextureImage("../../data/level_complete.png");
}

void DrawObject(glm::mat4 model, const char* name, int id) {
  glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
  glUniform1i(g_object_id_uniform, id);
  DrawVirtualObject(name);
}

// Change portal position after shooting the portal gun
void UpdatePortalPosition(glm::vec4 colision_point, glm::vec4 surface_normal,
                          int portal_color) {
  float normal_x = surface_normal.x;
  float normal_z = surface_normal.z;

  if (colision_point.x == -1 && colision_point.y == -1 &&
      colision_point.z == -1 && colision_point.w == -1) {
    return;
  }

  const float delta_wall = 0.05;

  // Check if the z or x normals are positive or negative to define the orientation of the portal
  if (normal_z > 1e-6f) {
    if (portal_color == BLUE_PORTAL) {
      bluePortalRotation = Matrix_Identity();
      bluePortalSeesDirection = SOUTH;

      bluePortalLooksAt = colision_point;
      bluePortalLooksAt.z += 5.0;

      bluePortalPosition = colision_point;
      bluePortalPosition.z += delta_wall + 0.05;
    } else {
      orangePortalRotation = Matrix_Identity();
      orangePortalSeesDirection = SOUTH;

      orangePortalLooksAt = colision_point;
      orangePortalLooksAt.z += 5.0;

      orangePortalPosition = colision_point;
      orangePortalPosition.z += delta_wall;
    }
  }

  if (normal_z < -1e-6f) {
    if (portal_color == BLUE_PORTAL) {
      bluePortalRotation = Matrix_Rotate_Y(-M_PI);
      bluePortalSeesDirection = NORTH;

      bluePortalLooksAt = colision_point;
      bluePortalLooksAt.z -= 5.0;

      bluePortalPosition = colision_point;
      bluePortalPosition.z -= delta_wall + 0.05;
    } else {
      orangePortalRotation = Matrix_Rotate_Y(-M_PI);
      orangePortalSeesDirection = NORTH;

      orangePortalLooksAt = colision_point;
      orangePortalLooksAt.z -= 5.0;

      orangePortalPosition = colision_point;
      orangePortalPosition.z -= delta_wall;
    }
  }

  if (normal_x > 1e-6f) {
    if (portal_color == BLUE_PORTAL) {
      bluePortalRotation = Matrix_Rotate_Y(M_PI_2);
      bluePortalSeesDirection = EAST;

      bluePortalLooksAt = colision_point;
      bluePortalLooksAt.x += 5.0;

      bluePortalPosition = colision_point;
      bluePortalPosition.x += delta_wall + 0.05;
    } else {
      orangePortalRotation = Matrix_Rotate_Y(M_PI_2);
      orangePortalSeesDirection = EAST;

      orangePortalLooksAt = colision_point;
      orangePortalLooksAt.x += 5.0;

      orangePortalPosition = colision_point;
      orangePortalPosition.x += delta_wall;
    }
  }

  if (normal_x < -1e-6f) {
    if (portal_color == BLUE_PORTAL) {
      bluePortalRotation = Matrix_Rotate_Y(-M_PI_2);
      bluePortalSeesDirection = WEST;

      bluePortalLooksAt = colision_point;
      bluePortalLooksAt.x -= 5.0;

      bluePortalPosition = colision_point;
      bluePortalPosition.x -= delta_wall + 0.05;
    } else {
      orangePortalRotation = Matrix_Rotate_Y(-M_PI_2);
      orangePortalSeesDirection = WEST;

      orangePortalLooksAt = colision_point;
      orangePortalLooksAt.x -= 5.0;

      orangePortalPosition = colision_point;
      orangePortalPosition.x -= delta_wall;
    }
  }
}

// Calculate the point t of a Bézier curve
glm::vec4 interpolate(float t, glm::vec4 point_a, glm::vec4 point_b) {
  return point_a + t * (point_b - point_a);
}

glm::vec4 calculateBezierCurve(float t, glm::vec4 point_a, glm::vec4 point_b,
                               glm::vec4 point_c, glm::vec4 point_d) {
  glm::vec4 ab = interpolate(t, point_a, point_b);
  glm::vec4 bc = interpolate(t, point_b, point_c);
  glm::vec4 cd = interpolate(t, point_c, point_d);

  glm::vec4 abc = interpolate(t, ab, bc);
  glm::vec4 bcd = interpolate(t, bc, cd);

  return interpolate(t, abc, bcd);
}
