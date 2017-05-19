/*
* Copyright 2013 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* The code contained herein is licensed under the GNU Lesser General
* Public License. You may obtain a copy of the GNU Lesser General
* Public License Version 2.1 or later at the following locations:
*
* http://www.opensource.org/licenses/lgpl-license.html
* http://www.gnu.org/copyleft/lgpl.html
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/mxcfb.h>
#include <linux/ipu.h>

//#define GLOBAL_ALPHA
//#define COLOR_KEY
#define SEPERATE_ALP_BUF

int main (int argc, char *argv[])
{
	struct fb_var_screeninfo fb0_var;
	struct fb_fix_screeninfo fb0_fix;
	struct fb_var_screeninfo fb1_var;
	struct fb_fix_screeninfo fb1_fix;
    char *ov_buf, *p_buf, *cur_buf;
	int fd_fb0, fd_fb1, ovsize, isize, screen_size = 0;
    int blank, i, color_count = 0;
#ifdef GLOBAL_ALPHA
    struct mxcfb_gbl_alpha g_alpha;
    struct mxcfb_color_key ckey;
#else
    struct mxcfb_loc_alpha l_alpha;
#ifdef SEPERATE_ALP_BUF
    unsigned long loc_alpha_phy_addr0;
    unsigned long loc_alpha_phy_addr1;
    unsigned long alpha_buf_size;
    char *alpha_buf0, *alpha_buf1;
    char local_alpha;
#endif
#endif

    /* Configure the size of overlay buffer */
    unsigned int g_display_width = 1920;
    unsigned int g_display_height = 1080;

	// Open Framebuffer and gets its address
	if ((fd_fb0 = open("/dev/fb0", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb0\n");
		goto done;
	}

	if ( ioctl(fd_fb0, FBIOGET_FSCREENINFO, &fb0_fix) < 0) {
		printf("Get FB fix info failed!\n");
        close(fd_fb0);
		goto done;
	}

	if ( ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb0_var) < 0) {
		printf("Get FB var info failed!\n");
        close(fd_fb0);
		goto done;
	}

    printf("\nFB0 information \n");
    printf("fb0->xres = %d\n",  fb0_var.xres);
    printf("fb0->xres_virtual = %d\n",  fb0_var.xres_virtual);
    printf("fb0->yres = %d\n",  fb0_var.yres);
    printf("fb0->yres_virtual = %d\n",  fb0_var.yres_virtual);
    printf("fb0->bits_per_pixel = %d\n",  fb0_var.bits_per_pixel);
    printf("fb0->pixclock = %d\n",  fb0_var.pixclock);
    printf("fb0->height = %d\n",  fb0_var.height);
    printf("fb0->width = %d\n",  fb0_var.width);
    printf(" Pixel format : RGBX_%d%d%d%d\n",fb0_var.red.length,
                                                 fb0_var.green.length,
                                                 fb0_var.blue.length,
                                                 fb0_var.transp.length);
    printf(" Begin of bitfields(Byte ordering):-\n");
    printf("  Red    : %d\n",fb0_var.red.offset);
    printf("  Blue   : %d\n",fb0_var.blue.offset);
    printf("  Green  : %d\n",fb0_var.green.offset);
    printf("  Transp : %d\n",fb0_var.transp.offset);

    /* 
     * Update all the buffers (double/triple) of /dev/fb0 for testing
     */
    isize = fb0_var.xres * fb0_var.yres_virtual * fb0_var.bits_per_pixel/8;
    printf("\nBackground Screen size is %d \n", isize);

    /* Map the device to memory */
    p_buf = (char *)mmap(0, isize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb0, 0);
    if ((int) p_buf == -1) {
        printf("Error: failed to map framebuffer device to memory.\n");
        close(fd_fb0);
		goto done;
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

    /* Set background buffer with white color */
    memset(p_buf, 0xFF, isize);
    
    sleep(1);

	// Open overlay Framebuffer and gets its address
	if ((fd_fb1 = open("/dev/fb1", O_RDWR, 0)) < 0) {
		printf("Unable to open /dev/fb1\n");
        close(fd_fb0);
		goto done;
	}

	if (ioctl(fd_fb1, FBIOGET_FSCREENINFO, &fb1_fix) < 0) {
		printf("Get FB fix info failed!\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;
	}

    if ( ioctl(fd_fb1, FBIOGET_VSCREENINFO, &fb1_var) < 0) {
		printf("Get FB1 var info failed!\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;
	}

    printf("\nFB1 information (Before change) \n");
    printf("fb1->xres = %d\n",  fb1_var.xres);
    printf("fb1->xres_virtual = %d\n",  fb1_var.xres_virtual);
    printf("fb1->xyres = %d\n",  fb1_var.yres);
    printf("fb1->yres_virtual = %d\n",  fb1_var.yres_virtual);
    printf("fb1->bits_per_pixel = %d\n",  fb1_var.bits_per_pixel);
    printf("fb1->pixclock = %d\n",  fb1_var.pixclock);
    printf("fb1->height = %d\n",  fb1_var.height);
    printf("fb1->width = %d\n",  fb1_var.width);
    printf(" Pixel format : RGBX_%d%d%d%d\n",fb1_var.red.length,
                                                 fb1_var.green.length,
                                                 fb1_var.blue.length,
                                                 fb1_var.transp.length);
    printf(" Begin of bitfields(Byte ordering):-\n");
    printf("  Red    : %d\n",fb1_var.red.offset);
    printf("  Blue   : %d\n",fb1_var.blue.offset);
    printf("  Green  : %d\n",fb1_var.green.offset);
    printf("  Transp : %d\n",fb1_var.transp.offset);


    /* Change overlay/foreground buffer's settings */
    fb1_var.xres = g_display_width;
    fb1_var.yres = g_display_height; 
    fb1_var.xres_virtual = g_display_width;
    /* Triple buffers enabled */
    fb1_var.yres_virtual = g_display_height * 3;  
    fb1_var.activate |= (FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE);
#ifdef SEPERATE_ALP_BUF
    fb1_var.bits_per_pixel  = 24;
    fb1_var.red.length = 8;
    fb1_var.blue.length = 8;
    fb1_var.green.length = 8;
    fb1_var.transp.length = 0;
    fb1_var.red.offset = 16;
    fb1_var.blue.offset = 0;
    fb1_var.green.offset = 8;
    fb1_var.transp.offset = 0;
#else
    fb1_var.bits_per_pixel  = 32;
    fb1_var.red.length = 8;
    fb1_var.blue.length = 8;
    fb1_var.green.length = 8;
    fb1_var.transp.length = 8;
    fb1_var.red.offset = 16;
    fb1_var.blue.offset = 0;
    fb1_var.green.offset = 8;
    fb1_var.transp.offset = 24;
#endif
    if (ioctl(fd_fb1, FBIOPUT_VSCREENINFO,
                &fb1_var) < 0) {
        printf("Put var of fb1 failed\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;
    }

    /* Unblank the fb1 */
    blank = FB_BLANK_UNBLANK;
    if (ioctl(fd_fb1, FBIOBLANK, blank) < 0) {
        printf("Blanking fb1 failed\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;

    }

    if ( ioctl(fd_fb1, FBIOGET_VSCREENINFO, &fb1_var) < 0) {
		printf("Get FB1 var info failed!\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;
	}

    printf("\nFB1 information (After change) \n");
    printf("fb1->xres = %d\n",  fb1_var.xres);
    printf("fb1->xres_virtual = %d\n",  fb1_var.xres_virtual);
    printf("fb1->xyres = %d\n",  fb1_var.yres);
    printf("fb1->yres_virtual = %d\n",  fb1_var.yres_virtual);
    printf("fb1->bits_per_pixel = %d\n",  fb1_var.bits_per_pixel);
    printf("fb1->pixclock = %d\n",  fb1_var.pixclock);
    printf("fb1->height = %d\n",  fb1_var.height);
    printf("fb1->width = %d\n",  fb1_var.width);
    printf(" Pixel format : RGBX_%d%d%d%d\n",fb1_var.red.length,
                                                 fb1_var.green.length,
                                                 fb1_var.blue.length,
                                                 fb1_var.transp.length);
    printf(" Begin of bitfields(Byte ordering):-\n");
    printf("  Red    : %d\n",fb1_var.red.offset);
    printf("  Blue   : %d\n",fb1_var.blue.offset);
    printf("  Green  : %d\n",fb1_var.green.offset);
    printf("  Transp : %d\n",fb1_var.transp.offset);



    ovsize = fb1_var.xres * fb1_var.yres_virtual * fb1_var.bits_per_pixel/8;
    screen_size = fb1_var.xres * fb1_var.yres * fb1_var.bits_per_pixel/8;
    printf("\nOverlay Screen size is %d \n", ovsize);

    /* Map the device to memory */
    ov_buf = (char *)mmap(0, ovsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb1, 0);
    if ((int) ov_buf == -1) {
        printf("Error: failed to map framebuffer device to memory.\n");
        close(fd_fb0);
        close(fd_fb1);
		goto done;
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

#ifdef GLOBAL_ALPHA
    if (argc < 2)
    {
        printf("Give global alpha value as argument [0 - 255]\n");
        exit(-1);
    }
    /* Enable global alpha */
    g_alpha.alpha = atoi(argv[1]);
    g_alpha.enable = 1;
    if (ioctl(fd_fb0, MXCFB_SET_GBL_ALPHA, &g_alpha) < 0) {
        printf("Set global alpha failed\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }

#ifdef COLOR_KEY
    /* Enable color key  */
    ckey.enable = 1;
    ckey.color_key = 0x0;
    if (ioctl(fd_fb0,MXCFB_SET_CLR_KEY,&ckey) < 0) {
        printf( "MXCFB_SET_CLR_KEY failed ");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }
#endif

#else /* Local Alpha methods */
#ifdef SEPERATE_ALP_BUF
    /*
     * For local alpha blending (separate buffer) to work, the pixel format 
     * should be 24 bits per pixel because we are not using alpha value that comes 
     * along for every pixel in the case of 32-bit bpp
     */
    l_alpha.enable = 1;
    l_alpha.alpha_in_pixel = 0;
    l_alpha.alpha_phy_addr0 = 0;
    l_alpha.alpha_phy_addr1 = 0;
    if (ioctl(fd_fb1, MXCFB_SET_LOC_ALPHA,
                &l_alpha) < 0) {
        printf("Set local alpha failed\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }
    loc_alpha_phy_addr0 =
        (unsigned long)(l_alpha.alpha_phy_addr0);
    loc_alpha_phy_addr1 =
        (unsigned long)(l_alpha.alpha_phy_addr1);

    alpha_buf_size = fb1_var.xres * fb1_var.yres;

    /* Memory map of first alpha buffer */
    alpha_buf0 = (char *)mmap(0, alpha_buf_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd_fb1,
            loc_alpha_phy_addr0);
    if ((int)alpha_buf0 == -1) {
        printf("\nError: failed to map alpha buffer 0"
                " to memory.\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }

    /* Memory map of second alpha buffer */
    alpha_buf1 = (char *)mmap(0, alpha_buf_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd_fb1,
            loc_alpha_phy_addr1);
    if ((int)alpha_buf1 == -1) {
        printf("\nError: failed to map alpha buffer 1"
                " to memory.\n");
        munmap((void *)alpha_buf0, alpha_buf_size);
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }

    /* Create both alpha buffers */
    memset(alpha_buf1, 255 , alpha_buf_size/4);
    memset((alpha_buf1 + alpha_buf_size/4), 127, alpha_buf_size/4);
    memset((alpha_buf1 + alpha_buf_size/2), 127, alpha_buf_size/4);
    memset((alpha_buf1 + alpha_buf_size*3/4), 0, alpha_buf_size/4);

    if (ioctl(fd_fb1, MXCFB_SET_LOC_ALP_BUF, &loc_alpha_phy_addr1) < 0) {
        printf("Set local alpha buf failed\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }

    memset(alpha_buf0, 0, alpha_buf_size/4);
    memset((alpha_buf0 + alpha_buf_size/4), 64, alpha_buf_size/4);
    memset((alpha_buf0 + alpha_buf_size/2), 127, alpha_buf_size/4);
    memset((alpha_buf0 + alpha_buf_size*3/4), 255, alpha_buf_size/4);

    if (ioctl(fd_fb1, MXCFB_SET_LOC_ALP_BUF, &loc_alpha_phy_addr0) < 0) {
        printf("Set local alpha buf failed\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }

#else /* Alpha in Pixel */
    /* 
     * Alpha value is part of each pixel data. Use 32-bit bpp 
     */
    l_alpha.enable = 1;
    l_alpha.alpha_in_pixel = 1;
    l_alpha.alpha_phy_addr0 = 0;
    l_alpha.alpha_phy_addr1 = 0;
    if (ioctl(fd_fb1, MXCFB_SET_LOC_ALPHA,
                &l_alpha) < 0) {
        printf("Set local alpha failed\n");
        close(fd_fb0);
        close(fd_fb1);
        goto done;
    }
#endif
#endif
    while(1) 
    {
#ifdef SEPERATE_ALP_BUF
        /* Set overlay buffer with white color */
        if ((color_count % 3) == 0)
        {
            cur_buf = ov_buf;
            fb1_var.yoffset = 0;
            for (i = 0; i <= screen_size - 3; i += 3)
            {
                /* Blue color */
                cur_buf[i] =  0xff;  /* Blue */
                cur_buf[i+1] = 0;  /* Green */
                cur_buf[i+2] = 0;  /* Red */
            }
        }
        else if ((color_count % 3) == 1)
        {
            cur_buf = ov_buf + screen_size;
            fb1_var.yoffset = fb1_var.yres;
            for (i = 0; i <= screen_size - 3; i += 3)
            {
                /* Green color */
                cur_buf[i] = 0;    /* Blue */
                cur_buf[i+1] = 0xff;  /* Green */
                cur_buf[i+2] = 0; /* Red */
            }
        }
        else if ((color_count % 3) == 2)
        {
            cur_buf = ov_buf + screen_size * 2;
            fb1_var.yoffset = fb1_var.yres * 2;
            for (i = 0; i <= screen_size - 3; i += 3)
            {
                /*  Red color */
                cur_buf[i] = 0;    /* Blue */
                cur_buf[i+1] = 0;  /* Green */
                cur_buf[i+2] = 0xff; /* Red */
            }
        }

        /* 
         * Switch framebuffers 0->1->2 (3 buffers) 
         * Alpha buffers are also alternated when pan display is
         * called.
         */
#if 0
        if ( ioctl(fd_fb1, FBIOPAN_DISPLAY, &fb1_var) < 0) {
            printf("Changing framebuffer failed!\n");
            close(fd_fb0);
            close(fd_fb1);
            goto done;
        }
#endif

#else
         if ((color_count % 3) == 0)
        {
            cur_buf = ov_buf;
            fb1_var.yoffset = 0;
            for (i = 0; i <= screen_size - 4; i += 4)
            {
                /* Blue color */
                cur_buf[i] =  250;  /* Blue */
                cur_buf[i+1] = 79;  /* Green */
                cur_buf[i+2] = 17;  /* Red */
                if ( i < screen_size/2)
                {
                    cur_buf[i+3] = 0;  /* Alpha - Transparent */
                }
                else
                {
                    cur_buf[i+3] = 255;  /* Alpha - Opaque */
                }
            }
        }
        else if ((color_count % 3) == 1)
        {
            cur_buf = ov_buf + screen_size;
            fb1_var.yoffset = fb1_var.yres;
            for (i = 0; i <= screen_size - 4; i += 4)
            {
                /* Orange color */
                cur_buf[i] = 17;    /* Blue */
                cur_buf[i+1] = 48;  /* Green */
                cur_buf[i+2] = 250; /* Red */
                if ( i < screen_size/2)
                {
                    cur_buf[i+3] = 127;  /* Alpha */
                }
                else
                {
                    cur_buf[i+3] = 255;  /* Alpha */
                }
            }
        }
        else if ((color_count % 3) == 2)
        {
            cur_buf = ov_buf + screen_size * 2;
            fb1_var.yoffset = fb1_var.yres * 2;
            for (i = 0; i <= screen_size - 4; i += 4)
            {
                /* Green color */
                cur_buf[i] = 17;    /* Blue */
                cur_buf[i+1] = 250;  /* Green */
                cur_buf[i+2] = 95; /* Red */
                if ( i < screen_size/2)
                {
                    cur_buf[i+3] = 127;  /* Alpha */
                }
                else
                {
                    cur_buf[i+3] = 64;  /* Alpha */
                }

            }
        }

        /* 
         * Switch framebuffers 0->1->2 (3 buffers) 
         * Alpha buffers are also alternated when pan display is
         * called.
         */
        if ( ioctl(fd_fb1, FBIOPAN_DISPLAY, &fb1_var) < 0) {
            printf("Changing framebuffer failed!\n");
            close(fd_fb0);
            close(fd_fb1);
            goto done;
        }
#endif
        color_count++;
        printf("Rendered frame:%d\n", color_count);
        usleep(5000000);
    } /* while (1) */

    close(fd_fb0);
    close(fd_fb1);
    return 0;

done:
    return -1;
}
