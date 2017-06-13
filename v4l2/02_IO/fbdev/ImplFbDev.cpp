/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 15 May 2017 04:00:25 PM CST
 Description: 
 ************************************************************************/

#include <iostream>
#include <cmath>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/videodev2.h>

#include "ImplFbDev.h"
#include "IFileParser.h"
#include "IPixFmt.h"

using namespace fbdev;
using namespace std;

#define CHECK_INVOC_ORDER \
    do {                                                \
        if (m_fd == -1){                                \
            cerr << "Please call init() first!" << endl;\
            return false;                               \
        }                                               \
    }while(0)


/********* PUBLIC ************/


ImplFbDev::ImplFbDev():
    m_default_pix_fmt(V4L2_PIX_FMT_RGB565),
    m_default_width(1280),
    m_default_height(720),
    m_fd(-1),
    m_fb_buf(NULL),
    m_fb_size(0)
{
    memset(&m_finfo, 0, sizeof(struct fb_fix_screeninfo));
    memset(&m_vinfo, 0, sizeof(struct fb_var_screeninfo));
}


ImplFbDev::~ImplFbDev()
{ /* empty */ }

void ImplFbDev::drawPixel(int x, int y, uint32_t pix_content)
{
    int offset = (x+m_vinfo.xoffset) * m_vinfo.bits_per_pixel/8 + (y+m_vinfo.yoffset) * m_finfo.line_length;

    switch (m_vinfo.nonstd)
    {
        case V4L2_PIX_FMT_UYVY:
            // only draw the even pixels
            if (x % 2 == 0)
                *((uint32_t*)(m_fb_buf+offset)) = pix_content;
            break;
        case V4L2_PIX_FMT_RGB565:
            *((uint16_t*)(m_fb_buf+offset)) = (uint16_t)(pix_content);
            break;
        default:
            std::cerr << "Unsupported format: " << IPixFmt::fourccToStr(m_vinfo.nonstd) << std::endl;
            break;
    }
}


