#ifndef _MDTV_COMMON_H
#define _MDTV_COMMON_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "svgui.h"

extern SVGUI_FONT_INFO_T ptr_font_infos[4];

typedef enum 
{
    DRAW_IMAGEBG_TILED,
    DRAW_IMAGEBG_SCALED,
    DRAW_IMAGEBG_CENTER
}DRAW_IMAGEBG_MODE;

BOOL mdtv_paintimage_by_tiled(HDC hdc, const char *filename);
BOOL mdtv_paintimage_by_scaled(HDC hdc, int x, int y, int width, int height, const char *filename);
BOOL mdtv_paintimage_by_center(HDC hdc, const char *filename );

#endif
