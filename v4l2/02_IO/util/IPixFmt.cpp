/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Sat 27 May 2017 04:37:52 PM CST
 Description: 
 ************************************************************************/

#include <iostream>

#include "IPixFmt.h"

uint32_t IPixFmt::bpp(uint32_t pix_fmt)
{
    switch (pix_fmt)
    {
        case V4L2_PIX_FMT_UYVY:
        case V4L2_PIX_FMT_RGB565:
        case V4L2_PIX_FMT_YUYV:
            return 16;
        case  V4L2_PIX_FMT_RGB32:
            return 32;
        default:
            std::cerr << "Not supported format: " <<  fourccToStr(pix_fmt) << std::endl;
            return 0;
    }
}

uint32_t IPixFmt::strToFourcc(std::string pix_fmt_str)
{
    uint32_t pix_fmt;

    *((uint8_t*)(&pix_fmt)+0) = pix_fmt_str.c_str()[0];
    *((uint8_t*)(&pix_fmt)+1) = pix_fmt_str.c_str()[1];
    *((uint8_t*)(&pix_fmt)+2) = pix_fmt_str.c_str()[2];
    *((uint8_t*)(&pix_fmt)+3) = pix_fmt_str.c_str()[3];

    return pix_fmt;
}

std::string IPixFmt::fourccToStr(uint32_t pix_fmt)
{
    std::string str;
    str.insert(0, 1, (pix_fmt >> 24) & 0xff);
    str.insert(0, 1, (pix_fmt >> 16) & 0xff);
    str.insert(0, 1, (pix_fmt >> 8) & 0xff);
    str.insert(0, 1, pix_fmt & 0xff);
    return str;
}
