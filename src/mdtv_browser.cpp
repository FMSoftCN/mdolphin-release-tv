#include <unistd.h>
#include "svgui.h"

#include "mdtv_app.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_browser.h"
#include "mdtv_toolbar.h"
#include "mdtv_toolbar_zoom.h"
#include "mdtv_ime.h"
#include "mdtv_keyboard.h"
#include "mdtv_key.h"
#include "mdtv_animation.h"
#include "mdtv_fav.h"
#include "mdtv_fav_file.h"
#include "mdtv_tooltipwin.h"

#define NDEBUG  1
#include <mdolphin/mdolphin.h>

extern HWND g_hMainWnd ;
#define HAVE_BROWSER_TITLE  1

#define MOUSE_MOVE_UP_BORDER        20
#define MOUSE_MOVE_DOWN_BORDER      MOUSE_MOVE_UP_BORDER
#define MOUSE_MOVE_LEFT_BORDER      20
#define MOUSE_MOVE_RIGHT_BORDER     50

#define MOUSE_SCROLL_SPEED          20
#define MOUSE_SCROLL_SPEEDUP_SPEED  50

#define MOUSE_MOVE_SPEED_X          10
#define MOUSE_MOVE_SPEED_Y          10
#define MOUSE_MOVE_SPEEDUP_SPEED_X  40
#define MOUSE_MOVE_SPEEDUP_SPEED_Y  40

#define IME_LIVE_TIME_ONCE          500 //300*10 ms

typedef enum{
    MDStatusRoller,
    MDStatusMouse,
/*
   *Caijun.Lee remove statuslink
    MDStatusLink,
*/
    MDStatusMaxItem,
}MdolphinStatus;

typedef struct _MD_STATUS
{
    //check if it is in edit frame
    bool m_is_editable;
    // mdolphin status
    MdolphinStatus m_md_status;
    // ime timer id
    int m_ime_timer_id;
} MD_STATUS;


HWND g_mdolphin_main_hwnd;
HWND g_mdolphin_hwnd;
extern HWND sg_imehMainWnd;
extern HWND sg_phone_ime_hwnd;

extern int g_win_width;
extern int g_win_height;
extern void my_error_code_callback (HWND hWnd, int errCode, const char* failedUrl);
extern int prompt_box(HWND parent, const char *info, const char *definp, char *buf, int buflen);
static MD_STATUS g_md_status = {FALSE, MDStatusRoller, 0};
static BITMAP progressBmp;

extern char home_url[];

#if MDTV_ENABLE_SOFTIME 
extern HWND  g_kbwnd ;
#endif

char * get_local_url(const char *url )
{
    char *current_dir = get_current_dir_name();

    if (current_dir) {
        int len;
           len = strlen("file://")+strlen(current_dir)+strlen(url);
        char *fileName = (char *)malloc(len+1);
        if (fileName) {
            memset(fileName, 0, len+1);
            strncpy(fileName, "file://", strlen("file://"));
            strncat(fileName, current_dir, strlen(current_dir));
            strncat(fileName, url, strlen(url));
            fileName[len]='\0';
            free(current_dir);
            return fileName;
        }
    }
    return NULL;
}

#define CARET_REPLY "/var/tmp/caretreply"

#if HAVE_BROWSER_TITLE
#define BROWSER_TITLE_X             0
#define BROWSER_TITLE_Y             0
#define BROWSER_TITLE_WIDTH         (DEFAULT_SCREEN_WIDTH-2*BROWSER_TITLE_X)
#define BROWSER_TITLE_HEIGHT        (43)
//#define BROWSER_TITLE_IMAGE_WIDTH   (40)
#define BROWSER_TITLE_IMAGE_WIDTH   (40+5+7)
#define BROWSER_TITLE_IMAGE_HEIGHT  (BROWSER_TITLE_HEIGHT)
#define BROWSER_TITLE_CLOSE_RIGHT_SPACE     (25)
#define BROWSER_TITLE_IMAGE_INTERNAL_SPACE  (0)

#define BROWSER_TITLE_BKGND_2_WIDTH         (BROWSER_TITLE_WIDTH-6*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-5*BROWSER_TITLE_IMAGE_INTERNAL_SPACE)
#define BROWSER_TITLE_BKGND_2_HEIGHT        (BROWSER_TITLE_HEIGHT)

#define BROWSER_TITLE_TEXT_IDX      (1)

#define BROWSER_TITLE_PRDRD_BLOCK_COLOR_BK   (0x000000)
enum {
    BROWSER_TITLE_PRDRD_BLOCK_BKGND_1,
    BROWSER_TITLE_PRDRD_BLOCK_BKGND_2,
    BROWSER_TITLE_PRDRD_BLOCK_IMAGE,
};
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // BROWSER_TITLE_PRDRD_BLOCK_BKGND_1
    {
        BROWSER_TITLE_WIDTH,  // width
        BROWSER_TITLE_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, BROWSER_TITLE_WIDTH/2, BROWSER_TITLE_WIDTH,
        0x585858, 0x191919, 0x101010,    //
        //0xFFD67E, 0xFFD67E, 0xFFD67E,    //
        0,                  //int border_width;
        0xFFD67E,           //RGBCOLOR border_color;
        8,                 //int corner;
        BROWSER_TITLE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // BROWSER_TITLE_PRDRD_BLOCK_BKGND_2
    {
        BROWSER_TITLE_WIDTH-6*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-5*BROWSER_TITLE_IMAGE_INTERNAL_SPACE,  // width
        BROWSER_TITLE_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, (BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-2*BROWSER_TITLE_IMAGE_INTERNAL_SPACE)/2, BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-2*BROWSER_TITLE_IMAGE_INTERNAL_SPACE,
        0x585858, 0x191919, 0x101010,    //
        //0xFFD67E, 0xFFD67E, 0xFFD67E,    //
        //0xE3A841, 0xE3A841, 0xE3A841,    //
        0,                  //int border_width;
        0xFFD67E,           //RGBCOLOR border_color;
        0,                 //int corner;
        BROWSER_TITLE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // BROWSER_TITLE_PRDRD_BLOCK_IMAGE
    {
        BROWSER_TITLE_IMAGE_WIDTH,  // width
        BROWSER_TITLE_IMAGE_HEIGHT, // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, (BROWSER_TITLE_IMAGE_WIDTH)/2, BROWSER_TITLE_IMAGE_WIDTH,
        0x585858, 0x191919, 0x101010,    //
        //0x191919, 0x191919, 0x101010,    //
        0,                  //int border_width;
        0x585858,           //RGBCOLOR border_color;
        0,                 //int corner;
        BROWSER_TITLE_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

//---- SVGUI_TEXT_AREA_T
static SVGUI_TEXT_AREA_T ptr_title_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE,BROWSER_TITLE_HEIGHT},
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFFFFFF,               // RGBCOLOR color;
        BROWSER_TITLE_TEXT_IDX,                      // int idx_font;
        NULL,         // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_ime_status_block_text_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFFFF00,               // RGBCOLOR color;
        1,//BROWSER_TITLE_TEXT_IDX,                      // int idx_font;
        "ABC",         // char* text;
    },
};

