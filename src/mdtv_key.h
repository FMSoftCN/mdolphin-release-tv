#ifndef MDOLPHIN_KEY_H
#define MDOLPHIN_KEY_H

enum {
   change_url,
   forward,
   backward,
   reload,
   stop,
   home,
   move_left,
   move_right,
   move_up,
   move_down,
   link_forward,
   link_backward, 
   zoom_in,
   zoom_out,
   keyboard,
   toolbar,
   fav,
   search,
   enter,
   esc,
   page_up,
   page_down,
};

void init_keymap(void);

int translate_command(int scan_code);

#endif
