/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 13 Jun 2017 09:52:06 AM CST
 Description: 
 ************************************************************************/

#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <sys/mman.h>
#include <fcntl.h>
#include <string>
#include "ImplCapture.h"
#include "IFileParser.h"
#include "IPixFmt.h"

static void ShowImageFormat(struct v4l2_format &format)
{
        std::cout << "Current image format(Capture):"
                  << "\n\twidth: " << format.fmt.pix.width
                  << "\n\theight: " << format.fmt.pix.height
                  << "\n\tpixel format: " << IPixFmt::fourccToStr(format.fmt.pix.pixelformat)
                  << "\n\tfield: " << format.fmt.pix.field
                  << "\n\tbytesperline: " << format.fmt.pix.bytesperline
                  << "\n\tsize image: " << format.fmt.pix.sizeimage
                  << "\n\tcolorspace: " << format.fmt.pix.colorspace << std::endl;
}

bool operator==(struct v4l2_format &format1, struct v4l2_format &format2)
{
    if (format1.type == format2.type &&
        format1.fmt.pix.width == format2.fmt.pix.width &&
        format1.fmt.pix.height == format2.fmt.pix.height &&
        format1.fmt.pix.pixelformat == format2.fmt.pix.pixelformat &&
        format1.fmt.pix.field == format2.fmt.pix.field &&
        format1.fmt.pix.bytesperline == format2.fmt.pix.bytesperline &&
        format1.fmt.pix.sizeimage == format2.fmt.pix.sizeimage &&
        format1.fmt.pix.colorspace == format2.fmt.pix.colorspace)
    {
#ifdef V4L2_PIX_FMT_PRIV_MAGIC
        if (format1.fmt.pix.priv == format2.fmt.pix.priv &&
            format1.fmt.pix.flags == format2.fmt.pix.flags &&   
            format1.fmt.pix.ycbcr_enc == format2.fmt.pix.ycbcr_enc &&   
            format1.fmt.pix.quantization == format2.fmt.pix.quantization &&   
            format1.fmt.pix.xfer_func == format2.fmt.pix.xfer_func)
        {
#endif
            return true;
#ifdef V4L2_PIX_FMT_PRIV_MAGIC
        }
#endif
    }

    ShowImageFormat(format1);
    ShowImageFormat(format2);

    return false;
}

void ImplCapture::Open(std::string dev)
{
    fd_ = open(dev.c_str(), O_RDWR);
    if (fd_ == -1)
    {
        perror("open");
        exit(-1);
    }
}

void ImplCapture::Close()
{
    close(fd_);
    fd_ = 0;
}


void ImplCapture::Init()
{
    IFileParser *parser = IFileParser::getInstance();

    /* Select input */

    int index = strtol(parser->getInfo()["VIDEO"]["input"].c_str(), NULL, 10);

    if (-1 == ioctl(fd_, VIDIOC_S_INPUT, &index))
    {
        perror("VIDIOC_S_INPUT");
        exit(-1);
    }

#if 0
    /* Select standard */

    v4l2_std_id std = 0;

    if (-1 == ioctl(fd_, VIDIOC_G_STD, &std))
    {
        perror("VIDIOC_G_STD");
        exit(-1);
    }

    if (-1 == ioctl(fd_, VIDIOC_S_STD, &std))
    {
        perror("VIDIOC_S_STD");
        exit(-1);
    }
    std::cout << "Standard: 0x" << std::hex << std << std::dec << std::endl;
#endif

    /* Select format */

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = strtol(parser->getInfo()["VIDEO"]["width"].c_str(), NULL, 10);
    format.fmt.pix.height =strtol(parser->getInfo()["VIDEO"]["height"].c_str(), NULL, 10);
    format.fmt.pix.pixelformat = IPixFmt::strToFourcc(
                                 parser->getInfo()["VIDEO"]["pix_fmt"]);

    /* capture shouldn't set "field" */
    //format.fmt.pix.field = V4L2_FIELD_INTERLACED;
    
    if (-1 == ioctl(fd_, VIDIOC_S_FMT, &format))
    {
        perror("VIDIOC_S_FMT");
        exit(-1);
    }

    image_size_ = format.fmt.pix.sizeimage;

    /* Following proves the returned format from VIDIOC_S_FMT
     * is the actual set format. */
#if 0
    /* Check if returned format from VIDIOC_S_FMT
     * is same as return from VIDIOC_G_FMT */
    struct v4l2_format format_g;
    memset(&format_g, 0, sizeof(format_g));
    format_g.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd_, VIDIOC_G_FMT, &format_g))
    {
        perror("VIDIOC_G_FMT");
        exit(-1);
    }
    if (format == format_g)
        std::cout << "VIDIOC_S_FMT returned format is same as VIDIOC_G_FMT" << std::endl;
    else
        std::cout << "VIDIOC_S_FMT returned format is NOT same as VIDIOC_G_FMT" << std::endl;
#endif
    

