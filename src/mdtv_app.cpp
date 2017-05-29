//#include "config.h"
#include "mdtv_app.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_common.h"
#include "mdtv_browser.h"
#include "mdtv_ime.h"
#include "mdtv_keyboard.h"
#include "mdtv_toolbar.h"
#include "mdtv_website.h"
#include "mdtv_fav.h"
#include <mgplus/mgplus.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int g_win_width;
int g_win_height;
float scale;
float scale_x;
float scale_y;

char MDTV_ETCFILEPATH[MAX_PATH+1];
char MDTV_RESFILEPATH[MAX_PATH+1];
#define MDTV_ETCFILE "mdtv.cfg"
#define MDTV_MAXLOGFONT 64

HWND g_hMainWnd ;
DEVFONT *g_devfont[MDTV_MAXLOGFONT];
int g_nr_fonts;
// create main window and display animation

#if MDTV_ENABLE_SOFTIME 
HWND  g_kbwnd = NULL;
#endif

static MSGHOOK main_old_hook;
//#define MDTV_APP_DEBUG

static PLOGFONT cap_font = NULL;
static PLOGFONT ctrl_font = NULL;
static PLOGFONT utf8_font = NULL;

static int MDTVMainWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

// font
typedef struct _DEVFONTINFO DEVFONTINFO;
struct _DEVFONTINFO
{
    char devfont_name[MAX_NAME + 1];
    char file_name[MAX_PATH + 1];
};

typedef struct _FONTINFO FONTINFO;
struct _FONTINFO
{
    int nr_fonts;
    DEVFONTINFO *devfont_info;
};

static BOOL load_dev_font(DEVFONT **devfont, int *number)
{
    FONTINFO font_info;
    BOOL ret = TRUE;
    int i;
    char fontname[MAX_NAME + 1];
    char filename[MAX_PATH + 1];
    
    if (GetIntValueFromEtcFile(MDTV_ETCFILEPATH, "extra_fonts", "font_number", &font_info.nr_fonts) != ETC_OK)
        return FALSE;
    if (font_info.nr_fonts <= 0)
        return TRUE;

    if (font_info.nr_fonts > MDTV_MAXLOGFONT)
    {
           
        *number = 0;
        fprintf(stderr, "Error: Exceed the max font number\n"); 
        return FALSE;
    }

    //malloc
    if ((font_info.devfont_info = (DEVFONTINFO *)malloc(font_info.nr_fonts * sizeof(DEVFONTINFO))) == NULL)
        return FALSE;
    for (i = 0; i < font_info.nr_fonts; ++i)
    {
        sprintf(fontname, "font%d", i);
        sprintf(filename, "file%d", i);
        if (GetValueFromEtcFile(MDTV_ETCFILEPATH, "extra_fonts", fontname, 
                    font_info.devfont_info[i].devfont_name, MAX_NAME) != ETC_OK)
            ret = FALSE;
        if (GetValueFromEtcFile(MDTV_ETCFILEPATH, "extra_fonts", filename, 
                    font_info.devfont_info[i].file_name, MAX_PATH) != ETC_OK)
            ret = FALSE;

        devfont[i] = LoadDevFontFromFile(font_info.devfont_info[i].devfont_name, font_info.devfont_info[i].file_name);
    }

#ifdef MDTV_APP_DEBUG
    for (i = 0; i < font_info.nr_fonts; ++i)
    {
        printf("font name: %s\n", font_info.devfont_info[i].devfont_name);
        printf("font file: %s\n", font_info.devfont_info[i].file_name);
    }
#endif
    //free
    free(font_info.devfont_info);
    *number = font_info.nr_fonts;
    return ret;
}

void DestroyAllDevFont ()
{
   int i = 0;

   for (i=0; i<g_nr_fonts; i++)
   {
       DestroyDynamicDevFont (&g_devfont[i]);  
   }
}

BOOL mdtv_SelectWindowFontBycfg(HDC hdc, const char *fontname)
{

    char logfontname [MAX_NAME+1]; 
    PLOGFONT logfont = NULL;

    if (GetValueFromEtcFile(MDTV_ETCFILEPATH, "fonts", fontname, logfontname, MAX_NAME) != ETC_OK)
        return FALSE;

    logfont = CreateLogFontByName (logfontname);
    SelectFont (hdc, logfont);
    return TRUE;
}

