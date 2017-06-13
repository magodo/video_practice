/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 13 Jun 2017 04:07:04 PM CST
 Description: 
 ************************************************************************/

#include "ImplCapture.h"

ICapture *ICapture::getInstance()
{
    return new ImplCapture;
}
