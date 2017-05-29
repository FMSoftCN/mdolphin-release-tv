#include <unistd.h>

#include "svgui.h"
#include "mdtv_app.h"
#include "mdtv_common.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_toolbar.h"
#include "mdtv_toolbar_zoom.h"
#include "mdtv_animation.h"
#include "mdtv_website.h"
#include "mdtv_weather.h"
#include "mdtv_finance.h"
#include "mdtv_search.h"
#include "mdtv_fav.h"
#include "mdtv_setting.h"
#include "mdtv_browser.h"
#include "mdtv_setting_homepage.h"

#define HAVE_ANIMATION      1

extern HWND g_hMainWnd ;
static MSGHOOK old_hook;
#define ARRAY_LEN(array) \
    (sizeof(array)/sizeof(array[0]))

#define NDEBUG  1
#ifndef NDEBUG
#define DEBUG_TRACE(P) {fprintf (stderr, "FIXME: %s,%d,%s: %s\n", __FILE__, __LINE__, __FUNCTION__ ,(P));}

#define error(fmt...) fprintf (stderr, "mdtv_toolbar[ERROR]:"fmt)
#define debug(fmt...) fprintf (stderr, "mdtv_toolbar[DEBUG]:"fmt)

#else
#define DEBUG_TRACE(P) 
#define error(fmt...) fprintf (stderr, "mdtv_toolbar[ERROR]:"fmt)
#define debug(fmt...)
#endif

// toolbar position in screen
RECT toolbar_rect;
static int toolbar_item_width;
static int toolbar_space_width;
static int toolbar_arrow_width;

// screen width: DEFAULT_SCREEN_WIDTH, height: DEFAULT_SCREEN_HEIGHT
// these sizes is corresponding to the screen size:1024x768
//DEFAULT_SCREEN_WIDTH ( 1024 ), DEFAULT_SCREEN_HEIGHT( 768 )
#define TOOLBAR_SPACE_WIDTH     (9)
#define TOOLBAR_SPACE_HEIGHT    (73-7)
#define TOOLBAR_WIDTH           (DEFAULT_SCREEN_WIDTH)
#define TOOLBAR_HEIGHT          (TOOLBAR_ITEM_HEIGHT)

#define TOOLBAR_WIDTH_SCALE    (1.0*TOOLBAR_ITEM_WIDTH/DEFAULT_SCREEN_WIDTH)
#define TOOLBAR_HEIGHT_SCALE    (1.0*TOOLBAR_ITEM_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define TOOLBAR_ITEM_PENCENT    (1.0*TOOLBAR_ITEM_WIDTH/DEFAULT_SCREEN_WIDTH)       // 4 * 2/9
#define TOOLBAR_ARROW_PENCENT   (1.0*TOOLBAR_ARROW_WIDTH/DEFAULT_SCREEN_WIDTH)      // 2 * 2/63
#define TOOLBAR_SPACE_PENCENT   (1.0*TOOLBAR_SPACE_WIDTH/DEFAULT_SCREEN_WIDTH)      // 3 * 1/63


#define TOOLBAR_COLOR_TEXT_SELECTED     (0xFFFFFF)
//#define TOOLBAR_COLOR_TEXT_NOT_SELECTED (0x000000)
#define TOOLBAR_COLOR_TEXT_NOT_SELECTED (0xF6FF00)
#define ANIMATION_LEFT_RIGHT_INTERVAL   20
#define ANIMATION_LEFT_RIGHT_FRAMES     5
// toolbar item index
typedef enum _ToolbarItem{
    TOOLBAR_START_ITEM_ID=0,
    TOOLBAR_HOME=TOOLBAR_START_ITEM_ID,
    TOOLBAR_WEBSITE,
    TOOLBAR_FINANCE,
    TOOLBAR_WEATHER,
#if ENABLE_FAV     
    TOOLBAR_FAV,
#endif    
    TOOLBAR_SETTING,
    TOOLBAR_SEARCH,
#if TOOLBAR_WITH_ZOOM
    TOOLBAR_ZOOM_IN_OUT,
    TOOLBAR_MAX_ITEM_ID=TOOLBAR_ZOOM_IN_OUT,
#else
    TOOLBAR_MAX_ITEM_ID=TOOLBAR_SEARCH,
#endif
} ToolbarItem;
static ToolbarItem curr_item = TOOLBAR_HOME;

//---- SVGUI_PRDRD_BLOCK_T
// toolbar background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[];
typedef enum _ToolBarBkgndBlock{
    TOOLBAR_SELECTED_BLOCK,
    TOOLBAR_NOT_SELECTED_BLOCK,
    TOOLBAR_SPACE_BLOCK,
    TOOLBAR_ARROW_BLOCK,
} ToolBarBkgndBlock;

