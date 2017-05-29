/*
** $Id$
** 
** svgui.c: Implementation of SVGUI module.
** 
** Copyright (C) 2010 Feynman Software.
**
** Current maintainer: TBD
** Author:  WEI Yongming
** Create date: 2010-02-07
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <mgplus/mgplus.h>

#include "svgui.h"

#define SCALE_BY_WIDTH  1

#define DRAW_GRADIENT  1

#define CAL_TIME        0
#if CAL_TIME
static double sg_start_time;
static inline void getTimer(double* timer)
{
    struct timeval tv; 
    gettimeofday(&tv, 0);
    double curtime = tv.tv_sec * 1000000.0 + (double)(tv.tv_usec);
    *timer = curtime; 
}
static void start_time (void)
{
    getTimer(&sg_start_time);
}

static void end_time(void)
{
    double sg_end_time;
    getTimer(&sg_end_time);
    printf("The used time is: %fus, %fms\n ", (sg_end_time - sg_start_time), (sg_end_time - sg_start_time)/1000.0);
}
#endif

static int svgui_init_prdrd_block (SVGUI_PRDRD_BLOCK_I* prdrd_block_i, 
                const SVGUI_PRDRD_BLOCK_T* prdrd_block_t, HDC hdc, float scale)
{
    int width = (int)(prdrd_block_t->width * scale+0.5);
    int height = (int)(prdrd_block_t->height * scale+0.5);

    prdrd_block_i->width = width;
    prdrd_block_i->height = height;

    HDC memdc;

    memdc = CreateCompatibleDCEx (hdc, width, height);
    if (memdc == HDC_INVALID) {
        fprintf (stderr, "SVGUI: can not create memdc: %dx%d\n", width, height);
        return 1;
    }

    prdrd_block_i->mem_dc = memdc;

    prdrd_block_i->pixel_key = RGBA2Pixel (memdc, 
            GetRValue (prdrd_block_t->color_bk),
            GetGValue (prdrd_block_t->color_bk),
            GetBValue (prdrd_block_t->color_bk),
            GetAValue (prdrd_block_t->color_bk));
    SetBrushColor (memdc, prdrd_block_i->pixel_key);
    FillBox (memdc, 0, 0, width, height);

    switch (prdrd_block_t->gradient_type) {
        case SVGUI_PRDRD_GRADIENT_NONE:
        {
            int pen_width = (int)(prdrd_block_t->border_width * scale);
            int radius = (int)(prdrd_block_t->corner * scale);

            SetPenColor (memdc, 
                RGB2Pixel (memdc, GetRValue (prdrd_block_t->border_color),
                                GetGValue (prdrd_block_t->border_color),
                                GetBValue (prdrd_block_t->border_color)));
            SetPenWidth (memdc, pen_width);
            SetBrushColor (memdc, 
                RGB2Pixel (memdc, GetRValue (prdrd_block_t->c1),
                                GetGValue (prdrd_block_t->c1),
                                GetBValue (prdrd_block_t->c1)));
            RoundRect (memdc, 0, 0, width, height, radius, radius);
            break;
        }

        case SVGUI_PRDRD_GRADIENT_HORZ:
        {
            int pen_width = (int)(prdrd_block_t->border_width * scale);
            int radius = (int)(prdrd_block_t->corner * scale);
            RECT drawrect = {0, 0, width, height};
            float fheight = (float)(prdrd_block_t->height);
            ARGB color [3];
            HGRAPHICS hgs;
            HBRUSH brush;
            HPEN hpen;

            hgs = MGPlusGraphicCreateWithoutCanvas (memdc);
            brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT);
            MGPlusSetLinearGradientBrushMode (brush, MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
            color[0] = (prdrd_block_t->c1 | (0xFF << 24) ); 
            color[1] = (prdrd_block_t->c2 | (0xFF << 24) ); 
            color[2] = (prdrd_block_t->c3 | (0xFF << 24) ); 

            MGPlusLinearGradientBrushAddColor (brush, color[0], 0.0);
            MGPlusLinearGradientBrushAddColor (brush, color[1], 
                    (float)(prdrd_block_t->pos_c2/fheight));
            MGPlusLinearGradientBrushAddColor (brush, color[2], 1.0);
#if CAL_TIME
printf("------ SVGUI_PRDRD_GRADIENT_HORZ: [width,height]:[%d,%d]\n",width,height);
start_time();
#endif
            if (radius > 0){
#if DRAW_GRADIENT
                int i;
                RECT small_rc = {0, 0, width, 3*radius};

                MGPlusSetLinearGradientBrushRect (brush, &small_rc);
                MGPlusFillRoundRectI (hgs, brush, small_rc.left, small_rc.top, 
                        RECTW(small_rc),  RECTH(small_rc), radius);

                /* tile */
                BitBlt (memdc, 0, radius*2, width, radius, memdc, 0, height-radius, 0);
                for (i=radius; i<height-radius; i++){
                    BitBlt (memdc, 0, radius, width, 1, memdc, 0, i, 0);
                }
