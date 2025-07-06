#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#define NORMAL 0
#define FLIPPED 1

#include "configs.h"

int CheckColisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane);
int CheckColisionAABBtoAABB(SceneObject obj1, glm::mat4 transf1, SceneObject obj2, glm::mat4 transf2);
int CheckColisionAABBtoPlane(BoundingBox box, glm::vec4 pn, float pd);
int CheckColisionPointWalls(glm::vec4 point);

BoundingBox GetBboxModel(ObjModel* model, glm::mat4 transformation);

int CheckColisionAABBtoAABB(SceneObject obj1, glm::mat4 transf1, SceneObject obj2, glm::mat4 transf2){

  BoundingBox obj1b = {transf1 * glm::vec4(obj1.bbox_min.x, obj1.bbox_min.y, obj1.bbox_min.z, 1), transf1 * glm::vec4(obj1.bbox_max.x, obj1.bbox_max.y, obj1.bbox_max.z, 1)};
  BoundingBox obj2b = {transf2 * glm::vec4(obj2.bbox_min.x, obj2.bbox_min.y, obj2.bbox_min.z, 2), transf2 * glm::vec4(obj2.bbox_max.x, obj2.bbox_max.y, obj2.bbox_max.z, 2)};

  int collisionX = obj1b.max.x >= obj2b.min.x && obj2b.max.x >= obj1b.min.x;

  int collisionY = obj1b.max.y >= obj2b.min.y && obj2b.max.y >= obj1b.min.y;

  int collisionZ = obj1b.max.z >= obj2b.min.z && obj2b.max.z >= obj1b.min.z;

  return collisionX && collisionY && collisionZ;

}


int CheckColisionAABBtoPlane(BoundingBox box, glm::vec4 pn, float pd){

  glm::vec4 center = (box.max + box.min)/2.0f;
  glm::vec4 e = box.max - center;

  float r = e.x * abs(pn.x) + e.y * abs(pn.y) + e.z * abs(pn.z);

  float s = dotproduct(pn, center) - pd;

  return abs(s) <= r;

}


int CheckColisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane){

  //                    moving the point to not enter in the wall
  glm::vec4 v = point - (point_in_plane + (pn/norm(pn))*1.1f);

  float d = dotproduct(pn, v);

  if(d > 0.01f){
    return 0;
  }

  return 1;

}

glm::vec4 GetNormalWall(BoundingBox wall, int flip){

  glm::vec4 wall_normal;

  if(!flip){
    wall_normal = -crossproduct(glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.max,
				glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.min);
  } else {
    wall_normal = -crossproduct(glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.max,
				glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.min);
  }

  return wall_normal;

}

int CheckColisionPointWalls(glm::vec4 point){

  BoundingBox wallPoints = {glm::vec4(g_VirtualScene["the_wall"].bbox_min.x, g_VirtualScene["the_wall"].bbox_min.y, g_VirtualScene["the_wall"].bbox_min.z, 1),
			    glm::vec4(g_VirtualScene["the_wall"].bbox_max.x, g_VirtualScene["the_wall"].bbox_max.y, g_VirtualScene["the_wall"].bbox_max.z, 1)};

  glm::mat4 wallDefault = Matrix_Scale(10.0f, 10.0f, 0.0f);

  // Testing for the first wall
  glm::mat4 wallModel = Matrix_Translate(-30.0f, 0.0f, -30.0f) * wallDefault;
  BoundingBox wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  glm::vec4 wall_normal = GetNormalWall(wall, NORMAL);
  int c1 = CheckColisionPointToPlane(point, wall_normal, wall.min);

  // Testing for the second wall
  wallModel = Matrix_Translate(-30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall, FLIPPED);
  int c2 = CheckColisionPointToPlane(point, wall_normal, wall.min);

  // Testing for the third wall
  wallModel = Matrix_Translate(30.0f, 0.0f, -30.0f) *  Matrix_Rotate_Y(-3.141592f / 2) *  wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall, FLIPPED);
  int c3 = CheckColisionPointToPlane(point, wall_normal, wall.min);


  // Testing for the forth wall
  wallModel =  Matrix_Translate(30.0f, 0.0f, 30.0f) * Matrix_Rotate_Y(-3.141592f) * wallDefault;
  wall = {wallModel * wallPoints.min, wallModel * wallPoints.max};
  wall_normal = GetNormalWall(wall, NORMAL);
  int c4 = CheckColisionPointToPlane(point, wall_normal, wall.min);

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