//---- SVGUI_IMAGE
static SVGUI_IMAGE_T ptr_status_link_block_images[]={
    { 14, 14, 32, "browser/link_14x14.png",},
    { 17, 18, 32, "browser/link_17x18.png",},
    { 20, 21, 32, "browser/link_20x21.png",},
};
static SVGUI_IMAGE_T ptr_status_mouse_block_images[]={
    { 14, 14, 32, "browser/mouse_14x14.png",},
    { 17, 18, 32, "browser/mouse_17x18.png",},
    { 17, 23, 32, "browser/mouse_17x23.png",},
};
static SVGUI_IMAGE_T ptr_status_roller_block_images[]={
    { 14, 14, 32, "browser/roller_14x14.png",},
    { 17, 18, 32, "browser/roller_17x18.png",},
    { 18, 23, 32, "browser/roller_18x23.png",},
};
static SVGUI_IMAGE_T ptr_zoom_in_block_images[]={
    { 13, 14, 32, "browser/zoom_in_13x14.png",},
    { 17, 18, 32, "browser/zoom_in_17x18.png",},
    { 21, 22, 32, "browser/zoom_in_21x22.png",},
};
static SVGUI_IMAGE_T ptr_zoom_out_block_images[]={
    { 13, 14, 32, "browser/zoom_out_13x14.png",},
    { 17, 18, 32, "browser/zoom_out_17x18.png",},
    { 21, 22, 32, "browser/zoom_out_21x22.png",},
};
static SVGUI_IMAGE_T ptr_close_block_images[]={
    { 14, 14, 32, "browser/close_14x14.png",},
    { 17, 18, 32, "browser/close_17x18.png",},
    { 21, 22, 32, "browser/close_21x22.png",},
};
//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T ptr_status_link_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        TABLESIZE(ptr_status_link_block_images),                      // int nr_images;
        ptr_status_link_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_status_mouse_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        TABLESIZE(ptr_status_mouse_block_images),                      // int nr_images;
        ptr_status_mouse_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_status_roller_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        TABLESIZE(ptr_status_roller_block_images),                      // int nr_images;
        ptr_status_roller_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_zoom_in_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_zoom_in_block_images),                      // int nr_images;
        ptr_zoom_in_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_zoom_out_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_zoom_out_block_images),                      // int nr_images;
        ptr_zoom_out_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_close_block_images_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_IMAGE_WIDTH, BROWSER_TITLE_IMAGE_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        TABLESIZE(ptr_close_block_images),                      // int nr_images;
        ptr_close_block_images,  // SVGUI_IMAGE* images;
    },
};
#if 1
enum{
    BLOCK_BKGND_0,
    BLOCK_BKGND_1,
    BLOCK_STATUS_ROLLER,
    BLOCK_STATUS_MOUSE,
    BLOCK_STATUS_LINK,
    BLOCK_IME_STATUS,
    BLOCK_ZOOM_IN,
    BLOCK_ZOOM_OUT,
    BLOCK_CLOSE,
};
#endif
static SVGUI_BLOCK_T ptr_blocks[]={
    {
        BLOCK_BKGND_0,     // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, BROWSER_TITLE_WIDTH, BROWSER_TITLE_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_BKGND_1,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_BKGND_1,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_IMAGE_WIDTH, 0, BROWSER_TITLE_WIDTH-5*BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-5*BROWSER_TITLE_IMAGE_INTERNAL_SPACE, BROWSER_TITLE_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_BKGND_2,// int idx_prdrd_block;
        TABLESIZE(ptr_title_block_text_areas),// int nr_text_areas;
        ptr_title_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_STATUS_ROLLER,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-5*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_status_roller_block_images_areas),  // int nr_image_areas;
        ptr_status_roller_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_STATUS_MOUSE,     // int id;
        FALSE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-5*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_status_mouse_block_images_areas),  // int nr_image_areas;
        ptr_status_mouse_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_STATUS_LINK,     // int id;
        FALSE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-5*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-4*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_status_link_block_images_areas),  // int nr_image_areas;
        ptr_status_link_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_IME_STATUS,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-4*BROWSER_TITLE_IMAGE_WIDTH-3*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-3*BROWSER_TITLE_IMAGE_WIDTH-3*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        FALSE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        TABLESIZE(ptr_ime_status_block_text_areas),// int nr_text_areas;
        ptr_ime_status_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        0,  // int nr_image_areas;
        NULL, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_ZOOM_IN,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-3*BROWSER_TITLE_IMAGE_WIDTH-2*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-2*BROWSER_TITLE_IMAGE_WIDTH-2*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_zoom_in_block_images_areas),  // int nr_image_areas;
        ptr_zoom_in_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_ZOOM_OUT,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-2*BROWSER_TITLE_IMAGE_WIDTH-1*BROWSER_TITLE_IMAGE_INTERNAL_SPACE-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE-1*BROWSER_TITLE_IMAGE_INTERNAL_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_zoom_out_block_images_areas),  // int nr_image_areas;
        ptr_zoom_out_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        BLOCK_CLOSE,     // int id;
        TRUE,                   // BOOL is_visible;
        {BROWSER_TITLE_WIDTH-BROWSER_TITLE_IMAGE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE, 0, BROWSER_TITLE_WIDTH-BROWSER_TITLE_CLOSE_RIGHT_SPACE, BROWSER_TITLE_IMAGE_HEIGHT}, // RECT rc;
        TRUE,               // BOOL is_hotspot;
        BROWSER_TITLE_PRDRD_BLOCK_IMAGE,// int idx_prdrd_block;
        0,// int nr_text_areas;
        NULL, // SVGUI_TEXT_AREA_T* text_areas;
        TABLESIZE(ptr_close_block_images_areas),  // int nr_image_areas;
        ptr_close_block_images_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
};

