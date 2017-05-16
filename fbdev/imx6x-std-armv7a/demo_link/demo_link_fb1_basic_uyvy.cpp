/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 15 May 2017 05:54:37 PM CST
 Description: 
 ************************************************************************/

#include <stdint.h>
#include <iostream>
#include "ImplFbDev.hpp"

using namespace std;
using namespace Fbdev;

#define RGB24_RED       0xff0000
#define RGB24_GREEN     0x00ff00
#define RGB24_BLUE      0x0000ff
#define RGB24_BLACK     0x000000

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07c0
#define RGB565_BLUE     0x001f

#define UYVY_GREEN      0xff00ff00
#define UYVY_PURPLE     0xffffffff

int main()
{
    ImplFbDev fb1;

    fb1.init("/dev/fb1");

    // unblank fb1
    fb1.unBlank();

    fb1.setPixFormat(PixFmt::UYVY);

    uint32_t colors[2] = {UYVY_PURPLE, UYVY_GREEN};
    // draw fb1
    for (uint16_t x = 0; x < fb1.getXRes(); ++x)
        for (uint16_t y = 0; y < fb1.getYRes(); ++y)
        {
            fb1.drawPixel(x, y, colors[y/(fb1.getYRes()/2)]);
        }

    fb1.setLocalAlpha();
    fb1.setColorKey(UYVY_GREEN); // not working
}
