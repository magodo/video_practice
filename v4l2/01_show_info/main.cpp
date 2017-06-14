/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Fri 09 Jun 2017 12:59:39 PM CST
 Description: 
 ************************************************************************/

#include <iostream>
#include <string>

#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Helper function for enumarating video device capability infos
 */

void EnumCapbility_(uint32_t cap)
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
 * Enumarate every field from "VIDIOC_QUERYCAP"
 */

void EnumCapbility(int fd)
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
    EnumCapbility_(cap.capabilities);
    if (cap.capabilities & V4L2_CAP_DEVICE_CAPS)
    {
        std::cout << "\n\tCapability(current node): ";
        EnumCapbility_(cap.device_caps);
    }
    std::cout << std::endl;
}

/**
 * Show every field from "VIDIOC_ENUMINPUT"
 */
void ShowInputInfo(int fd)
{
    struct v4l2_input input;
    std::string type("Unknwon");

    memset(&input, 0, sizeof(input));

    input.index = 0;

    while (ioctl(fd, VIDIOC_ENUMINPUT, &input) == 0)
    {
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

        std::cout << "Video Input (index: " << input.index << ")"
                  << "\n\tname: " << input.name
                  << "\n\ttype: " << type
                  << "\n\taudio set: " << std::hex << input.audioset << std::dec;

        if (input.type == V4L2_INPUT_TYPE_TUNER)
            std::cout << "\n\ttuner index: " << input.tuner;
        else
            std::cout << "\n\ttuner index: N/A";

        std::cout << "\n\tsupporeted standard id set: " << std::hex << input.std << std::dec << std::endl;

        /* Following status info only makes sense when current input is selected */
        if (ioctl(fd, VIDIOC_S_INPUT, &input.index) == -1)
        {
            perror("VIDIOC_S_INPUT");
            exit(-1);
        }

        std::cout << "\n\tstatus (only make sense if current input/output is selected): "
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

        ++input.index;
    }
    
    if (errno != EINVAL)
    {
        perror("VIDIOC_ENUMINPUT");
        exit(-1);
    }
}

/**
 * Show current standard
 */

void ShowCurrentStandard(int fd)
{
    /* Get currrent standard id */

    v4l2_std_id std_id;

    memset(&std_id, 0, sizeof(std_id));

    if (-1 == ioctl(fd, VIDIOC_G_STD, &std_id))
    {
        perror("VIDIOC_G_STD");
        exit(-1);
    }

    /* Enumarate all video standards, check which current
     * standard id. Get the corresponding detail information. */

    struct v4l2_standard standard;
    int index;

    memset(&standard, 0, sizeof(standard));
    index = 0;
    standard.index = index;

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
        exit(-1);
    }

    return;
}

/**
 * Enum supported standards
 */
void EnumSupportedStandard(int fd)
{
    struct v4l2_input input;
    struct v4l2_standard standard;

    if (0 != ioctl(fd, VIDIOC_G_INPUT, &input.index))
    {
        perror("VIDIOC_G_INPUT");
        exit(-1);
    }

    if (0 != ioctl(fd, VIDIOC_ENUMINPUT, &input))
    {
        perror("VIDIOC_ENUMINPUT");
        exit(-1);
    }

    memset(&standard, 0, sizeof(standard));
    standard.index = 0;

    std::cout << "All supported standards:" << std::endl;
    while (0 == ioctl(fd, VIDIOC_ENUMSTD, &standard))
    {
        if (standard.id & input.std)
            std::cout << "\t* " << standard.name << std::endl;
    }

    /* EINVAL indicates the end of enumaration, which cannot
     * be empty unless this device falls under USB exception*/
    if (errno != EINVAL || standard.index == 0)
    {
        perror("VIDIOC_ENUMSTD");
        exit(-1);
    }
}

/**
 * Enumerate User Controls
 */

#ifdef VIDIOC_QUERY_EXT_CTRL
void EnumMenu(int fd, uint32_t id, struct v4l2_query_ext_ctrl queryctrl, bool is_type_int)
#else
void EnumMenu(int fd, uint32_t id, struct v4l2_queryctrl queryctrl, bool is_type_int)
#endif
{
    struct v4l2_querymenu querymenu;

    std::cout << "\n\t\tMenu items (" << queryctrl.minimum << "-" << queryctrl.maximum
              << ", default: " << queryctrl.default_value << ")" << std::endl;

    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = id;

    for (querymenu.index = queryctrl.minimum;
         static_cast<int>(querymenu.index) <= queryctrl.maximum;
         querymenu.index++)
    {
        if (0 == ioctl(fd, VIDIOC_QUERYMENU, &querymenu))
        {
            if (is_type_int)
                std::cout << "\t\t" << querymenu.index<< ". " << querymenu.value << std::endl;
            else
                std::cout << "\t\t" << querymenu.index<< ". " << querymenu.name << std::endl;
        }
    }
}

