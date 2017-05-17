/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Wed 17 May 2017 06:52:41 PM CST
 Description: 
 ************************************************************************/

#include "IFbDevBL.hpp"
#include <iostream>

using namespace vehicle::videoservice;
using namespace std;

int main(int argc, char *argv[])
{
    IFbDevBL *obj =  IFbDevBL::getInstance();

    obj->init();

    int choice;

    while (choice)
    {
        cout << "1: Home\n2: Animation\n3: Camera\n0: Quit" << endl;
        cin >> choice;

        switch (choice)
        {
            case 1:
                obj->setFb(kHome);
                break;
            case 2:
                obj->setFb(kAnimation);
                break;
            case 3:
                obj->setFb(kCamera);
                break;
        }
    }

    obj->deinit();
}
