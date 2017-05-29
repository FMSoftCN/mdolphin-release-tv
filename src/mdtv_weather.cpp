#include <string.h>

#include "svgui.h"
#include "mdtv_app.h"
#include "mdtv_common.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_weather.h"
#include "mdtv_animation.h"
#include "mdtv_toolbar.h"

#define HAVE_ANIMATION      1

#define NDEBUG  1
#ifndef NDEBUG
#define DEBUG_TRACE(P) {fprintf (stderr, "FIXME: %s,%d,%s: %s\n", __FILE__, __LINE__, __FUNCTION__ ,(P));}

#define error(fmt...) fprintf (stderr, "mdtv_weather[ERROR]:"fmt)
#define debug(fmt...) fprintf (stderr, "mdtv_weather[DEBUG]:"fmt)

#else
#define DEBUG_TRACE(P) 
#define error(fmt...) fprintf (stderr, "mdtv_weather[ERROR]:"fmt)
#define debug(fmt...)
#endif


#define WEATHER_EMBED_WND_X_OFFSET      (7)

#define WEATHER_WND_WIDTH           (388)
#define WEATHER_WND_HEIGHT          (682)

#define WEATHER_EMBED_WND_START_X   (WEATHER_EMBED_WND_X_OFFSET)

#define WEATHER_TITLE_1_WIDTH       (379)
#define WEATHER_TITLE_1_HEIGHT      (50)
#define WEATHER_TITLE_2_WIDTH       (373)
#define WEATHER_TITLE_2_HEIGHT      (36)
#define WEATHER_ARROW_WND_WIDTH     (41)
#define WEATHER_ARROW_WND_HEIGHT    (222)
#define WEATHER_CONTENT_WIDTH        (WEATHER_WND_WIDTH-2*WEATHER_EMBED_WND_X_OFFSET-2*WEATHER_ARROW_WND_WIDTH)
#define WEATHER_CONTENT_HEIGHT       (WEATHER_ARROW_WND_HEIGHT)

#define WEATHER_ITEM_WIDTH          (373)
//#define WEATHER_ITEM_WIDTH          (WEATHER_WND_WIDTH-2*WEATHER_EMBED_WND_X_OFFSET)
#define WEATHER_ITEM_HEIGHT         ((WEATHER_WND_HEIGHT-WEATHER_TITLE_1_HEIGHT-WEATHER_TITLE_2_HEIGHT-WEATHER_ARROW_WND_HEIGHT-20)/6)
//#define WEATHER_ITEM_HEIGHT         (50)

#define WEATHER_BIG_WND_BKGND_1_WIDTH   (WEATHER_WND_WIDTH)
#define WEATHER_BIG_WND_BKGND_1_HEIGHT  (WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_ARROW_WND_HEIGHT+10)
#define WEATHER_BIG_WND_BKGND_2_WIDTH   (WEATHER_WND_WIDTH-9)
#define WEATHER_BIG_WND_BKGND_2_HEIGHT  (WEATHER_BIG_WND_BKGND_1_HEIGHT-31)
#define BIG_WND_SCALE               (1.6)
#define WEATHER_BIG_WND_WIDTH       (WEATHER_CONTENT_WIDTH*BIG_WND_SCALE)
#define WEATHER_BIG_WND_HEIGHT      (WEATHER_CONTENT_HEIGHT*BIG_WND_SCALE)


#define WEATHER_ITEM_START_X        (WEATHER_EMBED_WND_START_X)
#define WEATHER_ITEM_START_Y        (WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_CONTENT_HEIGHT)

#define WEATHER_WND_POS_X           (51)
#define WEATHER_WND_POS_Y           (20)
#define WEATHER_LEFTSPACE_PERCENT   (1.0*WEATHER_WND_POS_X/DEFAULT_SCREEN_WIDTH)
#define WEATHER_TOPSPACE_PERCENT    (1.0*WEATHER_WND_POS_Y/DEFAULT_SCREEN_HEIGHT)
#define WEATHER_WND_WIDTH_PERCENT   (1.0*WEATHER_WND_WIDTH/DEFAULT_SCREEN_WIDTH)
#define WEATHER_WND_HEIGHT_PERCENT  (1.0*WEATHER_WND_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define WEATHER_LARGE_WND_WIDTH_PERCENT   (1.3*WEATHER_WND_WIDTH_PERCENT)
#define WEATHER_LARGE_WND_HEIGHT_PERCENT  (1.3*WEATHER_WND_HEIGHT_PERCENT)

#define BIG_WND_CORNER               10

static HWND s_hwnd_normal_wnd = NULL;
static RECT weather_nomal_wnd_rect;
static RECT weather_large_wnd_rect;
typedef enum _WND_STATUS{
    WND_STATUS_NOT_DISPLAY,
    WND_STATUS_SMALL,
    WND_STATUS_LARGE,
}WND_STATUS;
static WND_STATUS s_weather_wnd_status = WND_STATUS_NOT_DISPLAY;
static int weather_status = 0;
static int weather_big_status = 0;

static const char* string_content[6][8]={
    {"北京","8月7日(周五)","小雨","27-14度","湿度:37%","气压:105hpa","风力:东北二级","能见度:15km"},
    {"香港","8月7日(周五)","大雨","37-24度","湿度:27%","气压:104hpa","风力:北二级","能见度:10km"},
    {"纽约","8月7日(周五)","晴","17-4度","湿度:17%","气压:103hpa","风力:南二级","能见度:20km"},
    {"伦敦","8月7日(周五)","阴","37-24度","湿度:37%","气压:102hpa","风力:西南二级","能见度:25km"},
    {"巴黎","8月7日(周五)","晴转多云","37-24度","湿度:27%","气压:105hpa","风力:东南二级","能见度:15km"},
    {"东京","8月7日(周五)","小雨","37-24度","湿度:17%","气压:100hpa","风力:西北二级","能见度:15km"},
};
static const char *item_pic_path_0[6]={
    "weather/light_rain_29x35.png",
    "weather/heavy_rain_30x34.png",
    "weather/sunny_29x30.png",
    "weather/cloudy_31x29.png",
    "weather/sun_cloud_30x31.png",
    "weather/light_rain_29x35.png",
};
static const char *item_pic_path_1[6]={
    "weather/light_rain_36x43.png",
    "weather/heavy_rain_38x43.png",
    "weather/sunny_37x37.png",
    "weather/cloudy_37x36.png",
    "weather/sun_cloud_38x40.png",
    "weather/light_rain_36x43.png",
};
static const char *item_pic_path_2[6]={
    "weather/light_rain_48x55.png",
    "weather/heavy_rain_50x54.png",
    "weather/sunny_47x48.png",
    "weather/cloudy_48x46.png",
    "weather/sun_cloud_48x50.png",
    "weather/light_rain_48x55.png",
};
static const char *content_pic_path_0[6]={
    "weather/light_rain_84x113.png",
    "weather/heavy_rain_84x114.png",
    "weather/sunny_114x115.png",
    "weather/cloudy_114x94.png",
    "weather/sun_cloud_114x115.png",
    "weather/light_rain_84x113.png",
};
static const char *content_pic_path_1[6]={
    "weather/light_rain_123x160.png",
    "weather/heavy_rain_121x159.png",
    "weather/sunny_156x158.png",
    "weather/cloudy_159x133.png",
    "weather/sun_cloud_156x158.png",
    "weather/light_rain_123x160.png",
};

