#ifndef FBDEVBL_HPP
#define FBDEVBL_HPP

#include "IFbDev.hpp"
#include "IFbDevBL.hpp"
#include "FbDevState.hpp"
#include <stdint.h>

using namespace fbdev;

namespace vehicle
{
namespace videoservice
{
    // forward declare 
    class FbDevState;

    class FbDevBL: public IFbDevBL
    {
        public:

            FbDevBL();
            void setCurrentState(FbDevState *state);
            inline uint32_t getColorKey() {return color_key_;}

            virtual bool setFb(FbState state);

        public:

            IFbDev* fb0_;
            IFbDev* fb1_;

        private:

            // Avoid being deleted from heap.
            // (not necessary if we create the singleton in stack.
            ~FbDevBL();

            // Avoid copy or assignment
            FbDevBL(const FbDevBL&);
            const FbDevBL& operator=(const FbDevBL&);

        private:
            FbDevState *state_;
            const uint32_t color_key_; // negotiated color key, can be improved by reading from some config file
    };
};
};

#endif // FBDEVBL_HPP
