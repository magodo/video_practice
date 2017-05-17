/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 06:21:56 PM CST
 Description: 
 ************************************************************************/

#include "FbDevState.hpp"

using namespace vehicle::videoservice;

// Base

IFbDev *FbDevState::fb0_;
IFbDev *FbDevState::fb1_;

bool FbDevState::init()
{
    fb0_ = IFbDev::getInstance();
    fb1_ = IFbDev::getInstance();

    return (fb0_->init("/dev/fb0") && fb1_->init("/dev/fb1"));
}

bool FbDevState::deinit()
{
    if (!fb0_->deinit() || !fb1_->deinit()) 
        return false;

    delete fb0_;
    delete fb1_;

    return true;
}

FbDevState::~FbDevState()
{}

// Home

bool FbDevStateHome::toAnimation(FbDevBL& bl)
{
    fb1_->unBlank();

    bl.setCurrentState(new FbDevStateAnimation);
    delete this;

    return true;
}


bool FbDevStateHome::toCamera(FbDevBL& bl)
{
    fb0_->setLocalAlpha();
    fb1_->unBlank();
    fb0_->setColorKey(bl.getColorKey());

    bl.setCurrentState(new FbDevStateCamera);
    delete this;

    return true;
}

// Animation

bool FbDevStateAnimation::toHome(FbDevBL& bl)
{
    fb1_->blank();

    bl.setCurrentState(new FbDevStateHome);
    delete this;

    return true;
}

bool FbDevStateAnimation::toCamera(FbDevBL& bl)
{
    fb0_->setColorKey(bl.getColorKey());
    fb0_->setLocalAlpha();

    bl.setCurrentState(new FbDevStateCamera);
    delete this;

    return true;
}

// Camera

bool FbDevStateCamera::toHome(FbDevBL& bl)
{
    fb0_->unsetColorKey();
    fb1_->blank();
    fb1_->setGlobalAlpha(0xff);

    bl.setCurrentState(new FbDevStateHome);
    delete this;

    return true;
}

bool FbDevStateCamera::toAnimation(FbDevBL& bl)
{
    fb1_->setGlobalAlpha(0xff);
    fb0_->unsetColorKey();

    bl.setCurrentState(new FbDevStateAnimation);
    delete this;

    return true;
}


