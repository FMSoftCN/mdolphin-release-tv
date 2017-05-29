#if TOOLBAR_WITH_ZOOM

#include <unistd.h>
#include "svgui.h"
#include "mdtv_app.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_toolbar.h"
#include "mdtv_browser.h"

extern HWND g_hMainWnd ;
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


#define TOOLBAR_ZOOM_IN_WIDTH   (200)
#define TOOLBAR_ZOOM_IN_HEIGHT  (TOOLBAR_ITEM_HEIGHT*2.0/3) 

#define TOOLBAR_ZOOM_OUT_WIDTH  (TOOLBAR_ZOOM_IN_WIDTH)
#define TOOLBAR_ZOOM_OUT_HEIGHT (TOOLBAR_ZOOM_IN_HEIGHT) 

#define TOOLBAR_ZOOM_X_PERCENT   (1.0*(TOOLBAR_ARROW_WIDTH+(TOOLBAR_ITEM_WIDTH-TOOLBAR_ZOOM_IN_WIDTH)/2)/DEFAULT_SCREEN_WIDTH)      
//#define TOOLBAR_ZOOM_X_PERCENT   (1.0*TOOLBAR_ARROW_WIDTH/DEFAULT_SCREEN_WIDTH)      
#define TOOLBAR_ZOOM_Y_PERCENT   (1.0*(2*TOOLBAR_ZOOM_IN_HEIGHT+TOOLBAR_ITEM_HEIGHT)/DEFAULT_SCREEN_HEIGHT)      
#define TOOLBAR_ZOOM_WIDTH_PERCENT   (1.0*TOOLBAR_ZOOM_IN_WIDTH/DEFAULT_SCREEN_WIDTH)
#define TOOLBAR_ZOOM_HEIGHT_PERCENT   (2.0*TOOLBAR_ZOOM_IN_HEIGHT/DEFAULT_SCREEN_HEIGHT)


#define TOOLBAR_COLOR_TEXT_SELECTED     (0xFFFFFF)
#define TOOLBAR_COLOR_TEXT_NOT_SELECTED (0xF6FF00)

//---- SVGUI_PRDRD_BLOCK_T
// toolbar background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[];
enum{
    TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK,
    TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK,
};

#define TOOLBAR_PRDRD_BLOCK_COLOR_BK   (0x000000)
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // 0: zoom window backgound block : TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK
    {
        TOOLBAR_ZOOM_IN_WIDTH,    // width
        TOOLBAR_ZOOM_IN_HEIGHT,   // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, TOOLBAR_ZOOM_IN_WIDTH/2, TOOLBAR_ZOOM_IN_WIDTH,
        0x531A00, 0xFF6500, 0x531A00,//(83,26,0),(255,101,0),(83,26,0)
        2,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;
        2,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
    // 1: zoom window backgound block : TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK
    {
        TOOLBAR_ZOOM_IN_WIDTH,    // width
        TOOLBAR_ZOOM_IN_HEIGHT,   // height
        SVGUI_PRDRD_GRADIENT_VERT,// int gradient_type;
        0, TOOLBAR_ZOOM_IN_WIDTH/2, TOOLBAR_ZOOM_IN_WIDTH,
        0x010101, 0x3C3B3B, 0x010101,// (1,1,1),(60,59,60),(1,1,1)
        2,                      //int border_width;
        0x452222,               //RGBCOLOR border_color;
        2,                      //int corner;
        TOOLBAR_PRDRD_BLOCK_COLOR_BK,               //RGBCOLOR color_bk;  /* will be used as color key */
    },
};

static SVGUI_FONT_INFO_T ptr_font_infos[]={
    {"fmsong", 20, "UTF-8"},
    {"fmhei", 20, "UTF-8"},
    {"fmhei", 30, "UTF-8"},
    {"fmhei", 40, "UTF-8"},
};

