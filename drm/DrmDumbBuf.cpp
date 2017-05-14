/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Fri 28 Apr 2017 09:09:34 AM CST
 Description: 
 ************************************************************************/

#include <fcntl.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

struct fb_info
{
    void *fb_base;
    long fb_w;
    long fb_h;
};


int main()
{
    //-------------------------------------
    // Open dri device
    //-------------------------------------
    
    int dri_fd = open("/dev/dri/card0", O_RDWR);
    if (dri_fd == -1)
        cerr << "Open card failed: "  << strerror(errno) << endl;

    //-------------------------------------
    // KMS
    //-------------------------------------
    
    // set drm as master
    if (ioctl(dri_fd, DRM_IOCTL_SET_MASTER, 0) == -1)
    {
        cerr << "DRM_IOCTL_SET_MASTER failed: "  << strerror(errno) << endl;
    }

    // get resource counts then resource IDs
    struct drm_mode_card_res res = {0};

    errno = 0;
    if (ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res) == -1)
        cerr << "DRM_IOCTL_MODE_GETRESOURCES failed: "  << strerror(errno) << endl;

    cout << "[RES] Count: " << res.count_fbs << "(fb)"\
         << ", " << res.count_crtcs << "(crtc)" \
         << ", " << res.count_connectors << "(conn)" \
         << ", " << res.count_encoders << "(enc)" << endl;

    res.fb_id_ptr = (uint64_t)(new uint64_t[res.count_fbs]());
    res.crtc_id_ptr = (uint64_t)(new uint64_t[res.count_crtcs]());
    res.connector_id_ptr = (uint64_t)(new uint64_t[res.count_connectors]());
    res.encoder_id_ptr = (uint64_t)(new uint64_t[res.count_encoders]());

    if (ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res) == -1)
        cerr << "DRM_IOCTL_MODE_GETRESOURCES failed: "  << strerror(errno) << endl;

    // --------------------------------------------
    // Loop through all available connecters
    // --------------------------------------------
    
    vector<struct fb_info> v_fbs; 

    for (int i = 0; i < res.count_connectors; ++i)
    {
        cout << endl << "Connector Index: " << i << endl;
        struct drm_mode_get_connector conn={0};
        conn.connector_id = ((uint64_t*)(res.connector_id_ptr))[i];

        // get connector resource counts
        if (ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn) == -1)
        {
            cerr << "Get connector resource counters for connid " << conn.connector_id << " failed: "  << strerror(errno) << endl;
            continue;
        }

        cout << "[Conn] Count: " << conn.count_encoders << "(enc)" \
             << ", " << conn.count_props << "(prop)" \
             << ", " << conn.count_modes << "(mode)" << endl;

       // FIXME (next 3 lines) the type of buffer probably not uint64_t, if it ought to be larger, then there is a bug here...
        conn.encoders_ptr = (uint64_t)(new uint64_t[conn.count_encoders]());
        conn.props_ptr =(uint64_t)(new uint64_t[conn.count_props]());
        conn.prop_values_ptr = (uint64_t)(new uint64_t[conn.count_props]());
        conn.modes_ptr = (uint64_t)(new struct drm_mode_modeinfo[conn.count_modes]());

        // get connector resources
        if (ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn) == -1)
        {
            cerr << "Get connector resources for connid " << conn.connector_id << " failed: "  << strerror(errno) << endl;
            goto err1;
        }

        // check if the connector is OK to use (connected to something)
        if (conn.count_encoders < 1 || conn.count_modes < 1 || !conn.encoder_id || !conn.connection)
        {
            cerr << "[Conn] Connector " << conn.connector_id << " not available!" << endl;
            goto err1;
        }

        // pick up the fist available mode of the connector for use
        struct drm_mode_modeinfo mode_info;
        mode_info = ((struct drm_mode_modeinfo*)(conn.modes_ptr))[0];
        cout << "[Conn-mode] display: " << mode_info.hdisplay << "x" << mode_info.vdisplay << endl;

        //-----------------------------------
        // Creating a dumb buffer
        //-----------------------------------
        struct drm_mode_create_dumb create_dumb;
        struct drm_mode_map_dumb map_dumb;
        struct drm_mode_fb_cmd cmd_dumb;

        create_dumb = {0};
        map_dumb = {0};
        cmd_dumb = {0};

        create_dumb.width = mode_info.hdisplay;
        create_dumb.height = mode_info.vdisplay;
        create_dumb.bpp = 32;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) == -1)
        {
            cerr << "[Dumb] create dumb failed: " << strerror(errno) << endl;
            goto err1;
        }

        cmd_dumb.width = create_dumb.width;
        cmd_dumb.height = create_dumb.height;
        cmd_dumb.bpp = create_dumb.bpp;
        cmd_dumb.pitch = create_dumb.pitch;
        cmd_dumb.depth = 24;
        cmd_dumb.handle = create_dumb.handle;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_ADDFB, &cmd_dumb) == -1)
        {
            cerr << "[Dumb] add dumb fb failed: " << strerror(errno) << endl;
            goto err1;
        }

        map_dumb.handle = create_dumb.handle;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) == -1)
        {
            cerr << "[Dumb] map dumb failed: " << strerror(errno) << endl;
            goto err1;
        }

        void *fb_base;
        fb_base = mmap(0, create_dumb.size, PROT_READ|PROT_WRITE, MAP_SHARED, dri_fd, map_dumb.offset);
        if (fb_base == MAP_FAILED)
        {
            cerr << "[Dumb] mmap dumb failed: " << strerror(errno) << endl;
            goto err1;
        }

        v_fbs.push_back({fb_base, create_dumb.width, create_dumb.height});

        // set crtc
        struct drm_mode_get_encoder enc;
        enc = {0};
        enc.encoder_id = conn.encoder_id;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_GETENCODER, &enc) == -1)
        {
            cerr << "[Enc] get encoder failed: " << strerror(errno) << endl;
            goto err1;
        }

        struct drm_mode_crtc crtc;
        crtc = {0};

        crtc.crtc_id = enc.crtc_id;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_GETCRTC, &crtc) == -1)
        {
            cerr << "[Crtc] get crtc failed: " << strerror(errno) << endl;
            goto err1;
        }
        crtc.fb_id = cmd_dumb.fb_id;
        crtc.set_connectors_ptr = res.connector_id_ptr+i;
        crtc.count_connectors = 1;
        crtc.mode = ((struct drm_mode_modeinfo*)conn.modes_ptr)[0];
        crtc.mode_valid = 1;
        if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) == -1)
        {
            cerr << "[Crtc] set crtc failed: " << strerror(errno) << endl;
            goto err1;
        }

        continue;