#else
                MGPlusSetLinearGradientBrushRect (brush, &drawrect);
                MGPlusFillRoundRectI (hgs, brush, drawrect.left, drawrect.top, 
                    RECTW(drawrect),  RECTH(drawrect), radius);
#endif
            }else{
#if DRAW_GRADIENT
                int i;
                RECT small_rc = {0, 0, width, 1};

                MGPlusSetLinearGradientBrushRect (brush, &small_rc);
                MGPlusFillRectangleI (hgs, brush, small_rc.left , small_rc.top, 
                    RECTW(small_rc),  RECTH(small_rc));
                for (i=1; i<height; i++) {
                    BitBlt (memdc, 0, 0, width, 1, memdc, 0, i, 0);
                }
#else
                MGPlusSetLinearGradientBrushRect (brush, &drawrect);
                MGPlusFillRectangleI (hgs, brush, drawrect.left , drawrect.top, 
                    RECTW(drawrect),  RECTH(drawrect));
#endif
            }
#if CAL_TIME
end_time();
#endif
            if (pen_width > 0)
            {
                hpen = MGPlusPenCreate (pen_width, (prdrd_block_t->border_color | 0xFF<<24));
                if (radius > 0)
                    MGPlusDrawRoundRect (hgs, hpen,  drawrect.left, drawrect.top, 
                        RECTW(drawrect), RECTH(drawrect), radius);
                else
                    MGPlusDrawRectangleI (hgs, hpen,  drawrect.left, drawrect.top, 
                        RECTW(drawrect), RECTH(drawrect));
                MGPlusPenDelete (hpen);
            }
            MGPlusBrushDelete (brush);
            MGPlusGraphicDelete (hgs);
            break;
        }

        case SVGUI_PRDRD_GRADIENT_VERT:
        {
            int pen_width = (int)(prdrd_block_t->border_width * scale);
            int radius = (int)(prdrd_block_t->corner * scale);
            RECT drawrect = {0, 0, width, height};
            float fwidth = (float)(prdrd_block_t->width);
            ARGB color [3];
            HGRAPHICS hgs ;
            HBRUSH brush;
            HPEN hpen;

            hgs = MGPlusGraphicCreateWithoutCanvas (memdc);

            brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT);
            MGPlusSetLinearGradientBrushMode (brush, MP_LINEAR_GRADIENT_MODE_VERTICAL);
            color[0] = (prdrd_block_t->c1 | (0xFF << 24) ); 
            color[1] = (prdrd_block_t->c2 | (0xFF << 24) ); 
            color[2] = (prdrd_block_t->c3 | (0xFF << 24) ); 

            MGPlusLinearGradientBrushAddColor (brush, color[0], 0.0);
            MGPlusLinearGradientBrushAddColor (brush, color[1], 
                    (float)(prdrd_block_t->pos_c2/fwidth));
            MGPlusLinearGradientBrushAddColor (brush, color[2], 1.0);

