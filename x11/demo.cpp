/*************************************************************************
 Author: Zhaoting Weng
 Created Time: Mon 08 May 2017 10:09:38 AM CST
 Description: 
 ************************************************************************/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/un.h>

#include <iostream>
#include <string>

using namespace std;

/* Core OP Code (only define the ones we need to use) */

#define X11_OP_REQ_CREATE_WINDOW	0x01
#define X11_OP_REQ_MAP_WINDOW		0x08
#define X11_OP_REQ_CREATE_GC		0x37

/* Flag (only define the ones we need to use) */

#define X11_FLAG_GC_FUNC 0x00000001
#define X11_FLAG_GC_PLANE 0x00000002
#define X11_FLAG_GC_BG 0x00000004
#define X11_FLAG_GC_FG 0x00000008
#define X11_FLAG_GC_LINE_WIDTH 0x00000010
#define X11_FLAG_GC_LINE_STYLE 0x00000020
#define X11_FLAG_GC_FONT 0x00004000

#define X11_FLAG_WIN_BG_IMG 0x00000001
#define X11_FLAG_WIN_BG_COLOR 0x00000002
#define X11_FLAG_WIN_BORDER_IMG 0x00000004
#define X11_FLAG_WIN_BORDER_COLOR 0x00000008
#define X11_FLAG_WIN_EVENT 0x00000800

/* Structures */

struct x11_conn_req
{
    uint8_t bit_order;
    uint8_t pad1;
    uint16_t major, minor;
    uint16_t auth_proto, auth_data;
    uint16_t pad2;
};

struct x11_conn_reply
{
    uint8_t is_success; // 1: success; 0: fail
    uint8_t pad1;
    uint16_t major, minor;
    uint16_t quarter_length;
};

// fixed size of 24 bytes
struct x11_conn_setup
{
    uint32_t release;                   // the actual version of X server running
    uint32_t id_base, id_mask;          // resource IDs
    uint32_t motion_buffer_size;
    uint16_t vendor_length;             // we know how many bytes is the vendor section
    uint16_t request_max;
    uint8_t n_roots;                    // number of root windows
    uint8_t n_pixmap_formats;           // number of available formats
    uint8_t image_order;                // for endians
    uint8_t bitmap_order;               // for endians
    uint8_t scanline_unit, scanline_pad;
    uint8_t keycode_min, keycode_max;
    uint32_t pad;
};

// immediately following setup section is the vendor section (const char*)

// One or more following structures
// (the ammount is determined in "x11_conn_setup.n_pixmap_formats")

struct x11_pixmap_format
{
    uint8_t depth;
    uint8_t bpp;
    uint8_t scanline_pad;
    uint8_t pad1;
    uint32_t pad2;
};

// There might be multiple root windows (determined by "x11_conn_setup.n_roots")

struct x11_root_window
{
    uint32_t id;
    uint32_t colormap;
    uint32_t white, black;
    uint32_t input_mask;
    uint16_t width, height;
    uint16_t width_mm, height_mm;
    uint16_t maps_min, maps_max;
    uint32_t root_visual_id;
    uint8_t backing_store;
    uint8_t save_unders;
    uint8_t depth;
    uint8_t n_depths; // number of depth
};

// might be multiple depths (determined by x11_root_window.n_depths)

struct x11_depth
{
    uint8_t depth;
    uint8_t pad1;
    uint16_t n_visuals; // number of visuals for each depth
    uint32_t pad2;
};

// might be multiple visuals (determined by x11_depth.n_visuals)
struct x11_visual
{
    uint8_t group;
    uint8_t bits;
    uint16_t colormap_entires;
    uint32_t mask_red, mask_green, mask_blue;
    uint32_t pad;
};

/** normally the reply packet should have following structure
 *  
 *  |_ header
 *  |
 *  |_ setup
 *  |
 *  |_ vendor info
 *  |
 *  |_ format1
 *  .
 *  ._ formatN
 *  |
 *  |_ root window 1
 *  |
 *  |__ depth11
 *  |
 *  |___ visual111
 *  .
 *  .___ visual11N
 *  .
 *  .__ depth1N
 *  |
 *  ._ root window N
 *  |
 *  |__ depthN1
 *  |
 *  |___ visualNN1
 *  .
 *  .___ visualNNN
 *  .
 *  .__ depthNN
 *
 */

struct x11_connection
{
    struct x11_conn_reply header;       // store the reply header
    struct x11_conn_setup *setup;       // pointer to the reset reply (setup sections)
    struct x11_pixmap_format *format;   // pointer to the 1st format section, and there are consecutive setup.n_pixmap_formats elements in this array
    struct x11_root_window **root;      // root[i] is pointer to i-th root window section
    struct x11_depth ***depth;          // depth[i][j] is pointer to i-th root window j-th depth section
    struct x11_visual ****visual;       // visual[i][j][k] is pointer to i-th root window j-th depth k-th visual section
};


/* Implementation */

void err_exit(string msg)
{
    cerr << msg << ": " << strerror(errno) << endl;
    exit(1);
}