//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_browser = {
    BROWSER_TITLE_WIDTH,  // width
    BROWSER_TITLE_HEIGHT, // height
    BROWSER_TITLE_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    0xFFFFFF,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_t),// nr_prdrd_blocks;
    ptr_prdrd_block_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;
    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks),// int nr_blocks;
    ptr_blocks,         // SVGUI_BLOCK_T* blocks;
};
#endif

static const char* default_language[]=
{
      "zh",
      "en"
};

HWND mdtv_CreateMdolphinWindow(HWND hParent)
{

#if HAVE_BROWSER_TITLE 
#define MDOLPHIN_LEFT  0
#define MDOLPHIN_TOP    0
#define MDOLPHIN_HEIGHT (g_win_height - BROWSER_TITLE_HEIGHT*scale_y)
#define MDOLPHIN_WIDTH  (g_win_width - MDOLPHIN_LEFT*2)
#else
#define MDOLPHIN_LEFT   0
#define MDOLPHIN_TOP    0
#define MDOLPHIN_HEIGHT (g_win_height - MDOLPHIN_TOP)
#define MDOLPHIN_WIDTH  (g_win_width - MDOLPHIN_LEFT*2)
#endif

    g_mdolphin_hwnd = CreateWindow(MDOLPHIN_CTRL,
            "",
            WS_VISIBLE | WS_CHILD,
            IDC_MDOLPHIN,
            MDOLPHIN_LEFT, MDOLPHIN_TOP, MDOLPHIN_WIDTH, MDOLPHIN_HEIGHT, hParent, 0);
    //set up font
    SETUP_INFO setup_info;
    mdolphin_fetch_setup(g_mdolphin_hwnd, &setup_info);
   

    setup_info.default_encoding = MD_CHARSET_UTF8;
    setup_info.medium_fontsize = 18 ;
    setup_info.medium_fixed_fontsize = 18;

   //Caijun.Lee add 2010-04-06 
    memset(setup_info.default_language, 0, sizeof(setup_info.default_language));
    strcpy(setup_info.default_language, default_language[0]);
    

    mdolphin_import_setup(g_mdolphin_hwnd, &setup_info);
    //mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, 200);
    return g_mdolphin_hwnd; 
}

#if 1
static void stopImeTimer(HWND hWnd, int *timer_id)
{
#if 1
    if (*timer_id) {
        KillTimer(hWnd, *timer_id);
        *timer_id = 0;
    }
#else
        KillTimer(hWnd, timer_id);
#endif
}
static void start_ime_timer(HWND hWnd, int *timer_id)
{
    static unsigned int t_id = 1;
    stopImeTimer(hWnd, timer_id);
    while (IsTimerInstalled(HWND_NULL, t_id))
        t_id++;

    if (SetTimer(hWnd, t_id, IME_LIVE_TIME_ONCE)){
        *timer_id = t_id;
        t_id++;
    }else
        fprintf(stderr, "mdtv: SetTimer failed !\n");
}
static void hide_ime_window(void)
{
    if (mdtv_ShowIme (IME_CMD_GET_STATUS, (HWND)NULL)) //open status
    {
        mdtv_ShowIme (IME_CMD_CLOSE, (HWND)NULL); //close ime
        mdtv_ShowSoftIme (FALSE); //close soft ime
    }
}
static void show_ime_window(void)
{
    if (!mdtv_ShowIme (IME_CMD_GET_STATUS, (HWND)NULL)){ //get status
        mdtv_ShowIme (IME_CMD_OPEN, (HWND)NULL); //display ime
        mdtv_ShowSoftIme (TRUE); //open soft ime
    }//if
    return;
}
static void close_ime_wnd(void)
{
#if MDTV_ENABLE_SOFTIME
    if (g_kbwnd){
        PostMessage (g_kbwnd, MSG_CLOSE, 0, 0);
        g_kbwnd = NULL;
    }
#else
    mdtv_DestroyPhoneIme (NULL);
#endif
}
#endif
static char prompt_text [256];

static HWND tooltip_hwnd = 0;
static void my_tooltip(HWND hWnd, int x, int y, const char *text, BOOL bShow)
{
    if (bShow) {
        ClientToScreen(hWnd, &x, &y);
        y += GetSysCharHeight();
        if (!tooltip_hwnd)
            tooltip_hwnd = mdolphin_createToolTipWin(hWnd, x, y, 2000, text);
        else
            mdolphin_resetToolTipWin(tooltip_hwnd, x, y, text);
    } else 
        mdolphin_hideToolTip(tooltip_hwnd);
}

static void my_message_callback (HWND parent, const char * text, const char * caption)
{
    MessageBox (parent, text, caption, MB_OK|MB_ICONINFORMATION);
}

static BOOL my_confirm_callback (HWND parent, const char * text, const char * caption)
{
    if (MessageBox (parent, text, caption, MB_OKCANCEL|MB_ICONQUESTION) == IDOK)
        return TRUE;
    return FALSE;
}

static char * my_prompt_callback (HWND parent, const char* text, const char* defaultval, const char* caption)
{
    memset (prompt_text, 0 , sizeof(prompt_text));
    if (prompt_box(parent, text, defaultval, prompt_text, sizeof(prompt_text)))
        return prompt_text;
    return NULL;
}

