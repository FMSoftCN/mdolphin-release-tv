#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <minigui/common.h>
#include <sys/poll.h>
#include <stdarg.h>

#include "mgi-phone.h"
#include "mgi_helper.h"
#include "mdtv_ime.h"
#include "ime_res.h"
#include "mdtv_keyboard.h"

#include <mgplus/mgplus.h>

#define NDEBUG  1
#include <mdolphin/mdolphin.h>

#define MAX_LEN_KEYSTROKES          30

#ifndef MAX_LEN_USER_WORD
#  define MAX_LEN_USER_WORD         12
#endif

#ifndef MAX_LEN_SYMBOL
#  define MAX_LEN_SYMBOL            256
#endif

#define PHONE_DEFAULT_CASE_num      0x01
#define PHONE_DEFAULT_CASE_ABC      0x02
#define PHONE_DEFAULT_CASE_abc      0x03
#define PHONE_DEFAULT_CASE_MAX      (PHONE_DEFAULT_CASE_abc + 1)

#define PHONE_ERROR_INPUT_STATUS    -1 
#define PHONE_NO_INPUT_STATUS       0
#define PHONE_SYMBOL_INPUT_STATUS   1
#define PHONE_CHAR_INPUT_STATUS     2
#define PHONE_PRED_INPUT_STATUS     3   /*predict word input*/

#define CARET_REPLY "/var/tmp/caretreply"
#define HEADER_LEN 10
#define BODY_LEN 30

#ifdef _MGRM_PROCESSES
static int conn_fd_read = -1;
#endif

#define X_OFFSIZE 10
#define Y_OFFSIZE 80
#define INPUT_BRACKGROUND_COLORKEY 0x00000000

#define MDTV_MGPHONE_DEBUG

//extern char * punctuate_table[] ;

pthread_mutex_t mgphone_lock;

extern char ch_punctuate [];
extern char en_punctuate[];
extern char website_punctuate[];

char * punctuate_table[] = 
{
    en_punctuate, 
    ch_punctuate,
    website_punctuate, 
};

BOOL mgi_phone_ime_get_curr_methodcode(char *code, int code_len, int method_id);
static void show_prev_page(HWND hwnd);
static void show_next_page(HWND hwnd);
static void  handle_data (HWND hwnd, const char *stokes, int cursor);

static void update_state(HWND hwnd);
static void get_cur_word_rect(HWND hwnd);
static int process_confirm_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);
static int process_quit_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam);
static void handle_translate_word(HWND hwnd, int index, int cursor);

static BITMAP ime_ch_bkg;
static BITMAP ime_en_bkg;
static BITMAP ime_nav_bkg;
static PLOGFONT font_cn;

static BITMAP ime_enable_left_arrow;
static BITMAP ime_enable_right_arrow;
static BITMAP ime_disable_left_arrow;
static BITMAP ime_disable_right_arrow;

static const char* imepicture [] = 
{
    "res/ime/dis_left.png",
    "res/ime/en_left.png",
    "res/ime/dis_right.png",
    "res/ime/en_right.png"
};

RECT ime_arrow_pos [] = 
{
    {360, 38, 368, 53},
    {375, 38, 383, 53}
};

struct _MGI_PHONE_IME_DATA
{
    /* Save move flag */
    BOOL    is_left;
    BOOL    is_right;
    BOOL    is_top;
    BOOL    is_bottom;
                                            
    /* Save next and prev cursor value */
    int     next_cursor;
    int     prev_cursor;
    int     cur_cursor;

    int     next_cursor_down;
    int     prev_cursor_down;
    int     cur_cursor_down;

    int     str_num;
    int     str_down_num;
    
    /*current selected word index*/
    int     cur_index;
    int     cur_index_down;
    RECT    cur_rect;
                                                                                            
    MGI_PHONE_IME_METHOD *head_method;
    MGI_PHONE_IME_METHOD *cur_method;
                                                                                                            
    char    cur_word[PHONE_RESULT_BUFF_LONG+1];
    char    old_strokes[MAX_LEN_KEYSTROKES+1];
    char    key_strokes[MAX_LEN_KEYSTROKES+1];
    char    str[PHONE_RESULT_BUFF_LONG+1];      /* str display in up line */
    char    str_down[PHONE_RESULT_BUFF_LONG+1]; /* str display in down line */
    char    symbol[MAX_LEN_SYMBOL+1];           /* all symbol */

    MGICB_ON_PHONE_IME_SWITCHED cb_notify;      /* notify callback */
    const MGI_PHONE_KEY_MAP         (*key_map)[MAX_PHONE_KEY_NUM];
    const MGI_PHONE_IME_TEXTCOLOR   *text_color;
    BITMAP                          *bkgnd_bmp;
    
    int                         is_opened; 
    int                         pti_switch_flag;
    int                         ime_switch_flag;
    int                         ptim_case;          /* num ABC abc ...*/

    HWND                        phone_imehwnd;
    HWND                        sg_target_hwnd; 
    BOOL                        is_focus_up;        /* if focus in up line */
    int                         phone_input_status; /*PHONE_SYMBOL_INPUT_STATUS 
                                                      PHONE_NO_INPUT_STATUS 
                                                      PHONE_CHAR_INPUT_STATUS
                                                      PHONE_PRED_INPUT_STATUS
                                                      PHONE_ERROR_INPUT_STATUS*/
    HGRAPHICS hgs ;  //FixMe: add by lvlei

};
typedef struct _MGI_PHONE_IME_DATA *MGI_PHONE_IME_DATA;

HWND g_phone_hwnd = HWND_INVALID;
static void send_word (MGI_PHONE_IME_DATA pdata, char *word);

#ifdef _MGRM_PROCESSES
static int listen_fd;
static BOOL listen_socket (HWND hwnd)
{
    if ((listen_fd = serv_listen (CARET_REPLY)) < 0)
        return FALSE;
    return RegisterListenFD (listen_fd, POLLIN, hwnd, NULL);
}
#endif



void output_phone_input_status (int status)
{
#ifdef MDTV_MGPHONE_DEBUG 
    switch (status) {
        case PHONE_NO_INPUT_STATUS:
            md_debug ("the phone_input_status is: PHONE_NO_INPUT_STATUS\n");
            break;
        case PHONE_SYMBOL_INPUT_STATUS:
            md_debug ("the phone_input_status is: PHONE_SYMBOL_INPUT_STATUS\n");
            break;
        case PHONE_PRED_INPUT_STATUS:
            md_debug ("the phone_input_status is: PHONE_PRED_INPUT_STATUS\n");
            break;
        case PHONE_ERROR_INPUT_STATUS:
            md_debug ("the phone_input_status is: PHONE_ERROR_INPUT_STATUS\n");
            break;
        case PHONE_CHAR_INPUT_STATUS:
            md_debug ("the phone_input_status is: PHONE_CHAR_INPUT_STATUS\n");
            break;
        default :
            md_debug ("the phone_input_status is: error\n");
    }
    return ;

        
#else
#endif
}

static void phone_adddata_init (void *data)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) data;

    /*todo: initialize addtional data*/
    pdata->is_left = pdata->is_top = pdata->is_right = pdata->is_bottom = FALSE;
    
    pdata->prev_cursor = pdata->cur_cursor = 0;
    pdata->next_cursor = -1;

    pdata->prev_cursor_down = pdata->cur_cursor_down = 0;
    pdata->next_cursor_down = -1;

    pdata->cur_index = 0;
    pdata->cur_index_down = 0;
    SetRectEmpty(&pdata->cur_rect);

    memset(pdata->cur_word, 0, sizeof(pdata->cur_word));
    memset(pdata->old_strokes, 0, sizeof(pdata->old_strokes));
    memset(pdata->key_strokes, 0, sizeof(pdata->key_strokes));
    memset(pdata->str, 0, sizeof(pdata->str));
    memset(pdata->str_down, 0, sizeof(pdata->str_down));
    memset(pdata->symbol, 0, sizeof(pdata->symbol));

    pdata->is_focus_up = TRUE;
    pdata->phone_input_status = PHONE_NO_INPUT_STATUS; 
}

/*======================begin tradition ime======================*/

/*for traditional input method*/
static int rep_count = 0;
static inline void get_window_rect(HWND hwnd, RECT *rect)
{
    RECT my_rect;
    GetWindowRect(hwnd, &my_rect);
    SetRect(rect, 0, 0, RECTW(my_rect), RECTH(my_rect));

    return;
}

static inline void new_move_ime_window(HWND hWnd, int xsize, int ysize)
{
    RECT rect;
    GetWindowRect(hWnd, &rect);

    int max_x = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - 1;
    
    //if the right border of ime window > max_x
    if (xsize + RECTW(rect) > max_x)
        xsize = max_x - RECTW(rect);

    if (xsize < 0 )
        xsize = 0;

    MoveWindow(hWnd,  xsize, ysize, RECTW(rect), RECTH(rect), FALSE);
}

#ifdef _MGRM_PROCESSES
static void socketProcess(HWND hWnd, char *buf)
{
    md_debug("socket msg is %s\n", buf);
    
    //buff: X=111Y=111R=111B=111\t\t...\t
    int xsize;
    int ysize = 0;
    char leftStr[8] = {0};
    char topStr[8] = {0};
    char bottomStr[8] = {0};
    char *p = strchr(buf, 'Y');
    strncpy(leftStr, buf + 2, p - buf - 2);
    int left = atoi(leftStr);
    xsize = left + X_OFFSIZE;
    char *p2 = strchr(buf, 'R');
    strncpy(topStr, p + 2, p2 - p -2);
    int top = atoi(topStr);

    if (left == 0 && top == 0)
    {
        new_move_ime_window(hWnd, 0, -100);
        return;
    }
    p = strchr(buf, 'B');
    p2 = strchr(buf, '\t');
    strncpy(bottomStr, p + 2, p2 -p -2);
    int bottom = atoi(bottomStr);

    RECT rect;
    get_window_rect(hWnd, &rect);
    md_debug("rect is [%d, %d, %d, %d]\n", rect.left, rect.top, rect.right, rect.bottom);

    ysize = bottom - rect.top + Y_OFFSIZE;
    
    int max_y = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - 1;
    if (ysize + RECTH(rect) > max_y)
        ysize = top - rect.top - Y_OFFSIZE;


    new_move_ime_window(hWnd, xsize, ysize);
}
#endif

static BOOL filter_trad_en_key (HWND hwnd, MGI_PHONE_IME_DATA pdata, unsigned int key, LPARAM lParam)
{
    static UINT prev_char = 0;
    BOOL bRep;
    unsigned char c;
    /*for long press*/
#if 0
#ifndef __NOUNIX__
#define TIME_UNIT       1000000
#define TIME_INTERVAL   800000    
    static struct timeval oldtime = {0, 0}, newtime = {0, 0};
#elif defined __VXWORKS__
#define TIME_UNIT       1000000000
#define TIME_INTERVAL   800000000 
    static struct timespec oldtime={0, 0}, newtime = {0, 0};
#endif
#endif

#define TIME_INTERVAL   80   
    static unsigned int oldtick, newtick;
    unsigned long interval;

    if (key == 127) {
        return FALSE;
    }

#if 0
    oldtime = newtime;
#ifndef __NOUNIX__
    gettimeofday (&newtime, NULL);
    interval = (newtime.tv_sec - oldtime.tv_sec)*TIME_UNIT + 
                (newtime.tv_usec - oldtime.tv_usec);
#elif defined __VXWORKS__
    clock_gettime(CLOCK_REALTIME, &newtime);
    interval = (newtime.tv_sec - oldtime.tv_sec)*TIME_UNIT + 
                (newtime.tv_nsec - oldtime.tv_nsec);
#endif
#endif
    oldtick = newtick;
    newtick = GetTickCount();
    interval = newtick - oldtick;

    if (prev_char && prev_char == key && interval < TIME_INTERVAL) {
        rep_count ++;
        bRep = TRUE;
    }
    else {
        rep_count = 0;
        bRep = FALSE;
    }

    prev_char = key;

    if (key > 0) {
        const char *pchar;
        pchar =(*pdata->key_map)[key].letters;

        if (!pchar)
            return FALSE;

        unsigned int i = 0;
        unsigned int j = 0;
        for(; i < strlen(pchar)-1; ++i, ++j)
        {
            pdata->str[j] = *(pchar + i);
            pdata->str[++j] = ' ';
        }
        pdata->str[j] = '\0';
        pdata->str_num = strlen(pchar);

        c = *(pchar+rep_count);
        if ( isdigit(c) && rep_count > 0) {
            rep_count = 0;
            c = *(pchar);
        }
        pdata->cur_index = rep_count;
        //for key:1 
        if (key == MGI_PHONE_KEY_1)
        {
            char tmp_str[80]={0};
            int half_len=0;
            int tmp_len = 0;

            strcpy(tmp_str, pdata->str);
            tmp_len = strlen(tmp_str);
            half_len = tmp_len / 2;

            if ((unsigned int)rep_count < strlen(pchar)/2)
            {
                strncpy(pdata->str, tmp_str, half_len);
                pdata->str[half_len] = '\0';
            } else {
                strncpy(pdata->str, tmp_str + half_len, tmp_len - half_len);
                pdata->str[tmp_len - half_len] = '\0';
                pdata->cur_index = rep_count - strlen(pchar) / 2;
            }
            pdata->str_down[0] = '\0';
        }

#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        if (bRep) {
            Send2ActiveWindow (mgTopmostLayer, MSG_KEYDOWN, (*pdata->key_map)[MGI_PHONE_KEY_CLEAR].scancode, 0);
            Send2ActiveWindow (mgTopmostLayer, MSG_KEYUP, (*pdata->key_map)[MGI_PHONE_KEY_CLEAR].scancode, 0);
        }

        if (pdata->ptim_case == PHONE_DEFAULT_CASE_ABC)
            Send2ActiveWindow (mgTopmostLayer, MSG_CHAR, toupper(c), 0);
        else
            Send2ActiveWindow (mgTopmostLayer, MSG_CHAR, c, 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        if (bRep) {
            PostMessage(pdata->sg_target_hwnd, MSG_KEYDOWN, (*pdata->key_map)[MGI_PHONE_KEY_CLEAR].scancode, lParam|KS_IMEPOST);
            PostMessage(pdata->sg_target_hwnd, MSG_KEYUP, (*pdata->key_map)[MGI_PHONE_KEY_CLEAR].scancode, lParam|KS_IMEPOST);
        }
#endif
        if (pdata->ptim_case == PHONE_DEFAULT_CASE_ABC){
            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, toupper(c), lParam|KS_IMEPOST);
        }
        else{
            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, c, lParam|KS_IMEPOST);
        }
    }
    
    pdata->is_focus_up = TRUE;
    InvalidateRect(hwnd, NULL, TRUE);
    return TRUE;
}