#if CAL_TIME
printf("------ SVGUI_PRDRD_GRADIENT_VERT: [width,height]:[%d,%d]\n",width,height);
start_time();
#endif
            if (radius > 0){
#if DRAW_GRADIENT
                int i;
                RECT small_rc = {0, 0, 3*radius, height};

                MGPlusSetLinearGradientBrushRect (brush, &small_rc);
                MGPlusFillRoundRectI (hgs, brush, small_rc.left, small_rc.top, 
                        RECTW(small_rc),  RECTH(small_rc), radius);

                /* tile */
                BitBlt (memdc, radius*2, 0, radius, height, memdc, width-radius, 0, 0);
                for (i=radius; i<width-radius; i++) {
                    BitBlt (memdc, radius, 0, 1, height, memdc, i, 0, 0);
                }
#else
                MGPlusSetLinearGradientBrushRect (brush, &drawrect);
                MGPlusFillRoundRectI (hgs, brush, drawrect.left , drawrect.top, 
                    RECTW(drawrect),  RECTH(drawrect), radius);
#endif
            }
            else {
#if DRAW_GRADIENT
                int i;
                RECT small_rc = {0, 0, 1, height};

                MGPlusSetLinearGradientBrushRect (brush, &small_rc);
                MGPlusFillRectangleI (hgs, brush, small_rc.left , small_rc.top, 
                    RECTW(small_rc),  RECTH(small_rc));
                for (i = 1; i < width; i++){
                   BitBlt (memdc, 0, 0, 1, height, memdc, i, 0, 0);
                }
#else
                MGPlusSetLinearGradientBrushRect (brush, &drawrect);
                MGPlusFillRectangleI (hgs, brush, drawrect.left , drawrect.top, 
                    RECTW(drawrect),  RECTH(drawrect));
#endif
            }
#if CAL_TIME
end_time();
#endif
            if (pen_width > 0) {
                hpen = MGPlusPenCreate (pen_width, (prdrd_block_t->border_color | 0xFF<<24));
                if (radius > 0)
                    MGPlusDrawRoundRectI (hgs, hpen,  drawrect.left, drawrect.top, 
                        RECTW(drawrect), RECTH(drawrect), radius);
                else 
                    MGPlusDrawRectangleI (hgs, hpen,  drawrect.left, drawrect.top, 
                        RECTW(drawrect), RECTH(drawrect));
                MGPlusPenDelete (hpen);
            }

            MGPlusBrushDelete (brush);
            MGPlusGraphicDelete (hgs);
            break;
        }
        default:
            fprintf (stderr, "SVGUI: unknown gradient type: %d\n", prdrd_block_t->gradient_type);
            return 1;
    }
    return 0;
}

static void svgui_deinit_prdrd_block (SVGUI_PRDRD_BLOCK_I* prdrd_block_i)
{
    if (prdrd_block_i->mem_dc != HDC_INVALID 
            && prdrd_block_i->mem_dc != 0) {
        DeleteMemDC (prdrd_block_i->mem_dc);
    }
}

static int svgui_init_font_info (SVGUI_FONT_INFO_I* font_info_i, 
                const SVGUI_FONT_INFO_T* font_info_t, HDC hdc, float scale)
{
    int size = (int)(font_info_t->size * scale);
    char font_name [256];

    sprintf (font_name, "ttf-%s-rrncnn-*-%d-%s", font_info_t->name, size, 
            font_info_t->encoding);

    font_info_i->log_font = CreateLogFontByName (font_name);
    if (font_info_i->log_font == NULL) {
        fprintf (stderr, "SVGUI: can not create logical font: %s\n", font_name);
        return 1;
    }

    return 0;
}

static void svgui_deinit_font_info (SVGUI_FONT_INFO_I* font_info_i)
{
    if (font_info_i->log_font != NULL) {
        DestroyLogFont (font_info_i->log_font);
    }
}