#if defined (_MGRM_PROCESSES) 
static int conn_fd_write = -1;
static void send_reply(const RECT* rect)
{
    char msg[32] = {0};
    int msg_len = 30;

    //len:30 format: X=111Y=111R=111B=111\t...\t
    sprintf(msg, "X=%dY=%dR=%dB=%d", rect->left, rect->top, rect->right, rect->bottom);
    int len = strlen(msg);
    int i = 0;
    for (; i < msg_len - len; ++i)
    {
        strcat(msg, "\t");
    }
    md_debug("strlen msg is %d\n", strlen(msg));
    md_debug("msg is %s\n", msg);

    int ret = -1;
    int timeout = 10;
    if (conn_fd_write != -1)
        ret = sock_write_t (conn_fd_write, msg, msg_len, timeout);
    if (ret != msg_len)
    {    
        conn_fd_write = cli_conn(CARET_REPLY, 'c');
        if (conn_fd_write >= 0)
            ret = sock_write_t (conn_fd_write, msg, msg_len, timeout);
    }    
}

static void my_get_caret(const RECT* caret)
{
    static RECT rect = {0} ;
    if ( (caret->left !=rect.left) || (caret->top != rect.top) )
    {
        md_debug("rect [%d %d %d %d]\n", caret->left, caret->top, caret->right, caret->bottom);
        SetRect(&rect, caret->left, caret->top, caret->right, caret->bottom);
        send_reply(caret);
    }
}
#elif defined(_MGRM_THREADS)

static RECT sg_caret_rect = {0} ;
static void my_get_caret(const RECT* caret)
{
    if ( (caret->left !=sg_caret_rect.left) || (caret->top != sg_caret_rect.top) )
    {
        //md_debug("my_get_caret(): sg_caret_rect [%d %d %d %d]\n", caret->left, caret->top, caret->right, caret->bottom);
        SetRect(&sg_caret_rect, caret->left, caret->top, caret->right, caret->bottom);
        mdtv_NotifyCursorPos (&sg_caret_rect);
    }
}
#endif

int mdtv_set_browser_title_ime_status( HWND hBrowserMainWnd, const char *str_ime_status )
{
    SVGUI_HEADER_I  *svgui_header_i;
    SVGUI_BLOCK_I   *block;
    HWND hBrowserTitleWnd;

    hBrowserTitleWnd = GetWindowAdditionalData2(hBrowserMainWnd);
    if( !hBrowserTitleWnd ){
        return -1;
    }
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hBrowserTitleWnd );
    if (svgui_header_i == NULL){
        return -1;
    }

    block = svgui_get_block(svgui_header_i, BLOCK_IME_STATUS); 
    if (block == NULL){
        return -1;
    }
    block->text_areas->text = str_ime_status;
    InvalidateRect(hBrowserTitleWnd, &(block->rc), TRUE);
    return 0;
}
static BOOL change_browser_ime_status(HWND hWnd, BOOL bstate, EditorElementInfo editor_info)
{
    g_md_status.m_is_editable =  bstate;
    if( !bstate ){
        hide_ime_window();
    }
    return TRUE;
}

BOOL get_browser_ime_status()
{
    return g_md_status.m_is_editable;
}

#if HAVE_BROWSER_TITLE
static void my_set_location_text (HWND hWnd, const char * text)
{
    hide_ime_window();
    g_md_status.m_is_editable = FALSE;
    if (!text) //you minigui not support utf-8 codecs
        return;
	BrowserTitleWndInfo *p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(hWnd);
    if( p_info ){
        if( p_info->m_url )
            free(p_info->m_url);
        p_info->m_url = strdup(text);
#if ENABLE_FAV
        if(p_info->m_fav_info){
            p_info->m_fav_info->m_url = p_info->m_url;
        }
#endif
    }
}
#endif

static HWND my_create_new_window(const char* url, DWORD styles, int x, int y, int width, int height)
{
	mdolphin_navigate(g_mdolphin_hwnd, NAV_GOTO, url, FALSE);

	return g_mdolphin_hwnd;
}

#if HAVE_BROWSER_TITLE
static void my_set_title_text(HWND hWnd, const char * text)
{
    if (!text) //you minigui not support utf-8 codecs
        return;
	BrowserTitleWndInfo *p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(hWnd);
    if( p_info ){
        if( p_info->m_title )
            free(p_info->m_title);
        p_info->m_title = strdup(text);
#if ENABLE_FAV
        if(p_info->m_fav_info){
            p_info->m_fav_info->m_title = p_info->m_title;
        }
#endif
        if(p_info->m_hTitleWnd != HWND_INVALID){
            SVGUI_HEADER_I *header = NULL;
            SVGUI_BLOCK_I* block = NULL;
            header = (SVGUI_HEADER_I *)GetWindowAdditionalData(p_info->m_hTitleWnd);
            if (header == NULL)
                return;
            block = svgui_get_block (header, 1);
            if (block == NULL)
                return;
            if( block->text_areas[0].text ){
                free((void*)(block->text_areas[0].text));
            }
            block->text_areas[0].text = strdup(text);
            InvalidateRect(p_info->m_hTitleWnd, &(block->rc), TRUE);
        }
    }
    return;
}

static void my_set_loading_status (HWND hWnd, BOOL loading, unsigned int progress)
{
    // FixMe: for bug: mdolphin-core loads web page for long time when progress is 89%
    if( progress >= 89 ){
        progress = 100;
        loading = FALSE;
    }

    BrowserTitleWndInfo *p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(hWnd);
    if( p_info ){
        p_info->m_loading   = loading;
        p_info->m_progress  = progress;
        if( p_info->m_hTitleWnd  != HWND_INVALID){
            if(loading){
                InvalidateRect(p_info->m_hTitleWnd, NULL, FALSE);
            }else{
                InvalidateRect(p_info->m_hTitleWnd, NULL, TRUE);
            }
        }
    }
}
#endif
static void set_callback_info(HWND hWnd)
{
    CB_INFO cb_info;
    memset (&cb_info, 0, sizeof(CB_INFO));

    ////////////////////////////////////    
    cb_info.CB_SET_LOCATION = NULL;

    cb_info.CB_SET_TOOLTIP = my_tooltip;
    cb_info.CB_MESSAGE_BOX = my_message_callback;
    cb_info.CB_CONFIRM_BOX = my_confirm_callback;
    cb_info.CB_PROMPT_BOX = my_prompt_callback;
    cb_info.CB_CHOOSEFILE_BOX = NULL;
    cb_info.CB_ERROR = my_error_code_callback;
    cb_info.CB_SAVE_FILE_DATA = NULL;
//#if defined (_MGRM_PROCESSES)
    cb_info.CB_GET_CARET_RECT = my_get_caret;
//#endif
    cb_info.CB_SET_IME_STATUS = change_browser_ime_status;
    
#if HAVE_BROWSER_TITLE
    cb_info.CB_SET_LOCATION = my_set_location_text;
#endif

#if HAVE_BROWSER_TITLE
    cb_info.CB_SET_TITLE = my_set_title_text;
    cb_info.CB_SET_LOADING_STATUS = my_set_loading_status;
#endif
    cb_info.CB_OPEN_WINDOW = my_create_new_window;
    mdolphin_set_callback_info(g_mdolphin_hwnd, &cb_info);
}