#define TOOLBAR_PRDRD_BLOCK_COLOR_BK   (0x000000)
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // 0: background block for selected item: TOOLBAR_SELECTED_BLOCK
    {
        TOOLBAR_ITEM_WIDTH,     // width
        TOOLBAR_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_HORZ,// int gradient_type;
        0, TOOLBAR_ITEM_HEIGHT/2, TOOLBAR_ITEM_HEIGHT,
        0x531A00, 0xFF6500, 0x531A00,//(83,26,0),(255,101,0),(83,26,0)

        2,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;

        14,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // 1: background block for selected item: TOOLBAR_NOT_SELECTED_BLOCK
    {
        TOOLBAR_ITEM_WIDTH,     // width
        TOOLBAR_ITEM_HEIGHT,    // height
        SVGUI_PRDRD_GRADIENT_HORZ,// int gradient_type;
        0, TOOLBAR_ITEM_HEIGHT/2, TOOLBAR_ITEM_HEIGHT,
        //0x452222, 0x45bf39, 0x452222,
        0x101010, 0x3C3B3B, 0x101010,// (1,1,1),(60,59,59),(1,1,1)

        2,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;

        14,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // 2: space block between items: TOOLBAR_SPACE_BLOCK
    {
        TOOLBAR_SPACE_WIDTH,    // width
        TOOLBAR_SPACE_HEIGHT,   // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, TOOLBAR_SPACE_HEIGHT/2, TOOLBAR_SPACE_HEIGHT+1,
        0x5F5F5F, 0x252525, 0x101010,//(95,95,95),(37,37,37),(0,0,0)

        0,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;

        0,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // 3: arrow backgound block : TOOLBAR_ARROW_BLOCK
    {
        TOOLBAR_ARROW_WIDTH,    // width
        TOOLBAR_ARROW_HEIGHT,   // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, TOOLBAR_ARROW_WIDTH/2, TOOLBAR_ARROW_WIDTH,
        0x5F5F5F, 0x252525, 0x101010,//(95,95,95),(37,37,37),(0,0,0)

        0,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;

        0,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },

};

//---- SVGUI_TEXT_AREA_T
#define TOOLBAR_FONT_IDX    2
static SVGUI_TEXT_AREA_T ptr_home_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        //{10, 10, 10, 10},       // RECT rc;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        TOOLBAR_COLOR_TEXT_SELECTED,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "主页 || Home",         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_website_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        TOOLBAR_COLOR_TEXT_NOT_SELECTED,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "网址 || Website",         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_finance_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        TOOLBAR_COLOR_TEXT_NOT_SELECTED,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "金融 || Finance",         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_weather_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*3/4, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        TOOLBAR_COLOR_TEXT_NOT_SELECTED,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "天气 || Weather",         // char* text;
    },
};
#if ENABLE_FAV
static SVGUI_TEXT_AREA_T ptr_fav_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        TOOLBAR_COLOR_TEXT_NOT_SELECTED,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "收藏 || Favorite",     // char* text;
    },
};
#endif
static SVGUI_TEXT_AREA_T ptr_setting_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFF00FF,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "设置 || Setting",      // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_search_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFF00FF,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "搜索 || Search",      // char* text;
    },
};

#if TOOLBAR_WITH_ZOOM
static SVGUI_TEXT_AREA_T ptr_zoom_in_out_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ITEM_WIDTH*2/3, TOOLBAR_ITEM_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFF00FF,               // RGBCOLOR color;
        TOOLBAR_FONT_IDX,                      //................... int idx_font;
        "缩放 || Zoom",      // char* text;
    },
};
#endif
//---- SVGUI_IMAGE
static SVGUI_IMAGE_T ptr_home_block_images[]={
    { 35, 36, 16, "toolbar/home_35x37.png",},
    { 43, 44, 16, "toolbar/home_43x44.png",},
    { 50, 60, 16, "toolbar/home_58x64.png",},
};
static SVGUI_IMAGE_T ptr_finance_block_images[]={
    { 35, 36, 16, "toolbar/finance_35x37.png",},
    { 43, 44, 16, "toolbar/finance_43x44.png",},
    { 50, 60, 16, "toolbar/finance_51x60.png",},
};
static SVGUI_IMAGE_T ptr_website_block_images[]={
    { 35, 36, 16, "toolbar/website_32x34.png",},
    { 43, 44, 16, "toolbar/website_40x41.png",},
    { 50, 60, 16, "toolbar/website_48x56.png",},
};
static SVGUI_IMAGE_T ptr_weather_block_images[]={
    { 35, 36, 16, "toolbar/weather_39x37.png",},
    { 43, 44, 16, "toolbar/weather_49x45.png",},
    { 50, 60, 16, "toolbar/weather_59x78.png",},
};
#if ENABLE_FAV
static SVGUI_IMAGE_T ptr_fav_block_images[]={
    { 35, 36, 16, "toolbar/fav_29x30.png",},
    { 43, 44, 16, "toolbar/fav_38x39.png",},
    { 50, 60, 16, "toolbar/fav_48x64.png",},
};
#endif
static SVGUI_IMAGE_T ptr_setting_block_images[]={
    { 35, 36, 16, "toolbar/setting_28x29.png",},
    { 43, 44, 16, "toolbar/setting_44x41.png",},
    { 50, 60, 16, "toolbar/setting_56x59.png",},
};
static SVGUI_IMAGE_T ptr_search_block_images[]={
    { 35, 36, 16, "toolbar/search_31x31.png",},
    { 43, 44, 16, "toolbar/search_44x43.png",},
    { 50, 60, 16, "toolbar/search_66x65.png",},
};
#if TOOLBAR_WITH_ZOOM
static SVGUI_IMAGE_T ptr_zoom_in_out_block_images[]={
    { 56, 59, 16, "toolbar/zoom_in_out.png",},
};
#endif
static SVGUI_IMAGE_T ptr_arrow_1_block_images[]={
    { 13, 23, 16, "toolbar/arrow_left_13x23.png",},
    { 16, 28, 16, "toolbar/arrow_left_16x28.png",},
    { 23, 44, 16, "toolbar/arrow_left_23x44.png",},
};
static SVGUI_IMAGE_T ptr_arrow_2_block_images[]={
    { 13, 23, 16, "toolbar/arrow_right_13x23.png",},
    { 16, 28, 16, "toolbar/arrow_right_16x28.png",},
    { 23, 48, 16, "toolbar/arrow_right_23x48.png",},
};