static int svgui_init_block_text_area (SVGUI_TEXT_AREA_I* text_area_i, 
                const SVGUI_TEXT_AREA_T* text_area_t, HDC hdc, float scale)
{
    text_area_i->id = text_area_t->id;
    text_area_i->is_visible = text_area_t->is_visible;

    text_area_i->rc.top = (int)(text_area_t->rc.top * scale);
    text_area_i->rc.left = (int)(text_area_t->rc.left * scale);
    text_area_i->rc.right = (int)(text_area_t->rc.right * scale);
    text_area_i->rc.bottom = (int)(text_area_t->rc.bottom * scale);

    text_area_i->align = text_area_t->align;
    text_area_i->idx_font = text_area_t->idx_font;

    /* need to copy? */
    text_area_i->text = text_area_t->text;

    text_area_i->text_pixel = RGB2Pixel (hdc, 
                        GetRValue (text_area_t->color),
                        GetGValue (text_area_t->color),
                        GetBValue (text_area_t->color));
    return 0;
}

static void svgui_deinit_block_text_area (SVGUI_TEXT_AREA_I* text_area_i)
{
    /* do nothing currently */
}

static int svgui_init_block_image_area (SVGUI_IMAGE_AREA_I* image_area_i, 
                const SVGUI_IMAGE_AREA_T* image_area_t, HDC hdc, float scale)
{
    int i, best_image;
    int area_size;

    image_area_i->id = image_area_t->id;
    image_area_i->is_visible = image_area_t->is_visible;

    image_area_i->rc.top = (int)(image_area_t->rc.top * scale);
    image_area_i->rc.left = (int)(image_area_t->rc.left * scale);
    image_area_i->rc.right = (int)(image_area_t->rc.right * scale);
    image_area_i->rc.bottom = (int)(image_area_t->rc.bottom * scale);

    image_area_i->fill_way = image_area_t->fill_way;

    best_image = 0;
#if SCALE_BY_WIDTH
    area_size = RECTW (image_area_i->rc)-12;
    for (i = 0; i < image_area_t->nr_images; i++) {
        if (image_area_t->images[i].width > area_size)
            break;
        best_image = i;
    }
#else
    area_size = RECTH (image_area_i->rc)-12;
    for (i = 0; i < image_area_t->nr_images; i++) {
        if (image_area_t->images[i].height > area_size)
            break;
        best_image = i;
    }
#endif
    image_area_i->bmp = (BITMAP*)LoadResource (image_area_t->images[best_image].file_name, 
                                RES_TYPE_IMAGE, hdc);


    if (image_area_i->bmp == NULL) {
        fprintf (stderr, "SVGUI: can not load resource: %s\n", 
                image_area_t->images[best_image].file_name);
        return 1;
    }
    else {
        image_area_i->key = Str2Key (image_area_t->images[best_image].file_name);
    }

    return 0;
}

static void svgui_deinit_block_image_area (SVGUI_IMAGE_AREA_I* image_area_i)
{
    /* release the BITMAP object */
    if (image_area_i->bmp) {
        if (ReleaseRes (image_area_i->key) < 0) {
            fprintf (stderr, "SVGUI: can not release resource.\n");
        }
    }
}

static int svgui_init_block (SVGUI_BLOCK_I* block_i, 
                const SVGUI_BLOCK_T* block_t, HDC hdc, float scale)
{
    int i;

    block_i->id = block_t->id;
    block_i->rc.top = (int)(block_t->rc.top * scale);
    block_i->rc.left = (int)(block_t->rc.left * scale);
    block_i->rc.right = (int)(block_t->rc.right * scale);
    block_i->rc.bottom = (int)(block_t->rc.bottom * scale);
    block_i->is_visible = block_t->is_visible;
    block_i->is_hotspot = block_t->is_hotspot;
    block_i->idx_prdrd_block = block_t->idx_prdrd_block;

    if (block_t->nr_text_areas > 0) {
        block_i->text_areas = (SVGUI_TEXT_AREA_I*) calloc (block_t->nr_text_areas,
                        sizeof (SVGUI_TEXT_AREA_I));
        if (block_i->text_areas == NULL)
            goto error;

        block_i->nr_text_areas = block_t->nr_text_areas;
    }
    else {
        block_i->nr_text_areas = 0;
        block_i->text_areas = NULL;
    }

    for (i = 0; i < block_i->nr_text_areas; i++) {
        SVGUI_TEXT_AREA_T* text_area_t = block_t->text_areas + i;
        SVGUI_TEXT_AREA_I* text_area_i = block_i->text_areas + i;

        if (svgui_init_block_text_area (text_area_i, text_area_t, hdc, scale))
            goto error;
    }

    if (block_t->nr_image_areas > 0) {
        block_i->image_areas = (SVGUI_IMAGE_AREA_I*) calloc (block_t->nr_image_areas,
                        sizeof (SVGUI_IMAGE_AREA_I));
        if (block_i->image_areas == NULL)
            goto error;
        block_i->nr_image_areas = block_t->nr_image_areas;
    }
    else {
        block_i->nr_image_areas = 0;
        block_i->image_areas = NULL;
    }

    for (i = 0; i < block_i->nr_image_areas; i++) {
        SVGUI_IMAGE_AREA_T* image_area_t = block_t->image_areas + i;
        SVGUI_IMAGE_AREA_I* image_area_i = block_i->image_areas + i;

        if (svgui_init_block_image_area (image_area_i, image_area_t, hdc, scale))
            goto error;
    }

    return 0;

error:
    fprintf (stderr, "SVGUI: error in svgui_init_block\n");
    return 1;
}

