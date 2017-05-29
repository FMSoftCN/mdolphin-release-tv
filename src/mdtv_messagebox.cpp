#include <unistd.h>
#include <string.h>
#include "svgui.h"

extern HWND g_mdolphin_main_hwnd;
extern float scale_x;
extern float scale_y;
extern HDC g_dc_uisec;

#define SMB_W 260
#define SMB_H 200

#define SMB_CAP_H 40
#define SMB_BOT_H 60
#define SMB_BOD_H 160
#define SMB_OK_W 60 
#define SMB_OK_H 40 
#define SMB_OK_L 20 
#define SMB_OK_R (SMB_OK_L+SMB_OK_W)
#define SMB_1OK_T 140 
#define SMB_1OK_B (SMB_1OK_T+ SMB_OK_H)
#define SMB_CRO_L 220
#define SMB_CRO_T 8
#define SMB_CRO_R 242
#define SMB_CRO_B 32

enum {
   tab_ok=0,
   tab_cancel,
};

static SVGUI_IMAGE_T images_block1 [] = 
{
    {16, 16, 16, "messagebox/cross-16X16.png"},
    {22, 22, 16, "messagebox/cross-22X22.png"},
    {24, 24, 16, "messagebox/cross-24X24.png"},
};

static SVGUI_IMAGE_AREA_T image_areas_block1 [] =
{
    {1, TRUE, {SMB_CRO_L, SMB_CRO_T,\
        242, 32}, SVGUI_IMAGE_FILL_WAY_SCALED, 
        TABLESIZE(images_block1), images_block1},
};

static SVGUI_TEXT_AREA_T c_text_areas_block3 [] =
{
    {0, TRUE, {0, 0, SMB_OK_W, SMB_OK_H},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,0x0, 1, "cancel"},
};

static SVGUI_TEXT_AREA_T c_text_areas_block2 [] =
{
    {0, TRUE, {0, 0, SMB_OK_W, SMB_OK_H},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,0x0, 1, "ok"},
};

static SVGUI_TEXT_AREA_T text_areas_block2 [] =
{
    {0, TRUE, {0, 0, SMB_OK_W, SMB_OK_H},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,0x0, 1, "ok"},
};

static SVGUI_TEXT_AREA_T text_areas_block1 [] =
{
    {0, TRUE, {0, 0, SMB_W , SMB_BOT_H},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,0x0, 1, "text"},
};

static SVGUI_TEXT_AREA_T text_areas_block0 [] =
{
    {0, TRUE, {0, 0, SMB_W-40, SMB_CAP_H},\
        SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,0x0, 1, "caption"},
};

static SVGUI_PRDRD_BLOCK_T prdrd_blocks [] =
{

    {SMB_W, SMB_H-SMB_CAP_H,\
        SVGUI_PRDRD_GRADIENT_VERT, 0, 0, 0,\
        0xeeeff0, 0xeeeff0, 0xeeeff0, 0, 0x0, 0, 0x035cb0},
    {SMB_W, SMB_CAP_H, SVGUI_PRDRD_GRADIENT_VERT,\
        0, 10, 30, 0xb4d4f1, 0x7bb2e6, 0x035cb0, 2, 0x696969, 4, 0xffffff},
    {SMB_W, SMB_BOD_H+2, SVGUI_PRDRD_GRADIENT_HORZ,\
        0, 25, 50, 0xeeeff0, 0xeeeff0, 0xeeeff0, 2, 0x696969, 0, 0xffffff},
    {SMB_OK_W, SMB_OK_H, SVGUI_PRDRD_GRADIENT_VERT,\
        0, 10, 10, 0xe4e4e4, 0xe4e4e4, 0xb6b6b6, 2, 0x696969, 4, 0xffffff},
    {SMB_W, 4, SVGUI_PRDRD_GRADIENT_VERT,\
        0, 10, 10, 0x035cb0, 0x035cb0, 0x035cb0, 0, 0x696969, 0, 0xffffff},
    {SMB_OK_W, SMB_OK_H, SVGUI_PRDRD_GRADIENT_VERT,\
        0, 10, 10, 0xc0c0c0, 0xc0c0c0, 0xa9a9a9, 2, 0x696969, 4, 0xffffff},

};