//---- SVGUI_PRDRD_BLOCK_T
// toolbar background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[];
enum {
    WEATHER_PRDRD_BLOCK_BKGND_1,
    WEATHER_PRDRD_BLOCK_BKGND_2,
    WEATHER_PRDRD_BLOCK_BKGND_CONTENT,
    WEATHER_PRDRD_BLOCK_TITLE_1,
    WEATHER_PRDRD_BLOCK_TITLE_2,
    WEATHER_PRDRD_BLOCK_LEFT_ARROW,
    WEATHER_PRDRD_BLOCK_RIGHT_ARROW,
    WEATHER_PRDRD_BLOCK_CONTENT,
    WEATHER_PRDRD_BLOCK_SELECTED,
    WEATHER_PRDRD_BLOCK_NOT_SELECTED,
};
#define WEATHER_PRDRD_BLOCK_COLOR_BK   (0x000000)
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // WEATHER_PRDRD_BLOCK_BKGND_1
    {
        WEATHER_WND_WIDTH,  // width
        WEATHER_WND_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        0x000000, 0x000000, 0x000000,            // (1,1,1),(60,59,60),(1,1,1)
        0,                  //int border_width;
        0x000000,           //RGBCOLOR border_color;
        15,                 //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_BKGND_2
    {
        WEATHER_WND_WIDTH-11,  // width
        (WEATHER_WND_HEIGHT-29), // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, (WEATHER_WND_HEIGHT-29)/2, (WEATHER_WND_HEIGHT-29),
        0x101010, 0xFFFFFF, 0x000000,            // (1,1,1),(60,59,60),(1,1,1)
        0,                  //int border_width;
        0x000000,           //RGBCOLOR border_color;
        0,                 //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_BKGND_CONTENT
    {
        BIG_WND_CORNER+WEATHER_CONTENT_WIDTH,  // width
        BIG_WND_CORNER+WEATHER_CONTENT_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        //0x672900, 0x672900, 0x672900,            // (1,1,1),(60,59,60),(1,1,1)
        0x0066FF, 0x0066FF, 0x0066FF,            // (1,1,1),(60,59,60),(1,1,1)
        3,                          //int border_width;
        //0x672900, 0xFF6600, 0x672900,    //
        0x0066FF,               //RGBCOLOR border_color;
        BIG_WND_CORNER,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_TITLE_1
    {
        WEATHER_TITLE_1_WIDTH,     // width
        WEATHER_TITLE_1_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, WEATHER_TITLE_1_WIDTH*4/10, WEATHER_TITLE_1_WIDTH,
        0x919191, 0x090909, 0x0D0D0D,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        14,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_TITLE_2
    {
        WEATHER_TITLE_2_WIDTH,     // width
        WEATHER_TITLE_2_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, WEATHER_TITLE_2_WIDTH*3/10, WEATHER_TITLE_2_WIDTH,
        0x585858, 0x191919, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        5,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,   //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_LEFT_ARROW
    {
        WEATHER_ARROW_WND_WIDTH,     // width
        WEATHER_ARROW_WND_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_HORZ,// int gradient_type;
        0, WEATHER_ARROW_WND_HEIGHT/2, WEATHER_ARROW_WND_HEIGHT,
        0x727272, 0x2F2F2F, 0x101010,    //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_RIGHT_ARROW
    {
        WEATHER_ARROW_WND_WIDTH,     // width
        WEATHER_ARROW_WND_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_HORZ,// int gradient_type;
        0, WEATHER_ARROW_WND_HEIGHT/2, WEATHER_ARROW_WND_HEIGHT,
        0x101010, 0x2F2F2F, 0x727272,      //
        2,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        0,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_CONTENT
    {
        WEATHER_CONTENT_WIDTH,     // width
        WEATHER_CONTENT_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, WEATHER_CONTENT_HEIGHT/2, WEATHER_CONTENT_HEIGHT,
        0x727272, 0x2F2F2F, 0x101010,    //
        0,                          //int border_width;
        0x727272,               //RGBCOLOR border_color;
        0,                     //int corner;
        0xFFFF,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_SELECTED
    {
        WEATHER_ITEM_WIDTH,     // width
        WEATHER_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, WEATHER_ITEM_WIDTH/2, WEATHER_ITEM_WIDTH,
        0x672900, 0xFF6600, 0x672900,    //
        //0xFFFFFF, 0xFFFFFF, 0xFFFFFF,    //
        0,                          //int border_width;
        0x452222,               //RGBCOLOR border_color;
        5,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // WEATHER_PRDRD_BLOCK_NOT_SELECTED
    {
        WEATHER_ITEM_WIDTH,     // width
        WEATHER_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, WEATHER_ITEM_WIDTH*3/10, WEATHER_ITEM_WIDTH,
        0x585858, 0x191919, 0x101010,    //
        0,                          //int border_width;
        0x000000,               //RGBCOLOR border_color;
        5,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

#define WEATHER_COLOR_TEXT      (0xFFFFFF)
#define WEATHER_CONTENT_COLOR_TEXT  (0x000000)
//---- SVGUI_TEXT_AREA_T
static SVGUI_TEXT_AREA_T ptr_title_1_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_TITLE_1_WIDTH/2, WEATHER_TITLE_1_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        WEATHER_COLOR_TEXT,               // RGBCOLOR color;
        3,                      // int idx_font;
        "Weather",         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_title_2_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_TITLE_2_WIDTH, WEATHER_TITLE_2_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        WEATHER_COLOR_TEXT,               // RGBCOLOR color;
        2,                      // int idx_font;
        "今天",         // char* text;
    },
};

#define WEATHER_CONTENT_X               (WEATHER_CONTENT_WIDTH/2)
#define WEATHER_CONTENT_CITY_Y          (WEATHER_CONTENT_HEIGHT/4)
#define WEATHER_CONTENT_DATE_Y          (WEATHER_CONTENT_CITY_Y+WEATHER_CONTENT_HEIGHT/8)
#define WEATHER_CONTENT_WEATHER_Y       (WEATHER_CONTENT_DATE_Y+WEATHER_CONTENT_HEIGHT/5)
#define WEATHER_CONTENT_TEMPARATURE_Y   (WEATHER_CONTENT_WEATHER_Y+WEATHER_CONTENT_HEIGHT/6)
#define WEATHER_CONTENT_HUMIDITY_Y      (WEATHER_CONTENT_TEMPARATURE_Y+WEATHER_CONTENT_HEIGHT/10)
#define WEATHER_CONTENT_BAROMETER_Y     (WEATHER_CONTENT_HEIGHT)

#define WEATHER_CONTENT_WIND_PICTURE_Y  (WEATHER_CONTENT_HEIGHT*(1.0/4+1.0/8+1.0/5+1.0/6))
#define WEATHER_CONTENT_WIND_Y          (WEATHER_CONTENT_WIND_PICTURE_Y+WEATHER_CONTENT_HEIGHT*1.0/10)

#define DEFINE_CONTENT_TEXT_AREA(num)    \
static SVGUI_TEXT_AREA_T ptr_content_block_text_areas_##num[]={\
    {\
        0,                      \
        TRUE,                   \
        {0, 0, WEATHER_CONTENT_X, WEATHER_CONTENT_CITY_Y},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        3,                      \
        string_content[num][0],         \
    },\
    {\
        1,                      \
        TRUE,                   \
        {0, WEATHER_CONTENT_CITY_Y, WEATHER_CONTENT_X, WEATHER_CONTENT_DATE_Y},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][1],         \
    },\
    {\
        2,                      \
        TRUE,                   \
        {0, WEATHER_CONTENT_DATE_Y, WEATHER_CONTENT_X, WEATHER_CONTENT_WEATHER_Y},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        3,                      \
        string_content[num][2],         \
    },\
    {\
        3,                      \
        TRUE,                   \
        {0, WEATHER_CONTENT_WEATHER_Y, WEATHER_CONTENT_X, WEATHER_CONTENT_TEMPARATURE_Y},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][3],         \
    },\
    {\
        4,                      \
        TRUE,                   \
        {WEATHER_CONTENT_WIDTH/100, WEATHER_CONTENT_TEMPARATURE_Y, WEATHER_CONTENT_X, WEATHER_CONTENT_HUMIDITY_Y},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][4],         \
    },\
    {\
        5,                      \
        TRUE,                   \
        {WEATHER_CONTENT_WIDTH/100, WEATHER_CONTENT_HUMIDITY_Y, WEATHER_CONTENT_X, WEATHER_CONTENT_BAROMETER_Y},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][5],         \
    },\
    {\
        6,                      \
        TRUE,                   \
        {WEATHER_CONTENT_X, WEATHER_CONTENT_WIND_PICTURE_Y, WEATHER_CONTENT_WIDTH, WEATHER_CONTENT_WIND_Y},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][6],         \
    },\
    {\
        7,                      \
        TRUE,                   \
        {WEATHER_CONTENT_X, WEATHER_CONTENT_WIND_Y, WEATHER_CONTENT_WIDTH, WEATHER_CONTENT_HEIGHT},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_CONTENT_COLOR_TEXT,               \
        2,                      \
        string_content[num][7],         \
    },\
}
DEFINE_CONTENT_TEXT_AREA(0);
DEFINE_CONTENT_TEXT_AREA(1);
DEFINE_CONTENT_TEXT_AREA(2);
DEFINE_CONTENT_TEXT_AREA(3);
DEFINE_CONTENT_TEXT_AREA(4);
DEFINE_CONTENT_TEXT_AREA(5);

#define WEATHER_ITEM_CITY_TEXT_WIDTH           (WEATHER_ITEM_WIDTH/3)
#define WEATHER_ITEM_PIC_WIDTH            (WEATHER_ITEM_WIDTH/3)
#define WEATHER_ITEM_TEMPARATURE_TEXT_WIDTH    (WEATHER_ITEM_WIDTH/3)

#define DEFINE_ITEM_TEXT_AREAS(num)   \
static SVGUI_TEXT_AREA_T ptr_item_block_text_areas_##num[]={\
    {                           \
        0,                      \
        TRUE,                   \
        {0, 0, WEATHER_ITEM_CITY_TEXT_WIDTH, WEATHER_ITEM_HEIGHT},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_COLOR_TEXT,     \
        2,                      \
        string_content[num][0],                 \
    },                          \
    {                           \
        1,                      \
        TRUE,                   \
        {WEATHER_ITEM_CITY_TEXT_WIDTH+WEATHER_ITEM_PIC_WIDTH, 0, WEATHER_ITEM_WIDTH, WEATHER_ITEM_HEIGHT},               \
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        WEATHER_COLOR_TEXT,     \
        2,                      \
        string_content[num][3],              \
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
    { 78, 58, 32, "weather/sun_78x58.png", },
    { 98, 73, 32, "weather/sun_98x73.png", },
    { 123,91, 32, "weather/sun_123x91.png",},
};
static SVGUI_IMAGE_T ptr_left_arrow_block_images[]={
    { 14, 26, 32, "weather/left_arrow_14x26.png", },
    { 17, 31, 32, "weather/left_arrow_17x31.png", },
    { 23, 40, 32, "weather/left_arrow_23x40.png", },
};
static SVGUI_IMAGE_T ptr_right_arrow_block_images[]={
    { 14, 26, 32, "weather/right_arrow_14x26.png", },
    { 17, 31, 32, "weather/right_arrow_17x31.png", },
    { 23, 40, 32, "weather/right_arrow_23x40.png", },
};

#define DEFINE_CONTENT_IMAGE(num)   \
static SVGUI_IMAGE_T ptr_content_block_images_##num[]={\
    {\
        29,                     \
        35,                     \
        32,                      \
        item_pic_path_0[num],\
    },\
    {\
        36,                     \
        43,                     \
        32,                      \
        item_pic_path_1[num],\
    },\
    {\
        48,                     \
        55,                     \
        32,                      \
        item_pic_path_2[num],\
    },\
    {\
        114,                     \
        115,                     \
        32,                      \
        content_pic_path_0[num],\
    },\
    {\
        156,                     \
        158,                     \
        32,                      \
        content_pic_path_1[num],\
    },\
};
DEFINE_CONTENT_IMAGE(0);
DEFINE_CONTENT_IMAGE(1);
DEFINE_CONTENT_IMAGE(2);
DEFINE_CONTENT_IMAGE(3);
DEFINE_CONTENT_IMAGE(4);
DEFINE_CONTENT_IMAGE(5);

#define DEFINE_ITEM_IMAGE(num)   \
static SVGUI_IMAGE_T ptr_item_block_images_##num[]={\
    {\
        29,                     \
        35,                     \
        32,                      \
        item_pic_path_0[num],\
    },\
    {\
        36,                     \
        43,                     \
        32,                      \
        item_pic_path_1[num],\
    },\
    {\
        48,                     \
        55,                     \
        32,                      \
        item_pic_path_2[num],\
    },\
};
DEFINE_ITEM_IMAGE(0);
DEFINE_ITEM_IMAGE(1);
DEFINE_ITEM_IMAGE(2);
DEFINE_ITEM_IMAGE(3);
DEFINE_ITEM_IMAGE(4);
DEFINE_ITEM_IMAGE(5);

