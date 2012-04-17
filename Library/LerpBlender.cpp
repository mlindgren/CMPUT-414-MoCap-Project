#include "Library/LerpBlender.hpp"

#include <Vector/Vector.hpp>
#include <Vector/Quat.hpp>

#include <cassert>
#include <iostream>

#define INTERP_DIVISOR 2

using namespace Character;
using std::pair;

namespace Library
{

LerpBlender::LerpBlender(Motion &f, Motion &t)
: from(f),
  to(t),
  distance_map(f, t),
  isInterpolating(false),
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
  distance_map.calcShortestPath();

  /* Debug
  for(vector<pair<unsigned int, unsigned int> >::const_iterator it = distance_map.getShortestPath().begin();
      it < distance_map.getShortestPath().end();
      ++it)
      {
        cerr << it->first << ", " << it->second << endl;
      }
  */

}

void LerpBlender::incrementFrame()
{
  if(++cur_frame >= distance_map.getShortestPath().size())
  {
    cur_frame = 0;
  }
}

void LerpBlender::decrementFrame()
{
  // Remember frame is unsigned
  if(--cur_frame >= distance_map.getShortestPath().size())
  {
    cur_frame = distance_map.getShortestPath().size() - 1;
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

  const pair<unsigned int, unsigned int> frame_pair = distance_map.getShortestPath()[cur_frame];
  from.get_pose(frame_pair.first, from_pose);
  to.get_pose(frame_pair.second, to_pose);

  //float interp_value = (float) cur_frame / distance_map.getShortestPath().size();
  float interp_value = (float) frame_pair.first / from.frames();
  if(interp_value > 1.0f)
  {
    interp_value = 1.0f;
  }

  // float interp_conjugate = 1.0f - interp_value;

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
  
  isInterpolating = true;
  output = from_pose;

  // TODO: This prevents the character from ever moving from the origin
  // Fixes our skating problem for now, but this is not a proper solution
  /*
  from_pose.root_position.x = from_pose.root_position.x * interp_conjugate +
                                 to_pose.root_position.x * interp_value;
  from_pose.root_position.y = from_pose.root_position.y * interp_conjugate +
                                 to_pose.root_position.y * interp_value;
  from_pose.root_position.z = from_pose.root_position.z * interp_conjugate +
                                 to_pose.root_position.z * interp_value;
  */
  output.root_position.x = output.root_position.z = 0;

}

}
