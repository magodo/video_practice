/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Fri 09 Jun 2017 12:59:39 PM CST
 Description: 
 ************************************************************************/

#include <iostream>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>

/**
 * Helper function for dumping video device capability infos
 */

void DumpCapbility_(uint32_t cap)
{
    std::cout << std::boolalpha 
              << "\n\t  * capture: " << bool(cap & V4L2_CAP_VIDEO_CAPTURE )
              << "\n\t  * capture multi-planer: " << bool(cap & V4L2_CAP_VIDEO_CAPTURE_MPLANE )
              << "\n\t  * output: " << bool(cap & V4L2_CAP_VIDEO_OUTPUT )
              << "\n\t  * output multi-planer: " << bool(cap & V4L2_CAP_VIDEO_OUTPUT_MPLANE )
              << "\n\t  * mem-2-mem: " << bool(cap & V4L2_CAP_VIDEO_M2M )
              << "\n\t  * mem-2-mem multi-planer: " << bool(cap & V4L2_CAP_VIDEO_M2M_MPLANE )
              << "\n\t  * overlay: " << bool(cap & V4L2_CAP_VIDEO_OVERLAY)
              << "\n\t  * VBI capture: " << bool(cap & V4L2_CAP_VBI_CAPTURE)
              << "\n\t  * VBI output: " << bool(cap & V4L2_CAP_VBI_OUTPUT)
              << "\n\t  * slice VBI capture: " << bool(cap & V4L2_CAP_SLICED_VBI_CAPTURE)
              << "\n\t  * slice VBI output: " << bool(cap & V4L2_CAP_SLICED_VBI_OUTPUT)
              << "\n\t  * RDS capture:" << bool(cap & V4L2_CAP_RDS_CAPTURE )
              << "\n\t  * output overlay: " << bool(cap &  V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
              << "\n\t  * hardware freq seek: " << bool(cap & V4L2_CAP_HW_FREQ_SEEK)
              << "\n\t  * RDS output: " << bool(cap & V4L2_CAP_RDS_OUTPUT)
              << "\n\t  * tunner (receive video signal): " << bool(cap & V4L2_CAP_TUNER)
              << "\n\t  * audio: " << bool(cap & V4L2_CAP_AUDIO)
              << "\n\t  * radio receiver: " << bool(cap & V4L2_CAP_RADIO)
              << "\n\t  * modulator (emit video/audio signal): " << bool(cap & V4L2_CAP_MODULATOR)
#ifdef  V4L2_CAP_SDR_CAPTURE
              << "\n\t  * SDR capture: " << bool(cap & V4L2_CAP_SDR_CAPTURE)
#endif
#ifdef  V4L2_CAP_EXT_PIX_FORMAT
              << "\n\t  * pixel format fields: " << bool(cap & V4L2_CAP_EXT_PIX_FORMAT)
#endif
#ifdef  V4L2_CAP_SDR_OUTPUT
              << "\n\t  * SDR output: " << bool(cap & V4L2_CAP_SDR_OUTPUT)
#endif
#ifdef V4L2_CAP_META_CAPTURE
              << "\n\t  * metadata capture: " << bool(cap & V4L2_CAP_META_CAPTURE)
#endif
              << "\n\t  * read/write interface: " << bool(cap & V4L2_CAP_READWRITE)
              << "\n\t  * async I/O: " << bool(cap & V4L2_CAP_ASYNCIO)
              << "\n\t  * streaming I/O: " << bool(cap & V4L2_CAP_STREAMING)
#ifdef V4L2_CAP_TOUCH
              << "\n\t  * touch device: " << bool(cap & V4L2_CAP_TOUCH)
#endif
              ;
}

/**
 * Dump every field from "VIDIOC_QUERYCAP"
 */

void DumpCapbility(int fd)
{
    struct v4l2_capability cap;

    memset(&cap, 0, sizeof(cap));

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        std::cerr << "ioctl - VIDIOC_QUERYCAP failed: " << strerror(errno) << std::endl;
    }

    std::cout << std::boolalpha 
              << "Device capability:\n\tDriver: " << cap.driver 
              << "\n\tCard: " << cap.card 
              << "\n\tBusInfo: " << cap.bus_info 
              << "\n\tVersion: " << ((cap.version >> 16) & 0xff) 
              << "." << ((cap.version >> 8) & 0xff) 
              << "." << ((cap.version) & 0xff ) 
              << "\n\tCapability(all func): ";
    DumpCapbility_(cap.capabilities);
    if (cap.capabilities & V4L2_CAP_DEVICE_CAPS)
    {
        std::cout << "\n\tCapability(current node): ";
        DumpCapbility_(cap.device_caps);
    }
    std::cout << std::endl;
}

/**
 * Dump every field from "VIDIOC_ENUMINPUT"
 */
