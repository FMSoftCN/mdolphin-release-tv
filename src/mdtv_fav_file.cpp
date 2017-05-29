#if ENABLE_FAV
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <pwd.h>
#include <sys/types.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>
#include <jpeglib.h>

#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <mdolphin/mdolphin.h>
#include "mdtv_fav.h"
#include "mdtv_browser.h"
#include "mdtv_fav_file.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
#define NDEBUG  1
#ifndef NDEBUG
#define DEBUG_TRACE(P) {fprintf (stderr, "FIXME: %s,%d,%s: %s\n", __FILE__, __LINE__, __FUNCTION__ ,(P));}

#define ERROR_PRINT(fmt...) fprintf (stderr, "mdtv_fav_file[ERROR]:"fmt)
#define DEBUG_PRINT(fmt...) fprintf (stderr, "mdtv_fav_file[DEBUG]:"fmt)

#else
#define DEBUG_TRACE(P) 
#define ERROR_PRINT(fmt...) fprintf (stderr, "mdtv_fav_file[ERROR]:"fmt)
#define DEBUG_PRINT(fmt...)
#endif

#define     FAV_FILE_NAME       ("fav.xml")
#define     FAV_XSL_FILE_NAME   ("fav.xsl")
#define     MAX_XML_WEBSITE_NUM    (MDTV_FAV_ICON_MAX_NUMBER) 

#define     XML_LABEL_ROOT      ("favorite")
#define     XML_LABEL_WEBSITE   ("website")
#define     XML_LABEL_URL       ("url")
#define     XML_LABEL_TITLE     ("title")
#define     XML_LABEL_IMAGE     ("image")
#define     XML_LABEL_ID        ("id")
#define     XML_LABEL_LAST_TIME ("last_time")
#define     XML_LABEL_MAX_ID    ("max_id")
#define     XML_LABEL_NUMBER    ("number") 

#define     CFG_SECTION_NAME_GLOBAL ("global")
#define     CFG_KEY_NAME_FAVORITE_PATH ("favorite_path")
#define     XPATH_MAX           (500)
#define     URL_MAX             (1000)
#define     MAX_STR_SIZE_INT    (30)

#define     MDTV_ETCFILE        ("mdtv.cfg")

static int fav_image_width = 300;// width of mdolphin window
static int fav_image_height = 300;// height of mdolphin window
extern char MDTV_ETCFILEPATH[MAX_PATH+1];

static BOOL add_fav_node_prev_sibling(xmlNodePtr root_node, MDTV_PTR_FAV_INFO p_fav_info);


//static int dialog_confirm(HWND parent, const char *url, const char *title);

static xmlDocPtr get_doc (const char *docname) {

    xmlDocPtr doc = NULL;
    int res, size = 1024;
    char chars[2048];
    xmlParserCtxtPtr ctxt;
    xmlNodePtr curNode;
    FILE *f;

    if( !docname ){
        ERROR_PRINT( "[Error]:parameter error. docname:%s\n",docname );  
        goto RET_1;
    }
    f = fopen(docname, "rw");
    if (f == NULL) {
        ERROR_PRINT("[Error] fopen () : can't open file: %s\n",docname);
        goto RET_1;
    }//else

    res = fread(chars, 1, 4, f);
    if (res > 0) {
        ctxt = xmlCreatePushParserCtxt(NULL, NULL, chars, res, docname);
        if( NULL == ctxt ){
            ERROR_PRINT("[ERROR]xmlCreatePushParserCtxt():%s\n", docname);
            goto RET_2;
        }

        while ((res = fread(chars, 1, size, f)) > 0) {
            res = xmlParseChunk(ctxt, chars, res, 0);
            if( 0 != res ){
                ERROR_PRINT("[ERROR]xmlParseChunk():%s\n", docname);
                goto RET_3;
            }
        }
        res = xmlParseChunk(ctxt, chars, 0, 1);
        if( 0 != res ){
            ERROR_PRINT("[ERROR]xmlCreatePushParserCtxt():%s\n", docname);
            goto RET_3;
        }
        doc = ctxt->myDoc;
        curNode = xmlDocGetRootElement(doc); 
        if (NULL == curNode){
            ERROR_PRINT("[ERROR]xmlDocGetRootElement():  empty document\n");
            goto RET_3;
        } 
        DEBUG_PRINT("              xmlDocGetRootElement()  success, \n" );
    }
RET_3:
    xmlFreeParserCtxt(ctxt);
RET_2:
    fclose(f);
RET_1:
    return doc;
}

static xmlXPathObjectPtr get_node_set_for_reg_expr (xmlDocPtr doc, xmlChar *xpath){
    if( !doc || !xpath ){
        DEBUG_PRINT( "[Error]:parameter error. \n" );  
        return NULL;
    }
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    context = xmlXPathNewContext(doc);
    result = xmlXPathEvalExpression(xpath, context);
    if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
        ERROR_PRINT("No result\n");
        return NULL;
    }
    xmlXPathFreeContext(context);
    return result;
}

static xmlNodePtr get_root_node( const xmlDocPtr doc )
{
    if( !doc ){
        ERROR_PRINT( "[Error]:parameter error. \n" );  
        return NULL;
    }

    xmlNodePtr root  = xmlDocGetRootElement( doc );
    if( !root 
        || !root->name 
        || xmlStrcmp(root->name, (const xmlChar*)XML_LABEL_ROOT))
    {
        ERROR_PRINT("Error: Can't get the root node of favorite xml file!\n");
        xmlFreeDoc( doc );
        return NULL;  
    }
    return root;
}