//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T ptr_home_block_image_areas[]={
    {
        0,
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_home_block_images),                      // int nr_images;
        ptr_home_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_website_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_website_block_images),                      // int nr_images;
        ptr_website_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_weather_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_weather_block_images),                      // int nr_images;
        ptr_weather_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_finance_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_finance_block_images),                      // int nr_images;
        ptr_finance_block_images,  // SVGUI_IMAGE* images;
    },
};
#if ENABLE_FAV
static SVGUI_IMAGE_AREA_T ptr_fav_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_fav_block_images),                      // int nr_images;
        ptr_fav_block_images,   // SVGUI_IMAGE* images;
    },
};
#endif
static SVGUI_IMAGE_AREA_T ptr_setting_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_setting_block_images),                      // int nr_images;
        ptr_setting_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_search_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_search_block_images),                      // int nr_images;
        ptr_search_block_images,  // SVGUI_IMAGE* images;
    },
};
#if TOOLBAR_WITH_ZOOM
static SVGUI_IMAGE_AREA_T ptr_zoom_in_out_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ITEM_WIDTH*2/3,0,TOOLBAR_ITEM_WIDTH,TOOLBAR_ITEM_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_zoom_in_out_block_images),                      // int nr_images;
        ptr_zoom_in_out_block_images,  // SVGUI_IMAGE* images;
    },
};
#endif
static SVGUI_IMAGE_AREA_T ptr_arrow_1_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ARROW_WIDTH,TOOLBAR_ARROW_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_arrow_1_block_images),                      // int nr_images;
        ptr_arrow_1_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_arrow_2_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ARROW_WIDTH,TOOLBAR_ARROW_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_arrow_2_block_images),                      // int nr_images;
        ptr_arrow_2_block_images,// SVGUI_IMAGE* images;
    },
};

//---- SVGUI_BLOCK_T
typedef enum _ToolbarBlockIdx{
    TOOLBAR_ARROW_1=TOOLBAR_MAX_ITEM_ID+1,
    TOOLBAR_ARROW_2,
    TOOLBAR_SPACE_1,
    TOOLBAR_SPACE_2,
    TOOLBAR_SPACE_3,
} ToolbarBlockIdx;

