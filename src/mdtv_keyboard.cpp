#include "mdtvconfig.h"

#if MDTV_ENABLE_SOFTIME
#include <string.h>
#include "svgui.h"
#include "mdtv_app.h"
#include "mdtv_common.h"
#include "mdtv_dbbuffer.h"
#include "mdtv_ime.h"
#include "mgi-phone.h"
#include <mgi/ime_pinyin.h>
#include "mgi-phone.h"

extern const MGI_PHONE_KEY_MAP default_map[];
#define WITH_MOVE_WND   1

#define NDEBUG  1
#ifndef NDEBUG
#define DEBUG_TRACE(P) {fprintf (stderr, "FIXME: %s,%d,%s: %s\n", __FILE__, __LINE__, __FUNCTION__ ,(P));}

#define error(fmt...) fprintf (stderr, "mdtv_keyboard[ERROR]:"fmt)
#define debug(fmt...) fprintf (stderr, "mdtv_keyboard[DEBUG]:"fmt)

#else
#define DEBUG_TRACE(P) 
#define error(fmt...) fprintf (stderr, "mdtv_keyboard[ERROR]:"fmt)
#define debug(fmt...)
#endif

#define KB_BKGND_1_X_OFFSET             (3)
#define KB_BKGND_1_Y_OFFSET             (4)
// soft-keyboard
#define KB_WND_WIDTH                    (185+KB_BKGND_1_X_OFFSET)
#define KB_WND_HEIGHT                   (412+KB_BKGND_1_Y_OFFSET)

#define KB_BKGND_WIDTH                  (KB_WND_WIDTH-KB_BKGND_1_X_OFFSET)
#define KB_BKGND_HEIGHT                 (KB_WND_HEIGHT-KB_BKGND_1_Y_OFFSET)

#define KB_BUTTON_NUM_BKGND_X_OFFSET    (2)
#define KB_BUTTON_NUM_BKGND_Y_OFFSET    (2)

#define KB_BUTTON_NUM_BLOCK_WIDTH       (55)
#define KB_BUTTON_NUM_BLOCK_HEIGHT      (28)

#define KB_BUTTON_NUM_WIDTH             (KB_BUTTON_NUM_BLOCK_WIDTH-KB_BUTTON_NUM_BKGND_X_OFFSET)
#define KB_BUTTON_NUM_HEIGHT            (KB_BUTTON_NUM_BLOCK_HEIGHT-KB_BUTTON_NUM_BKGND_Y_OFFSET)

#define KB_BUTTON_NUM_HORZ_SPACE        (6)
#define KB_BUTTON_NUM_VERT_SPACE        (16)

#define KB_BUTTON_UP_ARROW_WIDTH        (77)
#define KB_BUTTON_UP_ARROW_HEIGHT       (39)

#define KB_BUTTON_LEFT_ARROW_WIDTH      (40)
#define KB_BUTTON_LEFT_ARROW_HEIGHT     (79)

#define KB_BUTTON_OK_BKGND_WIDTH        (63)
#define KB_BUTTON_OK_BKGND_HEIGHT       (63)
#define KB_BUTTON_OK_WIDTH              (41)
#define KB_BUTTON_OK_HEIGHT             (41)

#define KB_BUTTON_Q_WIDTH               (54)
#define KB_BUTTON_Q_HEIGHT              (36)


static const char *str_num_button[][2] = {
    {"1",""},
    {"2","abc"},
    {"3","def"},
    {"4","ghi"},
    {"5","jkl"},
    {"6","mno"},
    {"7","pqrs"},
    {"8","tuv"},
    {"9","wxyz"},
    {"0",""},
    {"*",""},
    {"#",""},
};