static int get_root_node_prop( xmlNodePtr root_node, const char *prop_name )
{
    if( !root_node || !prop_name ){
        ERROR_PRINT("[ERROR]: PARAMETER Error!\n"); 
        return -1;
    }

    xmlChar *strProp = xmlGetProp( root_node, (const xmlChar*)prop_name );
    if( !strProp ){
        ERROR_PRINT("Warning: can't get root's property\n"); 
        return -1;
    }
    int intProp = (int)atol( (const char *)strProp );
    xmlFree(strProp);
    // convert string to int
    return intProp;
}
static int get_fav_info_id( const xmlDocPtr doc )
{
    if( !doc ){
        ERROR_PRINT("[ERROR]: PARAMETER Error!\n"); 
        return -1;
    }
    xmlNodePtr root = get_root_node( doc );
    if ( !root ){
        return -1;
    }
#if 0
    str_max_id = xmlGetProp( root, (const xmlChar*)XML_LABEL_MAX_ID );
    if( !str_max_id ){
        DEBUG_PRINT("Warning: can't get max_id\n"); 
        return -1;
    }
    // convert string to int
    int max_id = (int)atol( (const char *)str_max_id );   
#endif

    unsigned int max_id = get_root_node_prop( root, XML_LABEL_MAX_ID );
    if(max_id < 0 ){
        ERROR_PRINT("[ERROR]:Can't get root's property: max_id\n");
        return -1;
    }
    return max_id+1;
}

static char * get_fav_info_last_time()
{
    char *last_time = (char*) malloc(10);
    return last_time;
}
#if 1
BOOL read_bmp_file(const char* infile,  unsigned char **addr,  int *nLen)
{
    FILE* fin;
	fin = fopen(infile, "rb");
	if(fin == NULL)
		return FALSE;

	fseek(fin, 0, SEEK_END);
	int len = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	if(len <= 0){
		fclose(fin);
		return FALSE;
	}
	
	unsigned char* bytes = (unsigned char*)malloc( len);
	fread(bytes, 1, len, fin);
    *addr = bytes;
    *nLen = len;
	fclose(fin);
	return TRUE;
}
#endif
#if 0
static BOOL JpegCompress(const char* jpeg_file_name, const char* bmp_file_name, int width,int height )
{
    DEBUG_PRINT("*************** JpegCompress() start!\n"); 
    BOOL ret_val;
    struct jpeg_compress_struct jcs;

	// 声明错误处理器，并赋值给jcs.err域
 	struct jpeg_error_mgr jem;
 	jcs.err = jpeg_std_error(&jem);

 	jpeg_create_compress(&jcs);
    unsigned char *pDataConv;
    int nLen;
    ret_val = read_bmp_file(bmp_file_name,  &pDataConv,  &nLen);
 	if ( !ret_val ) 
 	{
        ERROR_PRINT("[ERROR]read bmp file!\n");
  		return FALSE;
 	}
    
    DEBUG_PRINT("            start opening jpeg file!\n"); 
    DEBUG_PRINT("            width:%d, height:%d, bmp_file_name:%s, jpeg_file_name:%s\n", \
                               width, height, bmp_file_name, jpeg_file_name ); 
    FILE *f=fopen(jpeg_file_name,"wb");
 	if (f==NULL) 
 	{
        ERROR_PRINT("[ERROR]open jpeg file!\n");
  		free( pDataConv );
  		return FALSE;
 	}

 	jpeg_stdio_dest(&jcs, f);
    jcs.image_width = width;    // 为图的宽和高，单位为像素 
 	jcs.image_height = height;
 	jcs.input_components = 3;   // 在此为1,表示灰度图， 如果是彩色位图，则为3 
 	jcs.in_color_space = JCS_RGB;//JCS_GRAYSCALE; //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像 

 	jpeg_set_defaults(&jcs); 
	jpeg_set_quality (&jcs, 300, TRUE);
	jpeg_start_compress(&jcs, TRUE);

 	JSAMPROW row_pointer[1];   // 一行位图
 	int row_stride;      // 每一行的字节数 

	 row_stride = jcs.image_width*3;  // 如果不是索引图,此处需要乘以3

    DEBUG_PRINT("            start compress image!\n"); 
 	// 对每一行进行压缩
 	//while (jcs.next_scanline < jcs.image_height) {
    int y;
 	for(y=height-1; y>=0; y--){
     	//row_pointer[0] = & pDataConv[y * row_stride];
     	//row_pointer[0] = & pDataConv[jcs.next_scanline * row_stride];
     	row_pointer[0] = (unsigned char*)( pDataConv + y*row_stride + 600);
     	jpeg_write_scanlines(&jcs, row_pointer, 1);
 	}
    DEBUG_PRINT("            end compress image!\n"); 

 	jpeg_finish_compress(&jcs);
    jpeg_destroy_compress(&jcs);
    return TRUE;
}
#endif

