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
OpenhmdDisplay::OpenhmdDisplay()
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
    // Set up general objects
    mDispCtx = context_;  // rviz object
    mSceneMgr = scene_manager_;  // ogre objects
    mSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

    // Setup resources for compositors
    // For different headsets, different compositors are needed
    // Change this line to point to the resources.cfg file
    mResourcesCfg = "/home/beathan/rviz_openhmd/src/resources.cfg";
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

    // Debugging checks
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists("OpenHMD", "HMD.compositor")) 
        std::cout << "YEET!" << std::endl;
    else 
        std::cout << "Your resource does not exist" << std::endl;

    if (Ogre::ResourceGroupManager::getSingleton().isResourceGroupInitialised("OpenHMD"))
        std::cout << "YEEZUS!" << std::endl;
    else 
        std::cout << "OpenHMD is not initialized" << std::endl;
    
    // Required to use resources when loaded after Ogre::root init
    Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("OpenHMD");

    // Create a render window for the HMD
    // Window size is set here and may need to be adjusted
    root = rviz::RenderSystem::get()->root();
    mWindow = root->createRenderWindow("VRwindow", 2000, 1000, true);
    std::cout << "Render window created" << std::endl;

    // Create the camera node
    if (mSceneNode)
        mCamera = mSceneNode->createChildSceneNode("StereoCameraNode");
    else
        mCamera = mSceneMgr->getRootSceneNode()->createChildSceneNode("StereoCameraNode");

    // Create the cameras, will act as the eyes
    stereo_cam_left = mSceneMgr->createCamera("StereoCameraLeft");
    stereo_cam_right = mSceneMgr->createCamera("StereoCameraRight");

    // Spawn position of the cameras
    // This can be used to manipulate the initial point of view
    mCamera->setPosition(Ogre::Vector3(0,0,10));
    std::cout << "Cameras Created" << std::endl;

    // Setup OpenHMD object
    openhmd = new OpenHMD();
    openhmd->init();

    // Set the orientation of the cameras position
    stereo_cam_left->setPosition(openhmd->getLeftViewMatrix().getTrans());
    stereo_cam_right->setPosition(openhmd->getRightViewMatrix().getTrans());

    // Set clip distances (something to do with focus I think)
    stereo_cam_left->setNearClipDistance(0.000012);
    stereo_cam_left->setFarClipDistance(200*120);
    stereo_cam_right->setNearClipDistance(0.000012);
    stereo_cam_right->setFarClipDistance(200*120);
    std::cout << "Cameras positioned" << std::endl;

    // Put the cameras on the camera node
    stereo_cam_left->detachFromParent();
    stereo_cam_right->detachFromParent();

    mCamera->attachObject(stereo_cam_left);
    mCamera->attachObject(stereo_cam_right);

    // Setup viewports
    mWindow->removeAllViewports();
    leftVP = mWindow->addViewport(stereo_cam_left, 1, 0, 0, 0.5f, 1.0f);
    rightVP = mWindow->addViewport(stereo_cam_right, 2, 0.5f, 0, 0.5f, 1.0f);

    // Debugging checks
    if(leftVP == NULL) 
        std::cout << "leftVP is NULL!!!" << std::endl;
    else {
        // leftVP->setBackgroundColour(Ogre::ColourValue(0.145f, 0.25f, 0.4f));
        // rightVP->setBackgroundColour(Ogre::ColourValue(0.145f, 0.25f, 0.4f));
        std::cout << "Viewports added to window" << std::endl;
    }

    // Get physical screen resolution and use closest available compositor with stretching
    Ogre::Vector2 hmdScreenSize = openhmd->getScreenSize();

    // Choose which compositor to use (vive should be "HMD/GenericAutoScaling")
    if (!openhmd->isDummy())
    {
        if (hmdScreenSize[0] == 1280 && hmdScreenSize[1] == 800) //assume Oculus DK1 shader
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/OculusDK1");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/OculusDK1");
            leftComp->setEnabled(true);
            rightComp->setEnabled(true);
        }
        else if (hmdScreenSize[0] == 1920 && hmdScreenSize[1] == 1080) //assume Oculus DK2 shader
        {
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/OculusDK2");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/OculusDK2");
            leftComp->setEnabled(true);
            rightComp->setEnabled(true);
        }
        else
        {
            std::cout << "HMD generic" << std::endl;
            Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
            Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");

            // Debugging checks
            if(leftComp == NULL)
                std::cout << "leftComp is NULL" << std::endl;
            else {
                std::cout << "Enabling compositors" << std::endl;
                // Enable the compositors to show images
                leftComp->setEnabled(true);
                rightComp->setEnabled(true);
            }
        }
    }
    else
    {
        std::cout << "HMD is a dummy" << std::endl;
        Ogre::CompositorInstance* leftComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(leftVP, "HMD/GenericAutoScaling");
        Ogre::CompositorInstance* rightComp = Ogre::CompositorManager::getSingletonPtr()->addCompositor(rightVP, "HMD/GenericAutoScaling");
        leftComp->setEnabled(true);
        rightComp->setEnabled(true);
    }
    std::cout << "HMD fully initialized" << std::endl;
}

/************ Called by RViz ************/
void OpenhmdDisplay::update(float wall_dt, float ros_dr)
{
    // Force update the render window
    mWindow->update();

    // Update HMD
    openhmd->update();

    // Unknown for now
    stereo_cam_left->setCustomProjectionMatrix(true, openhmd->getLeftProjectionMatrix().transpose());
    stereo_cam_right->setCustomProjectionMatrix(true, openhmd->getRightProjectionMatrix().transpose());

    // Get the orientation to update cameras
    Ogre::Quaternion oculusCameraOrientation = openhmd->getQuaternion();
    // std::cout << oculusCameraOrientation << std::endl;
    stereo_cam_left->setOrientation(oculusCameraOrientation);
    stereo_cam_right->setOrientation(oculusCameraOrientation);
}

/************ Empty display reset ************/
void OpenhmdDisplay::reset()
{
}

};

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz_openhmd::OpenhmdDisplay, rviz::Display)