static void 
process_trad_en_num_key (HWND hwnd, MGI_PHONE_IME_DATA pdata, int message, int key)
{
    if (message == MSG_KEYDOWN) 
        filter_trad_en_key(hwnd,pdata, key, 0);
}
/*======================end tradition ime======================*/

/*
 *fn: static void phone_switch_mode (MGI_PHONE_IME_DATA pdata)
 * */
static void phone_switch_mode (MGI_PHONE_IME_DATA pdata)
{
    int method_num=0;
    MGI_PHONE_IME_METHOD *p=NULL;

    if (!pdata)
        return;

    p = pdata->head_method;
    while(p) {
        p = p->next;
        method_num++;
    }

    if (pdata->ptim_case < (PHONE_DEFAULT_CASE_MAX + method_num - 1))
        pdata->ptim_case++;
    else 
        pdata->ptim_case = PHONE_DEFAULT_CASE_num;

    if (pdata->ptim_case >= PHONE_DEFAULT_CASE_MAX) {
        int i = pdata->ptim_case - PHONE_DEFAULT_CASE_MAX;
        p = pdata->head_method;
        while (i && p) {
            p=p->next;
            i--;
        }
        pdata->cur_method = p;
    }
}

static int get_words_num(const char* buff) {
    int n = 0;
    char *p = NULL;

    p = strchr (buff, ' ');

    while (p != NULL) {
        n++;
        p++;

        p = strchr(p, ' ');
    }
        
    return n;
}

static char* 
get_word(const char* buff, int index, char *word, int len) {
    char *p = NULL;
    int n = 0;
    if (strlen(buff) <= 0)
        return NULL;

    p = strchr (buff, ' ');
    if (index == 0) {
        if (!p)
            return NULL;
        strncpy(word, buff, p - buff);
        word[p-buff] = '\0';
        return word;
    }

    while (p != NULL) {
        n++;
        if (n == index) {
            char *t;
            p++;
            t = strchr(p, ' ');

            if (t == NULL) {
                strcpy (word, p);
            }
            else {
                strncpy (word, p, t-p);
                word[t-p] = '\0';
            }

            return word;
        }
        p++;
        p = strchr(p, ' ');
    }
    return NULL;
}

static void process_down_key(HWND hwnd, MGI_PHONE_IME_DATA pdata);
void 
process_mousemsg(HWND hwnd,  int message, WPARAM wParam, LPARAM lParam) {

    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    char *p=NULL;
    BOOL is_up  = FALSE;
    BOOL is_switch = FALSE;
    RECT clientrc;
    HDC hdc = GetDC(hwnd);
    SIZE textsize, cur_size;
    char tmp[PHONE_RESULT_BUFF_LONG]={0};
    int xpos = LOWORD (lParam);
    int ypos = HIWORD (lParam);
    int screenxpos = xpos;
    int screenypos = ypos;

    ClientToScreen (hwnd, &screenxpos, &screenypos);

    memset(&textsize, 0, sizeof(SIZE));
    GetClientRect (hwnd, &clientrc);
    if (ypos < (float)(RECTH(clientrc))/2)
            is_up = TRUE;
    if (is_up)
        p = pdata->str;
    else 
        p=pdata->str_down;

    if (pdata->is_focus_up &&  !is_up  )
    {
        is_switch = TRUE;
    }

    float offset =   ime_nav_bkg.bmWidth/30;
    char *curstr = NULL, *prevstr=p;
    int curstrlen = 0;
    int wordnum =  0;
    if (is_up)
        wordnum = pdata->str_num;
    else
        wordnum = pdata->str_down_num;

    if (is_up && pdata->phone_input_status == PHONE_PRED_INPUT_STATUS){
        ReleaseDC (hdc);
        return ;
    }

    if (!is_up && pdata->ptim_case < PHONE_DEFAULT_CASE_MAX && !(pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) ){
        ReleaseDC (hdc);
        return ;
    }


    if (is_switch) {
            //pdata->cur_index_down = 0;
            //if (pdata->cur_index < 0)
            //    pdata->cur_index = 0;
            //handle_translate_word(hwnd, pdata->cur_index, 0);
            process_down_key(hwnd, pdata);
            pdata->is_focus_up = is_up;
    }

    md_debug ("str_down_num=%d\n", wordnum);
    for (int j = 0; j < wordnum; j++)
    {
        curstr = strchr (prevstr, ' '); 
        if (curstr == NULL)
            break;
        curstrlen  += (int )((curstr - prevstr) + 1);
        strncat (tmp, prevstr, curstr - prevstr +1);


        GetTextExtent(hdc, tmp, curstrlen, &cur_size);
        md_debug ("j = %d, curstrlen=%d, cur_size.cx=%d, xpos=%d\n", j, curstrlen, cur_size.cx, xpos);
        if (offset+cur_size.cx >= xpos)
        {
            char cur_word[64];

            if (is_up)
                pdata->cur_index = j;
            else
                pdata->cur_index_down = j;

            get_word(p, j, cur_word, 64);
            if (pdata->cur_word) {
                strcpy (pdata->cur_word, cur_word);
                get_cur_word_rect(hwnd);
            }
            //send_word(pdata, cur_word);
            if (pdata->ptim_case >= PHONE_DEFAULT_CASE_MAX || pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS)
                process_confirm_key (hwnd, MSG_KEYDOWN, 0, 0);
            break;
        }
        prevstr = curstr+1; 
    }
    
    ReleaseDC (hdc);

    if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        update_state(hwnd);

    // process   pageup/pagedown when clicking arrow
    if (pdata->ptim_case >= PHONE_DEFAULT_CASE_MAX  && !is_up) {
       if (PtInRect (&ime_arrow_pos[0],  xpos, ypos) && (pdata->cur_cursor_down > pdata->prev_cursor_down)) {
           md_debug ("click left arrow\n");
           show_prev_page(hwnd);
       }
       else if (PtInRect (&ime_arrow_pos[1],  xpos, ypos) &&(pdata->next_cursor_down > pdata->cur_cursor_down)) {
           md_debug ("click right arrow\n");
           show_next_page(hwnd);
       }
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

static BOOL 
get_substr_pos_ex (HDC hdc, const char* str, const char* substr, RECT *rc)
{

    int fit, startpos;
    int pos[2 *PHONE_RESULT_BUFF_LONG + 1]={0};
    int dx[2 * PHONE_RESULT_BUFF_LONG + 1]={0};
    SIZE txtsize;
    char *p = strstr (str, substr);

    if (!p)
        return FALSE;

    GetTextExtentPoint (hdc, 
        (const char*)str, 
        strlen(str), 0, 
        &fit,
        pos,
        dx,
        &txtsize);

    startpos = p - str;

    rc->top = 0;
    rc->left = dx [startpos];
    rc->right = dx [startpos + strlen(substr)] + 1;
    rc->bottom = rc->top + GetFontHeight (hdc) + 1; 
    
    return TRUE;
}

static void get_cur_word_rect(HWND hwnd)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    char *p=NULL;
    HDC hdc = GetDC(hwnd);
    
    if(pdata->is_focus_up)
        p=pdata->str;
    else
        p=pdata->str_down;

    if (isascii(pdata->cur_word[0])) {
        get_substr_pos_ex (hdc, 
            p, 
            pdata->cur_word, 
            &pdata->cur_rect);
    }
    else {
        SIZE textsize, cur_size;
        char tmp[PHONE_RESULT_BUFF_LONG]={0};
        char *t;
        int n;
        memset(&textsize, 0, sizeof(SIZE));

        /* maybe string have same word, so need ensure by word length  */
        t = strstr(p, pdata->cur_word);
        md_debug("p is %s pdata->cur_word is %s\n", p, pdata->cur_word);
        while(t) {
            n = get_words_num(t);
            if (pdata->is_focus_up) {
                if (pdata->cur_index == pdata->str_num-n) {
                    strncpy(tmp, p, t-p);
                    tmp[t-p] = '\0';
                    break;
                }
                else {
                    t++;
                    t = strstr(t, pdata->cur_word);
                }
            }
            else {
                md_debug("index_down %d down_num %d\n", pdata->cur_index_down, pdata->str_down_num);
                if (pdata->cur_index_down == pdata->str_down_num-n) {
                    strncpy(tmp, p, t-p);
                    tmp[t-p] = '\0';
                    break;
                }
                else {
                    t++;
                    t = strstr(t, pdata->cur_word);
                }
            }
        }

        GetTextExtent(hdc, pdata->cur_word, strlen(pdata->cur_word), &cur_size);
        GetTextExtent(hdc, tmp, strlen(tmp), &textsize);
        md_debug("tmp %s strlen %d\n", tmp, strlen(tmp));
        
        SetRect(&pdata->cur_rect, 
                textsize.cx, 
                0,
                textsize.cx+cur_size.cx+1,
                cur_size.cy+1);
    }

    if (!pdata->is_focus_up) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        OffsetRect(&pdata->cur_rect, 0 , RECTH(rc)/2);
    }
    ReleaseDC (hdc);
}

#define BUFFERSIZEMG 1024

static int convertgb2312toutf8(unsigned char *buffer, const unsigned char* characters, size_t mbs_length)
{
    int conved_mbs_len, ucs_len;
    char buffer2[BUFFERSIZEMG];

    PLOGFONT logfont = CreateLogFont (NULL, "arial", "gb2312",
            FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
            FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
            12, 0); 

    const unsigned char* source = characters;
    ucs_len = MBS2WCSEx (logfont, (void *)buffer2, FALSE,  source, mbs_length, BUFFERSIZEMG,  &conved_mbs_len); 
    DestroyLogFont(logfont);
#if 1
     logfont = CreateLogFont (NULL, "arial", "utf8",
            FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
            FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
            12, 0); 

    ucs_len = WCS2MBSEx (logfont, (unsigned char *)buffer,   (unsigned char *)buffer2, ucs_len, FALSE, BUFFERSIZEMG,  &conved_mbs_len); 

   // ucs_len = MBS2WCSEx (logfont, (void *)buffer, FALSE,  (unsigned char *)buffer2, ucs_len, BUFFERSIZEMG,  &conved_mbs_len); 
    DestroyLogFont(logfont);
#endif
    return ucs_len; 

}