static char* get_fav_info_image( int id, HWND hWnd , const char *fav_file_path )
{
    int ret_val;
    RECT rc;
    if( !hWnd ){
        ERROR_PRINT("[ERROR]: PARAMETER Error!\n"); 
        return NULL;
    }
    char *image_bmp_file_name = (char*)malloc( PATH_MAX );
    int count = snprintf(image_bmp_file_name,PATH_MAX,"%s/%d%s",fav_file_path, id, ".bmp");
    BITMAP bmp = {0};
    HDC hdc = GetClientDC( hWnd );
    GetWindowRect(hWnd, &rc );
    HDC memDC = CreateCompatibleDCEx(hdc, RECTW(rc), RECTH(rc));
    SetBitmapScalerType(memDC, BITMAP_SCALER_BILINEAR);
    GetBitmapFromDC(hdc, 0, 0, RECTW(rc) , RECTH(rc), &bmp);
    FillBoxWithBitmap(memDC,0,0,fav_image_width,fav_image_height, &bmp);
    GetBitmapFromDC(memDC, 0, 0, fav_image_width, fav_image_height, &bmp);
    ret_val = SaveBitmapToFile( memDC, &bmp, image_bmp_file_name) ;	
    if( ret_val < 0 )
       ERROR_PRINT( "[WARNING]:Can't catch image file!\n" );
    usleep(100000);
    
#if 0
    char *image_jpg_file_name = (char*)malloc( PATH_MAX );
    count = snprintf(image_jpg_file_name,PATH_MAX,"%s/%d%s",fav_file_path, id, ".jpg");
    if( count < 0 ){
       DEBUG_PRINT( "Error:Can't get image jpeg file name!\n" );  
       return NULL; 
    }

    DEBUG_PRINT("            fav_image_width:%d, fav_image_height:%d\n", \
                               fav_image_width, fav_image_height ); 
    ret_val = JpegCompress(image_jpg_file_name, image_bmp_file_name, fav_image_width, fav_image_height);
    printf("JpegCompress()\n");
    if( !ret_val ){
        DEBUG_PRINT( "[WARNING]:Can't catch image file!\n" );
        return NULL; 
    }
    ret_val = unlink( image_bmp_file_name );
    if( ret_val < 0 ){
        DEBUG_PRINT( "[Warning]:Can't get image file name when deleting node !\n" );  
    }
#endif
    count = snprintf(image_bmp_file_name,PATH_MAX,"%s/%d%s","res/fav", id, ".bmp");
    if( count < 0 ){
       ERROR_PRINT( "Error:Can't get image file name!\n" );  
       return NULL;
    }
	DeleteMemDC(memDC);
    return image_bmp_file_name ;
}

static BOOL get_node_property_str( const xmlNodePtr node, const char *prop_name, char *prop_val, int val_len )
{
#if 1
    if( !prop_name || !node || !prop_val || val_len<=0 ){
        DEBUG_PRINT("[ERROR]: PARAMETER Error!\n"); 
        return FALSE;
    }
    xmlNodePtr child_node;
    xmlChar *pStr;
    for( child_node=node->children; child_node; child_node=child_node->next){
        DEBUG_PRINT("      : %s\n",child_node->name);
        if( child_node->type != XML_ELEMENT_NODE ){
            continue;
        } 
        if( !xmlStrcmp( child_node->name, (const xmlChar*)prop_name) ){
            pStr = xmlNodeGetContent(child_node);
            if( !pStr ){
                ERROR_PRINT("[Warning]: Can't get image file name!\n"); 
                xmlFree( pStr );
                return FALSE;
            }
            strcpy( prop_val, (char*)pStr );
            DEBUG_PRINT("proprety name: %s, property value:%s, property value's size\n",prop_name, prop_val, sizeof(prop_val));
            xmlFree( pStr );
            return TRUE;
        }
    }//for
#endif
    return FALSE;
}
static BOOL get_node_image_file_name( const xmlNodePtr node, char *image_file_name )
{
#if 1
    if( !image_file_name || !node ){
        ERROR_PRINT("[ERROR]get_node_image_file_name(): PARAMETER Error!\n"); 
        return FALSE;
    }
    xmlNodePtr child_node;
    xmlChar *pStr;
    for( child_node=node->children; child_node; child_node=child_node->next){
        DEBUG_PRINT("      get_node_image_file_name():node name: %s",child_node->name);
        if( child_node->type != XML_ELEMENT_NODE ){
            continue;
        }
        pStr = xmlNodeGetContent(child_node);
        DEBUG_PRINT("    %s\n",(char*)pStr);
        if( !xmlStrcmp( child_node->name, (const xmlChar*)XML_LABEL_IMAGE) ){
            if( !pStr ){
                ERROR_PRINT("[Warning]get_node_image_file_name(): Can't get image file name!\n"); 
                xmlFree( pStr );
                return FALSE;
            }
            if( strlen( (char*)pStr ) > PATH_MAX ){
                ERROR_PRINT("[Warning]get_node_image_file_name(): image file's path is too long!\n");
                xmlFree( pStr );
                return FALSE;
            }
            strcpy( image_file_name, (char*)pStr );
            DEBUG_PRINT("   deleting  image: %s\n",(char*)pStr);
            xmlFree( pStr );
            return TRUE;
        }
    }//for
#endif
    return TRUE;
    //return FALSE;
}
// set number = number + step
static BOOL update_root_info_number(xmlDocPtr doc, int step){
    xmlNodePtr root = get_root_node( doc );
    if ( !root ){
        return -1;
    }
    int number = get_root_node_prop( root, XML_LABEL_NUMBER );
    if( number < 0 || number > MAX_XML_WEBSITE_NUM){
        ERROR_PRINT("[ERROR]:Can't get root's property: number\n");
        return -1;
    }
    DEBUG_PRINT("       root's number property: number=%d\n", number); 
    if( number+step > MAX_XML_WEBSITE_NUM ){
        number = MAX_XML_WEBSITE_NUM;
    } else if( number+step < 0 ){
        number = 0;
    } else {
        number = number + step;
    }
    char tmp[MAX_STR_SIZE_INT];
    snprintf( tmp, MAX_STR_SIZE_INT, "%d", number );
    xmlAttrPtr tmpAttr = xmlSetProp( root, (const xmlChar*)XML_LABEL_NUMBER, (const xmlChar*)tmp ); 
    if( NULL == tmpAttr ){
        ERROR_PRINT("[ERROR]:Can't set root property: number\n");
        return FALSE;
    }
    return TRUE;
}
static BOOL del_existed_node( const xmlDocPtr doc, const char* url )
{
    if( !doc || !url ){
        DEBUG_PRINT("[ERROR]: parameter Error!\n"); 
        return FALSE;
    }
    int ret_val;
    char image_file_name[PATH_MAX];
    char str_xpath[XPATH_MAX];
    int count = snprintf( str_xpath, XPATH_MAX,"/%s/%s[%s='%s']", XML_LABEL_ROOT, XML_LABEL_WEBSITE, XML_LABEL_URL, url );
    DEBUG_PRINT( "xpath:%s\n",str_xpath);
    if( count < 0 ){
       ERROR_PRINT( "Error:Can't get xpath !\n" );  
       return FALSE; 
    }
    //snprintf(xpath, "/favorite/website[url='%s']",url);
    xmlNodeSetPtr nodeset;
    xmlXPathObjectPtr result;
    int i;
    result = get_node_set_for_reg_expr (doc, (xmlChar*)str_xpath);
    if (result) {
        nodeset = result->nodesetval;
        DEBUG_PRINT("       the number of website with the same url: nodeset->nodeNr=%d\n", nodeset->nodeNr); 
        for (i=0; i < nodeset->nodeNr; i++) {
#if 1
            ret_val = get_node_image_file_name( nodeset->nodeTab[i], image_file_name );
            if( !ret_val ){
                ERROR_PRINT( "[Warning]:Can't get image file name when deleting node !\n" );  
            }
            ret_val = unlink( image_file_name );
            if( ret_val < 0 ){
                ERROR_PRINT( "[Warning]:Can't delete image file when deleting node !\n" );  
            }
#endif
            xmlUnlinkNode(nodeset->nodeTab[i]);
            xmlFree( nodeset->nodeTab[i] );
        }//for
        update_root_info_number(doc, -i );

        xmlXPathFreeObject (result);
    }//if
    return TRUE;
}


