#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>              
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
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <minigui/common.h>
#include <minigui/minigui.h>

#include <mdolphin/mdconfig.h>
#include <mdolphin/mdolphin.h>
#include <mdolphin/mdolphin_binding.h>
#include "mdtv_network.h"


#define MAXINTERFACES   16
#define MAXPROPERTYNAME 32
#define RESOLVE_FILE    "/etc/resolv.conf"
#define ROUTE_FILE      "/proc/net/route"
#define DHCPCD          "dhcpcd"

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

#if defined(SIOCADDRTOLD) || defined(RTF_IRTT)	/* route */
#define HAVE_NEW_ADDRT 1
#endif

#if HAVE_NEW_ADDRT
#define mask_in_addr(x) (((struct sockaddr_in *)&((x).rt_genmask))->sin_addr.s_addr)
#define full_mask(x) (x)
#else
#define mask_in_addr(x) ((x).rt_genmask)
#define full_mask(x) (((struct sockaddr_in *)&(x))->sin_addr.s_addr)
#endif

#define bb_str_default "255.255.255.0"
#define ENABLE_FEATURE_CLEAN_UP 1
#define ENABLE_FEATURE_ETC_NETWORKS 0

static const unsigned flagvals[] = { /* Must agree with flagchars[]. */
	RTF_GATEWAY,
	RTF_HOST,
	RTF_REINSTATE,
	RTF_DYNAMIC,
	RTF_MODIFIED,
#if ENABLE_FEATURE_IPV6
	RTF_DEFAULT,
	RTF_ADDRCONF,
	RTF_CACHE
#endif
};

#define IPV4_MASK (RTF_GATEWAY|RTF_HOST|RTF_REINSTATE|RTF_DYNAMIC|RTF_MODIFIED)

static bool isStaticIp = true;

static int do_ping(const char* addr);
static int read_line_value(const char* src, char* dst, int max);
static void set_flags(char *flagstr, int flags);
static char* rresolve(struct sockaddr_in *s_in, int numeric, uint32_t netmask);
static bool is_udhcpc_running(const char* buf, const char* except);

class NetSettings {
public:
    NetSettings();

    int apply();
    int test();

    bool isStaticIP() { return isStaticIp; }
    void enableStaticIP(bool value) { isStaticIp = value; }

    bool isSettled() { return m_isSettled; }
    void setSettled(bool value) { m_isSettled = value; }

    bool isConnected() { return m_isConnected; }
    void setConnected(bool value) { m_isConnected = value; }

    char* ifrName() { return m_ifrName; }
    char* ip() { return m_ipAddr; }
    char* netmask() { return m_netMask; }
    char* gateway() { return m_gateWay; }
    char* dnsServer() { return m_dnsServer; }
    char* dnsDomain() { return m_dnsDomain; }

    int ipSize() { return sizeof(m_ipAddr); }
    int netmaskSize() { return sizeof(m_netMask); }
    int gatewaySize() { return sizeof(m_gateWay); }
    int dnsServerSize() { return sizeof(m_dnsServer); }
    int dnsDomainSize() { return sizeof(m_dnsDomain); }

protected:

    char* getAvailInterface();
    bool getStaticIP(const char* ifname);
    bool setStaticIP(const char* ifname);

    bool getGateway();
    bool setGateway(char* ifname, const char* gateway);

    bool writeDNSConf(const char* filename);
    bool readDNSConf(const char* filename);

    bool isDHCPClientRunning();
    bool enableDHCPClient(bool enable);

    bool ifconfigUp(const char* ifname, bool enable);
    bool isIfUp(const char* ifname);
    bool waitIfrStatusChange(const char* ifrname, bool down, size_t timeout);

private:

    char m_ipAddr[32];
    char m_netMask[32];
    char m_gateWay[32];
    char m_dnsServer[64];
    char m_dnsDomain[64];

    char m_ifrName[32];

    bool m_isSettled;
    bool m_isConnected;
};


