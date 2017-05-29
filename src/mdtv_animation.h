#ifndef MDTV_ANIMATION_H
#define MDTV_ANIMATION_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#define ANIMATION_INTERVAL  (50)//ms
#define ANIMATION_FRAMES    (5)
//#define ANIMATION_FRAMES    (8)

typedef enum _AnimationMoveDirrect{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
}AnimationMoveDirrect;
// for toolbar animation
void AnimationMoveWndUpDown ( HWND hWnd, RECT dst_effrc, int direct,  int interval, int frames );

// display animation for level 2'menu or web page
void AnimationMoveWndLeftRight ( HWND hWnd, RECT dst_effrc, int direct,  int interval, int frames );

int AnimationZoomOutIn( HWND hWnd, RECT src_rc, RECT dst_rc, bool is_zoom_in , int interval, int frames );
int AnimationZoomIn( HWND hWnd, const RECT *src_rc_relative, const RECT *dst_rc,  int interval, int frames );
int AnimationZoomOut( HWND hBigWnd, HWND hNormalWnd, const RECT *dst_rc, int interval, int frames );
#endif