static xmlNodePtr get_first_child(const xmlNodePtr root )
{
    if( !root ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return NULL;
    }
    xmlNode *cur_node;
   
    for( cur_node = root->children; cur_node ; cur_node = cur_node->next){
        if( cur_node->type != XML_ELEMENT_NODE)
            continue;
//        DEBUG_PRINT("cur_node->type != XML_ELEMENT_NODE)\n");
 
        if( !xmlStrcmp( cur_node->name, (const xmlChar*)XML_LABEL_WEBSITE)){
            return cur_node; 
        }
    }
    DEBUG_PRINT("No Child Node\n");
    return NULL;
}

static int check_fav_file_full( xmlDocPtr doc ) 
{
    if( !doc ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return -1;
    }
    xmlNodePtr root = get_root_node( doc );
    if ( !root ){
        return -1;
    }
    int number = get_root_node_prop( root, XML_LABEL_NUMBER );
    if( number < 0 ){
        ERROR_PRINT("[ERROR]:Can't get root's property: number\n");
        return -1;
    }
    DEBUG_PRINT("       check_fav_file_full():root's number property: number=%d\n", number); 
    if ( number >= MAX_XML_WEBSITE_NUM ){
        return TRUE;
    } else {
        return FALSE;
    }
}

static xmlNodePtr get_last_node( xmlDocPtr doc )
{
    if( !doc ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return NULL;
    }
    char str_xpath[XPATH_MAX];
    int count = snprintf( str_xpath, XPATH_MAX,"/%s/%s[last()]", XML_LABEL_ROOT, XML_LABEL_WEBSITE);
    DEBUG_PRINT( "xpath:%s\n",str_xpath);
    if( count < 0 ){
       ERROR_PRINT( "Error:Can't get xpath !\n" );  
       return FALSE; 
    }
    xmlNodePtr last_node=NULL;
    xmlXPathObjectPtr result;
    result = get_node_set_for_reg_expr (doc, (xmlChar*)str_xpath);
    if (result) {
        last_node = result->nodesetval->nodeTab[0];
        xmlXPathFreeObject (result);
    }//if
    return last_node;
}
#if 0
static xmlDocPtr create_xml_file(const char *fav_file_name )// create xml file with file head and root node
{
    if( !fav_file_name ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return NULL;
    }
    return NULL;
}
#endif
/* get absolute path from relative path ( ~/path ./path  /path/  path/ )*/
static BOOL get_absolute_path_from_path(char *absolute_path, const char *rel_path, int size)
{
    if( !absolute_path || !rel_path || size<0 ){
        ERROR_PRINT("parameter error!\n");
        return FALSE;
    }
#if 1
    char tmp_rel_path[PATH_MAX];
    strcpy(tmp_rel_path, rel_path);
    struct passwd *pwd;
    // get favorite file path
    int len = strlen(tmp_rel_path);
    if( len == 0 ){
        return TRUE;
    }
    if(tmp_rel_path[len-1]=='/' && len != 1){
        tmp_rel_path[len-1] = 0;
    }
    switch(tmp_rel_path[0])
    {
        case '/':
            strcpy(absolute_path, tmp_rel_path);
            break;
        case '~':
            if( tmp_rel_path[1] != '/' ){
                ERROR_PRINT("fav path error in configuration file!\n");
                return FALSE;
            }
            if ((pwd = getpwuid (geteuid ())) != NULL) {
                strcpy (absolute_path, pwd->pw_dir);
                int len1 = strlen(absolute_path);
                if(absolute_path[len1-1]=='/' ){
                    absolute_path[len1-1] = 0;
                }
                strcat ( absolute_path, tmp_rel_path+1); 
            }
            break;
        case '.':
            int start_pos;
            if( tmp_rel_path[1] == '/' ){ // ./path
                start_pos = 2 ;
            } else {            // .mdtv/path
                start_pos = 0 ; 
            }
            if ( getcwd (absolute_path, MAX_PATH) ){
                int len2 = strlen(absolute_path);
                if(absolute_path[len2-1]!='/'){
                    strcat(absolute_path, "/");
                }
                strcat(absolute_path, tmp_rel_path+start_pos);
            }
            break;
        default:
            if ( getcwd (absolute_path, MAX_PATH) ){
                int len2 = strlen(absolute_path);
                if(absolute_path[len2-1]!='/'){
                    strcat(absolute_path, "/");
                }
                strcat(absolute_path, tmp_rel_path);
            }
            break;
    }
DEBUG_PRINT( "          relative path:%s       absolute_path:%s\n", rel_path, absolute_path);  
    return TRUE;
#endif
}
/* get the res or the fav absolute path in the configuration file :  get resource path when type = RES_PATH
 *  favorite path when type = FAV_PATH */
