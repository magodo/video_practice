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
#include <iostream>
#include <unistd.h>
#include <signal.h>

using namespace fbdev;

bool g_is_on = true;

void SigHandler(int signo)
{
    switch (signo) 
    {
        case SIGINT:
            g_is_on = false;
            break;
    }
}

int main()
{
    /* setup signal SIGINT */

    struct sigaction act;

    act.sa_handler = SigHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (-1 == sigaction(SIGINT, &act, NULL))
    {
        perror("sigaction");
        exit(-1);
    }


    IFileParser::getInstance()->parseFile("./my_config.ini");

    IFbDev *fb = IFbDev::getInstance();
    ICapture *cap = ICapture::getInstance();
    
#ifdef MXCFB
    fb->init("/dev/fb1");
#else
    fb->init("/dev/fb0");
#endif
    cap->Open("/dev/video0");
    cap->Init();

#ifdef MXCFB
    fb->setGlobalAlpha(0xff);
    fb->unBlank();
#endif

    cap->StreamOn();

    uint8_t *addr, *fb_buf;
    int index;
    size_t fb_size;

    fb_buf = fb->getVirtualFbAddr();
    fb_size = cap->GetImageSize();
    std::cout << "image siz: " << fb_size << std::endl;
    std::cout << "fb size: " << fb->getVirtualFbSize() << std::endl;;

    while (g_is_on)
    {
        index = cap->DequeOneBuffer(&addr);
        if (index >= 0)
        {
            memcpy(fb_buf, addr, fb_size);
            cap->EnqueOneBuffer(index);
        }
    }

    fb->deinit();
    cap->Deinit();
    cap->Close();
}
