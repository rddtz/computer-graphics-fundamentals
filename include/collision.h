#ifndef _COLLISIONS_H
#define _COLLISIONS_H

#include "configs.h"

glm::vec4 GetNormal(BoundingBox wall);

// AABB related collision
int CheckCollisionAABBtoAABB(BoundingBox obj1b, BoundingBox obj2b);
int CheckCollisionAABBtoPlane(BoundingBox box, glm::vec4 pn, float pd);
int CheckCollisionPointToAABB(SceneObject obj1, glm::mat4 transf1, glm::vec4 point);

// Points Collision
int CheckCollisionPointWalls(glm::vec4 point);
int CheckCollisionPlayerFloor(BoundingBox player);
int CheckCollisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane, float offset);


int CheckCollisionPlayerPortal(glm::vec4 player_point, glm::mat4 portal_transform);

std::pair<glm::vec4, glm::vec4> CheckCollisionLineToWalls(glm::vec4 camera_position, glm::vec4 view_vector);

BoundingBox GetBoundingBoxObject(const char* name);
BoundingBox GetBboxModel(ObjModel* model, glm::mat4 transformation);
void SetWallsInfo();

BoundingBox wallPoints;

#define N_WALLS 4
#define N_FLOORS 3

BoundingBox lower_walls[N_WALLS];
BoundingBox higher_walls[N_WALLS];
BoundingBox floors[N_FLOORS];


BoundingBox GetBoundingBoxObject(const char* name){

  BoundingBox obj = {glm::vec4(g_VirtualScene[name].bbox_min.x, g_VirtualScene[name].bbox_min.y, g_VirtualScene[name].bbox_min.z, 1),
		     glm::vec4(g_VirtualScene[name].bbox_max.x, g_VirtualScene[name].bbox_max.y, g_VirtualScene[name].bbox_max.z, 1)};

  float maxx = std::max(obj.max.x, obj.min.x);
  float maxy = std::max(obj.max.y, obj.min.y);
  float maxz = std::max(obj.max.z, obj.min.z);

  float minx = std::min(obj.max.x, obj.min.x);
  float miny = std::min(obj.max.y, obj.min.y);
  float minz = std::min(obj.max.z, obj.min.z);

  obj = {glm::vec4(minx, miny, minz, 1), glm::vec4(maxx, maxy, maxz, 1)};

  return obj;

}

int CheckCollisionAABBtoAABB(BoundingBox obj1b, BoundingBox obj2b){

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

  BoundingBox portalPoints = GetBoundingBoxObject("the_portal");
  /* {glm::vec4(g_VirtualScene["the_portal"].bbox_min.x, g_VirtualScene["the_portal"].bbox_min.y, g_VirtualScene["the_portal"].bbox_min.z, 1), */
  /* 			    glm::vec4(g_VirtualScene["the_portal"].bbox_max.x, g_VirtualScene["the_portal"].bbox_max.y, g_VirtualScene["the_portal"].bbox_max.z, 1)}; */

  BoundingBox portal = {portal_transform * portalPoints.min, portal_transform * portalPoints.max};

  glm::vec4 portal_normal = GetNormal(portal);


  float maxy = std::max(portal.max.y, portal.min.y);
  float miny = std::min(portal.max.y, portal.min.y);

  if(player_point.y <= maxy && player_point.y >= miny){
    // Portal can only be aligned with z axis or x axis because we can't have portals in the floor or ceil
    if(abs(portal_normal.x) > 0.01f){ // normal is x, portal is aligned with z

      float maxz = std::max(portal.max.z, portal.min.z);
      float minz = std::min(portal.max.z, portal.min.z);

      if(player_point.z <= maxz && player_point.z >= minz){

	return CheckCollisionPointToPlane(player_point, portal_normal, (portal.max + portal.min)/2.0f, 1.1);
      }
    } else { // normal is z, portal is aligned with x

      float maxx = std::max(portal.max.x, portal.min.x);
      float minx = std::min(portal.max.x, portal.min.x);

      if(player_point.x <= maxx && player_point.x >= minx){
	return CheckCollisionPointToPlane(player_point, portal_normal, (portal.max + portal.min)/2.0f, 1.1);
      }
    }
  }

  return 0;

}