static void send_word (MGI_PHONE_IME_DATA pdata, char *word)
{
    int i = 0; 
    int len = strlen (word);
    unsigned char buffer[BUFFERSIZEMG];
    BOOL    bDByte=FALSE;
    

    if (!isascii(word[0]))
        bDByte=TRUE;


    if (bDByte)
    {
        len =  convertgb2312toutf8(buffer, (unsigned char *)word, len);
        word = (char *)buffer;
        char *str = word;

        while(i<len)
        {
            WPARAM ch;
            //Process UTF8 flow 
            if(((Uint8)str[i]) <= 0x7F) //ascii code
            {
                ch = (WPARAM)str[i];
                i++;
            }
            else if(((Uint8)str[i]) <= 0xBF) //
            {
                i ++;
                continue;
            }
            else if(((Uint8)str[i]) <= 0xDF) //2 code
            {
                ch = ((Uint8) str[i])|(((Uint8)str[i+1])<<8);
                i += 2;
            }
            else if(((Uint8)str[i]) <= 0xEF) //three code
            {
                ch = ((Uint8)str[i])
                    | (((Uint8)str[i+1])<<8)
                    | (((Uint8)str[i+2])<<16);
                i += 3;
            }
            else
            {
                i ++;
                continue;
            }

            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, ch, KS_IMEPOST);
        }
    } else {
        for (i = 0; i < len; i++)
        {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow (mgTopmostLayer, MSG_CHAR, word[i], 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
#ifdef __NOUNIX__
            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, word[i], KS_IMEPOST);
            //SendMessage(pdata->sg_target_hwnd, MSG_CHAR, word[i], 0);
#else
            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, word[i], KS_IMEPOST);
#endif
#endif
        }
    }
}


static void handle_add_data (HWND hwnd, MGI_PHONE_IME_DATA pdata)
{
    char cur_word[64]={0};
    
    /*memset (cur_word, 0, 64);*/
    if (pdata->is_focus_up){
        if (pdata->cur_index > pdata->str_num - 1)
            pdata->cur_index = pdata->str_num - 1;
        get_word(pdata->str, pdata->cur_index, cur_word, 64);
    }
    else {
        if (pdata->cur_index_down > pdata->str_down_num - 1)
            pdata->cur_index_down = pdata->str_down_num - 1;
        get_word(pdata->str_down, pdata->cur_index_down, cur_word, 64);
    }

    if (pdata->cur_word) {
        md_debug("begin get cur word\n");
        strcpy (pdata->cur_word, cur_word);
        get_cur_word_rect(hwnd);
    }
}

static void update_state(HWND hwnd)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);

    if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS){
        md_debug("symbol input status\n");
        if (pdata->is_focus_up) {
            if (pdata->cur_index == 0)
                pdata->is_left = FALSE;
            else
                pdata->is_left = TRUE;

            if (pdata->cur_index == pdata->str_num - 1)
                pdata->is_right = FALSE;
            else
                pdata->is_right = TRUE;
        }
        else {
             if (pdata->cur_index_down == 0)
                pdata->is_left = FALSE;
            else
                pdata->is_left = TRUE;

            if (pdata->cur_index_down == pdata->str_down_num - 1)
                pdata->is_right = FALSE;
            else
                pdata->is_right = TRUE;
        }

        if (pdata->prev_cursor == pdata->cur_cursor)
            pdata->is_top = FALSE;
        else
            pdata->is_top = TRUE;

        if (pdata->next_cursor == -1)
            pdata->is_bottom = FALSE;
        else
            pdata->is_bottom = TRUE;
    }
    else {
        if (pdata->is_focus_up) {
            if (pdata->cur_index == 0)
                pdata->is_left = FALSE;
            else
                pdata->is_left = TRUE;

            if (pdata->cur_index == pdata->str_num - 1)
                pdata->is_right = FALSE;
            else
                pdata->is_right = TRUE;

            if (pdata->prev_cursor == pdata->cur_cursor)
                pdata->is_top = FALSE;
            else
                pdata->is_top = TRUE;

            if (pdata->next_cursor == -1)
                pdata->is_bottom = FALSE;
            else
                pdata->is_bottom = TRUE;
        }
        else {
            if (pdata->cur_index_down == 0)
                pdata->is_left = FALSE;
            else
                pdata->is_left = TRUE;

            if (pdata->cur_index_down == pdata->str_down_num - 1)
                pdata->is_right = FALSE;
            else
                pdata->is_right = TRUE;

            if (pdata->prev_cursor_down == pdata->cur_cursor_down)
                pdata->is_top = FALSE;
            else
                pdata->is_top = TRUE;

            if (pdata->next_cursor_down == -1)
                pdata->is_bottom = FALSE;
            else
                pdata->is_bottom = TRUE;
        }
    }
}

static void get_translate_word(MGI_PHONE_IME_DATA pdata, int index, int cursor)
{
    char buff_translate[PHONE_RESULT_BUFF_LONG+1]={0};
    char buff[PHONE_RESULT_BUFF_LONG+1]={0};
    int tmp;
    
    if (!pdata)
        return;

    get_word(pdata->str, index, buff_translate, PHONE_RESULT_BUFF_LONG); 
    /*memset(buff, 0, sizeof(buff));*/
    tmp = pdata->cur_method->translate_word((void *)pdata->cur_method, buff_translate, 
            buff, PHONE_RESULT_BUFF_LONG, cursor);
    if (tmp == -1 && strlen(buff) < 1){
        pdata->str_down[0]='\0';
        return;
    }
    strncpy(pdata->str_down, buff, sizeof(pdata->str_down));
    pdata->str_down_num = get_words_num(pdata->str_down);
}

static void handle_predict_word(HWND hwnd, int cursor)
{
    char buff[PHONE_RESULT_BUFF_LONG+1]={0};
    int next, prev, tmp;
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    
    /* get predict word */
    //strcpy(pdata->str, pdata->cur_word);

    if ( !pdata->cur_method->predict_word || 
        strlen(pdata->str) <= 0 ||
        cursor == -1)
        return;

    //memset(buff, 0, sizeof(buff));
    prev = 0;
    /* Get prev cursor */
    if(pdata->cur_cursor_down != 0) {
        while(prev != -1) {
            tmp = pdata->cur_method->predict_word((void *)pdata->cur_method, 
                        pdata->str, buff, PHONE_RESULT_BUFF_LONG, prev);
            //fprintf(stderr, "predict 0, next=%d\n", tmp);
            if (tmp == cursor)
                break;
            prev = tmp;
        }
    }

    /* Get next cursor and get matched buff */
    next = pdata->cur_method->predict_word((void *)pdata->cur_method, 
                pdata->str, buff, PHONE_RESULT_BUFF_LONG, cursor);
    if (next == -1 && strlen(buff) < 1){
        pdata->phone_input_status = PHONE_NO_INPUT_STATUS;
        phone_adddata_init(pdata);
        return;
    }

    /* Update data */
    if (prev < 0)
        prev = 0;
    pdata->prev_cursor_down = prev;
    pdata->cur_cursor_down = cursor;

    strncpy(pdata->str_down, buff, PHONE_RESULT_BUFF_LONG);
    pdata->str_down_num = get_words_num(pdata->str_down);
    pdata->next_cursor_down = next;

    if(pdata->str_down_num > 0) {
        pdata->is_focus_up = FALSE;
        pdata->cur_index_down = 0;
    }

    update_state(hwnd);
}

static void handle_translate_word(HWND hwnd, int index, int cursor)
{
    md_debug("handle translate word \n");
    char buff[PHONE_RESULT_BUFF_LONG+1]={0};
    char buff_translate[PHONE_RESULT_BUFF_LONG+1]={0};
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    int next, prev, tmp;
    
    if (cursor == -1)
        return;

    /* get translate word */
    if ( !pdata->cur_method->translate_word || strlen(pdata->str) <= 0)
        return;

    get_word(pdata->str, index, buff_translate, PHONE_RESULT_BUFF_LONG); 

    prev = 0;
    /* Get prev cursor */
    if(pdata->cur_cursor_down != 0) {
        while(prev != -1) {
            if (pdata->cur_method->translate_word) {
                tmp = pdata->cur_method->translate_word((void *)pdata->cur_method, 
                        buff_translate, buff, PHONE_RESULT_BUFF_LONG, prev);
                if (tmp == cursor)
                    break;
                prev = tmp;
            }
        }
    }

    /* Get next cursor and get matched buff */
    next = pdata->cur_method->translate_word((void *)pdata->cur_method, 
                buff_translate, buff, PHONE_RESULT_BUFF_LONG, cursor);
    if (next == -1 && strlen(buff) < 1){
        fprintf(stderr, "handle_translate_data next is NULL, return\n");
        return;
    }

    /* Update data */
    if (prev == -1)
        prev = 0;
    pdata->prev_cursor_down = prev;
    pdata->cur_cursor_down = cursor;
    strncpy(pdata->str_down, buff, PHONE_RESULT_BUFF_LONG);
    pdata->str_down_num = get_words_num(pdata->str_down);

    if (next != -1) {
        tmp = pdata->cur_method->translate_word((void *)pdata->cur_method, 
                buff_translate, buff, PHONE_RESULT_BUFF_LONG, next);
        if (tmp == -1 && strlen(buff) < 1)
            pdata->next_cursor_down = tmp;
        else
            pdata->next_cursor_down = next;
    }
    else
        pdata->next_cursor_down = next;

    if (pdata->cur_index_down > pdata->str_down_num - 1)
        pdata->cur_index_down = pdata->str_down_num - 1;

    update_state(hwnd);
}

static void  handle_data (HWND hwnd, const char *stokes, int cursor)
{
    char buff[PHONE_RESULT_BUFF_LONG+1]={0};
    int next, prev, tmp;
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);

    if (cursor == -1 || stokes == NULL)
        return;
    if (pdata->cur_method == NULL)
        return;

    /* Get prev cursor */
    prev = 0;

    if (cursor != 0) {
        while (prev != -1) {
            if (pdata->cur_method->match_keystrokes) {
                tmp = pdata->cur_method->match_keystrokes((void *)pdata->cur_method, stokes, buff, PHONE_RESULT_BUFF_LONG, prev);
                if (tmp == cursor) 
                    break;
                prev = tmp;
            }
        }
    }

    /* Get next cursor and get matched buff */
    next = pdata->cur_method->match_keystrokes((void *)pdata->cur_method, stokes, buff, PHONE_RESULT_BUFF_LONG, cursor);
    if (next == -1 && strlen(buff) < 1)
        return;

    /* Update data */
    if (prev == -1)
        prev = 0;

    pdata->prev_cursor = prev;
    pdata->cur_cursor = cursor;

    strncpy(pdata->key_strokes ,stokes, sizeof(pdata->key_strokes));
    strncpy(pdata->str, buff, PHONE_RESULT_BUFF_LONG);
    pdata->str_num = get_words_num(pdata->str);
#if 1
    if (next != -1) {
        tmp = pdata->cur_method->match_keystrokes((void *)pdata->cur_method, stokes, buff, PHONE_RESULT_BUFF_LONG, next);
        if (tmp == -1 && strlen(buff) < 1) {
            //fprintf(stderr, "handle_data now have no value\n");
            pdata->next_cursor = tmp;
        }
        else
            pdata->next_cursor = next;
    }
    else
        pdata->next_cursor = next;
#else
    pdata->next_cursor = next;
#endif

    if (pdata->cur_index > pdata->str_num - 1)
        pdata->cur_index = pdata->str_num - 1;

    update_state(hwnd);
#if 0 
    /* get translate word */
    get_translate_word(pdata, pdata->cur_index, 0);
#endif
}

static int get_special_symbol(MGI_PHONE_IME_DATA pdata, const char *sym)
{
    const char *pletters;
    char * tmp;
    int i=0;

    if (!pdata)
        return -1;

    pletters=sym;
    if (!pletters)
        return -1;

    tmp = pdata->symbol;
    while (pletters && !isdigit(*pletters))
    {
       tmp[i++] = *pletters++;
       //tmp[i++] = ' ';
       if (i >= MAX_LEN_SYMBOL)
           break;
    }
#ifdef MDTV_MGPHONE_DEBUG
    md_debug ("input symbol: %s\n", tmp);
#endif
    return i;
}

static int get_default_symbol(MGI_PHONE_IME_DATA pdata)
{
    const char *pletters;
    char * tmp;
    int i=0;

    if (!pdata)
        return -1;

    pletters=((*pdata->key_map)[MGI_PHONE_KEY_1]).letters;
    if (!pletters)
        return -1;

    tmp = pdata->symbol;
    while (pletters && !isdigit(*pletters))
    {
       tmp[i++] = *pletters++;
       tmp[i++] = ' ';
       if (i >= MAX_LEN_SYMBOL)
           break;
    }
    return i;
}