void DumpInputInfo(int fd)
{
    struct v4l2_input input;
    std::string type("Unknwon");

    memset(&input, 0, sizeof(input));
    if (ioctl(fd, VIDIOC_G_INPUT, &input.index) == -1)
    {
        perror(" VIDIOC_G_INPUT");
        return;
    }

    if (ioctl(fd, VIDIOC_ENUMINPUT, &input) == -1)
    {
        perror("VIDIOC_ENUMINPUT");
        return;
    }

    switch (input.type)
    {
        case V4L2_INPUT_TYPE_TUNER:
            type = "tunner (RF demodulator)";
            break;
        case V4L2_INPUT_TYPE_CAMERA:
            type = "non-tuner video input";
            break;
#ifdef  V4L2_INPUT_TYPE_TOUCH
        case V4L2_INPUT_TYPE_TOUCH:
            type = "touch device for capturing raw data";
            break;
#endif
    }

    std::cout << "Video Input:" 
              << "\n\tname: " << input.name
              << "\n\ttype: " << type
              << "\n\taudio set: " << std::hex << input.audioset << std::dec;

    if (input.type == V4L2_INPUT_TYPE_TUNER)
        std::cout << "\n\ttuner index: " << input.tuner;
    else
        std::cout << "\n\ttuner index: N/A";

    std::cout << "\n\tsupporeted standard id set: " << std::hex << input.std << std::dec
              << "\n\tstatus (only make sense if current device is selected): "
              << std::boolalpha
              << "\n\t * No power: " << (bool)(input.status & V4L2_IN_ST_NO_POWER)
              << "\n\t * No signal: " << (bool)(input.status & V4L2_IN_ST_NO_SIGNAL)
              << "\n\t * No color in signal: " << (bool)(input.status & V4L2_IN_ST_NO_COLOR)
              << "\n\t * (Sensor) horizontal flip: " << (bool)(input.status & V4L2_IN_ST_HFLIP)
              << "\n\t * (Sensor) vertical flip: " << (bool)(input.status & V4L2_IN_ST_VFLIP)
              << "\n\t * (Anolog Video) no hsync: " << (bool)(input.status & V4L2_IN_ST_NO_H_LOCK)
              << "\n\t * (Anolog Video) color kill: " << (bool)(input.status & V4L2_IN_ST_COLOR_KILL)
#ifdef  V4L2_IN_ST_NO_V_LOCK
              << "\n\t * (Anolog Video) no vsync: " << (bool)(input.status & V4L2_IN_ST_NO_V_LOCK)
#endif
#ifdef  V4L2_IN_ST_NO_STD_LOCK
              << "\n\t * (Anolog Video) no standard lock: " << (bool)(input.status & V4L2_IN_ST_NO_STD_LOCK)
#endif
              << "\n\t * (Digital Video) no sync lock: " << (bool)(input.status & V4L2_IN_ST_NO_SYNC)
              << "\n\t * (Digital Video) no EQ lock: " << (bool)(input.status & V4L2_IN_ST_NO_EQU)
              << "\n\t * (Digital Video) carrier recover failed: " << (bool)(input.status & V4L2_IN_ST_NO_CARRIER)
              << "\n\t * (VCR & Set-Top Box) macrovision detected: " << (bool)(input.status & V4L2_IN_ST_MACROVISION)
              << "\n\t * (VCR & Set-Top Box) conditional access denied: " << (bool)(input.status & V4L2_IN_ST_NO_ACCESS)
              << "\n\t * (VCR & Set-Top Box) VTR time is constant: " << (bool)(input.status & V4L2_IN_ST_VTR)

              << "\n\tcapabilities: "
              << "\n\t * video timings: " << (bool)(input.capabilities & V4L2_IN_CAP_DV_TIMINGS)
              << "\n\t * set video standard: " << (bool)(input.capabilities & V4L2_IN_CAP_STD) << "(not always means can't use std-related API if it is false)"
#ifdef  V4L2_IN_CAP_NATIVE_SIZE
              << "\n\t * set native size: " << (bool)(input.capabilities & V4L2_IN_CAP_NATIVE_SIZE)
#endif

              << std::endl;
}

/**
 * Dump current standard
 */

void DumpCurrentStandard(int fd)
{
    /* Get supported standard id (one or a set of standards) */

    v4l2_std_id std_id;

    if (-1 == ioctl(fd, VIDIOC_G_STD, &std_id))
    {
        perror("VIDIOC_G_STD");
        return;
    }

    /* Enumarate all video standards, check which corresponds to our
     * standard id. Get the corresponding detail information. */

    struct v4l2_standard standard;
    int index;

    memset(&standard, 0, sizeof(standard));
    index = 0;
    standard.index = index;

    std::cout << "Current Standard(s):";
    while (0 == ioctl(fd, VIDIOC_ENUMSTD, &standard))
    {
        if (standard.id & std_id)
        {
            std::cout << "\n\t* name: " << standard.name
                      << "\n\t* frame period: "
                      << (static_cast<double>(standard.frameperiod.numerator)/standard.frameperiod.denominator)
                      << "\n\t* frame lines: " << standard.framelines
                      << std::endl;
        }
        index++;
        standard.index = index;
        std::cout << "index: " << standard.index << std::endl;
    }

    if ((errno == EINVAL) || (errno == ENOTTY) || (standard.index == 0))
    {
        perror("VIDIOC_ENUMSTD");
        return;
    }

    return;
}


int main()
{
    int fd;

    if ((fd = open("/dev/video0", O_RDONLY)) == -1)
    {
        perror("open");
        return -1;
    }

    DumpCapbility(fd);
    DumpInputInfo(fd);
    DumpCurrentStandard(fd);
    //DumpSupportedStandard(fd);

    close(fd);
}
