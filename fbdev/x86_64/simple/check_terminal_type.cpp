/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Fri 28 Apr 2017 06:25:59 AM CST
 Description: 
 ************************************************************************/

#include <iostream>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <string>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

using namespace std;

int main()
{
    int mode;
    int fd;
    string term;

    while (1)
    {
        cout << "Please enter terminal to check: ";
        cin >> term;
        if ((fd = open(term.c_str(), O_RDONLY)) == -1)
        {
            cerr << "Open failed: " << strerror(errno) << endl;
            continue;
        }
        ioctl(fd, KDGETMODE, &mode);
        cout << "Mode: " << mode << endl;
    }
}
