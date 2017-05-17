/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 05:56:59 PM CST
 Description: 
 ************************************************************************/

#include "IFbDevBL.hpp"
#include "FbDevBL.hpp"

using namespace vehicle::videoservice;

IFbDevBL *IFbDevBL::getInstance()
{
    static IFbDevBL *instance = new FbDevBL;
    return instance;
}