static SVGUI_BLOCK_T blocks [] =
{
      {0, TRUE, {0, 0, SMB_W, SMB_CAP_H}, TRUE, 1, TABLESIZE(text_areas_block0), text_areas_block0, TABLESIZE(image_areas_block1), image_areas_block1},
      {1, TRUE, {0, SMB_CAP_H-2, SMB_W, SMB_H}, TRUE, 2, TABLESIZE(text_areas_block1), text_areas_block1, 0, NULL},
      {2, TRUE, {(SMB_W-SMB_OK_W)/2, SMB_H-70, \
          (SMB_W-SMB_OK_W)/2+SMB_OK_W,  \
          SMB_H-30}, TRUE, 5, TABLESIZE(text_areas_block2),\
          text_areas_block2, 0, NULL},
      {3, TRUE, {0, SMB_CAP_H-2,SMB_W, SMB_CAP_H+2}, TRUE, 4, 0, NULL, 0, NULL},
};

static SVGUI_BLOCK_T blocks4 [] =
{
      {0, TRUE, {0, 0, SMB_W, SMB_CAP_H}, TRUE, 1, TABLESIZE(text_areas_block0),text_areas_block0, TABLESIZE(image_areas_block1), image_areas_block1},
      {1, TRUE, {0, SMB_CAP_H, SMB_W, SMB_H}, TRUE, 2, TABLESIZE(text_areas_block1),text_areas_block1, 0, NULL},
      {2, TRUE, {(SMB_W-SMB_OK_W*2)/3, SMB_H-70,\
          (SMB_W-SMB_OK_W*2)/3+SMB_OK_W, SMB_H-20}, TRUE, 5, TABLESIZE(c_text_areas_block2),c_text_areas_block2, 0, NULL},
      {3, TRUE, {(SMB_W-SMB_OK_W*2)/3*2+\
          SMB_OK_W, SMB_H-70, \
          (SMB_W-SMB_OK_W*2)/3*2+\
          SMB_OK_W,SMB_H-30}, TRUE, 3,\
          TABLESIZE(c_text_areas_block3), c_text_areas_block3, 0, NULL},
      {4, TRUE, {0, SMB_CAP_H-2,SMB_W, SMB_CAP_H+2}, TRUE, 4, 0, NULL, 0, NULL},

};

static SVGUI_FONT_INFO_T font_infos [] =
{
    {"fmsong", 16, "UTF-8"},
    {"fmhei", 16, "UTF-8"}
};

static SVGUI_HEADER_T header_message =
{
    SMB_W, SMB_H,
    0xffffff, 0xffffff,
    TABLESIZE(prdrd_blocks), prdrd_blocks,
    TABLESIZE(font_infos), font_infos,
    TABLESIZE(blocks), blocks,
};

