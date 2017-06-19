#ifndef IMPL_FBDEV_HPP
#define IMPL_FBDEV_HPP

#include <stdint.h>
#include "IFbDev.h"

namespace fbdev
{
    class ImplFbDev: public IFbDev
    {
        public:
            ImplFbDev();
            ~ImplFbDev();

            /**
             * @brief       Draw one or more pixels starts from (x,y) with "color" (pix_fmt is the same as fb current setting).
             *
             * @param[in]   x coordinate
             * @param[in]   y coordinate
             * @param[in]   the content to be set in the corresponding pixel(s)
             *
             * @note        For some pixel format, calling this API will draw more than one pixels at some point and 0 pixel at others.
             *              For example, for "UYVY" format, calling this API with x,y = 0,0 will draw (0,0) and (0,1); but will draw
             *                           nothing if calling with x,y = 1,0.
             */
            void drawPixel(int x, int y, uint32_t pix_content);
            inline uint32_t getXRes() { return m_vinfo.xres_virtual; }
            inline uint32_t getYRes() { return m_vinfo.yres_virtual; }
            
            virtual bool init(std::string dev);
            virtual bool deinit();
            virtual bool setPixFormat(uint32_t pix_fmt);
            virtual inline uint8_t *getVirtualFbAddr() {return m_fb_buf;}
            virtual inline size_t getVirtualFbSize() {return m_fb_size;} 
            virtual void renderFrame(uint8_t*, size_t);

#ifdef MXCFB
            virtual bool setGlobalAlpha(uint16_t alpha);
            virtual bool setColorKey(uint32_t color_key);
            virtual bool unsetColorKey();
            virtual bool blank();
            virtual bool unBlank();
#endif

        private:
            const uint32_t m_default_pix_fmt;
            const uint16_t m_default_width;
            const uint16_t m_default_height;

            int m_fd;
            struct fb_fix_screeninfo m_finfo;
            struct fb_var_screeninfo m_vinfo;
            uint8_t *m_fb_buf;
            size_t m_fb_size;
    };
};

#endif // IMPL_FBDEV_HPP
