#if ENABLE_FAV
#include <unistd.h>
#include <string.h>
#include "mdtv_app.h"
#include "mdtv_browser.h"
#include "mdtv_toolbar.h"
#include "mdtv_fav.h"
#include "mdtv_fav_file.h"

//#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <mgeff/mgeff.h>

#define  MDTV_HAVE_FAVFILE  1


#define AMI_INTERVAL   50
#define ARRAY_LEN   TABLESIZE

enum {
    MDTV_DESK_BK,
    MDTV_ICON_BK,
};

#define IDC_ICONFLOW     102
#define FAV_CONFIG_FILE "mdtv_fav.cfg"

// width and height of each icon , changed according to resolution
static unsigned int s_icon_width = 300;
static unsigned int s_icon_height = 300;

static FAV_ICONFLOW_INFO s_iconflow_info[MDTV_FAV_ICON_MAX_NUMBER];

static const char *app_pics[]=
{
	"background.png",
	"fav/icon_bk.bmp",
};
#ifndef MDTV_HAVE_FAVFILE
static char* mdtv_fav_pics[]=
{
	"fav/tom.bmp",
	"fav/tudou.bmp",
	"fav/baidu.bmp",
	"fav/msn.bmp",
	"fav/minigui.bmp",
	"fav/baidu.bmp",
	"fav/baidu.bmp",
	"fav/baidu.bmp",
	"fav/baidu.bmp",
	"fav/yahoo.bmp",
};
static const char* mdtv_website_addr[] ={
		"http://www.tom.com",
		"http://www.tudou.com",
		"http://www.baidu.com",
		"http://www.msn.com",
		"http://www.minigui.com",
		"http://www.baidu.com",
		"http://www.baidu.com",
		"http://www.baidu.com",
		"http://www.baidu.com",
		"http://www.yahoo.com",
};
#else
//static char* mdtv_fav_pics[MDTV_FAV_ICON_MAX_NUMBER];
//static char* mdtv_website_addr[MDTV_FAV_ICON_MAX_NUMBER];
#endif

static HDC iconflow_dc_bkgnd;
static HDC iconflow_dc_icon;
static HWND hwnd_iconflow;
static HDC mdtv_win_dc [MDTV_FAV_ICON_MAX_NUMBER];

static HDC* get_default_win_dc(void)
{
	int i;
	for(i=0;i<MDTV_FAV_ICON_MAX_NUMBER;i++)
	{
		mdtv_win_dc[i] = CreateCompatibleDCEx(HDC_SCREEN,s_icon_width,s_icon_height);
		FillBoxWithBitmap(mdtv_win_dc[i],0,0,0,0,RetrieveRes(s_iconflow_info[i].m_img_path));
	}
	return mdtv_win_dc;
}