NetSettings::NetSettings()
    : m_isSettled(false)
    , m_isConnected(false)
{
    memset(m_dnsServer, 0x0, 64);
    memset(m_dnsDomain, 0x0, 64);
    memset(m_ipAddr, 0x0, 32);
    memset(m_netMask, 0x0, 32);
    memset(m_gateWay, 0x0, 32);

    strcpy(m_ifrName, getAvailInterface());
    strcpy(m_ipAddr, "0.0.0.0");
    strcpy(m_netMask, "255.255.255.0");
    strcpy(m_gateWay, "0.0.0.0");

    getStaticIP(m_ifrName);
    getGateway();
    readDNSConf(RESOLVE_FILE);

    if (isDHCPClientRunning())
        enableStaticIP(false);
}

bool NetSettings::getStaticIP(const char* ifname)
{
    int fd;
    char* str;
    struct ifreq req;
    struct sockaddr_in* sin;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return false;

    memset(&req, 0x0, sizeof(req));
    strcpy(req.ifr_name, ifname);

    /* Get IP of the net card */
    if ((ioctl(fd, SIOCGIFADDR, (char*)&req)))
        goto Failed;

    sin = (struct sockaddr_in *)&req.ifr_addr;
    str = inet_ntoa(sin->sin_addr);
    strncpy(ip(), str, ipSize());

    /* Get NetMask */
    if (!ioctl(fd, SIOCGIFNETMASK, &req)) {
        sin = (struct sockaddr_in *)&req.ifr_netmask;
        str = inet_ntoa(sin->sin_addr);
        strncpy(netmask(), str, netmaskSize());
        close(fd);
        return true;
    }

Failed:
    close(fd);
    return false;
}

char* NetSettings::getAvailInterface()
{
    return const_cast<char*>("eth0");
}

