#ifndef BROWSEMODE_HPP
#define BROWSEMODE_HPP

#include "Mode.hpp"

#include <Character/Character.hpp>
#include <Character/Skin.hpp>
#include <Library/LerpBlender.hpp>

#include <vector>
#include <deque>
#include <utility>
#include <string>

using std::deque;
using std::vector;
using std::pair;
using std::string;

class BrowseMode : public Mode
{
public:
  BrowseMode();
  virtual ~BrowseMode();

  virtual void update(float const elapsed_time);

  /* Switches between motions by adding delta to the current motion index. */
  virtual void switch_motion(short delta);

  virtual void handle_event(SDL_Event const &event);

  virtual void draw();

  /* If true, the browser will automatically cycle through animations by
   * switching to a new animation blend as soon as the first animation in
   * the current blend is done. */
  bool auto_advance;

  Vector3f camera;
  Vector3f target;
  bool track;

  Character::Pose current_pose;
  Character::State current_state;
  Vector3f current_motion_root;
  unsigned int current_motion;
  unsigned int frame;
  float time;
  float play_speed;

  Character::Skin skin;

private:
  Library::LerpBlender blender;
};

#endif //BROWSEMODE_HPP
