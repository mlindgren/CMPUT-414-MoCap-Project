#include "Library/LerpBlender.hpp"

#include <Vector/Vector.hpp>
#include <Vector/Quat.hpp>

#include <cmath>
#include <cassert>
#include <iostream>

#define INTERP_DIVISOR 4

using namespace Character;
using std::pair;

namespace Library
{

LerpBlender::LerpBlender(Motion &f, Motion &t)
: from(f),
  to(t),
  distance_map(f, t),
  last_frame(0),
  cur_frame(0)
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

  // TODO: cout message should go somewhere else
  std::cout << "Please be patient while the distance map is populated..." << endl;
  distance_map.populate();
  distance_map.calcShortestPath(n_interp_frames);

  global_state.clear();
  velocity_control.clear();

}

void LerpBlender::changeFrame(int delta)
{
  last_frame = cur_frame;
  if(cur_frame + delta >= distance_map.getShortestPath().size())
  {
    if(delta > 0)
    {
      last_frame = 0;
      cur_frame = 0;
      global_state.clear();
    }
    else if(delta < 0)
    {
      cur_frame = distance_map.getShortestPath().size() - 1;
      last_frame = cur_frame + delta;
      global_state.clear();
    }
  }
  else
  {
    cur_frame += delta;
  }
}

void LerpBlender::getPose(Pose &output)
{

  /* Create and initialize (not sure why these are separate steps, very
   * non-idiomatic; herpderp I'm pedantic) poses for the from and to
   * to motions */
  Pose from_pose;
  Pose to_pose;
  from_pose.clear();
  to_pose.clear();

  const pair<unsigned int, unsigned int> frame_pair = 
    distance_map.getShortestPath()[cur_frame];
  from.get_pose(frame_pair.first, from_pose);
  to.get_pose(frame_pair.second, to_pose);

  //float interp_value = (float) cur_frame / distance_map.getShortestPath().size();
  //float interp_value = (float) frame_pair.first / from.frames();
  float interp_value = expf((float) frame_pair.second / n_interp_frames) - 1;
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

  // Assign the output frame to be equal to the interpolated from pose.
  // We're not done yet, though; using root positions is unreliable,
  // so instead we'll want to use velocities, which we'll calculate below.
  // For now, set the output x and z positions to 0.
  output = from_pose;
  output.root_position.x = output.root_position.z = 0;

  // Get the next frame pair.  If we're at the end of both animations,
  // we just use the current frame for simplicity, since there obviously
  // can't be any velocity change after the end of the animation anyway.
  const pair<unsigned int, unsigned int> last_pair = 
    distance_map.getShortestPath()[last_frame];

  // Determine velocity in each of the animations
  Pose from_last;
  from_last.clear();
  from.get_pose(last_pair.first, from_last);

  Pose to_last;
  to_last.clear();
  to.get_pose(last_pair.second, to_last);

  // We interpolate the velocity according to the interpolation value
  // calculated above
  velocity_control.desired_velocity = 
    (to_pose.root_position - to_last.root_position) * interp_value +
    (from_pose.root_position - from_last.root_position) * interp_conjugate;

  velocity_control.apply_to(global_state, 1);
  global_state.apply_to(output);

  /*if(frame_pair.second > 0 && frame_pair.first < from.frames())
    isInterpolating = true;
  else
    isInterpolating = false;*/
}  

}