void InitBackGround(void)
{
    char position [MAX_PATH + 1];
    DRAW_IMAGEBG_MODE draw_mode = DRAW_IMAGEBG_CENTER;
#ifdef WITH_BKGND_PIC
    char *filename = NULL;
    char bkimage[MAX_PATH + 1];
    unsigned int bkcolor = 0xFFFFFFFF;
    if (GetIntValueFromEtcFile(MDTV_ETCFILEPATH, "background", "bkcolor",(int *)(&bkcolor)) != ETC_OK)
    {
        fprintf (stderr, "Error: Get bkColor error\n");
    }

    if (GetValueFromEtcFile(MDTV_ETCFILEPATH, "background", "bkimage", bkimage, MAX_PATH) == ETC_OK)
    {
        //filename = (char *)malloc(MAX_PATH + 1);
        filename = (char *)calloc(MAX_PATH + 1, 1);
        strncpy (filename, MDTV_RESFILEPATH, strlen(MDTV_RESFILEPATH));
        strcat (filename, "/");
        strcat (filename, bkimage);

#ifdef MDTV_APP_DEBUG
        printf("bkground image is %s\n", filename);
#endif
    }
#endif

    if (GetValueFromEtcFile(MDTV_ETCFILEPATH, "background", "position", position, MAX_NAME) == ETC_OK)
    {
#ifdef MDTV_APP_DEBUG
        printf("bkground position is %s\n", position);
#endif
         if (strncasecmp (position, "center", strlen(position)) == 0)
            draw_mode = DRAW_IMAGEBG_CENTER;
         else if (strncasecmp (position, "tiled", strlen(position)) == 0)
            draw_mode = DRAW_IMAGEBG_TILED;
         else if (strncasecmp (position, "scaled", strlen(position)) == 0)
            draw_mode = DRAW_IMAGEBG_SCALED;
    }
   
#ifdef WITH_BKGND_PIC
    init_background_dc (bkcolor, filename, draw_mode);
    if (filename != NULL)
        free(filename);
#endif
}

static void mdtv_SetResPath (void)
{
    getcwd (MDTV_RESFILEPATH, MAX_PATH);
    strcat (MDTV_RESFILEPATH, "/");
    strcat (MDTV_RESFILEPATH, "res");
    SetResPath(MDTV_RESFILEPATH);
}

static BOOL GetEtcFile (void)
{
     struct stat     statbuf;

     getcwd (MDTV_ETCFILEPATH, MAX_PATH);
     strcat (MDTV_ETCFILEPATH, "/");
     strcat (MDTV_ETCFILEPATH, MDTV_ETCFILE);
     if ((stat(MDTV_ETCFILEPATH, &statbuf) == -1) || !S_ISREG(statbuf.st_mode))
     {
        fprintf(stderr, "Error: open config file mdtv.cfg failed\n");
        return FALSE;
     }

     return TRUE;
}

static BOOL InitEtcFile(void)
{

   if (!GetEtcFile())
       return FALSE;

// load dev font
#if defined (_MGRM_PROCESSES) 
    InitVectorialFonts();
#endif
    if (!load_dev_font(g_devfont, &g_nr_fonts))
    {
        fprintf(stderr, "Load dev font error\n");
    }

    // init bkgnd dc
    InitBackGround();
    return TRUE;
}

static HWND CreateMDTVMainWnd()
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle =  WS_EX_TOOLWINDOW;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = MDTVMainWndProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = RECTW(g_rcScr);
    CreateInfo.by = RECTH(g_rcScr);
    CreateInfo.iBkColor = COLOR_black;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    return CreateMainWindow (&CreateInfo);
}

