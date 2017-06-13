/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 22 May 2017 01:06:38 PM CST
 Description: 
 ************************************************************************/

#include "IFileParser.h"
#include "InitFileParser.h"

IFileParser *IFileParser::getInstance()
{
    static IFileParser *obj = new InitFileParser;
    return obj;
}
