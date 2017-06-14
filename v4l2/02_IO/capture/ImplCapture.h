/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 13 Jun 2017 02:48:28 PM CST
 Description: 
 ************************************************************************/

#ifndef __IMPL_CAPTURE_H__
#define __IMPL_CAPTURE_H__

#include <string>
#include <linux/videodev2.h>
#include "ICapture.h"

class ImplCapture: public ICapture
{
    struct MmapBuffer
    {
        void *start;
        size_t length;
    };

    public:

        virtual void Open(std::string dev);

        virtual void Close();

        virtual void StreamOn();

        virtual void StreamOff();

        // blockable
        virtual int DequeOneBuffer(uint8_t **addr);

        virtual void EnqueOneBuffer(int index);

        virtual size_t GetBufferSize(){ return buffer_size_;}

    private:

        void Init();
        void Deinit();
        void AllocateBuffers();
        void FreeBuffers();
        void MapBuffers();
        void QBuffers();
        void UnmapBuffers(MmapBuffer *buffers, int count);

    private:
        int fd_;
        MmapBuffer *buffers_;
        unsigned int count_;
        v4l2_std_id std_;
        size_t buffer_size_;
        uint32_t input_cap_;
};

#endif
