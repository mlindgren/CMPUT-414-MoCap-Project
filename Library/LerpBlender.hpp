#ifndef __LERPBLENDER_H__
#define __LERPBLENDER_H__

#include <Library/Library.hpp>
#include <Library/DistanceMap.hpp>
#include <Character/Character.hpp>

#include <vector>

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

  static LerpBlender blendFromBlend(const LerpBlender &old, const Motion *m);

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

  /* Returns true if the first animation is done, indicating that
   * the next animation may be advanced to (using the blendFromBlend factory
   * method.) */
  inline bool firstAnimationIsDone()
  {
    return distance_map.getShortestPath()[cur_frame].first == n_from_frames - 1;
  }


private:

  const Motion *from;
  const Motion *to;

  Character::State global_state;
  Character::Control velocity_control;

  DistanceMap distance_map;

  /* WARNING: These aren't actually frame numbers, but indexes into the distance
   * map, which provides pairs of frame numbers.
   * TODO: These names are confusing and should be changed. */
  unsigned int last_frame;
  unsigned int cur_frame;

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