static int  CreateCallbackWnd(WNDPROC hWndProc,const SVGUI_HEADER_T *svgui_header_t,\
        HWND hHostingWnd, const char * text, const char * caption)
{
    MAINWINCREATE CreateInfo;
    int ret = IDCANCEL;
    MSG Msg;
    RECT rect;

    rect.left = (RECTW(g_rcScr)-SMB_W)/2;
    rect.top = (RECTH(g_rcScr)-SMB_H)/2;
    rect.right = ((RECTW(g_rcScr)-SMB_W)/2 + SMB_W);
    rect.bottom = ((RECTW(g_rcScr)-SMB_H)/2 + SMB_H);

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST ;
    CreateInfo.spCaption = caption;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = hWndProc;
    CreateInfo.lx = rect.left;
    CreateInfo.ty = rect.top;
    CreateInfo.rx = rect.right;
    CreateInfo.by = rect.bottom;

    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)svgui_header_t;

    CreateInfo.hHosting = hHostingWnd;

    HWND hDlg = CreateMainWindow (&CreateInfo);

    SetWindowAdditionalData2 (hDlg, (DWORD)(&ret));

    if (hHostingWnd && hHostingWnd != HWND_DESKTOP) {
        if (IsWindowEnabled (hHostingWnd)) {
            EnableWindow (hHostingWnd, FALSE);
        }
        while (PeekPostMessage (&Msg, hHostingWnd,
                                MSG_KEYDOWN, MSG_KEYUP, PM_REMOVE));
    }

    while (GetMessage (&Msg, hDlg)) {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
    }

    if (hHostingWnd != HWND_DESKTOP && hHostingWnd != HWND_NULL) {
        EnableWindow (hHostingWnd, TRUE);
        SetActiveWindow (hHostingWnd);
        InvalidateRect(hHostingWnd, NULL, TRUE);
    }

    return ret;
}


static void process_tab(HWND hWnd, int tab_count)
{ 
    DWORD dwAddData1;
    DWORD dwAddData2;
    RECT dst_rc = {0, 0, SMB_W, SMB_H};

    dwAddData1 = GetWindowAdditionalData (hWnd);
    dwAddData2 = GetWindowAdditionalData2 (hWnd);

    if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 5)
    {
        (*(SVGUI_HEADER_I*)dwAddData1).blocks[2].idx_prdrd_block=5;
    }
    else if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 6)
    {
        if(tab_count == tab_ok) {
            (*(SVGUI_HEADER_I*)dwAddData1).blocks[2].idx_prdrd_block=5;
            (*(SVGUI_HEADER_I*)dwAddData1).blocks[3].idx_prdrd_block=3;
        }
        else if(tab_count == tab_cancel){
            (*(SVGUI_HEADER_I*)dwAddData1).blocks[2].idx_prdrd_block=3;
            (*(SVGUI_HEADER_I*)dwAddData1).blocks[3].idx_prdrd_block=5;
        }
    }
    InvalidateRect(hWnd, &dst_rc, TRUE);
}

static void process_enter(HWND hWnd, int tab_count)
{ 
    DWORD dwAddData1;
    DWORD dwAddData2;

    dwAddData1 = GetWindowAdditionalData (hWnd);
    dwAddData2 = GetWindowAdditionalData2 (hWnd);

    if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 5)
    {
        SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[2]\
                                 .text_areas;
        if(strcmp(tmp->text, "ok") == 0){
            *(int*)dwAddData2 = IDOK;
        }
    }
    else if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 6)
    {
        if(tab_count == tab_ok) {
            (*(SVGUI_HEADER_I*)dwAddData1).blocks[2].idx_prdrd_block=5;
            SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[2]\
                                     .text_areas;
            if(strcmp(tmp->text, "ok") == 0){
                *(int*)dwAddData2 = IDOK;
            }
        }
        else if(tab_count == tab_cancel){
                (*(SVGUI_HEADER_I*)dwAddData1).blocks[3].idx_prdrd_block=5;
                SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[3]\
                                         .text_areas;
                if(strcmp(tmp->text, "cancel") == 0){
                    *(int*)dwAddData2 = IDCANCEL;
                }
        }
    }

    DestroyMainWindow (hWnd);
}

