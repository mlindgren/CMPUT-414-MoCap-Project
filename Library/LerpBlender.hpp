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
  LerpBlender(const Motion *f, const Motion *t);
  LerpBlender(const LerpBlender &other);
  LerpBlender& operator= (const LerpBlender &other);

  /* Increment or decrement frame */ 
  void changeFrame(int delta);

  void getPose(Character::Pose &output);

  /* Accessors for motions */
  const Motion *getFromMotion() const { return from; }
  const Motion *getToMotion() const { return to; }

  /* Get the current frame number */
  unsigned int getFrame() { return cur_frame; }

  /* Get the number of frames in the interpolated animation */
  unsigned int workingFrames() { return distance_map.getShortestPath().size(); }

private:
  const Motion *from;
  const Motion *to;

  Character::State global_state;
  Character::Control velocity_control;

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
