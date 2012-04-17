#ifndef __DISTANCEMAP_H__
#define __DISTANCEMAP_H__

#include <Library/Library.hpp>
#include <Character/Character.hpp>

#include <vector>
#include <iostream>

namespace Library
{

/* The distance map class builds an n * m map of distances between the frames
 * in two animations, where n is the number of frames in the first animation
 * and n the number of frames in the second */
class DistanceMap
{
public:
  /* Initializes a lerp blender from two motions to be blended */
  DistanceMap(Motion &f, Motion &t);
  DistanceMap(const DistanceMap &other);
  DistanceMap& operator= (const DistanceMap &other);

  ~DistanceMap();

  /* DO NOT index into the distance map manually or a mistake will inevitably
   * be made at some point.  ALWAYS use this function to get the correct
   * address. */
  float* getDistance(unsigned int from_frame, unsigned int to_frame);

  /* Finds joint positions in the world coordinate system and puts them in the
   * positions vector. They will be in the same order as the bones are defined
   * in the motion. */
  void getJointPositions(Character::Pose const &pose, 
                         std::vector<Vector3f> &positions);


  friend ostream& operator<<(ostream &out, DistanceMap &map);

private:
  float *distances;

  /* These should really be const, but I can't make them const and still have
   * distances be heap-allocated without ignoring the rule of three... I am
   * fairly certain I am doing something wrong here in terms of idiomatic C++
   * but I don't know how to fix it. :( */
  Motion &from;
  Motion &to;
};

}

#endif
