/* 
** $Id$
**
** svgui_test.c: A test program of SVGUI
**
** Copyright (C) 2010 Feynman Software.
**
** Current maintainer: TBD
** Author:  WEI Yongming
** Create date: 2010-02-07
*/

#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "svgui.h"

static int SVGUITestWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_CREATE:
            SetTimer (hWnd, 100, 200);
            break;

        case MSG_TIMER:
            break;
            
        case MSG_LBUTTONDOWN:
            break;

        case MSG_PAINT:
            break;

        case MSG_CLOSE:
            KillTimer (hWnd, 100);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}

static SVGUI_TEXT_AREA_T text_areas_block1 [] =
{
    {1, TRUE, {0, 0, 50, 50}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 0, "Text4Block1"},
};

static SVGUI_IMAGE_T images_block1 [] = 
{
    {16, 16, 16, "gpm-ups-000-16x16.png"},
    {22, 22, 16, "gpm-ups-000-22x22.png"},
    {24, 24, 16, "gpm-ups-000-24x24.png"},
};

static SVGUI_IMAGE_AREA_T image_areas_block1 [] =
{
    {1, TRUE, {50, 0, 100, 50}, SVGUI_IMAGE_FILL_WAY_SCALED, 
            TABLESIZE(images_block1), images_block1},
};

static SVGUI_TEXT_AREA_T text_areas_block2 [] =
{
    {1, TRUE, {0, 0, 100, 50}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 
            0xFF00FF, 1, "Text4Block2"},
};

static SVGUI_PRDRD_BLOCK_T prdrd_blocks [] = 
{
    {100, 50, SVGUI_PRDRD_GRADIENT_HORZ, 0, 25, 50, 0x8b563f, 0xfb7b1e, 0x91411e, 5, 0xFF, 
                10, 0x000000},
    
    {100, 50, SVGUI_PRDRD_GRADIENT_VERT, 0, 50, 100, 0x8b563f, 0xfb7b1e, 0x91411e, 5, 0xFF0000, 
                10, 0x000000},
};

static SVGUI_BLOCK_T blocks [] = 
{
    {0, TRUE, {0, 0, 100, 100}, FALSE, 0, 0, NULL, 0, NULL},
    {1, TRUE, {0, 0, 100, 50}, TRUE, 0, TABLESIZE(text_areas_block1), text_areas_block1, 
                TABLESIZE(image_areas_block1), image_areas_block1},
    {2, TRUE, {0, 50, 100, 100}, TRUE, 1, TABLESIZE(text_areas_block2), text_areas_block2, 0, NULL},
};

static SVGUI_FONT_INFO_T font_infos [] =
{
    {"fmsong", 20, "UTF-8"},
    {"fmhei", 20, "UTF-8"}
};

static SVGUI_HEADER_T header =
{
    100, 100,
    0x000000, 0xFFFFFF,
    TABLESIZE(prdrd_blocks), prdrd_blocks,
    TABLESIZE(font_infos), font_infos,
    TABLESIZE(blocks), blocks,
};

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER , "svgui" , 0 , 0);
#endif

    {
        char cwd [MAX_PATH+1];
        getcwd (cwd, MAX_PATH);
        SetResPath (cwd);
    }

    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
    CreateInfo.spCaption = "svgui test";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = SVGUITestWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = 200;
    CreateInfo.by = 200;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)(&header);
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

