# Realtime Blending of Raw Motion-Captured Data
### University of Alberta Winter 2012 CMPUT 414 Project
Authors: Mitch Lindgren, Teri Drummond

**Important!**  Before modifying this code, please read the USING file, which contains important information on useful techniques and classes.

## Acknowledgements
This project is a modification of [Jim McCann's ASF/AMC Viewer](http://www.cs.cmu.edu/~jmccann/).  Jim McCann's original readme file is located in README_MCCANN.  Blending techniques are based on processes described in Kristine Slot's 2007 paper on motion blending [(PDF)](http://image.diku.dk/projects/media/kristine.slot.07.pdf), provided by the University of Copenhagen.  Additional guidance was provided by Dr. Anup Basu and Amirhossein Firouzmanesh of the University of Alberta.

## Overview
The goal of this project is to perform simple real-time motion-blended transitions between raw motion-captured animations.  To accomplish this, we perform weighted spherical linear interpolation between joint positions in each animation and velocity-based interpolation between character root positions.

## Compiling
Compilation requires the following dependencies:
* Jam (http://www.perforce.com/jam/jam.html)
* SDL (http://www.libsdl.org/)
* libpng (http://www.libpng.org/pub/png/libpng.html)

Jim McCann's original code compiled on Mac OS X, Linux, and Windows.  We have not made any platform-specific changes to the code, so that should still be the case.  However, development and testing for this project occurred exclusively on Mac OS X; other platforms have not been tested.

On Mac OS X it is easiest to get these dependencies from MacPorts.  You may see a warning that `ld: warning: directory not found for option '-L/usr/lib64'`; this does not mean that compilation has failed.

Once you have all of the dependencies installed, simply type `jam` in the project root directory.  If successful, the compiled application will be placed output to `dist/browser`.  See Jim McCann's original readme file, README_MCANN, for additional instructions.

## Running and Controls
To run the application, type `./browser <path_to_data_files>` in the dist directory.  The provided data directory must contain a set of ASF and AMC motion files; a large variety collection of such files is available on the [CMU Motion Capture Database](http://mocap.cs.cmu.edu/).  Animations are loaded and played in alphabetical order.

Some sample motions from the CMU Motion Capture Database are provided in the working_data and all_data directories.

Controls are as follows:
* **Page Up** advances the starting animation (i.e. the first of the two animations being blended) to the next animation in the directory; **Page Down** returns to the previous animation.
* **Space** toggles speed; available speeds are 1.0x, 0.5x, 0.2x, 0.1x and 0x (paused).
* **Tab** toggles camera tracking; if enabled the camera will track the skeleton.
* **Left mouse button** allows rotation of the camera by mouse movement.
* **Right mouse button** allows zooming of the camera by mouse movement.
* **Middle mouse button** allows panning of the camera by mouse movement.
* **D** dumps currently loaded motion into global position data files. See README_MCCANN for more information on this.
* **A** toggles auto-advance.  If enabled, the browser will continuously cycle through all available animations in the data directory, sequentially blending each pair.  It attempts to maintain smooth transitions throughout this process.  Otherwise, only one pair of animations will be blended, and upon completion of the second animation the animation will reset to the start of the first animation.
* **Escape, ⌘+W or ⌘+Q** quit the program.

## Copyright and License
Original code is copyright Jim McCann.  Modifications and new files (specifically, `Library/DistanceMap.[ch]pp` and `Library/LerpBlender.[ch]pp` are copyright Mitch Lindgren and Teri Drummond.

Jim McCann does not specify a license under which his original code is released.  However, he says "Feel free to use in your own projects (please retain some pointer or reference to the original source in a readme or credits section, however)."  Therefore, please feel free to use this code however you see fit, according to those conditions.