//---- SVGUI_PRDRD_BLOCK_T
// keyboard background block index
// order must be same as SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[];
enum {
    KB_PRDRD_BLOCK_BKGND_1,
    KB_PRDRD_BLOCK_BKGND_2,
    KB_PRDRD_BLOCK_BKGND_BUTTON_NUM,
    KB_PRDRD_BLOCK_FOREGND_BUTTON_NUM,
    KB_PRDRD_BLOCK_LEFT_ARROW,
    KB_PRDRD_BLOCK_UP_ARROW,
    KB_PRDRD_BLOCK_BUTTON_OK,
    KB_PRDRD_BLOCK_BUTTON_Q,
    KB_PRDRD_BLOCK_SELECT_BUTTON_NUM,
};
#define KB_PRDRD_BLOCK_COLOR_BK   (0x000000)
/*
typedef struct _SVGUI_PRDRD_BLOCK_T {
    int width, height; int gradient_type;
    int pos_c1, pos_c2, pos_c3; RGBCOLOR c1, c2, c3;
    int border_width;
    RGBCOLOR border_color;
    int corner;
    RGBCOLOR color_bk;
} SVGUI_PRDRD_BLOCK_T;
 */
#define COLOR_BKGND     0x232323
static SVGUI_PRDRD_BLOCK_T ptr_prdrd_block_t[]={
    // KB_PRDRD_BLOCK_BKGND_1
    { KB_BKGND_WIDTH, KB_BKGND_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, 0x101010, (COLOR_BKGND+0x3E3E3E)/2, COLOR_BKGND,
        0, 0x000000, 8, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_BKGND_2
    { KB_BKGND_WIDTH, KB_BKGND_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, KB_BKGND_HEIGHT/2, KB_BKGND_HEIGHT, COLOR_BKGND, (COLOR_BKGND+0x3E3E3E)/2, 0x3E3E3E,
        0, 0x000000, 8, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_BKGND_BUTTON_NUM
    { KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, 0x101010, 0x000000, 0x000000,
        0, 0x101010, 4, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_FOREGND_BUTTON_NUM
    { KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, KB_BUTTON_NUM_WIDTH/2, KB_BUTTON_NUM_WIDTH, 0xB5B5B4, (0xEEEFE5+0xB5B5B4)/2, 0xB5B5B4,
        0, 0x101010, 4, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_LEFT_ARROW
    { KB_BUTTON_LEFT_ARROW_WIDTH, KB_BUTTON_LEFT_ARROW_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, COLOR_BKGND, COLOR_BKGND, COLOR_BKGND,
        0, COLOR_BKGND, 0, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_UP_ARROW
    { KB_BUTTON_UP_ARROW_WIDTH, KB_BUTTON_UP_ARROW_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, COLOR_BKGND, COLOR_BKGND, COLOR_BKGND,
        0, COLOR_BKGND, 0, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_BUTTON_OK
    { KB_BUTTON_OK_BKGND_WIDTH, KB_BUTTON_OK_BKGND_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, COLOR_BKGND, COLOR_BKGND, COLOR_BKGND,
        0, COLOR_BKGND, 0, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_BUTTON_Q
    { KB_BUTTON_Q_WIDTH, KB_BUTTON_Q_HEIGHT, SVGUI_PRDRD_GRADIENT_NONE,
        0, 0, 0, COLOR_BKGND, COLOR_BKGND, COLOR_BKGND,
        0, COLOR_BKGND, 0, KB_PRDRD_BLOCK_COLOR_BK, },
    // KB_PRDRD_BLOCK_SELECT_BUTTON_NUM
    { 
        KB_BUTTON_NUM_WIDTH+KB_BUTTON_NUM_BKGND_X_OFFSET, 
        KB_BUTTON_NUM_HEIGHT+KB_BUTTON_NUM_BKGND_X_OFFSET, 
        SVGUI_PRDRD_GRADIENT_NONE,
        0, KB_BUTTON_NUM_WIDTH/2, KB_BUTTON_NUM_WIDTH,
        //0x672900, 0xFF6600, 0x672900, 
        0xFFFFFF, 0xFF6600, 0x672900, 
      //  0xB5B5B4, (0xEEEFE5+0xB5B5B4)/2, 0xB5B5B4,
        0,
        0x101010, 
        11, 
        KB_PRDRD_BLOCK_COLOR_BK, 
    },
};

//---- SVGUI_IMAGE
static SVGUI_IMAGE_T left_arrow_images[]={
    { 40, 79, 32, "keyboard/left_arrow_40x79.png", },
};
static SVGUI_IMAGE_T up_arrow_images[]={
    { 77, 39, 32, "keyboard/up_arrow_77x39.png", },
};
static SVGUI_IMAGE_T right_arrow_images[]={
    { 40, 79, 32, "keyboard/right_arrow_40x79.png", },
};
static SVGUI_IMAGE_T down_arrow_images[]={
    { 77, 39, 32, "keyboard/down_arrow_77x39.png", },
};
static SVGUI_IMAGE_T button_ok_bkgnd_images[]={
    { 41, 41, 32, "keyboard/ok_bkgnd_63x63.png", },
};
static SVGUI_IMAGE_T button_ok_images[]={
    { 41, 41, 32, "keyboard/ok_41x41.png", },
};
static SVGUI_IMAGE_T button_q_images[]={
    { 55, 38, 32, "keyboard/button_q_55x38.png", },
};
static SVGUI_IMAGE_T button_c_images[]={
    { 55, 38, 32, "keyboard/button_c_55x38.png", },
};
//---- SVGUI_IMAGE_AREA_T
static SVGUI_IMAGE_AREA_T left_arrow_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_LEFT_ARROW_WIDTH, KB_BUTTON_LEFT_ARROW_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(left_arrow_images), left_arrow_images,},
};
static SVGUI_IMAGE_AREA_T up_arrow_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_UP_ARROW_WIDTH, KB_BUTTON_UP_ARROW_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(up_arrow_images), up_arrow_images,},
};
static SVGUI_IMAGE_AREA_T right_arrow_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_LEFT_ARROW_WIDTH, KB_BUTTON_LEFT_ARROW_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(right_arrow_images), right_arrow_images,},
};
static SVGUI_IMAGE_AREA_T down_arrow_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_UP_ARROW_WIDTH, KB_BUTTON_UP_ARROW_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(down_arrow_images), down_arrow_images,},
};
static SVGUI_IMAGE_AREA_T button_ok_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_OK_BKGND_WIDTH, KB_BUTTON_OK_BKGND_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(button_ok_bkgnd_images), button_ok_bkgnd_images,},
    { 0, TRUE, {0, 0, KB_BUTTON_OK_BKGND_WIDTH, KB_BUTTON_OK_BKGND_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(button_ok_images), button_ok_images,},
};
static SVGUI_IMAGE_AREA_T button_q_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_Q_WIDTH, KB_BUTTON_Q_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(button_q_images), button_q_images,},
};
static SVGUI_IMAGE_AREA_T button_c_img_areas[]={
    { 0, TRUE, {0, 0, KB_BUTTON_Q_WIDTH, KB_BUTTON_Q_HEIGHT},
        SVGUI_IMAGE_FILL_WAY_CENTER, TABLESIZE(button_c_images), button_c_images,},
};

