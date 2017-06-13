/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 22 May 2017 12:58:50 PM CST
 Description: 
 ************************************************************************/

#ifndef __IFILEPARSER_H__
#define __IFILEPARSER_H__

#include <string>
#include <map>

typedef std::map<std::string, std::map<std::string, std::string> > CfgInfo;
class IFileParser
{
    public:

        static IFileParser *getInstance();

        /**
         * @brief           Parse a config file and output a map with following format:
         *                  [section][key] = value
         *
         * @param[in]       config file name
         * @return          reference of map of parsed info
         */
        virtual CfgInfo  &parseFile(std::string file) = 0;

        /**
         * @brief           Return the parsed map
         *
         * @return          reference of map of parsed info
         */
        virtual CfgInfo &getInfo() = 0;

};
#endif
