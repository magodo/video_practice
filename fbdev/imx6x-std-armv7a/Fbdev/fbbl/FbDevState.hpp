#ifndef FBDEVSTATE_HPP
#define FBDEVSTATE_HPP

#include "FbDevBL.hpp"
#include "IFbDev.hpp"

using namespace fbdev;

namespace vehicle
{
namespace videoservice
{
    class FbDevBL;

    class FbDevState
    {
        public:

            bool init();
            bool deinit();

            virtual inline bool toHome(FbDevBL& bl) {return true;}
            virtual inline bool toAnimation(FbDevBL& bl) {return true;}
            virtual inline bool toCamera(FbDevBL& bl) {return true;}
            virtual ~FbDevState();

        protected:
            static IFbDev* fb0_;
            static IFbDev* fb1_;
    };

    class FbDevStateHome: public FbDevState
    {
        public:
            virtual bool toAnimation(FbDevBL& bl);
            virtual bool toCamera(FbDevBL& bl);
    };

    class FbDevStateAnimation: public FbDevState
    {
        public:
            virtual bool toHome(FbDevBL& bl);
            virtual bool toCamera(FbDevBL& bl);
    };

    class FbDevStateCamera: public FbDevState
    {
        public:
            virtual bool toHome(FbDevBL& bl);
            virtual bool toAnimation(FbDevBL& bl);
    };
};
};
#endif // FBDEVSTATE_HPP