static SVGUI_BLOCK_T ptr_blocks[]={
    {
        TOOLBAR_HOME,   // int id;
        TRUE,           // BOOL is_visible;
        {0*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+0*TOOLBAR_SPACE_WIDTH, 0, 1*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+0*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_SELECTED_BLOCK,//TOOLBAR_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_home_block_text_areas),// int nr_text_areas;
        ptr_home_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_home_block_image_areas),              // int nr_image_areas;
        ptr_home_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_WEBSITE,        // int id;
        TRUE,           // BOOL is_visible;
        {1*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+1*TOOLBAR_SPACE_WIDTH, 0, 2*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+1*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_website_block_text_areas),              // int nr_text_areas;
        ptr_website_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_website_block_image_areas),              // int nr_image_areas;
        ptr_website_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_FINANCE,            // int id;
        TRUE,               // BOOL is_visible;
        {2*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+2*TOOLBAR_SPACE_WIDTH, 0, 3*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+2*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,              // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_finance_block_text_areas),                  // int nr_text_areas;
        ptr_finance_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_finance_block_image_areas),                  // int nr_image_areas;
        ptr_finance_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_WEATHER,            // int id;
        TRUE,               // BOOL is_visible;
        {3*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+3*TOOLBAR_SPACE_WIDTH, 0, 4*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+3*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,              // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_weather_block_text_areas),                  // int nr_text_areas;
        ptr_weather_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_weather_block_image_areas),                  // int nr_image_areas;
        ptr_weather_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
#if ENABLE_FAV    
    {
        TOOLBAR_FAV,                // int id;
        FALSE,               // BOOL is_visible;
        {4*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+4*TOOLBAR_SPACE_WIDTH, 0, 5*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+4*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,              // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_fav_block_text_areas),                  // int nr_text_areas;
        ptr_fav_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_fav_block_image_areas),                  // int nr_image_areas;
        ptr_fav_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
#endif    
    {
        TOOLBAR_SETTING,            // int id;
        FALSE,               // BOOL is_visible;
        {5*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+5*TOOLBAR_SPACE_WIDTH, 0, 6*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+5*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_setting_block_text_areas),                  // int nr_text_areas;
        ptr_setting_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_setting_block_image_areas),                  // int nr_image_areas;
        ptr_setting_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_SEARCH,            // int id;
        FALSE,               // BOOL is_visible;
        {6*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+6*TOOLBAR_SPACE_WIDTH, 0, 7*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+6*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_search_block_text_areas),                  // int nr_text_areas;
        ptr_search_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_search_block_image_areas),                  // int nr_image_areas;
        ptr_search_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
#if TOOLBAR_WITH_ZOOM
    {
        TOOLBAR_ZOOM_IN_OUT,            // int id;
        FALSE,               // BOOL is_visible;
        {6*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+6*TOOLBAR_SPACE_WIDTH, 0, 7*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+6*TOOLBAR_SPACE_WIDTH, TOOLBAR_ITEM_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_NOT_SELECTED_BLOCK, // int idx_prdrd_block;

        ARRAY_LEN(ptr_zoom_in_out_block_text_areas),                  // int nr_text_areas;
        ptr_zoom_in_out_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_zoom_in_out_block_image_areas),                  // int nr_image_areas;
        ptr_zoom_in_out_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
#endif
    {
        TOOLBAR_ARROW_1,        // int id;
        TRUE,                   // BOOL is_visible;
        {0, 10, TOOLBAR_ARROW_WIDTH, TOOLBAR_ARROW_HEIGHT}, // RECT rc;

        TRUE,                  // BOOL is_hotspot;
        TOOLBAR_ARROW_BLOCK,    // int idx_prdrd_block;

        0,                      // int nr_text_areas;
        NULL,                   // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_arrow_1_block_image_areas),              // int nr_image_areas;
        ptr_arrow_1_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_ARROW_2,        // int id;
        TRUE,           // BOOL is_visible;
        {4*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+3*TOOLBAR_SPACE_WIDTH, 10, 4*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+3*TOOLBAR_SPACE_WIDTH+TOOLBAR_ARROW_WIDTH, TOOLBAR_ARROW_HEIGHT}, // RECT rc;

        TRUE,          // BOOL is_hotspot;
        TOOLBAR_ARROW_BLOCK,    // int idx_prdrd_block;

        0,              // int nr_text_areas;
        NULL,           // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_arrow_2_block_image_areas),              // int nr_image_areas;
        ptr_arrow_2_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_SPACE_1,        // int id;
        TRUE,           // BOOL is_visible;
        {1*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+0*TOOLBAR_SPACE_WIDTH, 10, 1*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+1*TOOLBAR_SPACE_WIDTH, TOOLBAR_SPACE_HEIGHT}, // RECT rc;

        FALSE,          // BOOL is_hotspot;
        TOOLBAR_SPACE_BLOCK,    // int idx_prdrd_block;

        0,              // int nr_text_areas;
        NULL,           // SVGUI_TEXT_AREA_T* text_areas;

        0,              // int nr_image_areas;
        NULL,           // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_SPACE_2,        // int id;
        TRUE,           // BOOL is_visible;
        {2*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+1*TOOLBAR_SPACE_WIDTH, 10, 2*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+2*TOOLBAR_SPACE_WIDTH, TOOLBAR_SPACE_HEIGHT}, // RECT rc;

        FALSE,          // BOOL is_hotspot;
        TOOLBAR_SPACE_BLOCK,    // int idx_prdrd_block;

        0,              // int nr_text_areas;
        NULL,           // SVGUI_TEXT_AREA_T* text_areas;

        0,              // int nr_image_areas;
        NULL,           // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_SPACE_3,        // int id;
        TRUE,           // BOOL is_visible;
        {3*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+2*TOOLBAR_SPACE_WIDTH, 10, 3*TOOLBAR_ITEM_WIDTH+TOOLBAR_ARROW_WIDTH+3*TOOLBAR_SPACE_WIDTH, TOOLBAR_SPACE_HEIGHT}, // RECT rc;

        FALSE,          // BOOL is_hotspot;
        TOOLBAR_SPACE_BLOCK,    // int idx_prdrd_block;

        0,              // int nr_text_areas;
        NULL,           // SVGUI_TEXT_AREA_T* text_areas;

        0,              // int nr_image_areas;
        NULL,           // SVGUI_IMAGE_AREA_T* image_areas;
    },
};
//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_toolbar = {

    TOOLBAR_WIDTH,// width
    TOOLBAR_HEIGHT,// height

    TOOLBAR_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    TOOLBAR_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;

    //back ground block description: SVGUI_PRDRD_BLOCK_T
    ARRAY_LEN(ptr_prdrd_block_t),// nr_prdrd_blocks;
    ptr_prdrd_block_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;

    ARRAY_LEN(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    ARRAY_LEN(ptr_blocks),// int nr_blocks;
    ptr_blocks,         // SVGUI_BLOCK_T* blocks;
};

static BOOL set_window_seconddc(HWND hWnd, int   off_x, int      off_y,int   width,  int height)
{
    SVGUI_HEADER_I * header;
//    gal_pixel pixel_bk;
    HDC hdc_secondary;

    debug("************ set_window_seconddc  *******\n");
    debug("width=%d, height=%d\n", width, height);
    //HDC hdc = alloc_subdc_from_uisec( off_x, off_y, width, height);
    hdc_secondary = CreateSubMemDC(g_dc_uisec, off_x, off_y, width, height, TRUE);
    if (hdc_secondary == HDC_INVALID){
        return FALSE;
    }

    SetSecondaryDC(hWnd, hdc_secondary, ON_UPDSECDC_DEFAULT);
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header)
    {
        SetBrushColor (hdc_secondary, header->pixel_key);
        FillBox (hdc_secondary, 0, 0, GetGDCapability(hdc_secondary,GDCAP_MAXX), GetGDCapability(hdc_secondary,GDCAP_MAXY));
        SetMemDCColorKey (hdc_secondary, MEMDC_FLAG_SRCCOLORKEY, header->pixel_key);
    }
    return TRUE;
}

