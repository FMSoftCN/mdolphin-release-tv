#ifndef __MDOLPHIN_H
#define __MDOLPHIN_H

#if ENABLE_FAV

#include <libxml/parser.h>
#include <libxml/tree.h>
#include "mdtv_fav.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct _MDTV_FAV_INFO {
    unsigned int     m_id;
    char    *m_title;
    char    *m_image;//the absolute path of image
    char    *m_url;
    char    *m_last_time;
}MDTV_FAV_INFO, *MDTV_PTR_FAV_INFO;
// res path or fav path in configuration file
typedef enum {
    RES_PATH,
    FAV_PATH
} PATH_PARAM_TYPE;

#define NOT_OVERWRITE   2

int mdtv_save_favorite_website( HWND hWnd );

int mdtv_read_fav_info(const char *fav_file_name, FAV_ICONFLOW_INFO *p_iconinfo, unsigned max_num);

void mdtv_uninit_fav_icon_info(const FAV_ICONFLOW_INFO *p_iconinfo, unsigned max_num);

/* get the res or the fav absolute path in the configuration file :  get resource path when type = RES_PATH
 *  favorite path when type = FAV_PATH */
BOOL get_path( char *file_path, int size, PATH_PARAM_TYPE path_type);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
#endif /* __MDOLPHIN_H */
