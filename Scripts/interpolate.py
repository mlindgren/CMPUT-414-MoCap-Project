#!/usr/bin/python
# CMPUT 414 Project Motion Linear Interpolation Script
import sys
import re

HEADER = ":FULLY-SPECIFIED\n:DEGREES\n"

class JointPosition:
    """
    A position of a single joint specified in the AMC file.
    """

    name = ""
    positions = []

    def __init__(self, n, pos):
        """
        Initialize a new joint position with given name (n) and position.
        """

        self.name = n
        self.positions = [float(x) for x in pos]

    def __str__(self):
        """
        String representation
        """
        
        # Truncate floats to 4 decimal places
        truncf = lambda x: "%.4f" % x

        return self.name + ' ' + ' '.join(map(truncf, self.positions))

    def __repr__(self):
        """
        Debugging representation
        """

        return self.name + str(self.positions)


    def add_position(self, pos):
        """
        Adds a position to the position list.
        """

        self.positions.append(pos)

    def difference(self, other):
        """
        Calculates and returns the difference between two joint positions
        by subtracting each component of this joint position from each
        component of the other.
        """

        assert(self.name == other.name)

        differences = []
        for i in range(len(other.positions)):
            difference = other.positions[i] - self.positions[i]

            # We should never be rotating more than 180 degrees, or we're going
            # in the wrong direction.  Thus, the difference between two
            # components of a joint position should never be more than 180
            # degrees.  Note that this is not really a safe assumption since
            # the positions could in theory have translation components,
            # although in practice that only happens with the root which we
            # do not interpolate.
            if difference > 180:
                difference = 360 - difference
            elif difference < -180:
                difference += 360

            differences.append(difference)

        return JointPosition(self.name, differences)

    def sum(self, other):
        """
        Calculates and returns the sum of two joint positions by adding
        corresponding components.
        """

        assert(self.name == other.name)

        sums = []
        for i in range(len(other.positions)):
            sums.append((self.positions[i] + other.positions[i]) % 360)

        return JointPosition(self.name, sums)

    def divide(self, val):
        """
        Divides each component by a scalar value and returns a new JointPosition.
        """

        return JointPosition(self.name, [item/val for item in self.positions])

    def multiply(self, val):
        """
        Multiplies each component by a scalar value.
        """

        return JointPosition(self.name, [item * val for item in self.positions])


def findFirstFrame(filename):
    """
    Finds the first frame in the given motion file and returns it as a list of
    JointPosition classes.
    """

    motion_file = open(filename, 'r')
    
    found_first = False

    # I feel like this should be a dictionary, but I am unsure as to whether or
    # not the joint positions have to be given in a specific order
    frame_specification = []

    # Loop through all the lines in the file until we find the first frame
    # In practice we'll only read in until we reach the second frame
    for line in motion_file:
        if line.strip() == "1":
            found_first = True
            continue

        # Quit at the start of the second frame
        if line.strip() == "2":
            break

        if found_first:
            split_line = line.split()
            frame_specification.append(
                JointPosition(split_line[0], split_line[1:]))
        
    return frame_specification

def findLastFrame(filename):
    """
    Finds the last frame in the given file and returns a list of JointPosition
    classes.
    """

    motion_file = open(filename, 'r')

    # Loop through all the lines in the file.  Because I'm hacking this thing
    # together in a rush, we're going to do this really sloppily and just make
    # a frame specification for each frame, and throw them away until we reach
    # the end of the file.
    for line in motion_file:

        # New frames occur on lines with only integers
        if re.match("\d", line.strip()):
            frame_specification = []
            continue
        elif line[0] != '#' and line[0] != ':':
            split_line = line.split()
            frame_specification.append(
                JointPosition(split_line[0], split_line[1:]))

    return frame_specification

def interpolateMotions(first_frame, last_frame, n_frames):
    """
    Performs linear interpolation between two frames by adding
    (difference/n_frames) to each component of each frame
    specification, n_frames times.
    """

    # We really only need the deltas but having both of these makes me feel
    # warm and fuzzy
    differences = []
    deltas = []

    # Set up buffer for string output
    out_buf = []

    for i in range(len(first_frame)):
        difference = first_frame[i].difference(last_frame[i])
        differences.append(difference)
        deltas.append(difference.divide(n_frames))

    for i in range(1, n_frames + 1):

        out_buf.append(str(i))
        for position_idx, joint_position in enumerate(first_frame):

            # We don't perform interpolation on the root
            if joint_position.name == "root":
                out_buf.append(str(joint_position))
                continue

            out_buf.append(str(joint_position.sum(deltas[position_idx].multiply(i))))

    return '\n'.join(out_buf)

def Main():
    """
    Main function.  Program entry point.
    """

    # We require 3 command line arguments in addition to the first argument
    # which is the name of the exectuable.
    # 1. Starting motion file
    # 2. Ending motion file
    # 3. Number of frames for interpolation
    if len(sys.argv) < 4:
        print "Not enough command line arguments"
        sys.exit(1)

    first = findLastFrame(sys.argv[1])
    last = findFirstFrame(sys.argv[2])

    frames = int(sys.argv[3])

    print HEADER + interpolateMotions(first, last, frames)

if __name__ == "__main__":
    Main()
