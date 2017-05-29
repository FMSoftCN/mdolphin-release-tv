#include <unistd.h>
#include <string.h>

#include <math.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mdolphin/mdolphin.h>
#include "mdtv_app.h"
#include "mdtv_setting.h"
#include "mdtv_network.h"
#include "mdtv_toolbar.h"
#include "mdtv_browser.h"
#include "mdtv_setting_homepage.h"


extern int g_win_width;
extern int g_win_height;

//static   HWND hMainWnd;
HWND mdolphin_h1; 
const char *setting_url=NULL;
HWND mdolphin_h = 0;
#define ID_MDOLPHIN 10000
HWND InitSettingWnd()
{
#if 1
    //get_setting_url();
    int percent;
    setting_url = get_local_url("/res/setting/tabview.html");
    register_homepage();
    register_networkobject();
    mdolphin_h = mdtv_CreateWebSiteNavigate (setting_url);

    if (g_win_width <= 800){
        percent = 70;
        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
    }
    else if (g_win_width <= 640){
        percent = 60;
        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
    }



    if (setting_url)
        free ((char *)setting_url);
    setting_url = NULL;
#else
    MSG Msg;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
    CreateInfo.spCaption = "mDolphin";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MDolphinProc;
    CreateInfo.lx = (RECTW(g_rcScr)-670)/2;
    CreateInfo.ty = (RECTH(g_rcScr)-320)/2;
    CreateInfo.rx = CreateInfo.lx+670;
    CreateInfo.by = CreateInfo.ty+320;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;
    mdolphin_h = hMainWnd;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);
#endif

    return 0;
}