static int copy_word_from_source (const char* source, char* buff, int copied)
{
    int len;
    char* space;

    while (source) {
        space = strchr (source, ' ');
        if (space == NULL) {
            len = strlen (source) + 1;
            if (len == 1)
                return copied;

            if (copied + len < PHONE_RESULT_BUFF_LONG) {
                strcpy (buff + copied, source);
                copied += len;
                buff [copied - 1] = ' ';
            }
            return copied;
        }
        else {
            len = space - source + 1;
            if (len == 1)
                return copied;

            if (copied + len < PHONE_RESULT_BUFF_LONG) {
                strncpy (buff + copied, source, len);
                copied += len;
                source += len;
            }
            else
                return copied;
        }
    }
    return copied;
}

/*
 *handle symbol
 * */
void handle_symbol_data(MGI_PHONE_IME_DATA pdata, int cursor)
{
    md_debug("handle symbol data\n");
    char *pstr=NULL, *cur, *prev;
    char tmp[2][PHONE_RESULT_BUFF_LONG+1];
    int len, up_len;
    /*int down_len;*/

    cur = (char *)cursor;
    if (cur <= 0)
        cur = pdata->symbol;
    else if ( cur < pdata->symbol || 
              cur > pdata->symbol + sizeof(pdata->symbol)) 
    {
        fprintf(stderr, "handle_symbol_data, cur is too large\n");
        return;
    }

    prev = pdata->symbol;
    if (prev != cur) {
        int len[2]={0};
        while (prev < (pdata->symbol+strlen(pdata->symbol))) {
            len[0] = copy_word_from_source(prev, tmp[0], 0);
            len[1] = copy_word_from_source(prev+len[0], tmp[1], 0);

            if (prev+len[0]+len[1] >= cur)
                break;
            prev += len[0]+len[1];
        }
    }

    pstr = cur;
    up_len = strlen(pdata->str);
    /*memset(pdata->str, 0, sizeof(pdata->str));*/
    pdata->str[0] = '\0';
    len = copy_word_from_source(pstr, pdata->str, 0);

    /*down_len = strlen(pdata->str_down);*/

    /*memset(pdata->str_down, 0, sizeof(pdata->str_down));*/
    pdata->str_down[0] = '\0';
    pstr += len;
    if ( pstr < pdata->symbol + sizeof(pdata->symbol))
        len = copy_word_from_source(pstr, pdata->str_down, 0);

    pdata->str_num = get_words_num(pdata->str);
    pdata->str_down_num = get_words_num(pdata->str_down);
    if (pdata->is_focus_up) {
        if (pdata->cur_index > pdata->str_num -1)
            pdata->cur_index = pdata->str_num -1;
    }
    else {
        if (pdata->cur_index_down > pdata->str_down_num -1)
            pdata->cur_index_down = pdata->str_down_num -1;
    }

    pdata->prev_cursor = (int)prev;
    if (pdata->prev_cursor < (int)pdata->symbol )
        pdata->prev_cursor = (int)pdata->symbol;

    pdata->cur_cursor = (int)cur;
    pdata->next_cursor = (int)(pstr + len);

    if ( pdata->next_cursor >=(int)(pdata->symbol + strlen(pdata->symbol)))
        pdata->next_cursor = -1;
}

static void show_prev_page(HWND hwnd)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    int cursor;

    if (pdata->is_focus_up ||
        pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS)
        cursor = pdata->prev_cursor;
    else
        cursor = pdata->prev_cursor_down;

    if(pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) {
        pdata->is_focus_up = FALSE;
        pdata->cur_index_down = pdata->cur_index;
        handle_symbol_data(pdata, cursor);
    }
    else if (pdata->phone_input_status == PHONE_PRED_INPUT_STATUS)
        handle_predict_word(hwnd, cursor);
    else 
        handle_translate_word(hwnd, pdata->cur_index, cursor);
}

static void show_next_page(HWND hwnd)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    int cursor;

    if (pdata->is_focus_up ||
        pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS)
        cursor = pdata->next_cursor;
    else
        cursor = pdata->next_cursor_down;

    if(pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) {
        if (cursor == -1)
            return;
        pdata->is_focus_up = TRUE;
        pdata->cur_index = pdata->cur_index_down;
        handle_symbol_data(pdata, cursor);
    }
    else if (pdata->phone_input_status == PHONE_PRED_INPUT_STATUS)
        handle_predict_word(hwnd, cursor);
    else 
        handle_translate_word(hwnd, pdata->cur_index, cursor);
}


int get_scancode_index(MGI_PHONE_IME_DATA pdata, int scancode)
{
    int i;
    for (i = 0;i < MAX_PHONE_KEY_NUM; i++) {
        if ( ( *(pdata->key_map) )[i].scancode == scancode )
        {
            return i;
        }
    }
    return -1;
}

static int 
process_en_num_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);

    int index = get_scancode_index(pdata, LOWORD(wParam));
    if (message == MSG_KEYDOWN) {
        /* Handle the key 1 for punctuation marks and special characters*/
        if (index == MGI_PHONE_KEY_1){
                md_debug ("switch symbol status 3333333\n");
                output_phone_input_status (pdata->phone_input_status);
            if (pdata->phone_input_status == PHONE_NO_INPUT_STATUS ||
                 pdata->phone_input_status == PHONE_PRED_INPUT_STATUS) 
            {
                md_debug ("switch symbol status\n");
                phone_adddata_init(pdata);
                pdata->phone_input_status = PHONE_SYMBOL_INPUT_STATUS;
                pdata->cur_cursor = 0;

                if (pdata->ptim_case >= PHONE_DEFAULT_CASE_MAX && 
                    pdata->cur_method->get_symbol){
                    pdata->cur_method->get_symbol(pdata, pdata->symbol, MAX_LEN_SYMBOL);
                }
                else
                    get_default_symbol(pdata);

                handle_symbol_data(pdata, pdata->cur_cursor);
                update_state(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
#if 0
        else if (index == MGI_PHONE_KEY_0) {
            /* Handle the key 0*/
            char *cur_word = pdata->cur_word;
            int nr_word = strlen (cur_word);

            /*send word and send space key*/ 
            if (nr_word == 0) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
                Send2ActiveWindow (mgTopmostLayer, MSG_CHAR, ' ', 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
                PostMessage(pdata->sg_target_hwnd, MSG_CHAR, ' ', lParam|KS_IMEPOST);
#endif
            }
            else if ( ! pdata->is_focus_up ||
                    (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS)) {
                /*send word to target window*/
                send_word (pdata, cur_word);
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
                Send2ActiveWindow (mgTopmostLayer, MSG_CHAR, ' ', 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
                PostMessage(pdata->sg_target_hwnd, MSG_CHAR, ' ', lParam|KS_IMEPOST);
#endif
                phone_adddata_init (pdata);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
#endif
        else if (index >= MGI_PHONE_KEY_2 && index <= MGI_PHONE_KEY_9) {
            char c[2];
            char stokes[MAX_LEN_KEYSTROKES+1];

            if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS)
                return 1;
            else if(pdata->phone_input_status == PHONE_PRED_INPUT_STATUS)
                phone_adddata_init(pdata);

            pdata->phone_input_status=PHONE_CHAR_INPUT_STATUS;

            /*c[0] = wParam - 1 + '0';*/
            c[0] = index - MGI_PHONE_KEY_0 + '0';
            c[1] = '\0';
            if (strlen(pdata->key_strokes) < MAX_LEN_KEYSTROKES) {
                strcpy(stokes, pdata->key_strokes);
                strcat (stokes, c);
            }
            else
                return 1;

            pdata->is_focus_up = TRUE;
            pdata->cur_index = 0;
            handle_data(hwnd, stokes, 0);
            //fprintf(stderr, "handle_data over\n");

            /* get translate word */
            get_translate_word(pdata, pdata->cur_index, 0);
            //fprintf(stderr, "get_translate_word over\n");

            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
    return 0;
}

static void process_left_key(HWND hwnd, MGI_PHONE_IME_DATA pdata)
{
    md_debug("left key entered\n");
    char cur_word[64];

    if (pdata->is_focus_up) {
        if (pdata->cur_index <= 0) {
            if( pdata->prev_cursor != pdata->cur_cursor &&
                pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS) 
            {
                handle_data(hwnd, pdata->key_strokes, pdata->prev_cursor);
                pdata->cur_index = pdata->str_num-1;
                if (pdata->cur_index < 0)
                    pdata->cur_index = 0;

                /* get translate word */
                get_translate_word(pdata, pdata->cur_index, 0);
            }
            return;
        }
    }
    else {
        if (pdata->cur_index_down <= 0 && pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
            return;
        md_debug ("3321232323 %d\n", pdata->cur_index_down);
        if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS && pdata->cur_index_down <= 0) {
            pdata->is_focus_up = TRUE;
            if ( strlen(pdata->str) <= 0) {
                pdata->is_focus_up = FALSE;
                return;
            }
            pdata->cur_index = pdata->str_num - 1;
            return ;
        }
    }

    if (pdata->is_focus_up){
        pdata->cur_index--;
        get_word(pdata->str, pdata->cur_index, cur_word, 64);
    }
    else{
        pdata->cur_index_down--;
        get_word(pdata->str_down, pdata->cur_index_down, cur_word, 64);

    }

    if (pdata->cur_word) {
        strcpy (pdata->cur_word, cur_word);
        get_cur_word_rect(hwnd);
    }
    /* Update arraw state */
    if (pdata->is_focus_up && pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        get_translate_word(pdata, pdata->cur_index, 0);
#if 0
    if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        update_state(hwnd);
#endif
}

static void process_right_key(HWND hwnd, MGI_PHONE_IME_DATA pdata)
{
    if (pdata->is_focus_up && pdata->cur_index >= pdata->str_num - 1) 
    {
        if( pdata->next_cursor != -1 && 
            pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        {/* in up line, right key can page possible*/
            //fmd_debug(stderr, "right key, page\n");
            handle_data(hwnd, pdata->key_strokes, pdata->next_cursor);
            pdata->cur_index = 0;

            /* get translate word */
            get_translate_word(pdata, pdata->cur_index, 0);
        }

        if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) {
            pdata->is_focus_up = FALSE;
            if ( strlen(pdata->str_down) <= 0) {
                pdata->is_focus_up = TRUE;
                return;
            }
            pdata->str_down_num = get_words_num(pdata->str_down);
            pdata->cur_index_down = 0;

        }

        return;
    }
    else if (!pdata->is_focus_up && pdata->cur_index_down >= pdata->str_down_num -1)
        return;
    else {
        char cur_word[64];
        if (pdata->is_focus_up){
            pdata->cur_index++;
            get_word(pdata->str, pdata->cur_index, cur_word, 64);
        }
        else{
            pdata->cur_index_down++;
            get_word(pdata->str_down, pdata->cur_index_down, cur_word, 64);
        }

        if (pdata->cur_word) {
            strcpy (pdata->cur_word, cur_word);
            get_cur_word_rect(hwnd);
        }
    }
    /* Update arraw state */
    if (pdata->is_focus_up && pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        get_translate_word(pdata, pdata->cur_index, 0);
#if 0
    if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        update_state(hwnd);
#endif
}

static void process_up_key(HWND hwnd, MGI_PHONE_IME_DATA pdata)
{
    if (pdata->is_focus_up) {
        /* symbol input */
        if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS &&
            pdata->prev_cursor != pdata->cur_cursor)
            show_prev_page(hwnd);
    }
    else {
        if(pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) 
        {/*when input symbol*/
            pdata->is_focus_up = TRUE;
            pdata->cur_index = pdata->cur_index_down;
        }
        else {
            if (pdata->prev_cursor_down == pdata->cur_cursor_down)
            {/*the beginning of content in down line, now switch focus to up line*/

                /* when predict input, focus can not move to up*/
                if (pdata->phone_input_status == PHONE_PRED_INPUT_STATUS) {
                    return;
                }
                pdata->is_focus_up = TRUE;
                pdata->cur_index_down = 0;
            }
            else
                show_prev_page(hwnd);
        }
    }
    if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        update_state(hwnd);
}

/* handle down key */
static void process_down_key(HWND hwnd, MGI_PHONE_IME_DATA pdata)
{
    md_debug("down key\n");
    if (pdata->is_focus_up) {
        pdata->is_focus_up = FALSE;

        /* handle symbol input */
        if(pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) {
           if ( strlen(pdata->str_down) <= 0) {
                pdata->is_focus_up = TRUE;
                return;
           }
           pdata->str_down_num = get_words_num(pdata->str_down);
           if ((unsigned int)pdata->cur_index > strlen(pdata->str_down))
               pdata->cur_index_down = pdata->str_down_num -1;
           else
               pdata->cur_index_down = pdata->cur_index;
        }
        else {/* handle char input */
            pdata->cur_index_down = 0;
            handle_translate_word(hwnd, pdata->cur_index, 0);
        }
    }
    else /* focus in down line */
        show_next_page(hwnd);
#if 0
    if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
        update_state(hwnd);
#endif
}

static int 
process_navigator_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    md_debug("navigator key\n");
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);

    if ( pdata->phone_input_status == PHONE_NO_INPUT_STATUS) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);

#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(pdata->sg_target_hwnd, message, wParam, lParam);
#endif
    md_debug("PHONE_NO_INPUT_STATUS\n");
        return 0;
    }
    else if (message == MSG_KEYDOWN) {
        int index = get_scancode_index(pdata, LOWORD(wParam));
        switch (index) {
            case MGI_PHONE_KEY_LEFT:
                process_left_key(hwnd, pdata);
                break;
            case MGI_PHONE_KEY_RIGHT:
                process_right_key(hwnd, pdata);
                break;
            case MGI_PHONE_KEY_UP:
                process_up_key(hwnd, pdata);
                break;
            case MGI_PHONE_KEY_DOWN:
                process_down_key(hwnd, pdata);
                break;
        }
#if 0
        if(pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS)
            update_state(hwnd);
#else
        update_state(hwnd);
#endif
        InvalidateRect(hwnd, NULL, TRUE);
        // for mdolphin not to close ime window when exceeding selected time while selecting word on ime window.
        PostMessage(pdata->sg_target_hwnd, MSG_IME_CMD_KEYDOWN, wParam, lParam);
    }
    return 0;
}

static int 
process_confirm_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    char *cur_word = pdata->cur_word;

    if (pdata->phone_input_status == PHONE_NO_INPUT_STATUS) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(pdata->sg_target_hwnd, message, wParam, lParam);
#endif
        return 0;
    }
    else if (pdata->phone_input_status == PHONE_SYMBOL_INPUT_STATUS) {
        send_word(pdata, cur_word);
        phone_adddata_init (pdata);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    else {
        if (pdata->is_focus_up){
            /*get next level word list*/
            pdata->is_focus_up = FALSE;
            pdata->cur_index_down = 0;
            handle_translate_word(hwnd, pdata->cur_index, 0);
        }
        else {
            send_word(pdata, cur_word);
            if (pdata->cur_method && 
                pdata->cur_method->predict_word)
            {
                pdata->phone_input_status = PHONE_PRED_INPUT_STATUS;
                strncpy(pdata->str, pdata->cur_word, sizeof(pdata->str));
                handle_predict_word(hwnd, 0);
            }
            else 
                phone_adddata_init (pdata);
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    return 0;
}

static int process_quit_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    md_debug(" process_quit_key() \n");
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
    if( !pdata || pdata->sg_target_hwnd == HWND_INVALID ){
        return -1;
    }
    PostMessage(hwnd, MSG_IME_CMD_CLOSE, wParam, lParam);
    return 0;
}
static int process_clear_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    md_debug("clear key entered\n");
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
    if( !pdata ){
        return -1;
    }

    if (pdata->phone_input_status == PHONE_NO_INPUT_STATUS) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(pdata->sg_target_hwnd, message, wParam, lParam);
#endif
        return 0;
    }
    else if (message == MSG_KEYDOWN) {
        int len = strlen(pdata->key_strokes);
        if ( pdata->phone_input_status != PHONE_PRED_INPUT_STATUS &&
             len > 1 ) 
        {
            if (pdata->old_strokes[0]) {
                handle_data(hwnd, pdata->old_strokes, 0);
                memset (pdata->old_strokes, 0, MAX_LEN_KEYSTROKES);
            }
            else {
                pdata->key_strokes[len -1] = '\0';
                handle_data(hwnd, pdata->key_strokes, 0);
            }
            pdata->is_focus_up = TRUE;
            pdata->cur_index = 0;

            /* get translate word */
            get_translate_word(pdata, pdata->cur_index, 0);
        }
        else 
        {
            phone_adddata_init (pdata);
        }
        InvalidateRect (hwnd, 0, TRUE);
    }
    return 0;
}

static int 
process_sharp_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    phone_switch_mode (pdata);

    if(pdata->cb_notify) {
        int method_id;
        if (pdata->ptim_case < PHONE_DEFAULT_CASE_MAX){
            method_id = pdata->ptim_case;                    
            md_debug("---- process_sharp_key(): method_id = pdata->ptim_case=%d\n", method_id);
        }
        else {
            if (pdata->cur_method)
                method_id = pdata->cur_method->method_id;
            else {
                fprintf(stderr, "process_sharp_key error\n");
                return -1;
            }
            md_debug("---- process_sharp_key(): method_id = pdata->cur_method->method_id=%d\n", method_id);
        }
        pdata->cb_notify((void *)pdata, method_id);
    }
    if(pdata->sg_target_hwnd != HWND_INVALID){
        PostMessage(pdata->sg_target_hwnd, MSG_IME_CMD_KEYDOWN, wParam, lParam);
    }
    return 0;
}

