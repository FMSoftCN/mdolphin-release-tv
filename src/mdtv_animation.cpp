#include <unistd.h>
#include <string.h>

#include "svgui.h"
#include "mdtv_animation.h"
#include "mdtv_weather.h"
#include "mdtv_dbbuffer.h"

// for toolbar animation
void AnimationMoveWndUpDown ( HWND hWnd, RECT dst_effrc_1, int dirrect,  int interval, int frames )
{
    int offset;
    int offset_total;
    int width_rect;
    int height_rect;
    SVGUI_HEADER_I  *svgui_header_i;
    HDC hdc_secondary;
    HDC hdc_client;
    RECT curr_dst_rc;//current rect bliting(stretch blitting)from secondary dc to HDCSCRENN
    RECT dst_effrc;
    if(!GetWindowRect(hWnd, &dst_effrc)){
        fprintf(stderr, "[ERROR] can't get window rect\n");
        return;
    }
    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hWnd);
    hdc_client = GetClientDC(hWnd);

    width_rect = RECTW(dst_effrc);
    height_rect = RECTH(dst_effrc);

    // draw secondary dc: sub_dc_toolbar_move
    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
    if( svgui_header_i ){
        svgui_draw ( svgui_header_i, hdc_secondary, NULL);
    }

    // offset of position between frames
    offset = height_rect/frames;
    offset_total = 0;

    curr_dst_rc.left = 0;
    curr_dst_rc.right  = width_rect;
    curr_dst_rc.bottom = height_rect;
    // start animation
    for ( int i = 0; i < frames; i++ ){
        // Blitting Secondary DC to real DC
        if( dirrect == MOVE_UP ){
            BitBlt( hdc_secondary, 0, 0,  width_rect,  offset_total, hdc_client, 0 , height_rect-offset_total, 0 );
            offset_total += offset;
        } else if( dirrect == MOVE_DOWN ){
            curr_dst_rc.top  = offset_total;
            ExcludeClipRect(hdc_client, &curr_dst_rc );
#ifdef WITH_BKGND_PIC
            BitBlt( g_dc_bkgnd, dst_effrc.left, dst_effrc.top, width_rect, height_rect, hdc_client, 0, 0, 0 );
#else
            SetBrushColor(hdc_client, RGB2Pixel(hdc_client, 0x00, 0x00, 0x00) );
            FillBox(hdc_client, 0, 0, width_rect, height_rect);
#endif
            IncludeClipRect(hdc_client, &curr_dst_rc );
            BitBlt( hdc_secondary, 0, 0,  width_rect,  height_rect-offset_total, hdc_client, 0, offset_total, 0 );
            if(i == frames-2)
                offset_total =  width_rect;
            else if(i < frames-2)
                offset_total += offset;
            if( i== frames-1 ){
            }
        }
        usleep(interval*1000);
        //usleep(1000*1000);
    }
#ifdef WITH_BKGND_PIC
    BitBlt( g_dc_bkgnd, dst_effrc.left, dst_effrc.top, width_rect, height_rect, hdc_client, 0, 0, 0 );
#else
    SetBrushColor(hdc_client, RGB2Pixel(hdc_client, 0x00, 0x00, 0x00) );
    FillBox(hdc_client, 0, 0, width_rect, height_rect);