static void ToolbarOnPressEnter(HWND hWnd,int Index)
{
//    HDC subdc_toolbar;
    switch(Index)
    {
        case TOOLBAR_HOME:
            {
                SendMessage(hWnd, MSG_CLOSE, 0, 0);
                printf("----- init home window -----\n");
                mdtv_CreateWebSiteNavigate(get_setted_home_url());
                break;
            }
        case TOOLBAR_WEBSITE:
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            printf("----- init website window -----\n");
            mdtv_CreateWebsiteWindow (g_hMainWnd);
            //InitWebsiteWnd();
            break;
        case TOOLBAR_FINANCE:
            printf("----- init finance window -----\n");
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            InitFinanceWnd();
            break;
        case TOOLBAR_WEATHER:
            printf("----- init weather window -----\n");
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            InitWeatherWnd();
            break;
#if ENABLE_FAV            
        case TOOLBAR_FAV:
            printf("----- init fav window -----\n");
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            InitMDTVFavWnd();
            break;
#endif            
        case TOOLBAR_SETTING:
            printf("----- init setting window -----\n");
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            InitSettingWnd();
            break;
        case TOOLBAR_SEARCH:
            SendMessage(hWnd, MSG_CLOSE, 0, 0);
            {
                printf("----- init search window -----\n");
                navigate_to_search(hWnd);
                break;
            }
#if TOOLBAR_WITH_ZOOM
        case TOOLBAR_ZOOM_IN_OUT:
            InitToolbarZoomWnd();
            break;
#endif
    }
}
HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc, HWND hHostingWnd )
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST | WS_EX_AUTOSECONDARYDC ;
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
    CreateInfo.dwAddData = (DWORD)&svgui_header_t_toolbar;

    CreateInfo.hHosting = hHostingWnd;

    return CreateMainWindow (&CreateInfo);
}

static int ToobarMoveToNextItem(int curr_item, int dirrect){
    if(curr_item<0 || curr_item>TOOLBAR_MAX_ITEM_ID){
        error("[Error] current item is bigger max item!\n");
        return -1;
    }
    debug("     curr_item   =   %d\n", curr_item);
    if( dirrect == MOVE_LEFT ){
        if( curr_item == TOOLBAR_MAX_ITEM_ID ){
            curr_item = 0;
        } else {
            curr_item++;
        }
    } else if( dirrect == MOVE_RIGHT ){
        if( curr_item == 0 ){
            curr_item = TOOLBAR_MAX_ITEM_ID;
        } else {
            curr_item--;
        }
    }
    debug("     after move: curr_item   =   %d\n", curr_item);
    return curr_item;
}

static int ToolbarNextItem(int curr_item ){
    if(curr_item<0 || curr_item>TOOLBAR_MAX_ITEM_ID){
        error("[Error] current item is bigger max item!\n");
        return -1;
    }
    if( curr_item == TOOLBAR_MAX_ITEM_ID ){
        curr_item = 0;
    } else {
        curr_item++;
    }
    return curr_item;
}

