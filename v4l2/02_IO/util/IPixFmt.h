/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Sat 27 May 2017 04:31:49 PM CST
 Description: 
 ************************************************************************/

#ifndef __IPIXFMT_H__
#define __IPIXFMT_H__

#include <cstdint>
#include <string>
#include "linux/videodev2.h"

class IPixFmt
{
    public:
        static uint32_t bpp(uint32_t pix_fmt);
        static uint32_t strToFourcc(std::string pix_fmt_str);
        static std::string fourccToStr(uint32_t pix_fmt);
};

#endif
