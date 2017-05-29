/*
 * mdtv_fav.h
 *
 *  Created on: 2009-8-31
 *      Author: chenlei
 */

#ifndef MDTV_FAV_H_
#define MDTV_FAV_H_

#if ENABLE_FAV
#define MDTV_FAV_ICON_MAX_NUMBER    7 
typedef  struct{
    char *m_url;
    char *m_title;
    char *m_img_path;
}FAV_ICONFLOW_INFO;

int InitMDTVFavWnd();
void UninitMDTVFavWnd();

void InitMDTVFavWndRes();
void UninitMDTVFavWndRes();

//init the website address array, must do it at the application start
void InitMDTVFavWebsite();
void UninitMDTVFavWebsite();
int mdtv_save_fav(HWND hWnd);

#endif /*ENABLE_FAV*/
#endif /* MDTV_FAV_H_ */
