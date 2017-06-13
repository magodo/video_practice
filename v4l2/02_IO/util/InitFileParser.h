/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 22 May 2017 01:04:14 PM CST
 Description: 
 ************************************************************************/

#ifndef __INITFILEPARSER_H__
#define __INITFILEPARSER_H__

#include "IFileParser.h"

class InitFileParser: public IFileParser
{
    public:

        virtual CfgInfo &parseFile(std::string file);
        virtual inline CfgInfo &getInfo() {return info_;}

    private:

       CfgInfo info_;

};

#endif
