#include <string.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "mdtv_key.h"
#if 0
struct key_func{
    int func;
    int key;
}key_func_map[] = {
   { change_url, SCANCODE_F9 },
   { forward, SCANCODE_RIGHTCONTROL },
   { backward, SCANCODE_LEFTCONTROL },
   { reload, SCANCODE_PAUSE },
   { stop, SCANCODE_LEFTSHIFT },
   { home, SCANCODE_BACKSLASH },
   { move_left, SCANCODE_CURSORBLOCKLEFT },
   { move_right, SCANCODE_CURSORBLOCKRIGHT },
   { move_up, SCANCODE_CURSORBLOCKUP },
   { move_down, SCANCODE_CURSORBLOCKDOWN },
   { link_forward, SCANCODE_RIGHTWIN },
   { link_backward, SCANCODE_LEFTWIN },
   { zoom_in, SCANCODE_PAGEDOWN },
   { zoom_out, SCANCODE_PAGEUP },
   { keyboard, SCANCODE_F10 },
   { toolbar,SCANCODE_SLASH},
   { fav,SCANCODE_F3},
   { search,SCANCODE_F5},
   { enter,SCANCODE_ENTER},
   { esc,SCANCODE_ESCAPE},
};
#endif
struct key_func{
    int func;
    int key;
}key_func_map[] = {
   { link_forward, SCANCODE_F3 },
   { link_backward, SCANCODE_F4 },
   { zoom_in, SCANCODE_F8 },
   { zoom_out, SCANCODE_F9 },
   { forward, SCANCODE_HOME },
   { backward, SCANCODE_END },
   { reload, SCANCODE_PAUSE },
   { stop, SCANCODE_LEFTSHIFT },
   { home, SCANCODE_BACKSLASH },
   { move_left, SCANCODE_CURSORBLOCKLEFT },
   { move_right, SCANCODE_CURSORBLOCKRIGHT },
   { link_forward, SCANCODE_RIGHTWIN },
   { link_backward, SCANCODE_LEFTWIN },
   { move_down, SCANCODE_CURSORBLOCKDOWN },
   { move_up, SCANCODE_CURSORBLOCKUP },
   { page_down, SCANCODE_PAGEDOWN },
   { page_up, SCANCODE_PAGEUP },
   { keyboard, SCANCODE_F10 },
   { toolbar,SCANCODE_SLASH},
   { fav,SCANCODE_F6},
   { search,SCANCODE_F7},
   { enter,SCANCODE_ENTER},
   { esc,SCANCODE_ESCAPE},
};

struct name_func{
    const char *name;
    int func;
}name_func_map[] = {
   { "change_url", change_url },
   { "forward", forward },
   { "backward", backward },
   { "reload", reload },
   { "stop", stop },
   { "home", home },
   { "move_left", move_left },
   { "move_right", move_right },
   { "move_up", move_up },
   { "move_down", move_down },
   { "link_forward", link_forward },
   { "link_backward", link_backward },
   { "zoom_in", zoom_in },
   { "zoom_out", zoom_out },
   { "keyboard", keyboard },
   { "toolbar", toolbar},
   { "fav", fav},
   { "search", search},
   { "enter", enter},
   { "esc", esc},
};

struct key_code{
    const char * name;
    int key;
}key_code_map[] = {
    {"RM_HW_MENU"         , SCANCODE_HOME },
    {"RM_HW_ON_OFF"        , SCANCODE_POWER },
    {"RM_HW_INTERNET"      , SCANCODE_F1 },
    {"RM_HW_MEDIABAR"      , SCANCODE_F2 },
    {"RM_HW_RADIO"         , SCANCODE_F3 },
    {"RM_HW_VIDEO"         , SCANCODE_F4 },
    {"RM_HW_PHOTO"         , SCANCODE_F5 },
    {"RM_HW_MUSIC"         , SCANCODE_F6 },
    {"RM_HW_ZOOM"          , SCANCODE_F7 },
    {"RM_HW_PAGEUP"        , SCANCODE_PAGEUP },
    {"RM_HW_LOOPBACK"      , SCANCODE_F8 },
    {"RM_HW_SUB_TITLE"     , SCANCODE_F9 },
    {"RM_HW_PAGEDOWN"      , SCANCODE_PAGEDOWN },
    {"RM_HW_FAVORITES"     , SCANCODE_BACKSLASH },
    {"RM_HW_FAST_REWIND"   , SCANCODE_LEFTCONTROL },
    {"RM_HW_FAST_FORWARD"  , SCANCODE_RIGHTCONTROL },
    {"RM_HW_STOP"          , SCANCODE_LEFTSHIFT },
    {"RM_HW_PREV_TRACK"    , SCANCODE_LEFTWIN },
    {"RM_HW_PAUSE_PLAY"    , SCANCODE_RIGHTSHIFT },
    {"RM_HW_NEXT_TRACK"    , SCANCODE_RIGHTWIN },
    {"RM_HW_UP"            , SCANCODE_CURSORBLOCKUP },
    {"RM_HW_LEFT"          , SCANCODE_CURSORBLOCKLEFT },
    {"RM_HW_RIGHT"         , SCANCODE_CURSORBLOCKRIGHT },
    {"RM_HW_DOWN"          , SCANCODE_CURSORBLOCKDOWN },
    {"RM_HW_REPEAT"        , SCANCODE_PAUSE },
    {"RM_HW_ESC"          , SCANCODE_ESCAPE },
    {"RM_HW_VOL_PLUS"      , SCANCODE_EQUAL },
    {"RM_HW_VOL_MINUS"     , SCANCODE_MINUS },
    {"RM_HW_SHUTUP"        , SCANCODE_CAPSLOCK },
    {"RM_HW_KEYBOARD"      , SCANCODE_F10 },
    {"RM_HW_TOOLBAR"       , SCANCODE_SLASH},
    {"RM_HW_FAV"           , SCANCODE_F11},
    {"RM_HW_SEARCH"        , SCANCODE_F12},
    {"RM_HW_ENTER"         , SCANCODE_ENTER},
};

#if 0
static int get_scancode_from_key(const char* key_name)
{
    for (int i = 0; i < (int)TABLESIZE(key_code_map); i++)
        if (strcmp(key_name, key_code_map[i].name) == 0)
            return key_code_map[i].key;

    return -1;
}
#endif
static int get_funccode_from_name(const char* func_name)
{
    for (int i = 0; i < (int)TABLESIZE(name_func_map); i++)
        if (strcmp(name_func_map[i].name, func_name) == 0)
            return name_func_map[i].func;

    return -1;
}

void init_keymap(void)
{
    for (int i = 0; i < (int)TABLESIZE(key_code_map); i++) {
        char value[64] = {0};
        int func_id = get_funccode_from_name(value);
        if (func_id != -1) {
            for (int j = 0 ; j < (int)TABLESIZE(key_func_map); j++)
                if (key_func_map[j].func == func_id)
                    key_func_map[j].key = key_code_map[i].key;
        }
    }
}


int translate_command(int scan_code)
{
    for (int i = 0; i < (int)TABLESIZE(key_func_map); i++)
        if (scan_code == key_func_map[i].key)
            return key_func_map[i].func;

    return -1;
}