int x11_handshake(int sock, struct x11_connection *conn)
{
    struct x11_conn_req req = {0};
    req.bit_order = 'l'; // little endian
    req.major = 11;  // version 11.0
    req.minor = 0;
    write(sock, &req, sizeof(struct x11_conn_req)); // send request

    read(sock, &conn->header, sizeof(struct x11_conn_reply)); // read reply header
    if (!conn->header.is_success)
    {
        cerr << "request failed!" << endl;
        return conn->header.is_success;
    }

    conn->setup = (struct x11_conn_setup*)malloc(conn->header.quarter_length * 4);
    read(sock, conn->setup, conn->header.quarter_length * 4);

    conn->format = (struct x11_pixmap_format*)((uint8_t*)(conn->setup) + sizeof(struct x11_conn_setup) + conn->setup->vendor_length);

    conn->root = (struct x11_root_window**)malloc(sizeof(void*) * conn->setup->n_roots);
    conn->depth = (struct x11_depth***)malloc(sizeof(void*) * conn->setup->n_roots);
    conn->visual = (struct x11_visual****)malloc(sizeof(void*) * conn->setup->n_roots);

    uint8_t *p_tmp = (uint8_t*)(conn->format) + sizeof(struct x11_pixmap_format) * conn->setup->n_pixmap_formats;

    for (int i_root = 0; i_root < conn->setup->n_roots; ++i_root)
    {
        conn->root[i_root] = (struct x11_root_window*)p_tmp;
        p_tmp += sizeof(struct x11_root_window);

        conn->depth[i_root] = (struct x11_depth**)malloc(sizeof(void*) * conn->root[i_root]->n_depths);
        conn->visual[i_root] = (struct x11_visual***)malloc(sizeof(void*) * conn->root[i_root]->n_depths);

        for (int i_depth = 0; i_depth < conn->root[i_root]->n_depths; ++i_depth)
        {
            conn->depth[i_root][i_depth] = (struct x11_depth*)p_tmp;
            p_tmp += sizeof(struct x11_depth);

            conn->visual[i_root][i_depth] = (struct x11_visual**)malloc(sizeof(void*) * conn->depth[i_root][i_depth]->n_visuals);

            for (int i_visual = 0; i_visual < conn->depth[i_root][i_depth]->n_visuals; ++i_visual)
            {
                conn->visual[i_root][i_depth][i_visual] = (struct x11_visual*)p_tmp;
                p_tmp += sizeof(struct x11_visual);
            }
        }
    }

    return conn->header.is_success;
}

void x11_show_conn_info(struct x11_connection *conn)
{
    cout << "Number of format: " << (unsigned int)conn->setup->n_pixmap_formats << endl;
    cout << "Number of root windows: " << (unsigned int)conn->setup->n_roots << endl;
    for (int i_root = 0; i_root < conn->setup->n_roots; ++i_root)
    {
        cout << "* root window " << i_root << endl;
        cout << "  number of depths: " << (unsigned int)conn->root[i_root]->n_depths << endl;

        for (int i_depth = 0; i_depth < conn->root[i_root]->n_depths; ++i_depth)
        {
            cout << "** depth " << i_depth << endl;
            cout << "   number of visuals: " << (unsigned int)conn->depth[i_root][i_depth]->n_visuals << endl;

            for (int i_visual = 0; i_visual < conn->depth[i_root][i_depth]->n_visuals; ++i_visual)
            {
                /* do somthing on visual*/
            }
        }
    }
}

/* generate reosurce id, using a static variable to record last id used */
uint32_t x11_generate_id(struct x11_connection *conn)
{
    static uint32_t id = 0;
    return ((conn->setup->id_mask & id++) + conn->setup->id_base);
}

// count bit of 32-bit integer
int count_bits(uint32_t n)
{
    uint32_t tmp;
    tmp = n - ((n >> 1) & 033333333333) - (n >> 2) & 011111111111;
    return ((tmp + (tmp >> 3)) & 030707070707) % 63;
}

void x11_create_gc(int sock, struct x11_connection* conn, uint32_t id, uint32_t target, uint32_t flags, uint32_t *list)
{
    uint16_t flag_count = count_bits(flags);
    uint16_t length = 4 + flag_count;
    uint32_t *packet = (uint32_t*)malloc(length*sizeof(uint32_t));

    packet[0] = X11_OP_REQ_CREATE_GC | length << 16; // high-16 bits mean the count of packet, lower-16 bits mean the OP code
    packet[1] = id;
    packet[2] = target;
    packet[3] = flags;

    int i;
    for (i = 0; i < flag_count; ++i)
        packet[4+i] = list[i];

    write(sock, packet, length*4);
    free(packet);
    return;
}

void x11_create_win(int sock, struct x11_connection *conn, uint32_t id, uint32_t parent,
                    uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    uint16_t border, uint16_t group, uint32_t visual,
                    uint32_t flags, uint32_t *list)
{
    uint16_t flag_count = count_bits(flags);

    uint16_t length = 8 + flag_count;
    uint32_t *packet = (uint32_t*)malloc(length * sizeof(uint32_t));

    packet[0] = X11_OP_REQ_CREATE_WINDOW | length << 16;
    packet[1] = id;
    packet[2] = parent;
    packet[3] = x | y << 16;
    packet[4] = w | h << 16;
    packet[5] border << 16 | group;
    packet[6] = visual;
    packet[7] = flags;
    int i;
    for (i = 0; i < flag_count; ++i)
        packet[8+i] = list[i];

    write(sock, packet, length*sizeof(uint32));
    free(packet);
    return;
}

void x11_map_window(int sock, struct x11_connection *conn, uint32_t id)
{
    uint32_t packet[2];
    packet[0] = X11_OP_REQ_MAP_WINDOW | 2 << 16;
    packet[1] = id;
    write(sock, packet, 8);
    return;
}




int main()
{
    int sockfd;
    struct sockaddr_un serv_addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
        err_exit("Error opeing socket");

    serv_addr.sun_family = AF_UNIX;
    // check DISPLAY env.var.
    strcpy(serv_addr.sun_path, "/tmp/.X11-unix/X1");
    // Connect to socket
    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_un));
    
    // handshake with x11 server
    struct x11_connection conn = {0};
    x11_handshake(sockfd, &conn);

    //x11_show_conn_info(&conn);
}