static int ToolBarSetSvguiHeaderI(SVGUI_HEADER_I *svgui_header_i, int curr_item, HDC hdc){
    debug("*************** ToolBarSetSvguiHeaderI **********\n");
    SVGUI_BLOCK_I       *svgui_block_i;
    SVGUI_TEXT_AREA_I   *text_areas;
    int i;
    if(!svgui_header_i){
        error("[Error]svgui instance is NULL!\n");
        return -1;
    }
    for(i=0; i<4; i++){
        debug("     : curr_item   =   %d\n", curr_item);
        svgui_block_i = svgui_get_block (svgui_header_i, curr_item);
        text_areas  = svgui_get_text_area (svgui_header_i,svgui_block_i, 0);

        svgui_block_i->is_visible       = TRUE;
        if( i==0 ){
            svgui_block_i->idx_prdrd_block  = TOOLBAR_SELECTED_BLOCK;
            text_areas->text_pixel = RGB2Pixel(hdc, GetRValue(TOOLBAR_COLOR_TEXT_SELECTED), GetGValue(TOOLBAR_COLOR_TEXT_SELECTED), GetBValue(TOOLBAR_COLOR_TEXT_SELECTED));
        }else{
            svgui_block_i->idx_prdrd_block  = TOOLBAR_NOT_SELECTED_BLOCK;
            text_areas->text_pixel = RGB2Pixel(hdc, GetRValue(TOOLBAR_COLOR_TEXT_NOT_SELECTED), GetGValue(TOOLBAR_COLOR_TEXT_NOT_SELECTED), GetBValue(TOOLBAR_COLOR_TEXT_NOT_SELECTED));
        }
        svgui_block_i->rc.left = i*toolbar_item_width+toolbar_arrow_width+i*toolbar_space_width;
        svgui_block_i->rc.top = 0;
        svgui_block_i->rc.right = svgui_block_i->rc.left+toolbar_item_width;
        svgui_block_i->rc.bottom = svgui_block_i->rc.top+RECTH(toolbar_rect);

        curr_item = ToolbarNextItem(curr_item);

    debug("     InitWidth(): svgui_block_i->rc.left=%d,svgui_block_i->rc.right=%d,width=%d\n", svgui_block_i->rc.left,svgui_block_i->rc.right,RECTW(svgui_block_i->rc));
    }
    // set other item to not visible
    for(i=0;i<=TOOLBAR_MAX_ITEM_ID-4; i++){
        svgui_block_i = svgui_get_block (svgui_header_i, curr_item);
        debug("     :block_id=%d, text_areas->text=%s\n",svgui_block_i->id, svgui_block_i->text_areas->text);
        svgui_block_i->is_visible       = FALSE;
        curr_item = ToolbarNextItem(curr_item);
    }

    return 0;
}

