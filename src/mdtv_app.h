#ifndef MDTV_APP_H
#define MDTV_APP_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#define DEFAULT_SCREEN_WIDTH    1024
#define DEFAULT_SCREEN_HEIGHT   768 

// default screen's size devide real screen's size
extern float scale;
extern float scale_x;
extern float scale_y;
extern HWND  g_hMainWnd;
extern int g_win_width;
extern int g_win_height;

BOOL mdtv_SelectWindowFontBycfg(HDC hdc, const char *fontname);
void InitBackGround(void);

#endif
