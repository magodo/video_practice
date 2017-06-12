/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 12 Jun 2017 05:45:31 PM CST
 Description: 
 ************************************************************************/
#include <stdio.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define  VS_VIDEO_CAMERA_V4L2 "/dev/video0"

static int vsCaptureSetupCapture(int capture_fd)
{
    struct v4l2_capability cap;
    struct v4l2_dbg_chip_ident chip;
    int input = 1;
    v4l2_std_id std = 0;
    struct v4l2_cropcap cropcap;
    struct v4l2_streamparm parm;
    //struct v4l2_format fmt;
    unsigned int min;
    struct v4l2_requestbuffers req;
    
    /* query device capabilities */
    if (ioctl(capture_fd, VIDIOC_QUERYCAP, &cap) < 0)
    {
        printf("[Capture] Unable to query capbility of the capture device\n");
        return -1;
    }
    printf("[Capture] capture device capability: dirver: %s, card: %s, bus_info: %s, version: %d, capabilities: 0x%08x, device_caps: 0x%08x\n", 
            cap.driver, cap.card, cap.bus_info, cap.version, cap.capabilities, cap.device_caps);
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        printf("[Capture] %s is not a video capture device\n", VS_VIDEO_CAMERA_V4L2);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("[Capture] %s does not support streaming\n", VS_VIDEO_CAMERA_V4L2);
        return -1;
    }
    
    /* check video input chip */
    if (ioctl(capture_fd, VIDIOC_DBG_G_CHIP_IDENT, &chip))
    {
        printf("[Capture] VIDIOC_DBG_G_CHIP_IDENT failed");
        return -1;
    }
    printf("[Capture] TV decoder chip is %s\n", chip.match.name);
    
    /* select input (default input is 1, RVC?) */
    if (ioctl(capture_fd, VIDIOC_S_INPUT, &input) < 0)
    {
        printf("[Capture] VIDIOC_S_INPUT failed");
        return -1;
    }
    printf("[Capture] video input: %d\n", input);
    
    /* Get std */
    if (ioctl(capture_fd, VIDIOC_G_STD, &std) < 0)
    {
        printf("[Capture] VIDIOC_G_STD failed");
        return -1;
    }
    printf("[Capture] input video standard: 0x%08x\n", std);
    /* Select video standard */
    if (ioctl(capture_fd, VIDIOC_S_STD, &std) < 0)
    {
        printf("[Capture] VIDIOC_S_STD failed");
        return -1;
    }
    
    /* Get the crop capability */
    (void)memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(capture_fd, VIDIOC_CROPCAP, &cropcap) < 0)
    {
        printf("[Capture] capture device does not support crop capability");
        return -1;
    }
    printf("[Capture] capture device: cropcap.bounds.width = %d cropcap.bound.height = %d cropcap.defrect.width = %d cropcap.defrect.height = %d\n",
              cropcap.bounds.width, cropcap.bounds.height, cropcap.defrect.width, cropcap.defrect.height);
    
    /* select fps */
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = 0;
    parm.parm.capture.capturemode = 0;
    if (ioctl(capture_fd, VIDIOC_S_PARM, &parm) < 0)
    {
        printf("[Capture] VIDIOC_S_PARM failed");
        return -1;
    }
    
    /* set pixel format */
    struct v4l2_format format;

    (void)memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 0;
    format.fmt.pix.height = 0;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    format.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(capture_fd, VIDIOC_S_FMT, &format) < 0)
    {
        printf("[Capture] capture device VIDIOC_S_FMT failed");
        return -1;
    }
    /* NOTE VIDIOC_S_FMT may change width and height */
    printf("[Capture] capture device: fmt.fmt.pix.width=%d, fmt.fmt.pix.height=%d, fmt.fmt.pix.bytesperline=%d, fmt.fmt.pix.sizeimage=%d\n", 
             format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.bytesperline, format.fmt.pix.sizeimage);
    min = format.fmt.pix.width * 2;
    if (format.fmt.pix.bytesperline < min)
    {
        format.fmt.pix.bytesperline = min;
    }
    min = format.fmt.pix.bytesperline * format.fmt.pix.height;
    if (format.fmt.pix.sizeimage < min)
    {
        format.fmt.pix.sizeimage = min;
    }
    if (ioctl(capture_fd, VIDIOC_G_FMT, &format) < 0)
    {
        printf("[Capture] capture device: VIDIOC_G_FMT failed");
        return -1;
    }
    
    /* allocate buffer in capture device */
    (void)memset(&req, 0, sizeof(req));
    req.count = 6;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(capture_fd, VIDIOC_REQBUFS, &req) < 0)
    {
        printf("[Capture] VIDIOC_REQBUFS failed");
        return -1;
    }
    return 0;
}

int main()
{
    int fd = open( VS_VIDEO_CAMERA_V4L2, O_RDWR, 0);
    vsCaptureSetupCapture(fd);
}

