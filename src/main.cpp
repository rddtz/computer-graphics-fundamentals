#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils.h"
#include "matrices.h"


GLuint BuildTriangles();
void LoadShadersFromFiles();
GLuint LoadShader_Vertex(const char* filename);
GLuint LoadShader_Fragment(const char* filename);
void LoadShader(const char* filename, GLuint shader_id);
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);


void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);


void TextRendering_ShowModelViewProjection(GLFWwindow* window, glm::mat4 projection, glm::mat4 view, glm::mat4 model, glm::vec4 p_model);
void TextRendering_ShowEulerAngles(GLFWwindow* window);
void TextRendering_ShowProjection(GLFWwindow* window);
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);


void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

struct SceneObject
{
    const char*  name;
  void*        first_index;
  int          num_indices;
  GLenum       rendering_mode;
};

std::map<const char*, SceneObject> g_VirtualScene;

float g_ScreenRatio = 1.0f;

float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

bool g_LeftMouseButtonPressed = false;

float tecla_A = false;
bool tecla_D = false;
bool tecla_W = false;
float tecla_S = false;

float g_CameraTheta = 0.0f;
float g_CameraPhi = 0.0f;
float g_CameraDistance = 2.5f;

bool g_UsePerspectiveProjection = true;

bool g_ShowInfoText = true;

GLuint g_GpuProgramID = 0;

