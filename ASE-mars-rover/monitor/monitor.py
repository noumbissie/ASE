#!/usr/bin/env python2
"""
monitor.py: A small monitor to run control programs in the ASE rover
simulator
"""

from __future__ import print_function
import sys
from subprocess import Popen, PIPE
from threading import Thread
from Queue import Queue, Empty
import urllib2
import atexit

# Handler that will be mapped to the Rover control process
proc = None

# Default ase-rover server
SERVER="http://localhost:8888"

def kill_proc():
    """
    The monitor should kill the Rover control program before exiting
    """
    if proc:
        proc.kill()

atexit.register(kill_proc)

def fail(msg):
    """
    Fail handler: prints a message and kills the monitor
    """
    print(msg, file=sys.stderr)
    sys.exit(1)

class RoverClosedStream(Exception): pass

# Non Blocking Steam Reader
# based upon http://eyalarubas.com/python-subproc-nonblock.html
class NonBlockingStreamReader:
    def __init__(self, stream):
        """
        stream: the stream to read from.
                Usually a process' stdout or stderr.
        """

        self._s = stream
        self._q = Queue()

        def _populateQueue(stream, queue):
            """
            Collect lines from 'stream' and put them in 'quque'.
            """
            while True:
                line = stream.readline()
                queue.put(line)

        self._t = Thread(target = _populateQueue,
                args = (self._s, self._q))
        self._t.daemon=True
        self._t.start() #start collecting lines from the stream

    def readline(self, timeout = None):
        try:
            line = self._q.get(block = timeout is not None,
                    timeout = timeout)
            if line:
                return line
            else:
                raise RoverClosedStream()
        except Empty:
            return None

def camera(nouns):
    """
    Handle Camera commands
    """
    resp = urllib2.urlopen("{server}/CAMERA"
                    .format(server=SERVER)).read()
    proc.stdin.write(resp+"\n")

def turn(nouns):
    """
    Handle Turn commands
    """
    if len(nouns) != 1:
        fail('missing degrees in TURN command')

    try:
        degrees = float(nouns[0])
    except ValueError:
        fail('TURN command expects a float argument')

    resp = urllib2.urlopen("{server}/TURN/{angle}"
                    .format(server=SERVER, angle=degrees)).read()

    proc.stdin.write(resp+"\n")
    assert(resp.startswith("OK"))

def forward(nouns):
    """
    Handle Forward commands
    """
    if len(nouns) != 1:
        fail('missing meters in FORWARD command')

    try:
        distance = float(nouns[0])
    except ValueError:
        fail('FORWARD command expects a float argument')

    resp = urllib2.urlopen("{server}/FORWARD/{distance}"
                    .format(server=SERVER, distance=distance)).read()

    if "CRASH" in resp:
        fail('ROVER crashed')

    if "WIN" in resp:
        fail('TARGET REACHED! You win!')

    proc.stdin.write(resp+"\n")
    assert(resp.startswith("OK"))

"""
The dictionary of valid Rover command
"""
cmds = {"CAMERA": camera,
        "TURN": turn,
        "FORWARD": forward,
        }

def dispatch_cmd(data):
    """
    Handles commands that the Rover control process sends to the monitor
    """
    parts = data.split()
    verb = parts[1]
    nouns = parts[2:]
    if verb in cmds:
        cmds[verb](nouns)
    else:
        fail('Unknown command `{}` from Rover'.format(verb))

def launch(process):
    """
    Launch the student Rover control process in a separate execution process
    """
    global proc
    try:
        proc = Popen([process], stdout = PIPE, stdin = PIPE,
                     shell=False)
    except OSError as e:
        fail('Cannot execute `{}`: {}'.format(process, e))

    nbsr = NonBlockingStreamReader(proc.stdout)

    while(1):
        try:
            data = nbsr.readline()
        except RoverClosedStream:
            fail("Rover closed output stream")

        if not data: continue
        print("ROVER: {}".format(data))
        if data.startswith("CMD"):
            dispatch_cmd(data)

if __name__ == "__main__":
    from argparse import ArgumentParser

    # Deal with command line arguments
    p = ArgumentParser(description='Monitor ASE Rover process')
    p.add_argument('--server', dest='server', default=SERVER,
            help='The simulator server (default {})'.format(SERVER))
    p.add_argument('process',
                   help='the Rover controler executable')

    args = p.parse_args()
    SERVER = args.server
    launch(args.process)
