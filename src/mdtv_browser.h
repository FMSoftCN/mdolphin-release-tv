#ifndef _MDTV_BROWSER_H
#define _MDTV_BROWSER_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mdolphin/mdolphin.h>
#include "mdtv_fav_file.h"

#define IDC_MDOLPHIN        106
typedef struct _BrowserTitleWndInfo{
    HWND    m_hTitleWnd;
    BOOL    m_loading;
    int     m_progress;
    char    *m_title;
    char    *m_url;
#if ENABLE_FAV
    MDTV_FAV_INFO *m_fav_info;
#endif
}BrowserTitleWndInfo;

extern HWND g_mdolphin_hwnd;
extern HWND g_mdolphin_main_hwnd;

HWND mdtv_CreateMdolphinWindow(HWND hParent);
BOOL mdtv_set_browser_title_ime_status( HWND hBrowserMainWnd, const char *str_ime_status );

#define MDTV_BROWSER_DEBUG

//#ifdef MDTV_BROWSER_DEBUG
HWND  mdtv_CreateWebSiteNavigate(const char *url );
 char * get_local_url(const char *url );
//#endif

#endif