int main()
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
    window = glfwCreateWindow(800, 800, "INF01047 - 00584919 - Rayan Raddatz de Matos", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    glfwSetCursorPosCallback(window, CursorPosCallback);

    glfwSetScrollCallback(window, ScrollCallback);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetWindowSize(window, 800, 800);


    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    LoadShadersFromFiles();

    GLuint vertex_array_object_id = BuildTriangles();

    TextRendering_Init();

    GLint model_uniform           = glGetUniformLocation(g_GpuProgramID, "model");
    GLint view_uniform            = glGetUniformLocation(g_GpuProgramID, "view");
    GLint projection_uniform      = glGetUniformLocation(g_GpuProgramID, "projection");
    GLint render_as_black_uniform = glGetUniformLocation(g_GpuProgramID, "render_as_black");

    glEnable(GL_DEPTH_TEST);

    glm::mat4 the_projection;
    glm::mat4 the_model;
    glm::mat4 the_view;

    float speed = 0.5f;
    float prev_time = (float)glfwGetTime();
    float current_time = (float)glfwGetTime();
    float delta_t = current_time - prev_time;

    float y = 0.0f;
    float z = -2.5f;
    float x = -2.5f;

    glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f);

    while (!glfwWindowShouldClose(window))
    {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


      glUseProgram(g_GpuProgramID);


      glBindVertexArray(vertex_array_object_id);

      glm::vec4 camera_view_vector = glm::vec4(cos(g_CameraPhi)*sin(g_CameraTheta),sin(g_CameraPhi),cos(g_CameraPhi)*cos(g_CameraTheta),0.0f);
      glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);

      glm::vec4 vector_W = -camera_view_vector/norm(camera_view_vector);
      glm::vec4 vector_U = crossproduct(camera_up_vector, vector_W)/norm(crossproduct(camera_up_vector, vector_W));

      current_time = (float)glfwGetTime();
      delta_t = current_time - prev_time;
      prev_time = current_time;

      if(tecla_W){
	camera_position_c += -vector_W*speed*delta_t;
      }
      if(tecla_A){
	camera_position_c += -vector_U*speed*delta_t;
      }
      if(tecla_S){
	camera_position_c += vector_W*speed*delta_t;
      }
      if(tecla_D){
	camera_position_c += vector_U*speed*delta_t;
      }

      glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

      glm::mat4 projection;

      float nearplane = -0.1f;
      float farplane  = -10.0f;

      if (g_UsePerspectiveProjection)
      {
	float field_of_view = 3.141592 / 3.0f;
	projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
      }
      else {
	float t = 1.5f*g_CameraDistance/2.5f;
	float b = -t;
	float r = t*g_ScreenRatio;
	float l = -r;
	projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
      }

      glUniformMatrix4fv(view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
      glUniformMatrix4fv(projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));


      for (int i = 1; i <= 3; ++i)
      {
	glm::mat4 model;

	if (i == 1)
	  {
	    model = Matrix_Identity();
	  }
	else if ( i == 2 )
	  {
	    model = Matrix_Translate(0.0f, 0.0f, -2.0f)
	      * Matrix_Rotate(3.141592f / 8.0f, glm::vec4(1.0f,1.0f,1.0f,0.0f))
	      * Matrix_Scale(2.0f, 0.5f, 0.5f);
	  }
	else if ( i == 3 )
            {
	      model = Matrix_Translate(-2.0f, 0.0f, 0.0f)
		* Matrix_Rotate_Z(g_AngleZ)
		* Matrix_Rotate_Y(g_AngleY)
		* Matrix_Rotate_X(g_AngleX);

	      the_model = model;
	      the_projection = projection;
	      the_view = view;
            }

	glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

	glUniform1i(render_as_black_uniform, false);

	glDrawElements(
		       g_VirtualScene["cube_faces"].rendering_mode,
		       g_VirtualScene["cube_faces"].num_indices,
		       GL_UNSIGNED_INT,
		       (void*)g_VirtualScene["cube_faces"].first_index
		       );

	glLineWidth(4.0f);

	glDrawElements(
		       g_VirtualScene["axes"].rendering_mode,
		       g_VirtualScene["axes"].num_indices,
		       GL_UNSIGNED_INT,
		       (void*)g_VirtualScene["axes"].first_index
		       );

	glUniform1i(render_as_black_uniform, true);

	glDrawElements(
		       g_VirtualScene["cube_edges"].rendering_mode,
		       g_VirtualScene["cube_edges"].num_indices,
		       GL_UNSIGNED_INT,
		       (void*)g_VirtualScene["cube_edges"].first_index
		       );

	if ( i == 3 )
	  {
	    glPointSize(15.0f);
	    glDrawArrays(GL_POINTS, 3, 1);
	  }
      }

      glm::mat4 model = Matrix_Identity();

      glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));

      glLineWidth(10.0f);

      glUniform1i(render_as_black_uniform, false);

      glDrawElements(
		     g_VirtualScene["axes"].rendering_mode,
		     g_VirtualScene["axes"].num_indices,
		     GL_UNSIGNED_INT,
		     (void*)g_VirtualScene["axes"].first_index
        );

      glBindVertexArray(0);

      glm::vec4 p_model(0.5f, 0.5f, 0.5f, 1.0f);
      // TextRendering_ShowModelViewProjection(window, the_projection, the_view, the_model, p_model);

      // TextRendering_ShowEulerAngles(window);

      // TextRendering_ShowProjection(window);

      // TextRendering_ShowFramesPerSecond(window);

      glfwSwapBuffers(window);

      glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}


