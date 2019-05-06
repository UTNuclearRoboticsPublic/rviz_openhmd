/**
BSD 3-Clause License

Copyright (c) 2018, UT NRG
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include "openhmd_display.h"

#include <OGRE/OgreRoot.h>

#include <rviz/display_context.h>
#include <rviz/view_manager.h>
#include <rviz/ogre_helpers/render_system.h>
#include <iostream>

#include <OgreCompositorManager.h>

#include <RenderSystems/GL/OgreGLTextureManager.h>

const float g_defaultIPD = 0.064f;

namespace rviz_openhmd
{

/************ Constructor ************/
OpenhmdDisplay::OpenhmdDisplay() :
    tfListener(tfBuffer)
{
}

/************ Destructor ************/
OpenhmdDisplay::~OpenhmdDisplay()
{
    delete openhmd;
}

/************ To instantiate the objects needed ************/
void OpenhmdDisplay::onInitialize()
{   

/*
initialize openhmd and check numeber of headsets
set up ogre resources
branch on num hmds
    initialize one window
    initialize two window
*/
    mResourcesCfg = "/home/me/Desktop/vr_moto/src/rviz_openhmd/src/resources.cfg";

    // TODO: this needs to be first in initialize
    // Setup OpenHMD object
    openhmd = new OpenHMD();
    // return -1, error
    // return 1, 1 hmd
    // return 2, 2 hmds
    NumHMDs = openhmd->init();
    if (NumHMDs == -1)
    {
        std::cout << "Error initializing OpenHMD" << std::endl;
        exit(1);
    }

    // Set up general objects
    mSceneMgr = scene_manager_;  // ogre objects
    mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

    // Setup resources for compositors
    // For different headsets, different compositors are needed
    // Change this line to point to the resources.cfg file
    mResourcesCfg = "/home/" + (std::string) std::getenv("USER") + "/Desktop/vr_moto/src/rviz_openhmd/src/resources.cfg";
    Ogre::ConfigFile cf;

    // Load config file
    cf.load(mResourcesCfg);

    // Load resource paths from config file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements()) 
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;

        for (i = settings->begin(); i != settings->end(); ++i) 
        {
            typeName = i->first;
            std::cout << typeName << std::endl;
            archName = i->second;
            std::cout << archName << std::endl;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
    
    // Required to use resources when loaded after Ogre::root init
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("OpenHMD");

    // Create a render window for the HMD
    // Window size is set here and may need to be adjusted
    root = rviz::RenderSystem::get()->root();
    mWindow = root->createRenderWindow("HMD1", 2160, 1200, false);
    if (NumHMDs == 2) mWindow2 = root->createRenderWindow("HMD2", 2160, 1200, false);
    std::cout << "Render window created" << std::endl;

    // Create the camera node
    if (mSceneNode)
        mCamera = mSceneNode->createChildSceneNode("StereoCameraNode");
    else
        mCamera = mSceneMgr->getRootSceneNode()->createChildSceneNode("StereoCameraNode");

    if (NumHMDs == 2)
    {
        if (mSceneNode)
            mCamera2 = mSceneNode->createChildSceneNode("StereoCameraNode2");
        else
            mCamera2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("StereoCameraNode2");
    }

    // Create the cameras for hmd1, will act as the eyes
    stereo_cam_left = mSceneMgr->createCamera("StereoCameraLeft");
    stereo_cam_right = mSceneMgr->createCamera("StereoCameraRight");
    if (NumHMDs == 2)
    {
        // Create the cameras for hmd2, will act as the eyes
        stereo_cam_left2 = mSceneMgr->createCamera("StereoCameraLeft2");
        stereo_cam_right2 = mSceneMgr->createCamera("StereoCameraRight2");
    }

    // Spawn position of the cameras
    // This can be used to manipulate the initial point of view
    // For hmd1
    mCamera->setPosition(Ogre::Vector3(0,0,0));
    mCamera->setOrientation(Ogre::Quaternion(-0.5, -0.5, 0.5, 0.5));
    if (NumHMDs == 2)
    {
        // For hmd2
        mCamera2->setPosition(Ogre::Vector3(0,0,0));
        mCamera2->setOrientation(Ogre::Quaternion(-0.5, -0.5, 0.5, 0.5)); // Initial rotation
    }
    std::cout << "Cameras Created" << std::endl;

    // Set the position of the cameras
    stereo_cam_left->setPosition(openhmd->getLeftViewMatrix().getTrans());
    stereo_cam_right->setPosition(openhmd->getRightViewMatrix().getTrans());
    if (NumHMDs == 2)
    {
        // For hmd2 Set the orientation of the cameras
        stereo_cam_left2->setPosition(openhmd->getLeftViewMatrix().getTrans());
        stereo_cam_right2->setPosition(openhmd->getRightViewMatrix().getTrans());
    }

    stereo_cam_left->setNearClipDistance(0.000012);
    stereo_cam_left->setFarClipDistance(200*120);
    stereo_cam_right->setNearClipDistance(0.000012);
    stereo_cam_right->setFarClipDistance(200*120);
    if (NumHMDs == 2)
    {
        // For hmd2
        stereo_cam_left2->setNearClipDistance(0.000012);
        stereo_cam_left2->setFarClipDistance(200*120);
        stereo_cam_right2->setNearClipDistance(0.000012);
        stereo_cam_right2->setFarClipDistance(200*120);
    }
    std::cout << "Cameras positioned" << std::endl;

    // Put the cameras on the camera node
    stereo_cam_left->detachFromParent();
    stereo_cam_right->detachFromParent();
    if (NumHMDs == 2)
    {
        // For hmd2
        stereo_cam_left2->detachFromParent();
        stereo_cam_right2->detachFromParent();
    }

    mCamera->attachObject(stereo_cam_left);
    mCamera->attachObject(stereo_cam_right);
    if (NumHMDs == 2)
    {
        // For hmd2
        mCamera2->attachObject(stereo_cam_left2);
        mCamera2->attachObject(stereo_cam_right2);
    }

    // Setup viewports -- aka rendered windows on the HMD screen
    mWindow->removeAllViewports();
    // 0.5 argument --> half of screen width
    leftVP = mWindow->addViewport(stereo_cam_left, 1, 0, 0, 0.5f, 1.0f);
    rightVP = mWindow->addViewport(stereo_cam_right, 2, 0.5f, 0, 0.5f, 1.0f);
    if (NumHMDs == 2)
    {
        // For hmd2
        mWindow2->removeAllViewports();
        leftVP2 = mWindow2->addViewport(stereo_cam_left2, 1, 0, 0, 0.5f, 1.0f);
        rightVP2 = mWindow2->addViewport(stereo_cam_right2, 2, 0.5f, 0, 0.5f, 1.0f);
    }
    std::cout << "Viewports added to window" << std::endl;

    // Assumes we are using the HTC Vive, else a different compositor should be added and used
    if (!openhmd->isDummy())
    {
        std::cout << "HMD generic" << std::endl;
        Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
        Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");
        std::cout << "Enabling compositors" << std::endl;
        // Enable the compositors to show images
        leftComp->setEnabled(true);
        rightComp->setEnabled(true);
        if (NumHMDs == 2)
        {
            // For hmd2
            Ogre::CompositorInstance* leftComp2 = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP2, "HMD/GenericAutoScaling");
            Ogre::CompositorInstance* rightComp2 = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP2, "HMD/GenericAutoScaling");
            leftComp2->setEnabled(true);
            rightComp2->setEnabled(true);
        }
    }
    else
    {
        // TODO: decide what to do if there is no hmd
        // probably throw an error but dont break system
        std::cout << "HMD is a dummy" << std::endl;
        Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
        Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");
        leftComp->setEnabled(true);
        rightComp->setEnabled(true);
        if (NumHMDs == 2)
        {
            // For hmd2
            Ogre::CompositorInstance* leftComp2 = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP2, "HMD/GenericAutoScaling");
            Ogre::CompositorInstance* rightComp2 = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP2, "HMD/GenericAutoScaling");
            leftComp2->setEnabled(true);
            rightComp2->setEnabled(true);
        }
    }
    std::cout << "HMD fully initialized" << std::endl;
}