#define KB_COLOR_TEXT      (0x000000)
//---- SVGUI_TEXT_AREA_T
#define DEFINE_TEXT_AREAS_BUTTOM_NUM(num) \
static SVGUI_TEXT_AREA_T text_areas_button_num_##num[]={\
    {\
        0,\
        TRUE,\
        {0, 0, KB_BUTTON_NUM_WIDTH*5/16, KB_BUTTON_NUM_HEIGHT},\
        SVGUI_TEXT_HALIGN_RIGHT | SVGUI_TEXT_VALIGN_CENTER,\
        KB_COLOR_TEXT,\
        1,\
        str_num_button[num-1][0],\
    },\
    {\
        1,\
        TRUE,\
        {KB_BUTTON_NUM_WIDTH*3/8, KB_BUTTON_NUM_HEIGHT/4, KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_HEIGHT},\
        SVGUI_TEXT_HALIGN_LEFT | SVGUI_TEXT_VALIGN_CENTER,\
        KB_COLOR_TEXT,\
        0,\
        str_num_button[num-1][1],\
    },\
}
DEFINE_TEXT_AREAS_BUTTOM_NUM(1);
DEFINE_TEXT_AREAS_BUTTOM_NUM(2);
DEFINE_TEXT_AREAS_BUTTOM_NUM(3);
DEFINE_TEXT_AREAS_BUTTOM_NUM(4);
DEFINE_TEXT_AREAS_BUTTOM_NUM(5);
DEFINE_TEXT_AREAS_BUTTOM_NUM(6);
DEFINE_TEXT_AREAS_BUTTOM_NUM(7);
DEFINE_TEXT_AREAS_BUTTOM_NUM(8);
DEFINE_TEXT_AREAS_BUTTOM_NUM(9);