#endif
    ReleaseDC(hdc_client);
}
// display animation for level 2'menu or web page
void AnimationMoveWndLeftRight ( HWND hWnd, RECT dst_effrc_1, int dirrect,  int interval, int frames )
{
    HDC hdc_secondary;
    HDC hdc_client;
    int width_rect;
    int height_rect;
    int offset;
    int offset_total;
    SVGUI_HEADER_I* svgui_header_i;
    RECT curr_dst_rc;//current rect bliting(stretch blitting)from secondary dc to HDCSCRENN

    RECT dst_effrc;
    if(!GetWindowRect(hWnd, &dst_effrc)){
        fprintf(stderr, "[ERROR] can't get window rect\n");
        return;
    }
    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hWnd);
    hdc_client    = GetClientDC(hWnd);

    width_rect  = RECTW(dst_effrc);
    height_rect = RECTH(dst_effrc);
    // offset of position between frames
    offset = width_rect/frames;
    offset_total = 0;

    if ( dirrect == MOVE_RIGHT ){
        // draw secondary dc: sub_dc_toolbar_move
        svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
        svgui_draw ( svgui_header_i, hdc_secondary, NULL);
    }
    curr_dst_rc.left = 0;
    curr_dst_rc.top  = 0;
    for ( int i = 0; i < frames; i++ ){
        if( dirrect == MOVE_RIGHT ){
            BitBlt( hdc_secondary, width_rect-offset_total, 0,  offset_total,  height_rect, hdc_client, 0 , 0, 0 );
        } else if( dirrect == MOVE_LEFT ){
            curr_dst_rc.right  = width_rect-offset_total;
            curr_dst_rc.bottom = height_rect;
            ExcludeClipRect(hdc_client, &curr_dst_rc );
#ifdef WITH_BKGND_PIC
            BitBlt( g_dc_bkgnd, dst_effrc.left, dst_effrc.top, width_rect, height_rect, hdc_client, 0, 0, 0 );
#else
            SetBrushColor(hdc_client, RGB2Pixel(hdc_client, 0x00, 0x00, 0x00) );
            FillBox(hdc_client, 0, 0, width_rect, height_rect);
#endif
            IncludeClipRect(hdc_client, &curr_dst_rc );
            BitBlt( hdc_secondary, offset_total, 0,  width_rect-offset_total,  height_rect, hdc_client, 0, 0, 0 );
        }
        usleep(interval*1000);
        if(i == frames-2)
            offset_total =  width_rect;
        else if(i < frames-2)
            offset_total =  offset_total+(width_rect-offset_total)/3;
    }
    ReleaseDC(hdc_client);
}

int AnimationZoomOutIn( HWND hWnd, RECT src_rc, RECT dst_rc, bool is_zoom_in , int interval, int frames )
{
    HDC hdc_secondary;
    RECT curr_dst_rc;//current rect bliting(stretch blitting)from secondary dc to HDCSCRENN
    int width_inc;
    int height_inc, real_dst_height, real_src_height;

    SVGUI_HEADER_I* svgui_header_i;

    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hWnd); // hWnd

    ExcludeClipRect(HDC_SCREEN, &src_rc);
#ifdef WITH_BKGND_PIC
    BitBlt( g_dc_bkgnd,0,0,0,0, HDC_SCREEN, 0, 0, 0 );
#else
    SetBrushColor(HDC_SCREEN, RGB2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00) );
    FillBox(HDC_SCREEN, 0, 0, GetGDCapability(HDC_SCREEN, GDCAP_MAXX), GetGDCapability(HDC_SCREEN, GDCAP_MAXY));