#if HAVE_BROWSER_TITLE
static void OnMsgPaintBrowserTitleWnd(HWND hWnd)
{
    HDC hdc;
    BrowserTitleWndInfo *p_info;

    // display progress bar
    hdc = GetClientDC(hWnd);
    p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(g_mdolphin_hwnd);
    if( p_info && p_info->m_loading && p_info->m_progress)
    {
        SVGUI_HEADER_I *header = NULL;
        header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
        if (header == NULL){
            return;
        }
        FillBoxWithBitmap(hdc, BROWSER_TITLE_X, BROWSER_TITLE_Y, BROWSER_TITLE_IMAGE_WIDTH+BROWSER_TITLE_BKGND_2_WIDTH*scale_x*p_info->m_progress/100, header->height, &progressBmp);
    }

    // display text foreground
    SVGUI_HEADER_I *header = NULL;
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(p_info->m_hTitleWnd);
    if (header == NULL)
        return;
    RECT rect;
    SetRect(&rect,BROWSER_TITLE_IMAGE_WIDTH*scale_x, BROWSER_TITLE_Y*scale_x, (BROWSER_TITLE_IMAGE_WIDTH+BROWSER_TITLE_BKGND_2_WIDTH)*scale_x, BROWSER_TITLE_HEIGHT*scale_x );
    SetBkMode(hdc, BM_TRANSPARENT);
    SelectFont (hdc, header->font_infos[BROWSER_TITLE_TEXT_IDX].log_font);
    SetTextColor (hdc, PIXEL_lightwhite);
    DrawText(hdc, p_info->m_title, -1, &rect, DT_LEFT |  DT_VCENTER | DT_SINGLELINE);
    ReleaseDC(hdc);
}

static void OnLButtonDown (HWND hWnd, int x_pos, int y_pos )
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;
    HWND hBrowserWnd;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if (block == NULL)
        return ;

    hBrowserWnd = GetWindowAdditionalData2(hWnd);
    if( !hBrowserWnd )
        return ;
    switch(block->id){
        case BLOCK_ZOOM_IN:
            SendMessage(hBrowserWnd,MSG_KEYDOWN,SCANCODE_F8,0);
            break;
        case BLOCK_ZOOM_OUT:
            SendMessage(hBrowserWnd,MSG_KEYDOWN,SCANCODE_F9,0);
            break;
        case BLOCK_CLOSE:
            SendMessage(hBrowserWnd,MSG_CLOSE,0,0);
            SendMessage(hWnd,MSG_CLOSE,0,0);
            close_ime_wnd();
            InitToolbar(g_hMainWnd);
            break;
        default:
            break;
    }
    return ;
}
static int BrowserTitleWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hBrowserWnd;
    BrowserTitleWndInfo *p_info;
    switch(message)
    {
        case MSG_CREATE:
            if (LoadBitmap(HDC_SCREEN, &progressBmp, "res/browser/progress.png")) 
                return -1;
            break;
        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_ESCAPE:
                    hBrowserWnd = GetWindowAdditionalData2(hWnd);
                    if( hBrowserWnd ){
                        SendMessage(hBrowserWnd,MSG_CLOSE,0,0);
                    }
                    SendMessage(hWnd,MSG_CLOSE,0,0);
                    close_ime_wnd();
                    InitToolbar(g_hMainWnd);
                    break;
            }
            break;
        case MSG_LBUTTONDOWN:
            OnLButtonDown (hWnd, LOWORD (lParam), HIWORD (lParam));
            break;
        case MSG_PAINT:
            OnMsgPaintBrowserTitleWnd(hWnd);
            break;
        case MSG_CLOSE:
            p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(g_mdolphin_hwnd);
            if(p_info){
                if(p_info->m_title)
                    free(p_info->m_title);
                if(p_info->m_url)
                    free(p_info->m_url);
                free(p_info);
                SetWindowAdditionalData(g_mdolphin_hwnd, (DWORD)0);
            }
            DestroyMainWindow(hWnd);
            break;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}
#endif

static HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc,const SVGUI_HEADER_T *svgui_header_t, HWND hHostingWnd )
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST ;
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

#if HAVE_BROWSER_TITLE
HWND create_browser_title_wnd(HWND mdolphin_main_hwnd, HWND hBrowserWnd )
{
    HWND hMDolphinTitleWnd;
    RECT mdolphin_title_wnd_rect;

    SetRect(&mdolphin_title_wnd_rect, BROWSER_TITLE_X*scale_x, \
            BROWSER_TITLE_Y*scale_x, (BROWSER_TITLE_WIDTH+BROWSER_TITLE_X)*scale_x, (BROWSER_TITLE_HEIGHT+BROWSER_TITLE_Y)*scale_x);
    hMDolphinTitleWnd = CreateMenuWnd( &mdolphin_title_wnd_rect, BrowserTitleWndProc, &svgui_header_t_browser, HWND_DESKTOP);
    if (hMDolphinTitleWnd == HWND_INVALID){
        fprintf(stderr, "[Error]Create browser title window!\n");
        return HWND_INVALID;
    }
    SetWindowAdditionalData2(hMDolphinTitleWnd, mdolphin_main_hwnd);
    SetWindowAdditionalData2(mdolphin_main_hwnd, hMDolphinTitleWnd);

    BrowserTitleWndInfo *pBrowserTitleWndInfo = (BrowserTitleWndInfo *)malloc(sizeof(BrowserTitleWndInfo));
    if( !pBrowserTitleWndInfo ){
        fprintf(stderr, "[Error] malloc browser title window info!\n");
        return NULL;
    }
    memset(pBrowserTitleWndInfo,0, sizeof(BrowserTitleWndInfo));
    pBrowserTitleWndInfo->m_hTitleWnd = hMDolphinTitleWnd;
#if ENABLE_FAV
    MDTV_FAV_INFO *p_fav_info = (MDTV_FAV_INFO *)malloc(sizeof(MDTV_FAV_INFO));
    if( !p_fav_info ){
        fprintf(stderr, "[Error] malloc browser fav info!\n");
        return NULL;
    }
    memset(p_fav_info,0, sizeof(MDTV_FAV_INFO));
    pBrowserTitleWndInfo->m_fav_info = p_fav_info;
#endif

    SetWindowAdditionalData(hBrowserWnd, (DWORD)pBrowserTitleWndInfo);

    return hMDolphinTitleWnd;
}
#endif