// for toolbar to move left and move right
void ToolbarAnimateMove(HWND hWnd, const RECT *dst_effrc, int dirrect, int *curr_item, int interval , int frames ){
    HDC hdc_secondary;
    HDC hdc_client;
    int width;
    int height;
    SVGUI_HEADER_I  *svgui_header_i;
    RECT window_rc;

    int offset;
    int offset_total;
    int start_x_sec_dc;
    int start_x_client_dc;
    // get second dc of toolbar or menu's window
    hdc_secondary   = GetSecondaryDC( hWnd );
    hdc_client      = GetClientDC( hWnd );

    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
#if HAVE_ANIMATION
    // draw screen when continuing pressing left/right (not drawing block in MSG_ERASEBKGND on the situation
    svgui_draw ( svgui_header_i, hdc_client, NULL);
#endif

    *curr_item = ToobarMoveToNextItem(*curr_item, dirrect);
    debug("     after move: *curr_item   =   %d\n", *curr_item);

    width   = RECTW(*dst_effrc)-2*toolbar_arrow_width;
    height  = RECTH(*dst_effrc);

    // draw secondary dc: sub_dc_toolbar_move
    ToolBarSetSvguiHeaderI(svgui_header_i, *curr_item, hdc_client);
#if HAVE_ANIMATION
    svgui_draw ( svgui_header_i, hdc_secondary, NULL);//dst_effrc);

    offset          = (toolbar_space_width + toolbar_item_width)/frames;
    offset_total    = offset;

    if( dirrect == MOVE_LEFT ){
        start_x_sec_dc=2*toolbar_space_width+3*toolbar_item_width+toolbar_arrow_width;
        start_x_client_dc= RECTW(*dst_effrc)-toolbar_arrow_width;
    } else if( dirrect == MOVE_RIGHT ){
        start_x_sec_dc=toolbar_space_width+toolbar_item_width+toolbar_arrow_width;
        start_x_client_dc=toolbar_arrow_width;
    } else {
        error("[Warning]animation dirrect is not MOVE_LEFT, MOVE_RIGHT!\n");
    }
    for ( int i = 0; i < frames; i++ ){
        if( dirrect == MOVE_LEFT ){
            BitBlt( hdc_client, toolbar_arrow_width+offset, 0,  width-offset, height, hdc_client, toolbar_arrow_width, 0, 0 );
            BitBlt( hdc_secondary, start_x_sec_dc, 0, offset_total, height, hdc_client, start_x_client_dc-offset_total, 0, 0 );
        } else if( dirrect == MOVE_RIGHT ){
            BitBlt( hdc_client, toolbar_arrow_width, 0, width-offset, height, hdc_client, toolbar_arrow_width+offset, 0, 0 );
            BitBlt( hdc_secondary, start_x_sec_dc-offset_total, 0, offset_total,  height, hdc_client, start_x_client_dc, 0, 0 );
        }
        offset_total+=offset;
        //usleep(1000*1000);
        usleep(interval*1000);
    }
#endif
    SVGUI_BLOCK_I *svgui_block_i_arrow_1;
    svgui_block_i_arrow_1 = svgui_get_block (svgui_header_i, TOOLBAR_ARROW_1);
    SetRect(&window_rc, RECTW(svgui_block_i_arrow_1->rc), 0, svgui_header_i->width-RECTW(svgui_block_i_arrow_1->rc), svgui_header_i->height);
#ifdef WITH_BKGND_PIC
    BitBlt (g_dc_bkgnd, window_rc.left, window_rc.top, RECTW(window_rc), RECTH(window_rc), 0, window_rc.left, window_rc.top, -1);
#else
    SetBrushColor(hdc_client, RGB2Pixel(hdc_client, 0x00, 0x00, 0x00) );
    FillBox ( hdc_client, window_rc.left, window_rc.top, RECTW(window_rc), RECTH(window_rc));
#endif
    InvalidateRect(hWnd, &window_rc, TRUE);
    ReleaseDC(hdc_client);
}

static int ToolbarOnLButtonDown (HWND hWnd, int x_pos, int y_pos, int prdrd_id_item_selected,int prdrd_id_item_not_selected, int* p_curr_item)
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return -1;

    block = svgui_get_block_by_point (header, x_pos, y_pos);

    debug (" xpos=%d, ypos=%d\n", x_pos, y_pos);

    if (block == NULL)
        return -1;
    switch( block->id ){
        case TOOLBAR_ARROW_2:
            ToolbarAnimateMove(hWnd, &toolbar_rect, MOVE_LEFT, p_curr_item, ANIMATION_LEFT_RIGHT_INTERVAL, ANIMATION_LEFT_RIGHT_FRAMES);
            //*p_curr_item = ToobarMoveToNextItem(*p_curr_item, MOVE_LEFT);
            break;
        case TOOLBAR_ARROW_1:
            ToolbarAnimateMove(hWnd, &toolbar_rect, MOVE_RIGHT, p_curr_item, ANIMATION_LEFT_RIGHT_INTERVAL, ANIMATION_LEFT_RIGHT_FRAMES);
            //*p_curr_item = ToobarMoveToNextItem(*p_curr_item, MOVE_RIGHT);
            break;
        default:
            *p_curr_item = block->id;
            ToolbarOnPressEnter(hWnd, curr_item);
            break;
    }
    debug (" curr_item=%d\n", *p_curr_item);

    return 0;
}

#if HAVE_ANIMATION
static void InitWidth(HWND hWnd)
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block(header, TOOLBAR_HOME); 
    if (block == NULL){
        return;
    }
    toolbar_item_width = RECTW(block->rc);

    block = svgui_get_block(header, TOOLBAR_SPACE_1); 
    if (block == NULL){
        return;
    }
    toolbar_space_width = RECTW(block->rc);

    block = svgui_get_block(header, TOOLBAR_ARROW_1); 
    if (block == NULL){
        return;
    }
    toolbar_arrow_width = RECTW(block->rc);

    debug("     InitWidth(): toolbar_item_width=%d,toolbar_space_width=%d,toolbar_arrow_width=%d\n", toolbar_item_width,toolbar_space_width,toolbar_arrow_width);
    return;
}
#endif

void OnMsgClose(HWND hWnd)
{
//    HDC subdc_toolbar;
    AnimationMoveWndUpDown ( hWnd, toolbar_rect, MOVE_DOWN, ANIMATION_INTERVAL, ANIMATION_FRAMES );
#if TOOLBAR_WITH_ZOOM
    if(g_mdolphin_main_hwnd){
        InitBackGround();
    }
#endif
    //subdc_toolbar = GetSecondaryDC(hWnd);
    //DeleteMemDC(subdc_toolbar);
    DestroyMainWindow(hWnd);
    RegisterKeyMsgHook ((&g_hMainWnd), old_hook);
}