#endif
    IncludeClipRect(HDC_SCREEN, &src_rc);

    svgui_header_i = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
    svgui_draw ( svgui_header_i, hdc_secondary, NULL);

    if(is_zoom_in){
        real_dst_height = RECTW(dst_rc)*RECTH(src_rc)/RECTW(src_rc);
        real_src_height = RECTH(src_rc);
    }else{
        real_dst_height = RECTW(dst_rc)*RECTH(src_rc)/RECTW(src_rc);
        real_src_height = RECTH(src_rc);
        //real_src_height = RECTW(src_rc)*RECTH(dst_rc)/RECTW(dst_rc);
    }
    width_inc = (RECTW(dst_rc) - RECTW(src_rc)) / frames;
    height_inc = (real_dst_height - real_src_height) / frames;

    curr_dst_rc.left    = src_rc.left;
    curr_dst_rc.top     = src_rc.top;
    for(int i=0;i<=frames;i++)
    {
        if(!is_zoom_in){
            curr_dst_rc.right   = src_rc.left+RECTW(src_rc)+width_inc*i;
            curr_dst_rc.bottom  = src_rc.top+real_src_height+height_inc*i;
            ExcludeClipRect(HDC_SCREEN, &curr_dst_rc);
#ifdef WITH_BKGND_PIC
            BitBlt(g_dc_bkgnd,src_rc.left,src_rc.top,RECTW(src_rc),RECTH(src_rc), HDC_SCREEN, dst_rc.left, dst_rc.top, 0);
#else
            SetBrushColor(HDC_SCREEN, RGB2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00) );
            FillBox(HDC_SCREEN, dst_rc.left, dst_rc.top, RECTW(src_rc),RECTH(src_rc));
#endif
            IncludeClipRect(HDC_SCREEN, &curr_dst_rc);
        }
        StretchBlt(hdc_secondary,0,0,RECTW(src_rc),RECTH(src_rc),HDC_SCREEN, src_rc.left, src_rc.top, RECTW(src_rc)+width_inc*i, real_src_height+height_inc*i, 0);
        usleep(interval*1000);
        //usleep(1000*1000);
    }
    return 0;
}
int AnimationZoomIn( HWND hWnd, const RECT *src_rc_relative, const RECT *dst_rc,  int interval, int frames )
{
    HDC hdc_secondary;
    RECT last_dst_rc;
    RECT last_dst_rc_weather_wnd;
    RECT last_dst_rc_bkgnd;
    int width_inc;
    int height_inc;
    RECT src_rc;
    int x_inc, y_inc;
    RECT src_wnd_rc;
    RECT last_inc_union_rc;
    RECT next_inc_rc;
    RECT last_inc_remain_rc;

    if(!GetWindowRect(hWnd, &src_wnd_rc))
    {
        fprintf(stderr, "[ERROR] can't get normal window rect\n");
        return -1;
    }
    // get second dc of toolbar or menu's window
    hdc_secondary = GetSecondaryDC(hWnd); // hWnd

    //src_rc = src_rc_relative;
    memcpy(&src_rc, src_rc_relative, sizeof(RECT));
    OffsetRect(&src_rc, src_wnd_rc.left, src_wnd_rc.top );
    memcpy(&last_dst_rc, &src_rc, sizeof(RECT));

    width_inc = (RECTW(*dst_rc) - RECTW(src_rc)) / frames;
    height_inc = (RECTH(*dst_rc) - RECTH(src_rc)) / frames;
    x_inc = ( dst_rc->left - src_rc.left ) /frames;
    y_inc = ( dst_rc->top - src_rc.top ) /frames;

    memcpy(&last_inc_union_rc, &src_rc, sizeof(RECT));
    OffsetRect( &last_inc_union_rc, +(x_inc+width_inc), +(y_inc+height_inc));

    SetBrushColor(HDC_SCREEN, RGB2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00) );
    for(int i=1;i<frames+1;i++)
    {
        // draw foreground
        if(IntersectRect(&last_dst_rc_weather_wnd, &last_dst_rc, &src_wnd_rc))
        {
            IntersectRect(&next_inc_rc, &last_inc_union_rc,  &last_dst_rc_weather_wnd);
            ExcludeClipRect(HDC_SCREEN, &next_inc_rc);
            BitBlt(hdc_secondary, last_dst_rc_weather_wnd.left-src_wnd_rc.left, last_dst_rc_weather_wnd.top-src_wnd_rc.top,RECTW(last_dst_rc_weather_wnd),RECTH(last_dst_rc_weather_wnd), HDC_SCREEN, last_dst_rc_weather_wnd.left, last_dst_rc_weather_wnd.top, 0);
            IncludeClipRect(HDC_SCREEN, &next_inc_rc);
        }

        // draw background
        if( SubtractRect(&last_dst_rc_bkgnd, &last_dst_rc, &src_wnd_rc))
        {
            if(IntersectRect(&last_inc_remain_rc, &last_inc_union_rc, &last_dst_rc_bkgnd))
            {
                ExcludeClipRect(HDC_SCREEN, &last_inc_remain_rc);
#ifdef WITH_BKGND_PIC
                BitBlt(g_dc_bkgnd,last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, RECTW(last_dst_rc_bkgnd), RECTH(last_dst_rc_bkgnd), HDC_SCREEN, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, 0);
#else

                FillBox(HDC_SCREEN, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, RECTW(last_dst_rc_bkgnd), RECTH(last_dst_rc_bkgnd) );
#endif
                IncludeClipRect(HDC_SCREEN, &last_inc_remain_rc);
            }
        }
        if(i < frames)
        {
            StretchBlt( hdc_secondary, src_rc_relative->left, src_rc_relative->top, RECTW(*src_rc_relative),RECTH(*src_rc_relative),
                    HDC_SCREEN, src_rc.left+x_inc*i, src_rc.top+y_inc*i, RECTW(src_rc)+width_inc*i, RECTH(src_rc)+height_inc*i, 0);
            SetRect(&last_dst_rc,src_rc.left+x_inc*i, src_rc.top+y_inc*i, src_rc.left+x_inc*i+RECTW(src_rc)+width_inc*i, src_rc.top+y_inc*i+RECTH(src_rc)+height_inc*i );

            CopyRect(&last_inc_union_rc, &last_dst_rc);
            OffsetRect( &last_inc_union_rc, +(x_inc+width_inc), +(y_inc+height_inc)); 
            usleep(interval*1000);
        }else{
            StretchBlt( hdc_secondary, src_rc_relative->left, src_rc_relative->top, RECTW(*src_rc_relative),RECTH(*src_rc_relative),
                    HDC_SCREEN, dst_rc->left, dst_rc->top, RECTW(*dst_rc), RECTH(*dst_rc), 0);
        }
        //usleep(1000*1000);
    }
    return 0;
}

