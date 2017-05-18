#ifndef IFBDEVBL_HPP
#define IFBDEVBL_HPP

namespace vehicle
{
namespace videoservice
{
    typedef enum
    {
        kHome,
        kAnimation,
        kCamera
    } FbState;

    class IFbDevBL
    {
        public:

            /**
             * @brief       get the BL singleton
             * @return      BL singleton
             */
            static IFbDevBL *getInstance();

            /**
             * @brief       set FBs to a specific state(defined by product)
             * @param[in]   FB state
             * @return      true if success, false otherwise.
             */
            virtual bool setFb(FbState state) = 0;

        protected:

            // avoid directly new its ineritances
            IFbDevBL() {}
    };
};
};
#endif // IFBDEVBL_HPP
