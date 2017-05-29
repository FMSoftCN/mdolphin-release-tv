/*
 * $Id$
 *
 * mdtv_dbbuffer.h: header for mdtv .
 *
 * Copyright (C) 2010 Feynman Software.
 *
 */

#ifndef _MDTV_DBBUFFER_H
#define _MDTV_DBBUFFER_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "mdtv_common.h"

//#define WITH_BKGND_PIC      0

#ifdef WITH_BKGND_PIC
extern HDC g_dc_bkgnd;
#endif

extern HDC g_dc_uisec;


void init_dbbuffer(void );
void init_background_dc(int bgcolor,  const char *filename, DRAW_IMAGEBG_MODE mode);
HDC alloc_subdc_from_uisec( int   off_x, int    off_y,int   width,  int height);
void destroy_subdc(HDC hdc);



#endif