//---- SVGUI_TEXT_AREA_T
static SVGUI_TEXT_AREA_T ptr_zoom_in_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ZOOM_IN_WIDTH/3, 0, TOOLBAR_ZOOM_IN_WIDTH, TOOLBAR_ZOOM_IN_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFF00FF,               // RGBCOLOR color;
        0,                      //................... int idx_font;
        "放大",      // char* text;
    },
};
static SVGUI_TEXT_AREA_T ptr_zoom_out_block_text_areas[]={
    {
        0,                      //.................. int id;
        TRUE,                   // BOOL is_visible;
        {TOOLBAR_ZOOM_IN_WIDTH/3, 0, TOOLBAR_ZOOM_IN_WIDTH, TOOLBAR_ZOOM_IN_HEIGHT},
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,// UINT align;
        0xFF00FF,               // RGBCOLOR color;
        0,                      //................... int idx_font;
        "缩小",      // char* text;
    },
};
//---- SVGUI_IMAGE
static SVGUI_IMAGE_T ptr_zoom_in_block_images[]={
    {
        56,                     // int widht;
        59,                     // int height;
        16,                      //..... int depth;
        "toolbar/zoom_in.png",// char* file_name;
    },
};
static SVGUI_IMAGE_T ptr_zoom_out_block_images[]={
    {
        56,                     // int widht;
        59,                     // int height;
        16,                      //..... int depth;
        "toolbar/zoom_out.png",// char* file_name;
    },
};

//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T ptr_zoom_in_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ZOOM_IN_WIDTH/2,TOOLBAR_ZOOM_IN_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_zoom_in_block_images),                      // int nr_images;
        ptr_zoom_in_block_images,  // SVGUI_IMAGE* images;
    },
};
static SVGUI_IMAGE_AREA_T ptr_zoom_out_block_image_areas[]={
    {
        0,                      // int id;
        TRUE,                   // BOOL is_visible;
        {0, 0, TOOLBAR_ZOOM_IN_WIDTH/2,TOOLBAR_ZOOM_IN_HEIGHT},// RECT rc;
        SVGUI_IMAGE_FILL_WAY_CENTER, // int fill_way;
        
        ARRAY_LEN(ptr_zoom_out_block_images),                      // int nr_images;
        ptr_zoom_out_block_images,  // SVGUI_IMAGE* images;
    },
};

