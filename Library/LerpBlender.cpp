#include "Library/LerpBlender.hpp"

#include <Vector/Vector.hpp>
#include <Vector/Quat.hpp>

#include <cassert>
#include <iostream>

#define INTERP_DIVISOR 2

using namespace Character;

namespace Library
{

LerpBlender::LerpBlender(Motion &f, Motion &t)
: from(f),
  to(t),
  isInterpolating(false)
{ 
  n_from_frames = from.frames();
  n_to_frames = to.frames();

  /* For now we're naively interpolating the last quarter of the first animation
   * with the first quarter of the last animation.
   * Since the animations aren't the same length, in practice we'll interpolate
   * n frames where n is one quarter of the number of frames in the shorter
   * animation.  This is probably a bad way to go about things, for a number of
   * reasons which I'm not going to enumerate at the moment.
   * Also: ternary ifs are bad. Do as I say, not as I do. */
  n_interp_frames = n_from_frames < n_to_frames ? 
                    (n_from_frames / INTERP_DIVISOR) : (n_to_frames / INTERP_DIVISOR);

}

void LerpBlender::nextFrame(unsigned int frame, Pose &output)
{

  /* Create and initialize (not sure why these are separate steps, very
   * non-idiomatic; herpderp I'm pedantic) poses for the from and to
   * to motions */
  Pose from_pose;
  Pose to_pose;
  from_pose.clear();
  to_pose.clear();

  // Calculate the from and to frames according to the frame number given to us
  // by the browser.  This is just an unsigned int which keeps incrementing
  // until it wraps around (TODO: bad coupling because implementation detail
  // here relies on implementation detail in BrowseMode.cpp), so we need to
  // do some math to determine which from/to frame we're on
  unsigned int from_frame = frame % (n_from_frames + n_to_frames - n_interp_frames);
  unsigned int to_frame = from_frame - n_from_frames + n_interp_frames;
  if(to_frame > n_to_frames) to_frame = 0;

  // Load the poses from their respective motions
  // Note: modulus is necessary in getting the from pose because
  // my if logic is sloppy below so I need to leave from_frames equal
  // to num_from_frames when playing just the second animation
  // Should find a better way to do that (as mentioned in below)
  from.get_pose(from_frame % n_from_frames, from_pose);
  to.get_pose(to_frame, to_pose);

  // Ensure that the two animations have the same number of bone orientations
  // We can't transition between two animations on different skeletons
  assert(from_pose.bone_orientations.size() == 
         to_pose.bone_orientations.size());

  isInterpolating = false;

  // TODO: Sloppy way of doing this; I'm sure it can be done more elegantly.
  if(from_frame >= n_from_frames - n_interp_frames && 
     from_frame < n_from_frames)
  {
    float interp_value = (float) to_frame / n_interp_frames;
    if(interp_value > 1.0f)
    {
      interp_value = 1.0f;
    }

    float interp_conjugate = 1.0f - interp_value;

    /* Loop through all bones and interpolate their orientation quaternions
     * Note that we asserted above that the two animations have the same number
     * of bones */
    for(unsigned int i = 0; i < from_pose.bone_orientations.size(); ++i)
    {
      from_pose.bone_orientations[i] = slerp(from_pose.bone_orientations[i],
                                             to_pose.bone_orientations[i],
                                             interp_value);
    }

    // Interpolate the root position and orientation
    from_pose.root_orientation = slerp(from_pose.root_orientation,
                                       to_pose.root_orientation,
                                       interp_value);

    from_pose.root_position.x = from_pose.root_position.x * interp_conjugate +
                                   to_pose.root_position.x * interp_value;
    from_pose.root_position.y = from_pose.root_position.y * interp_conjugate +
                                   to_pose.root_position.y * interp_value;
    from_pose.root_position.z = from_pose.root_position.z * interp_conjugate +
                                   to_pose.root_position.z * interp_value;
                                   
    
    isInterpolating = true;
    output = from_pose;
  }
  else if(from_frame < n_from_frames - n_interp_frames)
  {
    output = from_pose;
  }
  else if(from_frame >= n_from_frames)
  {
    output = to_pose;
  }

  // TODO: This prevents the character from ever moving from the origin
  // Fixes our skating problem for now, but this is not a proper solution
  output.root_position.x = output.root_position.z = 0;

}

}
