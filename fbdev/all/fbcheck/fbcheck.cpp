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

    ss << "type: ";
    switch (finfo->type)
    {
        case FB_TYPE_PACKED_PIXELS:
            ss << "FB_TYPE_PACKED_PIXELS" << endl;
            break;
        case FB_TYPE_PLANES:
            ss << "FB_TYPE_PLANES" << endl;
            break;
        case FB_TYPE_INTERLEAVED_PLANES:
            ss << "FB_TYPE_INTERLEAVED_PLANES" << endl;
            break;
        case FB_TYPE_TEXT:
            ss << "FB_TYPE_TEXT" << endl;
            break;
        case FB_TYPE_VGA_PLANES:
            ss << "FB_TYPE_VGA_PLANES" << endl;
            break;
        case FB_TYPE_FOURCC:
            ss << "FB_TYPE_FOURCC" << endl;
            break;
        default:
            ss << "Unknown" << endl;
            break;
    }

    ss << "type aux: " << finfo->type_aux << endl;

    ss << "visual: ";
    switch (finfo->visual)
    {
        case FB_VISUAL_MONO01:
            ss << "FB_VISUAL_MONO01" << endl;
            break;
        case FB_VISUAL_MONO10:
            ss << "FB_VISUAL_MONO10" << endl;
            break;
        case FB_VISUAL_TRUECOLOR:
            ss << "FB_VISUAL_TRUECOLOR" << endl;
            break;
        case FB_VISUAL_PSEUDOCOLOR:
            ss << "FB_VISUAL_PSEUDOCOLOR" << endl;
            break;
        case FB_VISUAL_DIRECTCOLOR:
            ss << "FB_VISUAL_DIRECTCOLOR" << endl;
            break;
        case FB_VISUAL_STATIC_PSEUDOCOLOR:
            ss << "FB_VISUAL_STATIC_PSEUDOCOLOR" << endl;
            break;
        case FB_VISUAL_FOURCC:
            ss << "FB_VISUAL_FOURCC" << endl;
            break;
        default:
            ss << "Unknown" << endl;
            break;
    }

    ss << "xpanstep: " << finfo->xpanstep << endl;
    ss << "ypanstep: " << finfo->ypanstep << endl;
    ss << "ywrapstep: " << finfo->ywrapstep << endl;
    ss << "linelength: " << finfo->line_length << endl;
    ss << "mmio_start: " << hex << "0x" << finfo->mmio_start << dec << endl;
    ss << "mmio_len: " << finfo->mmio_len << endl;
    ss << "accel: " << finfo->accel << endl;

    ss << "capabilities: ";
    switch (finfo->capabilities)
    {
        case FB_CAP_FOURCC:
            ss << "FB_CAP_FOURCC" << endl;
            break;
        default:
            ss << "Unknown" << endl;
            break;
    }
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
    if (vinfo->grayscale == 1)
        ss << "grayscale";
    else if (vinfo->grayscale == 0)
        ss << "color";
    else if (vinfo->grayscale > 1)
        ss << "FOURCC";
    else
        ss << "Unknown";
    ss << endl;

    ss << "rgba [offset/length/msb_right]: " << vinfo->red.offset << "/" << vinfo->red.length << "/" << vinfo->red.msb_right << "," \
                   << vinfo->green.offset << "/" << vinfo->green.length << "/" << vinfo->green.msb_right << "," \
                   << vinfo->blue.offset << "/" << vinfo->blue.length << "/" << vinfo->blue.msb_right << "," \
                   << vinfo->transp.offset << "/" << vinfo->transp.length << "/" << vinfo->transp.msb_right << endl;

    char tmp[5] = {0};
    for (int i = 0; i < 4; ++i)
        tmp[i] = *((uint8_t*)(&vinfo->nonstd)+i);
    ss << "standard pixel format: " << ((vinfo->nonstd)? "No":"Yes") << "(" << tmp << ")" << endl;
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


int main(int argc, char *argv[])
{
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    int fb_fd;

    if (argc != 2)
    {
        cerr << "Usage: ./fbcheck [full_path_to_fb]" << endl;
        return 1;
    }

    fb_fd = open(argv[1], O_RDWR);
    if (fb_fd == -1)
    {
        cerr << "Open " << argv[1] << "failed: " << strerror(errno) << endl;
        return 1;
    }

    // var info
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    show_vinfo(&vinfo);
    show_finfo(&finfo);
}