//---- SVGUI_BLOCK_T
enum {
    TOOLBAR_BLOCK_ZOOM_IN,
    TOOLBAR_BLOCK_ZOOM_OUT,
};
static SVGUI_BLOCK_T ptr_blocks[]={
    {
        TOOLBAR_BLOCK_ZOOM_IN,            // int id;
        TRUE,               // BOOL is_visible;
        {0, 0, TOOLBAR_ZOOM_IN_WIDTH, TOOLBAR_ZOOM_IN_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK, // int idx_prdrd_block;
        ARRAY_LEN(ptr_zoom_in_block_text_areas),                  // int nr_text_areas;
        ptr_zoom_in_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;
        ARRAY_LEN(ptr_zoom_in_block_image_areas),                  // int nr_image_areas;
        ptr_zoom_in_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
    {
        TOOLBAR_BLOCK_ZOOM_OUT,            // int id;
        TRUE,               // BOOL is_visible;
        {0,  TOOLBAR_ZOOM_IN_HEIGHT, TOOLBAR_ZOOM_IN_WIDTH, 2*TOOLBAR_ZOOM_IN_HEIGHT}, // RECT rc;

        TRUE,           // BOOL is_hotspot;
        TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK, // int idx_prdrd_block;
        //TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK,

        ARRAY_LEN(ptr_zoom_out_block_text_areas),                  // int nr_text_areas;
        ptr_zoom_out_block_text_areas, // SVGUI_TEXT_AREA_T* text_areas;

        ARRAY_LEN(ptr_zoom_out_block_image_areas),                  // int nr_image_areas;
        ptr_zoom_out_block_image_areas, // SVGUI_IMAGE_AREA_T* image_areas;
    },
};
//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t_toolbar = {

    TOOLBAR_ZOOM_IN_WIDTH,// width
    2*TOOLBAR_ZOOM_IN_HEIGHT,// height

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
static int on_update_ui_secdc (HWND hWnd, HDC secondary_dc,
        HDC real_dc, const RECT* secondary_rc, const RECT* real_rc,
        const RECT* main_update_rc)
{
    SVGUI_HEADER_I *svgui_header;
    /* erase background by using the content in g_dc_bkgnd */
    //BitBlt (g_dc_bkgnd, toolbar_rect.left, toolbar_rect.top, RECTW(toolbar_rect), RECTH(toolbar_rect), real_dc, 0, 0, 0);

    /* blending g_dc_uisec to the real dc */
    svgui_header = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
    SetMemDCColorKey(secondary_dc, MEMDC_FLAG_SRCCOLORKEY, svgui_header->pixel_key);
    //SetMemDCAlpha (secondary_dc, MEMDC_FLAG_SRCALPHA, 0xD0);
    BitBlt (secondary_dc,0,0,RECTW(*secondary_rc),RECTH(*secondary_rc), real_dc,0,0,0);
    return 0;
}

static BOOL set_window_seconddc(HWND hWnd, int   off_x, int      off_y,int   width,  int height)
{
    HDC subdc;
    debug("************ set_window_seconddc  *******\n");
    debug("width=%d, height=%d\n", width, height);
    //HDC hdc = alloc_subdc_from_uisec( off_x, off_y, width, height);
    //HDC subdc_toolbar = GetSubDC(g_dc_uisec, 100, 40, 1024, 200);
    subdc = CreateSubMemDC(g_dc_uisec, off_x, off_y, width, height, TRUE);
#if 0
    SetBrushColor (subdc_toolbar, RGBA2Pixel(subdc_toolbar, 0xFF, 0xFF, 0xFF, 0xFF));

    FillBox (subdc_toolbar, 0, 0, width, height);
#endif
    if (subdc != HDC_INVALID){
        //SetSecondaryDC(hWnd, subdc, on_update_ui_secdc);
        SetSecondaryDC(hWnd, subdc, ON_UPDSECDC_DEFAULT);
        return TRUE;
    }
    return FALSE;
}

static HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc, HWND hHostingWnd )
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
    CreateInfo.dwAddData = (DWORD)&svgui_header_t_toolbar;

    CreateInfo.hHosting = hHostingWnd;

    return CreateMainWindow (&CreateInfo);
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

static int ToolbarOnLButtonDown (HWND hWnd, int x_pos, int y_pos, int prdrd_id_item_selected,int prdrd_id_item_not_selected)
{
    SVGUI_BLOCK_I *old_block = NULL;

    //int x_pos = LOWORD (lParam);
    //int y_pos = HIWORD (lParam);
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return -1;

    old_block = GetSelectedBlock (hWnd,prdrd_id_item_selected,TOOLBAR_BLOCK_ZOOM_IN, TOOLBAR_BLOCK_ZOOM_OUT);
    if( old_block ){
        old_block->idx_prdrd_block       = prdrd_id_item_not_selected;
        InvalidateRect(hWnd, &(old_block->rc), TRUE);
    }
    block = svgui_get_block_by_point (header, x_pos, y_pos);

    debug (" xpos=%d, ypos=%d\n", x_pos, y_pos);
    if( block ){
        block->idx_prdrd_block       = prdrd_id_item_selected;
        InvalidateRect(hWnd, &(block->rc), TRUE);
        if(g_mdolphin_main_hwnd)
            if(block->id == TOOLBAR_BLOCK_ZOOM_IN){
                SendMessage(g_mdolphin_main_hwnd,MSG_KEYDOWN,SCANCODE_PAGEUP, 0 );
            }
            else if(block->id == TOOLBAR_BLOCK_ZOOM_OUT){
                SendMessage(g_mdolphin_main_hwnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0 );
            }
        }
    return 0;
}

static void InitZoomWnd(HWND hWnd, BOOL is_visible, BOOL curr_is_zoom_in)
{
    SVGUI_HEADER_I  *svgui_header_i;
    SVGUI_BLOCK_I   *svgui_block_i;
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );

    // zoom in menu's block
    svgui_block_i = svgui_get_block (svgui_header_i, TOOLBAR_BLOCK_ZOOM_IN);
    if(!svgui_block_i){
        return;
    }
    printf("--- InitZoomWnd(): id=%d, is_visible=%d\n",svgui_block_i->id, is_visible);
    svgui_block_i->is_visible       = is_visible;
    if(curr_is_zoom_in)
        svgui_block_i->idx_prdrd_block       = TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK;
    else
        svgui_block_i->idx_prdrd_block       = TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK;
    //InvalidateRect(hWnd, &(svgui_block_i->rc), TRUE);

    // zoom out menu's block
    svgui_block_i = svgui_get_block (svgui_header_i, TOOLBAR_BLOCK_ZOOM_OUT);
    if(!svgui_block_i){
        return;
    }
    printf("--- InitZoomWnd(): id=%d, is_visible=%d\n",svgui_block_i->id, is_visible);
    svgui_block_i->is_visible       = is_visible;
    if(curr_is_zoom_in)
        svgui_block_i->idx_prdrd_block       = TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK;
    else
        svgui_block_i->idx_prdrd_block       = TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK;

    //InvalidateRect(hWnd, &(svgui_block_i->rc), TRUE);

    InvalidateRect(hWnd, NULL, TRUE);

    return;
}
static int ToolbarOnPressDown(HWND hWnd)
{
    static BOOL curr_is_zoom_in=TRUE;
    curr_is_zoom_in = curr_is_zoom_in?FALSE:TRUE;
    InitZoomWnd(hWnd, TRUE, curr_is_zoom_in);
}

