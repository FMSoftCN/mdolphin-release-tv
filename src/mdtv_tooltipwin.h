#ifndef __MDOLPHIN_TOOLTIPWIN_H__
#define __MDOLPHIN_TOOLTIPWIN_H__
int mdolphin_ToolTipWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam);

HWND mdolphin_createToolTipWin (HWND hParentWnd, int x, int y, int timeout_ms, 
                const char* text, ...);


void mdolphin_resetToolTipWin (HWND hwnd, int x, int y, const char* text, ...);

void mdolphin_hideToolTip(HWND hwnd);
void mdolphin_destroyToolTipWin (HWND hwnd);

#endif