int AnimationZoomOut( HWND hBigWnd, HWND hNormalWnd, const RECT* dst_rc, int interval, int frames )
{
    HDC hdc_secondary_normal_wnd;
    HDC hdc_secondary_big_wnd;
    RECT src_rc;
    //RECT dst_rc;
    RECT last_dst_rc;
    RECT last_dst_rc_weather_wnd;//current rect bliting(stretch blitting)from secondary dc to HDCSCRENN
    RECT last_remain_rc;
    RECT last_intersect_rc;

    RECT last_dst_rc_bkgnd;//current rect bliting(stretch blitting)from secondary dc to HDCSCRENN
    RECT last_dst_union_rc;
    RECT last_dst_union_remain_rc;

    int width_dec;
    int height_dec;
    int x_dec, y_dec;
    RECT normal_wnd_rc;

    // get second dc of toolbar or menu's window
    hdc_secondary_normal_wnd = GetSecondaryDC(hNormalWnd); // hWnd
    hdc_secondary_big_wnd = GetSecondaryDC(hBigWnd); // hWnd
    if(!hdc_secondary_big_wnd || !hdc_secondary_normal_wnd ){
        fprintf(stderr, "[ERROR] can't window's secondary dc\n");
        return -1;
    }

    if(!GetWindowRect(hBigWnd, &src_rc)){
        fprintf(stderr, "[ERROR] can't get big window rect\n");
        return -1;
    }
    if(!GetWindowRect(hNormalWnd, &normal_wnd_rc)){
        fprintf(stderr, "[ERROR] can't get normal window rect\n");
        return -1;
    }
    memcpy(&last_dst_rc, &src_rc, sizeof(RECT));
    memcpy(&last_intersect_rc, &src_rc, sizeof(RECT));
    memcpy(&last_dst_union_rc, &src_rc, sizeof(RECT));

    width_dec = (RECTW(src_rc) - RECTW(*dst_rc)) / frames;
    height_dec = (RECTH(src_rc) - RECTH(*dst_rc)) / frames;
    x_dec = ( src_rc.left - dst_rc->left ) /frames;
    y_dec = ( src_rc.top - dst_rc->top ) /frames;

    OffsetRect(&last_dst_union_rc, -(x_dec+width_dec), -(y_dec+height_dec));

    SetBrushColor(HDC_SCREEN, RGB2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00));
    for(int i=0;i<frames;i++)
    {
        // draw foreground
        if(IntersectRect(&last_dst_rc_weather_wnd, &last_dst_rc, &normal_wnd_rc))
        {
            SubtractRect(&last_remain_rc, &last_dst_rc_weather_wnd, &last_dst_union_rc );
            BitBlt(hdc_secondary_normal_wnd, last_remain_rc.left-normal_wnd_rc.left, last_remain_rc.top-normal_wnd_rc.top,RECTW(last_remain_rc),RECTH(last_remain_rc), HDC_SCREEN, last_remain_rc.left, last_remain_rc.top, 0);
        }

        // draw background
        if( SubtractRect(&last_dst_rc_bkgnd,  &last_dst_rc, &normal_wnd_rc))
        {
            if(IntersectRect(&last_dst_union_remain_rc, &last_dst_rc_bkgnd, &last_dst_union_rc))
            {
                ExcludeClipRect(HDC_SCREEN, &last_dst_union_remain_rc);
#ifdef WITH_BKGND_PIC
                BitBlt(g_dc_bkgnd, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, RECTW(last_dst_rc_bkgnd), RECTH(last_dst_rc_bkgnd), HDC_SCREEN, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, 0);
#else
                FillBox(HDC_SCREEN, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, RECTW(last_dst_rc_bkgnd), RECTH(last_dst_rc_bkgnd));
#endif
                IncludeClipRect(HDC_SCREEN, &last_dst_union_remain_rc);
            }
        }

        if( i < frames-1)
        {
            StretchBlt( hdc_secondary_big_wnd, 0, 0, RECTW(src_rc),RECTH(src_rc),
                    HDC_SCREEN, src_rc.left-x_dec*i, src_rc.top-y_dec*i, RECTW(src_rc)-width_dec*i, RECTH(src_rc)-height_dec*i, -1);
            SetRect(&last_dst_rc, src_rc.left-x_dec*i, src_rc.top-y_dec*i, src_rc.left-x_dec*i+RECTW(src_rc)-width_dec*i, src_rc.top-y_dec*i+RECTH(src_rc)-height_dec*i);
            CopyRect(&last_dst_union_rc, &last_dst_rc);
            OffsetRect( &last_dst_union_rc, -(x_dec+width_dec), -(y_dec+height_dec)); 
            usleep(interval*1000);
        }

        if(i == frames-1)
        {
            if( SubtractRect(&last_dst_rc_bkgnd,  &last_dst_rc, &normal_wnd_rc))
            {
                FillBox(HDC_SCREEN, last_dst_rc_bkgnd.left, last_dst_rc_bkgnd.top, RECTW(last_dst_rc_bkgnd), RECTH(last_dst_rc_bkgnd));
            }
            if(IntersectRect(&last_dst_rc_weather_wnd, &last_dst_rc, &normal_wnd_rc))
            {
                BitBlt(hdc_secondary_normal_wnd, last_dst_rc_weather_wnd.left-normal_wnd_rc.left, last_dst_rc_weather_wnd.top-normal_wnd_rc.top,RECTW(last_dst_rc_weather_wnd),RECTH(last_dst_rc_weather_wnd), HDC_SCREEN, last_dst_rc_weather_wnd.left, last_dst_rc_weather_wnd.top, 0);
            }
        }
        //usleep(1000*1000);
    }
    return 0;
}
