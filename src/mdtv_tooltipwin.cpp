/*
** $id: mywindows.c 8225 2007-11-26 05:37:49z xwyan $
**
** mywindows.c: implementation file of libmywins library.
**
** copyright (c) 2003 ~ 2007 feynman software.
** copyright (c) 2000 ~ 2002 wei yongming.
**
** current maintainer: wei yongming.
**
** create date: 2000.xx.xx
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mdolphin/mdolphin.h>

#ifdef MINIGUI_V3
#include <mgutils/mgutils.h>
#else
#include <minigui/mgext.h>
#include <minigui/newfiledlg.h>
#include <minigui/mywindows.h>
#endif


#define _MARGIN     2
#define _ID_TIMER   100

void mdolphin_hideToolTip(HWND hwnd)
{
    if (hwnd && IsWindowVisible(hwnd))
        ShowWindow(hwnd, SW_HIDE);
}

static int mdolphin_ToolTipWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
        case MSG_CREATE:
        {
            int timeout = (int)GetWindowAdditionalData (hWnd);
            if (timeout >= 10)
                SetTimer (hWnd, _ID_TIMER, timeout / 10);
            break;
        }

        case MSG_TIMER:
            KillTimer (hWnd, _ID_TIMER);
            mdolphin_hideToolTip(hWnd);
            break;

        case MSG_PAINT:
        {
            HDC hdc;
            const char* text;
            
            hdc = BeginPaint (hWnd);
 
            text = GetWindowCaption (hWnd);
            SetBkMode (hdc, BM_TRANSPARENT);
            PLOGFONT old = SelectFont (hdc, GetSystemFont (SYSLOGFONT_CAPTION));
            TabbedTextOut (hdc, _MARGIN, _MARGIN, text);
            SelectFont (hdc, old);
 
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_DESTROY:
        {
            KillTimer (hWnd, _ID_TIMER);
            return 0;
        }

        case MSG_SETTEXT:
        {
            int timeout = (int)GetWindowAdditionalData (hWnd);
            if (timeout >= 10) {
                KillTimer (hWnd, _ID_TIMER);
                SetTimer (hWnd, _ID_TIMER, timeout / 10);
            }

            break;
        }
    }
 
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND mdolphin_createToolTipWin (HWND hParentWnd, int x, int y, int timeout_ms, 
                const char* text, ...)
{
    HWND hwnd;
    char* buf = NULL;
    MAINWINCREATE CreateInfo;
    SIZE text_size;
    int width, height;

    if (strchr (text, '%')) 
    {
        va_list args;
        int size = 0;
        int i = 0;

        va_start(args, text);
        do {
            size += 1000;
            if (buf) free(buf);
            buf =(char*) malloc(size);
            i = vsnprintf(buf, size, text, args);
        } while (i == size);
        va_end(args);
    }

    PLOGFONT old = SelectFont (HDC_SCREEN, GetSystemFont (SYSLOGFONT_CAPTION));
    GetTabbedTextExtent (HDC_SCREEN, buf ? buf : text, -1, &text_size);
    text_size.cx += _MARGIN << 1;
    text_size.cy = GetSysCharHeight () + (_MARGIN << 1);
    width = ClientWidthToWindowWidth (WS_THINFRAME, text_size.cx);
    height= ClientHeightToWindowHeight (WS_THINFRAME, text_size.cy, FALSE);
    SelectFont (HDC_SCREEN, old);

    if (x + width > g_rcScr.right)
        x = g_rcScr.right - width;
    if (y + height > g_rcScr.bottom)
        y = g_rcScr.bottom - height;

    CreateInfo.dwStyle = WS_ABSSCRPOS | WS_THINFRAME | WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    CreateInfo.spCaption = buf ? buf : text;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor (IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = mdolphin_ToolTipWinProc;
    CreateInfo.lx = x;
    CreateInfo.ty = y;
    CreateInfo.rx = CreateInfo.lx + width;
    CreateInfo.by = CreateInfo.ty + height;
#ifdef MINIGUI_V3
    CreateInfo.iBkColor = GetWindowElementAttr(HWND_DESKTOP, WE_BGC_TOOLTIP);
#else
    CreateInfo.iBkColor = GetWindowElementColor (BKC_TIP);
#endif
    CreateInfo.dwAddData = (DWORD) timeout_ms;
    CreateInfo.hHosting = hParentWnd;

    hwnd = CreateMainWindow (&CreateInfo);

    if (buf)
        free (buf);

    return hwnd;
}

void mdolphin_resetToolTipWin (HWND hwnd, int x, int y, const char* text, ...)
{
    char* buf = NULL;
    SIZE text_size;
    int width, height;

    if (strchr (text, '%')) 
    {
        va_list args;
        int size = 0;
        int i = 0;

        va_start(args, text);
        do {
            size += 1000;
            if (buf) free(buf);
            buf = (char*)malloc(size);
            i = vsnprintf(buf, size, text, args);
        } while (i == size);
        va_end(args);
    }

    PLOGFONT old = SelectFont (HDC_SCREEN, GetSystemFont (SYSLOGFONT_CAPTION));
    GetTabbedTextExtent (HDC_SCREEN, buf ? buf : text, -1, &text_size);

    text_size.cx += _MARGIN << 1;
    text_size.cy += /*GetSysCharHeight () + */(_MARGIN << 1);
    width = ClientWidthToWindowWidth (WS_THINFRAME, text_size.cx);
    height= ClientHeightToWindowHeight (WS_THINFRAME, text_size.cy, FALSE);

    SetWindowCaption (hwnd, buf ? buf : text);
    if (buf) free (buf);

    if (x + width > g_rcScr.right)
        x = g_rcScr.right - width;
    if (y + height > g_rcScr.bottom)
        y = g_rcScr.bottom - height;

    SelectFont(HDC_SCREEN, old);

    MoveWindow (hwnd, x, y, width, height, TRUE);
    ShowWindow (hwnd, SW_SHOWNORMAL);
}

void mdolphin_destroyToolTipWin (HWND hwnd)
{
    DestroyMainWindow (hwnd);
    MainWindowThreadCleanup (hwnd);
}


