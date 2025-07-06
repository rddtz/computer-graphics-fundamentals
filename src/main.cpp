#include "configs.h" // Globals variables, includes and structs
#include "lab_functions.h" // Functions defined in the labs
#include "collision.h" // Collision functions

void LoadTextures(); // Function to Load the texture from images
void DrawObject(glm::mat4 model, const char* name, int id); // Function to draw the object in the screen
void PrintVec4(glm::vec4 vec);

#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define PORTALGUN 3
#define CUBE 4

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


  ObjModel cube("../../data/the_cube.obj");
  ComputeNormals(&cube);
  BuildTrianglesAndAddToVirtualScene(&cube);


  ObjModel gunmodel("../../textures/portalgun/portalgun.obj");
  BuildTrianglesAndAddToVirtualScene(&gunmodel);

  if ( argc > 1 )
    {
      ObjModel model(argv[1]);
      BuildTrianglesAndAddToVirtualScene(&model);
    }

  // GLuint bounding_box_object_id = CreateBox();

  TextRendering_Init();

  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glm::vec4 camera_position_c  = glm::vec4(0.0f,0.0f,5.0f,1.0f);
  float speed = 3.0f;
  float prev_time = (float) glfwGetTime();

  PrintObjModelInfo(&planemodel);
  for (auto const& x : g_VirtualScene)
    {
      std::cout << x.first  // string (key)
		<< ':'
		<< x.second.name // string's value
		<< ":Min(" << x.second.bbox_min.x
		<< ", " << x.second.bbox_min.y
	        << ", " << x.second.bbox_min.y
		<< ")"
		<< ":Max(" << x.second.bbox_max.x
		<< ", " << x.second.bbox_max.y
	        << ", " << x.second.bbox_max.y
		<< ")"
		<< ":FirstIndex(" << x.second.first_index
		<< ")"
		<< ":NumIndices(" << x.second.num_indices
		<< ")"
		<< ":VAO_ID(" << x.second.vertex_array_object_id
		<< ")"
		<< std::endl;
    }

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

      glm::vec4 cam_temp = camera_position_c;

      if(g_KeyW_Pressed){

	cam_temp += -camera_w_vector * speed * delta_t;

	if(!CheckColisionPointWalls(cam_temp)){
	  camera_position_c = cam_temp;
	}

      }
      if(g_KeyS_Pressed){
	cam_temp += camera_w_vector * speed * delta_t;

	if(!CheckColisionPointWalls(cam_temp)){
	  camera_position_c = cam_temp;
	}
      }

      if(g_KeyD_Pressed ){
	glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
	cam_temp += (upw_crossprod / norm(upw_crossprod)) * speed * delta_t;

	if(!CheckColisionPointWalls(cam_temp)){
	  camera_position_c = cam_temp;
	}
      }

      if(g_KeyA_Pressed){
	glm::vec4 upw_crossprod = crossproduct(camera_up_vector, camera_w_vector);
	cam_temp += -(upw_crossprod / norm(upw_crossprod)) * speed * delta_t;

	if(!CheckColisionPointWalls(cam_temp)){
	  camera_position_c = cam_temp;
	}
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
      // DrawObject(model, "cube_faces", bounding_box_object_id);

      BoundingBox obj = {glm::vec4(g_VirtualScene["the_sphere"].bbox_min.x, g_VirtualScene["the_sphere"].bbox_min.y, g_VirtualScene["the_sphere"].bbox_min.z, 1), glm::vec4(g_VirtualScene["the_sphere"].bbox_max.x, g_VirtualScene["the_sphere"].bbox_max.y, g_VirtualScene["the_sphere"].bbox_max.z, 1)};

      // std::cout << "min";
      // PrintVec4(obj.min);
      // std::cout << "max";
      // PrintVec4(obj.max);
      // std::cout << "size";
      // PrintVec4(size);
      // std::cout << "center";
      // PrintVec4(center);

      // model = model * Matrix_Translate(center.x, center.y, center.z) * Matrix_Scale(size.x, size.y, size.z);
      // DrawObject(model, "cube", CUBE);


      // Drawing the portal gun
      model = T_view
	* Matrix_Translate(0.2,  -0.1, -0.2)
	* Matrix_Scale(0.2, 0.2, 0.2);
      DrawObject(model, "PortalGun", PORTALGUN);
      glm::mat4 modelPortalGun = model;

      obj = {glm::vec4(g_VirtualScene["PortalGun"].bbox_min.x, g_VirtualScene["PortalGun"].bbox_min.y, g_VirtualScene["PortalGun"].bbox_min.z, 1), glm::vec4(g_VirtualScene["PortalGun"].bbox_max.x, g_VirtualScene["PortalGun"].bbox_max.y, g_VirtualScene["PortalGun"].bbox_max.z, 1)};

      model = modelPortalGun * Matrix_Translate(obj.max.x, obj.max.y, obj.max.z) * Matrix_Scale(0.7, 0.7, 0.7);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.max.x, obj.max.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.max.x, obj.min.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.max.x, obj.min.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.min.x, obj.max.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.min.x, obj.min.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.min.x, obj.max.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelPortalGun * Matrix_Translate(obj.min.x, obj.min.y, obj.min.z) * Matrix_Scale(0.3, 0.3, 0.3);
      DrawObject(model, "cube", CUBE);



      // model = model * Matrix_Translate(center.x, center.y, center.z) * Matrix_Scale(size.x, size.y, size.z);
      // DrawObject(model, "cube", CUBE);

      // Drawing the bunny
      model = Matrix_Translate(1.0f,0.0f,0.0f)
	* Matrix_Rotate_X(g_AngleX + (float)glfwGetTime() * 0.1f);
      DrawObject(model, "the_bunny", BUNNY);
      glm::mat4 modelBunny =  Matrix_Translate(1.0f,0.0f,0.0f) * Matrix_Scale(1.5, 1.5, 1.5);
      // model = Matrix_Identity();
      // DrawObject(model, "cube", CUBE);

      obj = {glm::vec4(g_VirtualScene["the_bunny"].bbox_min.x, g_VirtualScene["the_bunny"].bbox_min.y, g_VirtualScene["the_bunny"].bbox_min.z, 1), glm::vec4(g_VirtualScene["the_bunny"].bbox_max.x, g_VirtualScene["the_bunny"].bbox_max.y, g_VirtualScene["the_bunny"].bbox_max.z, 1)};

      model = modelBunny * Matrix_Translate(obj.max.x, obj.max.y, obj.max.z) * Matrix_Scale(0.7, 0.7, 0.7);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.max.x, obj.max.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.max.x, obj.min.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.max.x, obj.min.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.min.x, obj.max.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.min.x, obj.min.y, obj.max.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.min.x, obj.max.y, obj.min.z) * Matrix_Scale(0.5, 0.5, 0.5);
      DrawObject(model, "cube", CUBE);

      model = modelBunny * Matrix_Translate(obj.min.x, obj.min.y, obj.min.z) * Matrix_Scale(0.3, 0.3, 0.3);
      DrawObject(model, "cube", CUBE);


      // Drawing the planes
      model = Matrix_Translate(0.0f,-1.1f,0.0f) // chão
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
      TextRendering_ShowModelViewProjection(
					    window,
					    projection,
					    view,
					    modelPortalGun,
					    camera_position_c
					    );

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

void PrintVec4(glm::vec4 vec){

  std::cout << "(" << vec.x << "," << vec.y << "," << vec.z << ")\n";

}
