#ifndef  _MDTV_IME_H
#define _MDTV_IME_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

// use SCANCODE_KEYPAD1, not SCANCODE_1
#define USE_PAD_KEY 0

#ifdef _MGRM_PROCESSES

#define PHONE_REQID (MAX_SYS_REQID + 1)
#define PHONE_GETSTATE 201
BOOL mdtv_GetImeStatus(void);
BOOL mdtv_ShowIme (BOOL show, int method);

#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
//method: 0-close ime , 1-display ime, 2-switch input method, 3-get ime status
// 4-set hwnd
enum{
    IME_CMD_CLOSE,
    IME_CMD_OPEN,
    IME_CMD_SWITCH_INPUT_METHOD,
    IME_CMD_GET_STATUS,
    IME_CMD_SET_TARGET,
    IME_CMD_HIDE,
    IME_CMD_MAX,
};
#define MSG_IME_CMD_OPEN    (MSG_USER + 1)
#define MSG_IME_CMD_CLOSE   (MSG_USER + 2)
#define MSG_IME_CMD_KEYDOWN (MSG_USER + 3)
#define MSG_IME_CMD_SETPOS  (MSG_USER + 4)

//BOOL mdtv_ShowIme (BOOL show, int method);
int mdtv_ShowIme(int method, HWND hwnd);
int mdtv_ime_switch_method();
int mdtv_send_msg_to_ime(int message, WPARAM wParam, LPARAM lParam );
int mdtv_ShowSoftIme(BOOL show);
BOOL mdtv_GetImeStatus(void);
#endif  /* endif _MGRM_PROCESSES */

HWND mdtv_InitPhoneIme (RECT *caret_rect);
void mdtv_DestroyPhoneIme (HWND hwnd);
int mdtv_NotifyCursorPos (RECT *rc);
void mdtv_SetPhoneImeRect (RECT *rc);
BOOL mdtv_CreateImePinying();
HWND mdtv_GetPhoneImeWnd();

#endif     /* -----  not _MDTV_IME_H  ----- */
