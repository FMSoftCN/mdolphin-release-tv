#include	"mdtv_ime.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_browser.h"
#include "mdtv_symbol.h"

#include "svgui.h"

//#include <mgi/mgi-phone.h>
#include <mgi/ime_pinyin.h>
//#include <mgi/mgi.h>
#include "mgi-phone.h"

#undef MDTV_IME_DEBUG

HWND sg_imehMainWnd;
extern HWND g_hMainWnd ;
HWND sg_phone_ime_hwnd;
static RECT sg_phone_ime_rect = {0} ;

#ifdef _MGRM_PROCESSES
BOOL mdtv_GetImeStatus(void)
{
    REQUEST req;
    int data = PHONE_GETSTATE; 
    int reply;

    req.id = PHONE_REQID;
    req.data = &data;
    req.len_data = sizeof (data);

    ClientRequest (&req, &reply, sizeof (reply));

    return reply;
}

static int ime_map[] = {0, 3, 101};
static int ime_cur = 0;
BOOL mdtv_ShowIme(BOOL show, int method)
{
    int ret, msg;
    REQUEST req;
    req.id =PHONE_REQID;
    if (show) { 
        SendMessage(g_mdolphin_hwnd, MSG_SETFOCUS, 0, 0); 
        if (method != 0)
            ime_cur = method;
        else
        {
            ime_cur++;
            if (ime_cur == sizeof(ime_map)/sizeof(ime_map[0]))
                ime_cur = 0;
        }
        msg = ime_map[ime_cur];
    }
    else
    {
        ime_cur = 0;
        msg = 0;
    }

    req.data =&msg;
    req.len_data = sizeof(int);
    ClientRequest(&req, &ret, sizeof(int));

    return TRUE;
}

#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)

BOOL mdtv_GetImeStatus(void)
{
    int ret;
    if (!sg_phone_ime_hwnd)
        return -1;
    ret = SendMessage(sg_phone_ime_hwnd, MSG_IME_GETSTATUS, 0, 0);
    return ret;
}

int mdtv_ShowIme(int method, HWND hwnd)
{
    int ret = -1;
    if (!sg_phone_ime_hwnd)
        return 0;

    switch (method)
    {
        case IME_CMD_CLOSE:
            ret = PostMessage(sg_phone_ime_hwnd, MSG_IME_CMD_CLOSE, 0, 0);
            break;
        case IME_CMD_OPEN:
            ret = PostMessage(sg_phone_ime_hwnd, MSG_IME_CMD_OPEN, 0, 0);
            break;
        case IME_CMD_SWITCH_INPUT_METHOD:
            mgiPhoneKeyPadSetCurrMethod(101);
            ret = PostMessage(sg_phone_ime_hwnd, MSG_IME_CMD_OPEN, 0, 101);
            break;
        case IME_CMD_GET_STATUS:
            ret = PostSyncMessage (sg_phone_ime_hwnd, MSG_IME_GETSTATUS, 0, 0);
            break;

        case IME_CMD_SET_TARGET:
            ret = PostMessage(sg_phone_ime_hwnd, MSG_IME_SETTARGET, (WPARAM)hwnd, 0);
            break;
    }
    return ret;
}
#endif  /* endif _MGRM_PROCESSES */