#define DEFINE_TEXT_AREAS_BUTTOM_SYM(num) \
static SVGUI_TEXT_AREA_T text_areas_button_num_##num[]={\
    {\
        0,\
        TRUE,\
        {0, 0, KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_HEIGHT},\
         SVGUI_TEXT_HALIGN_CENTER | SVGUI_TEXT_VALIGN_CENTER,\
        KB_COLOR_TEXT,\
        1,\
        str_num_button[num-1][0],\
    },\
}
DEFINE_TEXT_AREAS_BUTTOM_SYM(10);
DEFINE_TEXT_AREAS_BUTTOM_SYM(11);
DEFINE_TEXT_AREAS_BUTTOM_SYM(12);

//---- SVGUI_BLOCK_T
enum{
    KB_BLOCK_BKGND_1=0,
    KB_BLOCK_BKGND_2,
    KB_BLOCK_NUM_BKGND_1,
    KB_BLOCK_NUM_BKGND_2,
    KB_BLOCK_NUM_BKGND_3,
    KB_BLOCK_NUM_BKGND_4,
    KB_BLOCK_NUM_BKGND_5,
    KB_BLOCK_NUM_BKGND_6,
    KB_BLOCK_NUM_BKGND_7,
    KB_BLOCK_NUM_BKGND_8,
    KB_BLOCK_NUM_BKGND_9,
    KB_BLOCK_NUM_BKGND_10,
    KB_BLOCK_NUM_BKGND_11,
    KB_BLOCK_NUM_BKGND_12,
    KB_BLOCK_NUM_1,
    KB_BLOCK_NUM_2,
    KB_BLOCK_NUM_3,
    KB_BLOCK_NUM_4,
    KB_BLOCK_NUM_5,
    KB_BLOCK_NUM_6,
    KB_BLOCK_NUM_7,
    KB_BLOCK_NUM_8,
    KB_BLOCK_NUM_9,
    KB_BLOCK_NUM_10,
    KB_BLOCK_NUM_11,
    KB_BLOCK_NUM_12,
    KB_BLOCK_LEFT_ARROW,
    KB_BLOCK_UP_ARROW,
    KB_BLOCK_RIGHT_ARROW,
    KB_BLOCK_DOWN_ARROW,
    KB_BLOCK_BUTTON_OK,
    KB_BLOCK_BUTTON_Q,
    KB_BLOCK_BUTTON_C,
    KB_MAX_BLOCK,
};
#define DEFINE_BUTTON_NUM_BKGND(num, x, y) \
    { KB_BLOCK_NUM_BKGND_##num,\
        TRUE,\
        {KB_BUTTON_NUM_BKGND_X_OFFSET+x*KB_BUTTON_NUM_HORZ_SPACE+(x-1)*KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_BKGND_Y_OFFSET+y*KB_BUTTON_NUM_VERT_SPACE+(y-1)*KB_BUTTON_NUM_HEIGHT, KB_BUTTON_NUM_BKGND_X_OFFSET+x*KB_BUTTON_NUM_HORZ_SPACE+x*KB_BUTTON_NUM_WIDTH, KB_BUTTON_NUM_BKGND_Y_OFFSET+y*KB_BUTTON_NUM_VERT_SPACE+y*KB_BUTTON_NUM_HEIGHT},\
        FALSE,\
        KB_PRDRD_BLOCK_BKGND_BUTTON_NUM,\
        0,\
        NULL, \
        0,\
        NULL, \
    },