static int set_title_wnd_browser_status(HWND hWnd, MdolphinStatus i_mdolphin_status)
{
    SVGUI_HEADER_I  *svgui_header_i;
    SVGUI_BLOCK_I   *block;
    int block_id;
    if( !hWnd ){
        return -1;
    }
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );

    for(block_id=(int)BLOCK_STATUS_ROLLER; block_id<(int)MDStatusMaxItem+BLOCK_STATUS_ROLLER; block_id++){
        block = svgui_get_block(svgui_header_i, block_id) ; 
        if (block == NULL){
            return -1;
        }
        if(((int)i_mdolphin_status+BLOCK_STATUS_ROLLER) == block->id ){
            block->is_visible = TRUE;
        }else{
            block->is_visible = FALSE;
        }
    }
    InvalidateRect(hWnd, &(block->rc), TRUE);
    return 0;

}
static int change_title_wnd_browser_status(HWND hWnd, MdolphinStatus *s_mdolphin_status)
{
    SVGUI_HEADER_I  *svgui_header_i;
    SVGUI_BLOCK_I   *block;
    if( !hWnd ){
        return -1;
    }

    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );

    block = svgui_get_block(svgui_header_i, (int)*s_mdolphin_status+BLOCK_STATUS_ROLLER) ; 
    if (block == NULL){
        return -1;
    }
    block->is_visible = FALSE;

    *s_mdolphin_status = (MdolphinStatus)(((int)*s_mdolphin_status+1)%MDStatusMaxItem);

    block = svgui_get_block(svgui_header_i, (int)*s_mdolphin_status +BLOCK_STATUS_ROLLER); 
    if (block == NULL){
        return -1;
    }
    block->is_visible = TRUE;

    InvalidateRect(hWnd, &(block->rc), TRUE);
    return 0;
}
static int move_mouse_browser(HWND hWnd,  MdolphinStatus mdolphin_status, int dirrect, int mouse_move_speed, int scrollbar_move_speed)
{
    POINT   point;
    HWND    hBrowserTitleWnd;
    BOOL    is_cursor_in_rect = TRUE;
    int     title_height = 0;
    int     x_pos = 0;
    int     y_pos = 0;
    RECT    rc;

    GetCursorPos( &point );
    hBrowserTitleWnd = GetWindowAdditionalData2(hWnd);
    if( GetWindowRect( hBrowserTitleWnd, &rc) ){
        title_height = RECTH(rc);
    }
    mdolphin_get_contents_position(g_mdolphin_hwnd, &x_pos, &y_pos);
    switch( dirrect ){
        case move_left:
            if( point.x < MOUSE_MOVE_LEFT_BORDER ){
                is_cursor_in_rect = FALSE;
            }
            x_pos   -= scrollbar_move_speed ;
            point.x -= mouse_move_speed;
            break;
        case move_right:
            if( point.x > g_win_width - MOUSE_MOVE_RIGHT_BORDER ){
                is_cursor_in_rect = FALSE;
            }
            x_pos   += scrollbar_move_speed ;
            point.x += mouse_move_speed;
            break;
        case move_up:
            if( point.y < title_height + MOUSE_MOVE_UP_BORDER ){
                is_cursor_in_rect = FALSE;
            }
            y_pos   -= scrollbar_move_speed ;
            point.y -= mouse_move_speed;
            if (point.y < title_height) {
                point.y = title_height;
            }
            break;
        case move_down:
            if( point.y >  ( g_win_height - MOUSE_MOVE_DOWN_BORDER) ){
                is_cursor_in_rect = FALSE;
            }
            y_pos   += scrollbar_move_speed;
            point.y += mouse_move_speed;
            break;
        default:
            break;
    }

    if( mdolphin_status == MDStatusRoller || ( mdolphin_status == MDStatusMouse && !is_cursor_in_rect ) ){
        if (!GetFocusChild(g_mdolphin_hwnd)){
            mdolphin_set_contents_position(g_mdolphin_hwnd, x_pos, y_pos);
            return 0;
        }
    }else if( mdolphin_status == MDStatusMouse && is_cursor_in_rect){
        SetCursorPos( point.x, point.y );
    }
    return 0;
}

static int alwaysKeyPressCnt = 0;
static int MDolphinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    char *url = NULL;
    HWND hBrowserWnd = HWND_NULL;
#if ENABLE_FAV
    int ret_val;
#endif
    int percent;
#if HAVE_BROWSER_TITLE
    HWND    hBrowserTitleWnd;
    POINT   point;
    int     title_height = 0;
    RECT    rc;
    int     x_pos, y_pos;