GLuint BuildTriangles()
{

  GLfloat model_coefficients[] = {

    -0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f,
    0.5f, -0.5f,  0.5f, 1.0f,
    0.5f,  0.5f,  0.5f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f,
    0.5f,  0.5f, -0.5f, 1.0f,

    0.0f,  0.0f,  0.0f, 1.0f,
    1.0f,  0.0f,  0.0f, 1.0f,

    0.0f,  0.0f,  0.0f, 1.0f,
    0.0f,  1.0f,  0.0f, 1.0f,

    0.0f,  0.0f,  0.0f, 1.0f,
    0.0f,  0.0f,  1.0f, 1.0f,
  };

  GLuint VBO_model_coefficients_id;
  glGenBuffers(1, &VBO_model_coefficients_id);

  GLuint vertex_array_object_id;
  glGenVertexArrays(1, &vertex_array_object_id);

  glBindVertexArray(vertex_array_object_id);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);

  glBufferData(GL_ARRAY_BUFFER, sizeof(model_coefficients), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(model_coefficients), model_coefficients);

  GLuint location = 0;
  GLint  number_of_dimensions = 4;
  glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(location);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLfloat color_coefficients[] = {

    1.0f, 0.5f, 0.0f, 1.0f,
    1.0f, 0.5f, 0.0f, 1.0f,
    0.0f, 0.5f, 1.0f, 1.0f,
    0.0f, 0.5f, 1.0f, 1.0f,
    1.0f, 0.5f, 0.0f, 1.0f,
    1.0f, 0.5f, 0.0f, 1.0f,
    0.0f, 0.5f, 1.0f, 1.0f,
    0.0f, 0.5f, 1.0f, 1.0f,

    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,

    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,

    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
  };

  GLuint VBO_color_coefficients_id;
  glGenBuffers(1, &VBO_color_coefficients_id);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_color_coefficients_id);
  glBufferData(GL_ARRAY_BUFFER, sizeof(color_coefficients), NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(color_coefficients), color_coefficients);
  location = 1;
  number_of_dimensions = 4;
  glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(location);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  GLuint indices[] = {

    0, 1, 2,
    7, 6, 5,
    3, 2, 6,
    4, 0, 3,
    4, 5, 1,
    1, 5, 6,
    0, 2, 3,
    7, 5, 4,
    3, 6, 7,
    4, 3, 7,
    4, 1, 0,
    1, 6, 2,

    0, 1,
    1, 2,
    2, 3,
    3, 0,
    0, 4,
    4, 7,
    7, 6,
    6, 2,
    6, 5,
    5, 4,
    5, 1,
    7, 3,

    8 , 9 ,
    10, 11,
    12, 13
  };

  SceneObject cube_faces;
  cube_faces.name           = "Cubo (faces coloridas)";
  cube_faces.first_index    = (void*)0;
  cube_faces.num_indices    = 36;
  cube_faces.rendering_mode = GL_TRIANGLES;

  g_VirtualScene["cube_faces"] = cube_faces;

  SceneObject cube_edges;
  cube_edges.name           = "Cubo (arestas pretas)";
  cube_edges.first_index    = (void*)(36*sizeof(GLuint));
  cube_edges.num_indices    = 24;
  cube_edges.rendering_mode = GL_LINES;

  g_VirtualScene["cube_edges"] = cube_edges;

  SceneObject axes;
  axes.name           = "Eixos XYZ";
  axes.first_index    = (void*)(60*sizeof(GLuint));
  axes.num_indices    = 6;
  axes.rendering_mode = GL_LINES;
  g_VirtualScene["axes"] = axes;

  GLuint indices_id;
  glGenBuffers(1, &indices_id);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), NULL, GL_STATIC_DRAW);

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

  glBindVertexArray(0);

  return vertex_array_object_id;
}

GLuint LoadShader_Vertex(const char* filename)
{
  GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

  LoadShader(filename, vertex_shader_id);

  return vertex_shader_id;
}


GLuint LoadShader_Fragment(const char* filename)
{
  GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

  LoadShader(filename, fragment_shader_id);

  return fragment_shader_id;
}

void LoadShader(const char* filename, GLuint shader_id)
{
  std::ifstream file;
  try {
    file.exceptions(std::ifstream::failbit);
    file.open(filename);
  } catch ( std::exception& e ) {
    fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
    std::exit(EXIT_FAILURE);
  }
  std::stringstream shader;
  shader << file.rdbuf();
  std::string str = shader.str();
  const GLchar* shader_string = str.c_str();
  const GLint   shader_string_length = static_cast<GLint>( str.length() );

  glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

  glCompileShader(shader_id);

  GLint compiled_ok;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

  GLint log_length = 0;
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

  GLchar* log = new GLchar[log_length];
  glGetShaderInfoLog(shader_id, log_length, &log_length, log);

  if ( log_length != 0 )
    {
      std::string  output;

      if ( !compiled_ok )
        {
	  output += "ERROR: OpenGL compilation of \"";
	  output += filename;
	  output += "\" failed.\n";
	  output += "== Start of compilation log\n";
	  output += log;
	  output += "== End of compilation log\n";
        }
      else
        {
	  output += "WARNING: OpenGL compilation of \"";
	  output += filename;
	  output += "\".\n";
	  output += "== Start of compilation log\n";
	  output += log;
	  output += "== End of compilation log\n";
        }

      fprintf(stderr, "%s", output.c_str());
    }

  delete [] log;
}

