#include <string.h>

#include "svgui.h"
#include "mdtv_app.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_finance.h"
#include "mdtv_animation.h"
#include "mdtv_toolbar.h"

#define NDEBUG  1
#ifndef NDEBUG
#define DEBUG_TRACE(P) {fprintf (stderr, "FIXME: %s,%d,%s: %s\n", __FILE__, __LINE__, __FUNCTION__ ,(P));}

#define error(fmt...) fprintf (stderr, "mdtv_finance[ERROR]:"fmt)
#define debug(fmt...) fprintf (stderr, "mdtv_finance[DEBUG]:"fmt)

#else
#define DEBUG_TRACE(P) 
#define error(fmt...) fprintf (stderr, "mdtv_finance[ERROR]:"fmt)
#define debug(fmt...)
#endif

#define HAVE_ANIMATION  1

struct gushi_t{
    float first_pix;
    int num_pixes;
    int height_pixes;
    int pixes[0];
};
static struct gushi_t* gushi;
static int finance_status = 0;
static int finance_big_status = 0;

static HWND s_hwnd_normal_wnd = NULL;
//static RECT finance_rect;
static RECT finance_nomal_wnd_rect;
static RECT finance_large_wnd_rect;
static RECT finance_content_right_block_normal_wnd_rect;
static RECT finance_content_right_block_big_wnd_rect;
typedef enum _WND_STATUS{
    WND_STATUS_NOT_DISPLAY,
    WND_STATUS_SMALL,
    WND_STATUS_LARGE,
}WND_STATUS;

static WND_STATUS s_finance_wnd_status = WND_STATUS_NOT_DISPLAY;

#define FINANCE_EMBED_WND_X_OFFSET      (7)

#define FINANCE_WND_WIDTH           (388)
#define FINANCE_WND_HEIGHT          (682)

#define FINANCE_WND_POS_X           (51)
#define FINANCE_WND_POS_Y            (20)

#define FINANCE_EMBED_WND_START_X   (FINANCE_EMBED_WND_X_OFFSET)

#define FINANCE_TITLE_1_WIDTH       (379)
#define FINANCE_TITLE_1_HEIGHT      (50)
#define FINANCE_TITLE_2_WIDTH       (376)
#define FINANCE_TITLE_2_HEIGHT      (36)
#define FINANCE_TITLE_3_ITEM_WIDTH       (94)
#define FINANCE_TITLE_3_ITEM_HEIGHT      (34)
#define FINANCE_TITLE_3_WIDTH       (FINANCE_TITLE_3_ITEM_WIDTH*4)
#define FINANCE_TITLE_3_HEIGHT      (FINANCE_TITLE_3_ITEM_HEIGHT)
#define FINANCE_ARROW_WND_WIDTH     (41)
//#define FINANCE_ARROW_WND_HEIGHT    (222)
#define FINANCE_CONTENT_WIDTH        (FINANCE_WND_WIDTH-2*FINANCE_EMBED_WND_X_OFFSET)
#define FINANCE_CONTENT_HEIGHT       (222-34)
#define FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH        (FINANCE_CONTENT_WIDTH)
#define FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT       (30)
#define FINANCE_CONTENT_LEFT_BLOCK_WIDTH        (80)
#define FINANCE_CONTENT_LEFT_BLOCK_HEIGHT       (FINANCE_CONTENT_HEIGHT-FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT)
#define FINANCE_CONTENT_RIGHT_BLOCK_WIDTH        (FINANCE_CONTENT_WIDTH-FINANCE_CONTENT_LEFT_BLOCK_WIDTH)
#define FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT       (FINANCE_CONTENT_LEFT_BLOCK_HEIGHT)

#define FINANCE_ITEM_WIDTH          (373)
//#define FINANCE_ITEM_WIDTH          (FINANCE_WND_WIDTH-2*FINANCE_EMBED_WND_X_OFFSET)
#define FINANCE_ITEM_HEIGHT         ((FINANCE_WND_HEIGHT-FINANCE_TITLE_1_HEIGHT-FINANCE_TITLE_2_HEIGHT-FINANCE_TITLE_3_ITEM_HEIGHT-FINANCE_CONTENT_HEIGHT-20)/6)
//#define FINANCE_ITEM_HEIGHT         (50)


#define FINANCE_ITEM_START_X        (FINANCE_EMBED_WND_START_X)
#define FINANCE_ITEM_START_Y        (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_HEIGHT)


#define FINANCE_LEFTSPACE_PERCENT   (1.0*FINANCE_WND_POS_X/DEFAULT_SCREEN_WIDTH)
#define FINANCE_TOPSPACE_PERCENT    (1.0*FINANCE_WND_POS_Y/DEFAULT_SCREEN_HEIGHT)
#define FINANCE_WND_WIDTH_PERCENT   (1.0*FINANCE_WND_WIDTH/DEFAULT_SCREEN_WIDTH)
#define FINANCE_WND_HEIGHT_PERCENT  (1.0*FINANCE_WND_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define FINANCE_LARGE_WND_SCALE     (1.3)
#define FINANCE_LARGE_WND_WIDTH_PERCENT   (FINANCE_LARGE_WND_SCALE*FINANCE_WND_WIDTH_PERCENT)
#define FINANCE_LARGE_WND_HEIGHT_PERCENT  (FINANCE_LARGE_WND_SCALE*FINANCE_WND_HEIGHT_PERCENT)

#define FINANCE_CONTENT_RIGHT_BLOCK_WND_X_PERCENT   (1.0*(FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_EMBED_WND_START_X)/DEFAULT_SCREEN_WIDTH)
#define FINANCE_CONTENT_RIGHT_BLOCK_WND_Y_PERCENT   (1.0*(FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)/DEFAULT_SCREEN_HEIGHT)
#define FINANCE_CONTENT_RIGHT_BLOCK_WND_WIDTH_PERCENT   (1.0*FINANCE_CONTENT_RIGHT_BLOCK_WIDTH/DEFAULT_SCREEN_WIDTH)
#define FINANCE_CONTENT_RIGHT_BLOCK_WND_HEIGHT_PERCENT   (1.0*FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT/DEFAULT_SCREEN_HEIGHT)

#define BIG_WND_SCALE               1.5

#define FINANCE_BIG_WND_WIDTH       (FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH*BIG_WND_SCALE)
#define FINANCE_BIG_WND_HEIGHT      ((FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT)*BIG_WND_SCALE)

