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

#pragma once

#ifndef Q_MOC_RUN
#include "rviz/display.h"
#endif

#include <OGRE/OgreTexture.h>
#include <OGRE/RenderSystems/GL/OgreGLTexture.h>

#include <rviz/display_context.h>
#include <rviz/view_manager.h>
#include <rviz/ogre_helpers/render_system.h>
#include <iostream>

#include "OpenHMD.h"

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreRenderTarget.h>
#include <OgreRenderSystem.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreCompositorManager.h>
#include <OgreCompositorInstance.h>
#include <OgreCompositorChain.h>

namespace rviz_openhmd
{

class OpenhmdDisplay: public rviz::Display
{
Q_OBJECT
public:
    OpenhmdDisplay();
    virtual ~OpenhmdDisplay();

    virtual void onInitialize();
    virtual void update(float wall_dt, float ros_dt);
    virtual void reset();

private:
    // OGRE Objects
    Ogre::SceneNode* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
    Ogre::Root* root;
    Ogre::Camera* stereo_cam_left;
    Ogre::Camera* stereo_cam_right;
    Ogre::Viewport* leftVP;
    Ogre::Viewport* rightVP;
    Ogre::SceneNode* mSceneNode;
    Ogre::SceneManager* _pSceneManager;

    //OpenHMD objects
    OpenHMD* openhmd;

    // Rviz stuff
    rviz::DisplayContext* mDispCtx;
    static rviz::RenderSystem* mRendSys;
    rviz::DisplayContext* _pDisplayContext;
};

};