BOOL get_path( char *file_path, int size, PATH_PARAM_TYPE path_type)
{
    char tmp_rel_file_path[PATH_MAX];
    char str_path_type[PATH_MAX];
    int ret_val;
    switch(path_type)
    {
        case RES_PATH: 
            strcpy(str_path_type, "res_path" );
            break;
        case FAV_PATH:
            strcpy(str_path_type, CFG_KEY_NAME_FAVORITE_PATH );
            break;
        default:
            return FALSE;
    }
    if( GetValueFromEtcFile( MDTV_ETCFILEPATH, CFG_SECTION_NAME_GLOBAL, str_path_type, tmp_rel_file_path, PATH_MAX) != ETC_OK){
        ERROR_PRINT("[ERROR]config file!");
        return FALSE;
    }
    ret_val = get_absolute_path_from_path(file_path, tmp_rel_file_path, PATH_MAX);
    if( !ret_val ){
        ERROR_PRINT("fav path error in configuration file!\n");
        return FALSE;
    }
    return TRUE;
}
static BOOL mk_dir( const char *path)
{
    if( !path ){
        ERROR_PRINT("parameter Error!\n"); 
        return FALSE;
    }
    DEBUG_PRINT("   create path :%s\n",path);
    char path_cpy[PATH_MAX];
    strcpy( path_cpy, path );

    int len = strlen( path_cpy );
    if( path_cpy[len-1] == '/' && len != 1){
        path_cpy[len-1] = 0;
    }
    char tmp_path[PATH_MAX];
    char *end = path_cpy;
    while(1){
        memset( tmp_path, 0, PATH_MAX );
        end = strchr( end+1, '/' ); 
        if( !end ){
            strcpy( tmp_path, path_cpy );
        } else {
            memcpy( tmp_path, path_cpy, end-path_cpy); 
        }
        if(access(tmp_path,0)==-1) {
            DEBUG_PRINT("   path not exsit, create it :%s\n",tmp_path);
            if (mkdir(tmp_path,0777)){
                ERROR_PRINT( "[Error]:create file  failed:%s\n", tmp_path );  
                return FALSE;
            }
        }
        if( !end ){
            break;
        }
    }//while(1)
    return TRUE;
}
static xmlDocPtr  get_fav_file_doc(char *fav_file_path )
{
    xmlDocPtr doc;

    DEBUG_PRINT( "********* get_fav_file_doc()!\n" );  
    DEBUG_PRINT( "       MDTV_ETCFILEPATH:%s\n", MDTV_ETCFILEPATH);
    char res_path[PATH_MAX];
    int ret_val = get_path( fav_file_path, PATH_MAX, FAV_PATH);
    if( !ret_val ){
        ERROR_PRINT("[ERROR]favorite path in config file!");
        return NULL;
    }

    ret_val = get_path( res_path, PATH_MAX, RES_PATH);
    if( !ret_val ){
        ERROR_PRINT("[ERROR]resouce path in config file!");
        return NULL;
    }
    DEBUG_PRINT( "       fav_absolute_path:%s, res_absolute_path:%s\n", fav_file_path, res_path);  
    // add path to file name string
    char fav_file_name[PATH_MAX];
    int count = snprintf(fav_file_name,PATH_MAX,"%s/%s",fav_file_path,FAV_FILE_NAME);
    if( count < 0 ){
       ERROR_PRINT( "Error:Can't get favorite file name!\n" );  
       return NULL;
    }

    // if xml file doesn't exist, create xml file 
    if(access(fav_file_name,0)==-1){
        DEBUG_PRINT("[WARNING]file not exist:%s\n",fav_file_name);
        if(access(fav_file_path,0)==-1) {
            DEBUG_PRINT("[WARNIN]path not exist:%s\n",fav_file_path);
            if ( mk_dir(fav_file_path) !=TRUE){
                ERROR_PRINT( "[Error]:create file  failed!!!\n" );  
                return NULL;
            }
        }
        char copy_cmd[PATH_MAX];
        int count = snprintf(copy_cmd,PATH_MAX,"cp %s/fav/%s %s",res_path,FAV_FILE_NAME, fav_file_path);
        if( count < 0 ){
            ERROR_PRINT( "Error:Can't get copy favorite file cmd!\n" );  
           return NULL;
        }
        DEBUG_PRINT( "       copy_cmd:%s\n", copy_cmd);  
        system(copy_cmd);
        count = snprintf(copy_cmd,PATH_MAX,"cp %s/fav/%s %s",res_path,FAV_XSL_FILE_NAME, fav_file_path);
        if( count < 0 ){
            ERROR_PRINT( "Error:Can't get copy favorite file cmd!\n" );  
           return NULL; 
        }
        DEBUG_PRINT( "       copy_cmd:%s\n", copy_cmd);
        system(copy_cmd);
    }// if( access() )
//        doc = create_xml_file( fav_file_name);// create xml file with file head and root node
    
    DEBUG_PRINT( "       fav_file_name:%s\n", fav_file_name);  
    doc = get_doc( fav_file_name );
    return doc ;
}


