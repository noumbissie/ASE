#!/usr/bin/env python2
"""
demo.py: demo manual Rover control process
"""

from __future__ import print_function
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import numpy as np
import sys

image = None

def left(event):
    """ Left callback """
    turn(event, +.5)

def right(event):
    """ Right callback """
    turn(event, -.5)

def turn(event, angle):
    """ Turn callback """

    # Send a TURN command to the rover
    print ("CMD TURN {}".format(angle), file=sys.stdout)
    sys.stdout.flush()

    # Check that rover executed the command
    assert("OK" in sys.stdin.readline())

    # Update camera view
    data = camera()
    image.set_data(data)
    plt.draw()

def forward(event):
    """ Forward callback """
  
    # Send a FORWARD command to the rover
    print ("CMD FORWARD 5.0", file=sys.stdout)
    sys.stdout.flush()

    # Check that rover executed the command
    assert("OK" in sys.stdin.readline())

    # Update camera view
    data = camera()
    image.set_data(data)
    plt.draw()

def camera():

    # Send a CAMERA command to the rover
    print("CMD CAMERA", file=sys.stdout)
    sys.stdout.flush()

    # Read rover response
    string = sys.stdin.readline()

    # check image has the correct format
    header = "P2 640 360 255 "
    assert(string.startswith(header))

    # Remove header
    string = string[len(header):]

    # Read image
    data = np.fromstring(string, dtype=int, sep=' ').reshape(360, 640)

    # Normalize colors 
    data = data / 255.0

    return data

def main():
    """ The control main function """
    global image

    # Request and show camera view
    data = camera()
    image = plt.imshow(data, cmap=plt.get_cmap('gray'))
    plt.draw()

    # Setup interface
    ax1 = plt.axes([0, 0.6, 0.1, 0.075])
    ax2 = plt.axes([0, 0.4, 0.1, 0.075])
    ax3 = plt.axes([.9, 0.5, 0.1, 0.075])

    b1 = Button(ax1, 'Left')
    b1.on_clicked(left)

    b2 = Button(ax2, 'Right')
    b2.on_clicked(right)

    b3 = Button(ax3, 'Forward')
    b3.on_clicked(forward)

    # Show everything
    plt.show()


if __name__ == "__main__":
    main()
