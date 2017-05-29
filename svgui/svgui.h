/*
 * $Id$
 *
 * svgui.h: header for SVGUI module.
 *
 * Copyright (C) 2010 Feynman Software.
 */

#ifndef SVGUI_H
#define SVGUI_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

/* structures for SVGUI template */
typedef struct _SVGUI_IMAGE_T {
    int width;
    int height;
    int depth;
    const char* file_name;
} SVGUI_IMAGE_T;

#define SVGUI_IMAGE_FILL_WAY_CENTER 0
#define SVGUI_IMAGE_FILL_WAY_TILED  1
#define SVGUI_IMAGE_FILL_WAY_SCALED 2

typedef struct _SVGUI_IMAGE_AREA_T {
    int id;
    BOOL is_visible;
    RECT rc;

    int fill_way;

    int nr_images;
    SVGUI_IMAGE_T* images;
} SVGUI_IMAGE_AREA_T;

#define SVGUI_TEXT_HALIGN_LEFT      0x00
#define SVGUI_TEXT_HALIGN_CENTER    0x01
#define SVGUI_TEXT_HALIGN_RIGHT     0x02
#define SVGUI_TEXT_HALIGN_MASK      0x0F

#define SVGUI_TEXT_VALIGN_TOP       0x00
#define SVGUI_TEXT_VALIGN_CENTER    0x10
#define SVGUI_TEXT_VALIGN_BOTTOM    0x20
#define SVGUI_TEXT_VALIGN_MASK      0xF0

typedef struct _SVGUI_TEXT_AREA_T {
    int id;
    BOOL is_visible;
    RECT rc;

    UINT align;
    RGBCOLOR color;
    int idx_font;
    const char* text;
} SVGUI_TEXT_AREA_T;

typedef struct _SVGUI_BLOCK_T {
    int id;
    BOOL is_visible;
    RECT rc;

    BOOL is_hotspot;
    int idx_prdrd_block;
    
    int nr_text_areas;
    SVGUI_TEXT_AREA_T* text_areas;
    
    int nr_image_areas;
    SVGUI_IMAGE_AREA_T* image_areas;
} SVGUI_BLOCK_T;

#define SVGUI_PRDRD_GRADIENT_NONE   0x00
#define SVGUI_PRDRD_GRADIENT_HORZ   0x01
#define SVGUI_PRDRD_GRADIENT_VERT   0x02

typedef struct _SVGUI_PRDRD_BLOCK_T {
    int width, height;
    int gradient_type;
    int pos_c1, pos_c2, pos_c3;
    RGBCOLOR c1, c2, c3;
    
    int border_width;
    RGBCOLOR border_color;
    
    int corner;

    RGBCOLOR color_bk;  /* will be used as color key */
} SVGUI_PRDRD_BLOCK_T;

typedef struct _SVGUI_FONT_INFO_T {
    const char* name;
    int size;
    const char* encoding;
} SVGUI_FONT_INFO_T;

typedef struct _SVGUI_HEADER_T {
    int width, height;
    RGBCOLOR color_bk;
    RGBCOLOR color_key;
    
    int nr_prdrd_blocks;
    SVGUI_PRDRD_BLOCK_T* prdrd_blocks;
    
    int nr_font_infos;
    SVGUI_FONT_INFO_T* font_infos;
    
    int nr_blocks;
    SVGUI_BLOCK_T* blocks;
} SVGUI_HEADER_T;

/* structures for SVGUI instance */
typedef struct _SVGUI_IMAGE_AREA_I {
    int id;
    BOOL is_visible;
    RECT rc;

    int fill_way;

    BITMAP* bmp;
    RES_KEY key;
} SVGUI_IMAGE_AREA_I;

typedef struct _SVGUI_TEXT_AREA_I {
    int id;
    BOOL is_visible;
    RECT rc;

    UINT align;
    gal_pixel text_pixel;
    int idx_font;
    const char* text;
} SVGUI_TEXT_AREA_I;

typedef struct _SVGUI_BLOCK_I {
    int id;
    BOOL is_visible;
    RECT rc;

    BOOL is_hotspot;
    int idx_prdrd_block;
    
    int nr_text_areas;
    SVGUI_TEXT_AREA_I* text_areas;
    
    int nr_image_areas;
    SVGUI_IMAGE_AREA_I* image_areas;
} SVGUI_BLOCK_I;

typedef struct _SVGUI_PRDRD_BLOCK_I {
    int width, height;
    gal_pixel pixel_key; /* will be used as ColorKey */
    
    HDC mem_dc;          /* Pre-rendered MemDC */
} SVGUI_PRDRD_BLOCK_I;

typedef struct _SVGUI_FONT_INFO_I {
    LOGFONT* log_font;
} SVGUI_FONT_INFO_I;

typedef struct _SVGUI_HEADER_I {
    int width, height;
    gal_pixel pixel_bk;
    gal_pixel pixel_key;
    
    int nr_prdrd_blocks;
    SVGUI_PRDRD_BLOCK_I* prdrd_blocks;
    
    int nr_font_infos;
    SVGUI_FONT_INFO_I* font_infos;
    
    int nr_blocks;
    SVGUI_BLOCK_I* blocks;
} SVGUI_HEADER_I;

SVGUI_HEADER_I* svgui_instantiate (const SVGUI_HEADER_T* svgui_template,
                HDC hdc, int width, int height);
void svgui_cleanup (SVGUI_HEADER_I* svgui);

void svgui_draw (SVGUI_HEADER_I* svgui, HDC hdc, const RECT* update_rc);

SVGUI_BLOCK_I* svgui_get_block (SVGUI_HEADER_I* svgui, int idx_block);
SVGUI_TEXT_AREA_I* svgui_get_text_area (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int text_id);
SVGUI_IMAGE_AREA_I* svgui_get_image_area (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int image_id);

SVGUI_BLOCK_I* svgui_get_block_by_point (SVGUI_HEADER_I* svgui,
                int x, int y);
SVGUI_TEXT_AREA_I* svgui_get_text_area_by_point (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int x, int y);
SVGUI_IMAGE_AREA_I* svgui_get_image_area_by_point (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int x, int y);

int DefaultSVGUIMainWinProc (HWND hwnd, int msg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* SVGUI_H */

