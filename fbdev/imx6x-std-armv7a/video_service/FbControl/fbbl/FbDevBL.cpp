/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 10:23:30 AM CST
 Description: 
 ************************************************************************/

#include "FbDevBL.hpp"

using namespace std;
using namespace vehicle::videoservice;
using namespace fbdev;

/********* PUBLIC ************/

FbDevBL::FbDevBL():
    state_(new FbDevStateHome), // home is init state
    color_key_(0x000000) // RGB Black
{
    fb0_ = IFbDev::getInstance();
    fb1_ = IFbDev::getInstance();
    fb0_->init("/dev/fb0");
    fb1_->init("/dev/fb1");
}

bool FbDevBL::setFb(FbState state)
{
    switch (state)
    {
        case kHome:
            return state_->toHome(*this);
        case kAnimation:
            return state_->toAnimation(*this);
        case kCamera:
            return state_->toCamera(*this);
    }
    return false;
}

void FbDevBL::setCurrentState(FbDevState *state)
{
    state_ = state;
}

/********* PRIVATE ************/

FbDevBL::~FbDevBL()
{
    fb0_->deinit();
    fb1_->deinit();
    delete fb0_;
    delete fb1_;
}