int CheckCollisionPointToAABB(SceneObject obj1, glm::mat4 transf1, glm::vec4 point){

  BoundingBox aabb = {transf1 * glm::vec4(obj1.bbox_min.x, obj1.bbox_min.y, obj1.bbox_min.z, 1), transf1 * glm::vec4(obj1.bbox_max.x, obj1.bbox_max.y, obj1.bbox_max.z, 1)};

  int collisionX = point.x >= aabb.min.x && aabb.max.x >= point.x;

  int collisionY = point.y >= aabb.min.y && point.y <= aabb.max.y;

  int collisionZ = point.z >= aabb.min.z && aabb.max.z >= point.z;

  return collisionX && collisionY && collisionZ;

}


/* Using line-to-plane collision to check the gun shot into the wall */
std::pair<glm::vec4, glm::vec4> CheckCollisionLineToWalls(glm::vec4 camera_position, glm::vec4 view_vector){

  float intersection_point = 10000;
  glm::vec4 hit_wall_normal;
  int index = 0;
  glm::vec4 point;

  bool usingLowers = true;

  for(int i = 0; i < N_WALLS; i++){

    glm::vec4 wall_normal = GetNormal(lower_walls[i]);

    float nd = dotproduct(view_vector, wall_normal);
    float pn = dotproduct(camera_position, wall_normal);

    if (nd >= 0.0f) {
      // it would be a return
    } else {

      float t = (dotproduct(wall_normal, lower_walls[i].min) - pn) / nd;
      if (t >= 0.0f) {
	if(t < intersection_point){
	  index = i;
	  intersection_point = t;
	  hit_wall_normal = wall_normal;
	}
      }
    }
  }

  point = camera_position + intersection_point*view_vector;

  if(point.y > lower_walls[index].max.y || intersection_point == 100000){

    intersection_point = 10000;
    index = 0;

    for(int i = 0; i < N_WALLS; i++){

      glm::vec4 wall_normal = GetNormal(higher_walls[i]);

      float nd = dotproduct(view_vector, wall_normal);
      float pn = dotproduct(camera_position, wall_normal);

      if (nd >= 0.0f) {
	// it would be a return
      } else {

	float t = (dotproduct(wall_normal, higher_walls[i].min) - pn) / nd;
	if (t >= 0.0f) {
	  if(t < intersection_point){
	    index = i;
	    intersection_point = t;
	    hit_wall_normal = wall_normal;
	  }
	}
      }
    }
  } else {
    if(abs(point.z) > 10){
      return {glm::vec4(-1,-1,-1,-1), glm::vec4(-1,-1,-1,-1)};
    }
  }

  point = camera_position + intersection_point*view_vector;
  if(intersection_point == 10000){
    return {glm::vec4(-1,-1,-1,-1), glm::vec4(-1,-1,-1,-1)};
  } else{
    point = camera_position + intersection_point*view_vector;

    if(!usingLowers && (point.y > higher_walls[index].max.y || point.y < higher_walls[index].min.y)){
      return {glm::vec4(-1,-1,-1,-1), glm::vec4(-1,-1,-1,-1)};
    }
  }

  point = camera_position + intersection_point*view_vector;
  std::pair<glm::vec4, glm::vec4> ret = {point, hit_wall_normal};

  return ret;

}

/* Using Point-to-Plane collision to do the wall and floor colisions */
int CheckCollisionPointToPlane(glm::vec4 point, glm::vec4 pn, glm::vec4 point_in_plane, float offset){

  //                    moving the point to not enter in the wall
  glm::vec4 v = point - (point_in_plane + (pn/norm(pn))*offset);

  float d = dotproduct(pn, v);


  if(d > 0.01f){
    return 0;
  }

  return 1;
}


glm::vec4 GetNormal(BoundingBox wall){

  glm::vec4 normal;

  normal = -crossproduct(glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.max,
			 glm::vec4(wall.min.x, wall.max.y, wall.max.z, 1) - wall.min);

  if(abs(normal.x) < 0.01f && abs(normal.y) < 0.01f && abs(normal.z) < 0.01f){
    normal = -crossproduct(glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.max,
			   glm::vec4(wall.max.x, wall.max.y, wall.min.z, 1) - wall.min);
  }

  return normal;

}

