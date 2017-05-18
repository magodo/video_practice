/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 06:21:56 PM CST
 Description: 
 ************************************************************************/

#include "FbDevState.hpp"

using namespace vehicle::videoservice;

// Home

bool FbDevStateHome::toAnimation(FbDevBL& bl)
{
    bl.fb1_->setGlobalAlpha(0xff);
    bl.fb1_->unBlank();

    bl.setCurrentState(new FbDevStateAnimation);
    delete this;

    return true;
}


bool FbDevStateHome::toCamera(FbDevBL& bl)
{
    bl.fb1_->setGlobalAlpha(0xff);
    bl.fb0_->setLocalAlpha();
    bl.fb0_->setColorKey(bl.getColorKey());
    bl.fb1_->unBlank();

    bl.setCurrentState(new FbDevStateCamera);
    delete this;

    return true;
}

// Animation

bool FbDevStateAnimation::toHome(FbDevBL& bl)
{
    bl.fb1_->blank();

    bl.setCurrentState(new FbDevStateHome);
    delete this;

    return true;
}

bool FbDevStateAnimation::toCamera(FbDevBL& bl)
{
    bl.fb0_->setColorKey(bl.getColorKey());
    bl.fb0_->setLocalAlpha();

    bl.setCurrentState(new FbDevStateCamera);
    delete this;

    return true;
}

// Camera

bool FbDevStateCamera::toHome(FbDevBL& bl)
{
    bl.fb0_->unsetColorKey();
    bl.fb1_->blank();

    bl.setCurrentState(new FbDevStateHome);
    delete this;

    return true;
}

bool FbDevStateCamera::toAnimation(FbDevBL& bl)
{
    bl.fb0_->unsetColorKey();
    bl.fb1_->setGlobalAlpha(0xff);

    bl.setCurrentState(new FbDevStateAnimation);
    delete this;

    return true;
}


