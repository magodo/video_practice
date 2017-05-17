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

bool FbDevBL::init()
{
    return state_->init();
}

bool FbDevBL::deinit()
{
    return state_->deinit();
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

FbDevBL::FbDevBL():
    state_(new FbDevStateHome), // home is init state
    color_key_(0x000000) // RGB Black
{}

FbDevBL::~FbDevBL()
{}