static void svgui_deinit_block (SVGUI_BLOCK_I* block_i)
{
    int i;

    for (i = 0; i < block_i->nr_text_areas; i++) {
        SVGUI_TEXT_AREA_I* text_area_i = block_i->text_areas + i;

        svgui_deinit_block_text_area (text_area_i);
    }

    for (i = 0; i < block_i->nr_image_areas; i++) {
        SVGUI_IMAGE_AREA_I* image_area_i = block_i->image_areas + i;

        svgui_deinit_block_image_area (image_area_i);
    }

    free (block_i->text_areas);
    free (block_i->image_areas);
}

SVGUI_HEADER_I* svgui_instantiate (const SVGUI_HEADER_T* svgui_t,
                HDC hdc, int width, int height)
{
    int i;
    float scale;
    SVGUI_HEADER_I* svgui_i;

    if (width <= 0 || height <= 0) {
        fprintf (stderr, "SVGUI: invalid width and height: (%dx%d)\n", width, height);
        return NULL;
    }

#if SCALE_BY_WIDTH
    /* scale according to width */
    scale = width / (float)svgui_t->width;
#else
    /* scale according to height */
    scale = height / (float)svgui_t->height;
#endif

    /* too small scale */
    if (scale < 0.001f) {
        fprintf (stderr, "SVGUI: too small scale factor: %f\n", scale);
        return NULL;
    }

    /* allocate memory for SVGUI instance */
    svgui_i = (SVGUI_HEADER_I*)calloc (1, sizeof (SVGUI_HEADER_I));
    if (svgui_i == NULL)
        goto error;

    svgui_i->width = (int)width;
    svgui_i->height = (int)height;

    svgui_i->pixel_bk = RGBA2Pixel (hdc, 
                        GetRValue (svgui_t->color_bk),
                        GetGValue (svgui_t->color_bk),
                        GetBValue (svgui_t->color_bk),
                        GetAValue (svgui_t->color_bk));

    svgui_i->pixel_key = RGBA2Pixel (hdc, 
                        GetRValue (svgui_t->color_key),
                        GetGValue (svgui_t->color_key),
                        GetBValue (svgui_t->color_key),
                        GetAValue (svgui_t->color_key));

    if (svgui_t->nr_prdrd_blocks > 0) {
        svgui_i->prdrd_blocks = (SVGUI_PRDRD_BLOCK_I*)calloc (svgui_t->nr_prdrd_blocks, 
                                sizeof (SVGUI_PRDRD_BLOCK_I));
        if (svgui_i->prdrd_blocks == NULL)
            goto error;

        svgui_i->nr_prdrd_blocks = svgui_t->nr_prdrd_blocks;
    }
    else {
        svgui_i->nr_prdrd_blocks = 0;
        svgui_i->prdrd_blocks = NULL;
    }

    for (i = 0; i < svgui_i->nr_prdrd_blocks; i++) {
        SVGUI_PRDRD_BLOCK_T* prdrd_block_t = svgui_t->prdrd_blocks + i;
        SVGUI_PRDRD_BLOCK_I* prdrd_block_i = svgui_i->prdrd_blocks + i;

        if (svgui_init_prdrd_block (prdrd_block_i, prdrd_block_t, hdc, scale))
            goto error;
    }

    if (svgui_t->nr_font_infos > 0) {
        svgui_i->font_infos = (SVGUI_FONT_INFO_I*)calloc (svgui_t->nr_font_infos, 
                                sizeof (SVGUI_FONT_INFO_I));
        if (svgui_i->font_infos == NULL)
            goto error;

        svgui_i->nr_font_infos = svgui_t->nr_font_infos;
    }
    else {
        svgui_i->nr_font_infos = 0;
        svgui_i->font_infos = NULL;
    }

    for (i = 0; i < svgui_i->nr_font_infos; i++) {
        SVGUI_FONT_INFO_T* font_info_t = svgui_t->font_infos + i;
        SVGUI_FONT_INFO_I* font_info_i = svgui_i->font_infos + i;

        if (svgui_init_font_info (font_info_i, font_info_t, hdc, scale))
            goto error;
    }

    if (svgui_t->nr_blocks > 0) {
        svgui_i->blocks = (SVGUI_BLOCK_I*)calloc (svgui_t->nr_blocks, 
                                sizeof (SVGUI_BLOCK_I));
        if (svgui_i->blocks == NULL)
            goto error;
        svgui_i->nr_blocks = svgui_t->nr_blocks;
    }
    else {
        svgui_i->nr_blocks = 0;
        svgui_i->blocks = NULL;
    }

    for (i = 0; i < svgui_i->nr_blocks; i++) {
        SVGUI_BLOCK_T* block_t = svgui_t->blocks + i;
        SVGUI_BLOCK_I* block_i = svgui_i->blocks + i;

        if (svgui_init_block (block_i, block_t, hdc, scale))
            goto error;
    }

    return svgui_i;

error:
    fprintf (stderr, "SVGUI: error in svgui_instantiate\n");

    if (svgui_i)
        svgui_cleanup (svgui_i);

    return NULL;
}

