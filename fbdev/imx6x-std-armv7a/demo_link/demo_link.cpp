/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 15 May 2017 05:54:37 PM CST
 Description: 
 ************************************************************************/

#include <stdint.h>
#include "ImplFbDev.hpp"

using namespace Fbdev;

int main()
{
    ImplFbDev obj;

    obj.init("/dev/fb1");
    obj.setGlobalAlpha(0xff);

    uint32_t primitive_clrs[3] = {0xff0000, 0x00ff00, 0x0000ff};

    int index;
    for (uint16_t x = 0; x < obj.getXRes(); ++x)
        for (uint16_t y = 0; y < obj.getYRes(); ++y)
        {
            index = y / (obj.getYRes()/3);
            obj.drawPixel(x, y, primitive_clrs[index]);
        }
    obj.uninit();
}