static void process_key_btn(HWND hWnd, int x, int y)
{ 
    DWORD dwAddData1;
    DWORD dwAddData2;

    dwAddData1 = GetWindowAdditionalData (hWnd);
    dwAddData2 = GetWindowAdditionalData2 (hWnd);

    if( x>=SMB_CRO_L && x<= SMB_CRO_R){
        if( y>= SMB_CRO_T && y<= SMB_CRO_B) {
            DestroyMainWindow (hWnd);
        }
    }

    if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 5)
    {
        if( x>=(SMB_W-SMB_OK_W)/2 && x<=(SMB_W-SMB_OK_W)/2\
                +SMB_OK_W)
            if(y >= SMB_H-70 && y <= SMB_H-30){
                SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[2]\
                                         .text_areas;
                if(strcmp(tmp->text, "ok") == 0){
                    *(int*)dwAddData2 = IDOK;
                }
                DestroyMainWindow (hWnd);
            }
    }
    else if((*(SVGUI_HEADER_I*)dwAddData1).nr_blocks < 6)
    {
        if( x>=(SMB_W-SMB_OK_W*2)/3 &&\
                x <= (SMB_W-SMB_OK_W*2)/3+SMB_OK_W){
            if(y >= SMB_H-70 && y <= SMB_H-30){
                SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[2]\
                                         .text_areas;
                if(strcmp(tmp->text, "ok") == 0){
                    *(int*)dwAddData2 = IDOK;
                }
                DestroyMainWindow (hWnd);
            }
        }
        if( x>=(SMB_W-SMB_OK_W*2)/3*2+SMB_OK_W &&\
                x<=(SMB_W-SMB_OK_W*2)/3*2+SMB_OK_W*2){
            if(y >= SMB_H-70  && y <= SMB_H-30){
                SVGUI_TEXT_AREA_I *tmp = (*(SVGUI_HEADER_I*)dwAddData1).blocks[3]\
                                         .text_areas;
                if(strcmp(tmp->text, "cancel") == 0){
                    *(int*)dwAddData2 = IDCANCEL;
                }
                DestroyMainWindow (hWnd);
            }
        }
    }

}

static int message_proc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static int tab_status = tab_ok;

    switch (message) {
        case MSG_CREATE:
            break;

        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_ESCAPE:
                    DestroyMainWindow (hWnd);
                    break;
                case SCANCODE_ENTER:
                {
                    process_enter(hWnd, tab_status);
                    break;
                }
                case SCANCODE_TAB:
                {
                    tab_status++;
                    if(tab_status > tab_cancel)
                        tab_status = tab_ok;
                    process_tab(hWnd, tab_status);
                    break;
                }
            }
            break;
        case MSG_LBUTTONDOWN:
        {
            int x = LOSWORD (lParam);
            int y = HISWORD (lParam);

            process_key_btn(hWnd, x, y);

            break;
        }
        case MSG_PAINT:
            break;

        case MSG_CLOSE:
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}

int svg_messagebox (HWND parent, const char * text, const char * caption, DWORD dwStyle)
{
      int ret;
      if ((dwStyle & 0x7) == MB_OK) {
        header_message.nr_blocks = 4; 
        header_message.blocks = blocks;
        SVGUI_TEXT_AREA_T *tmp = header_message.blocks[0].text_areas;
        tmp->text = caption;
        tmp = header_message.blocks[1].text_areas;
//        tmp->text = text;
        tmp->text = "中中中中中中中中中中中中中中中中中中";
        tmp = header_message.blocks[2].text_areas;
        tmp->text = "ok";
        ret =  CreateCallbackWnd(message_proc, &header_message, g_mdolphin_main_hwnd,\
                text, caption);
      }    
      else if((dwStyle & 0x7) == MB_OKCANCEL){
        header_message.nr_blocks = 5; 
        header_message.blocks = blocks4;
        SVGUI_TEXT_AREA_T *tmp = header_message.blocks[0].text_areas;
        tmp->text = caption;
        tmp = header_message.blocks[1].text_areas;
        tmp->text = text;
        tmp = header_message.blocks[2].text_areas;
        tmp->text = "ok";
        tmp = header_message.blocks[3].text_areas;
        tmp->text = "cancel";

        ret = CreateCallbackWnd(message_proc, &header_message, HDC_SCREEN, text, caption);
      }    

      return ret;
}

