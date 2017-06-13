/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 05:41:00 PM CST
 Description: 
 ************************************************************************/

#include "IFbDev.hpp"
#include "ImplFbDev.hpp"

using namespace fbdev;

IFbDev *IFbDev::getInstance()
{
    return new ImplFbDev;
}

IFbDev::~IFbDev()
{}