static int 
process_star_key (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    const char *sym;
    static int sf_sym_index = 0;

#if 0
    if (message == MSG_KEYDOWN) {
        if (pdata->cur_method &&
            pdata->cur_method->status_changed)
        {
            pdata->cur_method->status_changed((void *)pdata, 0);
            phone_adddata_init (pdata);
            InvalidateRect(hwnd, NULL, TRUE);
        }
    }
#else
    if (message == MSG_KEYDOWN) {
        md_debug ("enter star \n");
        if (pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS){
            phone_adddata_init(pdata);
            sf_sym_index = 0;
            sym = punctuate_table[sf_sym_index];
            pdata->phone_input_status = PHONE_SYMBOL_INPUT_STATUS;
        }
        else {
            phone_adddata_init(pdata);
            sf_sym_index = ((sf_sym_index + 1)%TABLESIZE(punctuate_table));
            sym = punctuate_table[sf_sym_index];
            pdata->phone_input_status = PHONE_SYMBOL_INPUT_STATUS;
        }
        pdata->cur_cursor = 0;
        get_special_symbol (pdata, sym);
        handle_symbol_data(pdata, pdata->cur_cursor);
        update_state(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        if(pdata->sg_target_hwnd != HWND_INVALID){
            PostMessage(pdata->sg_target_hwnd, MSG_IME_CMD_KEYDOWN, wParam, lParam);
        }
    }

#endif
    return 0;
}
#if 0
static void draw_nav_bmp (HWND hwnd, HDC hdc, void *data)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) data;
    int bw = 0, bh = 0, bmp_nr = 15;
    int y=0;
    RECT cli_rc;
    
    GetClientRect (hwnd, &cli_rc);

    if (!pdata->is_focus_up )
        y = RECTH(cli_rc)/2;
#if 0
    fprintf(stderr, "left=%d, top=%d, right=%d, bottom=%d\n", 
            pdata->is_left, pdata->is_top, pdata->is_right, pdata->is_bottom);
#endif
    bw = ime_nav_bkg.bmWidth/bmp_nr;
    bh = ime_nav_bkg.bmHeight;
    /*left navigator:9*/
    if (pdata->is_left) {
        FillBoxWithBitmapPart (hdc, 
                0, y, bw, bh, 
                0, 0, &ime_nav_bkg, 9 * bw, 0);
    } 

    /*top-right-bottom navigator:2*/
    if (pdata->is_right && pdata->is_top && pdata->is_bottom) {
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 2 * bw, 0);
        return;
    }
    else if (pdata->is_right && pdata->is_top ) {
        /*top-right navigator:6*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 6 * bw, 0);
        return;
    }
    else if (pdata->is_right && pdata->is_bottom) {
        /*right-bottom navigator:5*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 5 * bw, 0);
        return;
    }
    else if (pdata->is_right) {
        /*right navigator:10*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 10 * bw, 0);
        return;
    }
    else if (pdata->is_top && pdata->is_bottom) {
        /*top-bottom navigator:14*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 14 * bw, 0);
        return;
    }
    else if (pdata->is_top) {
        /*top navigator:11*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 11 * bw, 0);
        return;
    }
    else if (pdata->is_bottom) {
        /*bottom navigator:12*/
        //GetClientRect (hwnd, &cli_rc);
        FillBoxWithBitmapPart (hdc, 
                cli_rc.right - bw, y, bw, bh, 
                0, 0, &ime_nav_bkg, 12 * bw, 0);
        
        return;
    }
}
#endif

/* internal functions */
static void refresh_input_method_area (HWND hwnd, HDC hdc)
{
    md_debug("refresh_input_method_area\n");
    static RECT rc;
    RECT clientrc, tmprc;
    Uint32 prev_color=0;
    int off_x = 0;
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA) GetWindowAdditionalData(hwnd);
    if ( pdata->str[0] == 0)
    {
        md_debug("empty rect\n");
        //ShowWindow(hwnd, SW_HIDE);
        //return;
        SetRectEmpty (&rc);
        SetRectEmpty (&pdata->cur_rect);
#if 0
        if ( (first == TRUE) || !pdata->is_opened)
        {
            ShowWindow (hwnd, SW_HIDE);
            first = FALSE;
            return;
        }
#endif
    }
#if 0
    SetBkMode (hdc, BM_TRANSPARENT);
    int height = GetGDCapability (hdc, GDCAP_MAXY) + 1;
        
    if(pdata->ptim_case < PHONE_DEFAULT_CASE_MAX) {
        md_debug("draw en bkg\n");
        FillBoxWithBitmap(hdc, 0, height/2, 0, height/2, &ime_en_bkg);
    } 
    else if ((pdata->str_down[0] == '\0') && (pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS))
    {
        md_debug("draw ch bkg\n");
        FillBoxWithBitmap(hdc, 0, height/2, 0, height/2, &ime_ch_bkg);
    }
#else
    if (pdata->hgs)
    {

        //GetWindowRect(hwnd, &clientrc);
        //BitBlt (HDC_SCREEN, clientrc.left, clientrc.top, RECTW(clientrc), RECTH(clientrc), hdc, 0, 0, -1);
        HDC memdc = MGPlusGetGraphicDC (pdata->hgs);
        SetMemDCColorKey (memdc, MEMDC_FLAG_SRCCOLORKEY, RGBA2Pixel (memdc, 
                          GetRValue (INPUT_BRACKGROUND_COLORKEY),
                          GetGValue (INPUT_BRACKGROUND_COLORKEY),
                          GetBValue (INPUT_BRACKGROUND_COLORKEY),
                          GetAValue (INPUT_BRACKGROUND_COLORKEY)
                          ));
        BitBlt (memdc, 0, 0, 0, 0, hdc, 0, 0, -1);
    }
#endif

    handle_add_data (hwnd, pdata);

#if 0
    /*clear previous drawing rect*/
    if (!IsRectEmpty (&rc)) {
        md_debug("rc is not empty\n");
#if 0
        prev_color = SetPenColor (hdc, GetWindowBkColor (hwnd));
        Rectangle (hdc, rc.left, rc.top, rc.right, rc.bottom);
#else
        prev_color = SetBrushColor(hdc, GetWindowBkColor(hwnd));
        FillBox(hdc, rc.left, rc.top, RECTW(rc), RECTH(rc));
#endif
        SetPenColor (hdc, prev_color);
    }
#endif

    /*draw navigator key*/
    //draw_nav_bmp (hwnd, hdc, pdata);
    if (!IsRectEmpty (&pdata->cur_rect)) {

        md_debug("cur rect is not empty\n");
        md_debug("cur-rc: [%d, %d, %d, %d]\n", pdata->cur_rect.left, pdata->cur_rect.top, 
                pdata->cur_rect.right, pdata->cur_rect.bottom);
        CopyRect (&rc, &pdata->cur_rect);
        {
            off_x = ime_nav_bkg.bmWidth/30;
            OffsetRect (&rc, off_x, 0);
        }
        /*offset current rect*/
        //if (pdata->is_left || pdata->is_top)
        md_debug("pdata->cur_index_down %d\n", pdata->cur_index_down);
    }
#if 0
    if (!IsRectEmpty (&pdata->cur_rect)) {

        md_debug("cur rect is not empty\n");
        md_debug("cur-rc: [%d, %d, %d, %d]\n", pdata->cur_rect.left, pdata->cur_rect.top, 
                pdata->cur_rect.right, pdata->cur_rect.bottom);
        CopyRect (&rc, &pdata->cur_rect);
        /*offset current rect*/
        //if (pdata->is_left || pdata->is_top)
        md_debug("pdata->cur_index_down %d\n", pdata->cur_index_down);
        {
            off_x = ime_nav_bkg.bmWidth/30;
            OffsetRect (&rc, off_x, 0);
        }

        /*draw current rect*/
#if 0
        prev_color = SetPenColor (hdc, PIXEL_black);
        Rectangle (hdc, rc.left, rc.top, rc.right, rc.bottom);
        SetPenColor (hdc, prev_color);
#else
        if (pdata->text_color) {
            if (pdata->is_focus_up)
                prev_color = SetBrushColor(hdc, pdata->text_color->firstline_focus_color);
            else
                prev_color = SetBrushColor(hdc, pdata->text_color->secondline_focus_color);
        }
        else
            prev_color = SetBrushColor(hdc, PIXEL_darkblue);
        
            //off_x = ime_nav_bkg.bmWidth/30;

        FillBox(hdc, rc.left, rc.top, RECTW(rc), RECTH(rc));
        SetBrushColor(hdc, prev_color);
#endif
    }
