/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Tue 13 Jun 2017 02:48:28 PM CST
 Description: 
 ************************************************************************/

#ifndef __I_CAPTURE_H__
#define __I_CAPTURE_H__

#include <string>

class ICapture
{
    public:
        
        static ICapture *getInstance();

        virtual void Open(std::string dev) = 0;

        virtual void Close() = 0;

        virtual void Init() = 0;

        virtual void Deinit() = 0;

        virtual void StreamOn() = 0;

        virtual void StreamOff() = 0;

        // blockable
        virtual int DequeOneBuffer(uint8_t **addr) = 0;

        virtual void EnqueOneBuffer(int index) = 0;

        virtual size_t GetImageSize() = 0;
};

#endif
