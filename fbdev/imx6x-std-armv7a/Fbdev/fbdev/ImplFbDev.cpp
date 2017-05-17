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

#include "ImplFbDev.hpp"

using namespace fbdev;
using namespace std;

#define CHECK_INVOC_ORDER \
    do {                                                \
        if (m_fd == -1){                                \
            cerr << "Please call init() first!" << endl;\
            return false;                               \
        }                                               \
    }while(0)


/********* PRIVATE ************/

uint32_t ImplFbDev::bppOfFmt(uint32_t pix_fmt)
{
    switch (pix_fmt)
    {
        case pixfmt::UYVY:
            return 32;
        case pixfmt::RGB565:
            return 16;
        default:
            char tmp[5] = {0};
            for (int i = 0; i < 4; ++i)
                tmp[i] = *((uint8_t*)(&pix_fmt)+i);
            cerr << "Unknown format: " << tmp << endl;
            return 0;
    }
}

uint32_t ImplFbDev::colorFromRGB24(uint32_t clr_rgb24)
{

    uint32_t color;
    color = (((uint8_t)((double)(clr_rgb24>>16 & 0xff)/0xff * (pow(2, m_vinfo.red.length)  -1))   << m_vinfo.red.offset ) |
            ( (uint8_t)((double)(clr_rgb24>> 8 & 0xff)/0xff * (pow(2, m_vinfo.green.length)-1))   << m_vinfo.green.offset ) |
            ( (uint8_t)((double)(clr_rgb24>> 0 & 0xff)/0xff * (pow(2, m_vinfo.blue.length) -1))   << m_vinfo.blue.offset ));

#ifdef DEBUG
    static unsigned counter = 0;
    ++counter;

    // output the transfered color once
    if (counter == m_vinfo.xres_virtual*m_vinfo.yres_virtual)
    {
        char tmp[5] = {0};
        for (int i = 0; i < 4; ++i)
            tmp[i] = *((uint8_t*)(&m_vinfo.nonstd)+i);
        cout << hex << "color: 0x" << clr_rgb24 << "(rgb24) -> 0x"  << color << "(" << tmp << ")" << endl;
        cout << dec;
        counter = 0;
    }
#endif

    return color;
}

uint32_t ImplFbDev::colorToRGB24(uint32_t color)
{
    uint32_t clr_rgb24;

    clr_rgb24 = (((uint8_t)((double)(color>>m_vinfo.red.offset   & (uint8_t)(pow(2,m_vinfo.red.length)-1))    /(pow(2,m_vinfo.red.length)-1)   * 0xff) << 16) |
                 ((uint8_t)((double)(color>>m_vinfo.green.offset & (uint8_t)(pow(2,m_vinfo.green.length)-1))/(pow(2,m_vinfo.green.length)-1) * 0xff) << 8) |
                 ((uint8_t)((double)(color>>m_vinfo.blue.offset  & (uint8_t)(pow(2,m_vinfo.blue.length)-1))  /(pow(2,m_vinfo.blue.length)-1)  * 0xff) << 0));
    return clr_rgb24;

}

/********* PUBLIC ************/


ImplFbDev::ImplFbDev():
    m_default_pix_fmt(pixfmt::RGB565),
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
        case pixfmt::UYVY:
            // only draw the even pixels
            if (x % 2 == 0)
                *((uint32_t*)(m_fb_buf+offset)) = pix_content;
            break;
        case pixfmt::RGB565:
            *((uint16_t*)(m_fb_buf+offset)) = (uint16_t)(pix_content);
            break;
        default:
            cerr << "current pixel format is not supported!" << endl;
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

    if (!(m_vinfo.bits_per_pixel = bppOfFmt(pix_fmt)))
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

bool ImplFbDev::setLocalAlpha()
{
    CHECK_INVOC_ORDER;

    /* enable local alpha */
    struct mxcfb_loc_alpha loc_alpha;

    loc_alpha.enable = 1;

    switch (m_vinfo.nonstd)
    {
        case pixfmt::RGB565:
            loc_alpha.alpha_in_pixel = 0;
            break;
        case pixfmt::RGB32:
            loc_alpha.alpha_in_pixel = 1;
            break;
    }
    
    if (0 != ioctl(m_fd, MXCFB_SET_LOC_ALPHA, &loc_alpha))
    {
        cerr << "Set local alpha failed: " << strerror(errno) << endl;
        return false;
    }

    /* set each pixel's local alpha to opaque */
    int alpha_size = m_vinfo.xres * m_vinfo.yres; // every pixel has a corresponding 1-byte local alpha
    uint8_t *alpha_buf = (uint8_t*)mmap(NULL, alpha_size, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, loc_alpha.alpha_phy_addr0); // TODO: meaning of addr1?
    if (alpha_buf == MAP_FAILED)
    {
        cerr << "Mmap local alpha buffer failed: " << strerror(errno) << endl;
        return false;
    }
    memset(alpha_buf, 0xff, alpha_size);
    munmap(alpha_buf, alpha_size);

    return true;
}

// Note: the input "color" is of current pixel format of fb
bool ImplFbDev::setColorKey(uint32_t color)
{
    CHECK_INVOC_ORDER;

    struct mxcfb_color_key ckey;
    ckey.enable = 1;
    ckey.color_key = colorToRGB24(color); // seems mxcfb is always using rgb24 for clr_key setting

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