static BOOL set_favorite_info( MDTV_PTR_FAV_INFO p_fav_info , const xmlDocPtr doc, HWND hWnd, const char *fav_file_path  ) 
{
    if( !p_fav_info || !doc || !hWnd ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return FALSE;
    }

    p_fav_info->m_id = get_fav_info_id( doc );
    if( p_fav_info->m_id < 0 ){
        ERROR_PRINT("[ERROR]:Can't get current id\n ");
        return FALSE;
    }
    DEBUG_PRINT("   get_fav_info_id():get id:%d\n",p_fav_info->m_id );
       
    // set p_fav_info->m_last_time with current time
    if( p_fav_info->m_last_time ){
        free( p_fav_info->m_last_time );
    }
    p_fav_info->m_last_time= get_fav_info_last_time();
    if( p_fav_info->m_last_time == NULL ){
        ERROR_PRINT("[ERROR]:Can't get current time\n ");
        return FALSE;
    }

    // file name with absolute path of the picture of favorite website
    if( p_fav_info->m_image ){
        free( p_fav_info->m_image );
    }
    p_fav_info->m_image = get_fav_info_image( p_fav_info->m_id ,  hWnd , fav_file_path);
    if( p_fav_info->m_image == NULL ){
        ERROR_PRINT("[ERROR]:Can't get current image file\n ");
        return FALSE;
    }
    DEBUG_PRINT("   get_fav_info_image():get and save current image file:%s\n",p_fav_info->m_image);
    return TRUE;
}

static BOOL save_fav_file( const xmlDocPtr doc, const char *fav_file_name )
{
    BOOL ret_val=TRUE;
    xmlChar *xmlbuff;
    int buffersize;
    int fd;
    char rm_cmd[PATH_MAX];
    int res;
    if( !doc || !fav_file_name){
        ERROR_PRINT("[ERROR]save_fav_file(): parameter Error!\n"); 
        ret_val = FALSE;
        goto RET_1;
    }
    
    xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
    DEBUG_PRINT("    write --------------buffersize : %d\n", buffersize);

    sprintf(rm_cmd,"rm -f %s", fav_file_name);
    DEBUG_PRINT("    cmd -------------- : %s\n", rm_cmd);
    system(rm_cmd);
    fd = open(fav_file_name, O_CREAT|O_WRONLY, 0777);
    if ( fd == -1 ){
        ERROR_PRINT("[Error] open () : can't open file: %s\n",fav_file_name);
        perror("open()");
        ret_val = FALSE;
        goto RET_2;
    }
    res = write(fd, (char*)xmlbuff,buffersize);
    if( res <= 0 ){
        ERROR_PRINT( "[ERROR]:write() : write %d bytes\n",res );  
        perror( "[ERROR]:write() :  " );  
        ret_val = FALSE;
        goto RET_3;
    }
    DEBUG_PRINT( "   xmlSaveFormatFile() successfully: write %d bytes\n",res );  
    DEBUG_PRINT( "   save_favorite_info() successfully: fav_file_name=%s\n",fav_file_name );  
RET_3: 
    close(fd);
RET_2:
    xmlFree(xmlbuff);
RET_1:
    return ret_val;
}
static BOOL save_favorite_info( const MDTV_PTR_FAV_INFO p_fav_info , const xmlDocPtr doc, const char *fav_file_path )
{
    if( !p_fav_info || !doc ){
        DEBUG_PRINT("[ERROR]: parameter Error!\n"); 
        return FALSE;
    }
    BOOL ret_val;
    xmlNodePtr root_node = xmlDocGetRootElement(doc);
        // if the node with the same url exsits , delete it
    ret_val = del_existed_node( doc , p_fav_info->m_url );
    if( !ret_val) {
        ERROR_PRINT("[WARNING]:Can't delete node of url\n");
    } else {
        DEBUG_PRINT("       del_existed_node():delete node of url[%s] successfully\n",p_fav_info->m_url);
    }
        
    // add_fav_node( root_node, p_fav_info);
    ret_val = add_fav_node_prev_sibling(root_node, p_fav_info);
    if( !ret_val ){
        ERROR_PRINT("[ERROR]:Can't add Node!\n");
        return FALSE;
    }
    DEBUG_PRINT("       add_fav_node_prev_sibling():add the node successfully!\n");
        
    char fav_file_name[PATH_MAX];
    int count = snprintf(fav_file_name,PATH_MAX,"%s/%s",fav_file_path,FAV_FILE_NAME);
    if( count < 0 ){
       ERROR_PRINT( "Error:Can't get favorite file name!\n" );  
       return FALSE; 
    }

    ret_val = save_fav_file( doc, fav_file_name );
    if( !ret_val ){
        ERROR_PRINT("[ERROR]save_fav_file():Can't save to file:%s\n", fav_file_name);
        return FALSE;
    }

    return TRUE;
}
int mdtv_save_favorite_website( HWND hWnd )
{
    int ret_val=TRUE;
    xmlDocPtr doc;
    MDTV_PTR_FAV_INFO p_fav_info;
    BrowserTitleWndInfo *p_info;
    xmlKeepBlanksDefault(0);

    if( !hWnd ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        ret_val = FALSE;
        goto ERROR_1;
    }

	p_info = (BrowserTitleWndInfo *)GetWindowAdditionalData(hWnd);
    if(!p_info){
        ERROR_PRINT("[ERROR]: Can't get BrowserTitleWndInfo parameter!\n"); 
        ret_val = FALSE;
        goto ERROR_1;
    }
    p_fav_info = p_info->m_fav_info;
    if( !p_fav_info ){
        ERROR_PRINT("[ERROR]: Can't get MDTV_PTR_FAV_INFO parameter!\n"); 
        ret_val = FALSE;
        goto ERROR_1;
    }
    // check if the xml file exists , if no , create it with xml's head infomation , and return the file name with absolute path of xml file 
    char fav_file_path[PATH_MAX];
    doc = get_fav_file_doc(fav_file_path ) ;
    if( NULL == doc ){ 
        ERROR_PRINT("[ERROR]:get_fav_file_doc():Can't Get doc!\n");
        ret_val = FALSE;
        goto ERROR_1;
    }
    
    // check if number of records is bigger than 30 in favorite file
    ret_val = check_fav_file_full( doc );
    if( ret_val < 0 ){
        ERROR_PRINT("[ERROR]:check fav file full!\n");
        ret_val = FALSE;
        goto ERROR_2;
    }
    if ( ret_val == TRUE ) {
        char url[URL_MAX];
        char title[URL_MAX];
        char image_file_name[PATH_MAX];
        xmlNodePtr last_node = get_last_node( doc );

        // get url and title of last node
        ret_val = get_node_property_str( last_node, XML_LABEL_URL, url, URL_MAX );
        if(!ret_val){
            ERROR_PRINT("[WARNIN]can't get url property of last node");
        }
        ret_val = get_node_property_str( last_node, XML_LABEL_TITLE, title, URL_MAX );
        if(!ret_val){
            ERROR_PRINT("[WARNIN]can't get title property of last node");
        }
        //if ( dialog_confirm( hWnd, url, title ) == IDOK ){
        if ( 1 ){
            ret_val = get_node_image_file_name( last_node, image_file_name );
            if( !ret_val ){
                ERROR_PRINT( "[Warning]:Can't get image file name when deleting node !\n" );  
            }
            ret_val = unlink( image_file_name );
            if( ret_val < 0 ){
                ERROR_PRINT( "[Warning]:Can't get image file name when deleting node !\n" );  
            }
            xmlUnlinkNode(last_node);
            update_root_info_number(doc, -1 );// set number = number - 1
        } else {
            ret_val = NOT_OVERWRITE;
            goto ERROR_2;
        }
    }//if(ret_val == TRUE)
        
    // check the file and set  m_image, m_last_time, m_id
    ret_val = set_favorite_info( p_fav_info , doc, hWnd, fav_file_path ); // hWnd parameter is used when get image
    if( !ret_val ){
        ERROR_PRINT("[ERROR]:set_favorite_info()\n");
        ret_val = FALSE;
        goto ERROR_2;
    }
        
    ret_val = save_favorite_info( p_fav_info , doc, fav_file_path ); // save FAVORITE_INFO into fav.xml
    if( !ret_val ){
        ERROR_PRINT("[ERROR]:save_favorite_info()\n");
        ret_val = FALSE;
        goto ERROR_2;
    }
ERROR_2: xmlFreeDoc( doc );
ERROR_1: return ret_val;
}