void svgui_cleanup (SVGUI_HEADER_I* svgui)
{
    int i;

    if (svgui == NULL)
        return;

    for (i = 0; i < svgui->nr_prdrd_blocks; i++) {
        SVGUI_PRDRD_BLOCK_I* prdrd_block = svgui->prdrd_blocks + i;

        svgui_deinit_prdrd_block (prdrd_block);
    }

    free (svgui->prdrd_blocks);

    for (i = 0; i < svgui->nr_font_infos; i++) {
        SVGUI_FONT_INFO_I* font_info = svgui->font_infos + i;

        svgui_deinit_font_info (font_info);
    }

    free (svgui->font_infos);

    for (i = 0; i < svgui->nr_blocks; i++) {
        SVGUI_BLOCK_I* block = svgui->blocks + i;

        svgui_deinit_block (block);
    }

    free (svgui->blocks);

    free (svgui);
}

static void svgui_draw_block (HDC hdc, const SVGUI_HEADER_I* svgui, const SVGUI_BLOCK_I* block)
{
    int i;

    SetMemDCColorKey (svgui->prdrd_blocks[block->idx_prdrd_block].mem_dc, 
                MEMDC_FLAG_SRCCOLORKEY, 
                svgui->prdrd_blocks[block->idx_prdrd_block].pixel_key);
    BitBlt (svgui->prdrd_blocks[block->idx_prdrd_block].mem_dc, 0, 0, 
                svgui->prdrd_blocks[block->idx_prdrd_block].width,
                svgui->prdrd_blocks[block->idx_prdrd_block].height,
                hdc, block->rc.left, block->rc.top, 0);

    for (i = 0; i < block->nr_text_areas; i++) {
        SVGUI_TEXT_AREA_I* text_area = block->text_areas + i;

        if (text_area->is_visible) {
            UINT format = DT_SINGLELINE;
            RECT rc = text_area->rc;

            switch (text_area->align & SVGUI_TEXT_HALIGN_MASK) {
            case SVGUI_TEXT_HALIGN_LEFT:
                format |= DT_LEFT;
                break;
            case SVGUI_TEXT_HALIGN_CENTER:
                format |= DT_CENTER;
                break;
            case SVGUI_TEXT_HALIGN_RIGHT:
                format |= DT_RIGHT;
                break;
            }

            switch (text_area->align & SVGUI_TEXT_VALIGN_MASK) {
            case SVGUI_TEXT_VALIGN_TOP:
                format |= DT_TOP;
                break;
            case SVGUI_TEXT_VALIGN_CENTER:
                format |= DT_VCENTER;
                break;
            case SVGUI_TEXT_VALIGN_BOTTOM:
                format |= DT_BOTTOM;
                break;
            }

            SetBkMode (hdc, BM_TRANSPARENT);
            SetTextColor (hdc, text_area->text_pixel);
            SelectFont (hdc, svgui->font_infos[text_area->idx_font].log_font);

            OffsetRect (&rc, block->rc.left, block->rc.top);
            
            format |= DT_CHARBREAK;

            DrawText (hdc, text_area->text, -1, &rc, format);
        }
    }

    for (i = 0; i < block->nr_image_areas; i++) {
        SVGUI_IMAGE_AREA_I* image_area = block->image_areas + i;

        if (image_area->is_visible) {
            RECT rc = image_area->rc;
            OffsetRect (&rc, block->rc.left, block->rc.top);

            switch (image_area->fill_way) {
                case SVGUI_IMAGE_FILL_WAY_CENTER:
                {
                    int x = rc.left + (RECTW(rc) - (int)image_area->bmp->bmWidth)/2;
                    int y = rc.top + (RECTH(rc) - (int)image_area->bmp->bmHeight)/2;
                    FillBoxWithBitmap (hdc, x, y, 0, 0, image_area->bmp);
                    break;
                }
                case SVGUI_IMAGE_FILL_WAY_SCALED:
                    FillBoxWithBitmap (hdc, rc.left, rc.top, RECTW(rc), RECTH(rc), image_area->bmp);
                    break;
                case SVGUI_IMAGE_FILL_WAY_TILED:
                {
                    int old_bt = SetBrushType (hdc, BT_TILED);
                    SetBrushInfo (hdc, image_area->bmp, NULL);
                    SetBrushOrigin (hdc, rc.left, rc.top);
                    FillBox (hdc, rc.left, rc.top, RECTW(rc), RECTH(rc));
                    SetBrushType (hdc, old_bt);
                    break;
                }
            }
        }
    }
}