int mdtv_ShowSoftIme(BOOL show)
{
    if (!sg_imehMainWnd)
        return -1;
    if (show)
        ShowWindow (sg_imehMainWnd, SW_SHOWNORMAL);
    else
        ShowWindow (sg_imehMainWnd, SW_HIDE);
    return 0;
}
extern const MGI_PHONE_KEY_MAP default_map[] = 
{
    {SCANCODE_CURSORBLOCKLEFT, ""},
    {SCANCODE_CURSORBLOCKRIGHT, ""},
    {SCANCODE_CURSORBLOCKUP, ""},
    {SCANCODE_CURSORBLOCKDOWN, ""},
    {SCANCODE_BACKSPACE, ""},
    {SCANCODE_ENTER, ""},
    {SCANCODE_F5, ""}, // MGI_PHONE_KEY_QUIT
#if USE_PAD_KEY
    {SCANCODE_KEYPADMULTIPLY, ""},// switch symbol input
    {SCANCODE_KEYPADDIVIDE, ""},// switch ime method
    {SCANCODE_KEYPAD0, " 0"},
    {SCANCODE_KEYPAD1, "./:,@;!?\"'-#()_+&%*=<>$[]{}\\1"},
    {SCANCODE_KEYPAD2, "abc2"},
    {SCANCODE_KEYPAD3, "def3"},
    {SCANCODE_KEYPAD4, "ghi4"},
    {SCANCODE_KEYPAD5, "jkl5"},
    {SCANCODE_KEYPAD6, "mno6"},
    {SCANCODE_KEYPAD7, "pqrs7"},
    {SCANCODE_KEYPAD8, "tuv8"},
    {SCANCODE_KEYPAD9, "wxyz9"},
#else
    {SCANCODE_MINUS, ""},// switch symbol input
    {SCANCODE_EQUAL, ""},// switch ime method
    {SCANCODE_0, " 0"},
    {SCANCODE_1, "./:,@;!?\"'-#()_+&%*=<>$[]{}\\1"},
    {SCANCODE_2, "abc2"},
    {SCANCODE_3, "def3"},
    {SCANCODE_4, "ghi4"},
    {SCANCODE_5, "jkl5"},
    {SCANCODE_6, "mno6"},
    {SCANCODE_7, "pqrs7"},
    {SCANCODE_8, "tuv8"},
    {SCANCODE_9, "wxyz9"},
#endif
};

int pinyin_get_symbol(void *method_data, char *buff, int buff_len)
{
    //char ch_punctuate[] = "， 。 、 ： ； ！ · ？ ＃ ￥ ％ ℃ ～ × （ ） 〔 〕 《 》 〈 〉 【 】 〖 〗 『";
    //char ch_punctuate[] = "， 。 、 ： ； ！ · ？ ＃ ￥ ％ ℃ ～ × （ ） 〔 〕 《 》 〈 〉 【 】 〖 〗";
    if (buff == NULL || buff_len <= 0)
        return -1;
    if (strlen(ch_punctuate) < (size_t)buff_len)
        strcpy(buff, ch_punctuate);
    else {
        strncpy(buff, ch_punctuate, buff_len);
        buff[buff_len+1] = '\0';
    }
    return strlen(buff);
}


MGI_PHONE_IME_METHOD pinyin_method =
{
    101,
    "pinyin",
    NULL,
    pinyin_match_keystokes,
    pinyin_translate_word,
    pinyin_predict_pord,
    NULL,
    NULL,
    pinyin_get_symbol,
    NULL,
    NULL
};

static const char *sg_ime_method_hint[]  =
{
    " 123",
    "  EN",
    "  en",
    "pinyin"
};

static  int  sg_ime_method_hint_index = 0;
static const char *get_ime_hint_text ()
{
    if (sg_ime_method_hint_index > 3 || sg_ime_method_hint_index < 0)
        return NULL;

    return sg_ime_method_hint[sg_ime_method_hint_index];
}
int mdtv_method_switch_notify(void *method_data, int new_method_id)
{
    sg_ime_method_hint_index = new_method_id - 1;
    if (sg_ime_method_hint_index == 100) sg_ime_method_hint_index = 3;
    mdtv_set_browser_title_ime_status(g_mdolphin_main_hwnd, get_ime_hint_text ());
   return 0;
}

BOOL mdtv_CreateImePinying()
{
    RECT rect;

    SetRect(&rect, sg_phone_ime_rect.left, sg_phone_ime_rect.bottom+37, sg_phone_ime_rect.left+390, sg_phone_ime_rect.bottom+37+64);
    MGI_PHONE_IME_TEXTCOLOR *textcolor = (MGI_PHONE_IME_TEXTCOLOR *) malloc (sizeof (MGI_PHONE_IME_TEXTCOLOR));
    textcolor->firstline_text_color = MakeRGB (0xED, 0xED, 0xED);
    textcolor->firstline_focus_text_color = MakeRGB (0xFF, 0xD8, 0x51);  //0xffd851;
    textcolor->firstline_focus_color = MakeRGB (0xED, 0xED, 0xED);
    textcolor->secondline_text_color = MakeRGB (0xED, 0xED, 0xED);
    textcolor->secondline_focus_text_color = MakeRGB (0xFF, 0xD8, 0x51);
    textcolor->secondline_focus_color = MakeRGB (0xED, 0xED, 0xED);
    //sg_ime_handle = mgiCreateIMEContainer (2, FALSE); 
    ime_pinyin_init();
    if( ( sg_phone_ime_hwnd = mgiCreatePhoneKeyPadIME(&rect, NULL, NULL, textcolor, 
                    &default_map, mdtv_method_switch_notify) ) == HWND_INVALID )
    {
        fprintf(stderr, "Can not create phone IME. \n");
        return FALSE;
    }

    mgiPhoneKeyPadAddMethod(sg_phone_ime_hwnd, &pinyin_method);

    mdtv_set_browser_title_ime_status(g_mdolphin_main_hwnd, get_ime_hint_text ());
    //mgiAddIMEWindow (sg_ime_handle, sg_phone_ime_hwnd, "mgphone");
    //mgiSetActiveIMEWindow (sg_ime_handle, "mgphone");
    return TRUE;
}