#endif

    SetBkMode (hdc, BM_TRANSPARENT);
    /* fillbox focus region*/
    GetClientRect(hwnd, &clientrc);
    OffsetRect(&clientrc, ime_nav_bkg.bmWidth/30, 0);
    SetRect(&tmprc, clientrc.left, clientrc.top, 
                    clientrc.right - 2 * ime_nav_bkg.bmWidth/30, 
                    clientrc.top+RECTH(clientrc)/2);

    /* draw first line text*/
    if (pdata->text_color)
        prev_color = SetTextColor(hdc, RGB2Pixel (hdc,
                    GetRValue (pdata->text_color->firstline_text_color),
                    GetGValue (pdata->text_color->firstline_text_color),
                    GetBValue (pdata->text_color->firstline_text_color)
                    ));

    DrawText(hdc, pdata->str, -1, &tmprc, DT_NOCLIP|DT_SINGLELINE|DT_LEFT);
    if (pdata->text_color)
        SetTextColor(hdc, prev_color);

    /* draw second line text*/
    if (strlen(pdata->str_down)) { 
        if (pdata->text_color)
            prev_color = SetTextColor(hdc, RGB2Pixel(hdc, 
                        GetRValue (pdata->text_color->secondline_text_color),
                        GetGValue (pdata->text_color->secondline_text_color),
                        GetBValue (pdata->text_color->secondline_text_color)
                        ));
        OffsetRect(&tmprc, 0, RECTH(clientrc)/2);
        DrawText(hdc, pdata->str_down, -1, &tmprc, DT_NOCLIP|DT_SINGLELINE|DT_LEFT);
        
        if (pdata->text_color)
            SetTextColor(hdc, prev_color);
    }
    md_debug("cur word:%s\n", pdata->cur_word);
    if (pdata->cur_word == NULL || (pdata->cur_index_down <0 && pdata->cur_index <0) )
        return;

    if (pdata->text_color)
    {
        if (pdata->is_focus_up)
        {
             prev_color = SetTextColor(hdc, RGB2Pixel (hdc, 
                      GetRValue (pdata->text_color->firstline_focus_text_color), 
                      GetGValue (pdata->text_color->firstline_focus_text_color),
                      GetBValue (pdata->text_color->firstline_focus_text_color)
                      ));
        }
        else
            prev_color = SetTextColor(hdc, RGB2Pixel (hdc, 
                      GetRValue (pdata->text_color->secondline_focus_text_color), 
                      GetGValue (pdata->text_color->secondline_focus_text_color),
                      GetBValue (pdata->text_color->secondline_focus_text_color)
                      ));
    }
    else
        prev_color = SetTextColor(hdc, PIXEL_lightwhite);

    if (pdata->hgs && !IsRectEmpty (&rc) )  // draw focus background
    {
        HDC memdc = MGPlusGetGraphicDC (pdata->hgs);
        SetMemDCColorKey (memdc, MEMDC_FLAG_SRCCOLORKEY, RGBA2Pixel (memdc, 
                          GetRValue (INPUT_BRACKGROUND_COLORKEY),
                          GetGValue (INPUT_BRACKGROUND_COLORKEY),
                          GetBValue (INPUT_BRACKGROUND_COLORKEY),
                          GetAValue (INPUT_BRACKGROUND_COLORKEY)
                          ));
        BitBlt (memdc, rc.left, rc.top, RECTW(rc), RECTH(rc), hdc, rc.left, rc.top, -1);
    }
    DrawText(hdc, pdata->cur_word, -1, &rc, DT_NOCLIP|DT_SINGLELINE|DT_LEFT);
    SetTextColor(hdc, prev_color);

    // draw arrow
    if (pdata->ptim_case >= PHONE_DEFAULT_CASE_MAX ) {

        if (pdata->cur_cursor_down >  pdata->prev_cursor_down)
            FillBoxWithBitmap(hdc, ime_arrow_pos[0].left, ime_arrow_pos[0].top, 0, 0, &ime_enable_left_arrow);
        else
            FillBoxWithBitmap(hdc, ime_arrow_pos[0].left, ime_arrow_pos[0].top, 0, 0, &ime_disable_left_arrow);

        if (pdata->next_cursor_down > pdata->cur_cursor_down)
            FillBoxWithBitmap(hdc, ime_arrow_pos[1].left, ime_arrow_pos[1].top, 0, 0, &ime_enable_right_arrow);
        else
            FillBoxWithBitmap(hdc, ime_arrow_pos[1].left, ime_arrow_pos[1].top, 0, 0, &ime_disable_right_arrow);
    }
}

void DrawBackgroundWithMGPlus (HGRAPHICS hgs, int width, int height)
{
    int pen_width = -1;
    int radius = 10;
    RECT drawrect = {0, 0, width, height};
    ARGB color [3];
    HBRUSH brush;
    HPEN hpen;

    HDC memdc = MGPlusGetGraphicDC (hgs);
    SetBrushColor (memdc, RGBA2Pixel (memdc, 
                          GetRValue (INPUT_BRACKGROUND_COLORKEY),
                          GetGValue (INPUT_BRACKGROUND_COLORKEY),
                          GetBValue (INPUT_BRACKGROUND_COLORKEY),
                          GetAValue (INPUT_BRACKGROUND_COLORKEY)
                          ));
    FillBox (memdc, drawrect.left, drawrect.top, RECTW(drawrect), RECTH(drawrect));

    brush = MGPlusBrushCreate (MP_BRUSH_TYPE_LINEARGRADIENT);
    MGPlusSetLinearGradientBrushMode (brush, MP_LINEAR_GRADIENT_MODE_HORIZONTAL);
    color[0] = (0x616060 | (0xFF << 24) ); 
    color[1] = (0x36353a | (0xFF << 24) ); 
    color[2] = (0x2e2d35 | (0xFF << 24) ); 

    MGPlusLinearGradientBrushAddColor (brush, color[0], 0.0);
    MGPlusLinearGradientBrushAddColor (brush, color[1], (float)(height/2.0));
    MGPlusLinearGradientBrushAddColor (brush, color[2], 1.0);

    MGPlusSetLinearGradientBrushRect (brush, &drawrect);
    MGPlusFillRoundRectI (hgs, brush, drawrect.left , drawrect.top, RECTW(drawrect),  RECTH(drawrect), radius);

    if (pen_width > 0)
    {
        hpen = MGPlusPenCreate (pen_width, (0x515051 | 0xFF<<24));
        MGPlusDrawRoundRectI (hgs, hpen,  drawrect.left, drawrect.top, RECTW(drawrect), RECTH(drawrect), radius);
        MGPlusPenDelete (hpen);
    }

    MGPlusBrushDelete (brush);
}

#if MDTV_ENABLE_SOFTIME 
extern HWND  g_kbwnd ;
#endif

static void show_ime_window(HWND hwnd, WPARAM wParam, LPARAM lParam)
{

    MGI_PHONE_IME_DATA pdata;
    pthread_mutex_lock (&mgphone_lock);
    pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
    //if (! pdata->is_opened)
    if (! pdata->is_opened)
    {
        ShowWindow (hwnd, SW_SHOWNORMAL);
        phone_adddata_init(pdata);
        pdata->is_opened = 1;
#if MDTV_ENABLE_SOFTIME 
        if (g_kbwnd)
            PostMessage (g_kbwnd, MSG_IME_CMD_OPEN, LOWORD (wParam), HIWORD (lParam));
#endif
    }
    pthread_mutex_unlock (&mgphone_lock);
}

