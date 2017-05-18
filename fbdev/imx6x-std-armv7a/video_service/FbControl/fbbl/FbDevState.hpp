#ifndef FBDEVSTATE_HPP
#define FBDEVSTATE_HPP

#include "FbDevBL.hpp"

namespace vehicle
{
namespace videoservice
{
    // forwar declare
    class FbDevBL;

    class FbDevState
    {
        public:
            virtual inline ~FbDevState() {}
            virtual inline bool toHome(FbDevBL& bl) {return true;}
            virtual inline bool toAnimation(FbDevBL& bl){return true;}
            virtual inline bool toCamera(FbDevBL& bl){return true;}
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