static BOOL    change_root_node_info( xmlNodePtr root_node )
{
    if( !root_node ){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return FALSE;
    }
    char tmp[MAX_STR_SIZE_INT];
    int max_id = get_root_node_prop( root_node, XML_LABEL_MAX_ID );
    if( max_id < 0 ){
        ERROR_PRINT("[ERROR]:Can't get root's property: max_id\n");
        return FALSE;
    }
    snprintf( tmp, MAX_STR_SIZE_INT, "%d", max_id+1 );
    DEBUG_PRINT( "     change_root_node_info(),root node property:max_id[%s]!\n",tmp);
    xmlAttrPtr tmpAttr = xmlSetProp( root_node, (const xmlChar*)XML_LABEL_MAX_ID,  (const xmlChar*)tmp ); 
    if( NULL == tmpAttr ){
        ERROR_PRINT("[ERROR]:Can't set root property: max_id\n");
        return FALSE;
    }

//static BOOL update_root_info_number( doc, int step){
    int number = get_root_node_prop( root_node, XML_LABEL_NUMBER );
    if( number < 0 ){
        ERROR_PRINT("[ERROR]:Can't get root's property: number\n");
        return FALSE;
    }
    number = ( number >= MAX_XML_WEBSITE_NUM)?MAX_XML_WEBSITE_NUM:(number+1);
    snprintf( tmp, MAX_STR_SIZE_INT, "%d", number );
    DEBUG_PRINT( "     change_root_node_info(),root node property:number[%s]!\n",tmp);
    tmpAttr = xmlSetProp( root_node, (const xmlChar*)XML_LABEL_NUMBER, (const xmlChar*)tmp ); 
    if( NULL == tmpAttr ){
        ERROR_PRINT("[ERROR]:Can't set root property: number\n");
        return FALSE;
    }
    return TRUE;
}
static BOOL add_fav_node_prev_sibling(xmlNodePtr root_node, MDTV_PTR_FAV_INFO p_fav_info)
{
#if 1
    if( !root_node || !p_fav_info){
        ERROR_PRINT("[ERROR]: parameter Error!\n"); 
        return FALSE;
    }
    xmlNodePtr ret_node;
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST(XML_LABEL_WEBSITE));
    xmlNodePtr first_child = get_first_child( root_node );//= xmlFirstElementChild(root_node);

    if ( !first_child )
        ret_node = xmlAddChild(root_node,node);
    else
        ret_node = xmlAddPrevSibling(first_child, node);
    if ( !ret_node ){
        ERROR_PRINT("[ERROR]:Can't add node!\n");
        return FALSE;
    }
    
    char tmp_buf[30];
    int count = snprintf(tmp_buf, 30, "%d",p_fav_info->m_id);
    if( count < 0 ){
        ERROR_PRINT( "[Error]:Can't get favorite file name!\n" );  
        return FALSE;
    }
    DEBUG_PRINT( "     add_fav_node_prev_sibling(),new node info:id[%s],title[%s],image[%s],url[%s]!\n",\
                                tmp_buf, p_fav_info->m_title,p_fav_info->m_image, p_fav_info->m_url);  
    xmlNewTextChild(node, NULL, BAD_CAST(XML_LABEL_ID), BAD_CAST(tmp_buf));
    xmlNewTextChild(node, NULL, BAD_CAST(XML_LABEL_TITLE), BAD_CAST(p_fav_info->m_title));
    
    xmlNewTextChild(node, NULL, BAD_CAST(XML_LABEL_IMAGE), BAD_CAST(p_fav_info->m_image));
    xmlNewTextChild(node, NULL, BAD_CAST(XML_LABEL_URL), BAD_CAST(p_fav_info->m_url));