int phone_ime_win_proc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    MGI_PHONE_IME_DATA pdata;
    HDC   hdc;
    switch (message) {
        case MSG_NCCREATE:
        {
            RegisterIMEWindow (hwnd);
            break;
        }
        case MSG_DOESNEEDIME:
        {
            // not receive ime message from minigui such as MSG_IME_OPEN, and so on. 
            return FALSE;
        }
        case MSG_CREATE: {
            if (!(pdata = (MGI_PHONE_IME_DATA) calloc (1, sizeof (struct _MGI_PHONE_IME_DATA)))){
                fprintf(stderr, "phone_ime_win_proc calloc MGI_PHONE_IME_DATA failed\n");
                return -1;
            }

            RECT rc_client;
            GetClientRect (hwnd, &rc_client);
            PMAINWINCREATE create_info = (PMAINWINCREATE)lParam;
            font_cn = CreateLogFont("ttf", "fmhei", "GB2312-0",
                    FONT_WEIGHT_BOLD, FONT_SLANT_ROMAN, FONT_FLIP_NIL, FONT_OTHER_NIL,
                    FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 24, 0);
            SetWindowFont(hwnd, font_cn);
            md_debug("font name %s\n", font_cn->mbc_devfont->name);
            pdata->ptim_case = PHONE_DEFAULT_CASE_num;
            pdata->next_cursor = -1;
            pdata->next_cursor_down = -1;
            if (create_info->dwStyle & WS_VISIBLE){
                pdata->is_opened = 1;
            }
            else{
                pdata->is_opened = 0;
            }
            md_debug ("init window ime is_opend %d\n", pdata->is_opened);
            pdata->phone_input_status = PHONE_NO_INPUT_STATUS;
            pdata->hgs = MGPlusGraphicCreate(RECTW(rc_client), RECTH(rc_client));
            DrawBackgroundWithMGPlus (pdata->hgs, RECTW(rc_client), RECTH(rc_client));
            //phone_adddata_init (pdata);
            SetWindowAdditionalData(hwnd, (DWORD) pdata);

            //load ime background bmp
            memset (&ime_en_bkg, 0, sizeof(ime_en_bkg));
            if ( LoadBitmapFromMem (HDC_SCREEN, &ime_en_bkg, ime_en_data ,
                        ime_en_data_len, "png") )
            {
                fprintf (stderr, "Can not load en background bitmap for IME\n"); 
            }
            memset (&ime_ch_bkg, 0, sizeof(ime_ch_bkg));
            if ( LoadBitmapFromMem (HDC_SCREEN, &ime_ch_bkg, ime_ch_data ,
                        ime_ch_data_len, "png") )
            {
                fprintf (stderr, "Can not load ch background bitmap for IME\n"); 
            }
            memset (&ime_nav_bkg, 0, sizeof(ime_nav_bkg));
            if ( LoadBitmapFromMem (HDC_SCREEN, &ime_nav_bkg, ime_nav_data ,
                        ime_nav_data_len, "png") )
            {
                fprintf (stderr, "Can not load ch background bitmap for IME\n"); 
            }

            if (LoadBitmapFromFile (HDC_SCREEN, &ime_disable_left_arrow,  imepicture[0]))
                fprintf (stderr, "Can not load ime arrow bitmap for IME\n"); 
            if (LoadBitmapFromFile (HDC_SCREEN, &ime_enable_left_arrow,  imepicture[1]))
                fprintf (stderr, "Can not load ime arrow bitmap for IME\n"); 
            if (LoadBitmapFromFile (HDC_SCREEN, &ime_disable_right_arrow,  imepicture[2]))
                fprintf (stderr, "Can not load ime arrow bitmap for IME\n"); 
            if (LoadBitmapFromFile (HDC_SCREEN, &ime_enable_right_arrow,  imepicture[3]))
                fprintf (stderr, "Can not load ime arrow bitmap for IME\n"); 
            break;
        }
                         break;
        case MSG_IME_CMD_OPEN:
        {
            md_debug("---- MSG_IME_CMD_OPEN\n");
            show_ime_window(hwnd, LOWORD (wParam), HIWORD (lParam));
            break;
        }
        case MSG_IME_CMD_CLOSE:{
            md_debug("---- MSG_IME_CMD_CLOSE\n");
            pthread_mutex_lock (&mgphone_lock);
            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
            if (pdata->is_opened)
            {
                ShowWindow (hwnd, SW_HIDE);
                pdata->is_opened = 0;
            }
#if MDTV_ENABLE_SOFTIME 
                if (g_kbwnd)
                    PostMessage (g_kbwnd, MSG_IME_CMD_CLOSE, wParam, lParam);
                    //ShowWindow (g_kbwnd, SW_HIDE);
#endif
            pthread_mutex_unlock (&mgphone_lock);
            break;
        }
        case MSG_IME_GETSTATUS:
            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
            return (int)(pdata->is_opened);
        case MSG_IME_SETTARGET:{
            md_debug("---- ime proc:MSG_IME_SETTARGET:  set target hwdn 0x%x\n", (wParam));
            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
            if (pdata->sg_target_hwnd!= hwnd)
                pdata->sg_target_hwnd = (HWND)wParam;
            break;
        }
        case MSG_IME_GETTARGET:{
            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
            return (int)pdata->sg_target_hwnd;
            break;
        }

        case MSG_SETFOCUS:
            md_debug("----- MSG_SETFOCUS\n");
            break;
        case MSG_KILLFOCUS:
            md_debug("----- MSG_KILLFOCUS\n");
            break;

        case MSG_KEYUP:
            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
            PostMessage(pdata->sg_target_hwnd, message, wParam, lParam);
#endif
            return 0;
        case MSG_KEYDOWN: {
            md_debug(" MSG_KEYDOWN!\n");
            int scancode_index;
            // for bug: not show window when set focus to input box through press mouse 
            show_ime_window(hwnd, LOWORD (wParam), HIWORD (lParam));

            pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hwnd);
            if (!pdata->is_opened || (get_scancode_index (pdata, LOWORD(wParam)) == -1)) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
                Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
                PostMessage(pdata->sg_target_hwnd, message, wParam, lParam);
#endif
                return 0;
            }

            scancode_index = get_scancode_index(pdata, LOWORD(wParam));

            /*swith ime mode*/
            if ( scancode_index == MGI_PHONE_KEY_SHARP && message == MSG_KEYDOWN) {
                md_debug(" scancode_index == MGI_PHONE_KEY_SHARP && message == MSG_KEYDOWN!\n");
                process_sharp_key (hwnd, message, wParam, lParam);
                phone_adddata_init(pdata);
                InvalidateRect(hwnd, NULL, FALSE);
                break;
            }

            if ( scancode_index == MGI_PHONE_KEY_STAR && message == MSG_KEYDOWN) {
                process_star_key (hwnd, message, wParam, lParam);
                return 0;
            }

            /*process predictive english ime*/
            switch (scancode_index) {
                case MGI_PHONE_KEY_0:
                case MGI_PHONE_KEY_1:
                case MGI_PHONE_KEY_2:
                case MGI_PHONE_KEY_3:
                case MGI_PHONE_KEY_4:
                case MGI_PHONE_KEY_5:
                case MGI_PHONE_KEY_6:
                case MGI_PHONE_KEY_7:
                case MGI_PHONE_KEY_8:
                case MGI_PHONE_KEY_9:
                md_debug(" MGI_PHONE_KEY_num = %d\n", scancode_index-MGI_PHONE_KEY_0);
                    /*===================process ime num mode=======================*/
                    if (pdata->ptim_case == PHONE_DEFAULT_CASE_num) {
                        if (pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
                            Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
                            // send MSG_CHAR to mdolphin window when press num key
                            PostMessage(pdata->sg_target_hwnd, MSG_CHAR, '0'+scancode_index-MGI_PHONE_KEY_0, lParam|KS_IMEPOST);
#endif
                        }
                        break;
                    }

                    /*process traditional english ime*/
                    if (pdata->ptim_case < PHONE_DEFAULT_CASE_MAX) {
                        if (pdata->phone_input_status != PHONE_SYMBOL_INPUT_STATUS) {
                            process_trad_en_num_key (hwnd,pdata, message, scancode_index);
                            //process_en_num_key (hwnd, message, wParam, lParam);
                        }
                        break;
                    }
                    process_en_num_key (hwnd, message, wParam, lParam);
                    break;

                case MGI_PHONE_KEY_LEFT:
                case MGI_PHONE_KEY_RIGHT:
                case MGI_PHONE_KEY_UP:
                case MGI_PHONE_KEY_DOWN:
                    process_navigator_key (hwnd, message, wParam, lParam);
                    break;

                case MGI_PHONE_KEY_CLEAR:
                    process_clear_key (hwnd, message, wParam, lParam);
                    break;

                case MGI_PHONE_KEY_ENTER:
                    process_confirm_key (hwnd, message, wParam, lParam);
                    break;
                case MGI_PHONE_KEY_QUIT:
                md_debug(" MGI_PHONE_KEY_num = %d\n", scancode_index-MGI_PHONE_KEY_0);
                    process_quit_key (hwnd, message, wParam, lParam);
                    break;
                default:
                    break;
            }
            return 0;
        }
        case MSG_PAINT:{
                pthread_mutex_lock (&mgphone_lock);
            hdc = BeginPaint (hwnd);
            refresh_input_method_area (hwnd, hdc);
            EndPaint (hwnd, hdc);
                pthread_mutex_unlock (&mgphone_lock);
            return 0;
        }
        case MSG_ERASEBKGND:
                       return 0;
        case MSG_LBUTTONDOWN:
        case MSG_LBUTTONDBLCLK: {
            process_mousemsg(hwnd, message, wParam, lParam);
           }
           break;
        case MSG_CLOSE:
            md_debug("close message\n");
            SendMessage(HWND_DESKTOP, MSG_IME_UNREGISTER, (WPARAM) hwnd, 0);
#if MDTV_ENABLE_SOFTIME 
            if (g_kbwnd) {
                DestroyMainWindow (g_kbwnd);
                g_kbwnd = NULL;
            }
#endif
            DestroyMainWindow(hwnd);
            PostQuitMessage (hwnd);
            UnloadBitmap(&ime_en_bkg);
            UnloadBitmap(&ime_ch_bkg);
            UnloadBitmap(&ime_nav_bkg);
            md_debug("close message 2\n");
            return 0;
#ifdef _MGRM_THREADS
        case MSG_IME_CMD_SETPOS:
            {
                RECT *pos_rc = (RECT *)wParam; 
                if (!pos_rc){
                    break;
                }
                RECT hwnd_rc;
                int max_y = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - 1;
                int xsize = pos_rc->left +X_OFFSIZE;
                int ysize;

                ysize = pos_rc->bottom  + Y_OFFSIZE;
                get_window_rect(hwnd, &hwnd_rc);
                if (ysize + RECTH(hwnd_rc) > max_y) {
                    md_debug("The ime window will display on the rect \n");
                    ysize = pos_rc->top -RECTH(hwnd_rc) ; 
                }

                if (ysize < 0)
                    ysize = 0;
                pthread_mutex_lock (&mgphone_lock);
                new_move_ime_window (hwnd, xsize, ysize);
                pthread_mutex_unlock (&mgphone_lock);

                return 0;
            }
#endif
#ifdef _MGRM_PROCESSES
        case MSG_FDEVENT:
            md_debug("socket message\n");
            if (LOWORD (wParam) == listen_fd) {
                md_debug("begin\n");
                /* This message comes from the listen socket fd. */
                pid_t pid;
                uid_t uid;
                int conn_fd;
                conn_fd = serv_accept (listen_fd, &pid, &uid);
                if (conn_fd >= 0) {
                    UnregisterListenFD(conn_fd_read);
                    conn_fd_read = conn_fd;
                    RegisterListenFD (conn_fd_read, POLLIN, hwnd, NULL);
                }
            }
            else {
                md_debug("receive\n");
                /* Client send a request. */
                /* Handle the request from client. */

                char buf[BODY_LEN + 1] = {0};
                int timeout = 10;
                int ret = sock_read_t (conn_fd_read, buf, BODY_LEN, timeout);
                if (ret != BODY_LEN)
                {
                    md_debug("err receive\n");
                    UnregisterListenFD(conn_fd_read);
                    return -1;
                }
                md_debug("received: %s\n", buf);
                socketProcess(hwnd, buf);
                //UnregisterListenFD(fd);
            }
            break;
#endif
    }

    return DefaultMainWinProc(hwnd, message, wParam, lParam);
}

void init_phoneimewin_createinfo (PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_VISIBLE;
    pCreateInfo->dwExStyle = WS_EX_TOPMOST | WS_EX_BROUNDCNS | WS_EX_TROUNDCNS;
    //pCreateInfo->dwExStyle = WS_EX_TOPMOST ;
    pCreateInfo->spCaption = "Phone IME Window";
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor(0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = phone_ime_win_proc;
    pCreateInfo->lx = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) - 200; 
    pCreateInfo->ty = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) - 32; 
    pCreateInfo->rx = GetGDCapability (HDC_SCREEN, GDCAP_MAXX);
    pCreateInfo->by = GetGDCapability (HDC_SCREEN, GDCAP_MAXY);
    pCreateInfo->iBkColor = GetWindowElementColor(WE_THREED_BODY);
    pCreateInfo->dwAddData = 0;
    pCreateInfo->hHosting = 0;
}


HWND mgi_phone_create_win (HWND hosting, int lx, int ty, int rx, int by)
{
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;
    MGI_PHONE_IME_DATA pdata;

    init_phoneimewin_createinfo(&CreateInfo);

    if (lx != 0 || ty != 0 || rx != 0 || by != 0) {
        CreateInfo.lx = lx;
        CreateInfo.ty = ty;
        CreateInfo.rx = rx;
        CreateInfo.by = by;
    }

    CreateInfo.hHosting = hosting;
    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
    {
        fprintf(stderr, "create window fail\n");
        return HWND_INVALID;
    }

    // hide ime window
    pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(hMainWnd);
    ShowWindow(hMainWnd, SW_HIDE);
    pdata->is_opened = 0;
#ifdef _MGRM_PROCESSES
    listen_socket(hMainWnd);
#endif

    return hMainWnd;
}

HWND mgiPhoneIMEWindowEx (int lx, int ty, int rx, int by)
{
    HWND ptiMainWnd = mgi_phone_create_win(HWND_DESKTOP, lx, ty, rx, by);

    if(HWND_INVALID == ptiMainWnd) {
        fprintf (stderr, "Can not create pti ime window. \n");
        return HWND_INVALID;
    }
    return ptiMainWnd;
}

#ifdef _MGRM_PROCESSES
HWND mgiCreatePhoneKeyPadIME(RECT *rect, BITMAP *bkgnd_bmp, PLOGFONT log_font, const MGI_PHONE_IME_TEXTCOLOR *textcolor, \
       const MGI_PHONE_KEY_MAP (*key_map)[MAX_PHONE_KEY_NUM], MGICB_ON_PHONE_IME_SWITCHED on_ime_switched)
{
    HWND hPhoneWnd;

#ifndef _STAND_ALONE
    if (!mgIsServer)
        return HWND_INVALID;
#endif

    if (rect == NULL) {
        fprintf(stderr, "Can not create mgiphone ime window, rect is NULL\n");
        return HWND_INVALID;
    }

    hPhoneWnd = mgiPhoneIMEWindowEx(rect->left, rect->top, rect->right, rect->bottom);
    if( hPhoneWnd != HWND_INVALID) {
        MGI_PHONE_IME_DATA mgi_phone_ime_data=(MGI_PHONE_IME_DATA)GetWindowAdditionalData(hPhoneWnd);
        if (mgi_phone_ime_data == NULL) {
            mgi_phone_ime_data = (MGI_PHONE_IME_DATA)calloc(1, sizeof(struct _MGI_PHONE_IME_DATA));
            if (mgi_phone_ime_data == NULL) {
                fprintf(stderr, "calloc failed\n");
                return -1;
            }
        }
        mgi_phone_ime_data->cb_notify = on_ime_switched;
        mgi_phone_ime_data->text_color= (MGI_PHONE_IME_TEXTCOLOR *)textcolor;
        mgi_phone_ime_data->key_map = key_map;//here has a warn: assignment from incompatible pointer type
        mgi_phone_ime_data->bkgnd_bmp= bkgnd_bmp;

        if (log_font)
            SetWindowFont(hPhoneWnd, log_font);
    }
    g_phone_hwnd = hPhoneWnd;
    return hPhoneWnd;
}

#else
typedef struct _IME_INFO
{
    sem_t wait;
    RECT *rect;
    BITMAP *bkgnd_bmp;
    PLOGFONT log_font;
    const  MGI_PHONE_IME_TEXTCOLOR *textcolor;
    const MGI_PHONE_KEY_MAP (*key_map)[MAX_PHONE_KEY_NUM];
    MGICB_ON_PHONE_IME_SWITCHED on_ime_switched;
    HWND hwnd;
} PHONE_IME_INFO;