//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T ptr_title_1_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {WEATHER_TITLE_1_WIDTH-3*WEATHER_TITLE_1_HEIGHT, 0, WEATHER_TITLE_1_WIDTH, 2*WEATHER_TITLE_1_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_title_1_block_images),                      // int nr_images;
        ptr_title_1_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_left_arrow_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_ARROW_WND_WIDTH, WEATHER_ARROW_WND_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_left_arrow_block_images),                      // int nr_images;
        ptr_left_arrow_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_right_arrow_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_ARROW_WND_WIDTH, WEATHER_ARROW_WND_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_right_arrow_block_images),                      // int nr_images;
        ptr_right_arrow_block_images,  // SVGUI_IMAGE* images;
    },
};

#define DEFINE_CONTENT_IMAGE_AREA(num)   \
static SVGUI_IMAGE_AREA_T ptr_content_block_image_areas_##num[]={   \
    {\
        0,                      \
        TRUE,                   \
        {WEATHER_CONTENT_WIDTH/2, 0, WEATHER_CONTENT_WIDTH, WEATHER_CONTENT_HEIGHT*2/3},\
        SVGUI_IMAGE_FILL_WAY_CENTER, \
        \
        TABLESIZE(ptr_content_block_images_##num),                      \
        ptr_content_block_images_##num,  \
    },\
}
DEFINE_CONTENT_IMAGE_AREA(0);
DEFINE_CONTENT_IMAGE_AREA(1);
DEFINE_CONTENT_IMAGE_AREA(2);
DEFINE_CONTENT_IMAGE_AREA(3);
DEFINE_CONTENT_IMAGE_AREA(4);
DEFINE_CONTENT_IMAGE_AREA(5);