bool ImplFbDev::init(string dev)
{

    /* open device */

    m_fd = open(dev.c_str(), O_RDWR);
    if (m_fd == -1)
    {
        cerr << "Open failed: " << strerror(errno) << endl;
        return false;
    }

    /* get-then-put default screen variable info */

    if (0 != ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo))
    {
        cerr << "Get var screen info failed: " << strerror(errno) << endl;
        return false;
    }


    IFileParser *parser = IFileParser::getInstance();
    m_vinfo.xres = strtol(parser->getInfo()["FB"]["xres"].c_str(), NULL, 10);
    m_vinfo.yres = strtol(parser->getInfo()["FB"]["yres"].c_str(), NULL, 10);
    m_vinfo.xres_virtual = strtol(parser->getInfo()["FB"]["xres_virtual"].c_str(), NULL, 10);
    m_vinfo.yres_virtual = strtol(parser->getInfo()["FB"]["yres_virtual"].c_str(), NULL, 10);
    m_vinfo.nonstd = IPixFmt::strToFourcc(parser->getInfo()["FB"]["pix_fmt"]);
    m_vinfo.bits_per_pixel = IPixFmt::bpp(m_vinfo.nonstd);

    if (0 != ioctl(m_fd, FBIOPUT_VSCREENINFO, &m_vinfo))
    {
        cerr << "Put var screen info failed: " << strerror(errno) << endl;
        return false;
    }

    /* get the updated vinfo */
    if (0 != ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo))
    {
        cerr << "Get var screen info (2nd time) failed: " << strerror(errno) << endl;
        return false;
    }

    /* get screen fix info */
    if (0 != ioctl(m_fd, FBIOGET_FSCREENINFO, &m_finfo))
    {
        cerr << "Get fix screen info failed: " << strerror(errno) << endl;
        return false;
    }

    /* mmap fb */
    m_fb_size = m_vinfo.yres_virtual * m_finfo.line_length;
    m_fb_buf = (uint8_t*)mmap(NULL, m_fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (m_fb_buf == MAP_FAILED)
    {
        cerr << "Mmap fb buffer failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

bool ImplFbDev::deinit()
{
    CHECK_INVOC_ORDER;

    // unmap fb buffer
    munmap(m_fb_buf, m_fb_size);
    m_fb_size = 0;

    // close fb
    if (0 != close(m_fd))
    {
        cerr << "Close fb failed: " << strerror(errno) << endl;
        return false;
    }

    // reset internal members
    m_fd = -1;
    memset(&m_finfo, 0, sizeof(struct fb_fix_screeninfo));
    memset(&m_vinfo, 0, sizeof(struct fb_var_screeninfo));

    return true;
}

bool ImplFbDev::setPixFormat(uint32_t pix_fmt)
{
    CHECK_INVOC_ORDER;

    if (!(m_vinfo.bits_per_pixel = IPixFmt::bpp(pix_fmt)))
        return false;
    m_vinfo.nonstd = pix_fmt;


    /* set vinfo with fmt related members changed */
    if (0 != ioctl(m_fd, FBIOPUT_VSCREENINFO, &m_vinfo))
    {
        cerr << "Put var screen info failed: " << strerror(errno) << endl;
        return false;
    }

    /* get the updated vinfo */
    if (0 != ioctl(m_fd, FBIOGET_VSCREENINFO, &m_vinfo))
    {
        cerr << "Get var screen info failed: " << strerror(errno) << endl;
        return false;
    }

    /* get screen fix info since vinfo will affect it */
    if (0 != ioctl(m_fd, FBIOGET_FSCREENINFO, &m_finfo))
    {
        cerr << "Get fix screen info failed: " << strerror(errno) << endl;
        return false;
    }

    /* update mmaped fb buffer */
    munmap(m_fb_buf, m_fb_size);
    m_fb_size = m_vinfo.yres_virtual * m_finfo.line_length;
    m_fb_buf = (uint8_t*)mmap(NULL, m_fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
    if (m_fb_buf == MAP_FAILED)
    {
        cerr << "Mmap fb buffer failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

#ifdef MXCFB
bool ImplFbDev::setGlobalAlpha(uint16_t alpha)
{
    CHECK_INVOC_ORDER;

    struct mxcfb_gbl_alpha gbl_alpha;
    gbl_alpha.enable = 1;
    gbl_alpha.alpha = alpha;

    if (0 != ioctl(m_fd, MXCFB_SET_GBL_ALPHA, &gbl_alpha))
    {
        cerr << "Set global alpha failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

// Note: the input "color" is of RGB24 pixel format
bool ImplFbDev::setColorKey(uint32_t color)
{
    CHECK_INVOC_ORDER;

    struct mxcfb_color_key ckey;
    ckey.enable = 1;
    ckey.color_key = (color);

    if (0 != ioctl(m_fd, MXCFB_SET_CLR_KEY, &ckey))
    {
        cerr << "Set color key failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

bool ImplFbDev::unsetColorKey()
{
    CHECK_INVOC_ORDER;

    struct mxcfb_color_key ckey;
    ckey.enable = 0;

    if (0 != ioctl(m_fd, MXCFB_SET_CLR_KEY, &ckey))
    {
        cerr << "Set color key failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

bool ImplFbDev::blank()
{
    CHECK_INVOC_ORDER;

    if (0 != ioctl(m_fd, FBIOBLANK, FB_BLANK_NORMAL))
    {
        cerr << "Blank failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}

bool ImplFbDev::unBlank()
{
    CHECK_INVOC_ORDER;

    if (0 != ioctl(m_fd, FBIOBLANK, FB_BLANK_UNBLANK))
    {
        cerr << "Blank failed: " << strerror(errno) << endl;
        return false;
    }

    return true;
}
#endif
