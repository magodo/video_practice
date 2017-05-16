/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 15 May 2017 05:54:37 PM CST
 Description: 
 ************************************************************************/

#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include "ImplFbDev.hpp"

using namespace std;
using namespace Fbdev;

#define RGB32_RED       0xff0000
#define RGB32_GREEN     0x00ff00
#define RGB32_BLUE      0x0000ff
#define RGB32_BLACK     0x000000

#define RGB565_RED      0xf800
#define RGB565_GREEN    0x07c0
#define RGB565_BLUE     0x001f

int main()
{
    ImplFbDev fb0, fb1;

    fb0.init("/dev/fb0");
    fb1.init("/dev/fb1");

#if 0
    // set fb1 pix_fmt to RGB565
    fb1.setPixFormat(PixFmt::RGB565);

    fb1.setGlobalAlpha(0xff);
    cout << "fb1: set global alpha -> 0xff" << endl;
    sleep(3);
    
    // draw fb1 as "blue"
    for (uint16_t x = 0; x < fb1.getXRes(); ++x)
        for (uint16_t y = 0; y < fb1.getYRes(); ++y)
        {
            fb1.drawPixel(x,y,RGB565_BLUE);
        }
    cout << "fb1: draw" << endl;
    sleep(3);

    // set fb0 local alpha 
    fb0.setLocalAlpha();
    cout << "fb0: set local alpha" << endl;
    sleep(3);

    // unblank fb1
    fb1.unBlank();
    cout << "fb1: unblank" << endl;
    sleep(3);

    fb0.setColorKey(RGB32_BLACK);
    cout << "fb0: set color key" << endl;
    sleep(3);
#endif
    fb0.unsetColorKey();
    fb1.blank();
    fb1.setGlobalAlpha(0xff);
}
