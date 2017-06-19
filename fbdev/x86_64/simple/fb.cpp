/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 25 Apr 2017 09:03:57 PM CST
 Description: 
 ************************************************************************/

#include <linux/fb.h>
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;

void show_finfo(struct fb_fix_screeninfo *finfo)
{
    stringstream ss;
    ss << endl << "[FIX INFO]" << endl << endl;
    ss << "ID: " << finfo->id << endl;
    ss << "smem start: " << hex << "0x" << finfo->smem_start << dec << endl;
    ss << "smem length: " << finfo->smem_len << endl;
    ss << "type: "  << finfo->type << endl;
    ss << "type aux: " << finfo->type_aux << endl;
    ss << "visual: " << finfo->visual << endl;
    ss << "xpanstep: " << finfo->xpanstep << endl;
    ss << "ypanstep: " << finfo->ypanstep << endl;
    ss << "ywrapstep: " << finfo->ywrapstep << endl;
    ss << "linelength: " << finfo->line_length << endl;
    ss << endl;
    cout << ss.str();
}

void show_vinfo(struct fb_var_screeninfo *vinfo)
{
    stringstream ss;
    ss << endl << "[VAR INFO]" << endl << endl;
    ss << "visible xres: " << vinfo->xres << endl;
    ss << "visible yres: " << vinfo->yres << endl;
    ss << "virtual xres: " << vinfo->xres_virtual << endl;
    ss << "virtual yres: " << vinfo->yres_virtual << endl;
    ss << "xoffset: " << vinfo->xoffset << endl;
    ss << "yoffset: " << vinfo->yoffset << endl;
    ss << "bits per pixel: " << vinfo->bits_per_pixel << endl;
    ss << "grayscale: ";
    if (vinfo->grayscale)
        ss << "grayscale";
    else
        ss << "color";
    ss << endl;

    ss << "rgba [offset/length/msb_right]: " << vinfo->red.offset << "/" << vinfo->red.length << "/" << vinfo->red.msb_right << "," \
                   << vinfo->green.offset << "/" << vinfo->green.length << "/" << vinfo->green.msb_right << "," \
                   << vinfo->blue.offset << "/" << vinfo->blue.length << "/" << vinfo->blue.msb_right << "," \
                   << vinfo->transp.offset << "/" << vinfo->transp.length << "/" << vinfo->transp.msb_right << endl;

    ss << "standard pixel format: " << ((vinfo->nonstd)? "No":"Yes") << endl;
    ss << "activate: " << vinfo->activate << endl;
    ss << "height of pic (mm): " << vinfo->height << endl;
    ss << "width of pic (mm): " << vinfo->width << endl;

    ss << "pixclock(pico-sec): " << vinfo->pixclock << endl;
    ss << "margin (left/right/upper/lower): " << vinfo->left_margin << "/" \
       << vinfo->right_margin  << "/" \
       << vinfo->upper_margin  << "/" \
       << vinfo->lower_margin << endl;
    ss << "hsync length: " << vinfo->hsync_len << endl;
    ss << "vsync length: " << vinfo->vsync_len << endl;
    ss << "sync: " << vinfo->sync << endl;
    ss << "vmode: " << vinfo->vmode << endl;
    ss << "rotate: " << vinfo->rotate << endl;
    cout << ss.str();
}

/**
 * Draw a pixel at (x,y).
 *
 * x[in]: x axes
 * y[in]: y axes
 * pixel[in]: the pixel bits in some format
 * finfo[in]: fix fb info
 * vinfo[in]: var fb info
 * start[in]: pointer to start point of mmaped fb mem
 */

void draw_point(long x, long y, uint64_t pixel, struct fb_fix_screeninfo &finfo, struct fb_var_screeninfo &vinfo, uint8_t *start)
{
    // range check
    assert( (x < vinfo.xres) && (y < vinfo.yres) );

    long offset = (y*finfo.line_length) + (x*vinfo.bits_per_pixel/8);
    switch (vinfo.bits_per_pixel)
    {
        case 8:
            *((uint8_t*)(start+offset)) = pixel;
            break;
        case 16:  // high color
            *((uint16_t*)(start+offset)) = pixel;
            break;
        case 24: // true color
            *((uint16_t*)(start+offset)) = pixel;
            *((uint8_t*)(start+offset+2)) = pixel>>16;
            break;
        case 30: // deep color (10 bpc)
        case 32: // true color (plus transp)
            *((uint32_t*)(start+offset)) = pixel;
            break;
        case 36: // deep color (12 bpc)
            *((uint32_t*)(start+offset)) = pixel;
            *((uint8_t*)(start+offset+4)) = pixel>>32;
            break;
        case 48: // deep color (16 bpc)
            *((uint32_t*)(start+offset)) = pixel;
            *((uint16_t*)(start+offset+2)) = pixel>>32;
            break;
        default:
            cerr << "Unsupported bpp format!" <<endl;
            exit(1);
    }
}

/**
 * Construct true color pixel 
 */
inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo &vinfo)
{
	return (r<<vinfo.red.offset) | (g<<vinfo.green.offset) | (b<<vinfo.blue.offset);
}

int main()
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    int fb_fd;

    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd == -1)
    {
        cerr << "Open /dev/fb0: " << strerror(errno) << endl;
        return 1;
    }

    // var info
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.grayscale = 0;
    //vinfo.bits_per_pixel = 16;
    vinfo.nonstd = V4L2_PIX_FMT_YUYV;
    //vinfo.nonstd = V4L2_PIX_FMT_RGB32; 

    vinfo.xres = vinfo.xres_virtual = 640;
    vinfo.yres = vinfo.yres_virtual = 480;

    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    show_vinfo(&vinfo);

    // fix info
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    show_finfo(&finfo);

    // mmap fb
    long screensize = vinfo.yres_virtual * finfo.line_length;
    uint8_t *fbp = (uint8_t*)mmap(0, screensize, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);

    // draw something
    for (int x = 0; x < vinfo.xres; ++x)    
        for (int y = 0; y < vinfo.yres; ++y)
            draw_point(x, y, pixel_color(0x00, 0x00, 0xff, vinfo), finfo, vinfo, fbp);
    sleep(5);
}
