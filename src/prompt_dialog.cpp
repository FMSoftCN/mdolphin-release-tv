
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>


#define IDC_INPUT	1000

extern int g_win_width;
extern int g_win_height;

static char *inputboxbuf;
static int inputboxbuflen;
static int InputBoxProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    if ( message == MSG_COMMAND ) {
        if ( wParam == IDOK ) {
            GetWindowText (GetDlgItem (hDlg, IDC_INPUT), inputboxbuf, inputboxbuflen);
            EndDialog (hDlg, 1);
        }else if (wParam == IDCANCEL){
            EndDialog (hDlg, 0);
        } 
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}


int prompt_box(HWND parent, const char *info, const char *definp, char *buf, int buflen)
{
    int x, y, box_w, box_h;
    x = g_win_width;
    y = g_win_height;

    if (g_win_width >= 1024) {
        box_w = 320;
        box_h = 200;
    }
    else if(g_win_width <= 800) {
        box_w = 280;
        box_h = 160;
    }
    else {
        box_w = 300;
        box_h = 180;
    }

	static DLGTEMPLATE DlgBoxInputLen =
	{
    	WS_BORDER | WS_CAPTION, 
	    WS_EX_NONE,
    	(g_win_width-box_w)/2, (g_win_height-box_h)/2, box_w, box_h, 
		"Javascript",
	    0, 0,
    	4, NULL,
	    0
	};

	static CTRLDATA CtrlInputLen [] =
	{ 
    	{
        	CTRL_STATIC,
	        WS_VISIBLE | SS_SIMPLE,
    	    20, 10, box_w-40-20, box_h/6, 
        	IDC_STATIC, 
	        "InputBox",
    	    0
	    },
    	{
	        CTRL_EDIT,
    	    WS_VISIBLE | WS_TABSTOP | WS_BORDER,
        	20, box_h/6+20, box_w-40-20, box_h/6,
	        IDC_INPUT,
    	    NULL,
	        0
    	},
	    {
    	    CTRL_BUTTON,
        	WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
	        40, box_h/6*2+30, (box_w-120)/2, box_h/6,
    	    IDOK, 
        	"Ok",
	        0
    	},
	    {
    	    CTRL_BUTTON,
        	WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
	        (box_w-120)/2+80, box_h/6*2+30, (box_w-120)/2, box_h/6,
    	    IDCANCEL, 
        	"Cancel",
	        0
    	}
	};

	CtrlInputLen[0].caption = info;
	CtrlInputLen[1].caption = definp;
    DlgBoxInputLen.controls = CtrlInputLen;
	inputboxbuf = buf;
	inputboxbuflen = buflen;

    return DialogBoxIndirectParam (&DlgBoxInputLen, parent, InputBoxProc, 0);
}