const char *ime_key_num[] = 
{
    "1", "2", "3", NULL,  
    "4", "5", "6", NULL,  
    "7", "8", "9", NULL,  
    " 123", "0", "Enter", " Quit"
};

const char *ime_key_char[] = 
{
    ".,", "abc", "def", NULL,  
    "ghi", "jkl", "mno", NULL,  
    "pqrs", "tuv", "wxyz", NULL,  
    NULL, "0", NULL, NULL
};

#define IME_TEMPLATE_DEFAULT_WIDTH   268
#define IME_TEMPLATE_DEFAULT_HEIGHT   164
#define IME_TEMPLATE_DEFAULT_ROWS 4
#define IME_TEMPLATE_DEFAULT_COLS 4

#define IME_TEMPLATE_DEFAULT_CELL_WIDTH (IME_TEMPLATE_DEFAULT_WIDTH/IME_TEMPLATE_DEFAULT_COLS)
#define IME_TEMPLATE_DEFAULT_CELL_HEIGHT (IME_TEMPLATE_DEFAULT_HEIGHT/IME_TEMPLATE_DEFAULT_COLS)

#define IME_TEMPLATE_TEXT_LEFT_OFFSET_1 7 
#define IME_TEMPLATE_TEXT_TOP_OFFSET_1 6
#define IME_TEMPLATE_TEXT_WIDTH_1 50
#define IME_TEMPLATE_TEXT_HEIGHT_1 25


#define IME_TEMPLATE_TEXT_LEFT_OFFSET_2 22
#define IME_TEMPLATE_TEXT_TOP_OFFSET_2 13
#define IME_TEMPLATE_TEXT_WIDTH_2 40
#define IME_TEMPLATE_TEXT_HEIGHT_2 20


#define IME_TEMPLATE_TEXT_RECT_1 { IME_TEMPLATE_TEXT_LEFT_OFFSET_1, IME_TEMPLATE_TEXT_TOP_OFFSET_1, \
                                 IME_TEMPLATE_TEXT_LEFT_OFFSET_1 + IME_TEMPLATE_TEXT_WIDTH_1, \
                                 IME_TEMPLATE_TEXT_TOP_OFFSET_1 + IME_TEMPLATE_TEXT_HEIGHT_1, \
      }

#define IME_TEMPLATE_TEXT_RECT_2 {IME_TEMPLATE_TEXT_LEFT_OFFSET_2, IME_TEMPLATE_TEXT_TOP_OFFSET_2, \
                                 IME_TEMPLATE_TEXT_LEFT_OFFSET_2 + IME_TEMPLATE_TEXT_WIDTH_2, \
                                 IME_TEMPLATE_TEXT_TOP_OFFSET_2 + IME_TEMPLATE_TEXT_HEIGHT_2, \
      }

#define  IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(i)  \
static SVGUI_TEXT_AREA_T text_areas_block##i [] = \
{ \
    {0, TRUE, IME_TEMPLATE_TEXT_RECT_1, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 0x000000, 0, ime_key_num[i-1]},  \
    {1, TRUE, IME_TEMPLATE_TEXT_RECT_2, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0x000000, 0, ime_key_char[i-1]}, \
};

#define IME_FOR_EACH_TEXT_AREA (macro) \
    macro(1) \
    macro(2) \
    macro(3) \
    macro(4) 


/* define SVGUI_TEXT_AREA_T text_areas_block##i [] */
// IME_FOR_EACH_TEXT_AREA (IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK) 
#if 1
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(1)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(2)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(3)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(4)

IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(5)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(6)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(7)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(8)

IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(9)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(10)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(11)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(12)

IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(13)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(14)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(15)
IME_TEMPLATE_NUM_TEXT_AREAS_BLOCK(16)
#endif

#if 0
static SVGUI_TEXT_AREA_T text_areas_block1 [] =
{
    {1, TRUE, {10, 5, 20, 25}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 0, "1"},
    {1, TRUE, {30, 7, 60, 21}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 1, "1"},
};

static SVGUI_TEXT_AREA_T text_areas_block2 [] =
{
    {1, TRUE, {10, 5, 20, 25}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 0, "2"},
    {1, TRUE, {30, 7, 60, 21}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 1, "abc"},
};

static SVGUI_TEXT_AREA_T text_areas_block3 [] =
{
    {1, TRUE, {10, 5, 20, 25}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 0, "3"},
    {1, TRUE, {30, 7, 60, 21}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 1, "def"},
};

static SVGUI_TEXT_AREA_T text_areas_block4 [] =
{
    {1, TRUE, {10, 5, 20, 25}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 0, "4"},
    {1, TRUE, {30, 7, 60, 21}, SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER, 0xFF00FF, 1, "ghi"},
};
#endif

static SVGUI_PRDRD_BLOCK_T prdrd_blocks [] = 
{
    {IME_TEMPLATE_DEFAULT_CELL_WIDTH, IME_TEMPLATE_DEFAULT_CELL_HEIGHT, SVGUI_PRDRD_GRADIENT_VERT, 0,\
                IME_TEMPLATE_DEFAULT_CELL_HEIGHT/2, IME_TEMPLATE_DEFAULT_CELL_HEIGHT, 0xE2EDF9, 0xBFD2E5, 0x9DB7D2,\
                    1, 0xFFFFFFFF, 4, 0x000000},
    
};

#define IME_TEMPLATE_NUM_BLOCK_LEFT(i) \
    ((i-1)%IME_TEMPLATE_DEFAULT_COLS)*IME_TEMPLATE_DEFAULT_CELL_WIDTH

#define IME_TEMPLATE_NUM_BLOCK_TOP(i) \
    ((i-1)/IME_TEMPLATE_DEFAULT_COLS)*IME_TEMPLATE_DEFAULT_CELL_HEIGHT

#define IME_TEMPLATE_NUM_BLOCK_RIGHT(i) \
    (IME_TEMPLATE_NUM_BLOCK_LEFT(i)+IME_TEMPLATE_DEFAULT_CELL_WIDTH)

#define IME_TEMPLATE_NUM_BLOCK_BOTTOM(i) \
    (IME_TEMPLATE_NUM_BLOCK_TOP(i)+IME_TEMPLATE_DEFAULT_CELL_HEIGHT)

#define IME_TEMPLATE_NUM_BLOCK_RECT(i) \
     { IME_TEMPLATE_NUM_BLOCK_LEFT(i), IME_TEMPLATE_NUM_BLOCK_TOP(i), \
       IME_TEMPLATE_NUM_BLOCK_RIGHT(i), IME_TEMPLATE_NUM_BLOCK_BOTTOM(i) \
     }


