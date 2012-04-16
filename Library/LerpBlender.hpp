#ifndef __LERPBLENDER_H__
#define __LERPBLENDER_H__

#include <Library/Library.hpp>
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

 void nextFrame(unsigned int frame, Character::Pose &output);

 bool getIsInterpolating() { return isInterpolating; }

private:
  Motion &from;
  Motion &to;

  bool isInterpolating;

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
