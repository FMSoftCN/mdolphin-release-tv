#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>              
#include <sys/time.h>              
#include <sys/ioctl.h>              
#include <sys/socket.h>              
#include <sys/wait.h>              
#include <sys/stat.h>
#include <net/if.h>              
#include <netinet/in.h>              
#include <net/if_arp.h>              
#include <net/route.h>
#include <arpa/inet.h>      
#include <netdb.h>
#include <getopt.h>
#include <errno.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <mdolphin/mdconfig.h>
#include <mdolphin/mdolphin.h>
#include <mdolphin/mdolphin_binding.h>
#include "mdtv_setting_homepage.h"
extern char MDTV_ETCFILEPATH[];
#define MDTV_ETCFILE1 "mdtv.cfg"
#if defined (_MGRM_THREADS)
HWND ime_hwnd;
#endif
HWND mdolphin_hwnd1;

#define HEADER_LEN 10
#define MAXPROPERTYNAME 32

#define CARET_REQUEST "/var/tmp/caretrequest"
#define CARET_REPLY "/var/tmp/caretreply"

#ifdef MDTV_HOME_URL
char home_url[256] = MDTV_HOME_URL;
#else
char home_url[256] = "http://www.minigui.com";
#endif

class HPages {
public:
    HPages();
    char* ip() { return m_ipAddr; }
    int ipSize() { return sizeof(m_ipAddr); }
private:
    char m_ipAddr[32];
};


HPages::HPages()
{
    strncpy(ip(), home_url, ipSize());

}

static JSObjectRef constructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
static void destructor(JSObjectRef object);

//static JSValueRef applySettings(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
static JSValueRef getPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);
static bool setPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception);

static JSNativeFunction methods[] = {
    {0, 0, 0}
};

static JSProperty properties[] = {
    {"save",         getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"close",        NULL,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {0, 0, 0, 0}
};

static JSNativeClass HomePages = {
    "HomePages",
    0,
    constructor,
    destructor,
    methods,
    properties,
};

static JSObjectRef constructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    HPages *homePages = new HPages;
    JSObjectRef newObj = JSObjectMake(context, mdolphin_lookup_jsnativeclass(HomePages.name), homePages);
    return newObj;
}

static void destructor(JSObjectRef object)
{
    HPages *homePages = (HPages*)JSObjectGetPrivate(object);
    delete homePages;
    JSObjectSetPrivate(object, 0);
}

static inline void JSValueToCStr(JSContextRef context, const JSValueRef value, JSValueRef* exception, char* array, size_t size)
{
    JSStringRef strRef = JSValueToStringCopy(context, value, exception);
    memset(array, 0x0, size);
    JSStringGetUTF8CString(strRef, array, size-1);
    JSStringRelease(strRef);
}

static inline JSValueRef CStrToJSValue(JSContextRef context, JSValueRef* exception, char* array, size_t size)
{
    JSStringRef strRef = JSStringCreateWithUTF8CString(array);
    JSValueRef valRef = JSValueMakeString(context, strRef); 
    JSStringRelease(strRef);
    return valRef;
}

extern HWND mdolphin_h;

/* Set the value of the property */
static bool setPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    HPages *homePages = (HPages*)JSObjectGetPrivate(object);
	if (!homePages)
	    return JSValueMakeUndefined(context);

    char name[MAXPROPERTYNAME];
    memset(name, 0x0, sizeof(name));
    JSStringGetUTF8CString(propertyName, name, sizeof(name)-1);
    if (!strcasecmp(name, "save"))
    {
        JSValueToCStr(context, value, exception, homePages->ip(), homePages->ipSize());
       strncpy(home_url, homePages->ip(), 255 );
       home_url[255] = '\0';
       printf("The homepage is =%s\n", home_url);
       //DestroyMainWindow(mdolphin_h);
    }
    else if(!strcasecmp(name, "close"))
    {
        //DestroyMainWindow(mdolphin_h);
        //PostMessage (mdolphin_h, MSG_CLOSE, 0, 0);
        printf("SendMessage--------------hwnd = %x\n", mdolphin_h);
    }
    return JSValueMakeUndefined(context);
}

/* Get the value of the property */
static JSValueRef getPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    HPages *homePages = (HPages*)JSObjectGetPrivate(object);
	if (!homePages)
	    return JSValueMakeUndefined(context);

    char name[MAXPROPERTYNAME];
    memset(name, 0x0, sizeof(name));
    JSStringGetUTF8CString(propertyName, name, sizeof(name)-1);
    if (!strcasecmp(name, "save"))
    {
        return CStrToJSValue(context, exception, homePages->ip(), homePages->ipSize());
    }
    return JSValueMakeUndefined(context);
}

void register_homepage(void)
{
    mdolphin_define_jsnativeclass(&HomePages);
}

char* get_setted_home_url()
{
    return home_url;
}

