/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 13 Jun 2017 04:26:59 PM CST
 Description: 
 ************************************************************************/

#include "IFileParser.h"
#include "IPixFmt.h"
#include "IFbDev.h"
#include "ICapture.h"
#include <cstring>
#include <unistd.h>

using namespace fbdev;

int main()
{
    IFileParser::getInstance()->parseFile("./my_config.ini");

    IFbDev *fb1 = IFbDev::getInstance();
    ICapture *cap = ICapture::getInstance();
    
    fb1->init("/dev/fb1");
    cap->Open("/dev/video0");
    cap->Init();

#ifdef MXCFB
    fb1->setGlobalAlpha(0xff);
    fb1->unBlank();
#endif

    cap->StreamOn();

    uint8_t *addr, *fb_buf;
    int index;
    size_t fb_size;

    fb_buf = fb1->getVirtualFbAddr();
    fb_size = fb1->getVirtualFbSize();

    while (1)
    {
        index = cap->DequeOneBuffer(&addr);
        memcpy(fb_buf, addr, fb_size);
        usleep(40000);
        cap->EnqueOneBuffer(index);
    }
}