int CheckCollisionPlayerFloor(BoundingBox player){


  glm::vec4 point = glm::vec4((player.max.x + player.min.x)/2, player.min.y, (player.max.z + player.min.z)/2, 1);
  int res = 0;

  for(int i= 0; i < N_FLOORS; i++){

    res = 0;

    glm::vec4 floor_normal = GetNormal(floors[i]);

    float maxx = std::max(floors[i].max.x, floors[i].min.x);
    float minx = std::min(floors[i].max.x, floors[i].min.x);
    float maxz = std::max(floors[i].max.z, floors[i].min.z);
    float minz = std::min(floors[i].max.z, floors[i].min.z);


    if(point.x >= minx && point.x <= maxx && point.z >= minz && point.z <= maxz){
      res = point.y - 0.1 <= floors[i].max.y;  /* point.y > floors[i].max.y && */
    }

    /* if(){ */
    /*   res = 1; */
    /* } */

    if(res == 1){
      return 1;
    }
  }

  player.min.y = player.min.y - 0.1;
  res = CheckCollisionAABBtoAABB(player, g_MovingPlatform);

  if(res == 1){
    return 1;
  }

  BoundingBox fix_platform = GetBoundingBoxObject("platform");
  glm::mat4 platform_trasform = Matrix_Translate(0.0f, 13.0f, 0.0f) * Matrix_Scale(0.025f, 0.025f, 0.025f);
  fix_platform = {platform_trasform * fix_platform.min, platform_trasform * fix_platform.max};

  res = CheckCollisionAABBtoAABB(player, fix_platform) && (point.y + 0.1 > fix_platform.max.y);

  if(res == 1){
    return 1;
  }

  return 0;

}

int CheckCollisionPointWalls(glm::vec4 point){

  for(int i = 0; i < N_WALLS; i++){

    glm::vec4 wall_normal = GetNormal(higher_walls[i]);

    int res = 0;

    if(point.y >= higher_walls[i].min.y && point.y <= higher_walls[i].max.y){

      float maxx = std::max(higher_walls[i].max.x, higher_walls[i].min.x);
      float minx = std::min(higher_walls[i].max.x, higher_walls[i].min.x);
      float maxz = std::max(higher_walls[i].max.z, higher_walls[i].min.z);
      float minz = std::min(higher_walls[i].max.z, higher_walls[i].min.z);

      if(abs(wall_normal.z) >= 0.001f){
	if(point.x >= minx && point.y <= maxx){
	  res = CheckCollisionPointToPlane(point, wall_normal, higher_walls[i].min, 1.0);
	}
      } else {
	if(point.z >= minz && point.z <= maxz){
	  res = CheckCollisionPointToPlane(point, wall_normal, higher_walls[i].min, 1.0);
	}
      }

      if(res == 1){
	return 1;
      }
    }
  }


  for(int i = 0; i < N_WALLS; i++){

    glm::vec4 wall_normal = GetNormal(lower_walls[i]);

    int res = 0;

    if(abs(wall_normal).z > 0.001){

      if(point.y < lower_walls[i].max.y){
	 res = CheckCollisionPointToPlane(point, wall_normal, lower_walls[i].min, 1.1);
      }
    } else {
       res = CheckCollisionPointToPlane(point, wall_normal, lower_walls[i].min, 1.1);
    }

    if(res == 1){
      return 1;
    }
  }


  /* std::cout << c1 << c2 << c3 << c4 << "\n"; */
  //if at leat one collision, the player cant move
  return 0;

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


  lower_walls[0] =  {Matrix_Translate(-30.0f, -20.0f, -10.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		     Matrix_Translate(-30.0f, -20.0f, -10.0f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  lower_walls[1] = {Matrix_Translate(-30.0f, -20.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		    Matrix_Translate(-30.0f, -20.0f, 30.0f) * Matrix_Rotate_Y(3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  lower_walls[2] = {Matrix_Translate(30.0f, -20.0f, -30.0f) * Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		    Matrix_Translate(30.0f, -20.0f, -30.0f) * Matrix_Rotate_Y(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  lower_walls[3] = {Matrix_Translate(30.0f, -20.0f, 10.0f) * Matrix_Rotate_Y(3.141592f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
		    Matrix_Translate(30.0f, -20.0f, 10.0f) * Matrix_Rotate_Y(3.141592f) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};


  floors[0] = {Matrix_Translate(-30.0, 0.0f, -10.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
	       Matrix_Translate(-30.0, 0.0f, -10.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};

  floors[1] = {Matrix_Translate(-30.0, -20.0f, 10.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
	       Matrix_Translate(-30.0, -20.0f, 10.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};


  floors[2] = {Matrix_Translate(-30.0, 0.0f, 30.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.min,
	       Matrix_Translate(-30.0, 0.0f, 30.0f) * Matrix_Rotate_X(-3.141592f / 2) * Matrix_Scale(10.0f, 10.0f, 0.0f) * wallPoints.max};
}



#endif
