#include "mdtv_dbbuffer.h"
#include "mdtv_common.h"

#ifdef WITH_BKGND_PIC
HDC g_dc_bkgnd;
#endif

HDC g_dc_uisec;

void init_dbbuffer(void )
{
#ifdef WITH_BKGND_PIC
    g_dc_bkgnd = CreateCompatibleDC (HDC_SCREEN);
#endif
    g_dc_uisec = CreateCompatibleDC (HDC_SCREEN);

    return ;
}


#ifdef WITH_BKGND_PIC
void init_background_dc(int bgcolor,  const char *filename, DRAW_IMAGEBG_MODE mode)
{
        // draw bg color 
        //SetBrushColor(g_dc_bkgnd, DWORD2PIXEL(g_dc_bkgnd, bgcolor));
        SetBrushColor(g_dc_bkgnd, bgcolor);
        int width = GetGDCapability(g_dc_bkgnd, GDCAP_MAXX);
        int height = GetGDCapability(g_dc_bkgnd, GDCAP_MAXY);
        FillBox(g_dc_bkgnd, 0, 0, width, height);
        
        if (filename == NULL)
        return ;
        
        // draw bk-image
        switch (mode)
        {
            case DRAW_IMAGEBG_TILED:
                {
                    mdtv_paintimage_by_tiled(g_dc_bkgnd, filename);
                    break;
                }

            case DRAW_IMAGEBG_SCALED:
                {
                    mdtv_paintimage_by_scaled(g_dc_bkgnd, 0, 0, width, height, filename);
                    break;
                }

            case DRAW_IMAGEBG_CENTER:
                {
                    mdtv_paintimage_by_center(g_dc_bkgnd, filename);
                    break;
                }

        }
}
#endif

HDC alloc_subdc_from_uisec( int   off_x, int    off_y,int   width,  int height)
{
    HDC hdc = CreateSubMemDC(g_dc_uisec, off_x, off_y, width, height, TRUE);
    return hdc;   
}

void destroy_subdc(HDC hdc)
{
    DeleteMemDC(hdc);      
}
