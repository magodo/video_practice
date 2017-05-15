/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Fri 05 May 2017 03:19:58 PM CST
 Description: 
 ************************************************************************/

#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <cstdlib>
#include <linux/videodev2.h>
#include <linux/mxcfb.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <string>

using namespace std;

#if 0
// Found this here: http://www.emdebian.org/~zumbi/efika.OLD/xorg-video-imx/src/imx_xv_ipu.c
// Maybe it will help?
#define RGB565TOCOLORKEY(rgb)                              \
      ( ((rgb & 0xf800)<<8)  |  ((rgb & 0xe000)<<3)  |     \
        ((rgb & 0x07e0)<<5)  |  ((rgb & 0x0600)>>1)  |     \
        ((rgb & 0x001f)<<3)  |  ((rgb & 0x001c)>>2)  )   
#endif

#define RGB565_RED      (0x1f << 11)
#define RGB565_GREEN    (0x3f << 5)
#define RGB565_BLUE     (0x1f)

#define RGB24_RED       0xff0000
#define RGB24_GREEN     0x00ff00
#define RGB24_BLUE      0x0000ff

void exit_msg(string msg)
{
    cerr << msg << endl;
    exit(1);
}

//----------------------------------------
#define DEBUG
//uint64_t pix_fmt = V4L2_PIX_FMT_UYVY;
uint64_t pix_fmt = V4L2_PIX_FMT_RGB565;

uint16_t width = 1280; // pixels
uint16_t height = 720;
//----------------------------------------


/* set color key*/
void set_color_key(int fb_fd, uint32_t color_key)
{
    struct mxcfb_color_key ckey;

    ckey.enable = 1;
    ckey.color_key = color_key;

    if (0 != ioctl(fb_fd, MXCFB_SET_CLR_KEY, &ckey))
        exit_msg("set color key failed");
}

void unset_color_key(int fb_fd)
{
    struct mxcfb_color_key ckey;

    ckey.enable = 0;
    ckey.color_key = 0;
    if (0 != ioctl(fb_fd, MXCFB_SET_CLR_KEY, &ckey))
        exit_msg("unset color key failed");
}

/* draw a pixel */
void draw_foobar(int x, int y, uint32_t color, struct fb_var_screeninfo *vinfo, struct fb_fix_screeninfo *finfo, uint8_t *fb_buf)
{
    int offset = (x+vinfo->xoffset) * vinfo->bits_per_pixel/8 + (y+vinfo->yoffset) * finfo->line_length;

    switch (pix_fmt)
    {
        case V4L2_PIX_FMT_UYVY:
            // we draw the 2 pixels at once
            if (x % 2 == 0)
            {
                *((uint32_t*)(fb_buf+offset)) = 0x00000000;
            }
            break;
        case V4L2_PIX_FMT_RGB565:
            *((uint16_t*)(fb_buf+offset)) = color;
            break;
        default:
            exit_msg("Unknown format!");
    }
}

int main()
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    int fb_fd;
    
    /* open fb device */

    fb_fd = open("/dev/fb1", O_RDWR);
    if (fb_fd == -1)
        exit_msg("Open failed");

    /* get-set screen variable info */

    if (0 != ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo))
        exit_msg("Get vinfo failed");

    vinfo.xres = width;
    vinfo.yres = height;
    vinfo.xres_virtual = vinfo.xres;
    vinfo.yres_virtual = vinfo.yres;

    vinfo.nonstd = pix_fmt;

    switch (pix_fmt)
    {
        case V4L2_PIX_FMT_UYVY:
            vinfo.bits_per_pixel = 16;
            break;
        case V4L2_PIX_FMT_RGB565:
            vinfo.bits_per_pixel = 16;
            break;
        default:
            exit_msg("Unknown format!");
    }

    if (0 != ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo))
        exit_msg("Set vinfo failed");
    if (0 != ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo))
        exit_msg("Get vinfo failed");

#ifdef DEBUG 
    cout << "offset(r,g,b,t): " << vinfo.red.offset << "," \
                                << vinfo.green.offset << "," \
                                << vinfo.blue.offset << "," \
                                << vinfo.transp.offset << endl;
    cout << "xres: " << vinfo.xres << endl \
         << "yres: " << vinfo.yres << endl \
         << "xres_v: " << vinfo.xres_virtual<< endl  \
         << "yres_v: " << vinfo.yres_virtual<< endl  \
         << endl;
#endif

    /* get screen fix info */

    if (0 != ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo))
        exit_msg("Get finfo failed");
#ifdef DEBUG 
    cout << "line_length: " << finfo.line_length << endl;
#endif
    
    /* enable local alpha and set all pixels as opaque */

    struct mxcfb_loc_alpha local_alpha_struct;

    local_alpha_struct.enable = 1;
    local_alpha_struct.alpha_in_pixel = 0;
    if (0 != ioctl(fb_fd, MXCFB_SET_LOC_ALPHA, &local_alpha_struct))
        exit_msg("Set local alpha failed");

    int alpha_size = vinfo.xres * vinfo.yres; // every pixel has a corresponding 1byte local alpha
    char *alpha_buffer = (char*)mmap(NULL, alpha_size, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, local_alpha_struct.alpha_phy_addr0);
    memset(alpha_buffer, 0xff, alpha_size*2/3); // the first 2/3 pixels are opaque
    memset(alpha_buffer+(alpha_size/3*2), 0x80, alpha_size/3); // the last 1/3 pixels are transparent
    munmap(alpha_buffer, alpha_size);

    /* mmap fb */

    int fb_size = vinfo.yres_virtual * finfo.line_length;
    uint8_t *fb_buf = (uint8_t*)mmap(NULL, fb_size, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
    if (fb_buf == MAP_FAILED)
        exit_msg("mmap failed");
    
    /* write something to fb */

    uint32_t colors[3] = {RGB565_RED, RGB565_GREEN, RGB565_BLUE};
    int index;

    for (uint16_t x = 0; x < vinfo.xres_virtual; ++x)
        for (uint16_t y = 0; y < vinfo.yres_virtual; ++y)
        {
            index = y / (height/3);
            draw_foobar(x, y, colors[index], &vinfo, &finfo, fb_buf);
        }
    
    /* set color key - wait - unset color key */

    set_color_key(fb_fd, RGB24_RED);
    sleep(2);
    unset_color_key(fb_fd);

    /* unmap fb */

    munmap(fb_buf, fb_size);
}