#define  IME_TEMPLATE_NUM_BLOCK(i) \
 {i, TRUE, IME_TEMPLATE_NUM_BLOCK_RECT(i), TRUE, 0, TABLESIZE(text_areas_block##i), text_areas_block##i, 0, NULL}

SVGUI_IMAGE_T  svg_image[] = 
{
    {15, 23, 32, "ime/left.png"},
    {15, 23, 32, "ime/right.png"},
    {27, 14, 32, "ime/back.png"},
    {19, 8, 32, "ime/space.png"},
};

SVGUI_IMAGE_AREA_T image_area[] =
{
    {0, TRUE, {0, 0, IME_TEMPLATE_DEFAULT_CELL_WIDTH, IME_TEMPLATE_DEFAULT_CELL_HEIGHT}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, &svg_image[0]  },
    {0, TRUE, {0, 0, IME_TEMPLATE_DEFAULT_CELL_WIDTH, IME_TEMPLATE_DEFAULT_CELL_HEIGHT}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, &svg_image[1]  },
    {0, TRUE, {0, 0, IME_TEMPLATE_DEFAULT_CELL_WIDTH, IME_TEMPLATE_DEFAULT_CELL_HEIGHT}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, &svg_image[2]  },
    {0, TRUE, {20, 0, IME_TEMPLATE_DEFAULT_CELL_WIDTH, IME_TEMPLATE_DEFAULT_CELL_HEIGHT}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, &svg_image[3]  },
};

#define  IME_TEMPLATE_IMAGE_BLOCK(i, j) \
 {i, TRUE, IME_TEMPLATE_NUM_BLOCK_RECT(i), TRUE, 0, 0, NULL, 1, &(image_area[j])}


static SVGUI_TEXT_AREA_T speical_text_areas_block14 = {0, TRUE,  \
    IME_TEMPLATE_TEXT_RECT_1, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 0x000000, 0, ime_key_num[13] \
};

#define IME_TEMPLATE_14_BLOCK \
 {14, TRUE, IME_TEMPLATE_NUM_BLOCK_RECT(14), TRUE, 0, 1, &speical_text_areas_block14, 1, &(image_area[3])}

#if 1
static SVGUI_BLOCK_T ime_blocks [] = 
{
    {0, TRUE, {0, 0, IME_TEMPLATE_DEFAULT_WIDTH, IME_TEMPLATE_DEFAULT_HEIGHT}, FALSE, 0, 0, NULL, 0, NULL},  // back-ground
    IME_TEMPLATE_NUM_BLOCK(1),
    IME_TEMPLATE_NUM_BLOCK(2),
    IME_TEMPLATE_NUM_BLOCK(3),
    //IME_TEMPLATE_NUM_BLOCK(4),
    IME_TEMPLATE_IMAGE_BLOCK (4, 2),
    IME_TEMPLATE_NUM_BLOCK(5),
    IME_TEMPLATE_NUM_BLOCK(6),
    IME_TEMPLATE_NUM_BLOCK(7),
    //IME_TEMPLATE_NUM_BLOCK(8),
    IME_TEMPLATE_IMAGE_BLOCK (8, 0),
    IME_TEMPLATE_NUM_BLOCK(9),
    IME_TEMPLATE_NUM_BLOCK(10),
    IME_TEMPLATE_NUM_BLOCK(11),
    //IME_TEMPLATE_NUM_BLOCK(12),
    IME_TEMPLATE_IMAGE_BLOCK (12, 1),
    IME_TEMPLATE_NUM_BLOCK(13),
    //IME_TEMPLATE_NUM_BLOCK(14),
    IME_TEMPLATE_14_BLOCK,
    IME_TEMPLATE_NUM_BLOCK(15),
    IME_TEMPLATE_NUM_BLOCK(16),
};
#endif

#if 0
static SVGUI_BLOCK_T ime_blocks [] = 
{
    {0, TRUE, {0, 0, 256, 120}, FALSE, 0, 0, NULL, 0, NULL},  // back-ground
    
    // The first line 
    {1, TRUE, {0, 0, 64, 30}, TRUE, 0, TABLESIZE(text_areas_block1), text_areas_block1, 0, NULL},
    {2, TRUE, {64, 0, 128, 30}, TRUE, 0, TABLESIZE(text_areas_block2), text_areas_block2, 0, NULL},
    {3, TRUE, {128, 0, 192, 30}, TRUE, 0, TABLESIZE(text_areas_block3), text_areas_block3, 0, NULL},
    {4, TRUE, {192, 0, 256, 30}, TRUE, 0, TABLESIZE(text_areas_block4), text_areas_block4, 0, NULL},
#if 0
    // The second line 
    {5, TRUE, {0, 30, 64, 60}, TRUE, 0, TABLESIZE(text_areas_block5), text_areas_block5, 0, NULL},
    {6, TRUE, {64, 30, 128, 60}, TRUE, 0, TABLESIZE(text_areas_block6), text_areas_block6, 0, NULL},
    {7, TRUE, {128, 30, 192, 60}, TRUE, 0, TABLESIZE(text_areas_block7), text_areas_block7, 0, NULL},
    {8, TRUE, {192, 30, 256, 60}, TRUE, 0, TABLESIZE(text_areas_block8), text_areas_block8, 0, NULL},

    // The third line 
    {9, TRUE, {0, 60, 64, 90}, TRUE, 0, TABLESIZE(text_areas_block9), text_areas_block9, 0, NULL},
    {10, TRUE, {64, 60, 128, 90}, TRUE, 0, TABLESIZE(text_areas_block10), text_areas_block10, 0, NULL},
    {11, TRUE, {128, 60, 192, 90}, TRUE, 0, TABLESIZE(text_areas_block11), text_areas_block11, 0, NULL},
    {12, TRUE, {192, 60, 256, 90}, TRUE, 0, TABLESIZE(text_areas_block12), text_areas_block12, 0, NULL},


    // The fourth line 
    {13, TRUE, {0, 90, 64, 120}, TRUE, 0, TABLESIZE(text_areas_block13), text_areas_block13, 0, NULL},
    {14, TRUE, {64, 90, 128, 120}, TRUE, 0, TABLESIZE(text_areas_block14), text_areas_block14, 0, NULL},
    {15, TRUE, {128, 90, 192, 120}, TRUE, 0, TABLESIZE(text_areas_block15), text_areas_block15, 0, NULL},
    {16, TRUE, {192, 90, 256, 120}, TRUE, 0, TABLESIZE(text_areas_block16), text_areas_block16, 0, NULL},
#endif
};
#endif

static SVGUI_FONT_INFO_T font_infos [] =
{
    {"fmhei", 18, "UTF-8"},
    {"fmhei", 14, "UTF-8"}
};

static SVGUI_HEADER_T header =
{
    IME_TEMPLATE_DEFAULT_WIDTH, IME_TEMPLATE_DEFAULT_HEIGHT,
    0x000000, 0xFFFFFF,
    TABLESIZE(prdrd_blocks), prdrd_blocks,
    TABLESIZE(font_infos), font_infos,
    TABLESIZE(ime_blocks), ime_blocks,
};

static int sg_ime_width = IME_TEMPLATE_DEFAULT_WIDTH;
static int sg_ime_height = IME_TEMPLATE_DEFAULT_HEIGHT;

void  mdtv_ImeFunKeyMessageTranslate (HWND hwnd, int id)
{
        switch(id)
        {
            case 4:
                SendMessage (hwnd, MSG_KEYDOWN, SCANCODE_BACKSPACE, 0);
                break;
            case 8:
                SendMessage (hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKLEFT, 0);
                break;
            case 12:
                SendMessage (hwnd, MSG_KEYDOWN, SCANCODE_CURSORBLOCKRIGHT, 0);
                break;
            case 13:
                SendMessage (hwnd, MSG_KEYDOWN, SCANCODE_EQUAL, 0);
                break;
            case 15:
                SendMessage (hwnd, MSG_KEYDOWN, SCANCODE_ENTER, 0);
                break;
            case 16:
                break;
            default:
                break;
        }
}

static int ImeWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case MSG_CREATE:
            {
#ifdef MDTV_IME_DEBUG
                printf ("Create ime win proc\n");
#endif
                HDC secdc =  alloc_subdc_from_uisec(100, 100, sg_ime_width,  sg_ime_height);
                SetSecondaryDC (hWnd, secdc, ON_UPDSECDC_DEFAULT);
                mdtv_CreateImePinying ();
            break;
            }

        case MSG_LBUTTONDOWN:
            {
                int x_pos = LOWORD (lParam);
                int y_pos = HIWORD (lParam);
                SVGUI_HEADER_I *header = NULL;
                SVGUI_BLOCK_I* block = NULL;
                int keynumber, ret;
                
                ret = mdtv_GetImeStatus ();
                if (!ret)  //ime is close status
                    break;

                //SendMessage(sg_phone_ime_hwnd, MSG_IME_CMD_OPEN, 0, 3);
#ifdef MDTV_IME_DEBUG
                printf ("Debug: xpos=%d, ypos=%d\n", x_pos, y_pos);
#endif
                header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
                if (header == NULL)
                    break;
                block = svgui_get_block_by_point (header, x_pos, y_pos);

                if  (((block->id < 12)&&((block->id) % 4 != 0)) || (block->id == 14)) // number 
                {
                    keynumber =  atoi ((block->text_areas[0]).text);
#ifdef MDTV_IME_DEBUG
                    printf ("Debug: keydown %d\n", keynumber);
#endif
                    SendMessage (sg_phone_ime_hwnd, MSG_KEYDOWN, default_map[8+keynumber].scancode, 0);
                }
                else if (block->id == 16)
                {
                    mdtv_DestroyPhoneIme (hWnd);
                    return 0;
                }
                    //PostQuitMessage (hWnd);
                else
                    mdtv_ImeFunKeyMessageTranslate (sg_phone_ime_hwnd, block->id);
                break;
            }

        case MSG_PAINT:
            break;

        case MSG_CLOSE:
#ifdef MDTV_IME_DEBUG
            printf ("Quit window \n");
#endif
            DestroyMainWindow (hWnd);
            sg_imehMainWnd = NULL;
            //PostQuitMessage (hWnd);
            //mgiDestroyPhoneKeyPadIME (sg_phone_ime_hwnd);
            break;
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}

int mdtv_NotifyCursorPos (RECT *rc)
{
    if (sg_phone_ime_hwnd){
        PostMessage (sg_phone_ime_hwnd, MSG_IME_CMD_SETPOS, (DWORD)rc, 0);
    }
    return 0;
}

int mdtv_send_msg_to_ime(int message, WPARAM wParam, LPARAM lParam )
{
    PostMessage (sg_phone_ime_hwnd, message, wParam, lParam);
    //SendMessage (sg_phone_ime_hwnd, message, wParam, lParam);
    return 0;
}

int mdtv_ime_switch_method()
{
    mdtv_send_msg_to_ime ( MSG_KEYDOWN, SCANCODE_EQUAL, 0);
    mdtv_set_browser_title_ime_status(g_mdolphin_main_hwnd, get_ime_hint_text ());
    return 0;
}
HWND mdtv_GetPhoneImeWnd()
{
    return sg_phone_ime_hwnd;
}

void mdtv_SetPhoneImeRect (RECT *caret_rect)
{
    CopyRect (&sg_phone_ime_rect, caret_rect);
}

HWND mdtv_InitPhoneIme (RECT *caret_rect)
{

#ifdef MDTV_IME_DEBUG
    printf ("init phone\n");
#endif
    CopyRect (&sg_phone_ime_rect, caret_rect);
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER , "svgui" , 0 , 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST |WS_EX_TOOLWINDOW  ;
    CreateInfo.spCaption = "ime";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ImeWinProc;
    CreateInfo.lx = 300;
    CreateInfo.ty = 300;
    CreateInfo.rx = CreateInfo.lx + sg_ime_width;
    CreateInfo.by = CreateInfo.ty + sg_ime_height;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)(&header);
    CreateInfo.hHosting = HWND_DESKTOP;
    
    sg_imehMainWnd = CreateMainWindow (&CreateInfo);
    
    if (sg_imehMainWnd == HWND_INVALID)
        return NULL;

    ShowWindow(sg_imehMainWnd, SW_SHOWNORMAL);

    return sg_imehMainWnd;
}

