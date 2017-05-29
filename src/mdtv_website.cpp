#include "mdtv_app.h"
#include "mdtv_website.h"
#include "mdtv_ime.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_browser.h"
#include "mdtv_animation.h"
#include "mdtv_toolbar.h"

#include "svgui.h"

//#define  MDTV_WEBSITE_DEBUG

#define HAVE_ARROW_WND   0

HWND g_website_hwnd;
extern HWND g_hMainWnd;
static HWND sg_dlglevel3_hwnd = NULL;
#if HAVE_ARROW_WND
static HWND sg_arrow_hwnd = NULL;
#endif
static int sg_dlglevel2_selected_id = -1;
static int create_dlglevel3 = 0;

RECT weather_nomal_wnd_rect;

#define DEFAULT_DLGLEVEL2_WIDTH 380
#define DEFAULT_DLGLEVEL2_HEIGHT 662
#define DEFAULT_DLGLEVEL2_LEFTOFFSET 51
#define DEFAULT_DLGLEVEL2_TOPOFFSET 51

#define DEFAULT_WEBSITE_SCREEN_WIDTH 1024.0F
#define DEFAULT_WEBSITE_SCREEN_HEIGHT 768.0F

#define WEBSITE_ARROW_WND_X         431
#define WEBSITE_ARROW_WND_Y         400
#define WEBSITE_ARROW_WND_WIDTH     80
#define WEBSITE_ARROW_WND_HEIGHT    80

#define WEBSITE_LEFTSPACE               (51)
#define WEBSITE_TOPSPACE                (20)
#define WEBSITE_LEFTSPACE_PERCENT   (1.0*WEBSITE_LEFTSPACE/DEFAULT_SCREEN_WIDTH)
#define WEBSITE_TOPSPACE_PERCENT    (1.0*WEBSITE_TOPSPACE/DEFAULT_SCREEN_HEIGHT)

#define WEBSITE_WND_LEVEL2_WIDTH_PERCENT    (1.0*DEFAULT_DLGLEVEL2_WIDTH/DEFAULT_WEBSITE_SCREEN_WIDTH)
#define WEBSITE_WND_LEVEL2_HEIGHT_PERCENT    (1.0*DEFAULT_DLGLEVEL2_HEIGHT/DEFAULT_WEBSITE_SCREEN_HEIGHT)

#define  MAX_IMAGE_NUM 4
typedef SVGUI_IMAGE_T  SVGUI_IMAGE_T_4ARRAY[MAX_IMAGE_NUM];
static void CloseWebsiteAndOpenBrowser (HWND hWnd, const char *url);

static SVGUI_TEXT_AREA_T svgui_dlglevel2_layer2_text_area [] =
{
    {1, TRUE, {20, 0, 380, 65}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 0, "WebSite"},
};

static const char * dlglevel2_layer3_text [] = 
{
    "门户",
    "搜索",
    "新闻",
    "小说",
    "邮箱",
};

enum  DLGLEVEL2_HOTPOT_ID
{
    DLGLEVEL2_MIN_ID = 3,
    DLGLEVEL2_DOOR_ID = DLGLEVEL2_MIN_ID,
    DLGLEVEL2_SEARCH_ID,
    DLGLEVEL2_NEWS_ID,
    DLGLEVEL2_NOVEL_ID,
    DLGLEVEL2_MAIL_ID,
    DLGLEVEL2_MAX_ID = DLGLEVEL2_MAIL_ID,
};

static SVGUI_TEXT_AREA_T svgui_dlglevel2_layer3_text_area [] =
{
    {1, TRUE, {60, 0, 358, 110}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 1, dlglevel2_layer3_text[0]},
    {1, TRUE, {60, 0, 358, 110}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 1, dlglevel2_layer3_text[1]},
    {1, TRUE, {60, 0, 358, 110}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 1, dlglevel2_layer3_text[2]},
    {1, TRUE, {60, 0, 358, 110}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 1, dlglevel2_layer3_text[3]},
    {1, TRUE, {60, 0, 358, 110}, SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER, 
        0xFFFFFF, 1, dlglevel2_layer3_text[4]},
};

SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel2_layer3_doorimage_hotpot [] = 
{
{
    {44, 46, 32, "website/icon/44x46-door.png"},
    {55, 57, 32, "website/icon/55x57-door.png"},
    {69, 72, 32, "website/icon/door.png"},
},
};

#if 1
SVGUI_IMAGE_T  svgui_dlglevel2_layer3_image_hotpot[][4] = 
{
    {
        {44, 46, 32, "website/icon/44x46-door.png"},
        {55, 57, 32, "website/icon/55x57-door.png"},
        {69, 72, 32, "website/icon/69x72-door.png"},
    },
//54x54-search.png  68x68-search.png  86x86-search.png
    {
        { 54, 54, 32, "website/icon/54x54-search.png"},
        { 68, 68, 32, "website/icon/68x68-search.png"},
        { 86, 86, 32, "website/icon/86x86-search.png"},
    },
// 46x39-news.png  59x49-news.png  73x61-news.png  news.png
    {
        {46, 39, 32, "website/icon/46x39-news.png"},
        {59, 49, 32, "website/icon/59x49-news.png"},
        {73, 61, 32, "website/icon/73x61-news.png"},
    },
// 43x42-novel.png  50x51-novel.png  63x64-novel.png  novel.png
    {
        {43, 42, 32, "website/icon/43x42-novel.png"},
        {50, 51, 32, "website/icon/50x51-novel.png"},
        {63, 64, 32, "website/icon/63x64-novel.png"},
    },

    //58x39-email.png  65x46-email.png  90x58-email.png
    {
        {58, 39, 32, "website/icon/58x39-email.png"},
        {65, 46, 32, "website/icon/65x46-email.png"},
        {90, 58, 32, "website/icon/90x58-email.png"},
    }
};
#endif

SVGUI_IMAGE_T  svgui_dlglevel2_layer3_image_arrow[] = 
{
    {14, 29, 32, "website/icon/arrow.png"},
};

typedef  SVGUI_IMAGE_AREA_T SVGUI_IMAGE_AREA_T_2ARRAW[2];