static int ToolbarProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    MSG msg;
    switch (message)
    {
        case MSG_CREATE:
            break;
        case MSG_KEYDOWN:
            while(PeekMessage(&msg, hWnd, MSG_KEYDOWN, MSG_KEYDOWN, PM_REMOVE))
            {
                //printf("---------- PeekMessage()\n");
            }
            switch(wParam)
            {
                case SCANCODE_CURSORBLOCKRIGHT:
                    ToolbarAnimateMove(hWnd, &toolbar_rect, MOVE_LEFT, (int*)&curr_item, ANIMATION_LEFT_RIGHT_INTERVAL, ANIMATION_LEFT_RIGHT_FRAMES);
                    break;
                case SCANCODE_CURSORBLOCKLEFT:
                    ToolbarAnimateMove(hWnd, &toolbar_rect, MOVE_RIGHT, (int*)&curr_item, ANIMATION_LEFT_RIGHT_INTERVAL, ANIMATION_LEFT_RIGHT_FRAMES);
                    break;
                case SCANCODE_ENTER:
                    ToolbarOnPressEnter(hWnd, curr_item);
                    break;
            }
            break;
        case MSG_LBUTTONDOWN:
            ToolbarOnLButtonDown (hWnd, LOWORD (lParam), HIWORD (lParam), TOOLBAR_SELECTED_BLOCK,TOOLBAR_NOT_SELECTED_BLOCK, (int*)&curr_item);
            break;
        case MSG_CLOSE:
            OnMsgClose(hWnd);
            break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}

int my_hook (void* p_hWnd, HWND dst_wnd, int msg, WPARAM wParam, LPARAM lParam)
{
#if 0
    if( msg == MSG_KEYUP && wParam == SCANCODE_ESCAPE){
        OnMsgClose( *(HWND*)p_hWnd );
        return HOOK_STOP;
    }
#endif
    return HOOK_GOON;
}

HWND InitToolbar(HWND hWnd)
{
    static HWND hToolbarWnd;
    debug(" ******** InitToolBar():   start!\n");
    debug("RECTW(g_rcScr)=%d,RECTH(g_rcScr)=%d\n",RECTW(g_rcScr),RECTH(g_rcScr));

    float scale;
    scale = (1.0*RECTW(g_rcScr)/RECTH(g_rcScr))/(1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT);
    if(scale<1.0)
    {
        scale = 1.0;
    }
    // init toolbar position in screen
    toolbar_rect.left   = 0;
    toolbar_rect.top    = RECTH(g_rcScr)*(1-TOOLBAR_HEIGHT_SCALE*scale);
    toolbar_rect.right  = RECTW(g_rcScr);
    toolbar_rect.bottom = RECTH(g_rcScr);
    debug("     InitToolBar(): toolbar_rect.left=%d,top=%d,right=%d,bottom=%d \n", toolbar_rect.left,toolbar_rect.top,toolbar_rect.right,toolbar_rect.bottom);

#if TOOLBAR_WITH_ZOOM
    HDC hdc_mdolphin_wnd;
    if(g_mdolphin_main_hwnd)
    {
        hdc_mdolphin_wnd = GetClientDC(g_mdolphin_main_hwnd); 
#ifdef WITH_BKGND_PIC
        BitBlt (hdc_mdolphin_wnd, toolbar_rect.left, toolbar_rect.top, RECTW(toolbar_rect), RECTH(toolbar_rect), g_dc_bkgnd, toolbar_rect.left, toolbar_rect.top, 0);
#endif
        ReleaseDC(hdc_mdolphin_wnd);
    }
#endif
    hToolbarWnd = CreateMenuWnd( &toolbar_rect, ToolbarProc, hWnd);
    if (hToolbarWnd == HWND_INVALID)
    {
        error("Create toolbar window!\n");
        return HWND_INVALID;
    }
    set_window_seconddc(hToolbarWnd, toolbar_rect.left, toolbar_rect.top, RECTW(toolbar_rect),  RECTH(toolbar_rect) );
#if HAVE_ANIMATION
    InitWidth(hToolbarWnd);

    //start create window effect
    SVGUI_HEADER_I  *svgui_header_i;
    HDC hdc_client  = GetClientDC( hToolbarWnd );
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hToolbarWnd );
    ToolBarSetSvguiHeaderI(svgui_header_i, (int)curr_item, hdc_client);
    ReleaseDC(hdc_client);
    AnimationMoveWndUpDown ( hToolbarWnd, toolbar_rect, MOVE_UP,  ANIMATION_INTERVAL, ANIMATION_FRAMES );
#endif
    InvalidateRect(hToolbarWnd, NULL, TRUE);
    ShowWindow (hToolbarWnd, SW_SHOWNORMAL);
    
    old_hook = RegisterKeyMsgHook (&hToolbarWnd, my_hook);
    return hToolbarWnd;
}