bool NetSettings::setStaticIP(const char* ifname)
{
    int fd;
    struct ifreq  req;              
    struct sockaddr_in* sin;

    setuid(0);
    seteuid(0);

    if ((fd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        return false;

    memset(&req, 0x0 ,sizeof(req));
    strcpy(req.ifr_name, ifname);
    sin = (struct sockaddr_in*)&req.ifr_addr;
    sin->sin_family = PF_INET;
    if (!inet_aton(ip(), &sin->sin_addr))
        goto Failed;
    
    /* Set IP of the net card */
    if ((ioctl(fd, SIOCSIFADDR, (char*)&req)))
        goto Failed;

    sin->sin_family = PF_INET;
    if (!inet_aton(netmask(), &sin->sin_addr))
        goto Failed;

    if (ioctl(fd, SIOCSIFNETMASK, &req))
        goto Failed;

    /* Enable interface */
    req.ifr_flags = IFF_UP;
    if ((ioctl(fd, SIOCSIFFLAGS, (char*)&req)))
        goto Failed;

    close(fd);
    return true;

Failed:
    close(fd);
    return false;
}

bool NetSettings::getGateway()
{
	unsigned long d, g, m;
	char devname[64], flags[16], *sdest, *sgw;
	int flgs, ref, use, metric, mtu, win, ir;
	struct sockaddr_in s_addr;
	struct in_addr mask;

    int noresolve = 1;

	FILE *fp = fopen(ROUTE_FILE, "r");
	if (fscanf(fp, "%*[^\n]\n") < 0) {
        /* Skip the first line. Empty or missing line, or read error. */
        goto ERROR;
	}

    while (1) {
		int r;
        r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n", devname, &d, &g, &flgs,
                &ref, &use, &metric, &m, &mtu, &win, &ir);
        if (r != 11) {
            if ((r < 0) && feof(fp)) {
                /* EOF with no (nonspace) chars read. */
                break;
            }
 ERROR:
			perror("fscanf");
		}

		if (!(flgs & RTF_UP)) {
            /* Skip interfaces that are down. */
			continue;
		}

		set_flags(flags, (flgs & IPV4_MASK));
#ifdef RTF_REJECT
		if (flgs & RTF_REJECT)
			flags[0] = '!';
#endif

		memset(&s_addr, 0, sizeof(struct sockaddr_in));
		s_addr.sin_family = AF_INET;
		s_addr.sin_addr.s_addr = d;
		sdest = rresolve(&s_addr, (noresolve | 0x8000), m); /* 'default' instead of '*' */
		s_addr.sin_addr.s_addr = g;
		sgw = rresolve(&s_addr, (noresolve | 0x4000), m); /* Host instead of net */
		mask.s_addr = m;
		if (!strcmp(sdest, "0.0.0.0") && strcmp(sgw, "0.0.0.0"))
            strncpy(gateway(), sgw, gatewaySize());
		free(sdest);
		free(sgw);
	}
    return true;
}

bool NetSettings::setGateway(char* interface, const char* gateway)
{
    int fd;
    struct rtentry rt;
    struct sockaddr_in  sin;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return false;
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = INADDR_ANY;
    memcpy(&rt.rt_dst, &sin, sizeof(struct sockaddr));
    memcpy(&rt.rt_genmask, &sin, sizeof(struct sockaddr));

    rt.rt_dev = interface;
    rt.rt_flags = RTF_UP;

    /* Del default gateway */
    ioctl(fd, SIOCDELRT, &rt);

    /* Add route entry: route add default gw 192.168.1.3 */
    inet_aton(gateway, &sin.sin_addr);
    memcpy(&rt.rt_gateway, &sin, sizeof(struct sockaddr));

    rt.rt_flags = static_cast<typeof(rt.rt_flags)>(RTF_UP | RTF_GATEWAY | RTF_DEFAULT);
    if (ioctl(fd, SIOCADDRT, &rt) < 0) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool NetSettings::ifconfigUp(const char* ifname, bool enable)
{
    int fd;              
    struct ifreq  req;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return false;

    strcpy(req.ifr_name, ifname);
    if (enable)
        req.ifr_flags |= IFF_UP;
    else
        req.ifr_flags &= ~IFF_UP;

    if ((ioctl(fd, SIOCSIFFLAGS, (char*)&req)))
        goto Failed;

    close(fd);
    return true;

Failed:
    close(fd);
    return false;
}

bool NetSettings::isIfUp(const char* ifname)
{
    int fd;
    struct ifreq  req;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return false;

    memset(&req, 0x0, sizeof(req));
    strcpy(req.ifr_name, ifname);
    if ((ioctl(fd, SIOCGIFFLAGS, (char*)&req)))
        goto Failed;

    close(fd);
    return (req.ifr_flags && IFF_UP);

Failed:
    close(fd);
    return false;
}

bool NetSettings::isDHCPClientRunning()
{
    int fds[2];
    bool ret = false;

    if (pipe(fds))
        return false;

    if (fork() == 0) {
        close(1);
        /* redirect pdfs[1] to STDOUT */
        dup2(fds[1], 1);
       
        /* Close unused read end */
        close(fds[0]);
       
        system("ps ax|grep "DHCPCD);
        _exit(EXIT_SUCCESS);
    } else {
        char buf[1024];
        memset(buf, 0x0, sizeof(buf));
        close(0);
        /* redirect pdfs[0] to STDIN */
        //dup2(fds[0], 0);

        /* Close unused write end */
        close(fds[1]);
        read(fds[0], buf, 1024);
        ret = is_udhcpc_running(buf, "grep "DHCPCD);

        /* Wait for child */
        wait(NULL);
    }
    return ret;
}

bool NetSettings::waitIfrStatusChange(const char* ifrname, bool down, size_t timeout)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    gettimeofday(&end, NULL);

    while (static_cast<size_t>(end.tv_sec - start.tv_sec) < timeout) {
        gettimeofday(&end, NULL);
        if (isIfUp(ifrname) != down)
            return true;
    }
    return false;
}

bool NetSettings::enableDHCPClient(bool enable)
{
    system("killall "DHCPCD);

    char* ifrname = ifrName();
    if (waitIfrStatusChange(ifrname, true, 5))
        fprintf(stderr, "wait %s to down failed!\n", ifrname);

    if (!enable)
        return true;

    if (system(DHCPCD " &") < 0) {
        fprintf(stderr, "execl failed!\n");
        return false;
    }

    if (waitIfrStatusChange(ifrname, true, 5))
        fprintf(stderr, "wait %s to down failed!\n", ifrname);

    if (waitIfrStatusChange(ifrname, false, 5)) {
        getStaticIP(ifrname);
        
        if (strcmp(ip(), "0.0.0.0")) {
            getGateway();
            readDNSConf(RESOLVE_FILE);
            printf("ip=%s, gate=%s, dnsserver=%s\n", ip(), gateway(), dnsServer());
            return true;
        }
    }
    return false;
}

bool NetSettings::readDNSConf(const char* file)
{
    FILE* fp = NULL;
    char* buf = NULL;
    char* substr = NULL;
    struct stat st;

    setuid(0);

    if (stat(file, &st))
        return false;
    
    if (!(buf = (char*)calloc(st.st_size + 1, 1)))
        return false;

    if (!(fp = fopen(file, "r"))) {
        free(buf);
        return false;
    }

    fread(buf, st.st_size, 1, fp);
    substr = strcasestr(buf, "search ");
    if (!substr || !read_line_value(substr+7, m_dnsDomain, 63))
        memset(m_dnsDomain, 0, 64);

    substr = strcasestr(buf, "nameserver ");
    if (!substr || !read_line_value(substr+11, m_dnsServer, 63))
        memset(m_dnsServer, 0, 64);

    free(buf);
    fclose(fp);
    return true;
}

bool NetSettings::writeDNSConf(const char* file)
{
    FILE* fp = NULL;

    setuid(0);

    if (!(fp = fopen(file, "w+")))
        return false;

    fwrite("search ", 7, 1, fp);
    fwrite(dnsDomain(), strlen(dnsDomain()), 1, fp);
    fwrite("\n", 1, 1, fp);
    fwrite("nameserver ", 11, 1, fp);
    fwrite(dnsServer(), strlen(dnsServer()), 1, fp);
    fwrite("\n", 1, 1, fp);

    fclose(fp);
    return true;
}

/*  0 - sussessful. 
 * -1 - ping IP failed.
 * -2 - ping Gateway failed.
 * -3 - ping DNS Server failed.
 */
int NetSettings::test()
{
    setConnected(false);

    if (do_ping(ip()))
        return -1;

    if (do_ping(gateway()))
        return -2;

    if (do_ping(dnsServer()))
        return -3;

    setConnected(true);
    return 0;
}

 /* 0 - successful
 * -1 - invalid parameter 
 * -2 - start DHCPC failed
 * -3 - set IP failed
 * -4 - set DNS failed
 * -5 - set Route failed
 */
int NetSettings::apply()
{
    setSettled(false);

    char *ifname = ifrName();

    enableDHCPClient(false);

    /* If the the accessing type is DHCP then restart DHCPC
     * else set IP/Netmask. */
    if (isStaticIP()) {
        if (!setStaticIP(ifname))
            return -3;
        if (setGateway(ifname, gateway()))
            return -5;
        /* Write DNS Server and Domain to /etc/resolve.conf */
        if (!writeDNSConf(RESOLVE_FILE))
            return -4;
    } else {
        if (!enableDHCPClient(true))
            return -2;
    }

    setSettled(true);
    return 0;
}

static JSObjectRef constructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
static void destructor(JSObjectRef object);

static JSValueRef applySettings(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
static JSValueRef testSettings(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
static JSValueRef getPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);
static bool setPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception);

static JSNativeFunction methods[] = {
    {"apply", applySettings, kJSPropertyAttributeDontDelete},
    {"test", testSettings,  kJSPropertyAttributeDontDelete},
    {0, 0, 0}
};

static JSProperty properties[] = {
    {"connected", getPropertyValue,   NULL,   kJSPropertyAttributeReadOnly| kJSPropertyAttributeDontDelete},
    {"accessType", getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"ip",         getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"netmask",    getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"gateway",    getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"dnsServer",  getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {"dnsDomain",  getPropertyValue,   setPropertyValue,   kJSPropertyAttributeDontDelete},
    {0, 0, 0, 0}
};

static JSNativeClass NetworkSettings = {
    "NetworkSettings",
    0,
    constructor,
    destructor,
    methods,
    properties,
};

static JSObjectRef constructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    NetSettings *netSettings = new NetSettings;
    JSObjectRef newObj = JSObjectMake(context, mdolphin_lookup_jsnativeclass(NetworkSettings.name), netSettings);
    return newObj;
}

static void destructor(JSObjectRef object)
{
    NetSettings *netSettings = (NetSettings*)JSObjectGetPrivate(object);
    delete netSettings;
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

/* Set the value of the property */
static bool setPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    NetSettings *netSettings = (NetSettings*)JSObjectGetPrivate(object);
	if (!netSettings)
	    return JSValueMakeUndefined(context);

    char name[MAXPROPERTYNAME];
    memset(name, 0x0, sizeof(name));
    JSStringGetUTF8CString(propertyName, name, sizeof(name)-1);
    if (!strcasecmp(name, "accessType")) {
        JSStringRef strRef = JSValueToStringCopy(context, value, exception);
        char buf[10];
        memset(buf, 0x0, sizeof(buf));
        JSStringGetUTF8CString(strRef, buf, sizeof(buf)-1);
        netSettings->enableStaticIP(strcasecmp(buf, "dhcp"));
        JSStringRelease(strRef);
    } else if (!strcasecmp(name, "ip"))
    {
        JSValueToCStr(context, value, exception, netSettings->ip(), netSettings->ipSize());
    }
    else if (!strcasecmp(name, "netmask"))
        JSValueToCStr(context, value, exception, netSettings->netmask(), netSettings->netmaskSize());
    else if (!strcasecmp(name, "gateway")) 
        JSValueToCStr(context, value, exception, netSettings->gateway(), netSettings->gatewaySize());
    else if (!strcasecmp(name, "dnsserver"))
        JSValueToCStr(context, value, exception, netSettings->dnsServer(), netSettings->dnsServerSize());
    else if (!strcasecmp(name, "dnsdomain"))
        JSValueToCStr(context, value, exception, netSettings->dnsDomain(), netSettings->dnsDomainSize());
    return JSValueMakeUndefined(context);
}

/* Get the value of the property */
static JSValueRef getPropertyValue(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    NetSettings *netSettings = (NetSettings*)JSObjectGetPrivate(object);
	if (!netSettings)
	    return JSValueMakeUndefined(context);

    char name[MAXPROPERTYNAME];
    memset(name, 0x0, sizeof(name));
    JSStringGetUTF8CString(propertyName, name, sizeof(name)-1);
    if (!strcasecmp(name, "accessType")) {
        JSStringRef strRef;
        if (netSettings->isStaticIP())
            strRef = JSStringCreateWithUTF8CString("Static");
        else
            strRef = JSStringCreateWithUTF8CString("DHCP");
        JSValueRef valRef = JSValueMakeString(context, strRef); 
        JSStringRelease(strRef);
        return valRef;
    } else if (!strcasecmp(name, "ip"))
    {
        return CStrToJSValue(context, exception, netSettings->ip(), netSettings->ipSize());
    }

    else if (!strcasecmp(name, "netmask"))
        return CStrToJSValue(context, exception, netSettings->netmask(), netSettings->netmaskSize());
    else if (!strcasecmp(name, "gateway"))
        return CStrToJSValue(context, exception, netSettings->gateway(), netSettings->gatewaySize());
    else if (!strcasecmp(name, "dnsserver"))
        return CStrToJSValue(context, exception, netSettings->dnsServer(), netSettings->dnsServerSize());
    else if (!strcasecmp(name, "dnsdomain"))
        return CStrToJSValue(context, exception, netSettings->dnsDomain(), netSettings->dnsDomainSize());
    else if (!strcasecmp(name, "connected")) {
        JSStringRef strRef;
        if (netSettings->isConnected())
            strRef = JSStringCreateWithUTF8CString("ok");
        else
            strRef = JSStringCreateWithUTF8CString("failed");
        JSValueRef valRef = JSValueMakeString(context, strRef); 
        JSStringRelease(strRef);
        return valRef;
    }
    return JSValueMakeUndefined(context);
}

static JSValueRef applySettings(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	NetSettings *netSettings = (NetSettings*)JSObjectGetPrivate(thisObject);
    if (!netSettings)
        return JSValueMakeNumber(context, -1);
    return JSValueMakeNumber(context, netSettings->apply());
}

static JSValueRef testSettings(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	NetSettings *netSettings = (NetSettings*)JSObjectGetPrivate(thisObject);
    if (!netSettings)
        return JSValueMakeNumber(context, -1);
    return JSValueMakeNumber(context, netSettings->test());
}

void register_networkobject(void)
{
    mdolphin_define_jsnativeclass(&NetworkSettings);
}

static int do_ping(const char* addr)
{
	pid_t pid;
    int stat;

    if ((pid = vfork()) < 0) {
		printf("vfork falied!\n");
		return -1;
	} else if (pid == 0) {
		if (execlp("ping", "ping", "-c", "3", addr, (char*)0) < 0) {
			printf("execlp failed!\n");
			_exit(-1);
		}
		printf("execlp ok\n");
		_exit(0);
	}

    waitpid(pid, &stat, 0);
    if (!stat)
        return 0;

    return -1;
}

static int read_line_value(const char* src, char* dst, int max)
{
    int len = 0;
    if (!src || !dst)
        return len;

    while (*src!= '\0' && *src!= '\r' && *src!= '\n') {
        if (len>=max)
            break;
        *dst++ = *src++;
        len++;
    }

    return len;
}

/* Must agree with flagvals[]. */
static const char flagchars[] =
	"GHRDM"
#if ENABLE_FEATURE_IPV6
	"DAC"
#endif
;

static void set_flags(char *flagstr, int flags)
{
	int i;

	*flagstr++ = 'U';

	for (i = 0; (*flagstr = flagchars[i]) != 0; i++) {
		if (flags & flagvals[i]) {
			++flagstr;
		}
	}
}

/* numeric: & 0x8000: default instead of *,
 *          & 0x4000: host instead of net,
 *          & 0x0fff: don't resolve
 */
static char* rresolve(struct sockaddr_in *s_in, int numeric, uint32_t netmask)
{
	/* addr-to-name cache */
	struct addr {
		struct addr *next;
		struct sockaddr_in addr;
		int host;
		char name[1];
	};
	static struct addr *cache = NULL;

	struct addr *pn;
	char *name;
	uint32_t ad, host_ad;
	int host = 0;

	/* Grmpf. -FvK */
	if (s_in->sin_family != AF_INET) {
#ifdef DEBUG
		bb_error_msg("rresolve: unsupported address family %d!",
				  s_in->sin_family);
#endif
		return NULL;
	}
	ad = s_in->sin_addr.s_addr;
#ifdef DEBUG
	bb_error_msg("rresolve: %08x, mask %08x, num %08x", (unsigned)ad, netmask, numeric);
#endif
	if (ad == INADDR_ANY) {
		if ((numeric & 0x0FFF) == 0) {
			if (numeric & 0x8000)
				return strdup(bb_str_default);
			return strdup("*");
		}
	}
	if (numeric & 0x0FFF)
		return strdup(inet_ntoa(s_in->sin_addr));

	if ((ad & (~netmask)) != 0 || (numeric & 0x4000))
		host = 1;
	pn = cache;
	while (pn) {
		if (pn->addr.sin_addr.s_addr == ad && pn->host == host) {
#ifdef DEBUG
			bb_error_msg("rresolve: found %s %08x in cache",
					  (host ? "host" : "net"), (unsigned)ad);
#endif
			return strdup(pn->name);
		}
		pn = pn->next;
	}

	host_ad = ntohl(ad);
	name = NULL;
	if (host) {
		struct hostent *ent;
#ifdef DEBUG
		bb_error_msg("gethostbyaddr (%08x)", (unsigned)ad);
#endif
		ent = gethostbyaddr((char *) &ad, 4, AF_INET);
		if (ent)
			name = strdup(ent->h_name);
	} else if (ENABLE_FEATURE_ETC_NETWORKS) {
		struct netent *np;
#ifdef DEBUG
		bb_error_msg("getnetbyaddr (%08x)", (unsigned)host_ad);
#endif
		np = getnetbyaddr(host_ad, AF_INET);
		if (np)
			name = strdup(np->n_name);
	}
	if (!name)
		name = strdup(inet_ntoa(s_in->sin_addr));
	pn = (struct addr*)malloc(sizeof(*pn) + strlen(name)); /* no '+ 1', it's already accounted for */
	pn->next = cache;
	pn->addr = *s_in;
	pn->host = host;
	strcpy(pn->name, name);
	cache = pn;
	return name;
}

static bool is_udhcpc_running(const char* buf, const char* except)
{
    char line[1024];
    int len = 0;

    if (!buf)
        return false;

    memset(line, 0, sizeof(line));
    while ((len = read_line_value(buf, line, 1024))>0) {
        if (except) {
            if (strstr(line, except))
                goto read_next;
        }
        if (strstr(line, DHCPCD))
            return true;
read_next:
        buf += len;
    }
    return false;
}