#define DEFINE_ITEM_IMAGE_AREAS(num) \
static SVGUI_IMAGE_AREA_T ptr_item_block_image_areas_##num[]={\
    {                           \
        0,                      \
        TRUE,                   \
        {(WEATHER_ITEM_WIDTH-WEATHER_ITEM_HEIGHT+10)/2, 0, (WEATHER_ITEM_WIDTH+WEATHER_ITEM_HEIGHT+10)/2, WEATHER_ITEM_HEIGHT},\
        SVGUI_IMAGE_FILL_WAY_CENTER, \
        \
        TABLESIZE(ptr_item_block_images_##num),                      \
        ptr_item_block_images_##num,  \
    },\
}
DEFINE_ITEM_IMAGE_AREAS(0);
DEFINE_ITEM_IMAGE_AREAS(1);
DEFINE_ITEM_IMAGE_AREAS(2);
DEFINE_ITEM_IMAGE_AREAS(3);
DEFINE_ITEM_IMAGE_AREAS(4);
DEFINE_ITEM_IMAGE_AREAS(5);

//---- SVGUI_BLOCK_T
enum {
    WEATHER_BLOCK_BKGND_1=0,
    WEATHER_BLOCK_BKGND_2,
    WEATHER_BLOCK_TITLE_1,
    WEATHER_BLOCK_TITLE_2,
    WEATHER_BLOCK_LEFT_ARROW,
    WEATHER_BLOCK_RIGHT_ARROW,
    WEATHER_BLOCK_CONTENT_0,
    WEATHER_BLOCK_CONTENT_1,
    WEATHER_BLOCK_CONTENT_2,
    WEATHER_BLOCK_CONTENT_3,
    WEATHER_BLOCK_CONTENT_4,
    WEATHER_BLOCK_CONTENT_5,
    WEATHER_BLOCK_ITEM_0,
    WEATHER_BLOCK_ITEM_1,
    WEATHER_BLOCK_ITEM_2,
    WEATHER_BLOCK_ITEM_3,
    WEATHER_BLOCK_ITEM_4,
    WEATHER_BLOCK_ITEM_5,
    WEATHER_MAX_BLOCK,
};
static int weather_curr_item = WEATHER_BLOCK_ITEM_0;

