/*
 * 20090910 add specially MGEFF_ANIMATION_ZOOM_OUT and MGEFF_ANIMATION_ZOOM_IN by Caijun.Lee 
 */
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <mdolphin/mdolphin.h>
#include "mdtv_search.h"
#include "mdtv_browser.h"

extern int g_win_width;
extern int g_win_height;

void navigate_to_search(HWND hWnd)
{
    char *search_url; 
    int percent;
    search_url = get_local_url ("/res/search/search.html");
    HWND hwnd = mdtv_CreateWebSiteNavigate(search_url);

    if (g_win_width <= 800){
        percent = 80;
        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
    }
    else if (g_win_width <= 640){
        percent = 70;
        mdolphin_set_text_size_multiplier(g_mdolphin_hwnd, percent);
    }

    free ((char *)search_url);
} 
