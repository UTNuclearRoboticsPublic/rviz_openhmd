# RVIZ Plugin for the HMDs using OpenHMD

## About
This plugin was created for use in a University of Texas at Austin Neuclear Robotics Group (UT NRG) project. The UT NRG project is a Computer Engeneering senior design project focused on improving situational awareness for robot operators via a virtual reality headset. The idea is for VR headsets to replace the pan/tilt method of robot controls. The GUI project can be found [here](https://github.com/UTNuclearRoboticsPublic/project-crunch). The original basic project can be found [here](https://github.com/UTNuclearRoboticsPublic/ece-senior-design). Currently the plugin can support 1 or 2 (with some bugs) headsets.

## Requirements
Several smaller libraries are needed by these dependencies, but these are the major parts that are used.
* [openhmd](https://github.com/OpenHMD/OpenHMD) and accompanying requirements
* [hidapi](https://packages.debian.org/source/jessie/hidapi)
* [ROS Kinetic](http://wiki.ros.org/kinetic)
* [OGRE 3D](https://www.ogre3d.org/)
* [NVIDIA driver 410](https://www.nvidia.com/object/unix.html)
* Each dependency may have its own sub-dependecies. Check individual sources to see details.

## Installation
1. Clone the repository directly into your catkin/src workspace.
2. Change line 73 in src/openhmd_display.cpp to point to the correct location.
3. Change line 3 in src/resources.cfg to point to the correct location.
4. Use `catkin_make` or `catkin build` to compile.

## Use
1. Launch RViz.
2. Plug in the headset hdmi/dispaly and usb cables.
3. Add the plugin to the RViz window.
4. The compository may appear on your main monitor, drag it over to the other 'monitor' (HMD) and make if full screen.

Note: Only a mini-display to display cable will work for display ports.

Note: If you are having error in headset tracking, check your system [udev rules](https://github.com/OpenHMD/OpenHMD/wiki/Udev-rules-list).

## Troubleshooting
1. The USB on the HTC Vive (and potentially others) can be finicky. Try using a different USB port if you are still having issues after setting udev rules.
2. The desktop resolution can mess up when plugging in the headset. I found changing display settings to 'Display with smallest controls' fixes the issue.
3. The windows often start up on the wrong 'monitor' so you may want to launch RViz then attach the headset before adding the plugin.
4. It is possible that the GPU will deny access to the HMD when it sees it is not a regular monitor. See http://doc-ok.org/?p=1763 for the fix.
5. With 2 headsets, the plugin does not know which incomming data should update which window, since it does not know which window is on which headset. The accompanying python script can be called with `python3 position_windows.py X` where X is the number of headsets. This will set the position of the windows. If the windows are in the wrong headset, change the script to assign offsets to be swapped. 
6. The headsets may appear to drift when viewing. Make sure the plugin initializes with the headsets on a stable surface to initialize the gyros properly.

## Credits
The original Vive in RViz project used openVR and SteamVR to interface with the HTC Vive. Credit for the origins of this plugin goes to Andre Gilerson. His original project can be found [here](https://github.com/AndreGilerson/rviz_vive). Files modified directly from there are:
* CMakeLists.txt
* package.xml
* plugin_description.xml

Much of the structure for interfacing with the OpenHMD API came from the [demo project](https://github.com/OpenHMD/OpenHMDDemo). Files taken and modified directly from there are:
* resources/*
* OpenHMD.cpp
* OpenHMD.h
* resources.cfg

## Contact
The original repository and commit history is located [here](https://github.com/btandersen383/rviz_openhmd). For questions about the plugin and its support, contact Beathan Andersen at btandersen@utexas.edu.