#define BIG_WND_CORNER              10

static const char* string_item[6][2]={
    {"沪指","3164.42"},
    {"深指","20867.16"},
    {"中小板","1003.42"},
    {"恒指","12164.42"},
    {"道琼斯","9361.61"},
    {"纳斯达克","1998.72"},
};
static const char *string_content_left_block[]={
    "2831.08",
    "2712.87",
    "2591.66",
    "2410.45",
    "2341.24",
};
static const char *string_title_3_item[]={
    "沪深",
    "全球",
    "汇市",
    "期市",
};
static const char *string_content_bottom_block[]={
    "09:30",
    "09:30/13:00",
    "15:00",
};

//---- SVGUI_PRDRD_BLOCK_T
// finance background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[];
enum {
    FINANCE_PRDRD_BLOCK_BKGND_1,
    FINANCE_PRDRD_BLOCK_BKGND_2,
    FINANCE_PRDRD_BLOCK_BKGND_CONTENT,
    FINANCE_PRDRD_BLOCK_TITLE_1,
    FINANCE_PRDRD_BLOCK_TITLE_2,
    FINANCE_PRDRD_BLOCK_TITLE_3_SELECTED,
    FINANCE_PRDRD_BLOCK_TITLE_3_NOT_SELECTED,
    FINANCE_PRDRD_BLOCK_CONTENT_LEFT_BLOCK,
    FINANCE_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK,
    FINANCE_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK,
    FINANCE_PRDRD_BLOCK_SELECTED,
    FINANCE_PRDRD_BLOCK_NOT_SELECTED,
};

