#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "configs.h"

glm::vec4 GetNormalWall(BoundingBox wall);

int CheckCollisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane, float offset);
int CheckCollisionAABBtoAABB(SceneObject obj1, glm::mat4 transf1, SceneObject obj2, glm::mat4 transf2);
int CheckCollisionAABBtoPlane(BoundingBox box, glm::vec4 pn, float pd);
int CheckCollisionPointWalls(glm::vec4 point);
int CheckCollisionPlayerPortal(glm::vec4 player_point, glm::mat4 portal_transform);

std::pair<glm::vec4, glm::vec4> CheckCollisionLineToWalls(glm::vec4 camera_position, glm::vec4 view_vector);

BoundingBox GetBboxModel(ObjModel* model, glm::mat4 transformation);
void SetWallsInfo();

BoundingBox wallPoints;

#define N_WALLS 4
BoundingBox higher_walls[4];


void SetWallsInfo(){

  wallPoints = {glm::vec4(g_VirtualScene["the_wall"].bbox_min.x, g_VirtualScene["the_wall"].bbox_min.y, g_VirtualScene["the_wall"].bbox_min.z, 1),
		glm::vec4(g_VirtualScene["the_wall"].bbox_max.x, g_VirtualScene["the_wall"].bbox_max.y, g_VirtualScene["the_wall"].bbox_max.z, 1)};

  higher_walls[0] =  {Matrix_Translate(-30.0f, 0.0f, -30.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		      Matrix_Translate(-30.0f, 0.0f, -30.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  higher_walls[1] = {Matrix_Translate(-30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		     Matrix_Translate(-30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  higher_walls[2] = {Matrix_Translate(30.0f, 0.0f, -30.0f) *  Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		     Matrix_Translate(30.0f, 0.0f, -30.0f) *  Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  higher_walls[3] = {Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-3.141592f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		     Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-3.141592f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

}


int CheckCollisionAABBtoAABB(SceneObject obj1, glm::mat4 transf1, SceneObject obj2, glm::mat4 transf2){

  BoundingBox obj1b = {transf1 * glm::vec4(obj1.bbox_min.x, obj1.bbox_min.y, obj1.bbox_min.z, 1), transf1 * glm::vec4(obj1.bbox_max.x, obj1.bbox_max.y, obj1.bbox_max.z, 1)};
  BoundingBox obj2b = {transf2 * glm::vec4(obj2.bbox_min.x, obj2.bbox_min.y, obj2.bbox_min.z, 2), transf2 * glm::vec4(obj2.bbox_max.x, obj2.bbox_max.y, obj2.bbox_max.z, 2)};

  int collisionX = obj1b.max.x >= obj2b.min.x && obj2b.max.x >= obj1b.min.x;

  int collisionY = obj1b.max.y >= obj2b.min.y && obj2b.max.y >= obj1b.min.y;

  int collisionZ = obj1b.max.z >= obj2b.min.z && obj2b.max.z >= obj1b.min.z;

  return collisionX && collisionY && collisionZ;

}


int CheckCollisionAABBtoPlane(BoundingBox box, glm::vec4 pn, float pd){

  glm::vec4 center = (box.max + box.min)/2.0f;
  glm::vec4 e = box.max - center;

  float r = e.x * abs(pn.x) + e.y * abs(pn.y) + e.z * abs(pn.z);

  float s = dotproduct(pn, center) - pd;

  return abs(s) <= r;

}

int CheckCollisionPlayerPortal(glm::vec4 player_point, glm::mat4 portal_transform){

  BoundingBox portalPoints = {glm::vec4(g_VirtualScene["the_portal"].bbox_min.x, g_VirtualScene["the_portal"].bbox_min.y, g_VirtualScene["the_portal"].bbox_min.z, 1),
			    glm::vec4(g_VirtualScene["the_portal"].bbox_max.x, g_VirtualScene["the_portal"].bbox_max.y, g_VirtualScene["the_portal"].bbox_max.z, 1)};

  BoundingBox portal = {portal_transform * portalPoints.min, portal_transform * portalPoints.max};

  glm::vec4 portal_normal = GetNormalWall(portal);

  // Portal can only be aligned with z axis or x axis because we can't have portals in the floor or ceil
  if(abs(portal_normal.x) > 0.01f){ // normal is x, portal is aligned with z

    float maxz = std::max(portal.max.z, portal.min.z);
    float minz = std::min(portal.max.z, portal.min.z);

    if(player_point.z <= maxz && player_point.z >= minz){
      printf("Validou o Z quando entrou no X");
      return CheckCollisionPointToPlane(player_point, portal_normal, (portal.max + portal.min)/2.0f, 1.1);
    }
  } else { // normal is z, portal is aligned with x

    float maxx = std::max(portal.max.x, portal.min.x);
    float minx = std::min(portal.max.x, portal.min.x);

    if(player_point.x <= maxx && player_point.x >= minx){
      return CheckCollisionPointToPlane(player_point, portal_normal, (portal.max + portal.min)/2.0f, 1.1);
    }
  }

  return 0;

}


std::pair<glm::vec4, glm::vec4> CheckCollisionLineToWalls(glm::vec4 camera_position, glm::vec4 view_vector){

  float intersection_point = 10000;
  glm::vec4 hit_wall_normal;

  for(int i = 0; i < N_WALLS; i++){

    glm::vec4 wall_normal = GetNormalWall(higher_walls[i]);

    float nd = dotproduct(view_vector, wall_normal);
    float pn = dotproduct(camera_position, wall_normal);

    if (nd >= 0.0f) {
      // it would be a return
    } else {

      float t = (dotproduct(wall_normal, higher_walls[i].min) - pn) / nd;
      if (t >= 0.0f) {
	intersection_point = std::min(intersection_point, t);
	hit_wall_normal = wall_normal;
      }
    }

  }

  glm::vec4 point;

  if(intersection_point == 10000){
    return {glm::vec4(-1,-1,-1,-1), glm::vec4(-1,-1,-1,-1)};
  } else{
    glm::vec4 point = camera_position + intersection_point*view_vector;

    if(point.y > higher_walls[0].max.y || point.y < -20){
      return {glm::vec4(-1,-1,-1,-1), glm::vec4(-1,-1,-1,-1)};
    }
  }

  std::pair<glm::vec4, glm::vec4> ret = {point, hit_wall_normal};

  return ret;

}

int CheckCollisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane, float offset){

  //                    moving the point to not enter in the wall
  glm::vec4 v = point - (point_in_plane + (pn/norm(pn))*offset);

  float d = dotproduct(pn, v);


  if(d > 0.01f){
    return 0;
  }

  return 1;

}

glm::vec4 GetNormalWall(BoundingBox wall){

  glm::vec4 normal;

  normal = -crossproduct(glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.max,
			 glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.min);

  if(abs(normal.x) < 0.01f && abs(normal.y) < 0.01f && abs(normal.z) < 0.01f){
    normal = -crossproduct(glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.max,
			   glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.min);
  }

  return normal;

}

int CheckCollisionPointWalls(glm::vec4 point){

  BoundingBox wallPoints = {glm::vec4(g_VirtualScene["the_wall"].bbox_min.x, g_VirtualScene["the_wall"].bbox_min.y, g_VirtualScene["the_wall"].bbox_min.z, 1),
			    glm::vec4(g_VirtualScene["the_wall"].bbox_max.x, g_VirtualScene["the_wall"].bbox_max.y, g_VirtualScene["the_wall"].bbox_max.z, 1)};

  glm::mat4 wallDefault = Matrix_Scale(10.0f, 10.0f, 0.0f);

  // Testing for the first wall
  glm::mat4 wallModel = Matrix_Translate(-30.0f, 0.0f, -30.0f) * wallDefault;
  BoundingBox wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  glm::vec4 wall_normal = GetNormalWall(wall);
  int c1 = CheckCollisionPointToPlane(point, wall_normal, wall.min, 1.1);

  // Testing for the second wall
  wallModel = Matrix_Translate(-30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall);
  int c2 = CheckCollisionPointToPlane(point, wall_normal, wall.min, 1.1);

  // Testing for the third wall
  wallModel = Matrix_Translate(30.0f, 0.0f, -30.0f) *  Matrix_Rotate_Y(-3.141592f / 2) *  wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall);
  int c3 = CheckCollisionPointToPlane(point, wall_normal, wall.min, 1.1);


  // Testing for the forth wall
  wallModel =  Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-3.141592f) * wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall);
  int c4 = CheckCollisionPointToPlane(point, wall_normal, wall.min, 1.1);

  /* std::cout << c1 << c2 << c3 << c4 << "\n"; */
  //if at leat one collision, the player cant move
  return c1 || c2 || c3 || c4;

}


/*Get the perfect bounding box for the object at the moment */
BoundingBox GetBboxModel(ObjModel* model, glm::mat4 transformation){

   GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

  size_t first_index = indices.size();
  size_t num_triangles = model->shapes[0].mesh.num_face_vertices.size();

  const float minval = std::numeric_limits<float>::min();
  const float maxval = std::numeric_limits<float>::max();

  glm::vec4 bbox_min = glm::vec4(maxval,maxval,maxval, 1);
  glm::vec4 bbox_max = glm::vec4(minval,minval,minval, 1);

  for (size_t triangle = 0; triangle < num_triangles; ++triangle)
    {
      assert(model->shapes[0].mesh.num_face_vertices[triangle] == 3);

      for (size_t vertex = 0; vertex < 3; ++vertex)
	{
	  tinyobj::index_t idx = model->shapes[0].mesh.indices[3*triangle + vertex];

	  indices.push_back(first_index + 3*triangle + vertex);

	  glm::vec4 v = transformation * glm::vec4(model->attrib.vertices[3*idx.vertex_index + 0],
						   model->attrib.vertices[3*idx.vertex_index + 1],
						   model->attrib.vertices[3*idx.vertex_index + 2],
						   1);


	  bbox_min.x = std::min(bbox_min.x, v.x);
	  bbox_min.y = std::min(bbox_min.y, v.y);
	  bbox_min.z = std::min(bbox_min.z, v.z);
	  bbox_max.x = std::max(bbox_max.x, v.x);
	  bbox_max.y = std::max(bbox_max.y, v.y);
	  bbox_max.z = std::max(bbox_max.z, v.z);


	}
    }

  BoundingBox ret = {bbox_min, bbox_max};
  return ret;

}

#endif
