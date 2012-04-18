#include <Library/DistanceMap.hpp>
#include <Graphics/Graphics.hpp>
#include <Vector/VectorGL.hpp>
#include <Vector/QuatGL.hpp>

#include <cstring>
#include <cassert>
#include <algorithm>

#define UNINITIALIZED -1.0f

using namespace Character;
using namespace std;

namespace Library
{

DistanceMap::DistanceMap(const Motion *f, const Motion *t)
: from(f),
  to(t)
{
  distances = new float[from->frames() * to->frames()];
  for(unsigned int i = 0; i < from->frames() * to->frames(); ++i)
    distances[i] = UNINITIALIZED;

  // Mustn't have null motions
  assert(from != NULL);
  assert(to != NULL);
}

DistanceMap::DistanceMap(const DistanceMap &other)
: shortest_path(other.shortest_path),
  from(other.from),
  to(other.to)
{
  distances = new float[from->frames() * to->frames()];
  memcpy(distances, other.distances, 
         from->frames() * to->frames() * sizeof(float));
}

DistanceMap& DistanceMap::operator= (const DistanceMap &other)
{
  if(this == &other) return *this;

  delete[] distances;

  from = other.from;
  to = other.to;

  distances = new float[from->frames() * to->frames()];
  memcpy(distances, other.distances, 
         from->frames() * to->frames() * sizeof(float));

  shortest_path = other.shortest_path;

  return *this;
}

DistanceMap::~DistanceMap()
{
  delete[] distances;
}

float* DistanceMap::getDistance(unsigned int from_frame, unsigned int to_frame)
{

  // Determine the address of the distance in the map.  If it's already been
  // initialized, we can return the value immediately.
  float *addr =  &(distances[from_frame + to_frame * from->frames()]);
  if(*addr != UNINITIALIZED) return addr;

  // If the distance is uninitialized, we need to calculate the distance
  // between the two frames.  First initialize and clear from and to poses
  Pose from_pose;
  Pose to_pose;
  from_pose.clear();
  to_pose.clear();

  vector<Vector3f> from_pos_vector;
  vector<Vector3f> to_pos_vector;

  from_pos_vector.clear();
  to_pos_vector.clear();

  from->get_pose(from_frame, from_pose);
  to->get_pose(to_frame, to_pose);

  getJointPositions(from_pose, from_pos_vector);
  getJointPositions(to_pose, to_pos_vector);

  assert(from_pos_vector.size() == to_pos_vector.size());

  for(unsigned int k = 0; k < from_pos_vector.size(); ++k)
  {
    /* TODO: these distances should probably be weighted according to the
     * length or density, or perhaps most logically weight 
     * (volume * density) of the bones, so that distance between large 
     * bones is "more important" than distance between small bones when
     * when calculating the shortest path. */
    *addr += length_squared(from_pos_vector[k] - to_pos_vector[k]);
  }

  return addr;
}

void DistanceMap::getJointPositions(Pose const &pose, 
                                    vector<Vector3f> &positions) const
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

void DistanceMap::calcShortestPath(unsigned int n_interp_frames)
{
  shortest_path.clear();

  // The Kristine Slot paper suggests a slope limit of 3 frames
  static const unsigned int slope_limit = 3;

  // Start at the zeroth frame of the first (from) animation
  unsigned int from_frame = 0;

  // Going to always choose 0, 0 as a starting position for now
  // According to the Kristine Slot paper this may not result in the shortest
  // path.  However, because we're blending animations, I think ideally we
  // should hope to play as much of the first animation as possible before
  // starting the blend with the second
  unsigned int to_frame = 0;

  // Counters for subsequent increments
  unsigned int horiz_counter = 0;
  unsigned int vert_counter = 0;

  // Push the first frame pair
  shortest_path.push_back(make_pair(from_frame, to_frame));

  if(n_interp_frames > 0)
  {
    while(from_frame < from->frames() - 1 - n_interp_frames)
    {
      shortest_path.push_back(make_pair(++from_frame, to_frame));
    }
  }

  // NB: The path may end before the "to" animation ends, in which case the
  // blender should just finish playing the to animation
  while(from_frame < from->frames() - 1)
  {
    // If we've already reached the end of the two animation, we always pick the
    // next from frame.
    if(to_frame >= to->frames() - 1)
    {
      shortest_path.push_back(make_pair(++from_frame, to_frame));

      // We don't care about the slope limit here since we only have one way to
      // go
    }
    // Next check that we're adhering to our slope limit
    else if(horiz_counter >= slope_limit)
    {
      // Reached horizontal maximum; candidates are (x+1, y+1) and (x, y+1)
      if(*getDistance(from_frame + 1, to_frame + 1) < 
         *getDistance(from_frame, to_frame + 1))
      {
        shortest_path.push_back(make_pair(++from_frame, ++to_frame));
        vert_counter = 0;
      }
      else
      {
        shortest_path.push_back(make_pair(from_frame, ++to_frame));
      }

      horiz_counter = 0;
    }
    else if(vert_counter >= slope_limit)
    {
      // Reached vertical maximum; candidates are (x+1, y+1) and (x+1, y)
      if(*getDistance(from_frame + 1, to_frame + 1) <
         *getDistance(from_frame + 1, to_frame))
      {
        shortest_path.push_back(make_pair(++from_frame, ++to_frame));
        horiz_counter = 0;
      }
      else
      {
        shortest_path.push_back(make_pair(++from_frame, to_frame));
      }

      vert_counter = 0;
    }
    else
    {
      float horiz = *getDistance(from_frame + 1, to_frame);
      float diag = *getDistance(from_frame + 1, to_frame + 1);
      float vert = *getDistance(from_frame, to_frame + 1);

      float min_dist = min(diag, min(horiz, vert));

      if(min_dist == horiz)
      {
        shortest_path.push_back(make_pair(++from_frame, to_frame));
        ++horiz_counter;
        vert_counter = 0;
      }
      else if(min_dist == diag)
      {
        shortest_path.push_back(make_pair(++from_frame, ++to_frame));
        horiz_counter = 0;
        vert_counter = 0;
      }
      else if(min_dist == vert)
      {
        shortest_path.push_back(make_pair(from_frame, ++to_frame));
        horiz_counter = 0;
        ++vert_counter;
      }
    }

  } // End while - this function is a nightmare

  // Make sure we've pushed all the frames of the "to" animation
  while(to_frame < to->frames() - 1)
  {
    shortest_path.push_back(make_pair(from_frame, ++to_frame));
  }

}

const vector<pair<unsigned int, unsigned int> >& DistanceMap::getShortestPath() const
{
  return shortest_path;
}

ostream& operator<<(ostream &out, DistanceMap &map)
{
  for(unsigned int i = 0; i < map.from->frames(); ++i)
  {
    for(unsigned int j = 0; j < map.to->frames(); ++j)
    {
      out << *(map.getDistance(i, j)) << ", ";
    }

    out << endl;
  }

  return out;
}

}
