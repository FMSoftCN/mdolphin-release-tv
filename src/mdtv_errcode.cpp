
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mdolphin/mdolphin.h>
#include <mdolphin/mdolphin_errcode.h>

static char buf[32];

#if ENABLE_NATIVEERROR

#define MD_ERROR_404_PAGE  "/error.html"

static void user_defined_404_error_page(HWND hWnd, const char* failedUrl)
{
    FILE *fp = NULL;
    char fileName[512];
    char *current_dir = get_current_dir_name();
    if (current_dir) {
        memset(fileName, 0, 512);
        snprintf(fileName, 511, "%s%s", current_dir, MD_ERROR_404_PAGE);
        fileName[511]='\0';

        fp = fopen(fileName, "w");
        if (fp) {
            char *begin = "<HTML><HEAD>\n<TITLE>404 Not Found</TITLE>\n</HEAD><BODY>\n<H1>Not Found</H1>\nThe requested URL was not found on this server.\n<P><HR><ADDRESS>";
            char *end = "</ADDRESS>\n" "</BODY></HTML>\n";
            fwrite(begin, 1, strlen(begin), fp);
            fwrite(failedUrl, 1, strlen(failedUrl), fp);
            fwrite(end, 1, strlen(end), fp);
            fclose(fp);

            memset(fileName, 0, 512);
            snprintf(fileName, 511, "file://%s%s", current_dir, MD_ERROR_404_PAGE);
            fileName[511]='\0';
            mdolphin_navigate(hWnd, NAV_GOTO, fileName, TRUE);
        }
        free(current_dir);
    }
}
#endif

static char *get_error_description(HWND hWnd, int errCode, const char* failedUrl)
{
    switch (errCode) {
        case MDEC_NET_URL_ERROR:
            return const_cast<char*>("The URL was not properly formatted.");
        case MDEC_NET_UNSUPPORTED_PROTOCOL:
            return const_cast<char*>("unsupported protocol.");
        case MDEC_NET_DNS_ERROR:
            return const_cast<char*>("couldn't resolve the host.");
        case MDEC_NET_COULDNT_CONNECT:
            return const_cast<char*>("Failed to connect() to host or proxy.");
        case MDEC_NET_UNKNOWN_ERROR:
            return const_cast<char*>("It's an unknown network error.");
#if ENABLE_NATIVEERROR
        case MDEC_NET_HTTP_404:
            user_defined_404_error_page(hWnd, failedUrl);
            return NULL;
        case MDEC_NET_HTTP_400:
        case MDEC_NET_HTTP_401:
        case MDEC_NET_HTTP_402:
        case MDEC_NET_HTTP_403:
        case MDEC_NET_HTTP_405:
        case MDEC_NET_HTTP_406:
        case MDEC_NET_HTTP_407:
        case MDEC_NET_HTTP_408:
        case MDEC_NET_HTTP_409:
        case MDEC_NET_HTTP_410:
        case MDEC_NET_HTTP_411:
        case MDEC_NET_HTTP_412:
        case MDEC_NET_HTTP_413:
        case MDEC_NET_HTTP_414:
        case MDEC_NET_HTTP_415:
        case MDEC_NET_HTTP_416:
        case MDEC_NET_HTTP_417:
        case MDEC_NET_HTTP_500:
        case MDEC_NET_HTTP_501:
        case MDEC_NET_HTTP_502:
        case MDEC_NET_HTTP_503:
        case MDEC_NET_HTTP_504:
        case MDEC_NET_HTTP_505 :
            {
                memset(buf, 0 ,32);
                snprintf(buf, 31, "HTTP ERROR %d !", errCode - MDEC_NET_HTTP_BASE);
                buf[31]='\0';
                return buf;
            }
#endif
        case MDEC_NET_FTP_421:
        case MDEC_NET_FTP_425:
        case MDEC_NET_FTP_426:
        case MDEC_NET_FTP_450:
        case MDEC_NET_FTP_451:
        case MDEC_NET_FTP_452:
        case MDEC_NET_FTP_500:
        case MDEC_NET_FTP_501:
        case MDEC_NET_FTP_502:
        case MDEC_NET_FTP_503:
        case MDEC_NET_FTP_504:
        case MDEC_NET_FTP_530:
        case MDEC_NET_FTP_532:
        case MDEC_NET_FTP_550:
        case MDEC_NET_FTP_551:
        case MDEC_NET_FTP_552:
        case MDEC_NET_FTP_553:
            {
                memset(buf, 0 ,32);
                snprintf(buf, 31, "FTP ERROR %d !", errCode - MDEC_NET_FTP_BASE);
                buf[31]='\0';
                return buf;
            }
        case MDEC_NET_FTP_UNKNOWN_ERROR:
            return const_cast<char*>("It's an unknown FTP error.");
        case MDEC_NET_FILE_READ_ERROR:
            return const_cast<char*>("It's a FILE error. A file given with FILE:// couldn't be opened.");
        case MDEC_NET_SSL_CONNECT_ERROR:
            return const_cast<char*>("It's an SSL error which occurred somewhere in the SSL/TLS handshake.");
        case MDEC_NET_SSL_PEER_CERTIFICATE:
            return const_cast<char*>("The remote server's SSL certificate was deemed not OK.");
        case MDEC_NET_SSL_ENGINE_NOTFOUND:
            return const_cast<char*>("The specified crypto engine wasn't found.");
        case MDEC_NET_SSL_ENGINE_SETFAILED:
            return const_cast<char*>("Failed setting the selected SSL crypto engine as default.");
        case MDEC_NET_SSL_CERTPROBLEM:
            return const_cast<char*>("Problem with the local client certificate.");
        case MDEC_NET_SSL_CIPHER:
            return const_cast<char*>("Couldn't use specified cipher.");
        case MDEC_NET_SSL_CACERT:
            return const_cast<char*>("Peer certificate cannot be authenticated with known CA certificates.");
        case MDEC_NET_SSL_FTP_ERROR:
            return const_cast<char*>("Requested FTP SSL level failed.");
        case MDEC_NET_SSL_ENGINE_INITFAILED:
            return const_cast<char*>("Initiating the SSL Engine failed.");
        case MDEC_NET_SSL_CACERT_BADFILE:
            return const_cast<char*>("Problem with reading the SSL CA cert (path? access rights?)");
        case MDEC_NET_PROXY_ERROR:
            return const_cast<char*>("It's a PROXY error.");
        default:
            return NULL;
    }
}

void my_error_code_callback (HWND hWnd, int errCode, const char* failedUrl)
{
    char *err = get_error_description(hWnd, errCode, failedUrl);
    if (err) {
        char msg[128];
        memset(msg, 0, 128);
        snprintf(msg, 128, "%s\n%s", failedUrl, err);
        msg[127]='\0';
        MessageBox(GetParent(hWnd), msg, "ERROR", MB_OK);
    }
}

