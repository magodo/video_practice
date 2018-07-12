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
#include <sys/time.h>

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

#ifdef MXCFB
    fb->setGlobalAlpha(0xff);
    fb->unBlank();
#endif

    cap->StreamOn();

    uint8_t *addr, *fb_buf;
    int index;
    size_t fb_size, img_size;

    fb_buf = fb->getVirtualFbAddr();
    img_size = cap->GetBufferSize();
    fb_size = fb->getVirtualFbSize();
    std::cout << "image size: " << img_size << std::endl;
    std::cout << "fb size: " << fb_size << std::endl;;

    int field_counter = 0;
    struct timespec tsp_start, tsp_end;
    clock_gettime(CLOCK_MONOTONIC, &tsp_start);

    while (g_is_on)
    {
        index = cap->DequeOneBuffer(&addr);
        if (index >= 0)
        {
            //memcpy(fb_buf, addr, img_size);
            fb->renderFrame(addr,img_size); // low FPS
            cap->EnqueOneBuffer(index);
        }

        field_counter++;
        if (field_counter == 100)
        {
            clock_gettime(CLOCK_MONOTONIC, &tsp_end);
            std::cout << "FPS: " << 100/(tsp_end.tv_sec - tsp_start.tv_sec +
                         ((double)(tsp_end.tv_nsec-tsp_start.tv_nsec)/1000000000)) << std::endl;
            field_counter = 0;
            clock_gettime(CLOCK_MONOTONIC, &tsp_start);
        }
    }

    fb->deinit();
    cap->StreamOff();
    cap->Close();
}