static HWND CreateMDTVMainWndWithAnimation()
{
    HWND hWnd;


    hWnd = CreateMDTVMainWnd();
    if(hWnd==HWND_INVALID){
        fprintf(stderr,"MainWnd> Create MainWindow Error!\n");
        return HWND_INVALID;
    }


    return hWnd;
}

 static DESKTOPOPS *old_dsk_ops = NULL;
 static void* mdtv_this_init(void)
 {
    if (old_dsk_ops) 
        old_dsk_ops->init ();
    return NULL;
 }

 static void mdtv_this_deinit(void* context)
 {
    if (old_dsk_ops) 
        old_dsk_ops->deinit (context);
 }

 static void mdtv_this_paint_desktop(void* context,\
                         HDC dc_desktop, const RECT* inv_rc)
 {
#ifdef WITH_BKGND_PIC
    if (inv_rc != NULL)
        BitBlt(g_dc_bkgnd, inv_rc->left, inv_rc->top, RECTWP(inv_rc), RECTHP(inv_rc), \
                dc_desktop, inv_rc->left, inv_rc->top, 0);
    else
        BitBlt(g_dc_bkgnd, 0, 0, 0, 0, dc_desktop,0, 0, 0);
#else
    printf ("Erase background of desktop here\n");
    SetBrushColor (dc_desktop, 0x00);
    if (inv_rc != NULL)
        FillBox(dc_desktop, inv_rc->left, inv_rc->top, RECTWP(inv_rc), RECTHP(inv_rc));
    else
        FillBox(dc_desktop, 0, 0, g_rcScr.right + 2, g_rcScr.bottom + 2);
#endif
 }

 static void mdtv_this_keyboard_handler(void* context, int message,\
                                       WPARAM wParam, LPARAM lParam)
 {
    if (old_dsk_ops) 
        old_dsk_ops->keyboard_handler (context,message, wParam, lParam);
 }

 static void mdtv_this_mouse_handler(void* context, int message,\
                                            WPARAM wParam, LPARAM lParam)
 {
    if (old_dsk_ops) 
        old_dsk_ops->mouse_handler (context,message, wParam, lParam);
 }

 static void mdtv_this_customize_desktop_menu (void* context,\
                                           HMENU hmnu, int start_pos)
 {
    if (old_dsk_ops) 
        old_dsk_ops->customize_desktop_menu (context,hmnu, start_pos);
 }

 static void mdtv_this_desktop_menucmd_handler (void* context, int id)
 {
    if (old_dsk_ops) 
        old_dsk_ops->desktop_menucmd_handler (context, id);
 }

 static DESKTOPOPS mdtv_this_dsk_ops =
 {
     mdtv_this_init,
     mdtv_this_deinit,
     mdtv_this_paint_desktop,
     mdtv_this_keyboard_handler,
     mdtv_this_mouse_handler,
     mdtv_this_customize_desktop_menu,
     mdtv_this_desktop_menucmd_handler,
 };

int main_hook (void* p_hWnd, HWND dst_wnd, int msg, WPARAM wParam, LPARAM lParam)
{
#if 0
    if( msg == MSG_KEYUP && wParam == SCANCODE_SLASH){
        //printf ("I am here \n");
        PostMessage (*(HWND*)p_hWnd, msg, wParam, lParam);
        //PostMessage (g_hMainWnd, msg, wParam, lParam);

        return HOOK_STOP;
    }
#endif
    return HOOK_GOON;
}

static int MDTVMainWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case MSG_CREATE:
        SetWindowBkColor (hWnd, PIXEL_black);
        main_old_hook = RegisterKeyMsgHook (&hWnd, main_hook);
        break;
        case MSG_PAINT:
            {
                HDC hdc = BeginPaint (hWnd);
#ifdef WITH_BKGND_PIC
                BitBlt(g_dc_bkgnd, 0, 0, 0, 0, hdc,0, 0, 0);
#else
                SetBrushColor(hdc, RGB2Pixel(hdc, 0x00, 0x00, 0x00) );
                //FillBox(hdc, 0, 0, GetGDCapability(hdc, GDCAP_MAXX), GetGDCapability(hdc, GDCAP_MAXY));
                FillBox(hdc, 0, 0, g_win_width, g_win_height);
#endif
                EndPaint(hWnd, hdc);
                break;
            }
        case MSG_ERASEBKGND:
            return 0;
        case MSG_KEYUP:
            switch(wParam)
            {
                case SCANCODE_SLASH:
                    InitToolbar(g_hMainWnd);
                    break;
                case SCANCODE_F2:
#ifdef MDTV_APP_DEBUG
#endif
                   mdtv_CreateWebSiteNavigate (NULL); 
                    break;
                case SCANCODE_F3:
                mdtv_CreateWebsiteWindow (hWnd);
                break;
                case SCANCODE_F10:
                    break;
                case MSG_MOUSEMOVE:
                    break;
            }
            break;
        case MSG_RBUTTONDOWN:
            // avoid popping up minigui menu
            return 0;
        case MSG_CLOSE:
#ifdef _MGRM_PROCESSES
            //show_ime  (FALSE, 0);