static void ToolbarOnPressEnter(HWND hWnd)
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = GetSelectedBlock (hWnd, TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK, TOOLBAR_BLOCK_ZOOM_IN, TOOLBAR_BLOCK_ZOOM_OUT);
    if( block ){
        if(g_mdolphin_main_hwnd){
            if(block->id == TOOLBAR_BLOCK_ZOOM_IN){
                SendMessage(g_mdolphin_main_hwnd,MSG_KEYDOWN,SCANCODE_PAGEUP, 0 );
            }
            else if(block->id == TOOLBAR_BLOCK_ZOOM_OUT){
                SendMessage(g_mdolphin_main_hwnd,MSG_KEYDOWN,SCANCODE_PAGEDOWN,0 );
            }
        }
    }
}
static int ToolbarZoomProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC subdc_toolbar;
    switch (message)
    {
        case MSG_CREATE:
                MAINWINCREATE  *pCreateInfo;
                pCreateInfo = (MAINWINCREATE  *)(lParam);
                if (pCreateInfo != NULL)
                {
                    set_window_seconddc(hWnd, pCreateInfo->lx, pCreateInfo->ty, pCreateInfo->rx - pCreateInfo->lx ,  pCreateInfo->by-pCreateInfo->ty );
                }
            break;
        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_CURSORBLOCKUP:
                    ToolbarOnPressDown(hWnd);
                    debug("**** SCANCODE_CURSORBLOCKUP\n");
                    break;
                case SCANCODE_CURSORBLOCKDOWN:
                    ToolbarOnPressDown(hWnd);
                    debug("**** SCANCODE_CURSORBLOCKDOWN\n");
                    break;
                case SCANCODE_ENTER:
                    ToolbarOnPressEnter(hWnd);
                    break;
                case SCANCODE_ESCAPE:
                    SendMessage(hWnd,MSG_CLOSE,0,0);
                    break;
            }
            break;
        case MSG_LBUTTONDOWN:
            ToolbarOnLButtonDown (hWnd, LOWORD (lParam), HIWORD (lParam), TOOLBAR_ZOOM_PRDRD_BLOCK_SELECTED_BLOCK,TOOLBAR_ZOOM_PRDRD_BLOCK_NOT_SELECTED_BLOCK);
            break;
        case MSG_CLOSE:
            subdc_toolbar = GetSecondaryDC(hWnd);
            DeleteMemDC(subdc_toolbar);
            DestroyMainWindow(hWnd);
            break;
    }
    //return DefaultMainWinProc(hWnd, message, wParam, lParam);
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}
HWND InitToolbarZoomWnd(){
    HWND hToolbarZoomWnd;
    RECT zoom_rc;
    debug(" ******** InitToolbarZoomWnd():   start!\n");

    float scale;
    scale = (1.0*RECTW(g_rcScr)/RECTH(g_rcScr))/(1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT);
    if(scale<1.0){
        scale = 1.0 ;
    }
    // init toolbar position in screen
    zoom_rc.left   = RECTW(g_rcScr)*TOOLBAR_ZOOM_X_PERCENT;
    zoom_rc.top    = RECTH(g_rcScr)*(1.0-TOOLBAR_ZOOM_Y_PERCENT*scale);
    zoom_rc.right  = zoom_rc.left+RECTW(g_rcScr)*(TOOLBAR_ZOOM_WIDTH_PERCENT);
    zoom_rc.bottom = zoom_rc.top+RECTH(g_rcScr)*(TOOLBAR_ZOOM_HEIGHT_PERCENT*scale) ;

    debug("     InitToolbarZoomWnd(): zoom_rc.left=%d,top=%d,right=%d,bottom=%d \n", zoom_rc.left,zoom_rc.top,zoom_rc.right,zoom_rc.bottom);

    hToolbarZoomWnd = CreateMenuWnd( &zoom_rc, ToolbarZoomProc, HWND_DESKTOP);
    if (hToolbarZoomWnd == HWND_INVALID){
        error("Create toolbar zoom window!\n");
        return HWND_INVALID;
    }

    InvalidateRect(hToolbarZoomWnd, NULL, TRUE);
    ShowWindow (hToolbarZoomWnd, SW_SHOWNORMAL);
    return hToolbarZoomWnd;
}
#endif
