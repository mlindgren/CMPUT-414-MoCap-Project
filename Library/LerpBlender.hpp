#ifndef __LERPBLENDER_H__
#define __LERPBLENDER_H__

#include <Library/Library.hpp>
#include <Library/DistanceMap.hpp>
#include <Character/Character.hpp>

// Herp derp lerp
namespace Library
{

/* LerpBlender blends two animations using naive linear 
   interpolation of bone angles */
class LerpBlender
{
public:
  /* Initializes a lerp blender from two motions to be blended */
  LerpBlender(Motion &f, Motion &t);

  /* Increment or decrement frame */ 
  void changeFrame(int delta);

  void getPose(Character::Pose &output);

private:
  Motion &from;
  Motion &to;

  Character::Control velocity_control;
  Character::State global_state;

  DistanceMap distance_map;

  unsigned int last_frame;
  unsigned int cur_frame;

  // Current frame in from and to motions
  //unsigned int from_frame;
  //unsigned int to_frame;

  // Number of frames in from and to motions
  // TODO: Bad names
  unsigned int n_from_frames;
  unsigned int n_to_frames;

  // The number of frames to interpolate
  // This will be equal to one quarter of the total number of frames in the
  // motion which has fewer frames
  unsigned int n_interp_frames;
};

}

#endif