void svgui_draw (SVGUI_HEADER_I* svgui, HDC hdc, const RECT* update_rc)
{
    int i;

    for (i = 0; i < svgui->nr_blocks; i++) {
        SVGUI_BLOCK_I* block = svgui->blocks + i;
        if (block->is_visible && (update_rc == NULL || DoesIntersect (&block->rc, update_rc)))
            svgui_draw_block (hdc, svgui, block);
    }
}

SVGUI_BLOCK_I* svgui_get_block (SVGUI_HEADER_I* svgui, int block_id)
{
    int i;

    if (svgui == NULL)
        return NULL;

    for (i = 0; i < svgui->nr_blocks; i++) {
        SVGUI_BLOCK_I* block = svgui->blocks + i;
        if (block->id == block_id)
            return block;
    }

    return NULL;
}

SVGUI_TEXT_AREA_I* svgui_get_text_area (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int text_id)
{
    int i;

    if (block == NULL)
        return NULL;

    for (i = 0; i < block->nr_text_areas; i++) {
        SVGUI_TEXT_AREA_I* text_area = block->text_areas + i;
        if (text_area->id == text_id)
            return text_area;
    }

    return NULL;
}

SVGUI_IMAGE_AREA_I* svgui_get_image_area (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int image_id)
{
    int i;

    if (block == NULL)
        return NULL;

    for (i = 0; i < block->nr_image_areas; i++) {
        SVGUI_IMAGE_AREA_I* image_area = block->image_areas + i;
        if (image_area->id == image_id)
            return image_area;
    }

    return NULL;
}