#define DEFINE_BUTTON_NUM(num, x, y) \
    { KB_BLOCK_NUM_##num,\
        TRUE,\
        {x*KB_BUTTON_NUM_HORZ_SPACE+(x-1)*KB_BUTTON_NUM_WIDTH, y*KB_BUTTON_NUM_VERT_SPACE+(y-1)*KB_BUTTON_NUM_HEIGHT, x*KB_BUTTON_NUM_HORZ_SPACE+x*KB_BUTTON_NUM_WIDTH, y*KB_BUTTON_NUM_VERT_SPACE+y*KB_BUTTON_NUM_HEIGHT},\
        TRUE,\
        KB_PRDRD_BLOCK_FOREGND_BUTTON_NUM,\
        TABLESIZE(text_areas_button_num_##num),\
        text_areas_button_num_##num, \
        0,\
        NULL, \
    },
/*
typedef struct _SVGUI_BLOCK_T {
    int id;     BOOL is_visible;        RECT rc;

    BOOL is_hotspot;
    int idx_prdrd_block;

    int nr_text_areas;
    SVGUI_TEXT_AREA_T* text_areas;

    int nr_image_areas;
    SVGUI_IMAGE_AREA_T* image_areas;
} SVGUI_BLOCK_T;
 */
static SVGUI_BLOCK_T ptr_blocks[]={
    { KB_BLOCK_BKGND_1, TRUE, {KB_BKGND_1_X_OFFSET, KB_BKGND_1_Y_OFFSET, KB_BKGND_1_X_OFFSET+KB_BKGND_WIDTH, KB_BKGND_1_Y_OFFSET+KB_BKGND_HEIGHT},
        FALSE, KB_PRDRD_BLOCK_BKGND_1, 0, NULL, 0, NULL, },
    { KB_BLOCK_BKGND_2, TRUE, {0, 0, KB_BKGND_WIDTH, KB_BKGND_HEIGHT},
        FALSE, KB_PRDRD_BLOCK_BKGND_2, 0, NULL, 0, NULL, },
    DEFINE_BUTTON_NUM_BKGND(1, 1, 1)
    DEFINE_BUTTON_NUM_BKGND(2, 2, 1)
    DEFINE_BUTTON_NUM_BKGND(3, 3, 1)
    DEFINE_BUTTON_NUM_BKGND(4, 1, 2)
    DEFINE_BUTTON_NUM_BKGND(5, 2, 2)
    DEFINE_BUTTON_NUM_BKGND(6, 3, 2)
    DEFINE_BUTTON_NUM_BKGND(7, 1, 3)
    DEFINE_BUTTON_NUM_BKGND(8, 2, 3)
    DEFINE_BUTTON_NUM_BKGND(9, 3, 3)
    DEFINE_BUTTON_NUM_BKGND(10, 2, 4)
    DEFINE_BUTTON_NUM_BKGND(11,1, 4)
    DEFINE_BUTTON_NUM_BKGND(12,3, 4)

    DEFINE_BUTTON_NUM(1, 1, 1)
    DEFINE_BUTTON_NUM(2, 2, 1)
    DEFINE_BUTTON_NUM(3, 3, 1)
    DEFINE_BUTTON_NUM(4, 1, 2)
    DEFINE_BUTTON_NUM(5, 2, 2)
    DEFINE_BUTTON_NUM(6, 3, 2)
    DEFINE_BUTTON_NUM(7, 1, 3)
    DEFINE_BUTTON_NUM(8, 2, 3)
    DEFINE_BUTTON_NUM(9, 3, 3)
    DEFINE_BUTTON_NUM(10, 2, 4)
    DEFINE_BUTTON_NUM(11,1, 4)
    DEFINE_BUTTON_NUM(12,3, 4)

    { KB_BLOCK_LEFT_ARROW, TRUE, {14, 234, 14+KB_BUTTON_LEFT_ARROW_WIDTH, 234+KB_BUTTON_LEFT_ARROW_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_LEFT_ARROW, 0, NULL, TABLESIZE(left_arrow_img_areas), left_arrow_img_areas, },

    { KB_BLOCK_UP_ARROW, TRUE, {54, 195, 54+KB_BUTTON_UP_ARROW_WIDTH, 195+KB_BUTTON_UP_ARROW_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_UP_ARROW, 0, NULL, TABLESIZE(up_arrow_img_areas), up_arrow_img_areas, },

    { KB_BLOCK_RIGHT_ARROW, TRUE, {131, 234, 131+KB_BUTTON_LEFT_ARROW_WIDTH, 234+KB_BUTTON_LEFT_ARROW_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_LEFT_ARROW, 0, NULL, TABLESIZE(right_arrow_img_areas), right_arrow_img_areas, },

    { KB_BLOCK_DOWN_ARROW, TRUE, {54, 313, 54+KB_BUTTON_UP_ARROW_WIDTH, 313+KB_BUTTON_UP_ARROW_WIDTH},
        TRUE, KB_PRDRD_BLOCK_UP_ARROW, 0, NULL, TABLESIZE(down_arrow_img_areas), down_arrow_img_areas, },

    { KB_BLOCK_BUTTON_OK, TRUE, {(KB_WND_WIDTH-KB_BUTTON_OK_BKGND_WIDTH)/2, 234+10, (KB_WND_WIDTH+KB_BUTTON_OK_BKGND_WIDTH)/2, 234+10+KB_BUTTON_OK_BKGND_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_BUTTON_OK, 0, NULL, TABLESIZE(button_ok_img_areas), button_ok_img_areas,},

    { KB_BLOCK_BUTTON_Q, TRUE, {16, 313+10+KB_BUTTON_UP_ARROW_HEIGHT, 16+KB_BUTTON_Q_WIDTH, 313+10+KB_BUTTON_UP_ARROW_HEIGHT+KB_BUTTON_Q_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_BUTTON_Q, 0, NULL, TABLESIZE(button_q_img_areas), button_q_img_areas, },

    { KB_BLOCK_BUTTON_C, TRUE, {118, 313+10+KB_BUTTON_UP_ARROW_HEIGHT, 118+KB_BUTTON_Q_WIDTH, 313+10+KB_BUTTON_UP_ARROW_HEIGHT+KB_BUTTON_Q_HEIGHT},
        TRUE, KB_PRDRD_BLOCK_BUTTON_Q, 0, NULL, TABLESIZE(button_c_img_areas), button_c_img_areas, },
};

//---- SVGUI_HEADER_T
static SVGUI_HEADER_T svgui_header_t = {
    KB_WND_WIDTH,  // width
    KB_WND_HEIGHT, // height
    KB_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_bk;
    KB_PRDRD_BLOCK_COLOR_BK,//RGBCOLOR color_key;
    //back ground block description: SVGUI_PRDRD_BLOCK_T
    TABLESIZE(ptr_prdrd_block_t),// nr_prdrd_blocks;
    ptr_prdrd_block_t,  // SVGUI_PRDRD_BLOCK_T* prdrd_blocks;

    TABLESIZE(ptr_font_infos),  // int nr_font_infos;
    ptr_font_infos,     // SVGUI_FONT_INFO_T* font_infos;

    TABLESIZE(ptr_blocks),// int nr_blocks;
    ptr_blocks,         // SVGUI_BLOCK_T* blocks;
};

static int on_lbuttondown(HWND hWnd, int x_pos, int y_pos )
{
    SVGUI_HEADER_I *header;
    SVGUI_BLOCK_I* block;
    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return -1;
    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if (block == NULL)
        return -1;

    if (block->id >= KB_BLOCK_NUM_1 && block->id <= KB_BLOCK_NUM_12) 
    {
        //block->idx_prdrd_block = PRDRD_BLOCK_NOT_SELECTED;
        block->idx_prdrd_block = KB_PRDRD_BLOCK_SELECT_BUTTON_NUM;
        InvalidateRect (hWnd, &block->rc, TRUE);
    }
    return 0;
}

static int on_lbuttonup(HWND hWnd, int x_pos, int y_pos )
{
    SVGUI_HEADER_I *header;
    SVGUI_BLOCK_I* block;
    int keynumber;
    HWND phone_hwnd = mdtv_GetPhoneImeWnd();

    header = (SVGUI_HEADER_I *)GetWindowAdditionalData(hWnd);
    if (header == NULL)
        return -1;
    block = svgui_get_block_by_point (header, x_pos, y_pos);
    if (block == NULL)
        return -1;
    if( block->id )
    {
        printf("block->id=%d\n",block->id);
    }

    if (block->id >= KB_BLOCK_NUM_1 && block->id <= KB_BLOCK_NUM_10) {
        
        keynumber = (block->id + 1 -  KB_BLOCK_NUM_1) % (KB_BLOCK_NUM_BKGND_10 - KB_BLOCK_NUM_BKGND_1 +1) ;
        printf("block->id=%d,  keynumber=%d\n",block->id, keynumber);
        SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_0+keynumber].scancode, 0 );
        block->idx_prdrd_block = KB_PRDRD_BLOCK_FOREGND_BUTTON_NUM;
        InvalidateRect (hWnd, &block->rc, TRUE);
        return 0;
    }
    
    switch (block->id)
    {
        case   KB_BLOCK_NUM_11:
                block->idx_prdrd_block = KB_PRDRD_BLOCK_FOREGND_BUTTON_NUM;
                InvalidateRect (hWnd, &block->rc, TRUE);
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_STAR].scancode, 0);
            break;
        case   KB_BLOCK_NUM_12:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_SHARP].scancode, 0);
            break;
        case   KB_BLOCK_LEFT_ARROW:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_LEFT].scancode, 0);
            break;
        case   KB_BLOCK_UP_ARROW:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_UP].scancode, 0);
            break;
        case   KB_BLOCK_RIGHT_ARROW:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_RIGHT].scancode, 0);
            break;
        case   KB_BLOCK_DOWN_ARROW:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_DOWN].scancode, 0);
            break;
        case   KB_BLOCK_BUTTON_OK:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_ENTER].scancode, 0);
            break;
        case   KB_BLOCK_BUTTON_Q:
#if 0
                 //mdtv_DestroyPhoneIme (hWnd);
                 mdtv_ShowIme (0, NULL);
#else
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_QUIT].scancode, 0);
#endif
            break;
        case   KB_BLOCK_BUTTON_C:
                SendMessage (phone_hwnd, MSG_KEYDOWN, default_map[MGI_PHONE_KEY_CLEAR].scancode, 0);
            break;
    }
    return 0;
}
static int KBWndProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    int x_pos;
    int y_pos;
#if WITH_MOVE_WND
    static BOOL is_press_mousedown;
    static int old_mouse_pos_x;
    static int old_mouse_pos_y;
    int start_x;
    int start_y;
    RECT rc;
#endif
    switch(message)
    {
        case MSG_CREATE:
#if WITH_MOVE_WND
            is_press_mousedown = FALSE;
#endif
                mdtv_CreateImePinying ();
            break;
        case MSG_KEYUP:
            break;
        case MSG_LBUTTONDBLCLK:
        case MSG_LBUTTONDOWN:
#if WITH_MOVE_WND
            SetCapture(hWnd);
            old_mouse_pos_x = LOWORD (lParam);
            old_mouse_pos_y = HIWORD (lParam);
            is_press_mousedown = TRUE;
            ClientToScreen(hWnd, (int*)&old_mouse_pos_x, (int*)&old_mouse_pos_y);
#endif
            on_lbuttondown(hWnd, LOWORD (lParam), HIWORD (lParam));
            break;              

        case MSG_LBUTTONUP:
#if WITH_MOVE_WND
            is_press_mousedown = FALSE;
            if(GetCapture() == hWnd ){
                ReleaseCapture();
            }
            //we must transfer the x,y coordinate 
            //or when click link ,it may not response(the mouse postion is not correct
            x_pos=LOWORD(lParam);
            y_pos=HIWORD(lParam);
            ScreenToClient (hWnd, &x_pos, &y_pos);
#else
            x_pos = LOWORD (lParam);
            y_pos = HIWORD (lParam);
#endif
            on_lbuttonup(hWnd, x_pos, y_pos);
            break;
        case MSG_MOUSEMOVE:
#if WITH_MOVE_WND
            if (wParam & KS_LEFTBUTTON){
                if(GetCapture()==hWnd){
                    x_pos= LOWORD(lParam);
                    y_pos= HIWORD(lParam);
                }else{
                    x_pos = LOWORD (lParam);
                    y_pos = HIWORD (lParam);
                }
                start_x = 0;
                start_y = 0;
                if(is_press_mousedown){
                    if( !GetWindowRect(hWnd, &rc) ){
                        break;
                    }
                    ClientToScreen(hWnd, (int*)&start_x, (int*)&start_y);
                    start_x = start_x+(x_pos-old_mouse_pos_x);
                    start_y = start_y+(y_pos-old_mouse_pos_y);
                    MoveWindow(hWnd, start_x, start_y, RECTW(rc), RECTH(rc), FALSE);
                    old_mouse_pos_x = x_pos;
                    old_mouse_pos_y = y_pos;
                }
            }
#endif
            break;
        case MSG_IME_CMD_OPEN:
            ShowWindow (hWnd, SW_SHOWNORMAL);
            break;
        case MSG_IME_CMD_CLOSE:
            ShowWindow (hWnd, SW_HIDE);
            break;
        case MSG_CLOSE:
            DestroyMainWindow(hWnd);
            return 0;
        case MSG_DESTROY:
            mdtv_DestroyPhoneIme (NULL);
            break ;
    }
    return DefaultSVGUIMainWinProc(hWnd, message, wParam, lParam);
}
static HWND CreateMenuWnd( const RECT *rect, WNDPROC hWndProc,const SVGUI_HEADER_T *svgui_header_t, HWND hHostingWnd )
{
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle = WS_VISIBLE ;
    CreateInfo.dwExStyle = WS_EX_TOPMOST |WS_EX_TROUNDCNS | WS_EX_BROUNDCNS |WS_EX_TOOLWINDOW;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = hWndProc;
    CreateInfo.lx = rect->left;
    CreateInfo.ty = rect->top;
    CreateInfo.rx = rect->right;
    CreateInfo.by = rect->bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = (DWORD)svgui_header_t;

    //CreateInfo.hHosting = HWND_DESKTOP;
    CreateInfo.hHosting = hHostingWnd;

    return CreateMainWindow (&CreateInfo);
}

HWND mdtv_init_keyboard(HWND hWnd)
{
    HWND hKBWnd;
    RECT rc;
    SetRect(&rc, 100, 100, 100+KB_WND_WIDTH, 100+KB_WND_HEIGHT );
    hKBWnd = CreateMenuWnd( &rc, KBWndProc, &svgui_header_t, hWnd);
    if (hKBWnd == HWND_INVALID){
        error("Create keyboard window!\n");
        return HWND_INVALID;
    }
    //ShowWindow (hKBWnd, SW_SHOWNORMAL);
    ShowWindow (hKBWnd, SW_HIDE);
    return hKBWnd;
}
#endif