#ifdef VIDIOC_QUERY_EXT_CTRL
void ShowOneControl(int fd, struct v4l2_query_ext_ctrl queryctrl)
#else
void ShowOneControl(int fd, struct v4l2_queryctrl queryctrl)
#endif
{
    std::cout << "\t" << queryctrl.name;
    switch (queryctrl.type)
    {
        /* following types support "default value" */
        case V4L2_CTRL_TYPE_INTEGER:
        case V4L2_CTRL_TYPE_BOOLEAN:
        case V4L2_CTRL_TYPE_BITMASK:
        case V4L2_CTRL_TYPE_INTEGER64:
#ifdef V4L2_CTRL_TYPE_U8
        case V4L2_CTRL_TYPE_U8:
        case V4L2_CTRL_TYPE_U16:
        case V4L2_CTRL_TYPE_U32:
#endif
        {
            struct v4l2_control control;

            memset(&control, 0, sizeof(control));
            control.id = queryctrl.id;
            if (-1 == ioctl(fd, VIDIOC_G_CTRL, &control))
            {
                perror("VIDIOC_G_CTRL");
                exit(-1);
            }
            std::cout << "(default: " << queryctrl.default_value
                      << "): " << control.value
                      << std::endl;
            break;
        }

        /* following 2 menu types also support "default value"*/
        case V4L2_CTRL_TYPE_MENU:
            EnumMenu(fd, queryctrl.id, queryctrl, false);
            break;
        case V4L2_CTRL_TYPE_INTEGER_MENU:
            EnumMenu(fd, queryctrl.id, queryctrl,true);
            break;

        /* Only show control name... */
        case V4L2_CTRL_TYPE_BUTTON:
        case V4L2_CTRL_TYPE_STRING:
        case V4L2_CTRL_TYPE_CTRL_CLASS:
            break;
    }
}


void EnumControls_Old(int fd)
{
#ifdef VIDIOC_QUERY_EXT_CTRL
    struct v4l2_query_ext_ctrl queryctrl;
#else
    struct v4l2_queryctrl queryctrl;
#endif

    memset(&queryctrl, 0, sizeof(queryctrl));

    std::cout << "Control " << std::endl;

    /* Check pre-defined controls */

    for (queryctrl.id = V4L2_CID_BASE;
         queryctrl.id < V4L2_CID_LASTP1;
         queryctrl.id++)
    {

#ifdef  VIDIOC_QUERY_EXT_CTRL
        if (0 == ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &queryctrl))
#else
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
#endif
        {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            {
                continue;
            }

            /* Show one control info */
            ShowOneControl(fd, queryctrl);
        }
        else
        {
            if (errno == EINVAL)
            {
                /* driver not support this control */
                continue;
            }
            perror("VIDIOC_QUERYCTRL");
            exit(-1);
        }
    }

    /* Check driver-specific controls */

    for (queryctrl.id= V4L2_CID_PRIVATE_BASE;;
         queryctrl.id++)
    {
#ifdef   VIDIOC_QUERY_EXT_CTRL
        if (0 == ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &queryctrl))
#else
        if (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
#endif
        {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
            {
                continue;
            }

            /* Show one control info */
            ShowOneControl(fd, queryctrl);
        }
        else
        {
            if (errno == EINVAL)
                /* out of range */
                break;

            perror("VIDIOC_QUERYCTRL");
            exit(-1);
        }
    }
}

void EnumControls(int fd)
{
#ifdef VIDIOC_QUERY_EXT_CTRL
    struct v4l2_query_ext_ctrl queryctrl;
#else
    struct v4l2_queryctrl queryctrl;
#endif

    memset(&queryctrl, 0, sizeof(queryctrl));
    /* We do not enumerate id starting from zero is because,
     * >>> Drivers may return EINVAL if a control in this range
     * is not supported. 
     *
     * And
     *
     * >>> When the application ORs id with
     * V4L2_CTRL_FLAG_NEXT_CTRL the driver returns the next supported
     * non-compound control, or EINVAL if there is none.
     *
     * So we can always return 0 from ioctl if id is ORed with 
     * V4L2_CTRL_FLAG_NEXT_CTRL since it is guaranteed to be supported.
     * Unless out of range.
     *
     * NOTE: here we only check the non-compound control */
    queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    std::cout << "Control " << std::endl;
#ifdef VIDIOC_QUERY_EXT_CTRL
    while (0 == ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &queryctrl))
#else
    while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl))
#endif
    {
        /* V4L2_CTRL_FLAG_DISABLED flag means this control should be
         * permanently ignored. */
        if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
        {
            /* Show one control info */
            ShowOneControl(fd, queryctrl);

        }

        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    if (errno != EINVAL)
    {
        perror("VIDIOC_QUERY_EXT_CTRL");
        exit(-1);
    }
}

/**
 * Enumarate image formats(for capture only) supported by current device.
 */

void EnumCaptureImageFormat(int fd)
{
    struct v4l2_fmtdesc fmt_desc;

    memset(&fmt_desc, 0, sizeof(fmt_desc));
    
    fmt_desc.index = 0;
    fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    std::cout << "Supported image formats(Capture):" << std::endl;
    while (0 == ioctl(fd, VIDIOC_ENUM_FMT, &fmt_desc))
    {
        std::cout << "\t" << fmt_desc.description << "("
                  << ((fmt_desc.flags & V4L2_FMT_FLAG_COMPRESSED)? "compressed":"non-compressed")
                  << ")" << std::endl;
        fmt_desc.index++;
    }

    if (errno != EINVAL)
    {
        perror("VIDIOC_ENUM_FMT");
        exit(-1);
    }
    
}

