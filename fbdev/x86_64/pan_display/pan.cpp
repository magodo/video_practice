/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Sat 20 May 2017 12:39:59 AM CST
 Description: 
 ************************************************************************/

/*
 * Framebuffer test utility
 *
 * Configures a second buffer by doubling the virtual y-resolution. Flips
 * between the two using FBIOPAN_DISPLAY and optionally synchronize with
 * vertical blanking period by using FBIO_WAITFORVSYNC.
 *
 * This has been tested on Freescale Vybrid SoC using display controller
 * unit (DCU) fbdev driver.
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/*
 * Flip framebuffer, return the next buffer id which will be used
 */
int flip_buffer(int fb_fd, struct fb_var_screeninfo *vinfo, int vsync)
{
	static int id = 0;
	int dummy = 0;

	/* Pan the framebuffer */
	vinfo->yoffset = vinfo->yres * id;
	if (ioctl(fb_fd, FBIOPAN_DISPLAY, vinfo)) {
		perror("Error panning display");
		return -1;
	}

	id = !id;

	if (!vsync)
		return id;

	if (ioctl(fb_fd, FBIO_WAITFORVSYNC, &dummy)) {
		perror("Error waiting for VSYNC");
		return -1;
	}

	return id;
}

void write_buffer_32bit(char *fbp, long int size, uint32_t value)
{
	long int i;
	uint32_t *fbp32 = (uint32_t*) fbp;

	for (i = 0; i < size; i++)
		fbp32[i] = value;
}

int main(int argc, char *argv[])
{
	int fb_fd ;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize = 0;
	char *fbp;
	uint16_t color;
	int vsync = 1;

	if (argc < 2) {
		printf("Usage: %s fb-device [vsync-off]\n", argv[0]);
		exit(1);
	}

	if (argc > 2 && !strcmp(argv[2], "vsync-off"))
		vsync = 0;

	fb_fd = open(argv[1] , O_RDWR);
	if (fb_fd < 0) {
		perror("Cannot Open framebuffer device:");
		exit(1);
	}

	printf("Framebuffer device opened\n");

	/* Get variable and fixed screen info */
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
		perror("Error reading variable screen info from fb:");
		exit(1);
	}

	/* Set virtual display size double the width for double buffering */
	vinfo.yoffset = 0;
    vinfo.xres = 720;
    vinfo.yres = 1280;
    vinfo.xres_virtual = vinfo.xres;
	vinfo.yres_virtual = vinfo.yres * 2;

	if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo)) {
		perror("Error setting variable screen info from fb");
		exit(1);
	}

	printf("Display info: %d X %d @ %d bpp\n", vinfo.xres, vinfo.yres,
			vinfo.bits_per_pixel);

	printf("Virtual Display info: %d X %d, %d %d\n",
			vinfo.xres_virtual, vinfo.yres_virtual,
			vinfo.xoffset, vinfo.yoffset);

	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo)) {
		perror("Error reading fixed screen info from fb:");
		exit(1);
	}

	/* Map the whole frame buffer.. */
	fbp = (char *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED,
		       	   fb_fd, 0);
	if (fbp < 0) {
		perror("Failed to mmap:");
		exit(1);
	}

	screensize = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel >> 3);
	printf("screensize = %ld\n", screensize);

	while (1) {
		int buf;
		char *nextfb;

		buf = flip_buffer(fb_fd, &vinfo, vsync);
		if (buf < 0)
			break;

		/* Switch color every second page flip */
		if (buf)
			color = color == 0x0000ff ? 0xff0000 : 0x0000ff;

		/* Update the first buffer, which is currently not in use. */
		nextfb = fbp + screensize * buf;
		write_buffer_32bit(nextfb, vinfo.xres * vinfo.yres, color);

		usleep(200000);
	}

	munmap(fbp, screensize);

	close(fb_fd);
	return 0 ;
}

