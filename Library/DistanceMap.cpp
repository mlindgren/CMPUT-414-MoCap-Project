#include <Library/DistanceMap.hpp>
#include <Graphics/Graphics.hpp>
#include <Vector/VectorGL.hpp>
#include <Vector/QuatGL.hpp>

#include <cstring>

using namespace Character;
using std::vector;

namespace Library
{

DistanceMap::DistanceMap(Motion &f, Motion &t)
: from(f),
  to(t)
{
  distances = new float[from.frames() * to.frames()];
  memset(distances, 0, from.frames() * to.frames() * sizeof(float));

  // Initialize and clear from and to poses
  // Source and destination probably would have been better words
  // than from and to
  Pose from_pose;
  Pose to_pose;
  from_pose.clear();
  to_pose.clear();

  vector<Vector3f> from_pos_vector;
  vector<Vector3f> to_pos_vector;

  // Hm, this is going to be slow...
  for(unsigned int i = 0; i < from.frames(); ++i)
  {
    for(unsigned int j = 0; j < to.frames(); ++j)
    {
      from_pos_vector.clear();
      to_pos_vector.clear();

      from.get_pose(i, from_pose);
      to.get_pose(j, to_pose);

      getJointPositions(from_pose, from_pos_vector);
      getJointPositions(to_pose, to_pos_vector);

      assert(from_pos_vector.size() == to_pos_vector.size());

      for(unsigned int k = 0; k < from_pos_vector.size(); ++k)
      {
        *(getDistance(i, j)) += length_squared(from_pos_vector[k] - to_pos_vector[k]);
      }
    }
  }

  /* Testing
  Pose a;
  a.clear();

  from.get_pose(0, a);
  
  vector<Vector3f> position_vector;
  getJointPositions(a, position_vector);

  for(vector<Vector3f>::iterator it = position_vector.begin();
      it < position_vector.end();
      ++it)
  {
    std::cerr << "(" << it->x << ", " << it->y << ", " << it->z << ") ";
  }
  */

}

DistanceMap::DistanceMap(const DistanceMap &other)
: from(other.from),
  to(other.to)
{
  distances = new float[from.frames() * to.frames()];
  memcpy(distances, other.distances, 
         from.frames() * to.frames() * sizeof(float));
}

DistanceMap& DistanceMap::operator= (const DistanceMap &other)
{
  if(this == &other) return *this;

  delete[] distances;

  from = other.from;
  to = other.to;

  distances = new float[from.frames() * to.frames()];
  memcpy(distances, other.distances, 
         from.frames() * to.frames() * sizeof(float));

  return *this;
}

DistanceMap::~DistanceMap()
{
  delete[] distances;
}

float* DistanceMap::getDistance(unsigned int from_frame, unsigned int to_frame)
{
  return &(distances[from_frame + to_frame * from.frames()]);
}

void DistanceMap::getJointPositions(Pose const &pose, 
                                    vector<Vector3f> &positions)
{

  /* This code is mostly copypasta'd from Chracter/Draw.cpp, because we're using
   * OpenGL to calculate the joint positions from the ModelView matrix.  This is
   * kind of a huge hack, but it's also the fastest and easiest way I can think
   * of to do this. */

  glPushMatrix();
  
  /* IGNORING root position and orientation.  Not sure if this is actually okay
   * to do
  glTranslate(state.position);
  glRotatef(state.orientation * 180.0f / (float)M_PI, 0.0f, 1.0f, 0.0f);
  glTranslate(pose.root_position);
  glRotate(pose.root_orientation); */

  vector< int > parent_stack;
  parent_stack.push_back(-1);
  for (unsigned int b = 0; b < pose.bone_orientations.size(); ++b)
  {
    while(!parent_stack.empty() && parent_stack.back() != pose.skeleton->bones[b].parent)
    {
      glPopMatrix();
      parent_stack.pop_back();
    }
    assert(!parent_stack.empty());

    glPushMatrix();
    glRotate(pose.bone_orientations[b]);

    Vector3d t = pose.skeleton->bones[b].direction;
    Vector3d t2;

    if (t.x == t.y && t.y == t.z)
    {
      t2 = normalize(cross_product(t, make_vector(t.y,t.z,-t.x)));
    }
    else
    {
      t2 = normalize(cross_product(t, make_vector(t.y,t.z,t.x)));
    }

    Vector3d t3 = cross_product(t2, t);
    glPushMatrix();
    double mat[16] =
    {
      t2.x, t2.y, t2.z, 0,
      t3.x, t3.y, t3.z, 0,
      t.x, t.y, t.z, 0,
      0,0,0,1
    };
    glMultMatrixd(mat);
    
    float mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    positions.push_back(make_vector(mv[12], mv[13], mv[14]));

    /* Fairly certain the rotate/translate calls in this block are 
     * unnecessary for getting joint positions (I think they cancel
     * each other out), but I'm leaving them in
     * for now to be safe */
    {
      /*
      gluCylinder(quad, pose.skeleton->bones[b].radius, 
                  pose.skeleton->bones[b].radius, 
                  pose.skeleton->bones[b].length, detail?16:8, 1); 
      */
      glRotated(180,1,0,0);
      //gluDisk(quad, 0, pose.skeleton->bones[b].radius, detail?16:8, 1);
      glRotated(180,1,0,0);
      glTranslated(0, 0, pose.skeleton->bones[b].length);
      //gluDisk(quad, 0, pose.skeleton->bones[b].radius, detail?16:8, 1);
      glTranslated(-pose.skeleton->bones[b].radius, 0, -pose.skeleton->bones[b].length);
    }

    glPopMatrix();

    glTranslate(pose.skeleton->bones[b].direction * pose.skeleton->bones[b].length);
    parent_stack.push_back(b);
  } // end for

  while (!parent_stack.empty())
  {
    glPopMatrix();
    parent_stack.pop_back();
  }

}

ostream& operator<<(ostream &out, DistanceMap &map)
{
  for(unsigned int i = 0; i < map.from.frames(); ++i)
  {
    for(unsigned int j = 0; j < map.to.frames(); ++j)
    {
      out << *(map.getDistance(i, j)) << ", ";
    }

    out << endl;
  }

  return out;
}

}