std::string Fourcc2String(uint32_t pix_fmt)
{
    std::string str;
    str.insert(0, 1, (pix_fmt >> 24) & 0xff);
    str.insert(0, 1, (pix_fmt >> 16) & 0xff);
    str.insert(0, 1, (pix_fmt >> 8) & 0xff);
    str.insert(0, 1, pix_fmt & 0xff);
    return str;
}

void ShowCaptureCurrentFormat(int fd)
{
    struct v4l2_format format;

    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == ioctl(fd, VIDIOC_G_FMT, &format))
    {
        std::cout << "Current image format(Capture):"
                  << "\n\twidth: " << format.fmt.pix.width
                  << "\n\theight: " << format.fmt.pix.height
                  << "\n\tpixel format: " << Fourcc2String(format.fmt.pix.pixelformat)
                  << "\n\tfield: " << format.fmt.pix.field
                  << "\n\tbytesperline: " << format.fmt.pix.bytesperline
                  << "\n\tsize image: " << format.fmt.pix.sizeimage
                  << "\n\tcolorspace: " << format.fmt.pix.colorspace << std::endl;

#ifdef V4L2_PIX_FMT_PRIV_MAGIC
        struct v4l2_capability cap;
        memset(&cap, 0, sizeof(cap));

        if (0 == ioctl(fd, VIDIOC_QUERYCAP, &cap))
        {
            if (cap.capabilities & V4L2_CAP_EXT_PIX_FORMAT)
            {
                if (format.fmt.pix.priv == V4L2_PIX_FMT_PRIV_MAGIC)
                {
                    std::cout << "\tpixel flag: 0x" << std::hex << format.fmt.pix.flags << std::dec;
                    std::cout << "\n\tY'CbCr encoding: " << format.fmt.pix.ycbcr_enc;
                    std::cout << "\n\tquantization: " << format.fmt.pix.quantization;
                    std::cout << "\n\ttransfer function: " << format.fmt.pix.xfer_func << std::endl;
                }
            }
        }
#endif


    }
}

/**
 * Show cropping capability of capture
 */

void ShowCaptureCropCap(int fd)
{
    struct v4l2_cropcap crop_cap;

    memset(&crop_cap, 0, sizeof(crop_cap));
    crop_cap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == ioctl(fd, VIDIOC_CROPCAP, &crop_cap))
    {
        std::cout << "Crop capability: " <<std::endl;
        std::cout << "\tBoud: (" << crop_cap.bounds.left << ","
                  << crop_cap.bounds.top << "), "
                  << crop_cap.bounds.width << "x"
                  << crop_cap.bounds.height << std::endl;
        std::cout << "\tDefault Rect: (" << crop_cap.defrect.left << ","
                  << crop_cap.defrect.top << "), "
                  << crop_cap.defrect.width << "x"
                  << crop_cap.defrect.height << std::endl;
        std::cout << "\tPixel Aspect: " << crop_cap.pixelaspect.numerator
                  << "/" << crop_cap.pixelaspect.denominator << std::endl;
    }
    else
    {
        perror("VIDIOC_CROPCAP");
        exit(-1);
    }
}

/**
 * Show input device's streaming parameter 
 */

void ShowCaptureStreamParam(int fd)
{
    struct v4l2_streamparm param;

    param.type =  V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == ioctl(fd, VIDIOC_G_PARM, &param))
    {
        std::cout << "Capture streaming parameters:" << std::endl;
        std::cout << "\tHigh quality: " << std::boolalpha
                  << (param.parm.capture.capturemode == V4L2_MODE_HIGHQUALITY) << std::endl;
        std::cout << "\tTime per frame: " << (static_cast<double>(param.parm.capture.timeperframe.numerator)/
                                            param.parm.capture.timeperframe.denominator)
                  << " (sec)" << std::endl;
        std::cout << "\tInternal buffer amount: " << param.parm.capture.readbuffers << std::endl;
    }
}


int main()
{
    int fd;

    if ((fd = open("/dev/video0", O_RDONLY)) == -1)
    {
        perror("open");
        return -1;
    }

    /* device capability */
    EnumCapbility(fd);

    /* input device info */
    ShowInputInfo(fd);

    /* standard */
    /* (my thinkpad-t460p's camera doesn't has the capability to set standard) */

    //ShowCurrentStandard(fd);
    //EnumSupportedStandard(fd);

    /* control */

    //EnumControls_Old(fd);
    //EnumControls(fd);

    /* image format */
    EnumCaptureImageFormat(fd);
    ShowCaptureCurrentFormat(fd);

    /* crop */
    ShowCaptureCropCap(fd);

    /* streaming parameters */
    ShowCaptureStreamParam(fd);

    close(fd);
}