void mdtv_DestroyPhoneIme (HWND hwnd)
{
#if 1
    if (sg_phone_ime_hwnd != HWND_INVALID )
    {
        mgiPhoneKeyPadRemoveMethod (sg_phone_ime_hwnd, 101);
        mdtv_ShowIme (IME_CMD_SET_TARGET, (HWND)NULL); 
        mdtv_ShowIme(IME_CMD_CLOSE, sg_phone_ime_hwnd);  //close ime
        PostMessage (sg_phone_ime_hwnd, MSG_CLOSE, 0, 0);
        //SendMessage (sg_phone_ime_hwnd, MSG_CLOSE, 0, 0);
        sg_phone_ime_hwnd = (HWND)HWND_INVALID;
    }
    if (hwnd != HWND_INVALID )
        PostMessage (hwnd, MSG_CLOSE, 0, 0);
#else
    if (sg_phone_ime_hwnd != NULL){
        mgiPhoneKeyPadRemoveMethod (sg_phone_ime_hwnd, 101);
        //mdtv_ShowIme (IME_CMD_SET_TARGET, (HWND)NULL); 
        mdtv_ShowIme(IME_CMD_CLOSE, sg_phone_ime_hwnd);  //close ime
        ShowWindow(sg_phone_ime_hwnd, SW_HIDE);
    }
    if (hwnd != NULL )
        PostMessage (hwnd, MSG_CLOSE, 0, 0);
#endif
}