void LoadShadersFromFiles()
{
  GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
  GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

  if ( g_GpuProgramID != 0 )
    glDeleteProgram(g_GpuProgramID);

  g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
}

GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
  GLuint program_id = glCreateProgram();

  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);

  glLinkProgram(program_id);

  GLint linked_ok = GL_FALSE;
  glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

  if ( linked_ok == GL_FALSE )
    {
      GLint log_length = 0;
      glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

      GLchar* log = new GLchar[log_length];

      glGetProgramInfoLog(program_id, log_length, &log_length, log);

      std::string output;

      output += "ERROR: OpenGL linking of program failed.\n";
      output += "== Start of link log\n";
      output += log;
      output += "\n== End of link log\n";

      delete [] log;

      fprintf(stderr, "%s", output.c_str());
    }

  return program_id;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);

  g_ScreenRatio = (float)width / height;
}

double g_LastCursorPosX, g_LastCursorPosY;

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
      glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
      g_LeftMouseButtonPressed = true;
    }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
      g_LeftMouseButtonPressed = false;
    }
}



void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
  if (!g_LeftMouseButtonPressed)
    return;

  float dx = xpos - g_LastCursorPosX;
  float dy = ypos - g_LastCursorPosY;

  g_CameraTheta -= 0.003f*dx;
  g_CameraPhi   -= 0.003f*dy;

  float phimax = 3.141592f/2;
  float phimin = -phimax;

  if (g_CameraPhi > phimax)
    g_CameraPhi = phimax;

  if (g_CameraPhi < phimin)
    g_CameraPhi = phimin;

  g_LastCursorPosX = xpos;
  g_LastCursorPosY = ypos;
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  g_CameraDistance -= 0.1f*yoffset;

  const float verysmallnumber = std::numeric_limits<float>::epsilon();
  if (g_CameraDistance < verysmallnumber)
    g_CameraDistance = verysmallnumber;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
  for (int i = 0; i < 10; ++i)
    if (key == GLFW_KEY_0 + i && action == GLFW_PRESS && mod == GLFW_MOD_SHIFT)
      std::exit(100 + i);

  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key == GLFW_KEY_W)
    {
      if(action == GLFW_PRESS){
	tecla_W = true;
      }
      else if (action == GLFW_RELEASE){
	tecla_W = false;
      }
    }

  if (key == GLFW_KEY_A)
    {
      if(action == GLFW_PRESS){
	tecla_A = true;
      }
      else if (action == GLFW_RELEASE){
	tecla_A = false;
      }
    }

  if (key == GLFW_KEY_S)
    {
      if(action == GLFW_PRESS){
	tecla_S = true;
      }
      else if (action == GLFW_RELEASE){
	tecla_S = false;
      }
    }

  if (key == GLFW_KEY_D)
    {
      if(action == GLFW_PRESS){
	tecla_D = true;
      }
      else if (action == GLFW_RELEASE){
	tecla_D = false;
      }
    }

  float delta = 3.141592 / 16;

  if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
      g_AngleX += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

  if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
      g_AngleY += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }
  if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
      g_AngleZ += (mod & GLFW_MOD_SHIFT) ? -delta : delta;
    }

  if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
      g_AngleX = 0.0f;
      g_AngleY = 0.0f;
      g_AngleZ = 0.0f;
    }

  if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
      g_UsePerspectiveProjection = true;
    }

  if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
      g_UsePerspectiveProjection = false;
    }

  if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
      g_ShowInfoText = !g_ShowInfoText;
    }
}