static void* start_phone_ime (void* data)
{
    md_debug("----------- start_phone_ime()\n");
    MSG Msg;
    PHONE_IME_INFO* phone_ime_info = (PHONE_IME_INFO*)data;
    HWND phone_hwnd;
    MGI_PHONE_IME_DATA mgi_phone_ime_data;

    phone_hwnd = mgiPhoneIMEWindowEx(phone_ime_info->rect->left, 
                                     phone_ime_info->rect->top, 
                                     phone_ime_info->rect->right, 
                                     phone_ime_info->rect->bottom);
    if( phone_hwnd == HWND_INVALID)
        return NULL;

    g_phone_hwnd = phone_hwnd;

    mgi_phone_ime_data = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(phone_hwnd);
    if (mgi_phone_ime_data == NULL) {
        mgi_phone_ime_data =(MGI_PHONE_IME_DATA)calloc(1, sizeof(struct _MGI_PHONE_IME_DATA));
        if (mgi_phone_ime_data == NULL) {
            fprintf(stderr, "calloc failed\n");
            return NULL;
        }
    }
    mgi_phone_ime_data->cb_notify = phone_ime_info->on_ime_switched;
    mgi_phone_ime_data->text_color= phone_ime_info->textcolor;
    mgi_phone_ime_data->key_map = phone_ime_info->key_map;
    mgi_phone_ime_data->bkgnd_bmp= phone_ime_info->bkgnd_bmp;
    mgi_phone_ime_data->ptim_case = PHONE_DEFAULT_CASE_num;
    if (mgi_phone_ime_data->cb_notify)
        mgi_phone_ime_data->cb_notify((void *)mgi_phone_ime_data, PHONE_DEFAULT_CASE_num);

    if (phone_ime_info->log_font)
        SetWindowFont(phone_hwnd, phone_ime_info->log_font);
    
    phone_ime_info->hwnd = phone_hwnd;
    sem_post (&phone_ime_info->wait);

#if 0
#if MDTV_ENABLE_SOFTIME 
            if (!g_kbwnd)
                g_kbwnd =  mdtv_init_keyboard (HWND_DESKTOP);
#endif
#endif
    //pthread_mutex_init(&mgphone_lock, NULL);
    while (GetMessage (&Msg, phone_hwnd) ) {
        if (Msg.message == MSG_KEYDOWN || Msg.message == MSG_KEYUP) {
            if (mgi_phone_ime_data->is_opened) {
                TranslateMessage (&Msg);
            }
            else if (mgi_phone_ime_data->sg_target_hwnd) {
                PostMessage (mgi_phone_ime_data->sg_target_hwnd, 
                        Msg.message, Msg.wParam, Msg.lParam );//| KS_IMEPOST);
                continue;
            }
    }
        DispatchMessage(&Msg);
        //TranslateMessage (&Msg);
    }

    if (mgi_phone_ime_data) {
        MGPlusGraphicDelete (mgi_phone_ime_data->hgs);
        free(mgi_phone_ime_data);
    }

    MainWindowThreadCleanup (phone_hwnd);

    return NULL;
}

static pthread_t phone_ime_thread;
HWND mgiCreatePhoneKeyPadIME (RECT *rect, BITMAP *bkgnd_bmp, PLOGFONT log_font, const MGI_PHONE_IME_TEXTCOLOR *textcolor, \
       const MGI_PHONE_KEY_MAP(*key_map)[MAX_PHONE_KEY_NUM], MGICB_ON_PHONE_IME_SWITCHED on_ime_switched)
{
    PHONE_IME_INFO phone_ime_info;
    pthread_attr_t new_attr;

    if (!rect)
        return HWND_INVALID;

    sem_init (&phone_ime_info.wait, 0, 0);
    phone_ime_info.log_font = log_font;
    phone_ime_info.rect = rect;
    phone_ime_info.bkgnd_bmp = bkgnd_bmp;
    phone_ime_info.log_font = log_font;
    phone_ime_info.textcolor = textcolor;
    phone_ime_info.key_map = key_map;
    phone_ime_info.on_ime_switched = on_ime_switched;

    pthread_attr_init (&new_attr);
    pthread_attr_setdetachstate (&new_attr, PTHREAD_CREATE_DETACHED);
    pthread_create (&phone_ime_thread, &new_attr, start_phone_ime, &phone_ime_info);
    pthread_attr_destroy (&new_attr);

    sem_wait (&phone_ime_info.wait);
    sem_destroy (&phone_ime_info.wait);

    return phone_ime_info.hwnd;
}
#endif

#define MGPHONE_REQID     (MAX_SYS_REQID + 1)

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
typedef struct _MGPHONE_REQUEST_DATA {
    int op_id;
    MGI_PHONE_IME_METHOD method; 
    int cursor;
    int case_mode;
} MGPHONE_REQUEST_DATA;

typedef struct _MGPHONE_REPLY_DATA {
    int cursor;
    char buff [PHONE_RESULT_BUFF_LONG];
} MGPHONE_REPLY_DATA;

typedef enum {
    OPID_GET_METHOD_CODE,
    OPID_GET_CUR_METHOD,
    OPID_SET_CUR_METHOD
}REQUEST_TYPE;

#endif

BOOL mgi_phone_ime_get_curr_methodcode(char *code, int code_len, int method_id)
{
    MGI_PHONE_IME_DATA pdata;

    if(!code || g_phone_hwnd == HWND_INVALID)
        return FALSE;

    pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
    if (!pdata)
        return FALSE;
    if (method_id > 0 && method_id < PHONE_DEFAULT_CASE_MAX) {
        switch(method_id)
        {
            case PHONE_DEFAULT_CASE_num:
                strncpy(code, "123", code_len);
                break;
            case PHONE_DEFAULT_CASE_ABC:
                strncpy(code, "ABC", code_len);
                break;
            case PHONE_DEFAULT_CASE_abc:
                strncpy(code, "abc", code_len);
                break;
            default:
                break;
        }
        return TRUE;
    }
    else {
        if (pdata->cur_method && pdata->cur_method->method_id == method_id){
            strncpy(code, pdata->cur_method->method_name, code_len);
            return TRUE;
        }
        else {
            MGI_PHONE_IME_METHOD *p = pdata->head_method;
            while(p) {
                if (p->method_id == method_id){
                    strncpy(code, p->method_name, code_len);
                    return TRUE;
                }
                p = p->next;
            }
        }
    }
    return FALSE;

}

BOOL mgiPhoneKeyPadGetMethodCode(char *code, int code_len, int method_id)
{
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (!mgIsServer) {
        REQUEST req;
        MGPHONE_REQUEST_DATA data = {0};
        MGPHONE_REPLY_DATA reply;

        data.op_id = OPID_GET_METHOD_CODE;
        data.cursor = method_id;

        req.id = MGPHONE_REQID;
        req.data = &data;
        req.len_data = sizeof(data);

        ClientRequest (&req, &reply, sizeof (reply));
        strncpy(code, reply.buff, code_len);
        return TRUE;
    } else
        return mgi_phone_ime_get_curr_methodcode(code, code_len, method_id);
#else
    return mgi_phone_ime_get_curr_methodcode(code, code_len, method_id);
#endif
}

BOOL mgiPhoneKeyPadRemoveMethod(HWND ime_hwnd, int method_id)
{    
    MGI_PHONE_IME_DATA pdata=NULL;
    MGI_PHONE_IME_METHOD *p=NULL, *prev=NULL;
    int i=0;

    if (g_phone_hwnd == HWND_INVALID || method_id < 1)
        return FALSE;

    pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
    if (pdata == NULL)
        return FALSE;

    prev = p = pdata->head_method;
    while (p) {
        if(p->method_id == method_id)
            break;
        prev = p;
        p = p->next;
        i++;
    }

    if (p) 
    {
        if ( p == pdata->head_method) {
            pdata->head_method = p->next;
            if (pdata->cur_method == p) {
                pdata->cur_method = p->next;
                if (! pdata->cur_method )
                    pdata->ptim_case = PHONE_DEFAULT_CASE_num;
            }
        }
        else {
            prev->next = p->next;
            if (pdata->cur_method == p) {
                if (p->next)
                    pdata->cur_method = p->next;
                else {
                    pdata->cur_method = prev;
                    pdata->ptim_case --;
                }
                /*need notice and init*/
            }
        }
        phone_adddata_init(pdata);
        InvalidateRect(g_phone_hwnd, NULL, TRUE);
        return TRUE;
    }
    return FALSE;
}

int mgi_phone_ime_set_currmethod(int method_id)
{
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
    MGI_PHONE_IME_METHOD *p=NULL;
    int i=0;

    if (method_id < 0)
        return -1;
    if (!pdata)
        return -1;

    p = pdata->head_method;
    while(p) {
        if (p->method_id == method_id){
            if (pdata->cur_method) {
                if (pdata->cur_method->actived)
                    pdata->cur_method->actived(pdata, FALSE);
            }
            pdata->cur_method = p;
            pdata->ptim_case = PHONE_DEFAULT_CASE_MAX + i;

            /* notify */
            if (pdata->cb_notify)
                pdata->cb_notify((void *)pdata, method_id);
            if (pdata->cur_method->actived)
                pdata->cur_method->actived(pdata, TRUE);
            return 0;
        }
        p = p->next;
        i++;
    }
    return -1;
}

int mgiPhoneKeyPadSetCurrMethod(int method_id)
{
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (!mgIsServer) {
        REQUEST req;
        MGPHONE_REQUEST_DATA data = {0};
        MGPHONE_REPLY_DATA reply;

        data.op_id = OPID_SET_CUR_METHOD;
        data.cursor = method_id;

        req.id = MGPHONE_REQID;
        req.data = &data;
        req.len_data = sizeof(data);

        ClientRequest (&req, &reply, sizeof (reply));
        return 0;
    } else
        return mgi_phone_ime_set_currmethod(method_id);
#else
    return mgi_phone_ime_set_currmethod(method_id);
#endif
}

int mgiPhoneKeyPadGetCurrMethod()
{
#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
    if (!mgIsServer) {
        REQUEST req;
        MGPHONE_REQUEST_DATA data = {0};
        MGPHONE_REPLY_DATA reply;

        data.op_id = OPID_GET_CUR_METHOD;

        req.id = MGPHONE_REQID;
        req.data = &data;
        req.len_data = sizeof(data);

        ClientRequest (&req, &reply, sizeof (reply));
        return reply.cursor;
    } 
    else {
        MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
        if (!pdata)
            return -1;
        if( pdata->cur_method)
            return pdata->cur_method->method_id;
        else
            return pdata->ptim_case;
    }
#else
    MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
    if (!pdata)
        return -1;
    if( pdata->cur_method)
        return pdata->cur_method->method_id;
    else
        return pdata->ptim_case;
#endif
}

BOOL mgiPhoneKeyPadAddMethod(HWND ime_hwnd, MGI_PHONE_IME_METHOD *method_data)
{
    MGI_PHONE_IME_DATA pdata=NULL;
    MGI_PHONE_IME_METHOD *p=NULL;

    if (g_phone_hwnd == HWND_INVALID || method_data == NULL)
        return FALSE;

    pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
    if (pdata == NULL)
        return FALSE;

    p = pdata->head_method;
    if (p == NULL)/*first method*/
        pdata->head_method = method_data;
    else {
        while(p) {
            if (p->method_id == method_data->method_id) {
                return FALSE;
            }
            if (p->next == NULL)
                break;
            p = p->next;
        }
        if (p) { 
            p->next = method_data;
            method_data->next = NULL;
        }
    }
    return TRUE;
}

#if defined(_LITE_VERSION) && !defined(_STAND_ALONE)
int mgphone_handler (int cli, int clifd, void* buff, size_t len)
{
    MGPHONE_REQUEST_DATA* data = (MGPHONE_REQUEST_DATA*) buff;
    MGPHONE_REPLY_DATA reply;
    switch (data->op_id) {
        case OPID_GET_METHOD_CODE:
            mgi_phone_ime_get_curr_methodcode(reply.buff, sizeof(reply.buff), data->cursor);
            break;
        case OPID_GET_CUR_METHOD: {
            MGI_PHONE_IME_DATA pdata = (MGI_PHONE_IME_DATA)GetWindowAdditionalData(g_phone_hwnd);
            if (!pdata)
                reply.cursor = -1;
            if( pdata->cur_method)
                reply.cursor = pdata->cur_method->method_id;
            else
                reply.cursor = pdata->ptim_case;
            break;
        }
        case OPID_SET_CUR_METHOD:
            md_debug("mgphone_handler():  data->cursor: %d\n",data->cursor);
            mgi_phone_ime_set_currmethod(data->cursor);
            break;
        default:
            break;
    }
    return ServerSendReply (clifd, &reply, sizeof(reply));
}

BOOL mgiPhoneKeyPadServerInstallRequest(void)
{
    return RegisterRequestHandler(MGPHONE_REQID, mgphone_handler);
}

#endif