#endif  
            RegisterKeyMsgHook (0, main_old_hook);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
            break;
    }

    return DefaultMainWinProc(hWnd,message,wParam,lParam);
}

static void init_m_font()
{

    WINDOW_ELEMENT_RENDERER* render;    
    cap_font = g_SysLogFont[SYSLOGFONT_CAPTION];
    ctrl_font = g_SysLogFont[SYSLOGFONT_CONTROL];

    if(g_win_width >= 1024)
        utf8_font = CreateLogFontByName("*-Arial-rrncnn-*-24-UTF-8");
    else if(g_win_width >= 800)
        utf8_font = CreateLogFontByName("*-Arial-rrncnn-*-20-UTF-8");
    else 
        utf8_font = CreateLogFontByName("*-Arial-rrncnn-*-18-UTF-8");

    render = const_cast<WINDOW_ELEMENT_RENDERER*>(GetDefaultWindowElementRenderer());
    render->we_fonts[WE_MESSAGEBOX] = utf8_font;
    render->we_fonts[WE_CAPTION] = utf8_font;
    render->we_metrics[WE_CAPTION] = utf8_font->size + 6;

    g_SysLogFont[SYSLOGFONT_CAPTION] = utf8_font;
    g_SysLogFont[SYSLOGFONT_CONTROL] = utf8_font;

    local_SysText = GetSysTextInUTF8("zh_CN");
}

static void release_m_font()
{
    g_SysLogFont[SYSLOGFONT_CAPTION] = cap_font;
    g_SysLogFont[SYSLOGFONT_CONTROL] = ctrl_font;

    DestroyLogFont(utf8_font);
}

static int MiniGUIMain(int argc, const char* argv[])
{
    MSG Msg;
    g_win_width = RECTW(g_rcScr);
    g_win_height = RECTH(g_rcScr);
    scale = (1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT)/(1.0*RECTW(g_rcScr)/RECTH(g_rcScr));
    if(scale>1.0){
        scale = 1.0;
    }
    scale_x = 1.0*g_win_width/DEFAULT_SCREEN_WIDTH ;
    scale_y = 1.0*g_win_height/DEFAULT_SCREEN_HEIGHT ;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "mdtv" , 0 , 0);
#endif

    mdtv_SetResPath();
    init_dbbuffer();
    GetEtcFile();
    InitEtcFile();
    old_dsk_ops = SetCustomDesktopOperationSet (&mdtv_this_dsk_ops);

    MGPlusRegisterFashionLFRDR();
    SetDefaultWindowElementRenderer("skin");
    init_m_font();
    RegisterMDolphinControl();

#if ENABLE_FAV
    // init favorite res
    InitMDTVFavWndRes();
    InitMDTVFavWebsite();
#endif

    g_hMainWnd = CreateMDTVMainWndWithAnimation();
    if(g_hMainWnd==HWND_INVALID){
        fprintf(stderr,"MainWnd> Create MainWindow Error!\n");
        goto ERROR;
    }

#if MDTV_ENABLE_SOFTIME
        //g_kbwnd = mdtv_init_keyboard (g_hMainWnd);
        //mdtv_CreateImePinying ();
#else
        //mdtv_CreateImePinying ();
#endif

    InitToolbar(g_hMainWnd);
    while(GetMessage(&Msg,g_hMainWnd))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    UnregisterMDolphinControl();
    release_m_font();
    MainWindowThreadCleanup(g_hMainWnd);
ERROR:
#if ENABLE_FAV
    // destroy fav res
    UninitMDTVFavWebsite();
    UninitMDTVFavWndRes();
#endif

    DestroyAllDevFont();
    return 0;
}

#ifdef _USE_MINIGUIENTRY 
static int g_args = 0;
static const char **g_argv = NULL;

void * start_routine(void *arg)
{
	minigui_entry(g_args, g_argv);
	return 0;
}

int main(int args, const char* argv[])
{
	pthread_attr_t tattr;
	pthread_t tid;
	int ret;

    if (argv[1])
        home_url = argv[1];

    g_args = args;
    g_argv = argv;

    ret = pthread_attr_init(&tattr);
    pthread_attr_setstacksize (&tattr, 512 * 1004);
    ret = pthread_create(&tid, &tattr, start_routine, NULL);
    pthread_join(tid, NULL);
    return ret;
}
#endif

