#include "mdtv_common.h"
#include "svgui.h"

SVGUI_FONT_INFO_T ptr_font_infos[]={
    {"fmhei", 12, "UTF-8"},
    {"fmhei", 18, "UTF-8"},
    {"fmhei", 20, "UTF-8"},
    {"fmhei", 30, "UTF-8"},
};

BOOL mdtv_paintimage_by_tiled(HDC hdc, const char *filename)
{
    int width = GetGDCapability(hdc, GDCAP_MAXX);
    int height = GetGDCapability(hdc, GDCAP_MAXY);
    BITMAP bitmap;

    if (LoadBitmap(hdc, &bitmap, filename))
    {
        fprintf(stderr, "load bitmap %s error \n", filename);
        return FALSE;
    }
    int old_type = SetBrushType(hdc, BT_TILED);
    SetBrushOrigin(hdc, 0, 0);
    SetBrushInfo(hdc, &bitmap, NULL);
    FillBox(hdc, 0, 0, width, height);
    SetBrushType(hdc, old_type);
    UnloadBitmap(&bitmap);
    return TRUE;
}

BOOL mdtv_paintimage_by_scaled(HDC hdc, int x, int y, int width, int height, const char *filename)
{
    if (StretchPaintImageFromFile(hdc, x, y, width, height, filename) == 0)
        return TRUE;

    return FALSE;
}

BOOL mdtv_paintimage_by_center(HDC hdc, const char *filename )
{
    BITMAP bitmap;
    int dc_width = GetGDCapability(hdc, GDCAP_MAXX);
    int dc_height = GetGDCapability(hdc, GDCAP_MAXY);

    if (LoadBitmap(hdc, &bitmap, filename))
    {
        fprintf(stderr, "load bitmap %s error \n", filename);
        return FALSE;
    }
    int left = (dc_width - (int)(bitmap.bmWidth) )/2;
    int top =  (dc_height - (int)(bitmap.bmHeight) )/2;
    int width = bitmap.bmWidth;
    int height =  bitmap.bmHeight;
    if (left < 0) { left = 0; width = dc_width;}
    if (top < 0) { top = 0; height = dc_height;}

    FillBoxWithBitmapPart(hdc, left, top, width, height,0, 0,  &bitmap, 0, 0 );
    UnloadBitmap(&bitmap);

    return TRUE;
}

