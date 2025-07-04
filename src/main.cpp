#include "configs.h" // Globals variables, includes and structs
#include "lab_functions.h" // Functions defined in the labs

void LoadTextures(); // Function to Load the texture from images
void DrawObject(glm::mat4 model, const char* name, int id); // Function to draw the object in the screen

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define PORTALGUN 3

int main(int argc, char* argv[])
{
  int success = glfwInit();
  if (!success)
    {
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
  window = glfwCreateWindow(1920, 1080, "INF01047 - Trabalho Final Rayan e Gabriel Henrique", glfwGetPrimaryMonitor(), NULL);
  if (!window)
    {
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

  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
  FramebufferSizeCallback(window, 1920, 1080);

  const GLubyte *vendor      = glGetString(GL_VENDOR);
  const GLubyte *renderer    = glGetString(GL_RENDERER);
  const GLubyte *glversion   = glGetString(GL_VERSION);
  const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

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

  if ( argc > 1 )
    {
      ObjModel model(argv[1]);
      BuildTrianglesAndAddToVirtualScene(&model);
    }

  TextRendering_Init();

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glm::vec4 camera_position_c  = glm::vec4(0.0f,4.0f,5.0f,1.0f);
  float speed = 3.0f;
  float prev_time = (float) glfwGetTime();

  while (!glfwWindowShouldClose(window))
    {

      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram(g_GpuProgramID);

      float r = g_CameraDistance;
      float y = r*sin(g_CameraPhi);
      float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
      float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

      glm::vec4 camera_view_vector = glm::vec4(x,y,z,0.0f);
      glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);

      float current_time = (float) glfwGetTime();
      float delta_t = current_time - prev_time;
      prev_time = current_time;

      glm::vec4 camera_w_vector = -(camera_view_vector / norm(camera_view_vector));
      camera_w_vector.y = 0;

      if(g_KeyW_Pressed){
	camera_position_c += -camera_w_vector * speed * delta_t;
      }
      if(g_KeyS_Pressed){
	camera_position_c += camera_w_vector * speed * delta_t;
      }
      if(g_KeyD_Pressed){
	glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
	camera_position_c += (upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
      }
      if(g_KeyA_Pressed){
	glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
	camera_position_c += -(upw_crossprod / norm(upw_crossprod)) * speed * delta_t;
      }

      glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
      glm::mat4 T_view = T_Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

      glm::mat4 projection;

      float nearplane = -0.1f;
      float farplane  = -25.0f;

      if (g_UsePerspectiveProjection)
        {
	  float field_of_view = 3.141592 / 3.0f;
	  projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        }
      else
        {
	  float t = 1.5f*g_CameraDistance/2.5f;
	  float b = -t;
	  float r = t*g_ScreenRatio;
	  float l = -r;
	  projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

      glm::mat4 model = Matrix_Identity();

      glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
      glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

      // Drawing the sphere
      model = Matrix_Translate(-1.0f,0.0f,0.0f)
	* Matrix_Rotate_Z(0.6f)
	* Matrix_Rotate_X(0.2f)
	* Matrix_Rotate_Y(g_AngleY + (float)glfwGetTime() * 0.1f);
      DrawObject(model, "the_sphere", SPHERE);

      // Drawing the portal gun
      model = T_view
	* Matrix_Translate(1,  -1, -2)
	* Matrix_Scale(1, 1, 1);
      DrawObject(model, "PortalGun", PORTALGUN);

      // Drawing the bunny
      model = Matrix_Translate(1.0f,0.0f,0.0f)
	* Matrix_Rotate_X(g_AngleX + (float)glfwGetTime() * 0.1f);
      DrawObject(model, "the_bunny", BUNNY);

      // Drawing the planes
      model = Matrix_Translate(0.0f,-1.1f,0.0f)
	* Matrix_Scale(10.0f, 1.0f, 10.0f);
      DrawObject(model, "the_plane", PLANE);

      model = Matrix_Translate(0.0f,3.9f,-10.0f)
	* Matrix_Rotate_X(3.141592f /2)
	* Matrix_Scale(10.0f, 1.0f, 10.0f);
      DrawObject(model, "the_plane", PLANE);

      model = Matrix_Translate(-10.0f,3.9f,0.0f)
	* Matrix_Rotate_Y(3.141592f/2)
	* Matrix_Rotate_X(3.141592f/2)
	* Matrix_Scale(10.0f, 1.0f, 10.0f);
      DrawObject(model, "the_plane", PLANE);

      model = Matrix_Translate(10.0f,3.9f,0.0f)
	* Matrix_Rotate_Y(-3.141592f/2)
	* Matrix_Rotate_X(3.141592f/2)
	* Matrix_Scale(10.0f, 1.0f, 10.0f);
      DrawObject(model, "the_plane", PLANE);

      model = Matrix_Translate(0.0f,3.9f,10.0f)
	* Matrix_Rotate_Y(3.141592f)
	* Matrix_Rotate_X(3.141592f/2)
	* Matrix_Scale(10.0f, 1.0f, 10.0f);
      DrawObject(model, "the_plane", PLANE);

      TextRendering_ShowFramesPerSecond(window);

      glfwSwapBuffers(window);

      glfwPollEvents();
    }

  glfwTerminate();

  return 0;
}

void LoadTextures(){

  LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");
  LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif");
  LoadTextureImage("../../data/portalwall.png");
  LoadTextureImage("../../textures/portalgun/textures/portalgun_col.jpg");

}

void DrawObject(glm::mat4 model, const char* name, int id){

      glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
      glUniform1i(g_object_id_uniform, id);
      DrawVirtualObject(name);

}