static void init_iconflow(HWND hWnd)
{
    iconflow_dc_bkgnd = CreateCompatibleDCEx(HDC_SCREEN, g_win_width, g_win_height);
    iconflow_dc_icon = CreateCompatibleDCEx (HDC_SCREEN, s_icon_width, s_icon_width);
    hwnd_iconflow = CreateWindowEx2("iconflow", "", WS_VISIBLE, WS_EX_NONE,
            IDC_ICONFLOW, 0, 0, g_win_width, g_win_height, hWnd, NULL, NULL, 0);

    // set back ground
    FillBoxWithBitmap(iconflow_dc_bkgnd, (g_win_width-1024)/2, (g_win_height-768)/2, 1024, 768,
            RetrieveRes(app_pics[MDTV_DESK_BK]));

    SendMessage (hwnd_iconflow, MSG_SET_BKGND, 0, (LPARAM)iconflow_dc_bkgnd);
    SendMessage (hwnd_iconflow, MSG_SET_DCFLOW, MDTV_FAV_ICON_MAX_NUMBER, (LPARAM)mdtv_win_dc);
    // set icon back ground
    FillBoxWithBitmap (iconflow_dc_icon, 0, 0, 0, 0,
            RetrieveRes(app_pics[MDTV_ICON_BK]));
    SendMessage (hwnd_iconflow, MSG_SET_ICON_BKGND, 0, (LPARAM)iconflow_dc_icon);

    ShowWindow(hWnd, SW_SHOWNORMAL);
    SetActiveWindow(hWnd);
    //SetFocus(hwnd_iconflow);
}
static int AnimationZoomIn( HDC hdc, const RECT *src_rc, const RECT *dst_rc,  int interval, int frames )
{
    int width_inc;
    int height_inc;
    int x_inc, y_inc;

    width_inc = (RECTW(*dst_rc) - RECTW(*src_rc)) / frames;
    height_inc = (RECTH(*dst_rc) - RECTH(*src_rc)) / frames;
    x_inc = ( src_rc->left - dst_rc->left ) /frames;
    y_inc = ( src_rc->top - dst_rc->top ) /frames;

    for(int i=0;i<frames;i++){
	    StretchBlt(hdc, 0, 0, 0, 0, HDC_SCREEN, src_rc->left-i*x_inc, src_rc->top-i*y_inc, RECTW(*src_rc)+i*width_inc, RECTH(*src_rc)+i*height_inc,0);
        usleep(interval*1000);
    }
    return 0;
}
static void onAnimate_toLarge(int index)
{
	HDC hdc = mdtv_win_dc[index];
    RECT src_rc, dst_rc;

    src_rc.left = g_win_width*2/5;
    src_rc.top = g_win_height/3;
    src_rc.right = g_win_width*25/40;
    src_rc.bottom = g_win_height*2/3;

    dst_rc.left = 0;
    dst_rc.top = 0;
    dst_rc.right = g_win_width;
    dst_rc.bottom = g_win_height;

    AnimationZoomIn( hdc, &src_rc, &dst_rc, AMI_INTERVAL, 5);
}
static int display_title(const RECT *rc)
{
    unsigned int sel;
    sel = SendMessage(hwnd_iconflow, MSG_GET_INDEX, 0, 0);
    sel = (sel+1)%7;
    //printf("sel=%d, s_iconflow_info[sel].m_title=%s\n", sel,s_iconflow_info[sel].m_title );
    SetBkMode (HDC_SCREEN, BM_TRANSPARENT);
    DrawText (HDC_SCREEN, s_iconflow_info[sel].m_title, -1, (RECT*)rc, DT_CENTER | DT_WORDBREAK);
    return 0;
}

static RECT rc_title;
static int MDTVFavWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hWebWnd;
//    HDC hdc;
    int sel;
    SetRect( &rc_title, 0, g_win_height*7/10, g_win_width, g_win_height*4/5 );
    switch(message)
    {
    case MSG_CREATE:
    	init_iconflow(hWnd);
    	break;
        case MSG_KEYDOWN:
	    {
            switch(wParam)
            {
                case SCANCODE_ENTER:
                    {
                        sel = SendMessage(hwnd_iconflow, MSG_GET_INDEX, 0, 0);
                        sel = (sel+1)%7;
                        ShowWindow(hWnd,SW_HIDE);
                        onAnimate_toLarge(sel);
                        SendMessage(hWnd,MSG_CLOSE,0,0);
                        if(s_iconflow_info[sel].m_url){
                            hWebWnd = mdtv_CreateWebSiteNavigate(s_iconflow_info[sel].m_url );
                            ShowWindow(hWebWnd,SW_SHOWNORMAL);
                        }
                        break;
                    }
                case SCANCODE_CURSORBLOCKLEFT:
                    SendMessage(hwnd_iconflow,MSG_KEYDOWN,SCANCODE_CURSORBLOCKLEFT,0);
                    //InvalidateRect(hWnd, &rc_title, 0);
                    //display_title(&rc_title);
                    break;
                case SCANCODE_CURSORBLOCKRIGHT:
                    SendMessage(hwnd_iconflow,MSG_KEYDOWN,SCANCODE_CURSORBLOCKRIGHT,0);
                    //InvalidateRect(hWnd, &rc_title, 0);
                    //display_title(&rc_title);
                    break;
                case SCANCODE_ESCAPE:
                    SendMessage(hWnd,MSG_CLOSE,0,0);
                    InitToolbar(g_hMainWnd);
                    break;
            }
            break;
        }
    case MSG_IDLE:
        display_title(&rc_title);
        break;
	case MSG_CLOSE:
		DeleteMemDC(iconflow_dc_bkgnd);
		DeleteMemDC(iconflow_dc_icon);
		DestroyMainWindow(hWnd);
		break;
    }
    return DefaultMainWinProc(hWnd,message,wParam,lParam);
}
static HWND CreateMDTVFavWnd()
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MDTVFavWndProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = RECTW(g_rcScr);
    CreateInfo.by = RECTH(g_rcScr);
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    return CreateMainWindow (&CreateInfo);
}
#ifdef MDTV_HAVE_FAVFILE
static void get_fav_list()
{
    int ret;
    ret = mdtv_read_fav_info("res/fav/fav.xml", s_iconflow_info, MDTV_FAV_ICON_MAX_NUMBER);
    if( ret<0 ){
        fprintf(stderr, "can't read res/fav/fav.xml\n");
        return;
    }
}
#endif
void UninitMDTVFavWebsite()
{
	int i = 0;
	for(i=0;i<MDTV_FAV_ICON_MAX_NUMBER;i++)
	{
		ReleaseDC(mdtv_win_dc[i]);
	}
}
void InitMDTVFavWebsite()
{
	get_default_win_dc();
}
void InitMDTVFavWndRes()
{
	unsigned int i = 0;
#ifdef MDTV_HAVE_FAVFILE
	get_fav_list();
#endif
	for (i = 0; i < ARRAY_LEN(app_pics); i++)
	{
		if (LoadResource(app_pics[i], RES_TYPE_IMAGE, (DWORD)HDC_SCREEN) == NULL)
        {
            fprintf (stderr, "LoadResource %s error. \n", app_pics[i]);
        }
	}
	for (i = 0; i < ARRAY_LEN(s_iconflow_info); i++)
	{
		if (LoadResource(s_iconflow_info[i].m_img_path, RES_TYPE_IMAGE,
					(DWORD)HDC_SCREEN) == NULL)
			fprintf (stderr, "LoadResource %s error. \n", s_iconflow_info[i].m_img_path);
	}

	RegisterIconFlowControl();
}