static SVGUI_IMAGE_AREA_T_2ARRAW svgui_dlglevel2_layer3_image_area [] =
{
    {
        {0, TRUE, {215, 0, 286, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 3, (svgui_dlglevel2_layer3_image_hotpot[0])},
        {1, TRUE, {328, 0, 343, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_dlglevel2_layer3_image_arrow},
    },

    {
        {0, TRUE, {215, 0, 302, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 3, (svgui_dlglevel2_layer3_image_hotpot[1])},
        {1, FALSE, {328, 44, 343, 76}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_dlglevel2_layer3_image_arrow},
    },

    {
        {0, TRUE, {215, 0, 289, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 3, (svgui_dlglevel2_layer3_image_hotpot[2])},
        {1, FALSE, {328, 44, 343, 76}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_dlglevel2_layer3_image_arrow},
    },

    {
        {0, TRUE, {215, 0, 279, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 3, (svgui_dlglevel2_layer3_image_hotpot[3])},
        {1, FALSE, {328, 44, 343, 76}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_dlglevel2_layer3_image_arrow},
    },

    {
        {0, TRUE, {215, 0, 306, 110}, SVGUI_IMAGE_FILL_WAY_CENTER, 3, (svgui_dlglevel2_layer3_image_hotpot[4])},
        {1, FALSE, {328, 44, 343, 76}, SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_dlglevel2_layer3_image_arrow},
    },
};

#define DLGLEVEL2_BLOCK_TERMAGE(j) \
    1, &(svgui_dlglevel2_layer3_text_area[j]), 2, (svgui_dlglevel2_layer3_image_area[j])

static SVGUI_BLOCK_T  svgui_dlglevel2_blocks [] = 
{
    // layer 1 
    {0, TRUE, {0, 0, DEFAULT_DLGLEVEL2_WIDTH, DEFAULT_DLGLEVEL2_HEIGHT}, FALSE, 0, 0, NULL, 0, NULL},

    // layer 2
    {1, TRUE, {2, 10, 380, 75}, TRUE, 1, TABLESIZE(svgui_dlglevel2_layer2_text_area), svgui_dlglevel2_layer2_text_area, 0, NULL},
    {2, TRUE, {2, 76, 380, 662}, FALSE, 2, 0, NULL, 0, NULL},

    // layer 2
#if 0
    {3, TRUE, {12, 90, 370, 200}, FALSE, 3, 1, &(svgui_dlglevel2_layer3_text_area[0]), 0, NULL},
    {4, TRUE, {12, 203, 370, 313}, FALSE, 3, 1, &(svgui_dlglevel2_layer3_text_area[1]), 0, NULL},
    {5, TRUE, {12, 316, 370, 426}, FALSE, 3, 1, &(svgui_dlglevel2_layer3_text_area[2]), 0, NULL},
    {6, TRUE, {12, 429, 370, 539}, FALSE, 3, 1, &(svgui_dlglevel2_layer3_text_area[3]), 0, NULL},
    {7, TRUE, {12, 542, 370, 652}, FALSE, 3, 1, &(svgui_dlglevel2_layer3_text_area[4]), 0, NULL},
#else
    {DLGLEVEL2_DOOR_ID, TRUE, {12, 90, 370, 200}, TRUE, 4, DLGLEVEL2_BLOCK_TERMAGE(0)},
    {DLGLEVEL2_SEARCH_ID, TRUE, {12, 201, 370, 311}, TRUE, 3,  DLGLEVEL2_BLOCK_TERMAGE(1)},
    {DLGLEVEL2_NEWS_ID, TRUE, {12, 312, 370, 422}, TRUE, 3,  DLGLEVEL2_BLOCK_TERMAGE(2)},
    {DLGLEVEL2_NOVEL_ID, TRUE, {12, 423, 370, 533}, TRUE, 3,  DLGLEVEL2_BLOCK_TERMAGE(3)},
    {DLGLEVEL2_MAIL_ID, TRUE, {12, 534, 370, 644}, TRUE, 3,  DLGLEVEL2_BLOCK_TERMAGE(4)},
#endif
};


static SVGUI_PRDRD_BLOCK_T svgui_dlglevel2_prdrd_blocks [] = 
{
    {DEFAULT_DLGLEVEL2_WIDTH, DEFAULT_DLGLEVEL2_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE, 0, 0, 0, 0x000000, 0x000000, 0x000000, 5, 0x000000, 20, 0x000000},
    {378, 65, SVGUI_PRDRD_GRADIENT_VERT, 0, 25, 65, 0x777777, 0x040404, 0x0D0D0D, 0, 0x000000, 
        10, 0x000000},
    {378, 586, SVGUI_PRDRD_GRADIENT_VERT, 0, 291, 582, 0x181818, 0x0D0D0D, 0x181818, 0, 0x000000, 
        10, 0x000000},
    {358, 110, SVGUI_PRDRD_GRADIENT_VERT, 0, 55, 110, 0x313131, 0x131313, 0x000000, 2, 0x777777, 
        5, 0x000000},
    {358, 110, SVGUI_PRDRD_GRADIENT_VERT, 0, 55, 110, 0xC05100, 0x5E1D00, 0x210A00, 0, 0x0000000, 
        5, 0x000000},
};

static SVGUI_FONT_INFO_T svgui_dlglevel2_font_infos [] =
{
    {"fmhei", 18, "UTF-8"},
    {"fmsong", 20, "UTF-8"}
};

static SVGUI_HEADER_T svgui_dlglevel2_header =
{
    DEFAULT_DLGLEVEL2_WIDTH, DEFAULT_DLGLEVEL2_HEIGHT,
    0x000000, 0x000000,
    TABLESIZE(svgui_dlglevel2_prdrd_blocks), svgui_dlglevel2_prdrd_blocks,
    TABLESIZE(svgui_dlglevel2_font_infos), svgui_dlglevel2_font_infos,
    TABLESIZE(svgui_dlglevel2_blocks), svgui_dlglevel2_blocks,
};

static int sg_websit_dlglevel_width = DEFAULT_DLGLEVEL2_WIDTH;
static int sg_websit_dlglevel_height = DEFAULT_DLGLEVEL2_HEIGHT;

static SVGUI_BLOCK_I * GetSelectedBlock (HWND hWnd, int min_id, int max_id, int selected_prd_id)
{

    SVGUI_BLOCK_I *block = NULL;
    SVGUI_HEADER_I *header = NULL;
    int hotpot_id;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return NULL;

    if (max_id < min_id || min_id <0 || max_id <= 0 )
    {
        min_id = 0;
        max_id = header->nr_blocks;
    }

    for (hotpot_id = min_id; hotpot_id<=max_id; hotpot_id++) 
    {
        block = svgui_get_block(header, hotpot_id); 
        
        if (block == NULL)
            continue;

        if (block->idx_prdrd_block==selected_prd_id)
        {
            return block;
        }
    }

    return NULL;
}

SVGUI_BLOCK_I * GetDlgLevel2SelectedStatusByPos (HWND hWnd, int x_pos, int y_pos)
{
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return NULL;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if(block == NULL)
    {
//        SendMessage(hWnd, MSG_KEYUP, SCANCODE_ESCAPE, 0 );
        return NULL;
    }
    return block;
}

static void SetDlgLevel2SelectedStatusByPos (HWND hWnd, int x_pos, int y_pos)
{

    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I *old_block = NULL;
    SVGUI_BLOCK_I* block = NULL;

#ifdef MDTV_WEBSITE_DEBUG
    printf ("WebSite-Debug: xpos=%d, ypos=%d\n", x_pos, y_pos);
#endif
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    old_block = GetSelectedBlock(hWnd, -1, -1, 4);

    if (!block)
        return ;
   
    switch(block->id)
    {
        case 1:
            SendMessage(hWnd, MSG_KEYUP, SCANCODE_ESCAPE, 0 );
            return;
    }

    if (old_block != NULL)
    {
        old_block->idx_prdrd_block = 3;
        old_block->image_areas[1].is_visible = FALSE;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }

    block->idx_prdrd_block=4;
    block->image_areas[1].is_visible = TRUE;
    InvalidateRect (hWnd, &block->rc, TRUE);
}

void SetDlgLevel2SelectedStatusByNextId (HWND hWnd, BOOL DOWN)
{

    SVGUI_BLOCK_I *old_block = NULL;

    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;
    int nextid, idcount;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    old_block = GetSelectedBlock(hWnd, -1, -1, 4);


    idcount = DLGLEVEL2_MAX_ID - DLGLEVEL2_MIN_ID + 1;
    if (old_block != NULL)
    {
        if (DOWN)
            nextid = (old_block->id+1 - DLGLEVEL2_MIN_ID)%(idcount) + DLGLEVEL2_MIN_ID;
        else 
            nextid = (old_block->id-1 + idcount - DLGLEVEL2_MIN_ID)%idcount + DLGLEVEL2_MIN_ID;

        old_block->idx_prdrd_block = 3;
        old_block->image_areas[1].is_visible = FALSE;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }
    else 
    {
        if (DOWN)
            nextid = DLGLEVEL2_MIN_ID;
        else
            nextid = DLGLEVEL2_MAX_ID;
    }

    block = svgui_get_block (header, nextid);
    if (block == NULL)
        return;

    block->idx_prdrd_block=4;
    block->image_areas[1].is_visible = TRUE;
    InvalidateRect (hWnd, &block->rc, TRUE);
}


static const char *l3_title[]=
{
    "  门  户  ||  Portal",
    "  搜  索  ||  Search",
    "  新  闻  ||  News",
    "  小  说  ||  Novel",
    "  邮  箱  ||  Email"
};


static const char *label_portal[]=
{
    "网易",
    "搜狐",
    "新浪",
    "雅虎",
    "中华网",
    "凤凰网"
};

static const char *label_search[]=
{
    "百度",
    "搜狗",
    "Google",
    "Bing",
    "Yahoo"
};

static const char *label_news[]=
{
    "雅虎新闻",
    "新浪新闻",
    "腾讯新闻",
    "凤凰资讯",
    "搜狐新闻",
    "网易新闻"
};

static const char *label_novel[]=
{
    "起点中文网",
    "幻剑书盟",
    "世纪文学",
    "白鹿书院",
    "潇湘书院",
    "红袖添香"
};

static const char *label_email[]=
{
    "163邮箱",
    "126邮箱",
    "雅虎邮箱",
    "新浪邮箱",
    "搜狐邮箱",
    "Gmail邮箱"
};

static const char *url_portal[]=
{
    "http://www.163.com/index.html",
    "http://www.sohu.com",
    "http://www.sina.com.cn",
    "http://www.yahoo.com.cn",
    "http://www.china.com/zh_cn",
    "http://www.ifeng.com"
};

static const char *url_search[]=
{
    "http://www.baidu.com",
    "http://www.sogou.com",
    "http://www.google.com.cn",
    "http://www.bing.com",
    "http://cn.search.yahoo.com/websrch/"
};

static const char *url_news[]=
{
    "http://news.yahoo.cn/",
    "http://news.sina.com.cn/",
    "http://news.qq.com/",
    "http://news.ifeng.com/",
    "http://news.sohu.com/",
    "http://news.163.com/"
};

static const char *url_novel[]=
{
    "http://www.qidian.com/",
    "http://html.hjsm.tom.com/",
    "http://www.2100book.com/",
    "http://www.oklink.net/",
    "http://www.xxsy.net/",
    "http://www.hongxiu.com/"
};

static const char *url_email[]=
{
    "http://mail.163.com",
    "http://www.126.com",
    "http://mail.cn.yahoo.com/",
    "http://mail.sina.com.cn/",
    "http://mail.sohu.com/",
    "http://mail.google.com/"
};


SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel3_door_icon [] = 
{
    // 47x23-163.png 36x34-sohu.png  48x36-sina.png 53x30-yahoo.png 46x20-china.png 34x34-ifeng.png  //640 
    // 60x29-163.png 46x43-sohu.png  62x44-sina.png 67x38-yahoo.png 54x24-china.png 44x44-ifeng.png  // 800
    //
    {
        {47, 23, 32, "website/door/47x23-163.png"},
        {60, 29, 32, "website/door/60x29-163.png"},
        {68, 32, 32, "website/door/68x32-163.png"},
    }, 

    {
        {36, 34, 32, "website/door/36x34-sohu.png"},
        {46, 43, 32, "website/door/46x43-sohu.png"},
        {52, 49, 32, "website/door/52x49-sohu.png"},
    }, 

    {
        {48, 36, 32, "website/door/48x36-sina.png"},
        {62, 44, 32, "website/door/62x44-sina.png"},
        {71, 51, 32, "website/door/71x51-sina.png"},
    }, 

    {
        {53, 30, 32, "website/door/53x30-yahoo.png"},
        {67, 38, 32, "website/door/67x38-yahoo.png"},
        {76, 43, 32, "website/door/76x43-yahoo.png"},
    }, 

    {
        {46, 20, 32, "website/door/46x20-china.png"},
        {54, 24, 32, "website/door/54x24-china.png"},
        {60, 26, 32, "website/door/60x26-china.png"},
    }, 

    {
        {34, 34, 32, "website/door/34x34-ifeng.png"},
        {44, 44, 32, "website/door/44x44-ifeng.png"},
        {48, 48, 32, "website/door/48x48-ifeng.png"},
    }, 
};


SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel3_search_icon [] = 
{
    //49x19-baidu.png  54x16-sougou.png  54x21-google.png  53x21-bing.png  55x20-yahoo.png //640
    //63x24-baidu.png  62x18-sougou.png  66x25-google.png  66x27-bing.png  65x22-yahoo.png    //800
    //69x26-baidu.png  74x22-sougou.png  76x29-google.png  74x29-Bing.png 77x26-yahoo.png
    {
        {49, 19, 32, "website/search/49x19-baidu.png"},
        {63, 24, 32, "website/search/63x24-baidu.png"},
        {69, 26, 32, "website/search/69x26-baidu.png"},
    }, 

    {
        {54, 16, 32, "website/search/54x16-sougou.png"},
        {62, 18, 32, "website/search/62x18-sougou.png"},
        {74, 22, 32, "website/search/74x22-sougou.png"},
    }, 

    {
        {54, 21, 32, "website/search/54x21-google.png"},
        {66, 25, 32, "website/search/66x25-google.png"},
        {76, 29, 32, "website/search/76x29-google.png"},
    }, 

    {
        {53, 21, 32, "website/search/53x21-bing.png"},
        {66, 27, 32, "website/search/66x27-bing.png"},
        {74, 29, 32, "website/search/74x29-Bing.png"},
    }, 

    {
        {55, 20, 32, "website/search/55x20-yahoo.png"},
        {65, 22, 32, "website/search/65x22-yahoo.png"},
        {77, 26, 32, "website/search/77x26-yahoo.png"},
    }, 
};

SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel3_news_icon [] = 
{
    //50x30-yahoo.png 48x36-sina.png 32x33-qq.png  34x34-ifeng.png  36x34-sohu.png  47x23-163.png     //640x480
    //67x38-yahoo.png 62x44-sina.png 40x42-qq.png  44x44-ifeng.png  46x43-sohu.png  60x29-163.png          //800*600
    //76x43-yahoo.png 71x51-sina.png 45x46-qq.png  48x48-ifeng.png  52x49-sohu.png  68x32-163.png    
    //76x43yahoo.png  71x51 sina.png 45x46 qq.png  48x48 ifeng.png  52x49 sohu.png  68x32 163.png    
    {
        {50, 30, 32, "website/news/50x30-yahoo.png"},
        {67, 38, 32, "website/news/67x38-yahoo.png"},
        {76, 43, 32, "website/news/76x43-yahoo.png"},
    }, 

    {
        {48, 36, 32, "website/news/48x36-sina.png"},
        {62, 44, 32, "website/news/62x44-sina.png"},
        {71, 51, 32, "website/news/71x51-sina.png"},
    }, 

    {
        {32, 33, 32, "website/news/32x33-qq.png"},
        {40, 42, 32, "website/news/40x42-qq.png"},
        {45, 46, 32, "website/news/45x46-qq.png"},
    }, 

    {
        {34, 34, 32, "website/news/34x34-ifeng.png"},
        {44, 44, 32, "website/news/44x44-ifeng.png"},
        {48, 48, 32, "website/news/48x48-ifeng.png"},
    }, 

    {
        {36, 34, 32, "website/news/36x34-sohu.png"},
        {46, 43, 32, "website/news/46x43-sohu.png"},
        {52, 49, 32, "website/news/52x49-sohu.png"},
    }, 

    {
        {47, 23, 32, "website/news/47x23-163.png"},
        {60, 29, 32, "website/news/60x29-163.png"},
        {68, 32, 32, "website/news/68x32-163.png"},
    }, 
};

SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel3_novel_icon [] = 
{
    //39x39-qidian.png 38x37-huanjian.png 55x24-shiji.png 39x36-bailu.png 34x49-xiaoxian.png 44x45-hongxiu.png  
    //52x49-qidian.png 47x45-huanjian.png 69x34-shiji.png 54x49-bailu.png 45x49-xiaoxian.png 61x48-hongxiu.png  
    //57x53-qidian.png 54x54-huanjian.png 79x32-shiji.png 55x50-bailu.png 46x53-xiaoxian.png 64x49-hongxiu.png  
    //
    {
        {39, 39, 32, "website/novel/39x39-qidian.png"},
        {52, 49, 32, "website/novel/52x49-qidian.png"},
        {57, 53, 32, "website/novel/57x53-qidian.png"},
    }, 

    {
        {38, 37, 32, "website/novel/38x37-huanjian.png"},
        {47, 45, 32, "website/novel/47x45-huanjian.png"},
        {54, 54, 32, "website/novel/54x54-huanjian.png"},
    }, 

    {
        {55, 24, 32, "website/novel/55x24-shiji.png"},
        {69, 34, 32, "website/novel/69x34-shiji.png"},
        {79, 32, 32, "website/novel/79x32-shiji.png"},
    }, 

    {
        {39, 36, 32, "website/novel/39x36-bailu.png"},
        {54, 49, 32, "website/novel/54x49-bailu.png"},
        {55, 50, 32, "website/novel/55x50-bailu.png"},
    }, 

    {
        {34, 49, 32, "website/novel/34x49-xiaoxian.png"},
        {45, 49, 32, "website/novel/45x49-xiaoxian.png"},
        {46, 53, 32, "website/novel/46x53-xiaoxian.png"},
    }, 

    {
        {44, 45, 32, "website/novel/64x49-hongxiu.png"},
        {61, 48, 32, "website/novel/61x48-hongxiu.png"},
        {64, 49, 32, "website/novel/64x49-hongxiu.png"},
    }, 
};

SVGUI_IMAGE_T_4ARRAY  svgui_dlglevel3_email_icon [] = 
{
    //47x25-163.png  51x24-126.png  55x20-yahoo.png 48x36-sina.png 33x33-sohu.png 53x13-gmail.png   //640
    //57x31-163.png  61x28-126.png  65x22-yahoo.png 62x44-sina.png 43x43-sohu.png 65x17-gmail.png   //800
    //65x35-163.png  71x32-126.png  76x26-yahoo.png 71x51-sina.png 47x47-sohu.png 75x19-gmail.png  
    {
        {47, 25, 32, "website/email/47x25-163.png"},
        {57, 31, 32, "website/email/57x31-163.png"},
        {65, 35, 32, "website/email/65x35-163.png"},
    }, 

    {
        {51, 24, 32, "website/email/51x24-126.png"},
        {61, 28, 32, "website/email/61x28-126.png"},
        {71, 32, 32, "website/email/71x32-126.png"},
    }, 

    {
        {55, 20, 32, "website/email/55x20-yahoo.png"},
        {65, 22, 32, "website/email/65x22-yahoo.png"},
        {76, 26, 32, "website/email/76x26-yahoo.png"},
    }, 

    {
        {48, 36, 32, "website/email/48x36-sina.png"},
        {62, 44, 32, "website/email/62x44-sina.png"},
        {71, 51, 32, "website/email/71x51-sina.png"},
    }, 

    {
        {33, 33, 32, "website/email/33x33-sohu.png"},
        {43, 43, 32, "website/email/43x43-sohu.png"},
        {47, 47, 32, "website/email/47x47-sohu.png"},
    }, 

    {
        {53, 13, 32, "website/email/53x13-gmail.png"},
        {65, 17, 32, "website/email/65x17-gmail.png"},
        {75, 19, 32, "website/email/75x19-gmail.png"},
    }, 
};

typedef struct _DLGLEVEL3_HEADER_INFO 
{
    int width;
    int height;
    const char *title;
    int title_height;
    int menucount;
    SVGUI_IMAGE_T_4ARRAY *menuimage;
    const char **menutitle;
}DLGLEVEL3_HEADER_INFO;

static DLGLEVEL3_HEADER_INFO dlglevel3_header_info [] = 
{
    //door 
    {330, 455, l3_title[0], 37, 6, svgui_dlglevel3_door_icon,  (const char **)(label_portal)},

    //search
    {330, 390, l3_title[1], 37, 5, svgui_dlglevel3_search_icon,  (const char **)(label_search)},

    //news 
    {330, 455, l3_title[2], 37, 6, svgui_dlglevel3_news_icon,  (const char **)(label_news)},

    //novel
    {330, 455, l3_title[3], 37, 6, svgui_dlglevel3_novel_icon,  (const char **)(label_novel)},

    //email
    {330, 455, l3_title[4], 37, 6, svgui_dlglevel3_email_icon,  (const char **)(label_email)},

};
static RECT dlglevel3_rect [] = 
{
    //door
    {550, 100, 880, 555},
    //search
    {550, 100, 880, 490},
    //news
    {550, 150, 880, 605},
    //novel
    {550, 200, 880, 655},
    //email
    {550, 250, 880, 704},
};

#if 0
static RECT dlglevel3_rect [] = 
{
    //door
    {550, 100, 880, 535},
    //search
    {550, 100, 880, 466},
    //news
    {550, 150, 880, 585},
    //novel
    {550, 200, 880, 635},
    //email
    {550, 250, 880, 685},
};

#endif
static SVGUI_FONT_INFO_T svgui_dlglevel3_font_infos [] =
{
    {"fmhei", 18, "UTF-8"},
    {"fmsong", 20, "UTF-8"}
};

#define DLGLEVEL3BROAD 5 
#define DLGLEVEL3VSPACE 2
#define DLGLEVEL3HSPACE 0

static SVGUI_PRDRD_BLOCK_T * CreateWebsiteDlgLevel3PRDBlock (int width, int height,int title_width,  int title_height, int menuheight)
{
    SVGUI_PRDRD_BLOCK_T *prd_block = (SVGUI_PRDRD_BLOCK_T *)malloc(sizeof (SVGUI_PRDRD_BLOCK_T) *5 );
    if (prd_block == NULL)
    {
        fprintf (stderr, "error: alloc mem failded \n ");
        return NULL;
    }

    // the background prd block
    prd_block[0].width = width;
    prd_block[0].height = height;
    prd_block[0].gradient_type = SVGUI_PRDRD_GRADIENT_VERT;
    prd_block[0].pos_c1 = 0;
    prd_block[0].pos_c2 = prd_block[0].height *0.57;
    prd_block[0].pos_c3 = prd_block[0].height;
#if 1
    prd_block[0].c1 = 0x181818;
    prd_block[0].c2 = 0x373737;
    prd_block[0].c3 = 0x000000;
#else
    prd_block[0].c1 = 0xFF0000;
    prd_block[0].c2 = 0x00FF00;
    prd_block[0].c3 = 0x000000;
#endif
    prd_block[0].border_width = DLGLEVEL3BROAD;
    prd_block[0].border_color = 0x8b8b8b;
    prd_block[0].corner = 8; 
    //prd_block[0].corner = 11; 
    prd_block[0].color_bk = 0x000000; 


    // the title prd block
    prd_block[1].width = title_width;
    prd_block[1].height = title_height;
    prd_block[1].gradient_type = SVGUI_PRDRD_GRADIENT_VERT;
    prd_block[1].pos_c1 = 0;
    prd_block[1].pos_c2 = prd_block[1].height *0.36;
    prd_block[1].pos_c3 = prd_block[1].height;
#if 1
    prd_block[1].c1 = 0x363636;
    prd_block[1].c2 = 0x222222;
    prd_block[1].c3 = 0x000000;
#else
    prd_block[1].c1 = 0xFF0000;
    prd_block[1].c2 = 0x00FF00;
    prd_block[1].c3 = 0x808080;
#endif
    prd_block[1].border_width = 0;
    prd_block[1].border_color = 0x8b8b8b;
    prd_block[1].corner = 1; 
    //prd_block[1].corner = 11; 
    prd_block[1].color_bk = 0xFFFF00; 

    // the background of  menu 
    prd_block[2].width = title_width;
    prd_block[2].height = height - title_height - DLGLEVEL3BROAD - 2*DLGLEVEL3VSPACE;
    prd_block[2].gradient_type = SVGUI_PRDRD_GRADIENT_VERT;
    prd_block[2].pos_c1 = 0;
    prd_block[2].pos_c2 = prd_block[2].height *0.57;
    prd_block[2].pos_c3 = prd_block[2].height;
#if 1
    prd_block[2].c1 = 0x181818;
    prd_block[2].c2 = 0x373737;
    prd_block[2].c3 = 0x000000;
#else
    prd_block[2].c1 = 0xFF0000;
    prd_block[2].c2 = 0x00FF00;
    prd_block[2].c3 = 0x000000;
#endif
    prd_block[2].border_width = 5;
    prd_block[2].border_color = 0x8b8b8b;
    prd_block[2].corner = 0; 
    //prd_block[2].corner = 11; 
    prd_block[2].color_bk = 0x000000; 

    // the normal menu prd block
    prd_block[3].width = title_width-4;
    prd_block[3].height = menuheight;
    prd_block[3].gradient_type = SVGUI_PRDRD_GRADIENT_VERT;
    prd_block[3].pos_c1 = 0;
    prd_block[3].pos_c2 = prd_block[3].height *0.57;
    prd_block[3].pos_c3 = prd_block[3].height;
    prd_block[3].c1 = 0x181818;
    prd_block[3].c2 = 0x373737;
    prd_block[3].c3 = 0x000000;
    prd_block[3].border_width = 2;
    prd_block[3].border_color = 0x8b8b8b;
    prd_block[3].corner = 0; 
    //prd_block[3].corner = 11; 
    prd_block[3].color_bk = 0x000000; 


    // the selected menu prd block
    prd_block[4].width = title_width-4;
    prd_block[4].height = menuheight;
    prd_block[4].gradient_type = SVGUI_PRDRD_GRADIENT_VERT;
    prd_block[4].pos_c1 = 0;
    prd_block[4].pos_c2 = prd_block[4].height *0.50;
    prd_block[4].pos_c3 = prd_block[4].height;
    prd_block[4].c1 = 0xE69D00;
    prd_block[4].c2 = 0xBA6000;
    prd_block[4].c3 = 0x000000;
    prd_block[4].border_width = 0;
    prd_block[4].border_color = 0x000000;
    prd_block[4].corner = 0; 
    //prd_block[4].corner = 11; 
    prd_block[4].color_bk = 0x000000; 


    return prd_block;
}


static SVGUI_HEADER_T * CreateWebsiteDlgLevel3SvguiHeader ( int width, int height, const char *title,
        int title_height, int menucount, SVGUI_IMAGE_T_4ARRAY *menuimage, const char **menutitle)
{
    int menuwidth =width - 2*DLGLEVEL3BROAD;
    int menuheight =(height - title_height- 2*DLGLEVEL3BROAD - (menucount+1)*DLGLEVEL3VSPACE)/menucount;
    SVGUI_PRDRD_BLOCK_T *prd_block = NULL;
    SVGUI_BLOCK_T *block = NULL;
    SVGUI_TEXT_AREA_T *title_text_area = NULL;
    SVGUI_TEXT_AREA_T *menu_text_area = NULL;
    SVGUI_IMAGE_AREA_T *menu_image_area = NULL;
    SVGUI_HEADER_T * header = NULL;
    int i = 0;

    prd_block = CreateWebsiteDlgLevel3PRDBlock (width, height, menuwidth, title_height, menuheight);
    block = (SVGUI_BLOCK_T *) malloc (sizeof(SVGUI_BLOCK_T) * (3+menucount));

    // the background svgui_block 
    block[0].id = 0;
    block[0].is_visible = TRUE;
    block[0].rc.left = 0; 
    block[0].rc.top = 0;
    block[0].rc.right = width; 
    block[0].rc.bottom = height;
    block[0].is_hotspot = FALSE;
    block[0].idx_prdrd_block = 0;
    block[0].nr_text_areas = 0;
    block[0].text_areas = NULL;
    block[0].nr_image_areas = 0;
    block[0].image_areas = NULL;


    // the title svgui_block 

    block[1].id = 1;
    block[1].is_visible = TRUE;
    block[1].rc.left = DLGLEVEL3BROAD ; 
    block[1].rc.top = DLGLEVEL3BROAD;
    block[1].rc.right = block[1].rc.left + menuwidth; 
    block[1].rc.bottom = block[1].rc.top + title_height;
    block[1].is_hotspot = TRUE;
    block[1].idx_prdrd_block = 1;
    block[1].nr_text_areas = 0;
    block[1].text_areas = NULL;
    block[1].nr_image_areas = 0;
    block[1].image_areas = NULL;
    if (title != NULL)
    {
        title_text_area = (SVGUI_TEXT_AREA_T *)malloc (sizeof (SVGUI_TEXT_AREA_T ));
        title_text_area->id =1;
        title_text_area->is_visible = TRUE;
        title_text_area->rc.left = block[1].rc.left + 12;
        title_text_area->rc.top = 0;
        title_text_area->rc.right = block[1].rc.right;
        title_text_area->rc.bottom = block[1].rc.bottom;
        title_text_area->align = SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER;
        title_text_area->color = 0xFFFFFF;
        title_text_area->idx_font = 1;
        title_text_area->text =(char *) title;
        block[1].nr_text_areas = 1;
        block[1].text_areas = title_text_area;
    }

    // the background menu svgui_block
    block[2].id = 2;
    block[2].is_visible = TRUE;
    block[2].rc.left = DLGLEVEL3BROAD ; 
    block[2].rc.top = block[1].rc.bottom + DLGLEVEL3VSPACE;
    block[2].rc.right = block[2].rc.left + menuwidth; 
    block[2].rc.bottom =  height -  DLGLEVEL3VSPACE;
    block[2].is_hotspot = FALSE;
    block[2].idx_prdrd_block = 2;

    block[2].nr_text_areas = 0;
    block[2].text_areas = NULL;
    block[2].nr_image_areas = 0;
    block[2].image_areas = NULL;

    for (i=3; i<(menucount+3); i++)
    {
        block[i].id = i;
        block[i].is_visible = TRUE;
        block[i].rc.left = DLGLEVEL3BROAD ; 
        if (i== 3)
            block[i].rc.top = block[i-1].rc.top + DLGLEVEL3VSPACE;
        else
            block[i].rc.top = block[i-1].rc.bottom + DLGLEVEL3VSPACE;
        block[i].rc.right = block[i].rc.left + menuwidth; 
        block[i].rc.bottom = block[i].rc.top +  menuheight;
        block[i].is_hotspot = TRUE;
        if (i== 3)
            block[i].idx_prdrd_block = 4;
        else
            block[i].idx_prdrd_block = 3;

        block[i].nr_text_areas = 0;
        block[i].text_areas = NULL;
        block[i].nr_image_areas = 0;
        block[i].image_areas = NULL;

        if (menuimage != NULL)
        {
            menu_image_area = (SVGUI_IMAGE_AREA_T *)malloc (sizeof (SVGUI_IMAGE_AREA_T));
            menu_image_area->id = 1;
            menu_image_area->is_visible = TRUE;
            menu_image_area->rc.left =  5;
            menu_image_area->rc.top =  0;
            menu_image_area->rc.right = 100; // menu_image_area->rc.left + (menuimage[i-3])[0].widht + 1 ;
            menu_image_area->rc.bottom =  RECTH(block[i].rc);
            menu_image_area->fill_way = SVGUI_IMAGE_FILL_WAY_CENTER;
            menu_image_area->nr_images = 1; //MAX_IMAGE_NUM;
            menu_image_area->images = (menuimage[i-3]);

            block[i].nr_image_areas = 1;
            block[i].image_areas = menu_image_area;
        }

        if (menutitle != NULL)
        {
            menu_text_area = (SVGUI_TEXT_AREA_T *)malloc (sizeof (SVGUI_TEXT_AREA_T ));
            menu_text_area->id =1;
            menu_text_area->is_visible = TRUE;
            menu_text_area->rc.left =  100;
            menu_text_area->rc.top = 0;
            menu_text_area->rc.right = RECTW(block[i].rc);
            menu_text_area->rc.bottom = RECTH(block[i].rc);
            menu_text_area->align = SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER;
            menu_text_area->color = 0xFFFFFF;
            menu_text_area->idx_font = 1;
            menu_text_area->text =(char *)menutitle[i-3];
            block[i].nr_text_areas = 1;
            block[i].text_areas = menu_text_area;
        }

    }

    header = (SVGUI_HEADER_T *) malloc (sizeof (SVGUI_HEADER_T));
    header->width = width;
    header->height = height;
    header->color_bk = 0x000000;
    header->color_key = 0x000000;
    header->nr_prdrd_blocks = 5;
    header->prdrd_blocks = prd_block;
    header->nr_font_infos = 2;
    header->font_infos = svgui_dlglevel3_font_infos;
    header->nr_blocks = 3 + menucount; 
    header->blocks = block;

#ifdef MDTV_WEBSITE_DEBUG 
    for (int i=0; i<header->nr_prdrd_blocks; i++)
        printf ("Debug: prd_block index=%d width=%d, height=%d\n",i,  header->prdrd_blocks[i].width, header->prdrd_blocks[i].height);

    for (int i=0; i<header->nr_blocks; i++)
        printf ("Debug: block index=%d,  width=%d, height=%d, idx_prdrd_block=%d\n",i,   RECTW(header->blocks[i].rc), RECTH(header->blocks[i].rc), header->blocks[i].idx_prdrd_block);
#endif
    return header;
}


void SetDlgLevel3SelectedStatusByPos (HWND hWnd, int x_pos, int y_pos)
{

    SVGUI_BLOCK_I *old_block = NULL;
    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;

#ifdef MDTV_WEBSITE_DEBUG
    printf ("WebSite-Level3-Debug: xpos=%d, ypos=%d\n", x_pos, y_pos);
#endif
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    block = svgui_get_block_by_point (header, x_pos, y_pos);
    old_block = GetSelectedBlock(hWnd, -1, -1, 4);

    if (block == NULL)
    {
        return ;
    }

    if (old_block != NULL)
    {
        old_block->idx_prdrd_block = 3;
        InvalidateRect (hWnd, &old_block->rc, TRUE);

    }

    block->idx_prdrd_block=4;
    InvalidateRect (hWnd, &block->rc, TRUE);
#if 0
    int select_id = -1; 
    const char **url = NULL;

    select_id = block->id;

    switch (sg_dlglevel2_selected_id )
    {
        case 0:
            printf ("door3\n");
            url = url_portal;
            break;

        case 1:
            url = url_search;
            printf ("search\n");
            break;
        case 2:
            url = url_news;
            break;
        case 3:
            url = url_novel;
            break;
        case 4:
            url = url_email;
            break;
    }
#ifdef MDTV_WEBSITE_DEBUG
    printf ("The url is: %s\n", url[select_id - 3]);
#endif
    CloseWebsiteAndOpenBrowser (hWnd, url[select_id - 3]);
#endif
}

void SetDlgLevel3SelectedStatusByNextId (HWND hWnd, BOOL DOWN, int min_id, int max_id)
{

    SVGUI_BLOCK_I *old_block = NULL;

    SVGUI_HEADER_I *header = NULL;
    SVGUI_BLOCK_I* block = NULL;
    int nextid, idcount;

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return;

    old_block = GetSelectedBlock(hWnd, -1, -1, 4);


    idcount = max_id - min_id + 1;
    if (old_block != NULL)
    {
        if (DOWN)
            nextid = (old_block->id+1 - min_id)%(idcount) + min_id;
        else 
            nextid = (old_block->id-1 + idcount - min_id)%idcount + min_id;

        old_block->idx_prdrd_block = 3;
        InvalidateRect (hWnd, &old_block->rc, TRUE);
    }
    else 
    {
        if (DOWN)
            nextid = min_id;
        else
            nextid = max_id;
    }

    block = svgui_get_block (header, nextid);
    if (block == NULL)
        return;

    block->idx_prdrd_block=4;
    InvalidateRect (hWnd, &block->rc, TRUE);
}

static SVGUI_HEADER_T * CreateWebsiteDlgLevel3SvguiHeaderByDlgheaderinfo (DLGLEVEL3_HEADER_INFO *header_info)
{
    int width = header_info->width;
    int height = header_info->height; 
    const char *title = header_info->title;
    int title_height = header_info->title_height;
    int menucount = header_info->menucount;
    SVGUI_IMAGE_T_4ARRAY *menuimage = header_info->menuimage;
    const char **menutitle = header_info->menutitle;

    return CreateWebsiteDlgLevel3SvguiHeader (width, height, title, title_height, 
            menucount, menuimage, menutitle);
}

static int on_update_ui_secdc (HWND hWnd, HDC secondary_dc,
        HDC real_dc, const RECT* secondary_rc, const RECT* real_rc,
        const RECT* main_update_rc)
{
    SVGUI_HEADER_I *svgui_header;
    /* erase background by using the content in g_dc_bkgnd */
    //BitBlt (g_dc_bkgnd, toolbar_rect.left, toolbar_rect.top, RECTW(toolbar_rect), RECTH(toolbar_rect), real_dc, 0, 0, 0);

    /* blending g_dc_uisec to the real dc */
    svgui_header = (SVGUI_HEADER_I*)GetWindowAdditionalData( hWnd );
    SetMemDCColorKey(secondary_dc, MEMDC_FLAG_SRCCOLORKEY, svgui_header->pixel_key);
    //SetMemDCAlpha (secondary_dc, MEMDC_FLAG_SRCALPHA, 240);
    //BitBlt (secondary_dc,0,0,RECTW(*secondary_rc),RECTH(*secondary_rc), real_dc,0,0,0);
    //BitBlt (secondary_dc,0,0,0,0, real_dc,0,0,0);
    BitBlt (secondary_dc,secondary_rc->left,secondary_rc->top,RECTWP(secondary_rc),RECTHP(secondary_rc), 
            real_dc,real_rc->left,real_rc->top,0);
    //BitBlt (secondary_dc,0,0,RECTW(*secondary_rc),RECTH(*secondary_rc), real_dc,0,0,0);
    //BitBlt (secondary_dc,0,0,RECTW(*secondary_rc),RECTH(*secondary_rc), HDC_SCREEN,0,0,0);
    return 0;
}

static void WebsiteAnimageCloseWindow (HWND hWnd)
{
    SendMessage (hWnd, MSG_CLOSE, 0, 0);
}

#define DEFAULT_ARROW_WIDTH 88
#define DEFAULT_ARROW_HEIGHT 80

SVGUI_IMAGE_T  svgui_image_arrow[] = 
{
    {69, 65, 32, "website/arrow.png"},
};

#if 0
static SVGUI_IMAGE_AREA_T svgui_arrow_image_area =  {0, TRUE, {0, 0, DEFAULT_ARROW_WIDTH, DEFAULT_ARROW_HEIGHT},\
    SVGUI_IMAGE_FILL_WAY_CENTER, 1, svgui_image_arrow};
static SVGUI_PRDRD_BLOCK_T svgui_arrow_prdrd_blocks [] = 
{
    {DEFAULT_ARROW_WIDTH, DEFAULT_ARROW_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE, 0, 0, 0, 0x000000, 0x000000, 0x000000, 0, 0x000000, 0, 0x000000},
};

static SVGUI_BLOCK_T  svgui_arrow_blocks [] = 
{
    // layer 1 
    {0, TRUE, {0, 0, DEFAULT_ARROW_WIDTH, DEFAULT_ARROW_HEIGHT}, FALSE, 0, 0, NULL, 0, NULL},

    {1, TRUE, {0, 0, DEFAULT_ARROW_WIDTH, DEFAULT_ARROW_HEIGHT}, FALSE, 0, 0, NULL, 1, &svgui_arrow_image_area},
};
#endif
#if 0
static SVGUI_HEADER_T svgui_arrow_header =
{
    DEFAULT_ARROW_WIDTH, DEFAULT_ARROW_HEIGHT,
    0x000000, 0x000000,
    TABLESIZE(svgui_arrow_prdrd_blocks), svgui_arrow_prdrd_blocks,
    TABLESIZE(svgui_dlglevel2_font_infos), svgui_dlglevel2_font_infos,
    TABLESIZE(svgui_arrow_blocks), svgui_arrow_blocks,
};

static int WebsiteArrowProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{

    switch (message) 
    {
        case MSG_CREATE:
            {
#ifdef MDTV_WEBSITE_DEBUG
                printf ("enter  websit proc\n");
#endif

                MAINWINCREATE  *pCreateInfo;
                pCreateInfo = (MAINWINCREATE  *)(lParam);
                if (pCreateInfo != NULL)
                {
                    HDC secdc =  alloc_subdc_from_uisec(pCreateInfo->lx, pCreateInfo->ty, 
                            pCreateInfo->rx - pCreateInfo->lx ,  pCreateInfo->by-pCreateInfo->ty);
                    //SetSecondaryDC (hWnd, secdc, ON_UPDSECDC_DEFAULT);
                    SetSecondaryDC (hWnd, secdc, on_update_ui_secdc);
                }
                break;
            }
        case MSG_CLOSE:
            {
                RECT dstrc;
                GetWindowRect (hWnd, &dstrc);
                //                AnimationMoveWndLeftRight (hWnd, dstrc, MOVE_LEFT, 100, 10);
                //                AnimationMoveWndLeftRight (hWnd, dstrc, MOVE_LEFT, 50, 5);
                DestroyMainWindow (hWnd);
#if HAVE_ARROW_WND
                sg_arrow_hwnd = NULL;
#endif
                break;
            }
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}
#endif
#if HAVE_ARROW_WND
int CreateDlgArrowWindow (void)
{
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer (NAME_DEF_LAYER , "svgui" , 0 , 0);
#endif
    float scale;
    scale = (1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT)/(1.0*RECTW(g_rcScr)/RECTH(g_rcScr));
    if(scale>1.0){
        scale = 1.0;
    }

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST |WS_EX_TOOLWINDOW  ;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = WebsiteArrowProc;
    CreateInfo.lx = WEBSITE_ARROW_WND_X*RECTW(g_rcScr)/DEFAULT_SCREEN_WIDTH;
    CreateInfo.ty = WEBSITE_ARROW_WND_Y*RECTH(g_rcScr)/DEFAULT_SCREEN_HEIGHT;
    CreateInfo.rx = CreateInfo.lx + WEBSITE_ARROW_WND_WIDTH;
    CreateInfo.by = CreateInfo.ty + WEBSITE_ARROW_WND_HEIGHT;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)&svgui_arrow_header;
    CreateInfo.hHosting = g_hMainWnd;

    sg_arrow_hwnd = CreateMainWindow (&CreateInfo);

    if (sg_arrow_hwnd == HWND_INVALID)
        return -1;

    ShowWindow(sg_arrow_hwnd, SW_SHOWNORMAL);

    return 0;
}
#endif

static void CloseWebsiteAndOpenBrowser (HWND hWnd, const char *url)
{
    WebsiteAnimageCloseWindow (hWnd);
#if HAVE_ARROW_WND
    if (sg_arrow_hwnd)
    {
        ShowWindow (sg_arrow_hwnd, SW_HIDE);
        WebsiteAnimageCloseWindow (sg_arrow_hwnd);
    }
    sg_arrow_hwnd = NULL;
#endif

    if (g_website_hwnd)
        WebsiteAnimageCloseWindow (g_website_hwnd);
    g_website_hwnd = NULL;
    mdtv_CreateWebSiteNavigate (url);
}

static int WebsiteDlgLevel3Proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{

    switch (message) 
    {
        case MSG_CREATE:
            {
#ifdef MDTV_WEBSITE_DEBUG
                printf ("enter  websit proc\n");
#endif

                MAINWINCREATE  *pCreateInfo;
                pCreateInfo = (MAINWINCREATE  *)(lParam);
                if (pCreateInfo != NULL)
                {
                    HDC secdc =  alloc_subdc_from_uisec(pCreateInfo->lx, pCreateInfo->ty, 
                            pCreateInfo->rx - pCreateInfo->lx ,  pCreateInfo->by-pCreateInfo->ty);
                    SetSecondaryDC (hWnd, secdc, ON_UPDSECDC_DEFAULT);
                }
            }
            break;

        case MSG_LBUTTONDOWN:
#if 0
            SetDlgLevel3SelectedStatusByPos(hWnd,  LOWORD (lParam), HIWORD (lParam));
#else
            {
                SVGUI_BLOCK_I *old_block = NULL;
                SVGUI_BLOCK_I *selected_block = NULL;

                old_block =  GetSelectedBlock (hWnd, -1, -1, 4);
                selected_block = GetDlgLevel2SelectedStatusByPos(hWnd,  LOWORD (lParam), HIWORD (lParam));
                
                if( !selected_block)
                {
                    return -1;
                }

                if(selected_block->id == 1 )
                {
                    SendMessage(hWnd, MSG_KEYUP, SCANCODE_ESCAPE, 0 );
                    return 0;
                }

                if ( old_block == selected_block  )
                {
                    int select_id = -1; 
                    const char **url = NULL;

                    SVGUI_HEADER_I *header3 = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);

                    if (header3 == NULL)
                    {
                        return -1;
                    }

                    SVGUI_BLOCK_I* block3 = svgui_get_block_by_point (header3, LOWORD (lParam), HIWORD (lParam));

                    if (!block3)
                    {
                        return -1;
                    }
                    select_id = block3->id;

                    switch (sg_dlglevel2_selected_id )
                    {
                        case 0:
                            printf ("door3\n");
                            url = url_portal;
                            break;

                        case 1:
                            url = url_search;
                            printf ("search\n");
                            break;
                        case 2:
                            url = url_news;
                            break;
                        case 3:
                            url = url_novel;
                            break;
                        case 4:
                            url = url_email;
                            break;
                    }
#ifdef MDTV_WEBSITE_DEBUG
                    printf ("The url is: %s\n", url[select_id - 3]);
#endif
                    CloseWebsiteAndOpenBrowser (hWnd, url[select_id - 3]);
                }
                else
                {
                    SetDlgLevel3SelectedStatusByPos(hWnd,  LOWORD (lParam), HIWORD (lParam));
                }
            }
#endif
            break;
#if 0
        case MSG_LBUTTONDBLCLK:
            {
                int select_id = -1; 
                const char **url = NULL;

                SVGUI_HEADER_I *header3 = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
                
                if (header3 == NULL)
                {
                    return -1;
                }

                SVGUI_BLOCK_I* block3 = svgui_get_block_by_point (header3, LOWORD (lParam), HIWORD (lParam));

                if (block3 == NULL)
                {
                    SendMessage(hWnd, MSG_KEYUP, SCANCODE_ESCAPE, 0 );
                    return -1;
                }

                select_id = block3->id;

                switch (sg_dlglevel2_selected_id )
                {
                    case 0:
                        printf ("door3\n");
                        url = url_portal;
                        break;

                    case 1:
                        url = url_search;
                        printf ("search\n");
                        break;
                    case 2:
                        url = url_news;
                        break;
                    case 3:
                        url = url_novel;
                        break;
                    case 4:
                        url = url_email;
                        break;
                }
#ifdef MDTV_WEBSITE_DEBUG
                printf ("The url is: %s\n", url[select_id - 3]);
#endif
                CloseWebsiteAndOpenBrowser (hWnd, url[select_id - 3]);
            }

            break;
#endif   
        case MSG_KEYUP :
            switch (wParam)
            {
                case SCANCODE_CURSORBLOCKUP:
                    {
                        int min_id = 3;
                        int max_id =  dlglevel3_header_info[sg_dlglevel2_selected_id].menucount + min_id -1;
                        SetDlgLevel3SelectedStatusByNextId (hWnd, FALSE, min_id, max_id);
                    }
                    break;

                case SCANCODE_CURSORBLOCKDOWN:
                    {
                        int min_id = 3;
                        int max_id =  dlglevel3_header_info[sg_dlglevel2_selected_id].menucount + min_id -1;
                        SetDlgLevel3SelectedStatusByNextId (hWnd, TRUE, min_id, max_id);
                    }
                    break;

                case SCANCODE_ESCAPE:
                case SCANCODE_CURSORBLOCKLEFT:
                    SendMessage (hWnd, MSG_CLOSE, 0, 0);
#if HAVE_ARROW_WND
                    if (sg_arrow_hwnd)
                    {
                        SendMessage (sg_arrow_hwnd, MSG_CLOSE, 0, 0);
                        sg_arrow_hwnd = NULL;
                    }
#endif
                    SetActiveWindow (g_website_hwnd);
                    break;

                case SCANCODE_ENTER:
                    {
                        SVGUI_BLOCK_I * block = GetSelectedBlock (hWnd, -1, -1, 4);
                        int select_id = -1; 
                        const char **url = NULL;

                        if (block == NULL)
                            break;
                        select_id = block->id;
                        switch (sg_dlglevel2_selected_id )
                        {
                            case 0:
                                printf ("door\n");
                                url = url_portal;
                                break;
                            case 1:
                                url = url_search;
                                printf ("search\n");
                                break;
                            case 2:
                                url = url_news;
                                break;
                            case 3:
                                url = url_novel;
                                break;
                            case 4:
                                url = url_email;
                                break;
                        }

                        CloseWebsiteAndOpenBrowser (hWnd, url[select_id - 3]);
                    }
                    break;
            }
            break;
        case MSG_CLOSE:
            {
                RECT dstrc;
                GetWindowRect (hWnd, &dstrc);
                //                AnimationMoveWndLeftRight (hWnd, dstrc, MOVE_LEFT, 100, 10);
                AnimationMoveWndLeftRight (hWnd, dstrc, MOVE_LEFT, 50, 4);
                DestroyMainWindow (hWnd);
                sg_dlglevel3_hwnd = NULL;
                create_dlglevel3 = 0;
            }
            break;
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}

HWND CreateDlgLevel3 (HWND hParent,const char *title , int left, int top, int width, int height, SVGUI_HEADER_T *header)
{
    printf("CreateDlgLevel3\n");
    HWND hMainWnd = HWND_INVALID;
    MAINWINCREATE CreateInfo;
    RECT dstrc;

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;
    CreateInfo.spCaption = title;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = WebsiteDlgLevel3Proc;
    CreateInfo.lx = left;
    CreateInfo.ty = top;
    CreateInfo.rx = CreateInfo.lx + width;
    CreateInfo.by = CreateInfo.ty + height;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)(header);
    CreateInfo.hHosting = hParent;

    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return NULL;


    EnableWindow(hParent, FALSE);
    ShowWindow (hMainWnd, SW_SHOWNORMAL);
    dstrc.left = CreateInfo.lx;
    dstrc.top = CreateInfo.ty;
    dstrc.right = CreateInfo.rx;
    dstrc.bottom = CreateInfo.by;
    AnimationMoveWndLeftRight( hMainWnd, dstrc, MOVE_RIGHT, 50, 10 );
    return hMainWnd;
}

//static   HDC hd;
static int WebsiteDlgLevel2Proc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    int selected_id;
    float scale;
    float scale1;
    switch (message) 
    {
        case MSG_CREATE:
            {
#ifdef MDTV_WEBSITE_DEBUG
                printf ("enter websit Level2 proc\n");
#endif
                MAINWINCREATE  *pCreateInfo;
                pCreateInfo = (MAINWINCREATE  *)(lParam);
                SVGUI_HEADER_T *header = NULL;
                gal_pixel pixel_bk;


                HDC secdc =  alloc_subdc_from_uisec(51, 51, sg_websit_dlglevel_width,  sg_websit_dlglevel_height);

                if (pCreateInfo != NULL)
                {
                    header =(SVGUI_HEADER_T *) pCreateInfo->dwAddData ;
                    if (header)
                    {
                        pixel_bk = RGB2Pixel (secdc, 
                                GetRValue (header->color_bk),
                                GetGValue (header->color_bk),
                                GetBValue (header->color_bk));
                        SetBrushColor (secdc, pixel_bk);
                        FillBox (secdc, 0, 0, sg_websit_dlglevel_width,  sg_websit_dlglevel_height);
                    }
                }
                SetSecondaryDC (hWnd, secdc, on_update_ui_secdc);
            }
            break;

        case MSG_LBUTTONDOWN:
            {

                SVGUI_BLOCK_I *old_block = NULL;
                SVGUI_BLOCK_I *selected_block = NULL;

                old_block =  GetSelectedBlock (hWnd, -1, -1, 4);
                selected_block = GetDlgLevel2SelectedStatusByPos(hWnd,  LOWORD (lParam), HIWORD (lParam));
                if(! old_block)
                    return 0;
                if(!selected_block)
                    return 0;
                if ( old_block == selected_block  )
                {
                    printf ("enter next menu Level2 \n"); 
                    if (!sg_dlglevel3_hwnd)
                        SendMessage (hWnd, MSG_KEYUP, SCANCODE_CURSORBLOCKRIGHT, 0);
                }
                else
                {
                    if (sg_dlglevel3_hwnd)
                        WebsiteAnimageCloseWindow (sg_dlglevel3_hwnd);
                    SetDlgLevel2SelectedStatusByPos(hWnd,  LOWORD (lParam), HIWORD (lParam));

#if HAVE_ARROW_WND
                    if (sg_arrow_hwnd)
                        WebsiteAnimageCloseWindow (sg_arrow_hwnd);
#endif
                }
            }
            break;

        case MSG_KEYUP :
            switch (wParam)
            {
                case SCANCODE_ENTER:
                case SCANCODE_CURSORBLOCKRIGHT:
                    {
#if HAVE_ARROW_WND
                        CreateDlgArrowWindow ();
#endif
                        SVGUI_BLOCK_I *selected_block =  GetSelectedBlock (hWnd, -1, -1, 4);
                        if (selected_block == NULL)
                            break;

                        selected_id =  selected_block->id - DLGLEVEL2_MIN_ID ;
                        sg_dlglevel2_selected_id = selected_id;
                        SVGUI_HEADER_T * dlg3header =CreateWebsiteDlgLevel3SvguiHeaderByDlgheaderinfo
                            (&dlglevel3_header_info[selected_id]); 

                        scale = (1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT)/(1.0*RECTW(g_rcScr)/RECTH(g_rcScr));

                        scale1 = scale;
                        if(scale>1.0)
                        {
                            scale = 1.0;
                        }
                    
                        if(scale1 <= 1.0)
                            scale1 = 1.0;

                        create_dlglevel3 ++;
                       if(create_dlglevel3 == 1)
                       {
                           sg_dlglevel3_hwnd = CreateDlgLevel3 (g_hMainWnd,"test", 
                                   dlglevel3_rect[selected_id].left*RECTW(g_rcScr)/DEFAULT_SCREEN_WIDTH,  dlglevel3_rect[selected_id].top*RECTH(g_rcScr)/DEFAULT_SCREEN_HEIGHT,  RECTW(dlglevel3_rect[selected_id])*scale*RECTW(g_rcScr)/DEFAULT_SCREEN_WIDTH, RECTH(dlglevel3_rect[selected_id])*RECTH(g_rcScr)/DEFAULT_SCREEN_HEIGHT/scale1, dlg3header );
                       }

                    }
                    break;

                case SCANCODE_ESCAPE:
                case SCANCODE_CURSORBLOCKLEFT:
                    {
                        RECT dstrc;
                        GetWindowRect (hWnd, &dstrc);
                        AnimationMoveWndLeftRight (hWnd, dstrc, MOVE_LEFT, 50, 4);
                        
                        //Caijun.Lee.   
                        if (sg_dlglevel3_hwnd)
                            WebsiteAnimageCloseWindow (sg_dlglevel3_hwnd);
                        
                   //     PostMessage (hWnd, MSG_CLOSE, 0, 0);
                        SendMessage (hWnd, MSG_CLOSE, 0, 0);
                        InitToolbar(g_hMainWnd);
                    }
                    break;

                case SCANCODE_CURSORBLOCKUP:
                    if (sg_dlglevel3_hwnd)
                        WebsiteAnimageCloseWindow (sg_dlglevel3_hwnd);
                    SetDlgLevel2SelectedStatusByNextId (hWnd, FALSE);
                    break;

                case SCANCODE_CURSORBLOCKDOWN:
                    if (sg_dlglevel3_hwnd)
                        WebsiteAnimageCloseWindow (sg_dlglevel3_hwnd);
                    SetDlgLevel2SelectedStatusByNextId (hWnd, TRUE);
                    break;
            }
            break;

        case MSG_CLOSE:
            {
                DestroyMainWindow (hWnd);
            }
            break;
    }

    return DefaultSVGUIMainWinProc (hWnd, message, wParam, lParam);
}

int CreateDlgLevel2 (HWND hParent)
{
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
    RECT dstrc;

    printf("CreateDlgLevel2\n");
    float scale;
    scale = (1.0*DEFAULT_SCREEN_WIDTH/DEFAULT_SCREEN_HEIGHT)/(1.0*RECTW(g_rcScr)/RECTH(g_rcScr));
    if(scale>1.0){
        scale = 1.0;
    }

    sg_websit_dlglevel_width  = RECTW(g_rcScr)*(WEBSITE_WND_LEVEL2_WIDTH_PERCENT*scale);
    sg_websit_dlglevel_height = RECTH(g_rcScr)*(WEBSITE_WND_LEVEL2_HEIGHT_PERCENT);

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;
    CreateInfo.spCaption = "WebSite";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = WebsiteDlgLevel2Proc;
    CreateInfo.lx = RECTW (g_rcScr)*(WEBSITE_LEFTSPACE_PERCENT);
    CreateInfo.ty = RECTH(g_rcScr)*(WEBSITE_TOPSPACE_PERCENT);
    CreateInfo.rx = CreateInfo.lx + sg_websit_dlglevel_width;
    CreateInfo.by = CreateInfo.ty + sg_websit_dlglevel_height;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)(&svgui_dlglevel2_header);
    CreateInfo.hHosting = hParent;

    hMainWnd = CreateMainWindow (&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return NULL;

    dstrc.left = CreateInfo.lx;
    dstrc.top = CreateInfo.ty;
    dstrc.right = CreateInfo.rx;
    dstrc.bottom = CreateInfo.by;

    weather_nomal_wnd_rect.left = dstrc.left;
    weather_nomal_wnd_rect.top  = dstrc.top;
    weather_nomal_wnd_rect.right = dstrc.right;
    weather_nomal_wnd_rect.bottom = dstrc.bottom;

    //    AnimationMoveWndLeftRight( hMainWnd, dstrc, MOVE_RIGHT, 50, 10 );
    AnimationMoveWndLeftRight( hMainWnd, weather_nomal_wnd_rect, MOVE_RIGHT, 50, 10 );
    return hMainWnd;
}

void mdtv_CreateWebsiteWindow (HWND hParent)
{
    g_website_hwnd = CreateDlgLevel2 (hParent);
}