err1:
        delete[] (uint64_t*)(conn.encoders_ptr);
        delete[] (uint64_t*)(conn.props_ptr);
        delete[] (uint64_t*)(conn.prop_values_ptr);
        delete[] (struct drm_mode_modeinfo*)(conn.modes_ptr);
        continue;
    }

    // stop being "master" of DRI device 
    ioctl(dri_fd, DRM_IOCTL_DROP_MASTER, 0);

    // draw something

    struct timespec t_start, t_end;
    double dur;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    
    int x,y;
    for (int i = 0; i < 10; ++i)
    {
        for(int idx = 0; idx < v_fbs.size(); ++idx)
        {
            int col = (rand()%0x00ffffff)&0x00ff00ff;
            for (y = 0; y < v_fbs[idx].fb_h; ++y)
                for (x = 0; x < v_fbs[idx].fb_w; ++x)
                {
                    int location = y * v_fbs[idx].fb_w + x;
                    *((uint32_t*)v_fbs[idx].fb_base + location) = col;
                }
        }
        usleep(100000); // 100ms
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);

    dur = (t_end.tv_sec - t_start.tv_sec)+((double)(t_end.tv_nsec-t_start.tv_nsec))/1000000000;
    cout << "Rendering spent: " <<  dur << "(s)" << endl;

}