void UninitMDTVFavWndRes()
{
    unsigned int i;
    for (i = 0; i < ARRAY_LEN (app_pics); i++)
        UnregisterRes (app_pics[i]);
    for (i = 0; i < ARRAY_LEN (s_iconflow_info); i++)
        UnregisterRes (s_iconflow_info[i].m_img_path);
#ifdef MDTV_HAVE_FAVFILE
    mdtv_uninit_fav_icon_info( s_iconflow_info, ARRAY_LEN (s_iconflow_info) );
#endif
}

//int AddFavWebsite(HDC web_hdc, const char* website)
BOOL mdtv_save_fav(HWND hWnd)
{
    HDC web_hdc;
//    MDTV_PTR_FAV_INFO p_fav_info;
    BrowserTitleWndInfo *p_info;
    int ret;

    web_hdc = GetClientDC(hWnd);
    if( !hWnd ){
        fprintf(stderr, "[ERROR]: parameter Error!\n"); 
        return FALSE;
    }

	p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(hWnd);
    if(!p_info){
        fprintf(stderr, "[ERROR]: Can't get BrowserTitleWndInfo parameter!\n"); 
        return FALSE;
    }
    if(!p_info->m_url){
        fprintf(stderr, "[warning]: Can't get BrowserTitleWndInfo parameter - url!\n"); 
        return FALSE;
    }

	for(int i=MDTV_FAV_ICON_MAX_NUMBER-1;i>0;i--){
		s_iconflow_info[i].m_url = s_iconflow_info[i-1].m_url;
		s_iconflow_info[i].m_title = s_iconflow_info[i-1].m_title;
		BitBlt(mdtv_win_dc[i-1],0,0,0,0,mdtv_win_dc[i],0,0,0);
	}
    if( p_info->m_url ){
        if(s_iconflow_info[0].m_url){
            xmlFree(s_iconflow_info[0].m_url);
        }
	    s_iconflow_info[0].m_url = strdup(p_info->m_url);
    }
    if( p_info->m_title ){
        if(s_iconflow_info[0].m_title){
            xmlFree(s_iconflow_info[0].m_title);
        }
	    s_iconflow_info[0].m_title = strdup(p_info->m_title);
    }
	//BitBlt(web_hdc,0,0,0,0,mdtv_win_dc[0],0,0,0);
	StretchBlt(web_hdc,0,0,0,0,mdtv_win_dc[0],0,0,s_icon_width, s_icon_height, 0);
    usleep(10000);

#ifdef MDTV_HAVE_FAVFILE
    ret = mdtv_save_favorite_website( hWnd );
    if( ret == FALSE ){
        fprintf(stderr, "[Error]save fav file!\n");
        return FALSE;
    }
    return ret;
#endif
}
int InitMDTVFavWnd()
{
	HWND hMainWnd;

	hMainWnd = CreateMDTVFavWnd();
	if(hMainWnd==HWND_INVALID){
		return -1;
	}
	ShowWindow(hMainWnd,SW_SHOWNORMAL);
    return 0;
}
void UninitMDTVFavWnd()
{
	return;
}
#endif