#define FINANCE_PRDRD_BLOCK_COLOR_BK   (0x000000)
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // FINANCE_PRDRD_BLOCK_BKGND_1
    {
        FINANCE_WND_WIDTH,  // width
        FINANCE_WND_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        0x000000, 0x000000, 0x000000,            // (1,1,1),(60,59,60),(1,1,1)
        0,                  //int border_width;
        0x000000,           //RGBCOLOR border_color;
        15,                 //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_BKGND_2
    {
        377,  // width
        653, // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, 326, 653,
        0x000000, 0xFFFFFF, 0x000000,            // (1,1,1),(60,59,60),(1,1,1)
        0,                  //int border_width;
        0x000000,           //RGBCOLOR border_color;
        0,                 //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_BKGND_CONTENT
    {
        BIG_WND_CORNER+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,  // width
        BIG_WND_CORNER+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        0x0066FF, 0x0066FF, 0x0066FF,            // (1,1,1),(60,59,60),(1,1,1)
        3,                          //int border_width;
        //0x672900, 0xFF6600, 0x672900,    //
        0x0066FF,               //RGBCOLOR border_color;
        //BIG_WND_CORNER,                     //int corner;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_TITLE_1
    {
        FINANCE_TITLE_1_WIDTH,     // width
        FINANCE_TITLE_1_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_TITLE_1_WIDTH*4/10, FINANCE_TITLE_1_WIDTH,
        0x919191, 0x090909, 0x0D0D0D,    //
        2,                          //int border_width;
        0x000000,               //RGBCOLOR border_color;
        14,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_TITLE_2
    {
        FINANCE_TITLE_2_WIDTH,     // width
        FINANCE_TITLE_2_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_TITLE_2_WIDTH*3/10, FINANCE_TITLE_2_WIDTH,
        0x585858, 0x191919, 0x000000,    //
        2,                          //int border_width;
        0x000000,               //RGBCOLOR border_color;
        5,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,   //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_TITLE_3_SELECTED
    {
        FINANCE_TITLE_3_ITEM_WIDTH,     // width
        FINANCE_TITLE_3_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_HORZ,// int gradient_type;
        0, FINANCE_TITLE_3_ITEM_HEIGHT/2, FINANCE_TITLE_3_ITEM_HEIGHT,
        0x672900, 0xFF6600, 0x672900,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        5,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,   //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_TITLE_3_NOT_SELECTED
    {
        FINANCE_TITLE_3_ITEM_WIDTH,     // width
        FINANCE_TITLE_3_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_TITLE_3_ITEM_WIDTH*3/10, FINANCE_TITLE_3_ITEM_WIDTH,
        0x585858, 0x191919, 0x000000,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        5,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,   //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_CONTENT_LEFT_BLOCK
    {
        FINANCE_CONTENT_LEFT_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_LEFT_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, FINANCE_CONTENT_LEFT_BLOCK_WIDTH/2, FINANCE_CONTENT_LEFT_BLOCK_WIDTH,
        0x101010, 0x101010, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK
    {
        FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_CONTENT_RIGHT_BLOCK_WIDTH/2, FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,
        0x606060, 0x303030, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK
    {
        FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH/2, FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH,
        0x101010, 0x101010, 0x101010,      //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_SELECTED
    {
        FINANCE_ITEM_WIDTH,     // width
        FINANCE_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_ITEM_WIDTH/2, FINANCE_ITEM_WIDTH,
        0x672900, 0xFF6600, 0x672900,    //
        0,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        5,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_PRDRD_BLOCK_NOT_SELECTED
    {
        FINANCE_ITEM_WIDTH,     // width
        FINANCE_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_ITEM_WIDTH*3/10, FINANCE_ITEM_WIDTH,
        0x585858, 0x191919, 0x101010,    //
        0,                          //int border_width;
        0x000000,               //RGBCOLOR border_color;
        5,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

#define FINANCE_COLOR_TEXT      (0xFFFFFF)
#define FINANCE_CONTENT_COLOR_TEXT  (0x000000)
//---- SVGUI_TEXT_AREA_T
static SVGUI_TEXT_AREA_T ptr_title_1_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, FINANCE_TITLE_1_WIDTH/2, FINANCE_TITLE_1_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        FINANCE_COLOR_TEXT,               // RGBCOLOR color;
        3,                      // int idx_font;
        "Finance",         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_title_2_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, FINANCE_TITLE_2_WIDTH, FINANCE_TITLE_2_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        FINANCE_COLOR_TEXT,               // RGBCOLOR color;
        2,                      // int idx_font;
        string_item[0][0],         // char* text;
    },
};

#define DEFINE_TITLE_3_ITEM_TEXT_AREA(num)    \
static SVGUI_TEXT_AREA_T ptr_title_3_item_block_text_areas_##num[]={\
    {\
        0,                      \
        TRUE,                   \
        {0, 0, FINANCE_TITLE_3_ITEM_WIDTH, FINANCE_TITLE_3_ITEM_HEIGHT},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        FINANCE_COLOR_TEXT,               \
        2,                      \
        string_title_3_item[num],         \
    },\
}
DEFINE_TITLE_3_ITEM_TEXT_AREA(0);
DEFINE_TITLE_3_ITEM_TEXT_AREA(1);
DEFINE_TITLE_3_ITEM_TEXT_AREA(2);
DEFINE_TITLE_3_ITEM_TEXT_AREA(3);

        //{0, num*20, 30, (1+num)*20},
#define DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(num) \
    {\
        num,                      \
        TRUE,                   \
        {0, num*(FINANCE_CONTENT_LEFT_BLOCK_HEIGHT/5), FINANCE_CONTENT_LEFT_BLOCK_WIDTH, (1+num)*(FINANCE_CONTENT_LEFT_BLOCK_HEIGHT/5)}, \
        SVGUI_TEXT_HALIGN_RIGHT | SVGUI_TEXT_VALIGN_CENTER,\
        FINANCE_COLOR_TEXT,               \
        1,                      \
        string_content_left_block[num],         \
    },

static SVGUI_TEXT_AREA_T ptr_content_left_block_text_areas[]={
DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(0)
DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(1)
DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(2)
DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(3)
DEFINE_CONTENT_LEFT_BLOCK_TEXT_AREAS(4)
};

#define DEFINE_CONTENT_BOTTOM_BLOCK_TEXT_AREAS(num, h_align) \
    {\
        num,                      \
        TRUE,                   \
        {0, 0, FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH, FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT},\
        h_align | SVGUI_TEXT_VALIGN_CENTER,\
        FINANCE_COLOR_TEXT,               \
        2,                      \
        string_content_bottom_block[num],         \
    },

static SVGUI_TEXT_AREA_T ptr_content_bottom_block_text_areas[]={\
    DEFINE_CONTENT_BOTTOM_BLOCK_TEXT_AREAS(0,SVGUI_TEXT_HALIGN_LEFT)
    DEFINE_CONTENT_BOTTOM_BLOCK_TEXT_AREAS(1,SVGUI_TEXT_HALIGN_CENTER)
    DEFINE_CONTENT_BOTTOM_BLOCK_TEXT_AREAS(2,SVGUI_TEXT_HALIGN_RIGHT)
};
#define DEFINE_ITEM_TEXT_AREAS(num)   \
static SVGUI_TEXT_AREA_T ptr_item_block_text_areas_##num[]={\
    {                           \
        0,                      \
        TRUE,                   \
        {0, 0, FINANCE_ITEM_WIDTH/3, FINANCE_ITEM_HEIGHT},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        FINANCE_COLOR_TEXT,     \
        2,                      \
        string_item[num][0],                 \
    },                          \
    {                           \
        1,                      \
        TRUE,                   \
        {FINANCE_ITEM_WIDTH/2, 0, FINANCE_ITEM_WIDTH, FINANCE_ITEM_HEIGHT},               \
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        FINANCE_COLOR_TEXT,     \
        2,                      \
        string_item[num][1],              \
    },\
}
DEFINE_ITEM_TEXT_AREAS(0);
DEFINE_ITEM_TEXT_AREAS(1);
DEFINE_ITEM_TEXT_AREAS(2);
DEFINE_ITEM_TEXT_AREAS(3);
DEFINE_ITEM_TEXT_AREAS(4);
DEFINE_ITEM_TEXT_AREAS(5);

//---- SVGUI_IMAGE
static SVGUI_IMAGE_T ptr_title_1_block_images[]={
    {
        78,                     // int widht;
        58,                     // int height;
        32,                      // int depth;
        "finance/finance_78x58.png",// char* file_name;
    },
    {
        98,                     // int widht;
        73,                     // int height;
        32,                      // int depth;
        "finance/finance_98x73.png",// char* file_name;
    },
    {
        123,                     // int widht;
        91,                     // int height;
        32,                      // int depth;
        "finance/finance_123x91.png",// char* file_name;
    },
};

//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T ptr_title_1_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_TITLE_1_WIDTH-3*FINANCE_TITLE_1_HEIGHT, 0, FINANCE_TITLE_1_WIDTH, 2*FINANCE_TITLE_1_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_title_1_block_images),                      // int nr_images;
        ptr_title_1_block_images,  // SVGUI_IMAGE* images;
    },
};

//---- SVGUI_BLOCK_T
enum {
#if 1
    FINANCE_BLOCK_BKGND_1=0,
#endif
    FINANCE_BLOCK_BKGND_2,
    FINANCE_BLOCK_TITLE_1,
    FINANCE_BLOCK_TITLE_2,
    FINANCE_BLOCK_TITLE_3_ITEM_0,
    FINANCE_BLOCK_TITLE_3_ITEM_1,
    FINANCE_BLOCK_TITLE_3_ITEM_2,
    FINANCE_BLOCK_TITLE_3_ITEM_3,
    FINANCE_BLOCK_CONTENT_LEFT,
    FINANCE_BLOCK_CONTENT_RIGHT,
    FINANCE_BLOCK_CONTENT_BOTTOM,
    FINANCE_BLOCK_ITEM_0,
    FINANCE_BLOCK_ITEM_1,
    FINANCE_BLOCK_ITEM_2,
    FINANCE_BLOCK_ITEM_3,
    FINANCE_BLOCK_ITEM_4,
    FINANCE_BLOCK_ITEM_5,
    FINANCE_MAX_BLOCK,
};
#if 0
static int finance_curr_item = FINANCE_BLOCK_ITEM_0;
        //{FINANCE_EMBED_WND_START_X+num*FINANCE_TITLE_3_ITEM_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT, FINANCE_EMBED_WND_START_X+(1+num)*FINANCE_TITLE_3_ITEM_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT}, 
#endif
#define DEFINE_TITLE_3_ITEM_BLOCK(num, idx_prdrd_block)   \
    {\
        FINANCE_BLOCK_TITLE_3_ITEM_##num,     \
        TRUE,               \
        {FINANCE_EMBED_WND_START_X+num*FINANCE_TITLE_3_ITEM_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT, FINANCE_EMBED_WND_START_X+(1+num)*FINANCE_TITLE_3_ITEM_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT}, \
\
        FALSE,               \
        idx_prdrd_block,\
\
        TABLESIZE(ptr_title_3_item_block_text_areas_##num),\
        ptr_title_3_item_block_text_areas_##num, \
\
        0,\
        NULL, \
    },
#define DEFINE_ITEM_BLOCK(num, idx_prdrd_block)   \
    {\
        FINANCE_BLOCK_ITEM_##num,     \
        TRUE,               \
        {FINANCE_ITEM_START_X, FINANCE_ITEM_START_Y+num*FINANCE_ITEM_HEIGHT, FINANCE_ITEM_START_X+FINANCE_ITEM_WIDTH, FINANCE_ITEM_START_Y+(1+num)*FINANCE_ITEM_HEIGHT}, \
\
        TRUE,               \
        idx_prdrd_block,\
\
        TABLESIZE(ptr_item_block_text_areas_##num),\
        ptr_item_block_text_areas_##num, \
\
        0,\
        NULL, \
    },
static SVGUI_BLOCK_T ptr_blocks[]={
#if 1
    {
        FINANCE_BLOCK_BKGND_1,     // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, FINANCE_WND_WIDTH, FINANCE_WND_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_BKGND_1,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
#endif
    {
        FINANCE_BLOCK_BKGND_2,     // int id;
        TRUE,                   // BOOL is_visible;
        {5, 30, 5+377, 20+653}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_BKGND_2,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_TITLE_1,     // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_EMBED_WND_START_X-1, 0, FINANCE_EMBED_WND_START_X+FINANCE_TITLE_1_WIDTH, FINANCE_TITLE_1_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_TITLE_1,// int idx_prdrd_block;
        TABLESIZE(ptr_title_1_block_text_areas),// int nr_text_areas;
        ptr_title_1_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_title_1_block_image_areas),  // int nr_image_areas;
        ptr_title_1_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_TITLE_2,     // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_EMBED_WND_START_X, FINANCE_TITLE_1_HEIGHT, FINANCE_EMBED_WND_START_X+FINANCE_TITLE_2_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_TITLE_2,// int idx_prdrd_block;
        TABLESIZE(ptr_title_2_block_text_areas),// int nr_text_areas;
        ptr_title_2_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    DEFINE_TITLE_3_ITEM_BLOCK(0, FINANCE_PRDRD_BLOCK_TITLE_3_SELECTED)
    DEFINE_TITLE_3_ITEM_BLOCK(1, FINANCE_PRDRD_BLOCK_TITLE_3_NOT_SELECTED)
    DEFINE_TITLE_3_ITEM_BLOCK(2, FINANCE_PRDRD_BLOCK_TITLE_3_NOT_SELECTED)
    DEFINE_TITLE_3_ITEM_BLOCK(3, FINANCE_PRDRD_BLOCK_TITLE_3_NOT_SELECTED)
    {
        FINANCE_BLOCK_CONTENT_LEFT,     // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_EMBED_WND_START_X, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT, FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_LEFT_BLOCK_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_CONTENT_LEFT_BLOCK,// int idx_prdrd_block;
        TABLESIZE(ptr_content_left_block_text_areas),// int nr_text_areas;
        ptr_content_left_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_CONTENT_RIGHT,     // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_LEFT_BLOCK_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT, FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_CONTENT_BOTTOM,     // int id;
        TRUE,                   // BOOL is_visible;
        {FINANCE_EMBED_WND_START_X, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT, FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH, FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK,// int idx_prdrd_block;
        TABLESIZE(ptr_content_bottom_block_text_areas),// int nr_text_areas;
        ptr_content_bottom_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },

    DEFINE_ITEM_BLOCK(0, FINANCE_PRDRD_BLOCK_SELECTED)
    DEFINE_ITEM_BLOCK(1, FINANCE_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(2, FINANCE_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(3, FINANCE_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(4, FINANCE_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(5, FINANCE_PRDRD_BLOCK_NOT_SELECTED)
};
//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_finance = {
    FINANCE_WND_WIDTH,  // width
    FINANCE_WND_HEIGHT, // height
    FINANCE_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    FINANCE_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_t),// nr_prdrd_blocks;
    ptr_prdrd_block_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;

    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks),// int nr_blocks;
    ptr_blocks,         // SVGUI_BLOCK_T* blocks;
};



//---- SVGUI_PRDRD_BLOCK_T
// finance big window background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_big_window_t[];
enum {
    FINANCE_BIG_WND_PRDRD_BLOCK_BKGND_CONTENT,
    FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_LEFT_BLOCK,
    FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK,
    FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK,
};

static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_big_window_t[]={
    // FINANCE_BIG_WND_PRDRD_BLOCK_BKGND_CONTENT
    {
        BIG_WND_CORNER+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,  // width
        BIG_WND_CORNER+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        0x0066FF, 0x0066FF, 0x0066FF,            // (1,1,1),(60,59,60),(1,1,1)
        BIG_WND_CORNER/2,                          //int border_width;
        0x0066FF,               //RGBCOLOR border_color;
        BIG_WND_CORNER,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_LEFT_BLOCK
    {
        FINANCE_CONTENT_LEFT_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_LEFT_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, FINANCE_CONTENT_LEFT_BLOCK_WIDTH/2, FINANCE_CONTENT_LEFT_BLOCK_WIDTH,
        0x101010, 0x101010, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK
    {
        FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, FINANCE_CONTENT_RIGHT_BLOCK_WIDTH/2, FINANCE_CONTENT_RIGHT_BLOCK_WIDTH,
        0x606060, 0x303030, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK
    {
        FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH,     // width
        FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH/2, FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH,
        0x101010, 0x101010, 0x101010,      //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        FINANCE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

static SVGUI_BLOCK_T ptr_blocks_big_wnd[]={
    {
        0,     // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH+BIG_WND_CORNER, FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT+BIG_WND_CORNER}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_BIG_WND_PRDRD_BLOCK_BKGND_CONTENT,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_CONTENT_LEFT,     // int id;
        TRUE,                   // BOOL is_visible;
        {BIG_WND_CORNER/2, BIG_WND_CORNER/2, BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_WIDTH, BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_LEFT_BLOCK,// int idx_prdrd_block;
        TABLESIZE(ptr_content_left_block_text_areas),// int nr_text_areas;
        ptr_content_left_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_CONTENT_RIGHT,     // int id;
        TRUE,                   // BOOL is_visible;
        {BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_WIDTH, BIG_WND_CORNER/2, BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH, BIG_WND_CORNER/2+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_RIGHT_BLOCK,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        FINANCE_BLOCK_CONTENT_BOTTOM,     // int id;
        TRUE,                   // BOOL is_visible;
        {BIG_WND_CORNER/2, BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT, BIG_WND_CORNER/2+FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH, BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        FINANCE_BIG_WND_PRDRD_BLOCK_CONTENT_BOTTOM_BLOCK,// int idx_prdrd_block;
        TABLESIZE(ptr_content_bottom_block_text_areas),// int nr_text_areas;
        ptr_content_bottom_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
};


//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_finance_big_wnd = {
    BIG_WND_CORNER+FINANCE_CONTENT_BOTTOM_BLOCK_WIDTH,  // width
    BIG_WND_CORNER+(FINANCE_CONTENT_LEFT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT), // height
    FINANCE_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    FINANCE_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_big_window_t),// nr_prdrd_blocks;
    ptr_prdrd_block_big_window_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;
    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks_big_wnd),// int nr_blocks;
    ptr_blocks_big_wnd,         // SVGUI_BLOCK_T* blocks;
};
// random between (-max_num ,  max_num)
static int random_under(int max_num){
    return (int)(2*max_num*(rand()*1.0/(RAND_MAX+1.0)-0.5));
}
static int OnMsgPaint(HDC hdc,RECT rc){
    int i;
    //SetBrushColor (hdc, PIXEL_lightwhite);
    float former_y = 0;
    if (!gushi) {
        gushi = (struct gushi_t*)malloc(4*RECTW(rc)+12);
        if(!gushi)
            error("gushi malloc failed!\n");
        former_y = (rand()*1.0/(RAND_MAX+1.0));
        gushi->first_pix = former_y;
        gushi->num_pixes = RECTW(rc);
        gushi->height_pixes = RECTH(rc);
        for(i=0; i<RECTW(rc); i+=1){
            int rdm = random_under(10);
            gushi->pixes[i] = rdm;
        }
    }

    int y = rc.top+gushi->first_pix*RECTH(rc);
    SetPenColor (hdc, RGB2Pixel(hdc,0xFF,0xFF,0xFF));
    MoveTo(hdc, rc.left, y);
    LineTo(hdc, rc.left, y);
    for(i=0; i<gushi->num_pixes; i+=1){
        float num = static_cast<float>RECTW(rc)/gushi->num_pixes; 
        float hei = static_cast<float>RECTW(rc)/gushi->height_pixes;
        int index =  static_cast<float>(i)*num;
        y = y + gushi->pixes[i]*hei;
        if(y > rc.bottom){
            y = rc.bottom-8;
        }else if(y < rc.top){
            y = rc.top+2;
        }
        LineTo(hdc, rc.left+index, y);
    }
    return 0;

}

static HWND InitFinanceBigWnd(void);

static BOOL set_window_seconddc(HWND hWnd, int   off_x, int      off_y,int   width,  int height)
{
    SVGUI_HEADER_T * header;
    gal_pixel pixel_bk;
    HDC sec_hdc;
    debug("************ set_window_seconddc  *******\n");
    debug("width=%d, height=%d\n", width, height);
    debug("off_x=%d, off_y=%d\n", off_x, off_y);
    //HDC hdc = alloc_subdc_from_uisec( off_x, off_y, width, height);
    sec_hdc = CreateSubMemDC(g_dc_uisec, off_x, off_y, width, height, TRUE);
    if (sec_hdc == HDC_INVALID){
        return FALSE;
    }
    SetSecondaryDC(hWnd, sec_hdc, ON_UPDSECDC_DEFAULT);
    header = (SVGUI_HEADER_T *)GetWindowAdditionalData(hWnd);
    if (header)
    {
        pixel_bk = RGB2Pixel (sec_hdc, 
                GetRValue (header->color_bk),
                GetGValue (header->color_bk),
                GetBValue (header->color_bk));
        SetBrushColor (sec_hdc, pixel_bk);
        FillBox (sec_hdc, 0, 0, width,  height);
    }
    return TRUE;
}

static SVGUI_BLOCK_I* GetSelectedBlock (HWND hWnd, int prdrd_idx_selected_item, int item_min_id, int item_max_id)
{

     SVGUI_BLOCK_I *block = NULL;
     SVGUI_HEADER_I *header = NULL;
     int hotpot_id;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return NULL;

     for (hotpot_id = item_min_id; hotpot_id<=item_max_id; hotpot_id++)
     {
       block = svgui_get_block(header, hotpot_id); 
       if (block == NULL)
           continue;
        
        if (block->idx_prdrd_block==prdrd_idx_selected_item)
            return block;
     }

     return NULL;
}

static void SetSelectedStatusByPos (HWND hWnd, int x_pos, int y_pos,int prdrd_idx_selected_item,int prdrd_idx_not_selected_item, int item_min_id, int item_max_id, int content_min_id)
{
    
    SVGUI_BLOCK_I *old_block = NULL;

    //int x_pos = LOWORD (lParam);
    //int y_pos = HIWORD (lParam);
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;


    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    //old_block = GetSelectedBlock(hWnd);

    debug (" xpos=%d, ypos=%d\n", x_pos, y_pos);

    old_block = GetSelectedBlock (hWnd, prdrd_idx_selected_item, item_min_id, item_max_id);
    if (block == NULL)
        return ;

    if (old_block != NULL)
    {
        old_block->idx_prdrd_block = prdrd_idx_not_selected_item;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }

    block->idx_prdrd_block=prdrd_idx_selected_item;
    InvalidateRect (hWnd, &block->rc, TRUE);
}
static void SetSelectedStatusByNextId (HWND hWnd, BOOL DOWN,int prdrd_idx_selected_item,int prdrd_idx_not_selected_item, int item_min_id, int item_max_id, int content_min_id)
{
    
    SVGUI_BLOCK_I *old_block = NULL;

    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;
    int nextid, idcount;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    old_block = GetSelectedBlock (hWnd, prdrd_idx_selected_item, item_min_id, item_max_id);

    idcount = item_max_id - item_min_id + 1;
    if (old_block != NULL)
    {
        if (DOWN)
            nextid = (old_block->id+1 - item_min_id)%(idcount) + item_min_id;
        else 
            nextid = (old_block->id-1 + idcount - item_min_id)%idcount + item_min_id;

        old_block->idx_prdrd_block = prdrd_idx_not_selected_item;
        //old_block->idx_prdrd_block = 3;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }
    else
    {
        if (DOWN)
            nextid = item_min_id;
        else
            nextid = item_max_id;
    }

    block = svgui_get_block (header, nextid);
    if (block == NULL)
        return;


    block->idx_prdrd_block=prdrd_idx_selected_item;
    InvalidateRect (hWnd, &block->rc, TRUE);
}
static void FinanceOnKeyupEnter(HWND hWnd)
{
    RECT src_rc_relative;
    RECT dst_rc;
    HWND hWnd_big_wnd;
    debug("Enter Key up()\n");
    switch(s_finance_wnd_status)
    {
        
        case WND_STATUS_SMALL:
            {
                hWnd_big_wnd = InitFinanceBigWnd();
                
#if HAVE_ANIMATION
                SetRect(&src_rc_relative, \
                        (FINANCE_EMBED_WND_START_X)*scale*scale_x, \
                        (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)*scale*scale_x, \
                        (FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH)*scale*scale_x, \
                        (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT)*scale*scale_x );
                GetWindowRect( hWnd_big_wnd,&dst_rc);
                dst_rc.left += BIG_WND_CORNER*scale*scale_x/2;
                dst_rc.right -= BIG_WND_CORNER*scale*scale_x/2;
                dst_rc.top += BIG_WND_CORNER*scale*scale_x/2;
                dst_rc.bottom -= BIG_WND_CORNER*scale*scale_x/2;
                AnimationZoomIn( hWnd, &src_rc_relative,  &dst_rc, 50, ANIMATION_FRAMES );
#endif
            }
            break;


        case WND_STATUS_LARGE:
            finance_big_status = 0;
#if HAVE_ANIMATION
            SetRect(&dst_rc, \
                    (FINANCE_WND_POS_X+FINANCE_EMBED_WND_START_X*scale)*scale_x, \
                    (FINANCE_WND_POS_Y+FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)*scale_y, \
                    (FINANCE_WND_POS_X+FINANCE_EMBED_WND_START_X*scale+FINANCE_CONTENT_WIDTH*scale)*scale_x, \
                    (FINANCE_WND_POS_Y+FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_HEIGHT)*scale_y );
            AnimationZoomOut( hWnd, s_hwnd_normal_wnd, &dst_rc, 50, ANIMATION_FRAMES );
#endif
          //  DestroyMainWindow (hWnd);
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            EnableWindow(s_hwnd_normal_wnd, TRUE);
            SetActiveWindow(s_hwnd_normal_wnd);
            InvalidateRect(s_hwnd_normal_wnd, NULL, TRUE);
            s_finance_wnd_status = WND_STATUS_SMALL;
            finance_status = 0;
            return;
        default:
            break;
    }
}

static void FinanceOnLButtonDown(HWND hWnd, LPARAM lParam)
{
    debug("press left button down on mouse!\n");
    RECT src_rc_relative;
    RECT dst_rc;

    SVGUI_BLOCK_I *old_block = NULL;
    SVGUI_BLOCK_I* block = NULL;


    SVGUI_HEADER_I *header = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    //block = svgui_get_block_by_point (header, x_pos, y_pos);
    block = svgui_get_block_by_point (header, LOWORD (lParam), HIWORD (lParam));
    if (block == NULL)
        return ;

    switch(block->id)
    {
        case FINANCE_BLOCK_TITLE_1:
            {
#if HAVE_ANIMATION
                //start close window effect
                AnimationMoveWndLeftRight( hWnd, finance_nomal_wnd_rect, MOVE_LEFT, ANIMATION_INTERVAL, ANIMATION_FRAMES );
#endif
                s_finance_wnd_status = WND_STATUS_NOT_DISPLAY;
                SendMessage(hWnd,MSG_CLOSE,0,0);
                InitToolbar(g_hMainWnd);
            }
            break;

        case FINANCE_BLOCK_CONTENT_RIGHT:
            {
                switch(s_finance_wnd_status){
                    case WND_STATUS_SMALL:
                        InitFinanceBigWnd();
#if HAVE_ANIMATION
                        SetRect(&src_rc_relative, \
                        (FINANCE_EMBED_WND_START_X)*scale*scale_x, \
                        (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)*scale*scale_x, \
                        (FINANCE_EMBED_WND_START_X+FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH)*scale*scale_x, \
                        (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT+FINANCE_CONTENT_BOTTOM_BLOCK_HEIGHT)*scale*scale_x );

                        dst_rc = finance_large_wnd_rect;
                        dst_rc.left += BIG_WND_CORNER*scale*scale_x/2;
                        dst_rc.right -= BIG_WND_CORNER*scale*scale_x/2;
                        dst_rc.top += BIG_WND_CORNER*scale*scale_x/2;
                        dst_rc.bottom -= BIG_WND_CORNER*scale*scale_x/2;
                        AnimationZoomIn( hWnd, &src_rc_relative,  &dst_rc, 50, ANIMATION_FRAMES );
#endif
                        break;

                    case WND_STATUS_LARGE:
#if HAVE_ANIMATION
                        SetRect(&dst_rc, \
                        (FINANCE_WND_POS_X+FINANCE_EMBED_WND_START_X*scale)*scale_x, \
                        (FINANCE_WND_POS_Y+FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)*scale_y, \
                        (FINANCE_WND_POS_X+FINANCE_EMBED_WND_START_X*scale+FINANCE_CONTENT_WIDTH*scale)*scale_x, \
                        (FINANCE_WND_POS_Y+FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT+FINANCE_CONTENT_HEIGHT)*scale_y );
                        AnimationZoomOut( hWnd, s_hwnd_normal_wnd, &dst_rc, 50, ANIMATION_FRAMES );
#endif
                        DestroyMainWindow (hWnd);
                        EnableWindow(s_hwnd_normal_wnd, TRUE);
                        SetActiveWindow(s_hwnd_normal_wnd);
                        InvalidateRect(s_hwnd_normal_wnd, NULL, TRUE);
                        s_finance_wnd_status = WND_STATUS_SMALL;
                        //InitFinanceWnd();
                        break;

                    default:
                        break;
                }//end switch(s_finance_wnd_status)
            }//end case FINANCE_BLOCK_CONTENT_RIGHT;
            break;

        case FINANCE_BLOCK_ITEM_0:
        case FINANCE_BLOCK_ITEM_1:
        case FINANCE_BLOCK_ITEM_2:
        case FINANCE_BLOCK_ITEM_3:
        case FINANCE_BLOCK_ITEM_4:
        case FINANCE_BLOCK_ITEM_5:
            {
                old_block = GetSelectedBlock (hWnd, FINANCE_PRDRD_BLOCK_SELECTED, FINANCE_BLOCK_ITEM_0, FINANCE_BLOCK_ITEM_5);
                if (old_block->id != block->id) {
                    if (gushi) {
                        free(gushi);
                        gushi = 0;
                    }
                }
                //SetSelectedStatusByPos(hWnd, x_pos, y_pos, FINANCE_PRDRD_BLOCK_SELECTED,FINANCE_PRDRD_BLOCK_NOT_SELECTED, FINANCE_BLOCK_ITEM_0, FINANCE_BLOCK_ITEM_5, 0);
                SetSelectedStatusByPos(hWnd, LOWORD (lParam), HIWORD (lParam), FINANCE_PRDRD_BLOCK_SELECTED,FINANCE_PRDRD_BLOCK_NOT_SELECTED, FINANCE_BLOCK_ITEM_0, FINANCE_BLOCK_ITEM_5, 0);
                InvalidateRect(hWnd, &finance_content_right_block_normal_wnd_rect, TRUE);
            }//end case FINANCE_BLOCK_ITEM_0 ~ FINANCE_BLOCK_ITEM_5;
            break;

        default:
            break;
    }// end switch(block->id)
}

static int FinanceWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC subdc_finance;
    HDC hdc;
    switch(message)
    {
        case MSG_CREATE:
            finance_big_status = 0;
            break;

        case MSG_PAINT:
            hdc = BeginPaint(hWnd);
            OnMsgPaint(hdc, finance_content_right_block_normal_wnd_rect);
            EndPaint (hWnd, hdc);
            break;

        case MSG_KEYUP:
            switch(wParam)
            {

                case SCANCODE_CURSORBLOCKUP:
                    {
                        if(gushi){
                            free(gushi);
                            gushi = 0;
                        }
                        SetSelectedStatusByNextId (hWnd, FALSE, FINANCE_PRDRD_BLOCK_SELECTED,FINANCE_PRDRD_BLOCK_NOT_SELECTED, FINANCE_BLOCK_ITEM_0, FINANCE_BLOCK_ITEM_5, 0);
                        InvalidateRect(hWnd, &finance_content_right_block_normal_wnd_rect, TRUE);
                        break;
                    }
                case SCANCODE_CURSORBLOCKDOWN:
                    {
                        if(gushi){
                            free(gushi);
                            gushi = 0;
                        }
                        SetSelectedStatusByNextId (hWnd, TRUE, FINANCE_PRDRD_BLOCK_SELECTED,FINANCE_PRDRD_BLOCK_NOT_SELECTED, FINANCE_BLOCK_ITEM_0, FINANCE_BLOCK_ITEM_5,0);
                        InvalidateRect(hWnd, &finance_content_right_block_normal_wnd_rect, TRUE);
                    }    
                    break;

                case SCANCODE_ENTER:
                    finance_status ++;
                    if(finance_status == 1 )
                        FinanceOnKeyupEnter(hWnd);
                    return 0;
                case SCANCODE_ESCAPE:
                    {
                        finance_big_status ++;
                        if(finance_big_status == 1)
                        {
#if HAVE_ANIMATION
                            //start close window effect
                            AnimationMoveWndLeftRight( hWnd, finance_nomal_wnd_rect, MOVE_LEFT, ANIMATION_INTERVAL, ANIMATION_FRAMES );
#endif

                            s_finance_wnd_status = WND_STATUS_NOT_DISPLAY;
                            SendMessage(hWnd,MSG_CLOSE,0,0);
                            InitToolbar(g_hMainWnd);
                        }
                    }
                    break;
                //case SCANCODE_CURSORBLOCKLEFT:
                case SCANCODE_CURSORBLOCKRIGHT:
                    return 0;
            }
            break;

        case MSG_LBUTTONDOWN:
            FinanceOnLButtonDown(hWnd, lParam);
            break;
        case MSG_CLOSE:
            s_hwnd_normal_wnd = NULL;
            if(gushi)
            {
                free(gushi);
                gushi = 0;
            }
            subdc_finance = GetSecondaryDC(hWnd);
            DeleteMemDC(subdc_finance);
            DestroyMainWindow(hWnd);
            break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}



static int FinanceBigWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC subdc_finance;
    HDC hdc;
    switch(message)
    {
        case MSG_PAINT:
            hdc = BeginPaint(hWnd);
            OnMsgPaint(hdc, finance_content_right_block_big_wnd_rect);
            EndPaint (hWnd, hdc);
            return 0;

        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_CURSORBLOCKUP:
                case SCANCODE_CURSORBLOCKDOWN:
                case SCANCODE_CURSORBLOCKRIGHT:
                    return 0;

                case SCANCODE_ENTER:
                    FinanceOnKeyupEnter(hWnd);
                    return 0;

                case SCANCODE_ESCAPE:
                    SendMessage(hWnd, MSG_KEYUP, SCANCODE_ENTER, 0);
                    return 0;
            }
            break;

        case MSG_LBUTTONDOWN:
            FinanceOnLButtonDown(hWnd, lParam);
            break;

        case MSG_CLOSE:
            subdc_finance = GetSecondaryDC(hWnd);
            DeleteMemDC(subdc_finance);
            DestroyMainWindow(hWnd);
            break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}

static HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc,const SVGUI_HEADER_T *svgui_header_t, HWND hHostingWnd )
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST|WS_EX_TROUNDCNS | WS_EX_BROUNDCNS ;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = hWndProc;
    CreateInfo.lx = rect->left;
    CreateInfo.ty = rect->top;
    CreateInfo.rx = rect->right;
    CreateInfo.by = rect->bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)svgui_header_t;

    CreateInfo.hHosting = hHostingWnd;

    return CreateMainWindow (&CreateInfo);
}

HWND InitFinanceWnd(void)
{
    HWND hFinanceWnd;
    static int is_initialized = 0;

    if( !is_initialized ){
        finance_nomal_wnd_rect.left = FINANCE_WND_POS_X*scale_x*scale;
        finance_nomal_wnd_rect.top  = FINANCE_WND_POS_Y*scale_x*scale;
        finance_nomal_wnd_rect.right  = (FINANCE_WND_POS_X+FINANCE_WND_WIDTH)*scale_x*scale;
        finance_nomal_wnd_rect.bottom = (FINANCE_WND_POS_Y+FINANCE_WND_HEIGHT)*scale_x*scale;

        finance_large_wnd_rect.left = (FINANCE_WND_POS_X+200)*scale_x;
        finance_large_wnd_rect.top  = (FINANCE_WND_POS_Y+200)*scale_y;
        finance_large_wnd_rect.right  = finance_large_wnd_rect.left+(BIG_WND_CORNER*BIG_WND_SCALE+FINANCE_BIG_WND_WIDTH)*scale_x*scale;

        if ( scale_y/scale_x > 1.0 ) {
            finance_large_wnd_rect.bottom = finance_large_wnd_rect.top + (int)(BIG_WND_CORNER*BIG_WND_SCALE*scale_y*(1.0*FINANCE_BIG_WND_HEIGHT/FINANCE_BIG_WND_WIDTH)) + (int)( (finance_large_wnd_rect.right - finance_large_wnd_rect.left)*(1.0*FINANCE_BIG_WND_HEIGHT/FINANCE_BIG_WND_WIDTH) );
        } else {
            finance_large_wnd_rect.bottom = finance_large_wnd_rect.top + (BIG_WND_CORNER*BIG_WND_SCALE + FINANCE_BIG_WND_HEIGHT)*scale_y;
        }
        finance_content_right_block_normal_wnd_rect.left   = (FINANCE_CONTENT_LEFT_BLOCK_WIDTH+FINANCE_EMBED_WND_START_X)*scale*scale_x;
        finance_content_right_block_normal_wnd_rect.right   = finance_content_right_block_normal_wnd_rect.left+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH*scale*scale_x;
        finance_content_right_block_normal_wnd_rect.top    = (FINANCE_TITLE_1_HEIGHT+FINANCE_TITLE_2_HEIGHT+FINANCE_TITLE_3_HEIGHT)*scale*scale_x;
        finance_content_right_block_normal_wnd_rect.bottom    = finance_content_right_block_normal_wnd_rect.top+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT*scale*scale_x;

        finance_content_right_block_big_wnd_rect.left   = (BIG_WND_CORNER/2+FINANCE_CONTENT_LEFT_BLOCK_WIDTH)*BIG_WND_SCALE*scale_x*scale;
        finance_content_right_block_big_wnd_rect.right   = finance_content_right_block_big_wnd_rect.left+FINANCE_CONTENT_RIGHT_BLOCK_WIDTH*BIG_WND_SCALE*scale_x*scale;
        finance_content_right_block_big_wnd_rect.top    = BIG_WND_CORNER*BIG_WND_SCALE*scale_x*scale/2;
        finance_content_right_block_big_wnd_rect.bottom    = finance_content_right_block_big_wnd_rect.top+FINANCE_CONTENT_RIGHT_BLOCK_HEIGHT*BIG_WND_SCALE*scale_x*scale;
    }

    debug(" ******** InitFinanceWnd():   start!\n");

    hFinanceWnd = CreateMenuWnd( &finance_nomal_wnd_rect, FinanceWndProc, &svgui_header_t_finance, HWND_DESKTOP);
    if (hFinanceWnd == HWND_INVALID){
        error("Create toolbar window!\n");
        return HWND_INVALID;
    }

    set_window_seconddc(hFinanceWnd, finance_nomal_wnd_rect.left, finance_nomal_wnd_rect.top, RECTW(finance_nomal_wnd_rect),  RECTH(finance_nomal_wnd_rect) );

#if HAVE_ANIMATION
    if(WND_STATUS_NOT_DISPLAY == s_finance_wnd_status){
        //start create window effect
        AnimationMoveWndLeftRight( hFinanceWnd, finance_nomal_wnd_rect, MOVE_RIGHT, ANIMATION_INTERVAL, ANIMATION_FRAMES );
    }
#endif

    InvalidateRect(hFinanceWnd, &finance_nomal_wnd_rect, TRUE);
    ShowWindow (hFinanceWnd, SW_SHOWNORMAL);
    s_finance_wnd_status = WND_STATUS_SMALL;
    s_hwnd_normal_wnd = hFinanceWnd;
    return hFinanceWnd;
}

static HWND InitFinanceBigWnd(void)
{
    HWND hFinanceWnd = HWND_NULL;
    SVGUI_HEADER_I* svgui_header_i;
    HDC hdc_secondary;
    debug(" ******** InitFinanceBigWnd():   start!\n");

    if(!s_hwnd_normal_wnd)
    {
        return NULL;
    }

    hFinanceWnd = CreateMenuWnd( &finance_large_wnd_rect, FinanceBigWndProc, &svgui_header_t_finance_big_wnd, s_hwnd_normal_wnd);
     
    if (hFinanceWnd == HWND_INVALID){
        error("Create toolbar window!\n");
        return HWND_INVALID;
    }

    // x pos of secondary dc of large window is normal window's right avoid intersecting
    set_window_seconddc(hFinanceWnd, finance_nomal_wnd_rect.right, finance_large_wnd_rect.top, RECTW(finance_large_wnd_rect),  RECTH(finance_large_wnd_rect) );
    
    EnableWindow(s_hwnd_normal_wnd, FALSE);

    InvalidateRect(hFinanceWnd, &finance_large_wnd_rect, TRUE);
    ShowWindow (hFinanceWnd, SW_SHOWNORMAL);
    s_finance_wnd_status = WND_STATUS_LARGE;

    EnableWindow(s_hwnd_normal_wnd, FALSE);
    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hFinanceWnd); // hWnd
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hFinanceWnd );
    if(svgui_header_i == NULL){
        fprintf(stderr, "finance: svgui_header_i is NULL\n");
    }else{
        svgui_draw ( svgui_header_i, hdc_secondary, NULL);
    }
    return hFinanceWnd;
}