#endif
    MSG     msg;
    RECT    rc1;
    int     speedy = 0;
    int     speedx = 0;

    if( GetWindowRect(hWnd, &rc1) ){
        speedy = RECTH(rc1);
        speedx = RECTW(rc1);
    }

    switch (message)
    {
        case MSG_CREATE:
            {
                
                MAINWINCREATE  *pCreateInfo;
                pCreateInfo = (MAINWINCREATE  *)(lParam);

                hBrowserWnd = mdtv_CreateMdolphinWindow (hWnd);
#if HAVE_BROWSER_TITLE
                hBrowserTitleWnd = create_browser_title_wnd(hWnd, hBrowserWnd );
#endif
                set_title_wnd_browser_status(hBrowserTitleWnd, g_md_status.m_md_status);

                SetActiveWindow(hWnd);
                SetFocusChild(hBrowserWnd);

                set_callback_info (hBrowserWnd );

                if (pCreateInfo != NULL)
                    url = (char *) (pCreateInfo->dwAddData);
                if (url == NULL)
                    url = (char*)"www.minigui.com";

                mdolphin_navigate(g_mdolphin_hwnd, NAV_GOTO, url, FALSE);
            }
            break;
        case MSG_TIMER:
            {
                int timer_id = (int)wParam;
                if( g_md_status.m_ime_timer_id != timer_id)
                    break;
                stopImeTimer(hWnd, &(g_md_status.m_ime_timer_id));
                hide_ime_window();
            }
            break;
        case MSG_IME_CMD_KEYDOWN:
            if(g_md_status.m_md_status == MDStatusRoller && g_md_status.m_is_editable){
                start_ime_timer(hWnd, &(g_md_status.m_ime_timer_id));
            }
            break;
        case MSG_CHAR:
            if(g_md_status.m_md_status == MDStatusRoller){
                start_ime_timer(hWnd, &(g_md_status.m_ime_timer_id));
                if(wParam>='0'&& wParam<='9'){
                    if(!(lParam & KS_IMEPOST)){
                        return 0;
                    }
                }
                if( wParam == '=' || wParam == '-' ){
                    if(!(lParam & KS_IMEPOST)){
                        return 0;
                    }
                }
            } //if
            break;
        case MSG_KEYUP:
            alwaysKeyPressCnt = 0;
            break;
        case MSG_KEYDOWN:
            while(PeekMessage(&msg, hWnd, MSG_KEYDOWN, MSG_KEYDOWN, PM_REMOVE))
            {
                //repeat_cnt++;
                //printf("---------- PeekMessage()\n");
            }
            switch(wParam)
            {
                case SCANCODE_F10:
                    if (get_browser_ime_status ())
                    {
                        if (!sg_imehMainWnd)
#if 0
                             mdtv_InitPhoneIme (&sg_caret_rect);
#else
                            mdtv_SetPhoneImeRect (&sg_caret_rect);
#if MDTV_ENABLE_SOFTIME
                            mdtv_init_keyboard (g_mdolphin_main_hwnd);
#endif
#endif
#if !defined(_LITE_VERSION) && !defined(_MGRM_PROCESSES) && !defined(_STAND_ALONE)
                        {
                            mdtv_ShowIme (1, (HWND)NULL); //display ime
                            mdtv_ShowIme (4, g_mdolphin_hwnd);
                            mdtv_ShowSoftIme (TRUE);
                        }
#endif
                    break;
                    }
#if TOOLBAR_WITH_ZOOM
                case SCANCODE_SLASH:
                    InitToolbar();
                    break;
#endif
                case SCANCODE_F2:
                    {
                        hBrowserTitleWnd = GetWindowAdditionalData2(hWnd);
                        if( hBrowserTitleWnd == HWND_INVALID ){
                            return -1;
                        }
                        change_title_wnd_browser_status(hBrowserTitleWnd, &(g_md_status.m_md_status));
                        return 0;
                    }
#if USE_PAD_KEY
                case SCANCODE_KEYPAD1:
                case SCANCODE_KEYPAD2:
                case SCANCODE_KEYPAD3:
                case SCANCODE_KEYPAD4:
                case SCANCODE_KEYPAD5:
                case SCANCODE_KEYPAD6:
                case SCANCODE_KEYPAD7:
                case SCANCODE_KEYPAD8:
                case SCANCODE_KEYPAD9:
#else
                case SCANCODE_1:
                case SCANCODE_2:
                case SCANCODE_3:
                case SCANCODE_4:
                case SCANCODE_5:
                case SCANCODE_6:
                case SCANCODE_7:
                case SCANCODE_8:
                case SCANCODE_9:
#endif
                    if( g_md_status.m_md_status == MDStatusMouse ){
                        x_pos = (wParam-SCANCODE_1)%3;
                        y_pos = (wParam-SCANCODE_1)/3;
                        hBrowserTitleWnd = GetWindowAdditionalData2(hWnd);
                        if( GetWindowRect( hBrowserTitleWnd, &rc) ){
                            title_height = RECTH(rc);
                        }
                        x_pos = (2*x_pos + 1)*g_win_width/6;
                        y_pos = (2*y_pos + 1)*(g_win_height-title_height)/6+title_height;

                        SetCursorPos(x_pos, y_pos);
                        return 0;
                    }
                    //  break; // don't add break;
#if USE_PAD_KEY
                case SCANCODE_KEYPAD0:
#else
                case SCANCODE_0:
#endif
                    if(g_md_status.m_md_status == MDStatusRoller){
                        if( g_md_status.m_is_editable ){
                            show_ime_window();
                            start_ime_timer(hWnd, &(g_md_status.m_ime_timer_id));
                            mdtv_send_msg_to_ime(message, wParam, lParam);
                            return 0;
                        } //if
                    }
                    break;
#if USE_PAD_KEY
                case SCANCODE_KEYPADDIVIDE:
#else
                case SCANCODE_EQUAL:
#endif
                    // switch ime method
                    if(g_md_status.m_md_status == MDStatusRoller){
                        if( g_md_status.m_is_editable ){
                            show_ime_window();
                            start_ime_timer(hWnd, &(g_md_status.m_ime_timer_id));
                            //SendMessage (sg_phone_ime_hwnd, MSG_KEYDOWN, SCANCODE_F5, 0);
                            mdtv_ime_switch_method();
                            return 0;
                        }
                    } //if
                    break;
#if USE_PAD_KEY
                case SCANCODE_KEYPADMULTIPLY:
#else
                case SCANCODE_MINUS:
#endif
                    // switch symbol input
                    if(g_md_status.m_md_status == MDStatusRoller){
                        if( g_md_status.m_is_editable ){
                            show_ime_window();
                            start_ime_timer(hWnd, &(g_md_status.m_ime_timer_id));
                            mdtv_send_msg_to_ime(message, wParam, lParam);
                            return 0;
                        }
                    }
                    break;
                default:
                    break;
            }
            switch (translate_command((int)wParam)){
                case zoom_in:
                    {
                        if(!g_mdolphin_hwnd)
                            break;
                        mdolphin_get_text_size_multiplier(g_mdolphin_hwnd, &percent);
                        percent +=20;
                        if(percent > 200){
                            return 0;
                        }
                        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
                    }
                    return 0;
                case zoom_out:
                    {
                        if(!g_mdolphin_hwnd)
                            break;
                        mdolphin_get_text_size_multiplier(g_mdolphin_hwnd, &percent);
                        percent -=20;
                        if( percent < 40 ){
                            return 0;
                        }
                        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
                    }
                    return 0;
                case link_forward:
                    PostMessage(g_mdolphin_hwnd, MSG_KEYDOWN, WPARAM(SCANCODE_TAB), lParam);
                    return 0;
                case link_backward:
                    PostMessage(g_mdolphin_hwnd, MSG_KEYDOWN, WPARAM(SCANCODE_TAB), lParam|KS_SHIFT);
                    return 0;

                case stop:
                    mdolphin_navigate(g_mdolphin_hwnd, NAV_STOP, NULL, FALSE);
                    return 0;

                case reload:
                    mdolphin_navigate(g_mdolphin_hwnd, NAV_RELOAD, NULL, FALSE);
                    return 0;

                case backward:
                    mdolphin_navigate(g_mdolphin_hwnd, NAV_BACKWARD, NULL, FALSE);
                    return 0;

                case forward:
                    mdolphin_navigate(g_mdolphin_hwnd, NAV_FORWARD, NULL, FALSE);
                    return 0;

                case home:
                    mdolphin_navigate(g_mdolphin_hwnd, NAV_GOTO, home_url, FALSE);
                    return 0; 

                case move_right:
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_right, MOUSE_MOVE_SPEED_X*4, speedx/5);
                    } else {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_right, MOUSE_MOVE_SPEED_X, speedx/12);
                    }
                    break;

                case move_left:
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_left, MOUSE_MOVE_SPEED_X * 4, speedx/5);
                    } else {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_left, MOUSE_MOVE_SPEED_X, speedx/12);
                    }
                    break;

                case move_up:
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_up, MOUSE_MOVE_SPEED_X*4, speedy/4);
                    } else {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_up, MOUSE_MOVE_SPEED_X, speedy/12);
                    }
                    break;

                case move_down:
                    ++alwaysKeyPressCnt;
                    if (alwaysKeyPressCnt > 3) {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_down, MOUSE_MOVE_SPEED_X*4, speedy/4);
                    } else {
                        move_mouse_browser(hWnd, g_md_status.m_md_status, move_down, MOUSE_MOVE_SPEED_X, speedy/12);
                    }
                    break;
                case page_up:
                    move_mouse_browser(hWnd, g_md_status.m_md_status, move_up, speedy/2, speedy/2);
                    break;
                case page_down:
                    move_mouse_browser(hWnd, g_md_status.m_md_status, move_down, speedy/2, speedy/2);
                    break;
