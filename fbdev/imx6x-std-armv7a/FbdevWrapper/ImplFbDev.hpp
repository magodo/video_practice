#ifndef IMPL_FBDEV_HPP
#define IMPL_FBDEV_HPP

#include "IFbDev.hpp"

namespace Fbdev
{
    class ImplFbDev: public IFbDev
    {
        public:
            ImplFbDev();
            ~ImplFbDev();

            void drawPixel(int x, int y, uint32_t clr_rgb24);
            inline uint32_t getXRes() { return m_vinfo.xres_virtual; }
            inline uint32_t getYRes() { return m_vinfo.yres_virtual; }

            virtual bool init(std::string dev);
            virtual bool uninit();
            virtual bool setPixFormat(uint32_t pix_fmt);
            virtual bool setGlobalAlpha(uint16_t alpha);
            virtual bool setLocalAlpha();
            virtual bool setColorKey(uint32_t color_key);
            virtual bool unsetColorKey();

        private:
            const uint32_t m_default_pix_fmt;
            const uint16_t m_default_width;
            const uint16_t m_default_height;

            int m_fd;
            struct fb_fix_screeninfo m_finfo;
            struct fb_var_screeninfo m_vinfo;
            uint8_t *m_fb_buf;
            uint32_t m_fb_size;

            uint32_t bppOfFmt(uint32_t pix_fmt);
            uint32_t colorFromRGB24(uint32_t clr_rgb24);
    };
};

#endif // IMPL_FBDEV_HPP