void ErrorCallback(int error, const char* description)
{
  fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

void TextRendering_ShowModelViewProjection(
					   GLFWwindow* window,
					   glm::mat4 projection,
					   glm::mat4 view,
					   glm::mat4 model,
					   glm::vec4 p_model
					   )
{
  if ( !g_ShowInfoText )
    return;

  glm::vec4 p_world = model*p_model;
  glm::vec4 p_camera = view*p_world;
  glm::vec4 p_clip = projection*p_camera;
  glm::vec4 p_ndc = p_clip / p_clip.w;

  float pad = TextRendering_LineHeight(window);

  TextRendering_PrintString(window, " Model matrix             Model     In World Coords.", -1.0f, 1.0f-pad, 1.0f);
  TextRendering_PrintMatrixVectorProduct(window, model, p_model, -1.0f, 1.0f-2*pad, 1.0f);

  TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-6*pad, 1.0f);
  TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-7*pad, 1.0f);
  TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-8*pad, 1.0f);

  TextRendering_PrintString(window, " View matrix              World     In Camera Coords.", -1.0f, 1.0f-9*pad, 1.0f);
  TextRendering_PrintMatrixVectorProduct(window, view, p_world, -1.0f, 1.0f-10*pad, 1.0f);

  TextRendering_PrintString(window, "                                        |  ", -1.0f, 1.0f-14*pad, 1.0f);
  TextRendering_PrintString(window, "                            .-----------'  ", -1.0f, 1.0f-15*pad, 1.0f);
  TextRendering_PrintString(window, "                            V              ", -1.0f, 1.0f-16*pad, 1.0f);

  TextRendering_PrintString(window, " Projection matrix        Camera                    In NDC", -1.0f, 1.0f-17*pad, 1.0f);
  TextRendering_PrintMatrixVectorProductDivW(window, projection, p_camera, -1.0f, 1.0f-18*pad, 1.0f);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  glm::vec2 a = glm::vec2(-1, -1);
  glm::vec2 b = glm::vec2(+1, +1);
  glm::vec2 p = glm::vec2( 0,  0);
  glm::vec2 q = glm::vec2(width, height);

  glm::mat4 viewport_mapping = Matrix(
				      (q.x - p.x)/(b.x-a.x), 0.0f, 0.0f, (b.x*p.x - a.x*q.x)/(b.x-a.x),
				      0.0f, (q.y - p.y)/(b.y-a.y), 0.0f, (b.y*p.y - a.y*q.y)/(b.y-a.y),
				      0.0f , 0.0f , 1.0f , 0.0f ,
				      0.0f , 0.0f , 0.0f , 1.0f
				      );

  TextRendering_PrintString(window, "                                                       |  ", -1.0f, 1.0f-22*pad, 1.0f);
  TextRendering_PrintString(window, "                            .--------------------------'  ", -1.0f, 1.0f-23*pad, 1.0f);
  TextRendering_PrintString(window, "                            V                           ", -1.0f, 1.0f-24*pad, 1.0f);

  TextRendering_PrintString(window, " Viewport matrix           NDC      In Pixel Coords.", -1.0f, 1.0f-25*pad, 1.0f);
  TextRendering_PrintMatrixVectorProductMoreDigits(window, viewport_mapping, p_ndc, -1.0f, 1.0f-26*pad, 1.0f);
}

void TextRendering_ShowEulerAngles(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  float pad = TextRendering_LineHeight(window);

  char buffer[80];
  snprintf(buffer, 80, "Euler Angles rotation matrix = Z(%.2f)*Y(%.2f)*X(%.2f)\n", g_AngleZ, g_AngleY, g_AngleX);

  TextRendering_PrintString(window, buffer, -1.0f+pad/10, -1.0f+2*pad/10, 1.0f);
}

void TextRendering_ShowProjection(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  float lineheight = TextRendering_LineHeight(window);
  float charwidth = TextRendering_CharWidth(window);

  if ( g_UsePerspectiveProjection )
    TextRendering_PrintString(window, "Perspective", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
  else
    TextRendering_PrintString(window, "Orthographic", 1.0f-13*charwidth, -1.0f+2*lineheight/10, 1.0f);
}

void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
  if ( !g_ShowInfoText )
    return;

  static float old_seconds = (float)glfwGetTime();
  static int   ellapsed_frames = 0;
  static char  buffer[20] = "?? fps";
  static int   numchars = 7;

  ellapsed_frames += 1;

  float seconds = (float)glfwGetTime();

  float ellapsed_seconds = seconds - old_seconds;

  if ( ellapsed_seconds > 1.0f )
    {
      numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

      old_seconds = seconds;
      ellapsed_frames = 0;
    }

  float lineheight = TextRendering_LineHeight(window);
  float charwidth = TextRendering_CharWidth(window);

  TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}
