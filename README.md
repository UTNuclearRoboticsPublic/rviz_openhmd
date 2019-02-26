# RVIZ Plugin for the HMDs using OpenHMD

## About
This plugin was created for use in a University of Texas at Austin Neuclear Robotics Group (UT NRG) project. The UT NRG project is a Computer Engeneering senior design project focused on improving situational awareness for robot operators. The idea is for VR headsets to replace the pan/tilt method of robot controls. The full project can be found [here](https://github.com/UTNuclearRoboticsPublic/ece-senior-design). Currently we are working on having this plugin support 2 (or more) headsets on a single computer.

## Requirements
Several smaller libraries are needed by these dependencies, but these are the major parts that are used.
* [openhmd](https://github.com/OpenHMD/OpenHMD)
* [hidapi](https://packages.debian.org/source/jessie/hidapi)
* [ROS Kinetic](http://wiki.ros.org/kinetic)
* [OGRE 3D](https://www.ogre3d.org/)
* [NVIDIA driver 410](https://www.nvidia.com/object/unix.html) (or potentially other VR drivers)
* TODO list smalleer

## Installation
1. Clone the repository directly into your catkin/src workspace.
2. Change line 72 in src/openhmd_display.cpp to point to the correct location.
3. Change line 3 in src/resources.cfg to point to the correct location.
3. Use `catkin_make` or `catkin build` to compile.

## Use
1. Launch RViz.
2. Plug in the headset hdmi and usb.
3. Add the plugin to the window.
4. The compository may appear on your main monitor, drag it over to the other 'monitor' (HMD) and make if full screen.

## Credits
The original Vive in RViz project used openVR and Steam to interface with the HTC Vive. Credit for the skeleton of this plugin goes to Andre Gilerson. His original project can be found [here](https://github.com/AndreGilerson/rviz_vive). Files taken and modified directly from there are:
* CMakeLists.txt
* package.xml
* plugin_description.xml

Much of the structure for interfacing with the OpenHMD API came from the [demo project](https://github.com/OpenHMD/OpenHMDDemo). Files taken directly from there are:
* resources/
* OpenHMD.cpp
* OpenHMD.h
* resources.cfg

## Contact
For questions about the plugin and its support, contact Beathan Andersen at btandersen@utexas.edu.