SVGUI_BLOCK_I* svgui_get_block_by_point (SVGUI_HEADER_I* svgui,
                int x, int y)
{
    int i;

    if (svgui == NULL)
        return NULL;

    for (i = 0; i < svgui->nr_blocks; i++) {
        SVGUI_BLOCK_I* block = svgui->blocks + i;
        if (block->is_visible && block->is_hotspot && PtInRect (&block->rc, x, y))
            return block;
    }

    return NULL;
}

SVGUI_TEXT_AREA_I* svgui_get_text_area_by_point (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int x, int y)
{
    int i;

    if (block == NULL)
        return NULL;

    for (i = 0; i < block->nr_text_areas; i++) {
        SVGUI_TEXT_AREA_I* text_area = block->text_areas + i;
        RECT rc = text_area->rc;
        OffsetRect (&rc, block->rc.left, block->rc.top);

        if (PtInRect (&rc, x, y))
            return text_area;
    }

    return NULL;
}

SVGUI_IMAGE_AREA_I* svgui_get_image_area_by_point (SVGUI_HEADER_I* svgui,
                SVGUI_BLOCK_I* block, int x, int y)
{
    int i;

    if (block == NULL)
        return NULL;

    for (i = 0; i < block->nr_image_areas; i++) {
        SVGUI_IMAGE_AREA_I* image_area = block->image_areas + i;
        RECT rc = image_area->rc;
        OffsetRect (&rc, block->rc.left, block->rc.top);

        if (PtInRect (&rc, x, y))
            return image_area;
    }

    return NULL;
}

int DefaultSVGUIMainWinProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MSG_CREATE:
    {
        HDC hdc;
        HDC sec_dc;
        RECT rc_client;
        PMAINWINCREATE create_info = (PMAINWINCREATE)lParam;
        SVGUI_HEADER_I* svgui;
        gal_pixel pixel_colorkey;
        SVGUI_HEADER_T* header = (SVGUI_HEADER_T*)create_info->dwAddData;

        sec_dc =GetSecondaryDC (hwnd);
        if (sec_dc) {
            pixel_colorkey = RGB2Pixel (sec_dc, 
                    GetRValue (header->color_key),
                    GetGValue (header->color_key),
                    GetBValue (header->color_key));
            SetBrushColor (sec_dc, pixel_colorkey);
            FillBox (sec_dc, 0, 0, GetGDCapability (sec_dc, GDCAP_MAXX),  GetGDCapability (sec_dc, GDCAP_MAXY));
        }
        GetClientRect (hwnd, &rc_client);
        hdc = GetClientDC (hwnd);
        svgui = svgui_instantiate (header, hdc, 
                    RECTW(rc_client), RECTH(rc_client));
        ReleaseDC (hdc);

        if (svgui == NULL) {
            printf ("SVGUI: can not instantiate the SVGUI template.\n");
            return 1;
        }
        SetWindowAdditionalData (hwnd, (DWORD)svgui);
        break;
    }

    case MSG_ERASEBKGND:
    {
        const RECT* clip_rc = (const RECT *)lParam;
        SVGUI_HEADER_I* svgui = (SVGUI_HEADER_I*) GetWindowAdditionalData (hwnd);

        if (svgui) {
            HDC hdc;
            if (wParam == 0)
                hdc = GetClientDC (hwnd);
            else
                hdc = wParam;

            svgui_draw (svgui, hdc, clip_rc);

            if (wParam == 0)
                ReleaseDC (hdc);
        }

        return 0; 
    }

    case MSG_DESTROY:
    {
        SVGUI_HEADER_I* svgui = (SVGUI_HEADER_I*) GetWindowAdditionalData (hwnd);
        if (svgui) 
            svgui_cleanup (svgui);
        break;
    }
    }

    return DefaultMainWinProc (hwnd, message, wParam, lParam);
}

