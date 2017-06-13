/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 15 May 2017 03:31:56 PM CST
 Description: 
 ************************************************************************/

#ifndef I_FBDEV_HPP
#define I_FBDEV_HPP

#include <linux/fb.h>
#include <stdint.h>
#include <string>

#ifdef MXCFB
#include <linux/mxcfb.h>
#endif

namespace fbdev
{
    class IFbDev
    {
        public:

            static IFbDev* getInstance();

            virtual ~IFbDev();

            /**
             * @brief       Open fb and set default screen info.
             *
             * @param[in]   absolute path of fb device.
             * @return      true if success, false otherwise.
             */
            virtual bool init(std::string dev) = 0;

            /**
             * @brief       Close fb and reset some internal status.
             *
             * @return      true if success, false otherwise.
             */
            virtual bool deinit() = 0;

            /**
             * @brief       Set pixel format of opened fb.
             *
             * @param[in]   pixel format, in fourcc format
             * @return      true if success, false otherwise.
             */
            virtual bool setPixFormat(uint32_t pix_fmt) = 0;

            /**
             * @brief       Get virtual framebuffer address
             */
            virtual uint8_t *getVirtualFbAddr() = 0; 

            virtual size_t getVirtualFbSize() = 0; 

#ifdef MXCFB
            /**
             * @brief       Set global alpha for opened fb.
             *
             * @param[in]   16-bit integer: from 0xff(opaque) to 0x00, the transparency increases
             * @return      true if success, false otherwise.
             */
            virtual bool setGlobalAlpha(uint16_t alpha) = 0;

            /**
             * @brief       Set color key.
             *
             * @param[in]   RGB24 format color to hide.
             * @return      true if success, false otherwise.
             */
            virtual bool setColorKey(uint32_t color_key) = 0;

            /**
             * @brief       Unset color key.
             *
             * @return      true if success, false otherwise.
             */
            virtual bool unsetColorKey() = 0;

            /**
             * @brief       Set fb to be blank.
             *
             * @return      true if success, false otherwise.
             * @note        if current fb is background, then the overlay fb will be blank also
             */
            virtual bool blank() = 0;

            /**
             * @brief       Set fb to be unblank
             *
             * @return      true if success, false otherwise.
             */
            virtual bool unBlank() = 0;
#endif
    };

};

#endif // I_FBDEV_HPP