/************ Called by RViz ************/
void OpenhmdDisplay::update(float wall_dt, float ros_dr)
{
    // Force update the render window
    if (mWindow == NULL) {
        std::cout << "Window 1 is NULL" << std::endl;
    }
    mWindow->update();

    if (NumHMDs == 2)
    {
        if (mWindow2 == NULL) {
            std::cout << "Window 2 is NULL" << std::endl;
        }
        mWindow2->update();
    }

    /////////////////////
    // Update camera pose
    /////////////////////
    // Position comes from a ROS tf listener. This could be a static tf publisher in a launch file.
    // Orientation comes from openhmd->getQuaternion(), which uses the headset IMU

    // Get camera position from ROS
    geometry_msgs::TransformStamped transformStamped;
    try{
      transformStamped = tfBuffer.lookupTransform("openhmd_0_desired_position", "lighthouse_0",
                               ros::Time(0));
    }
    catch (tf2::TransformException &ex) {
      printf("%s\n",ex.what());
    }
    mCamera->setPosition(Ogre::Vector3(transformStamped.transform.translation.x,
                                       transformStamped.transform.translation.y,
                                       transformStamped.transform.translation.z));

    // Update HMD
    openhmd->update();

    // Camera orientation
    Ogre::Quaternion cameraOrientation = openhmd->getQuaternion();
    stereo_cam_left->setOrientation(cameraOrientation);
    stereo_cam_right->setOrientation(cameraOrientation);

    if (NumHMDs == 2)
    {
        // Get camera position from ROS
        geometry_msgs::TransformStamped transformStamped;
        try{
          transformStamped = tfBuffer.lookupTransform("openhmd_1_desired_position", "lighthouse_0",
                                   ros::Time(0));
        }
        catch (tf2::TransformException &ex) {
          printf("%s\n",ex.what());
        }
        mCamera2->setPosition(Ogre::Vector3(transformStamped.transform.translation.x,
                                            transformStamped.transform.translation.y,
                                            transformStamped.transform.translation.z));

        // Get the orientation from headset IMU
        Ogre::Quaternion cameraOrientation2 = openhmd->getQuaternion2();
        stereo_cam_left2->setOrientation(cameraOrientation2);
        stereo_cam_right2->setOrientation(cameraOrientation2);
    }

}

/************ Empty display reset ************/
void OpenhmdDisplay::reset()
{
}

};

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_openhmd::OpenhmdDisplay, rviz::Display)