#define DEFINE_CONTENT_BLOCK(num,is_visible)   \
    {\
        WEATHER_BLOCK_CONTENT_##num,     \
        is_visible,                   \
        {WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT, WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH+WEATHER_CONTENT_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_CONTENT_HEIGHT}, \
\
        TRUE,               \
        WEATHER_PRDRD_BLOCK_CONTENT,\
\
        TABLESIZE(ptr_content_block_text_areas_##num),\
        ptr_content_block_text_areas_##num, \
\
        TABLESIZE(ptr_content_block_image_areas_##num),              \
        ptr_content_block_image_areas_##num, \
    },

#define DEFINE_ITEM_BLOCK(num, idx_prdrd_block)   \
    {\
        WEATHER_BLOCK_ITEM_##num,     \
        TRUE,               \
        {WEATHER_ITEM_START_X, WEATHER_ITEM_START_Y+num*WEATHER_ITEM_HEIGHT, WEATHER_ITEM_START_X+WEATHER_ITEM_WIDTH, WEATHER_ITEM_START_Y+(1+num)*WEATHER_ITEM_HEIGHT}, \
\
        TRUE,               \
        idx_prdrd_block,\
\
        TABLESIZE(ptr_item_block_text_areas_##num),\
        ptr_item_block_text_areas_##num, \
\
        TABLESIZE(ptr_item_block_image_areas_##num),              \
        ptr_item_block_image_areas_##num, \
    },

static SVGUI_BLOCK_T ptr_blocks[]={
    {
        WEATHER_BLOCK_BKGND_1,     // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_WND_WIDTH, WEATHER_WND_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_BKGND_1,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        WEATHER_BLOCK_BKGND_2,     // int id;
        TRUE,                   // BOOL is_visible;
        {5, 30, 5+WEATHER_WND_WIDTH-11, 20+WEATHER_WND_HEIGHT-29}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_BKGND_2,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        WEATHER_BLOCK_TITLE_1,     // int id;
        TRUE,                   // BOOL is_visible;
        {WEATHER_EMBED_WND_START_X-3, 0, WEATHER_EMBED_WND_START_X+WEATHER_TITLE_1_WIDTH, WEATHER_TITLE_1_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_TITLE_1,// int idx_prdrd_block;
        TABLESIZE(ptr_title_1_block_text_areas),// int nr_text_areas;
        ptr_title_1_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_title_1_block_image_areas),  // int nr_image_areas;
        ptr_title_1_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        WEATHER_BLOCK_TITLE_2,     // int id;
        TRUE,                   // BOOL is_visible;
        {WEATHER_EMBED_WND_START_X, WEATHER_TITLE_1_HEIGHT, WEATHER_EMBED_WND_START_X+WEATHER_TITLE_2_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_TITLE_2,// int idx_prdrd_block;
        TABLESIZE(ptr_title_2_block_text_areas),// int nr_text_areas;
        ptr_title_2_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        WEATHER_BLOCK_LEFT_ARROW,     // int id;
        TRUE,                   // BOOL is_visible;
        {WEATHER_EMBED_WND_START_X, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT, WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_ARROW_WND_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_LEFT_ARROW,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_left_arrow_block_image_areas),              // int nr_image_areas;
        ptr_left_arrow_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        WEATHER_BLOCK_RIGHT_ARROW,     // int id;
        TRUE,                   // BOOL is_visible;
        {WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH+WEATHER_CONTENT_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT, WEATHER_EMBED_WND_START_X+WEATHER_CONTENT_WIDTH+2*WEATHER_ARROW_WND_WIDTH, WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_ARROW_WND_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        WEATHER_PRDRD_BLOCK_RIGHT_ARROW,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_right_arrow_block_image_areas),              // int nr_image_areas;
        ptr_right_arrow_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    DEFINE_CONTENT_BLOCK(0,TRUE)
    DEFINE_CONTENT_BLOCK(1,FALSE)
    DEFINE_CONTENT_BLOCK(2,FALSE)
    DEFINE_CONTENT_BLOCK(3,FALSE)
    DEFINE_CONTENT_BLOCK(4,FALSE)
    DEFINE_CONTENT_BLOCK(5,FALSE)
    DEFINE_ITEM_BLOCK(0, WEATHER_PRDRD_BLOCK_SELECTED)
    DEFINE_ITEM_BLOCK(1, WEATHER_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(2, WEATHER_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(3, WEATHER_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(4, WEATHER_PRDRD_BLOCK_NOT_SELECTED)
    DEFINE_ITEM_BLOCK(5, WEATHER_PRDRD_BLOCK_NOT_SELECTED)
};
enum{
    BIG_WEATHER_PRDRD_BLOCK_BKGND_CONTENT,
    BIG_WEATHER_PRDRD_BLOCK_CONTENT,
};
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_big_window_t[]={
    // BIG_WEATHER_PRDRD_BLOCK_BKGND_CONTENT
    {
        BIG_WND_CORNER+WEATHER_CONTENT_WIDTH,  // width
        BIG_WND_CORNER+WEATHER_CONTENT_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, 0, 0,
        //0x672900, 0x672900, 0x672900,            // (1,1,1),(60,59,60),(1,1,1)
        0x0066FF, 0x0066FF, 0x0066FF,            // (1,1,1),(60,59,60),(1,1,1)
        3,                          //int border_width;
        //0x672900, 0xFF6600, 0x672900,    //
        0x0066FF,               //RGBCOLOR border_color;
        BIG_WND_CORNER,                     //int corner;
        WEATHER_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // BIG_WEATHER_PRDRD_BLOCK_CONTENT
    {
        WEATHER_CONTENT_WIDTH,     // width
        WEATHER_CONTENT_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_NONE,// int gradient_type;
        0, WEATHER_CONTENT_HEIGHT/2, WEATHER_CONTENT_HEIGHT,
        0x727272, 0x2F2F2F, 0x101010,    //
        0,                          //int border_width;
        0x727272,               //RGBCOLOR border_color;
        0,                     //int corner;
        0xFFFF,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

#define DEFINE_BIG_WND_CONTENT_BLOCKS( num ) \
    {\
        WEATHER_BLOCK_CONTENT_##num,\
        TRUE,\
        {BIG_WND_CORNER/2, BIG_WND_CORNER/2, WEATHER_CONTENT_WIDTH+BIG_WND_CORNER/2, WEATHER_CONTENT_HEIGHT+BIG_WND_CORNER/2}, \
        FALSE,\
        BIG_WEATHER_PRDRD_BLOCK_CONTENT,\
        TABLESIZE(ptr_content_block_text_areas_##num),\
        ptr_content_block_text_areas_##num,\
        TABLESIZE(ptr_content_block_image_areas_##num),\
        ptr_content_block_image_areas_##num,\
    },

static SVGUI_BLOCK_T ptr_blocks_big_wnd[]={
    {
        0,     // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, WEATHER_CONTENT_WIDTH+BIG_WND_CORNER, WEATHER_CONTENT_HEIGHT+BIG_WND_CORNER}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BIG_WEATHER_PRDRD_BLOCK_BKGND_CONTENT,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    DEFINE_BIG_WND_CONTENT_BLOCKS(0)
    DEFINE_BIG_WND_CONTENT_BLOCKS(1)
    DEFINE_BIG_WND_CONTENT_BLOCKS(2)
    DEFINE_BIG_WND_CONTENT_BLOCKS(3)
    DEFINE_BIG_WND_CONTENT_BLOCKS(4)
    DEFINE_BIG_WND_CONTENT_BLOCKS(5)
};
//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_weather = {
    WEATHER_WND_WIDTH,  // width
    WEATHER_WND_HEIGHT, // height
    WEATHER_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    WEATHER_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_t),// nr_prdrd_blocks;
    ptr_prdrd_block_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;
    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks),// int nr_blocks;
    ptr_blocks,         // SVGUI_BLOCK_T* blocks;
};
//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_weather_big_wnd = {
    WEATHER_CONTENT_WIDTH+BIG_WND_CORNER,  // width
    WEATHER_CONTENT_HEIGHT+BIG_WND_CORNER, // height
    WEATHER_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    WEATHER_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_big_window_t),// nr_prdrd_blocks;
    ptr_prdrd_block_big_window_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;
    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks_big_wnd),// int nr_blocks;
    ptr_blocks_big_wnd,         // SVGUI_BLOCK_T* blocks;
};
static HWND InitWeatherBigWnd(void);

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
        pixel_bk = RGBA2Pixel (sec_hdc, 
                GetRValue (header->color_bk),
                GetGValue (header->color_bk),
                GetBValue (header->color_bk),
                GetAValue (header->color_bk));
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
static int SetContentBlockStatus (HWND hWnd, SVGUI_BLOCK_I *old_item_block, SVGUI_BLOCK_I* new_item_block, int item_min_id, int content_min_id){
    SVGUI_BLOCK_I *old_content_block = NULL;
    SVGUI_BLOCK_I *new_content_block = NULL;

    int old_content_block_id;
    int new_content_block_id;

    SVGUI_HEADER_I *header = NULL;
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return -1;

    if( old_item_block ){
        old_content_block_id = old_item_block->id - item_min_id + content_min_id;
        old_content_block = svgui_get_block(header, old_content_block_id); 
        if(old_content_block){
            old_content_block->is_visible = FALSE;
            InvalidateRect (hWnd, &old_content_block->rc, TRUE);
        }
    }
    if( new_item_block ){
        new_content_block_id = new_item_block->id - item_min_id + content_min_id;
        new_content_block = svgui_get_block(header, new_content_block_id);
        if(new_content_block){
            new_content_block->is_visible = TRUE;
            InvalidateRect (hWnd, &new_content_block->rc, TRUE);
        }
    }
    return 0;
}

//Caijun.Lee 20100407 
static void SetSelectedBigStatusByPos (HWND hWnd, int x_pos, int y_pos,int prdrd_idx_selected_item,int prdrd_idx_not_selected_item, int item_min_id, int item_max_id, int content_min_id)
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;


    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if( !block )
    {
            SendMessage(hWnd, MSG_KEYUP, SCANCODE_ENTER, 0);
        return;
    }
}

static void SetSelectedStatusByPos (HWND hWnd, int x_pos, int y_pos,int prdrd_idx_selected_item,int prdrd_idx_not_selected_item, int item_min_id, int item_max_id, int content_min_id)
{
    SVGUI_BLOCK_I *old_block = NULL;
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;


    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if( !block ){
        return;
    }

    debug (" xpos=%d, ypos=%d\n", x_pos, y_pos);

    switch(block->id )
    {
        case WEATHER_BLOCK_TITLE_1:
            SendMessage(hWnd, MSG_KEYUP, SCANCODE_ESCAPE, 0);
            return;

        case WEATHER_BLOCK_CONTENT_0:
         case WEATHER_BLOCK_CONTENT_1:
         case WEATHER_BLOCK_CONTENT_2:
         case WEATHER_BLOCK_CONTENT_3:
         case WEATHER_BLOCK_CONTENT_4:
         case WEATHER_BLOCK_CONTENT_5:
            SendMessage(hWnd, MSG_KEYUP, SCANCODE_ENTER, 0);
            return;
    }

    old_block = GetSelectedBlock (hWnd, prdrd_idx_selected_item, item_min_id, item_max_id);
    if (old_block != NULL)
    {
        old_block->idx_prdrd_block = prdrd_idx_not_selected_item;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }
    block->idx_prdrd_block=prdrd_idx_selected_item;
    InvalidateRect (hWnd, &block->rc, TRUE);

    // set current item
    weather_curr_item = block->id;

    SetContentBlockStatus (hWnd, old_block, block, item_min_id, content_min_id);
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

    // set current item
    weather_curr_item = block->id;

    SetContentBlockStatus (hWnd, old_block, block, item_min_id, content_min_id);
}
static void WeatherOnKeyupEnter(HWND hWnd)
{
    RECT src_rc_relative;
    RECT dst_rc;
    debug("Enter Key up()\n");
    switch(s_weather_wnd_status)
    {
        case WND_STATUS_NOT_DISPLAY:
            break;
        case WND_STATUS_SMALL:
            InitWeatherBigWnd();
#if HAVE_ANIMATION
            //memcpy(&wnd_without_items_rc, &weather_nomal_wnd_rect, sizeof(RECT));
            SetRect(&src_rc_relative, \
                    (WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH)*scale*scale_x, \
                    (WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT)*scale*scale_x, \
                    (WEATHER_EMBED_WND_START_X+WEATHER_ARROW_WND_WIDTH+WEATHER_CONTENT_WIDTH)*scale*scale_x, \
                    (WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_CONTENT_HEIGHT)*scale*scale_x );

            dst_rc = weather_large_wnd_rect;
            dst_rc.left += BIG_WND_CORNER*scale*scale_x/2;
            dst_rc.right -= BIG_WND_CORNER*scale*scale_x/2;
            dst_rc.top += BIG_WND_CORNER*scale_y/2;
            dst_rc.bottom -= BIG_WND_CORNER*scale_y/2;
            AnimationZoomIn( hWnd, &src_rc_relative,  &dst_rc, 50, ANIMATION_FRAMES );
#endif
            break;
        case WND_STATUS_LARGE:
            weather_big_status = 0;
#if HAVE_ANIMATION
            SetRect(&dst_rc, \
                    (WEATHER_WND_POS_X+WEATHER_EMBED_WND_START_X*scale+WEATHER_ARROW_WND_WIDTH*scale)*scale_x, \
                    (WEATHER_WND_POS_Y+WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT)*scale_y, \
                    (WEATHER_WND_POS_X+WEATHER_EMBED_WND_START_X*scale+WEATHER_ARROW_WND_WIDTH*scale+WEATHER_CONTENT_WIDTH*scale)*scale_x, \
                    (WEATHER_WND_POS_Y+WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_CONTENT_HEIGHT)*scale_y );
            AnimationZoomOut( hWnd, s_hwnd_normal_wnd, &dst_rc, 50, ANIMATION_FRAMES );
#endif
            //SendMessage(hWnd,MSG_CLOSE,0,0);
            DestroyMainWindow (hWnd);
            EnableWindow(s_hwnd_normal_wnd, TRUE);
            SetActiveWindow(s_hwnd_normal_wnd);
            InvalidateRect(s_hwnd_normal_wnd, NULL, TRUE);
            s_weather_wnd_status = WND_STATUS_SMALL;
            weather_status = 0;
            break;
    }
}

static int WeatherWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC subdc_weather;
    switch(message)
    {
        case MSG_CREATE:
            weather_big_status = 0;
            break;

        case MSG_KEYUP:
            switch(wParam)
            {

                case SCANCODE_CURSORBLOCKUP:
                    SetSelectedStatusByNextId (hWnd, FALSE, WEATHER_PRDRD_BLOCK_SELECTED,WEATHER_PRDRD_BLOCK_NOT_SELECTED, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5, WEATHER_BLOCK_CONTENT_0);
                    break;
                case SCANCODE_CURSORBLOCKDOWN:
                    SetSelectedStatusByNextId (hWnd, TRUE, WEATHER_PRDRD_BLOCK_SELECTED,WEATHER_PRDRD_BLOCK_NOT_SELECTED, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5,WEATHER_BLOCK_CONTENT_0);
                    break;

                case SCANCODE_ESCAPE:
                    {
                        weather_big_status ++;
                        if(weather_big_status == 1)
                        {
#if HAVE_ANIMATION
                            //start close window effect
                            AnimationMoveWndLeftRight( hWnd, weather_nomal_wnd_rect, MOVE_LEFT, ANIMATION_INTERVAL, ANIMATION_FRAMES );
#endif

                            s_weather_wnd_status = WND_STATUS_NOT_DISPLAY;
                            SendMessage(hWnd,MSG_CLOSE,0,0);
                            InitToolbar(g_hMainWnd);
                        }
                    }
                    break;

                case SCANCODE_ENTER:
                    weather_status ++;
                    if(weather_status == 1)
                        WeatherOnKeyupEnter(hWnd);
                    return 0;

                case SCANCODE_CURSORBLOCKLEFT:
                case SCANCODE_CURSORBLOCKRIGHT:
                    return 0;
            }
            break;

        case MSG_LBUTTONDOWN:
           SetSelectedStatusByPos (hWnd, LOWORD (lParam), HIWORD (lParam), WEATHER_PRDRD_BLOCK_SELECTED,WEATHER_PRDRD_BLOCK_NOT_SELECTED, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5,WEATHER_BLOCK_CONTENT_0);
            break;
        case MSG_CLOSE:
            s_hwnd_normal_wnd = NULL;
            subdc_weather = GetSecondaryDC(hWnd);
            DeleteMemDC(subdc_weather);
            DestroyMainWindow(hWnd);
            break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}
static int WeatherBigWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
//    RECT dst_rc;
    HDC subdc_weather;
    switch(message)
    {
        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_ESCAPE:
                    {
#if 0
#if HAVE_ANIMATION
                        SetRect(&dst_rc, \
                                (WEATHER_WND_POS_X+WEATHER_EMBED_WND_START_X*scale+WEATHER_ARROW_WND_WIDTH*scale)*scale_x, \
                                (WEATHER_WND_POS_Y+WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT)*scale_y, \
                                (WEATHER_WND_POS_X+WEATHER_EMBED_WND_START_X*scale+WEATHER_ARROW_WND_WIDTH*scale+WEATHER_CONTENT_WIDTH*scale)*scale_x, \
                                (WEATHER_WND_POS_Y+WEATHER_TITLE_1_HEIGHT+WEATHER_TITLE_2_HEIGHT+WEATHER_CONTENT_HEIGHT)*scale_y );
                        AnimationZoomOut( hWnd, s_hwnd_normal_wnd, &dst_rc, 50, ANIMATION_FRAMES );
#endif
#endif
                        SendMessage(hWnd,MSG_KEYUP,SCANCODE_ENTER,0);
                        EnableWindow(s_hwnd_normal_wnd, TRUE);
                        SetActiveWindow(s_hwnd_normal_wnd);
                        InvalidateRect(s_hwnd_normal_wnd, NULL, TRUE);
                        s_weather_wnd_status = WND_STATUS_SMALL;
                    }
                    return 0;

                case SCANCODE_CURSORBLOCKUP:
                case SCANCODE_CURSORBLOCKDOWN:
                //case SCANCODE_CURSORBLOCKLEFT:
                case SCANCODE_CURSORBLOCKRIGHT:
                    return 0;

                case SCANCODE_ENTER:
                    WeatherOnKeyupEnter(hWnd);
                    return 0;
            }
            break;
        case MSG_CLOSE:
            subdc_weather = GetSecondaryDC(hWnd);
            DeleteMemDC(subdc_weather);
            DestroyMainWindow(hWnd);
            break;
     case MSG_LBUTTONDOWN:
         SetSelectedBigStatusByPos (hWnd, LOWORD (lParam), HIWORD (lParam), WEATHER_PRDRD_BLOCK_SELECTED, WEATHER_PRDRD_BLOCK_NOT_SELECTED, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5, WEATHER_BLOCK_CONTENT_0);
         break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}

static void init_curr_item_status (HWND hWnd, int prdrd_idx_selected_item,int prdrd_idx_not_selected_item, int item_min_id, int item_max_id, int content_min_id)
{
    SVGUI_BLOCK_I *old_block = NULL;

    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    old_block = GetSelectedBlock (hWnd, prdrd_idx_selected_item, item_min_id, item_max_id);

    if (old_block != NULL){
        old_block->idx_prdrd_block = prdrd_idx_not_selected_item;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }

    block = svgui_get_block (header, weather_curr_item);
    if (block != NULL){
        block->idx_prdrd_block=prdrd_idx_selected_item;
        InvalidateRect(hWnd, &block->rc, TRUE);
    }

    SetContentBlockStatus (hWnd, old_block, block, item_min_id, content_min_id);
}

static void init_big_wnd_curr_item_status(int curr_item, int item_min_id, int item_max_id ){
    if(curr_item > item_max_id ){
        return ;
    }
    for( int i=1; i<item_max_id-item_min_id+1+1; i++){
        if( i == (curr_item-item_min_id+1)){
            ptr_blocks_big_wnd[i].is_visible = 1;
        }else{
            ptr_blocks_big_wnd[i].is_visible = 0;
        }
    }
    return ;
}

static HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc,const SVGUI_HEADER_T *svgui_header_t, HWND hHostingWnd )
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST |WS_EX_TROUNDCNS | WS_EX_BROUNDCNS;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = hWndProc;
    CreateInfo.lx = rect->left;
    CreateInfo.ty = rect->top;
    CreateInfo.rx = rect->right;
    CreateInfo.by = rect->bottom;
    CreateInfo.iBkColor = COLOR_black;
    CreateInfo.dwAddData = (DWORD)svgui_header_t;

    CreateInfo.hHosting = hHostingWnd;

    return CreateMainWindow (&CreateInfo);
}

HWND InitWeatherWnd(void)
{
    HWND hWeatherWnd;
    static int is_initialized = 0;

    if( !is_initialized ){
        weather_nomal_wnd_rect.left = RECTW(g_rcScr)*(WEATHER_LEFTSPACE_PERCENT);
        weather_nomal_wnd_rect.top  = RECTH(g_rcScr)*(WEATHER_TOPSPACE_PERCENT);
        weather_nomal_wnd_rect.right  = RECTW(g_rcScr)*(WEATHER_LEFTSPACE_PERCENT+WEATHER_WND_WIDTH_PERCENT*scale);
        weather_nomal_wnd_rect.bottom = RECTH(g_rcScr)*(WEATHER_TOPSPACE_PERCENT+WEATHER_WND_HEIGHT_PERCENT);

        weather_large_wnd_rect.left = (WEATHER_WND_POS_X+200)*scale_x*scale;
        weather_large_wnd_rect.top  = (WEATHER_WND_POS_Y+200)*scale_y;
        weather_large_wnd_rect.right  = weather_large_wnd_rect.left+(WEATHER_BIG_WND_WIDTH+BIG_WND_CORNER*BIG_WND_SCALE)*scale_x*scale;

        if ( scale_y/scale_x > 1.0 ) {
            weather_large_wnd_rect.bottom = weather_large_wnd_rect.top + (int)(BIG_WND_CORNER*BIG_WND_SCALE*scale_y*(1.0*WEATHER_CONTENT_HEIGHT/WEATHER_CONTENT_WIDTH) - 3 ) + (int)((weather_large_wnd_rect.right - weather_large_wnd_rect.left)*(1.0*WEATHER_CONTENT_HEIGHT/WEATHER_CONTENT_WIDTH) - 3 );
        } else {
            weather_large_wnd_rect.bottom = weather_large_wnd_rect.top+(WEATHER_BIG_WND_HEIGHT+BIG_WND_CORNER*BIG_WND_SCALE)*scale_y;
        }
 
    }

    debug(" ******** InitWeatherWnd():   start!\n");

    hWeatherWnd = CreateMenuWnd( &weather_nomal_wnd_rect, WeatherWndProc, &svgui_header_t_weather, HWND_DESKTOP);
    if (hWeatherWnd == HWND_INVALID){
        error("Create toolbar window!\n");
        return HWND_INVALID;
    }
    init_curr_item_status (hWeatherWnd, WEATHER_PRDRD_BLOCK_SELECTED,WEATHER_PRDRD_BLOCK_NOT_SELECTED, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5, WEATHER_BLOCK_CONTENT_0);

    set_window_seconddc(hWeatherWnd, weather_nomal_wnd_rect.left, weather_nomal_wnd_rect.top, RECTW(weather_nomal_wnd_rect),  RECTH(weather_nomal_wnd_rect) );
#if HAVE_ANIMATION
    if(WND_STATUS_NOT_DISPLAY == s_weather_wnd_status){
        //start create window effect
        AnimationMoveWndLeftRight( hWeatherWnd, weather_nomal_wnd_rect, MOVE_RIGHT, ANIMATION_INTERVAL, ANIMATION_FRAMES );
    }
#endif
    InvalidateRect(hWeatherWnd, &weather_nomal_wnd_rect, TRUE);
    ShowWindow (hWeatherWnd, SW_SHOWNORMAL);
    s_weather_wnd_status = WND_STATUS_SMALL;
    s_hwnd_normal_wnd = hWeatherWnd;
    return hWeatherWnd;
}

static HWND InitWeatherBigWnd(void){
    HWND hWeatherWnd;
    SVGUI_HEADER_I* svgui_header_i;
    HDC hdc_secondary;
    debug(" ******** InitWeatherBigWnd():   start!\n");

    if(!s_hwnd_normal_wnd){
        return NULL;
    }
    init_big_wnd_curr_item_status (weather_curr_item, WEATHER_BLOCK_ITEM_0, WEATHER_BLOCK_ITEM_5);

    hWeatherWnd = CreateMenuWnd( &weather_large_wnd_rect, WeatherBigWndProc, &svgui_header_t_weather_big_wnd, s_hwnd_normal_wnd);
    if (hWeatherWnd == HWND_INVALID){
        error("Create toolbar window!\n");
        return HWND_INVALID;
    }

    set_window_seconddc(hWeatherWnd, RECTW(g_rcScr)/2, weather_large_wnd_rect.top, RECTW(weather_large_wnd_rect),  RECTH(weather_large_wnd_rect) );
    
    EnableWindow(s_hwnd_normal_wnd, FALSE);

    InvalidateRect(hWeatherWnd, &weather_large_wnd_rect, TRUE);
    ShowWindow (hWeatherWnd, SW_SHOWNORMAL);
    s_weather_wnd_status = WND_STATUS_LARGE;

    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hWeatherWnd); // hWnd
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWeatherWnd );

    svgui_draw ( svgui_header_i, hdc_secondary, NULL);
    return hWeatherWnd;
}