#if 0
    /* Select stream parameter */

    struct v4l2_streamparm parm;

    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = 0;
    parm.parm.capture.capturemode = 0;
    if (ioctl(fd_, VIDIOC_S_PARM, &parm) < 0)
    {
        perror("VIDIOC_S_PARM");
        exit(-1);
    }
#endif


}

void ImplCapture::Deinit()
{
    UnmapBuffers(buffers_, count_);
}

void ImplCapture::UnmapBuffers(MmapBuffer *buffers, int count)
{
    for (int i = 0; i < count; i++)
    {
        munmap(buffers[i].start, buffers[i].length);
    }

    delete[] buffers;
    buffers_ = nullptr;
    count_ = 0;
}

void ImplCapture::MapBuffers()
{
    /* Request buffers */
    count_ = strtol(IFileParser::getInstance()->getInfo()["VIDEO"]["buffer_count"].c_str(), NULL, 10);
    struct v4l2_requestbuffers req_buf;

    memset(&req_buf, 0, sizeof(req_buf));
    req_buf.count = count_;
    req_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req_buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd_, VIDIOC_REQBUFS, &req_buf))
    {
        perror("VIDIOC_REQBUFS");
        exit(-1);
    }

    /* Check how many buffers been allocated */
    if (req_buf.count < count_)
    {
        std::cerr << "Not enough buffer allocated..." << std::endl;
        exit(-1);
    }

    /* Query each buffer and mmap them */
    buffers_ = new MmapBuffer[req_buf.count]();

    for (unsigned int i = 0; i < req_buf.count; ++i)
    {
        struct v4l2_buffer buffer;
        memset(&buffer, 0, sizeof(buffer));

        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.index = i;
        buffer.memory = V4L2_MEMORY_MMAP;

        if (-1 == ioctl(fd_, VIDIOC_QUERYBUF, &buffer))
        {
            /* If don't exit here, need to unmap mapped buffer
             * and delete buffers[] */
            perror("VIDIOC_QUERYBUF");
            exit(-1);
        }

        buffers_[i].length = buffer.length;
        buffers_[i].start = mmap(NULL, buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, buffer.m.offset);

        if (MAP_FAILED == buffers_[i].start)
        {
            /* If don't exit here, need to unmap mapped buffer
             * and delete buffers[] */
            perror("mmap");
            exit(-1);
        }
    }
}

void ImplCapture::QBuffers()
{
    struct v4l2_buffer buffer;

    for (unsigned int i = 0; i < count_; i++)
    {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.index = i;
        buffer.memory = V4L2_MEMORY_MMAP;
        if (-1 == ioctl(fd_, VIDIOC_QBUF, &buffer))
        {
            perror("VIDIOC_QBUF");
            Deinit();
            exit(-1);
        }
    }
}

void ImplCapture::StreamOn()
{
    /* following code only applys to input whose 
     * capability supprt "standard" interface. */
#if 0
    v4l2_std_id curr_std_id;
    int counter_keep, counter_elapse;

    counter_elapse = counter_keep = 0;

    if (-1 == ioctl(fd_, VIDIOC_G_STD, &std_))
    {
        perror("VIDIOC_G_STD");
        exit(-1);
    }

    /* wait until standard is steady for 100ms */
    while (counter_keep < 10 && counter_elapse < 500)
    {
        if (-1 == ioctl(fd_, VIDIOC_G_STD, &curr_std_id))
        {
            perror("VIDIOC_G_STD");
            exit(-1);
        }

        if (curr_std_id != V4L2_STD_UNKNOWN && curr_std_id == std_)
        {
            counter_keep++;
        }
        else 
        {
            std_ = curr_std_id;
            counter_keep = 0;

            if (curr_std_id == V4L2_STD_UNKNOWN)
            {
                /* TODO: black the framebuffer? */
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter_elapse++;
    }

    if (counter_elapse == 500)
    {
        std::cerr << "Signal standard is not steady for 5 seconds, quiting..." << std::endl;
        exit(-1);
    }
#endif

    MapBuffers();

    QBuffers();

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd_, VIDIOC_STREAMON, &type))
    {
        perror("VIDIOC_STREAMON");
        exit(-1);
    }
}

void ImplCapture::StreamOff()
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd_, VIDIOC_STREAMOFF, &type))
    {
        perror("VIDIOC_STREAMOFF");
        exit(-1);
    }
}

int ImplCapture::DequeOneBuffer(uint8_t **addr)
{
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;

    if (-1 == ioctl(fd_, VIDIOC_DQBUF, &buffer))
    {
        perror("VIDIOC_DQBUF");
        exit(-1);
    }

    if (addr != nullptr)
        *addr = static_cast<uint8_t*>(buffers_[buffer.index].start);
    std::cout << "Deque: " << buffer.index << std::endl;
    return buffer.index;
}

void ImplCapture::EnqueOneBuffer(int index)
{
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = index;

    if (-1 == ioctl(fd_, VIDIOC_QBUF, &buffer))
    {
        perror("VIDIOC_QBUF");
        exit(-1);
    }
}
