#!/usr/bin/env python3

import subprocess
import re
import sys

var = sys.argv[0]

# This will show you all displays connected to the computer
# (xrandr must be installed)
p1 = subprocess.Popen(['xrandr'], stdout=subprocess.PIPE)

# This will filter by finding all Vive HMD, we assume that the Vive resolution is unique (2160x1200)
# Our goal is to get the HMD display offset values (2160x1200+1920+1080) which will be +x+y
# For more info see - 
# https://askubuntu.com/questions/614145/output-of-xrandr-shows-1024x76813660-what-does-it-mean-and-can-i-change-it/614242#614242
opt, err = subprocess.Popen(['grep','2160x1200'], stdin=p1.stdout,stdout=subprocess.PIPE).communicate()
list_of_opt = opt.splitlines()
coords = []
for line in list_of_opt:
    line = line.decode("utf-8", "ignore")
    try:
        res_coords = re.search("\d+x\d+\+\d+\+\d+",line).group()
        _ , x, y = res_coords.split("+")
        coords.append((x,y))
    except:
        pass

# Now with the offser values, we can position the window(s)
# Use wmctrl (must be installed) to get the ID of the window
# Window name is set as HMD1 in the plugin
windows = subprocess.Popen(["wmctrl","-l"],stdout=subprocess.PIPE)
hmd1, err = subprocess.Popen(['grep','HMD1'],
        stdin=windows.stdout,
        stdout=subprocess.PIPE).communicate()

# get just the winID
wid1 = hmd1.split()[0]

# Use wmctrl to move the window to the given x+y offset and the given 2160x1200 window size
subprocess.call(["wmctrl","-ir",wid1,"-e","0,{},{},2160,1200".format(coords[0][0],coords[0][1])])

# Repeat if there are 2 headsets
if var is 2:
    windows = subprocess.Popen(["wmctrl","-l"],stdout=subprocess.PIPE)
    hmd2, err = subprocess.Popen(["grep","HMD2"],
            stdin=windows.stdout,
            stdout=subprocess.PIPE).communicate()
    wid2 = hmd2.split()[0]
    subprocess.call(["wmctrl","-ir",wid2,"-e","0,{},{},2160,1200".format(coords[1][0],coords[1][1])])