#endif
    BOOL ret_val = change_root_node_info( root_node );
    if( !ret_val ){
        ERROR_PRINT("[ERROR]:Can't get root's property!\n");
        return FALSE;
    }
    return TRUE;
}

int mdtv_read_fav_info(const char *fav_file_name, FAV_ICONFLOW_INFO *p_iconinfo, unsigned max_num){
    xmlDocPtr doc;
    xmlNodePtr curNode;
    xmlNodePtr curChildNode;
    int cnt;
    memset(p_iconinfo, 0, max_num*sizeof(FAV_ICONFLOW_INFO));
    doc = xmlReadFile(fav_file_name,"GB2312",XML_PARSE_RECOVER); 
    if (NULL == doc)
    {
        ERROR_PRINT("Document not parsed successfully. \n");    
            return -1;
    }
    curNode = xmlDocGetRootElement(doc); 
    if (NULL == curNode || xmlStrcmp(curNode->name, BAD_CAST "favorite") )
    {
        ERROR_PRINT("empty document\n");
        xmlFreeDoc(doc);
        return -1;
    }

    BOOL cnt_need_add=FALSE;
    for(cnt=0,curNode = curNode->xmlChildrenNode; curNode!=NULL;  curNode=curNode->next){
        if(xmlStrcmp(curNode->name, (const xmlChar *)"website")){
            continue;
        }
        for(curChildNode = curNode->xmlChildrenNode; curChildNode!=NULL; curChildNode = curChildNode->next){
            if(!xmlStrcmp(curChildNode->name, (const xmlChar *)"image")){
                cnt_need_add = TRUE;
                p_iconinfo[cnt].m_img_path = (char*)xmlNodeGetContent(curChildNode);
                DEBUG_PRINT("%d: img_path:%s\n", cnt, p_iconinfo[cnt].m_img_path);
            }
            if(!xmlStrcmp(curChildNode->name, (const xmlChar *)"url")){
                p_iconinfo[cnt].m_url = (char*)xmlNodeGetContent(curChildNode);
                DEBUG_PRINT("%d: url:%s\n", cnt, p_iconinfo[cnt].m_url);
            }
            if(!xmlStrcmp(curChildNode->name, (const xmlChar *)"title")){
                p_iconinfo[cnt].m_title = (char*)xmlNodeGetContent(curChildNode);
                DEBUG_PRINT("%d: title:%s\n", cnt, p_iconinfo[cnt].m_title);
            }// if
        }// for
        if(cnt_need_add){
            cnt++;
            cnt_need_add=FALSE;
        }
    }// for
    xmlFreeDoc(doc);
    return 0;
}
void mdtv_uninit_fav_icon_info(const FAV_ICONFLOW_INFO *p_iconinfo, unsigned max_num){
    unsigned cnt;
    for(cnt=0; cnt<max_num; cnt++){
        if(p_iconinfo[cnt].m_url)
            xmlFree(p_iconinfo[cnt].m_url);
        if(p_iconinfo[cnt].m_img_path)
            xmlFree(p_iconinfo[cnt].m_img_path);
        if(p_iconinfo[cnt].m_title)
            xmlFree(p_iconinfo[cnt].m_title);
    }
    return;
}

#if 0
static int InputBoxProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
    if ( message == MSG_COMMAND ) {
        if ( wParam == IDOK ) {
            EndDialog (hDlg, 1);
        }else if (wParam == IDCANCEL){
            EndDialog (hDlg, 0);
        } 
    }
    
    return DefaultDialogProc (hDlg, message, wParam, lParam);
}
static int dialog_confirm(HWND parent, const char *url, const char *title)
{
	static DLGTEMPLATE DlgBoxInputLen =
	{
    	WS_BORDER | WS_CAPTION, 
	    WS_EX_NONE,
    	300, 200, 400, 200, 
		"Warning",
	    0, 0,
    	5, NULL,
	    0
	};

	static CTRLDATA CtrlInputLen [] =
	{ 
    	{
        	CTRL_STATIC,
	        WS_VISIBLE | SS_SIMPLE,
    	    80, 30, 260, 40, 
        	IDC_STATIC, 
	        "text",
    	    0
	    },
    	{
        	CTRL_STATIC,
	        WS_VISIBLE | SS_SIMPLE,
    	    80, 60, 260, 40, 
        	IDC_STATIC, 
	        "URL",
    	    0
	    },
    	{
        	CTRL_STATIC,
	        WS_VISIBLE | SS_SIMPLE,
    	    80, 90, 260, 40, 
        	IDC_STATIC, 
	        "Title",
    	    0
	    },
	    {
    	    CTRL_BUTTON,
        	WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
	        100, 130, 70, 30,
    	    IDOK, 
        	"确定",
	        0
    	},
	    {
    	    CTRL_BUTTON,
        	WS_TABSTOP | WS_VISIBLE | BS_DEFPUSHBUTTON, 
	        240, 130, 70, 30,
    	    IDCANCEL, 
        	"取消",
	        0
    	}
	};

    char tmp_url[URL_MAX];
    char tmp_title[URL_MAX];
    strcpy(tmp_url, "URL : ");
    strcat(tmp_url, url);
    strcpy(tmp_title, "Title : ");
    strcat(tmp_title, title);
	CtrlInputLen[0].caption = "你是否要删除最早收藏的网页!";
	CtrlInputLen[1].caption = tmp_url;
	CtrlInputLen[2].caption = tmp_title;
    DlgBoxInputLen.controls = CtrlInputLen;

    return DialogBoxIndirectParam (&DlgBoxInputLen, parent, InputBoxProc, 0);
}
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