#if ENABLE_FAV
                case fav:
                    {
                        ret_val = mdtv_save_fav(g_mdolphin_hwnd);
                        if( ret_val==TRUE ){
                            MessageBox(hWnd, "您已经成功保存网页.\n", "Feynman mDolphin v2.0", MB_OK|MB_ICONINFORMATION);
                        } else if (ret_val==NOT_OVERWRITE ){
                            MessageBox(hWnd, "not overwrite\n", "Feynman mDolphin v2.0", MB_OK|MB_ICONINFORMATION);
                        } else {
                            MessageBox(hWnd, "保存网页失败!\n", "Feynman mDolphin v2.0", MB_OK|MB_ICONINFORMATION);
                        }
                    }
                    break;
#endif                    
                case enter:
                   if( g_md_status.m_md_status == MDStatusMouse ){
                        GetCursorPos( &point );
                        PostMessage( hWnd, MSG_LBUTTONDOWN, wParam, MAKELONG(point.x, point.y));
                        PostMessage( hWnd, MSG_LBUTTONUP, wParam, MAKELONG(point.x, point.y));
                    }
                    break;
                case esc:
                    {
#if HAVE_BROWSER_TITLE
                        hBrowserTitleWnd = GetWindowAdditionalData2(hWnd);
                        if( hBrowserTitleWnd ){
                            SendMessage(hBrowserTitleWnd,MSG_CLOSE,0,0);
                        }
#endif
                        close_ime_wnd();
                        SendMessage(hWnd,MSG_CLOSE,0,0);
                        InitToolbar(g_hMainWnd);
                    }
                    return 0;
            }
            break;
        case MSG_DESTROY:
            g_mdolphin_main_hwnd = HWND_NULL;
            DestroyAllControls (hWnd);
            break;

        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            break;
    }
    return DefaultMainWinProc (hWnd, message, wParam, lParam);
}

HWND  mdtv_CreateWebSiteNavigate_1(const char *url )
{
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
    CreateInfo.spCaption = "mDolphin";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MDolphinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = BROWSER_TITLE_HEIGHT*scale_x;
    CreateInfo.rx = RECTW(g_rcScr);
    CreateInfo.by = RECTH(g_rcScr);
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = ( DWORD )url;
    CreateInfo.hHosting = g_hMainWnd;

    hMainWnd = CreateMainWindow (&CreateInfo);
    g_mdolphin_main_hwnd = hMainWnd;
#if MDTV_ENABLE_SOFTIME
        g_kbwnd = mdtv_init_keyboard (g_hMainWnd);
#else
        mdtv_CreateImePinying ();
#endif
    if (hMainWnd == HWND_INVALID)
        return -1;
    return hMainWnd;
}


HWND  mdtv_CreateWebSiteNavigate(const char *url )
{
    HWND hWnd;
    hWnd =mdtv_CreateWebSiteNavigate_1(url); 
    return hWnd;
}

//#endif
