/*
   ----------------------------------------------------------------------------
   | mg_python: Python Extension for M/Cache/IRIS                             |
   | Author: Chris Munt cmunt@mgateway.com                                    |
   |                    chris.e.munt@gmail.com                                |
   | Copyright (c) 2016-2019 M/Gateway Developments Ltd,                      |
   | Surrey UK.                                                               |
   | All rights reserved.                                                     |
   |                                                                          |
   | http://www.mgateway.com                                                  |
   |                                                                          |
   | Licensed under the Apache License, Version 2.0 (the "License"); you may  |
   | not use this file except in compliance with the License.                 |
   | You may obtain a copy of the License at                                  |
   |                                                                          |
   | http://www.apache.org/licenses/LICENSE-2.0                               |
   |                                                                          |
   | Unless required by applicable law or agreed to in writing, software      |
   | distributed under the License is distributed on an "AS IS" BASIS,        |
   | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. |
   | See the License for the specific language governing permissions and      |
   | limitations under the License.                                           |      
   ----------------------------------------------------------------------------
*/

/*

Change Log:

Version 2.0.42 17 October 2009:
   First Open Source release.

Version 2.0.43 28 May 2012:
   Fix memory leak (mg_buf_free).

Version 2.1.44 12 December 2019:
   Update code base.
   IPv6 compliance.
   Open Source release to GitHub.
   Introduce support for Python v3.

*/

/*
   Building
   ========

   Use the following method:

   1) Build a shared object from first principles:

      Note: If building for GT.M include the path to the GT.M header gtmxc_types.h in the compiler options
            Example: gcc -c -fpic -DLINUX -I<path_to_python_dir> -I<path_to_python_dir>/Include -I<path_to_gtmxc_types.h> -o mg_python.o mg_python.
            Otherwise comment out the definition of pre-processor symbols MG_GTM and MG_GTM_SO below.

      Linux
      -----
      gcc -c -fpic -DLINUX -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      gcc -shared -rdynamic -o mg_python.so mg_python.o

      FreeBSD
      -------
      cc -c -DFREEBSD -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      ld -G -o mg_python.so mg_python.o

      AIX
      ---
      xlc_r -c -DAIX -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      xlc_r -G -H512 -T512 -bM:SRE mg_python.o -berok -bexpall -bnoentry -o mg_python.so

      Mac OS X
      --------
      gcc -c -fPIC -fno-common -DMACOSX -D_NOTHREADS -DDARWIN -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      gcc -bundle -flat_namespace -undefined suppress mg_python.o -o mg_python.so

      HPUX64
      ------
      cc -c -DHPUX11 -DNET_SSL -D_HPUX_SOURCE -Ae +DA2.0W +z -DMCC_HTTPD -DSPAPI20 -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      ld -b  mg_python.o -o mg_python.so

      Dec UNIX
      --------
      cc -c -DOSF1 -std0 -w -pthread -DIS_64 -ieee_with_inexact -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      ld -all -shared -expect_unresolved "*" -taso mg_python.o -o mg_python.so

      Solaris SPARC32
      ---------------
      cc -c Xa -w -DSOLARIS -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      ld -G mg_python.o -o mg_python.so

      Solaris SPARC64
      ---------------
      cc -c -Xa -w -xarch=v9 -KPIC -DBIT64PLAT -DSOLARIS -I<path_to_python_dir> -I<path_to_python_dir>/Include [-I<path_to_gtmxc_types.h>] -o mg_python.o mg_python.c
      ld -G mg_python.o -o mg_python.so
*/


#define MG_VERSION               "2.1.44"

#define MG_HOST                  "127.0.0.1"
/*
#define MG_PORT                  7040
*/
#define MG_PORT                  7041
#define MG_IPV6                  1
#define MG_UCI                   "USER"

#define MG_BUFSIZE               32768
#define MG_BUFMAX                32767

#define MG_MAX_KEY               256
#define MG_MAX_PAGE              256
#define MG_MAX_VARGS             32

#define MG_MAXCON                32

#define MG_T_VAR                 0
#define MG_T_STRING              1
#define MG_T_INTEGER             2
#define MG_T_FLOAT               3
#define MG_T_LIST                4

#define MG_TX_DATA               0
#define MG_TX_AKEY               1
#define MG_TX_AREC               2
#define MG_TX_EOD                3
#define MG_TX_AREC_FORMATTED     9

#define MG_ES_DELIM              0
#define MG_ES_BLOCK              1

#define MG_RECV_HEAD             8

#define MG_CHUNK_SIZE_BASE       62


#if defined(_WIN32)

#ifndef MG_WIN32
#define MG_WIN32                 1
#endif
#define MG_WINSOCK2              1
#if defined(_MSC_VER)
#if (_MSC_VER >= 1400)
#define _CRT_SECURE_NO_DEPRECATE    1
#define _CRT_NONSTDC_NO_DEPRECATE   1
#define MG_IPV6                  1
#endif
#endif
#define MG_LOG_FILE              "c:/mg/bin/mg_python.log"

#elif defined(__linux__) || defined(__linux) || defined(linux)

#if !defined(LINUX)
#define LINUX                       1
#endif
#define MG_LOG_FILE              "/tmp/mg_python.log"

#elif defined(__APPLE__)

#if !defined(MACOSX)
#define MACOSX                      1
#endif
#define MG_LOG_FILE              "/tmp/mg_python.log"

#endif

/* Comment-out if GT.M not installed */
/*
#define MG_GTM                   1
#define MG_GTM_SO                1
*/

/* include standard header */

#include <Python.h>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 1
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/resource.h>
#if !defined(HPUX) && !defined(HPUX10) && !defined(HPUX11)
#include <sys/select.h>
#endif
#if defined(SOLARIS)
#include <sys/filio.h>
#endif
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <dlfcn.h>

#endif

#ifdef MG_GTM
#include "gtmxc_types.h"
#endif

#if PY_MAJOR_VERSION >= 3
/*
#define MG_GET_PYSTRINGSIZE(a) PyBytes_Size(a)
#define MG_GET_PYSTRING(a) PyBytes_AsString(a)
#define MG_MAKE_PYSTRING(a)  PyBytes_FromString(a)
#define MG_MAKE_PYSTRINGN(a, b)  PyBytes_FromStringAndSize(a, b)
*/
#define MG_GET_PYSTRINGSIZE(a) PyUnicode_GET_LENGTH(a)
#define MG_GET_PYSTRING(a) PyUnicode_AsUTF8(a)
#define MG_MAKE_PYSTRING(a)  PyUnicode_FromString((char *) a)
#define MG_MAKE_PYSTRINGN(a, b) mg_make_pystringn((char *) a, b)

#else

#define MG_GET_PYSTRINGSIZE(a) PyString_Size(a)
#define MG_GET_PYSTRING(a) PyString_AsString(a)
#define MG_MAKE_PYSTRING(a)  PyString_FromString((char *) a)
#define MG_MAKE_PYSTRINGN(a, b) mg_make_pystringn((char *) a, b)

#endif

#define MG_ERROR(e) \
   PyErr_SetString(PyExc_RuntimeError, (char *) e); \


#define MG_WARN(e) \
   PyErr_WarnEx(PyExc_RuntimeWarning, (char *) e, 0); \

#define MG_MEMCHECK(e, c) \
   if (p_page && p_page->mem_error == 1) { \
      mg_db_disconnect(p_page, chndle, c); \
      PyErr_SetString(PyExc_RuntimeError, (char *) c); \
      return NULL; \
   } \

#define MG_FTRACE(e) \

#if defined(_WIN32)

typedef LPSOCKADDR      xLPSOCKADDR;
#ifdef _WIN64
typedef int             socklen_netx;
#else
typedef size_t          socklen_netx;
#endif
#define  NETX_FD_ISSET(fd, set)  WSAFDIsSet((SOCKET)(fd), (fd_set *)(set))
#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif
#define SOCK_ERROR(n)   (n == SOCKET_ERROR)
#define INVALID_SOCK(n) (n == INVALID_SOCKET)
#define NOT_BLOCKING(n) (n != WSAEWOULDBLOCK)
#define BZERO(b, len)   (bzero(b, len))

#else

typedef const void *    xLPSOCKADDR;
#if defined(OSF1) || defined(HPUX) || defined(HPUX10) || defined(HPUX11)
typedef int             socklen_netx;
#elif defined(LINUX) || defined(AIX) || defined(AIX5) || defined(MACOSX)
typedef socklen_t       socklen_netx;
#else
typedef size_t          socklen_netx;
#endif
#define NETX_FD_ISSET(fd, set) FD_ISSET(fd, set)
#define SOCK_ERROR(n)   (n < 0)
#define INVALID_SOCK(n) (n < 0)
#define NOT_BLOCKING(n) (n != EWOULDBLOCK && n != 2)
#define BZERO(b,len) (memset((b), '\0', (len)), (void) 0)

#endif

typedef struct tagMGPTYPE {
   short type;
   short byref;
} MGPTYPE, *LPMGPTYPE;


typedef struct tagMGCONX {
#if defined(_WIN32)
   WSADATA        wsadata;
   SOCKET         sockfd;
#else
   int            sockfd;
#endif
   char           ip_address[64];
   int            port;
   short          eod;
   short          keep_alive;
   short          in_use;

   int            base_port;
   int            child_port;

   char           mpid[128];
   char           base_uci[128];
   char           uci[128];
   char           dbtype[256];
   int            version;

} MGCONX, *LPMGCONX;

static int le_mg_user;


typedef struct tagMGBUF {
   unsigned long     size;
   unsigned long     data_size;
   unsigned long     increment_size;
   unsigned char *   p_buffer;
} MGBUF, *LPMGBUF;


typedef struct tagMGSTR {
   unsigned int      size;
   unsigned char *   ps;
} MGSTR, *LPMGSTR;


typedef struct tagMGVARGS {
   int phndle;
   char *global;
   PyObject *py_nkey[MG_MAX_VARGS];
   PyObject *pvars[MG_MAX_VARGS];
   MGSTR cvars[MG_MAX_VARGS];
} MGVARGS, *LPMGVARGS;


typedef struct tagMGUSER {
   char buffer[MG_BUFSIZE];
} MGUSER, *LPMGUSER;


typedef struct tagMGPAGE {
   short       mem_error;
   short       storage_mode;
   short       mode;
   int         error_mode;
   int         header_len;
   int         error_no;
   char        error_code[128];
   char        error_mess[256];
   char        info[256];
   char        server[64];
   char        uci[128];
   char        base_uci[128];
   char        ip_address[64];
   int         port;
   int         timeout;
   int         no_retry;
   int         nagle_algorithm;
   char        username[64];
   char        password[256];

   char        gtm_dist[128];
   char        gtmci[128];
   char        gtmroutines[128];
   char        gtmgbldir[128];

   LPMGCONX    pcon[MG_MAXCON];
} MGPAGE, *LPMGPAGE;

#if defined(_WIN32)
typedef HINSTANCE       MGHLIB;
typedef FARPROC         MGPROC;
#else
typedef void            * MGHLIB;
typedef void            * MGPROC;
#endif

typedef struct tagMGSO {
   short       flags;
   MGHLIB   h_library;
} MGSO, * LPMGSO;


static MGPAGE gpage;
static MGPAGE *tp_page[MG_MAX_PAGE] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#ifdef _WIN32
static WORD VersionRequested;
#endif

#ifndef _WIN32
pthread_mutex_t mg_lock       = PTHREAD_MUTEX_INITIALIZER;
#endif

static long request_no           = 0;

static char minit[256]           = {'\0'};
static char *mg_empty_string  = "";


/* GT.M call-in interface */

#ifdef MG_GTM
#ifdef MG_GTM_SO
#define mg_gtm_ci             mg_gtm.p_gtm_ci
#define mg_gtm_init           mg_gtm.p_gtm_init
#define mg_gtm_exit           mg_gtm.p_gtm_exit
#define mg_gtm_zstatus        mg_gtm.p_gtm_zstatus
#else
#define mg_gtm_ci             gtm_ci
#define mg_gtm_init           gtm_init
#define mg_gtm_exit           gtm_exit
#define mg_gtm_zstatus        gtm_zstatus
#endif

typedef xc_status_t     * (* LPFN_GTM_CI)       (const char *c_rtn_name, ...);
typedef xc_status_t     * (* LPFN_GTM_INIT)     (void);
typedef xc_status_t     * (* LPFN_GTM_EXIT)     (void);
typedef void            * (* LPFN_GTM_ZSTATUS)  (char* msg, int len);

typedef struct tagMGGTM {
   short                gtm;
   short                load_attempted;
   MGSO                 gtmshr;
   LPFN_GTM_CI          p_gtm_ci;
   LPFN_GTM_INIT        p_gtm_init;
   LPFN_GTM_EXIT        p_gtm_exit;
   LPFN_GTM_ZSTATUS     p_gtm_zstatus;
} MGGTM, * LPMGGTM;

MGGTM                mg_gtm;

#endif /* #ifdef MG_GTM */


PyObject *              mg_make_pystringn          (char *str, int strlen);
int                     mg_type                    (PyObject *item);
int                     mg_get_integer             (PyObject *item);
double                  mg_get_float               (PyObject *item);
char *                  mg_get_string              (PyObject *item, PyObject **item_tmp, int *size);
int                     mg_get_keys                (PyObject *keys, MGSTR *ckeys, PyObject **keys_tmp, char *record);
int                     mg_get_vargs               (PyObject *args, MGVARGS *pvargs);
int                     mg_set_list_item           (PyObject * list, int index, PyObject * item);
int                     mg_kill_list               (PyObject * list);
int                     mg_kill_list_item          (PyObject * list, int index);

int                     mg_set_error               (char * fun, char * error, int context);
int                     mg_get_error               (MGPAGE *p_page, char *buffer);

int                     mg_extract_substrings      (MGSTR * records, char* buffer, int tsize, char delim, int offset, int no_tail, short type);
int                     mg_compare_keys            (MGSTR * key, MGSTR * rkey, int max);
int                     mg_replace_substrings      (char * tbuffer, char *fbuffer, char * replace, char * with);

MGPAGE *                mg_ppage                   (int phndle);
int                     mg_ppage_init              (MGPAGE * p_page);

int                     mg_db_connect              (MGPAGE *p_page, int *chndle, short context);
int                     mg_db_connect_ex           (MGPAGE *p_page, MGCONX *lp_connection, xLPSOCKADDR p_srv_addr, socklen_netx srv_addr_len, int timeout);
int                     mg_db_disconnect           (MGPAGE *p_page, int chndle, short context);
int                     mg_db_send                 (MGPAGE *p_page, int chndle, MGBUF *p_buf, int mode);
int                     mg_db_receive              (MGPAGE *p_page, int chndle, MGBUF *p_buf, int size, int mode);
int                     mg_db_connect_init         (MGPAGE *p_page, int chndle);
int                     mg_db_ayt                  (MGPAGE *p_page, int chndle);
int                     mg_db_get_last_error       (int context);

int                     mg_request_header          (MGPAGE *p_page, MGBUF *p_buf, char *command);
int                     mg_request_add             (MGPAGE *p_page, int chndle, MGBUF *p_buf, unsigned char *element, int size, short byref, short type);

int                     mg_encode_size64           (int n10);
int                     mg_decode_size64           (int nxx);
int                     mg_encode_size             (unsigned char *esize, int size, short base);
int                     mg_decode_size             (unsigned char *esize, int len, short base);
int                     mg_encode_item_header      (unsigned char * head, int size, short byref, short type);
int                     mg_decode_item_header      (unsigned char * head, int * size, short * byref, short * type);

int                     mg_ucase                   (char *string);
int                     mg_lcase                   (char *string);

int                     mg_buf_init                (MGBUF *p_buf, int size, int increment_size);
int                     mg_buf_resize              (MGBUF *p_buf, unsigned long size);
int                     mg_buf_free                (MGBUF *p_buf);
int                     mg_buf_cpy                 (MGBUF *p_buf, char * buffer, unsigned long size);
int                     mg_buf_cat                 (MGBUF *p_buf, char * buffer, unsigned long size);

void *                  mg_malloc                  (unsigned long size);
void *                  mg_realloc                 (void *p_buffer, unsigned long size);
int                     mg_free                    (void *p_buffer);
int                     mg_log_event               (char *event, char *title);

int                     mg_load_gtm_library        (MGPAGE *p_page, short context);
int                     mg_unload_gtm_library      (MGPAGE *p_page, short context);

int                     mg_so_load                 (MGSO *p_mgso, char * library);
MGPROC                  mg_so_sym                  (MGSO *p_mgso, char * symbol);
int                     mg_so_unload               (MGSO *p_mgso);


PyObject * mg_make_pystringn(char *str, int strlen)
{
   PyObject *py;

   py = NULL;

#if PY_MAJOR_VERSION >= 3
   if (strlen < 1)
      py = PyUnicode_FromStringAndSize((char *) "", 0);
   else
      py = PyUnicode_FromStringAndSize((char *) str, strlen);

#else

   if (strlen < 1)
      py = PyString_FromStringAndSize((char *) "", 0);
   else
      py = PyString_FromStringAndSize((char *) str, strlen);

#endif

   return py;
}


static PyObject * ex_m_ext_version(PyObject *self, PyObject *args)
{
   char buffer[256];

   sprintf(buffer, "M/Gateway Developments Ltd. - mg_python: Python Gateway to M - Version %s", MG_VERSION);

   return Py_BuildValue("s", buffer);
}


static PyObject * ex_m_allocate_page_handle(PyObject *self, PyObject *args)
{
   int result, n;

   result = 0;

   for (n = 1; n < MG_MAX_PAGE; n ++) {
      if (!tp_page[n]) {
         tp_page[n] = (MGPAGE *) mg_malloc(sizeof(MGPAGE));
         if (tp_page[n]) {
            mg_ppage_init(tp_page[n]);
            result = n;
         }
         break;
      }
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_release_page_handle(PyObject *self, PyObject *args)
{
   int result, phndle;

   result = 0;

   if (!PyArg_ParseTuple(args, "i", &phndle))
      return NULL;

   if (phndle > 0 && phndle < MG_MAX_PAGE && tp_page[phndle]) {
      mg_free((void *) tp_page[phndle]);
      tp_page[phndle] = NULL;
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_set_storage_mode(PyObject *self, PyObject *args)
{
   int result, phndle, smode;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "ii", &phndle, &smode))
      return NULL;

   result = 0;
   p_page = mg_ppage(phndle);

   if (p_page) {
      p_page->storage_mode = smode;
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_set_host(PyObject *self, PyObject *args)
{
   int result, phndle, port;
   char *netname, *username, *password;
   MGPAGE *p_page;

   netname = NULL;
   username = NULL;
   password = NULL;

   if (!PyArg_ParseTuple(args, "isiss", &phndle, &netname, &port, &username, &password))
      return NULL;

   result = 0;

   p_page = mg_ppage(phndle);

   if (p_page) {
      strcpy(p_page->ip_address, netname);
      p_page->port = port;
      if (username) {
         strcpy(p_page->username, username);
      }
      if (password) {
         strcpy(p_page->password, password);
      }
      p_page->mode = 1;
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_set_uci(PyObject *self, PyObject *args)
{
   int result, phndle;
   char *uci;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "is", &phndle, &uci))
      return NULL;

   result = 0;

   p_page = mg_ppage(phndle);

   if (p_page) {
      strcpy(p_page->uci, uci);
      strcpy(p_page->base_uci, uci);
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_set_server(PyObject *self, PyObject *args)
{
   int result, phndle;
   char *server;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "is", &phndle, &server))
      return NULL;

   result = 0;

   p_page = mg_ppage(phndle);

   if (p_page) {
      strcpy(p_page->server, server);
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_bind_gtm_server(PyObject *self, PyObject *args)
{
   int result, phndle;
   char *gtm_dist, *gtmci, *gtmroutines, *gtmgbldir, *null1, *null2, *null3;
#ifdef MG_GTM
   gtm_status_t status;
   gtm_char_t msgbuf[256];
#endif
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "isssssss", &phndle, &gtm_dist, &gtmci, &gtmroutines, &gtmgbldir, &null1, &null2, &null3))
      return NULL;

   result = 0;
   p_page = mg_ppage(phndle);

#ifdef MG_GTM
   if (p_page) {
      strcpy(p_page->gtm_dist, gtm_dist);
      strcpy(p_page->gtmci, gtmci);
      strcpy(p_page->gtmroutines, gtmroutines);
      strcpy(p_page->gtmgbldir, gtmgbldir);
      result = 1;
   }

   if (*(p_page->gtm_dist))
      setenv("gtm_dist", p_page->gtm_dist, 1);

   if (*(p_page->gtmci))
      setenv("GTMCI", p_page->gtmci, 1);

   if (*(p_page->gtmroutines))
      setenv("gtmroutines", p_page->gtmroutines, 1);

   if (*(p_page->gtmgbldir))
      setenv("gtmgbldir", p_page->gtmgbldir, 1);

   result = mg_load_gtm_library(p_page, 0);
   if (result) {
      status = (gtm_status_t) mg_gtm_init();

      if (status != 0) {
         mg_gtm_zstatus(msgbuf, 256);
         strcpy(p_page->error_mess, msgbuf);
         mg_unload_gtm_library(p_page, 0);
         result = 0;
      }
      else {
         p_page->mode = 2;
         result = 1;
      }

   }

#endif

   if (!result) {
      if (!strlen(p_page->error_mess))
         strcpy(p_page->error_mess, "GT.M is not available on this computer");
      MG_ERROR(p_page->error_mess);
      return NULL;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_exit_gtm_server(PyObject *self, PyObject *args)
{
   int result, phndle;
   MGPAGE *p_page;
#ifdef MG_GTM
   gtm_status_t status;
   gtm_char_t msgbuf[256];
#endif

   if (!PyArg_ParseTuple(args, "i", &phndle))
      return NULL;

   result = 0;
   p_page = mg_ppage(phndle);

#ifdef MG_GTM
   if (p_page->mode == 2) {
      status = (gtm_status_t) mg_gtm_exit();
      if (status != 0) {
         mg_gtm_zstatus(msgbuf, 256);
         strcpy(p_page->error_mess, msgbuf);
         result = 0;
      }
      else {
         result = 1;
      }

      mg_unload_gtm_library(p_page, 0);
   }
#endif

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_get_last_error(PyObject *self, PyObject *args)
{
   int result, phndle;
   char *error;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "i", &phndle))
      return NULL;

   error = "";

   p_page = mg_ppage(phndle);

   if (p_page) {
      error = p_page->error_mess;
      result = 1;
   }

   return Py_BuildValue("s", error);
}


static PyObject * ex_m_set(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_set");

   n = mg_db_connect(p_page, &chndle, 1);

   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "S");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);

   mg_buf_free(p_buf);

   return output;
}


static PyObject * ex_ma_set(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, data_len;
   int ifc[4];
   char *global, *data;
   MGSTR nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_data;
   PyObject *p;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isOO", &phndle, &global, &key, &py_data))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_set' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_set");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "S");

   max = mg_get_keys(key, nkey, py_nkey, NULL);
   data = mg_get_string(py_data, &p, &data_len);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }
   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) data, data_len, (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);


   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_get(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_get");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "G");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_get(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   char *global;
   int ifc[4];
   MGSTR nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isO", &phndle, &global, &key)) {
      return NULL;
   }

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_get' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_get");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "G");
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);


   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;

}


static PyObject * ex_m_kill(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_kill");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "K");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_kill(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   char *global;
   int ifc[4];
   MGSTR nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isO", &phndle, &global, &key))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_kill' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_kill");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "K");

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_data(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_data");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "D");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}

static PyObject * ex_ma_data(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   char *global;
   int ifc[4];
   MGSTR  nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isO", &phndle, &global, &key))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_data' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_data");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "D");

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_order(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_order");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "O");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_order(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   char *global = NULL;
   int ifc[4];
   MGSTR nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isO", &phndle, &global, &key))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_order' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_order");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "O");

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   /* output = PyBytes_FromStringAndSize(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD); */

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_set_list_item(key, max, output);

   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_previous(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_previous");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "P");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_previous(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   char *global = NULL;
   int ifc[4];
   MGSTR nkey[MG_MAX_KEY];
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *key;
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isO", &phndle, &global, &key))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_previous' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_previous");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "P");

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_set_list_item(key, max, output);

   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_merge_to_db(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, mrec, rn, len;
   int ifc[4];
   char *global, *options, *ps;
   MGSTR nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isOOs", &phndle, &global, &key, &records, &options))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_merge_to_db' must be a list");
      return NULL;
   }
   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_merge_to_db' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_merge_to_db");

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "M");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   ifc[0] = 0;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   n = 0;
   for (rn = 0; rn < mrec; rn ++) {
      p = PyList_GetItem(records, rn);
      ps = mg_get_string(p, &temp, &len);

      if (rn == 0) {
         ifc[0] = 0;
         ifc[1] = MG_TX_AREC_FORMATTED;
      }
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) ps, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[0] = 0;
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) options, (int) strlen((char *) options), (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_merge_from_db(PyObject *self, PyObject *args)
{

   MGBUF mgbuf, *p_buf;
   int n, max, mrec, anybyref;
   int ifc[4];
   char *global, *options;
   MGSTR nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *records;
   PyObject *key;
   PyObject *p;
   int chndle, phndle;
   MGPAGE *p_page;
   PyObject *output;

   if (!PyArg_ParseTuple(args, "isOOs", &phndle, &global, &key, &records, &options))
      return NULL;

   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_merge_from_db' must be a list");
      return NULL;
   }
   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_merge_from_db' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_merge_from_db");

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "m");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   anybyref = 1;

   ifc[0] = 1;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);
   ifc[0] = 0;
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) options, (int) strlen((char *) options), (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   if (anybyref) {
      short byref, type, stop;
      int n, n1, rn, hlen, size, clen, rlen, argc, rec_len;
      unsigned char *parg, *par;

      stop = 0;
      parg = (p_buf->p_buffer + MG_RECV_HEAD);

      clen = mg_decode_size(p_buf->p_buffer, 5, MG_CHUNK_SIZE_BASE);

      rlen = 0;
      argc = 0;
      for (n = 0;; n ++) {
         hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
         if ((hlen + size + rlen) > clen) {
            stop = 1;
            break;
         }
         parg += hlen;
         rlen += hlen;

         parg += size;
         rlen += size;
         if (type == MG_TX_AREC) {
            par = parg;
            rn = 0;
            rec_len = 0;
            for (n1 = 0;; n1 ++) {
               hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
               if ((hlen + size + rlen) > clen) {
                  stop = 1;
                  break;
               }
               if (type == MG_TX_EOD) {
                  parg += (hlen + size);
                  rlen += (hlen + size);
                  break;
               }
               parg += hlen;
               rlen += hlen;
               rec_len += hlen;

               parg += size;
               rlen += size;
               rec_len += size;
               if (type == MG_TX_DATA) {

                  p = MG_MAKE_PYSTRINGN(par, rec_len);
                  mg_set_list_item(records, rn ++, p);

                  par = parg;
                  rec_len = 0;
               }
            }
         }
         if (rlen >= clen || stop)
            break;
         argc ++;
      }
      if (stop) {
         MG_ERROR("ma_merge_from_db: Bad return data");
         mg_buf_free(p_buf);
         return NULL;
      }
      output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return output;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_function(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_function");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "X");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_function(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, an, t, argn, chndle, phndle, len, anybyref, ret_size;
   int types[32];
   int byrefs[32];
   int ifc[4];
   char bstr[256], bstr1[256];
   MGSTR properties[MG_MAX_KEY];
   char *str;
   char *fun;
   char *ret;
   PyObject *a_list;
   PyObject *a;
   PyObject *pstr;
   PyObject *p;
   MGPAGE *p_page;
   char buffer[256];
   PyObject *output;

   *buffer = '\0';

   /* return MG_MAKE_PYSTRINGN(buffer, (int) strlen(buffer)); */

   if (!PyArg_ParseTuple(args, "isOi", &phndle, &fun, &a_list, &argn))
      return NULL;

   if (mg_type(a_list) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_function' must be a list");
      return NULL;
   }

   /* return MG_MAKE_PYSTRINGN(fun, (int) strlen(fun)); */

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_function");

   pstr = PyList_GetItem(a_list, 0);
   str = NULL;
   strcpy(bstr, "");
   strcpy(bstr1, "");
   if (pstr)
      str = (char *) MG_GET_PYSTRING(pstr);
   if (str) {
      strcpy(bstr, str);
   }

   /* return MG_MAKE_PYSTRINGN(bstr, (int) strlen(bstr)); */

   max = mg_extract_substrings(properties, bstr, (int) strlen(bstr), '#', 0, 0, MG_ES_DELIM);

   anybyref = 0;
   for (an = 1; an < 32; an ++) {
      if (an < max) {
         if (properties[an].ps[0] == '1')
            types[an] = 1;
         else
            types[an] = 0;
         if (properties[an].ps[1] == '1') {
            byrefs[an] = 1;
            anybyref = 1;
         }
         else
            byrefs[an] = 0;
      }
      else {
         types[an] = 0;
         byrefs[an] = 0;
      }
   }

   chndle = 0;

   max = 0;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "X");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) fun, (int) strlen((char *) fun), (short) ifc[0], (short) ifc[1]);

   for (an = 1; an <= argn; an ++) {

      str = NULL;
      pstr = PyList_GetItem(a_list, an);
      if (pstr) {

         if (byrefs[an])
            ifc[0] = 1;
         else
            ifc[0] = 0;

         t = mg_type(pstr);
         if (t == MG_T_LIST) {

            int max, n;

            ifc[1] = MG_TX_AREC;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);
            ifc[1] = MG_TX_DATA;
            mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

#if 0
{
   mg_db_disconnect(p_page, chndle, 1);
   return MG_MAKE_PYSTRINGN(p_buf->p_buffer, (int) strlen(p_buf->p_buffer));
   /* return MG_MAKE_PYSTRINGN(buffer, (int) strlen(buffer)); */
}
#endif

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   if (anybyref) {
      short byref, type, stop;
      int n, n1, rn, hlen, size, clen, rlen, argc, rec_len, argoffs;
      unsigned char *parg, *par;

      ret = (char *) (p_buf->p_buffer + MG_RECV_HEAD);
      ret_size = p_buf->data_size - MG_RECV_HEAD;

      argoffs = 2;
      stop = 0;
      parg = (p_buf->p_buffer + MG_RECV_HEAD);

      clen = mg_decode_size(p_buf->p_buffer, 5, MG_CHUNK_SIZE_BASE);

      rlen = 0;
      argc = 0;
      for (n = 0;; n ++) {
         hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
         if ((hlen + size + rlen) > clen) {
            stop = 1;
            break;
         }
         parg += hlen;
         rlen += hlen;
         an = argc - argoffs;

         if (an > 0) {
            pstr = PyList_GetItem(a_list, an);
            if (pstr)
               t = mg_type(pstr);
         }
         else
            pstr = NULL;

         if (argc == 0) {
               ret = (char *) parg;
               ret_size = size;
         }
         else if (pstr && type == MG_TX_DATA) {
            p = MG_MAKE_PYSTRINGN(parg, size);
            mg_set_list_item(a_list, an, p);
         }

         parg += size;
         rlen += size;
         if (type == MG_TX_AREC) {

            if (pstr && t == MG_T_LIST)
               mg_kill_list(pstr);

            par = parg;
            rn = 0;
            rec_len = 0;
            for (n1 = 0;; n1 ++) {
               hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
               if ((hlen + size + rlen) > clen) {
                  stop = 1;
                  break;
               }
               if (type == MG_TX_EOD) {
                  parg += (hlen + size);
                  rlen += (hlen + size);
                  break;
               }
               parg += hlen;
               rlen += hlen;
               rec_len += hlen;

               parg += size;
               rlen += size;
               rec_len += size;
               if (type == MG_TX_DATA) {

                  if (pstr && t == MG_T_LIST) {
                     p = MG_MAKE_PYSTRINGN(par, rec_len);
                     mg_set_list_item(pstr, rn ++, p);
                  }

                  par = parg;
                  rec_len = 0;
               }
            }
         }
         if (rlen >= clen || stop)
            break;
         argc ++;
      }
      if (stop) {
         MG_ERROR("ma_function: Bad return data");
         mg_buf_free(p_buf);
         return NULL;
      }
      output = MG_MAKE_PYSTRINGN(ret, ret_size);
      mg_buf_free(p_buf);
      return output;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_m_classmethod(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max;
   int ifc[4];
   int chndle;
   MGPAGE *p_page;
   MGVARGS vargs;
   PyObject *output;

   if ((max = mg_get_vargs(args, &vargs)) == -1)
      return NULL;

   p_page = mg_ppage(vargs.phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("m_classmethod");

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "x");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page, chndle, p_buf, 1);

   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}



static PyObject * ex_ma_classmethod(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, an, t, argn, chndle, phndle, anybyref, ret_size, len;
   int types[32];
   int byrefs[32];
   int ifc[4];
   char bstr[256], bstr1[256];
   MGSTR properties[MG_MAX_KEY];
   char *str;
   char *cclass;
   char *cmethod;
   char *ret;
   PyObject *a_list;
   PyObject *a;
   PyObject *pstr;
   PyObject *p;
   MGPAGE *p_page;
   PyObject *output;

   if (!PyArg_ParseTuple(args, "issOi", &phndle, &cclass, &cmethod, &a_list, &argn))
      return NULL;

   if (mg_type(a_list) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_classmethod' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_classmethod");

   pstr = PyList_GetItem(a_list, 0);
   str = NULL;
   strcpy(bstr, "");
   strcpy(bstr1, "");
   if (pstr)
      str = (char *) MG_GET_PYSTRING(pstr);
   if (str) {
      strcpy(bstr, str);
   }

   max = mg_extract_substrings(properties, bstr, (int) strlen(bstr), '#', 0, 0, MG_ES_DELIM);

   anybyref = 0;
   for (an = 1; an < 32; an ++) {
      if (an < max) {
         if (properties[an].ps[0] == '1')
            types[an] = 1;
         else
            types[an] = 0;
         if (properties[an].ps[1] == '1') {
            byrefs[an] = 1;
            anybyref = 1;
         }
         else
            byrefs[an] = 0;
      }
      else {
         types[an] = 0;
         byrefs[an] = 0;
      }
   }

   chndle = 0;

   max = 0;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "x");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) cclass, (int) strlen((char *) cclass), (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) cmethod, (int) strlen((char *) cmethod), (short) ifc[0], (short) ifc[1]);

   for (an = 1; an <= argn; an ++) {

      str = NULL;
      pstr = PyList_GetItem(a_list, an);
      if (pstr) {

         if (byrefs[an])
            ifc[0] = 1;
         else
            ifc[0] = 0;

         t = mg_type(pstr);
         if (t == MG_T_LIST) {

            int max, n;

            ifc[1] = MG_TX_AREC;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }

   if (anybyref) {
      short byref, type, stop;
      int n, n1, rn, hlen, size, clen, rlen, argc, rec_len, argoffs;
      unsigned char *parg, *par;

      ret = (char *) (p_buf->p_buffer + MG_RECV_HEAD);
      ret_size = p_buf->data_size - MG_RECV_HEAD;

      argoffs = 3;
      stop = 0;
      parg = (p_buf->p_buffer + MG_RECV_HEAD);

      clen = mg_decode_size(p_buf->p_buffer, 5, MG_CHUNK_SIZE_BASE);

      rlen = 0;
      argc = 0;
      for (n = 0;; n ++) {
         hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
         if ((hlen + size + rlen) > clen) {
            stop = 1;
            break;
         }
         parg += hlen;
         rlen += hlen;
         an = argc - argoffs;

         if (an > 0) {
            pstr = PyList_GetItem(a_list, an);
            if (pstr)
               t = mg_type(pstr);
         }
         else
            pstr = NULL;

         if (argc == 0) {
               ret = (char *) parg;
               ret_size = size;
         }
         else if (pstr && type == MG_TX_DATA) {
            p = MG_MAKE_PYSTRINGN(parg, size);
            mg_set_list_item(a_list, an, p);
         }

         parg += size;
         rlen += size;
         if (type == MG_TX_AREC) {

            if (pstr && t == MG_T_LIST)
               mg_kill_list(pstr);

            par = parg;
            rn = 0;
            rec_len = 0;
            for (n1 = 0;; n1 ++) {
               hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
               if ((hlen + size + rlen) > clen) {
                  stop = 1;
                  break;
               }
               if (type == MG_TX_EOD) {
                  parg += (hlen + size);
                  rlen += (hlen + size);
                  break;
               }
               parg += hlen;
               rlen += hlen;
               rec_len += hlen;

               parg += size;
               rlen += size;
               rec_len += size;
               if (type == MG_TX_DATA) {

                  if (pstr && t == MG_T_LIST) {
                     p = MG_MAKE_PYSTRINGN(par, rec_len);
                     mg_set_list_item(pstr, rn ++, p);
                  }

                  par = parg;
                  rec_len = 0;
               }
            }
         }
         if (rlen >= clen || stop)
            break;
         argc ++;
      }
      if (stop) {
         MG_ERROR("ma_classmethod: Bad return data");
         mg_buf_free(p_buf);
         return NULL;
      }
      output = MG_MAKE_PYSTRINGN(ret, ret_size);
      mg_buf_free(p_buf);
      return output;
   }
   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_html_ex(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, an, t, argn, chndle, phndle, len;
   int types[32];
   int byrefs[32];
   int ifc[4];
   char bstr[256], bstr1[256];
   MGSTR properties[MG_MAX_KEY];
   char *str;
   char *fun;
   PyObject *a_list;
   PyObject *a;
   PyObject *pstr;
   PyObject *p;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "isOi", &phndle, &fun, &a_list, &argn))
      return NULL;

   if (mg_type(a_list) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 3 to 'ma_html_ex' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_html_ex");

   pstr = PyList_GetItem(a_list, 0);
   str = NULL;
   strcpy(bstr, "");
   strcpy(bstr1, "");
   if (pstr)
      str = (char *) MG_GET_PYSTRING(pstr);
   if (str) {
      strcpy(bstr, str);
   }

   max = mg_extract_substrings(properties, bstr, (int) strlen(bstr), '#', 0, 0, MG_ES_DELIM);

   for (an = 1; an < 32; an ++) {
      if (an < max) {
         if (properties[an].ps[0] == '1')
            types[an] = 1;
         else
            types[an] = 0;
         if (properties[an].ps[1] == '1')
            byrefs[an] = 1;
         else
            byrefs[an] = 0;
      }
      else {
         types[an] = 0;
         byrefs[an] = 0;
      }
   }

   chndle = 0;

   max = 0;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "H");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) fun, (int) strlen((char *) fun), (short) ifc[0], (short) ifc[1]);

   for (an = 1; an <= argn; an ++) {

      str = NULL;
      pstr = PyList_GetItem(a_list, an);
      if (pstr) {

         if (byrefs[an])
            ifc[0] = 1;
         else
            ifc[0] = 0;

         t = mg_type(pstr);
         if (t == MG_T_LIST) {

            int max, n;

            ifc[1] = MG_TX_AREC;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_buf_free(p_buf);
   return Py_BuildValue("i", chndle);
}


static PyObject * ex_ma_html_classmethod_ex(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, an, t, argn, chndle, phndle, len;
   int types[32];
   int byrefs[32];
   int ifc[4];
   char bstr[256], bstr1[256];
   MGSTR properties[MG_MAX_KEY];
   char *str;
   char *cclass;
   char *cmethod;
   PyObject *a_list;
   PyObject *a;
   PyObject *pstr;
   PyObject *p;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "issOi", &phndle, &cclass, &cmethod, &a_list, &argn))
      return NULL;

   if (mg_type(a_list) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_html_classmethod_ex' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_html_classmethod_ex");

   pstr = PyList_GetItem(a_list, 0);
   str = NULL;
   strcpy(bstr, "");
   strcpy(bstr1, "");
   if (pstr)
      str = (char *) MG_GET_PYSTRING(pstr);
   if (str) {
      strcpy(bstr, str);
   }

   max = mg_extract_substrings(properties, bstr, (int) strlen(bstr), '#', 0, 0, MG_ES_DELIM);

   for (an = 1; an < 32; an ++) {
      if (an < max) {
         if (properties[an].ps[0] == '1')
            types[an] = 1;
         else
            types[an] = 0;
         if (properties[an].ps[1] == '1')
            byrefs[an] = 1;
         else
            byrefs[an] = 0;
      }
      else {
         types[an] = 0;
         byrefs[an] = 0;
      }
   }

   chndle = 0;

   max = 0;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "y");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) cclass, (int) strlen((char *) cclass), (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) cmethod, (int) strlen((char *) cmethod), (short) ifc[0], (short) ifc[1]);

   for (an = 1; an <= argn; an ++) {

      str = NULL;
      pstr = PyList_GetItem(a_list, an);
      if (pstr) {

         if (byrefs[an])
            ifc[0] = 1;
         else
            ifc[0] = 0;

         t = mg_type(pstr);
         if (t == MG_T_LIST) {

            int max, n;

            ifc[1] = MG_TX_AREC;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_buf_free(p_buf);
   return Py_BuildValue("i", chndle);
}


static PyObject * ex_ma_http_ex(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, chndle, phndle, len;
   int ifc[4];
   char *str;
   PyObject *a;
   PyObject *py_cgi;
   PyObject *py_content;
   PyObject *p;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "iOO", &phndle, &py_cgi, &py_content))
      return NULL;

   if (mg_type(py_cgi) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_http_ex' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_http_ex");

   chndle = 0;

   max = 0;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "h");

   ifc[0] = 0;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[1] = MG_TX_AREC_FORMATTED;

   max = (int) PyList_Size(py_cgi);

   for (n = 0; n < max; n ++) {
      a = PyList_GetItem(py_cgi, n);
      str = mg_get_string(a, &p, &len);
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   str = mg_get_string(py_content, &p, &n);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_buf_free(p_buf);
   return Py_BuildValue("i", chndle);
}


static PyObject * ex_ma_arg_set(PyObject *self, PyObject *args)
{
   int n, max, t, argn, by_ref, typ, phndle;
   char buffer[MG_BUFSIZE], error[256], props[256], bstr[256], bstr1[256];
   MGSTR properties[MG_MAX_KEY];
   char *str;
   PyObject *a_list;
   PyObject *a;
   PyObject *pstr;

   *props = '\0';

   if (!PyArg_ParseTuple(args, "iOiOi", &phndle, &a_list, &argn, &a, &by_ref))
      return NULL;


   if (mg_type(a_list) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_arg_set' must be a list");
      return NULL;
   }

   t = mg_type(a);

   if (t == MG_T_LIST) {
      typ = 1;
   }
   else {
      typ = 0;
   }

   pstr = NULL;
   if (PyList_Size(a_list) > 0)
      pstr = PyList_GetItem(a_list, 0);

   str = NULL;
   strcpy(bstr, "");
   strcpy(bstr1, "");

   if (pstr) {
      t = mg_type(pstr);
      if (t == MG_T_STRING) {
         str = (char *) MG_GET_PYSTRING(pstr);

      }
   }

   if (str) {
      strcpy(bstr, str);
   }

   strcpy(error, "");

   n = mg_extract_substrings(properties, bstr, (int) strlen(bstr), '#', 0, 0, MG_ES_DELIM);

   if (n > argn)
      max = n - 1;
   else
      max = argn;

   for (n = 1; n <= max; n ++) {
      strcat(bstr1, "#");
      if (n == argn) {
         sprintf(buffer, "%d%d", typ, by_ref);
         strcat(bstr1, buffer);
      }
      else {
         if (properties[n].ps)
            strcat(bstr1, (char *) properties[n].ps);
         else
            strcat(bstr1, "00");
      }
   }

   pstr = MG_MAKE_PYSTRING(bstr1);
   mg_set_list_item(a_list, 0, pstr);
   mg_set_list_item(a_list, argn, a);

   /* printf("cmcmcm: this will crash\n"); */

   strcpy(buffer, "");

   return Py_BuildValue("s", buffer);

}


static PyObject * ex_ma_get_stream_data(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, phndle, chndle;
   PyObject *py_chndle;
   MGPAGE *p_page;
   PyObject *output;

   if (!PyArg_ParseTuple(args, "iO", &phndle, &py_chndle))
      return NULL;

   chndle = mg_get_integer(py_chndle);

   if (chndle < 0) {
      MG_ERROR("Bad connection handle");
      return NULL;
   }

   p_page = tp_page[phndle];

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_get_stream_data");

   n = mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   if (n < 1) {
      strcpy((char *) p_buf->p_buffer, "");
      mg_db_disconnect(p_page, chndle, 0);
      output = Py_BuildValue("s", p_buf->p_buffer);
      mg_buf_free(p_buf);
      return output;
   }

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }
   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}



static PyObject * ex_ma_return_to_client(PyObject *self, PyObject *args)
{
   char record[MG_BUFSIZE];
   int phndle, len;
   PyObject *py_data;
   PyObject *p;
   char *ps;

   if (!PyArg_ParseTuple(args, "iO", &phndle, &py_data))
      return NULL;

   ps = mg_get_string(py_data, &p, &len);

   strcpy(record, "\x07");
   if (ps) {
      memcpy((void *) record, (void *) ps, len);
      len ++;
   }
   else
      len = 1;

   return MG_MAKE_PYSTRINGN(record, len);

}



static PyObject * ex_ma_local_set(PyObject *self, PyObject *args)
{
   int result, index, max, mrec, rmax, start, found, rn, phndle, n, len;
   char buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_record;
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *data;
   PyObject *p;
   PyObject *temp;
   char * ps;
   MGBUF mgbuf, *p_buf;
   PyObject *output;

   rmax = 0;

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   if (!PyArg_ParseTuple(args, "iOOOO", &phndle, &records, &py_index, &key, &data)) {
      mg_buf_free(p_buf);
      return NULL;
   }

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_set' must be a list");
      mg_buf_free(p_buf);
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_set' must be a list");
      mg_buf_free(p_buf);
      return NULL;
   }

   mrec = (int) PyList_Size(records);

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ps = mg_get_string(data, &p, &len);

   for (n = 1; n <= max; n ++) {
      mg_request_add(NULL, -1, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, 0, MG_TX_AKEY);
   }
   mg_request_add(NULL, -1, p_buf, (unsigned char *) ps, len, 0, MG_TX_DATA);

   py_record = MG_MAKE_PYSTRINGN(p_buf->p_buffer, p_buf->data_size);

   if (index == -2) {
      PyList_Append(records,  py_record);
   }
   else if (index > -1) {
      mg_set_list_item(records, index, py_record);
   }
   else {
      found = 0;
      start = 0;

      for (rn = start; rn < mrec; rn ++) {
         p = PyList_GetItem(records, rn);
         ps = mg_get_string(p, &temp, &len);
         memcpy((void *) buffer, (void *) ps, len);
         rmax = mg_extract_substrings(rkey, buffer, len, '#', 1, 0, MG_ES_BLOCK);
         rmax --;
         if (rmax == max) {
            if (mg_compare_keys(nkey, rkey, max) == 0) {

               mg_set_list_item(records, rn, py_record);
               found = 1;
               break;
            }
         }
      }
      if (!found) {
         mg_set_list_item(records, mrec, py_record);
      }
   }
   result = rmax;

   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer, p_buf->data_size);
   mg_buf_free(p_buf);
   return output;
}


static PyObject * ex_ma_local_get(PyObject *self, PyObject *args)
{
   int index, max, mrec, rmax, start, rn, phndle, len;
   char record[MG_BUFSIZE], buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   char * ps;
   char * result;

   rmax = 0;

   result = record;
   strcpy(record, "");
   len = 0;

   if (!PyArg_ParseTuple(args, "iOOO", &phndle, &records, &py_index, &key))
      return NULL;

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_get' must be a list");
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_get' must be a list");
      return NULL;
   }

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   if (index > -1) {
      if (index < mrec) {
         p = PyList_GetItem(records, index);
         ps = mg_get_string(p, &temp, &len);
         memcpy((void *) buffer, (void *) ps, len);
         rmax = mg_extract_substrings(rkey, buffer, (int) strlen(buffer), '#', 1, 0, MG_ES_BLOCK);
         result = (char *) rkey[rmax].ps;
         len = rkey[rmax].size;
      }
   }
   else {
      start = 0;
      for (rn = start; rn < mrec; rn ++) {
         p = PyList_GetItem(records, rn);
         ps = mg_get_string(p, &temp, &len);
         memcpy((void *) buffer, (void *) ps, len);
         rmax = mg_extract_substrings(rkey, buffer, len, '#', 1, 0, MG_ES_BLOCK);
         rmax --;
         if (rmax == max) {
            if (mg_compare_keys(nkey, rkey, max) == 0) {
               result = (char *) rkey[rmax + 1].ps;
               len = rkey[rmax + 1].size;
               break;
            }
         }
      }
   }


   return MG_MAKE_PYSTRINGN(result, len);

}


static PyObject * ex_ma_local_data(PyObject *self, PyObject *args)
{
   int result, index, max, mrec, rmax, start, rn, data, subs, phndle, len;
   char buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   char * ps;

   result = 0;
   rmax = 0;

   if (!PyArg_ParseTuple(args, "iOOO", &phndle, &records, &py_index, &key))
      return NULL;

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_data' must be a list");
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_data' must be a list");
      return NULL;
   }

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   result = 0;
   if (index > -1) {
      if (index < mrec) {
         result = 1;
      }
   }
   else {
      start = 0;
      data = 0;
      subs = 0;
      for (rn = start; rn < mrec; rn ++) {
         p = PyList_GetItem(records, rn);
         ps = mg_get_string(p, &temp, &len);
         memcpy((void *) buffer, (void *) ps, len);
         rmax = mg_extract_substrings(rkey, buffer, (int) strlen(buffer), '#', 1, 0, MG_ES_BLOCK);
         rmax --;
         if (rmax >= max) {
            if (mg_compare_keys(nkey, rkey, max) == 0) {
               if (rmax == max)
                  data = 1;
               else
                  subs = 10;
               if (data > 0 && subs > 0)
               break;
            }
         }
      }
      result = data + subs;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_ma_local_kill(PyObject *self, PyObject *args)
{
   int result, index, max, mrec, rmax, start, rn, phndle, len;
   char buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   char * ps;

   rmax = 0;
   result = 0;

   if (!PyArg_ParseTuple(args, "iOOO", &phndle, &records, &py_index, &key))
      return NULL;

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_kill' must be a list");
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_kill' must be a list");
      return NULL;
   }

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   if (index > -1) {
      if (index < mrec) {
         mg_kill_list_item(records, index);
         result ++;
      }
   }
   else {
      start = 0;
      for (rn = start; rn < mrec; rn ++) {
         p = PyList_GetItem(records, rn);
         ps = mg_get_string(p, &temp, &len);
         memcpy((void *) buffer, (void *) ps, len);
         rmax = mg_extract_substrings(rkey, buffer, len, '#', 1, 0, MG_ES_BLOCK);
         rmax --;
         if (rmax >= max) {
            if (mg_compare_keys(nkey, rkey, max) == 0) {
               mg_kill_list_item(records, rn);
               result ++;
            }
         }
      }
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_ma_local_order(PyObject *self, PyObject *args)
{
   int result, index, max, mrec, rmax, start, rn, found, next, phndle, len;
   char buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   char *vkey, *vrkey, *vrkey1;
   char * ps;

   rmax = 0;
   result = -1;
   vkey = NULL;
   vrkey = NULL;
   vrkey1 = NULL;

   if (!PyArg_ParseTuple(args, "iOOO", &phndle, &records, &py_index, &key))
      return NULL;

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_order' must be a list");
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_order' must be a list");
      return NULL;
   }

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);
   vkey = (char *) nkey[max].ps;

   start = 0;

   if (index > -1) {
      start = index;
   }

   found = 0;
   next = -1;

   for (rn = start; rn < mrec; rn ++) {
      p = PyList_GetItem(records, rn);
      ps = mg_get_string(p, &temp, &len);
      memcpy((void *) buffer, (void *) ps, len);
      rmax = mg_extract_substrings(rkey, buffer, len, '#', 1, 0, MG_ES_BLOCK);
      rmax --;
      if (rmax >= max) {
         vrkey = (char *) rkey[max].ps;
         if (mg_compare_keys(nkey, rkey, max - 1) == 0) {

            if (strlen(vkey) == 0 && strlen(vrkey) > 0) {
               p = MG_MAKE_PYSTRING(vrkey);
               mg_set_list_item(key, max, p);
               result = rn;
               break;
            }
            if (found == 1 && strcmp(vrkey, vkey)) {
               p = MG_MAKE_PYSTRING(vrkey);
               mg_set_list_item(key, max, p);
               result = rn;
               break;
            }
            if (strcmp(vrkey, vkey) > 0) {
               if (next == -1) {
                  next = rn;
                  vrkey1 = vrkey;
               }
            }
            if (mg_compare_keys(nkey, rkey, max) == 0) {
               found = 1;
            }
         }
      }
   }

   if (found == 0 && next != -1) {
      result = rn;
      p = MG_MAKE_PYSTRING(vrkey1);
      mg_set_list_item(key, max, p);
   }

   if (result == -1) {
      p = MG_MAKE_PYSTRING("");
      mg_set_list_item(key, max, p);
   }

   return Py_BuildValue("i", result);
}



static PyObject * ex_ma_local_previous(PyObject *self, PyObject *args)
{
   int result, index, max, mrec, rmax, start, rn, found, next, phndle, len;
   char buffer[MG_BUFSIZE];
   MGSTR rkey[MG_MAX_KEY], nkey[MG_MAX_KEY];
   PyObject *py_nkey[MG_MAX_KEY];
   PyObject *py_index;
   PyObject *records;
   PyObject *key;
   PyObject *p;
   PyObject *temp;
   char *vkey, *vrkey, *vrkey1;
   char * ps;

   rmax = 0;
   result = -1;
   vkey = NULL;
   vrkey = NULL;
   vrkey1 = NULL;

   if (!PyArg_ParseTuple(args, "iOOO", &phndle, &records, &py_index, &key))
      return NULL;

   index = mg_get_integer(py_index);

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_previous' must be a list");
      return NULL;
   }
   if (mg_type(key) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 4 to 'ma_local_previous' must be a list");
      return NULL;
   }

   mrec = (int) PyList_Size(records);
   max = mg_get_keys(key, nkey, py_nkey, NULL);
   vkey = (char *) nkey[max].ps;

   start = 0;

   if (index == -1) {
      index = mrec - 1;
   }

   found = 0;
   next = -1;

   for (rn = index; rn >= start; rn --) {
      p = PyList_GetItem(records, rn);
      ps = mg_get_string(p, &temp, &len);
      memcpy((void *) buffer, (void *) ps, len);
      rmax = mg_extract_substrings(rkey, buffer, len, '#', 1, 0, MG_ES_BLOCK);
      rmax --;
      if (rmax >= max) {
         vrkey = (char *) rkey[max].ps;
         if (mg_compare_keys(nkey, rkey, max - 1) == 0) {
            if (strlen(vkey) == 0 && strlen(vrkey) > 0) {
               p = MG_MAKE_PYSTRING(vrkey);
               mg_set_list_item(key, max, p);
               result = rn;
               break;
            }
            if (found == 1 && strcmp(vrkey, vkey)) {
               p = MG_MAKE_PYSTRING(vrkey);
               mg_set_list_item(key, max, p);
               result = rn;
               break;
            }
            if (strcmp(vrkey, vkey) < 0) {
               if (next == -1) {
                  next = rn;
                  vrkey1 = vrkey;
               }
            }
            if (mg_compare_keys(nkey, rkey, max) == 0) {
               found = 1;
            }
         }
      }
   }

   if (found == 0 && next != -1) {
      result = rn;
      p = MG_MAKE_PYSTRING(vrkey1);
      mg_set_list_item(key, max, p);
   }

   if (result == -1) {
      p = MG_MAKE_PYSTRING("");
      mg_set_list_item(key, max, p);
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_ma_local_sort(PyObject *self, PyObject *args)
{
   MGBUF mgbuf, *p_buf;
   int n, max, chndle, phndle, anybyref, len;
   int ifc[4];
   char buffer[MG_BUFSIZE];
   char *str;
   PyObject *records;
   PyObject *a;
   PyObject *p;
   MGPAGE *p_page;
   PyObject *output;

   if (!PyArg_ParseTuple(args, "iO", &phndle, &records))
      return NULL;

   if (mg_type(records) != MG_T_LIST) {
      MG_ERROR("mg_python: Argument 2 to 'ma_local_sort' must be a list");
      return NULL;
   }

   p_page = mg_ppage(phndle);

   p_buf = &mgbuf;
   mg_buf_init(p_buf, MG_BUFSIZE, MG_BUFSIZE);

   MG_FTRACE("ma_local_sort");

   chndle = 0;
   max = 0;
   anybyref = 1;

   n = mg_db_connect(p_page, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page, p_buf, "X");

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   strcpy(buffer, "sort^%ZMGS");
   mg_request_add(p_page, chndle, p_buf, (unsigned char *) buffer, (int) strlen((char *) buffer), (short) ifc[0], (short) ifc[1]);


   ifc[0] = 1;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[1] = MG_TX_AREC_FORMATTED;

   max = (int) PyList_Size(records);
   for (n = 0; n < max; n ++) {
      a = PyList_GetItem(records, n);
      str = mg_get_string(a, &p, &len);
      mg_request_add(p_page, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   mg_db_send(p_page, chndle, p_buf, 1);
   mg_db_receive(p_page, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page, chndle, 1);

   if ((n = mg_get_error(p_page, (char *) p_buf->p_buffer))) {
      MG_ERROR(p_buf->p_buffer + MG_RECV_HEAD);
      mg_buf_free(p_buf);
      return NULL;
   }


   if (anybyref) {
      short byref, type, stop;
      int n, n1, rn, hlen, size, clen, rlen, argc, rec_len;
      unsigned char *parg, *par;

      stop = 0;
      parg = (p_buf->p_buffer + MG_RECV_HEAD);

      clen = mg_decode_size(p_buf->p_buffer, 5, MG_CHUNK_SIZE_BASE);

      rlen = 0;
      argc = 0;
      for (n = 0;; n ++) {
         hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
         if ((hlen + size + rlen) > clen) {
            stop = 1;
            break;
         }
         parg += hlen;
         rlen += hlen;

         parg += size;
         rlen += size;
         if (type == MG_TX_AREC) {
            par = parg;
            rn = 0;
            rec_len = 0;
            for (n1 = 0;; n1 ++) {
               hlen = mg_decode_item_header(parg, &size, (short *) &byref, (short *) &type);
               if ((hlen + size + rlen) > clen) {
                  stop = 1;
                  break;
               }
               if (type == MG_TX_EOD) {
                  parg += (hlen + size);
                  rlen += (hlen + size);
                  break;
               }
               parg += hlen;
               rlen += hlen;
               rec_len += hlen;

               parg += size;
               rlen += size;
               rec_len += size;
               if (type == MG_TX_DATA) {

                  p = MG_MAKE_PYSTRINGN(par, rec_len);
                  mg_set_list_item(records, rn ++, p);

                  par = parg;
                  rec_len = 0;
               }
            }
         }
         if (rlen >= clen || stop)
            break;
         argc ++;
      }
      if (stop) {
         MG_ERROR("ma_local_sort: Bad return data");
         return NULL;
      }
   }
   output = MG_MAKE_PYSTRINGN(p_buf->p_buffer + MG_RECV_HEAD, p_buf->data_size - MG_RECV_HEAD);
   mg_buf_free(p_buf);
   return output;
}



static PyMethodDef mg_python_methods[] = {
	{"m_ext_version", ex_m_ext_version, METH_VARARGS, "m_ext_version() doc string"},

	{"m_allocate_page_handle", ex_m_allocate_page_handle, METH_VARARGS, "m_allocate_page_handle() doc string"},
	{"m_release_page_handle", ex_m_release_page_handle, METH_VARARGS, "m_release_page_handle() doc string"},
	{"m_set_storage_mode", ex_m_set_storage_mode, METH_VARARGS, "m_set_storage_mode() doc string"},

	{"m_set_host", ex_m_set_host, METH_VARARGS, "m_set_host() doc string"},
	{"m_set_uci", ex_m_set_uci, METH_VARARGS, "m_set_uci() doc string"},
	{"m_set_server", ex_m_set_server, METH_VARARGS, "m_set_server() doc string"},

	{"m_bind_gtm_server", ex_m_bind_gtm_server, METH_VARARGS, "m_bind_gtm_server() doc string"},
	{"m_exit_gtm_server", ex_m_exit_gtm_server, METH_VARARGS, "m_exit_gtm_server() doc string"},

	{"m_get_last_error", ex_m_get_last_error, METH_VARARGS, "m_get_last_error() doc string"},

   {"m_set", ex_m_set, METH_VARARGS, "m_set() doc string"},
   {"ma_set", ex_ma_set, METH_VARARGS, "ma_set() doc string"},
	{"m_get", ex_m_get, METH_VARARGS, "m_get() doc string"},
	{"ma_get", ex_ma_get, METH_VARARGS, "ma_get() doc string"},
	{"m_kill", ex_m_kill, METH_VARARGS, "m_kill() doc string"},
	{"ma_kill", ex_ma_kill, METH_VARARGS, "ma_kill() doc string"},
	{"m_delete", ex_m_kill, METH_VARARGS, "m_delete() doc string"},
	{"ma_delete", ex_ma_kill, METH_VARARGS, "ma_delete() doc string"},
	{"m_data", ex_m_data, METH_VARARGS, "m_data() doc string"},
	{"ma_data", ex_ma_data, METH_VARARGS, "ma_data() doc string"},
	{"m_defined", ex_m_data, METH_VARARGS, "m_defined() doc string"},
	{"ma_defined", ex_ma_data, METH_VARARGS, "ma_defined() doc string"},
	{"m_order", ex_m_order, METH_VARARGS, "m_order() doc string"},
	{"ma_order", ex_ma_order, METH_VARARGS, "ma_order() doc string"},
	{"m_previous", ex_m_previous, METH_VARARGS, "m_previous() doc string"},
	{"ma_previous", ex_ma_previous, METH_VARARGS, "ma_previous() doc string"},

	{"ma_merge_to_db", ex_ma_merge_to_db, METH_VARARGS, "ma_merge_to_db() doc string"},
	{"ma_merge_from_db", ex_ma_merge_from_db, METH_VARARGS, "ma_merge_from_db() doc string"},

	{"m_proc", ex_m_function, METH_VARARGS, "m_proc() doc string"},
	{"ma_proc", ex_ma_function, METH_VARARGS, "ma_proc() doc string"},
	{"m_function", ex_m_function, METH_VARARGS, "m_function() doc string"},
	{"ma_function", ex_ma_function, METH_VARARGS, "ma_function() doc string"},

	{"m_classmethod", ex_m_classmethod, METH_VARARGS, "m_classmethod() doc string"},
	{"ma_classmethod", ex_ma_classmethod, METH_VARARGS, "ma_classmethod() doc string"},

	{"ma_html_ex", ex_ma_html_ex, METH_VARARGS, "ma_html_ex() doc string"},
	{"ma_html_classmethod_ex", ex_ma_html_classmethod_ex, METH_VARARGS, "ma_html_classmethod_ex() doc string"},
	{"ma_http_ex", ex_ma_http_ex, METH_VARARGS, "ma_http_ex() doc string"},
	{"ma_arg_set", ex_ma_arg_set, METH_VARARGS, "ma_arg_set() doc string"},

	{"ma_get_stream_data", ex_ma_get_stream_data, METH_VARARGS, "ma_get_stream_data() doc string"},
	{"ma_return_to_client", ex_ma_return_to_client, METH_VARARGS, "ma_return_to_client() doc string"},

	{"ma_local_set", ex_ma_local_set, METH_VARARGS, "ma_local_set() doc string"},
	{"ma_local_get", ex_ma_local_get, METH_VARARGS, "ma_local_get() doc string"},
	{"ma_local_data", ex_ma_local_data, METH_VARARGS, "ma_local_data() doc string"},
	{"ma_local_kill", ex_ma_local_kill, METH_VARARGS, "ma_local_kill() doc string"},
	{"ma_local_order", ex_ma_local_order, METH_VARARGS, "ma_local_order() doc string"},
	{"ma_local_previous", ex_ma_local_previous, METH_VARARGS, "ma_local_previous() doc string"},
	{"ma_local_sort", ex_ma_local_sort, METH_VARARGS, "ma_local_sort() doc string"},

	{NULL, NULL}
};


#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "mg_python",           /* m_name */
        "mg_python module",    /* m_doc */
        -1,                   /* m_size */
        mg_python_methods,     /* m_methods */
        NULL,                 /* m_reload */
        NULL,                 /* m_traverse */
        NULL,                 /* m_clear */
        NULL,                 /* m_free */
    };
#endif

static PyObject * moduleinit(void)
{
    PyObject *m;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule("mg_python", mg_python_methods);
/*
    m = Py_InitModule3("mg_python", mg_python_methods, module___doc__);
*/
#endif
   return m;
}


#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_mg_python(void)
{
   return moduleinit();
}
PyMODINIT_FUNC initmg_python(void)
{
   return NULL;
}
#else
PyMODINIT_FUNC initmg_python(void)
{
   moduleinit();
   return;
}
#endif


int mg_type(PyObject *item)
{
   int result;

   if (!item)
      return -1;

#if PY_MAJOR_VERSION >= 3
   if (PyUnicode_Check(item))
      result = MG_T_STRING;
   else if (PyLong_Check(item))
      result = MG_T_INTEGER;
   else if (PyFloat_Check(item))
      result = MG_T_FLOAT;
   else if (PyList_Check(item))
      result = MG_T_LIST;
   else
      result = MG_T_VAR;
#else
   if (PyString_Check(item))
      result = MG_T_STRING;
   else if (PyInt_Check(item))
      result = MG_T_INTEGER;
   else if (PyFloat_Check(item))
      result = MG_T_FLOAT;
   else if (PyList_Check(item))
      result = MG_T_LIST;
   else
      result = MG_T_VAR;
#endif

   return result;
}


int mg_get_integer(PyObject *item)
{
   int t, result;
   char *ps;

   result = 0;
   t = mg_type(item);

   if (t == MG_T_INTEGER)
#if PY_MAJOR_VERSION >= 3
      result = (int) PyLong_AsLong(item);
#else
      result = (int) PyInt_AsLong(item);
#endif
   else if (t == MG_T_FLOAT) {
      result = (int) PyFloat_AsDouble(item);
   }
   else {
      ps = (char *) MG_GET_PYSTRING(item);
      if (ps)
         result = (int) strtol(ps, NULL, 10);
      else
         result = 0;
   }

   return result;
}


double mg_get_float(PyObject *item)
{
   int t;
   double result;
   char *ps;

   result = 0;
   t = mg_type(item);

   if (t == MG_T_INTEGER)
#if PY_MAJOR_VERSION >= 3
      result = (double) PyLong_AsLong(item);
#else
      result = (double) PyInt_AsLong(item);
#endif
   else if (t == MG_T_FLOAT) {
      result = (double) PyFloat_AsDouble(item);
   }
   else {
      ps = (char *) MG_GET_PYSTRING(item);
      if (ps)
         result = (double) strtod(ps, NULL);
      else
         result = 0;
   }

   return result;
}


char * mg_get_string(PyObject *item, PyObject **item_tmp, int *size)
{
   int t, x;
   double y;
   char * result;
   char buffer[64];

   result = NULL;
   t = mg_type(item);
   *size = 0;

   if (t == MG_T_INTEGER) {
#if PY_MAJOR_VERSION >= 3
      x = (int) PyLong_AsLong(item);
#else
      x = (int) PyInt_AsLong(item);
#endif
      sprintf(buffer, "%d", x);
      *size = (int) strlen(buffer);
      *item_tmp = MG_MAKE_PYSTRING(buffer);
      result = (char *) MG_GET_PYSTRING(*item_tmp);

   }
   else if (t == MG_T_FLOAT) {
      y = (double) PyFloat_AsDouble(item);
      sprintf(buffer, "%f", y);
      *size = (int) strlen(buffer);
      *item_tmp = MG_MAKE_PYSTRING(buffer);
      result = (char *) MG_GET_PYSTRING(*item_tmp);
   }
   else {
      result = (char *) MG_GET_PYSTRING(item);
      *size = (int) MG_GET_PYSTRINGSIZE(item);
   }

   if (!result) {
      result = mg_empty_string;
      *size = 0;
   }

   return result;
}


int mg_get_keys(PyObject *keys, MGSTR * ckeys, PyObject **keys_tmp, char *record)
{
   int kmax, max, n, len;
   PyObject *p;

   kmax = (int) PyList_Size(keys);

   p = PyList_GetItem(keys, 0);
   max = mg_get_integer(p);

   for (n = 1; n <= max; n ++) {
      p = PyList_GetItem(keys, n);

      ckeys[n].ps = (unsigned char *) mg_get_string(p, &keys_tmp[n], &len);
      ckeys[n].size = len;

   }

   return max;
}


int mg_get_vargs(PyObject *args, MGVARGS *pvargs)
{
   int n, max, len;
   char fmt[64];

   len = 0;
   fmt[len ++] = 'i';
   fmt[len ++] = 's';
   fmt[len ++] = '|';
   for (n = 0; n < MG_MAX_VARGS; n ++) {
      pvargs->pvars[n] = NULL;
      pvargs->cvars[n].ps = NULL;
      pvargs->cvars[n].size = 0;
      fmt[len ++] = 'O';
   }
   fmt[len] = '\0';

   if (!PyArg_ParseTuple(args, fmt, &(pvargs->phndle), &(pvargs->global), &(pvargs->pvars[0]), &(pvargs->pvars[1]),
            &(pvargs->pvars[2]),  &(pvargs->pvars[3]),  &(pvargs->pvars[4]),  &(pvargs->pvars[5]),  &(pvargs->pvars[6]),
            &(pvargs->pvars[7]),  &(pvargs->pvars[8]),  &(pvargs->pvars[9]),  &(pvargs->pvars[10]), &(pvargs->pvars[11]),
            &(pvargs->pvars[12]), &(pvargs->pvars[13]), &(pvargs->pvars[14]), &(pvargs->pvars[15]), &(pvargs->pvars[16]),
            &(pvargs->pvars[17]), &(pvargs->pvars[18]), &(pvargs->pvars[19]), &(pvargs->pvars[20]), &(pvargs->pvars[21]),
            &(pvargs->pvars[22]), &(pvargs->pvars[23]), &(pvargs->pvars[24]), &(pvargs->pvars[25]), &(pvargs->pvars[26]),
            &(pvargs->pvars[27]), &(pvargs->pvars[28]), &(pvargs->pvars[29]), &(pvargs->pvars[30]), &(pvargs->pvars[31]))) {

      return -1;
   }

   for (n = 0; n < MG_MAX_VARGS && pvargs->pvars[n]; n ++) {
      pvargs->cvars[n].ps = (unsigned char *) mg_get_string(pvargs->pvars[n], &(pvargs->py_nkey[n]), &len);
      pvargs->cvars[n].size = len;

   }
   max = n;

   return max;
}


int mg_set_list_item(PyObject * list, int index, PyObject * item)
{
   int size, n, len;
   PyObject *p;
   PyObject *a;

   /* printf("set at %d\n", index); */

   size = (int) PyList_Size(list);

   /* printf("set at %d size %d\n", index, size); */

   if (size < (index + 1)) {
      len = (index + 1) - size;
      for (n = 0; n < len; n ++) {
         p = MG_MAKE_PYSTRING("");
         PyList_Append(list,  p);
      }
   }

   a = PyList_GetItem(list, index);
   if (mg_type(a) == MG_T_LIST) {
      /* DON'T CLOBBER ARRAY */
      Py_INCREF(a);
   }

   /* printf("set SET at %d\n", index); */

   PyList_SetItem(list, index, item);
   Py_INCREF(list); /* cmtxxx */

   return index;
}


int mg_kill_list(PyObject * list)
{
   int result, index;

  index = (int) PyList_Size(list);

   result = PyList_SetSlice(list, 0, index, NULL);

   return result;
}


int mg_kill_list_item(PyObject * list, int index)
{
   int result;

   result = PyList_SetSlice(list, index, index + 1, NULL);

   return result;
}


int mg_pack_record(PyObject * key, PyObject * data)
{
   return 1;
}


int mg_set_error(char * fun, char * error, int context)
{
   return 1;
}


int mg_get_error(MGPAGE *p_page, char *buffer)
{
   int n;

   if (!strncmp(buffer + 5, "ce", 2)) {
      for (n = MG_RECV_HEAD; buffer[n]; n ++) {
         if (buffer[n] == '%')
            buffer[n] = '^';
      }
      return 1;
   }
   else
      return 0;
}


int mg_extract_substrings(MGSTR * records, char * buffer, int tsize, char delim, int offset, int no_tail, short type)
{
   int n;
   char *p;

   if (!buffer)
      return 0;

   n = offset;
   p = buffer;

   if (type == MG_ES_DELIM) {
      records[n].ps = (unsigned char *) p;
      records[n].size = (int) strlen((char *) records[n].ps);

      for (;;) {
         p = strchr(p, delim);
         if (!p) {
         records[n].size = (int) strlen((char *) records[n].ps);
            break;
         }

         *p = '\0';
         records[n].size = (int) strlen((char *) records[n].ps);
         n ++;
         records[n].ps = (unsigned char *) (++ p);
      }
      n ++;
      records[n].ps = NULL;
      records[n].size = 0;
      if (no_tail == 1 && n > 0)
         n --;

      records[n].ps = NULL;
      records[n].size = 0;
   }
   else {
      short byref, type;
      int size, hlen, rlen, i;
      
      rlen = 0;
      for (i = 0;; i ++) {
         hlen = mg_decode_item_header((unsigned char *) p, &size, &byref, &type);

         *p = '\0';
         rlen += hlen;
         if ((rlen + size) > tsize)
            break;
         records[n].ps = (unsigned char *) (p + hlen);
         records[n].size = size;
         n ++;
         p += (hlen + size);
         rlen += size;
         if (rlen >= tsize)
            break;
      }
      records[n].ps = NULL;
      records[n].size = 0;
   }

   return (n - offset);
}


int mg_compare_keys(MGSTR * key, MGSTR * rkey, int max)
{
   int n, result;

   result = 0;
   for (n = 1; n <= max; n ++) {
      result = strcmp((char *) key[n].ps, (char *) rkey[n].ps);
      if (result)
         break;
   }

   return result;
}



int mg_replace_substrings(char * tbuffer, char *fbuffer, char * replace, char * with)
{
   int len, wlen, rlen;
   char *pf1, *pf2, *pt1;
   char temp[32000];

   rlen = (int) strlen(replace);
   wlen = (int) strlen(with);

   pt1 = tbuffer;
   pf1 = fbuffer;
   if (pt1 == pf1) {
      pf1 = temp;
      strcpy(pf1, pt1);
   }
   while ((pf2 = strstr(pf1, replace))) {
      len = (int) (pf2 - pf1);
      strncpy(pt1, pf1, len);
      pt1 += len;
      strncpy(pt1, with, wlen);
      pt1 += wlen;
      pf1 = (pf2 + rlen);
   }
   strcpy(pt1, pf1);

   return 1;
}


MGPAGE * mg_ppage(int phndle)
{
   MGPAGE *p_page;

   p_page = NULL;

   if (phndle > 0 && phndle < MG_MAX_PAGE && tp_page[phndle]) {
      return tp_page[phndle];
   }

   if (phndle == 0) {
      p_page = &gpage;
      if (!tp_page[0]) {
         mg_ppage_init(p_page);
         tp_page[0] = p_page;
      }
   }

   return p_page;
}


int mg_ppage_init(MGPAGE * p_page)
{
   int n;

   p_page->mem_error = 0;
   p_page->mode = 0;
   p_page->storage_mode = 0;
   p_page->timeout = 0;
   strcpy(p_page->server, "");
   strcpy(p_page->uci, "");
   strcpy(p_page->base_uci, "");

   strcpy(p_page->gtm_dist, "");
   strcpy(p_page->gtmci, "");
   strcpy(p_page->gtmroutines, "");
   strcpy(p_page->gtmgbldir, "");

   strcpy(p_page->ip_address, MG_HOST);
   p_page->port = MG_PORT;
   strcpy(p_page->uci, MG_UCI);

   strcpy(p_page->username, "");
   strcpy(p_page->password, "");

   for (n = 0; n < MG_MAXCON; n ++) {
      p_page->pcon[n] = NULL;
   }

   return 1;
}


int mg_db_connect(MGPAGE *p_page, int *p_chndle, short context)
{
   short physical_ip, ipv6, connected, getaddrinfo_ok;
   int n, free, errorno;
   unsigned long inetaddr;
   unsigned long spin_count;
   char ansi_ip_address[64];
   struct sockaddr_in srv_addr, cli_addr;
   struct hostent *hp;
   struct in_addr **pptr;
   LPMGCONX lp_connection;

   if (p_page->mode == 2) {
      return 1;
   }

   free = -1;
   *p_chndle = -1;
   for (n = 0; n < MG_MAXCON; n ++) {
      if (p_page->pcon[n]) {
         if (!p_page->pcon[n]->in_use) {
            *p_chndle = n;
            p_page->pcon[*p_chndle]->in_use = 1;
            p_page->pcon[*p_chndle]->eod = 0;
            break;
         }
      }
      else {
         if (free == -1)
            free = n;
      }
   }

   if (*p_chndle != -1) {
      return 1;
   }

   if (free == -1)
      return 0;

   *p_chndle = free;
   p_page->pcon[*p_chndle] = (LPMGCONX) mg_malloc(sizeof(MGCONX));
   p_page->pcon[*p_chndle]->in_use = 1;
   p_page->pcon[*p_chndle]->keep_alive = 0;

   lp_connection = p_page->pcon[*p_chndle];
   strcpy(lp_connection->ip_address, p_page->ip_address);
   lp_connection->port = p_page->port;


   lp_connection->eod = 0;
   strcpy(p_page->error_mess, "");

#ifdef _WIN32
   VersionRequested = MAKEWORD(2, 2);
   n = WSAStartup(VersionRequested, &(lp_connection->wsadata));
   if (n != 0) {
      strcpy(p_page->error_mess, "Microsoft WSAStartup Failed");
      return 0;
   }
#endif

   connected = 0;
   getaddrinfo_ok = 0;
   spin_count = 0;

   ipv6 = 1;
#if !defined(MG_IPV6)
   ipv6 = 0;
#endif

   strcpy(ansi_ip_address, (char *) p_page->ip_address);

#if defined(_WIN32)

   VersionRequested = MAKEWORD(2, 2);
   n = WSAStartup(VersionRequested, &(lp_connection->wsadata));
   if (n != 0) {
      strcpy(p_page->error_mess, "Microsoft WSAStartup Failed");
      return 0;
   }

#endif /* #if defined(_WIN32) */

#if defined(MG_IPV6)

   if (ipv6) {
      short mode;
      struct addrinfo hints, *res;
      struct addrinfo *ai;
      char port_str[32];

      res = NULL;
      sprintf(port_str, "%d", p_page->port);
      connected = 0;
      p_page->error_no = 0;

      for (mode = 0; mode < 3; mode ++) {

         if (res) {
            freeaddrinfo(res);
            res = NULL;
         }

         memset(&hints, 0, sizeof hints);
         hints.ai_family = AF_UNSPEC;     /* Use IPv4 or IPv6 */
         hints.ai_socktype = SOCK_STREAM;
         /* hints.ai_flags = AI_PASSIVE; */
         if (mode == 0)
            hints.ai_flags = AI_NUMERICHOST | AI_CANONNAME;
         else if (mode == 1)
            hints.ai_flags = AI_CANONNAME;
         else if (mode == 2) {
            /* Apparently an error can occur with AF_UNSPEC (See RJW1564) */
            /* This iteration will return IPV6 addresses if any */
            hints.ai_flags = AI_CANONNAME;
            hints.ai_family = AF_INET6;
         }
         else
            break;

         n = getaddrinfo(ansi_ip_address, port_str, &hints, &res);

         if (n != 0) {
            continue;
         }

         getaddrinfo_ok = 1;
         spin_count = 0;
         for (ai = res; ai != NULL; ai = ai->ai_next) {

            spin_count ++;

	         if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6) {
               continue;
            }

	         /* Open a socket with the correct address family for this address. */
	         lp_connection->sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

            /* bind(lp_connection->sockfd, ai->ai_addr, (int) (ai->ai_addrlen)); */
            /* connect(lp_connection->sockfd, ai->ai_addr, (int) (ai->ai_addrlen)); */

            if (p_page->nagle_algorithm == 0) {
               int flag = 1;
               int result;

               result = setsockopt(lp_connection->sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));
               if (result < 0) {
                  strcpy(p_page->error_mess, "Connection Error: Unable to disable the Nagle Algorithm");
               }

            }

            p_page->error_no = 0;
            n = mg_db_connect_ex(p_page, lp_connection, (xLPSOCKADDR) ai->ai_addr, (socklen_netx) (ai->ai_addrlen), p_page->timeout);
            if (n == -2) {
               p_page->error_no = n;
               n = -737;
               continue;
            }
            if (SOCK_ERROR(n)) {
               errorno = (int) mg_db_get_last_error(0);
               p_page->error_no = errorno;
               mg_db_disconnect(p_page, *p_chndle, 0);
               continue;
            }
            else {
               connected = 1;
               break;
            }
         }
         if (connected)
            break;
      }

      if (p_page->error_no) {
         sprintf(p_page->error_mess, "Connection Error: Cannot Connect to Host (%s:%d): Error Code: %d", (char *) p_page->ip_address, p_page->port, p_page->error_no);
      }

      if (res) {
         freeaddrinfo(res);
         res = NULL;
      }
   }
#endif

   if (ipv6) {
      if (connected) {
         return 1;
      }
      else {
         if (getaddrinfo_ok) {
            mg_db_disconnect(p_page, *p_chndle, 0);
            return 0;
         }
         else {
            errorno = (int) mg_db_get_last_error(0);
            sprintf(p_page->error_mess, "Connection Error: Cannot identify Host: Error Code: %d", errorno);
            mg_db_disconnect(p_page, *p_chndle, 0);
            return 0;
         }
      }
   }

   ipv6 = 0;
   inetaddr = inet_addr(ansi_ip_address);

   physical_ip = 0;
   if (isdigit(ansi_ip_address[0])) {
      char *p;

      if ((p = strstr(ansi_ip_address, "."))) {
         if (isdigit(*(++ p))) {
            if ((p = strstr(p, "."))) {
               if (isdigit(*(++ p))) {
                  if ((p = strstr(p, "."))) {
                     if (isdigit(*(++ p))) {
                        physical_ip = 1;
                     }
                  }
               }
            }
         }
      }
   }

   if (inetaddr == INADDR_NONE || !physical_ip) {

      hp = gethostbyname((const char *) ansi_ip_address);

      if (hp == NULL) {
         strcpy(p_page->error_mess, "Connection Error: Invalid Host");
         return 0;
      }

      pptr = (struct in_addr **) hp->h_addr_list;
      connected = 0;

      spin_count = 0;

      for (; *pptr != NULL; pptr ++) {

         spin_count ++;

         lp_connection->sockfd = socket(AF_INET, SOCK_STREAM, 0);

         if (INVALID_SOCK(lp_connection->sockfd)) {
            errorno = (int) mg_db_get_last_error(0);
            sprintf(p_page->error_mess, "Connection Error: Invalid Socket: Context=1: Error Code: %d", errorno);
            break;
         }

#if !defined(_WIN32)
         BZERO((char *) &cli_addr, sizeof(cli_addr));
         BZERO((char *) &srv_addr, sizeof(srv_addr));
#endif

         cli_addr.sin_family = AF_INET;
         srv_addr.sin_port = htons((unsigned short) p_page->port);

         cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
         cli_addr.sin_port = htons(0);

         n = bind(lp_connection->sockfd, (xLPSOCKADDR) &cli_addr, sizeof(cli_addr));

         if (SOCK_ERROR(n)) {
            errorno = (int) mg_db_get_last_error(0);
            sprintf(p_page->error_mess, "Connection Error: Cannot bind to Socket: Error Code: %d", errorno);
            break;
         }

         if (p_page->nagle_algorithm == 0) {
            int flag = 1;
            int result;

            result = setsockopt(lp_connection->sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));
            if (result < 0) {
               strcpy(p_page->error_mess, "Connection Error: Unable to disable the Nagle Algorithm");
            }
         }

         srv_addr.sin_family = AF_INET;
         srv_addr.sin_port = htons((unsigned short) p_page->port);

         memcpy(&srv_addr.sin_addr, *pptr, sizeof(struct in_addr));

         n = mg_db_connect_ex(p_page, lp_connection, (xLPSOCKADDR) &srv_addr, sizeof(srv_addr), p_page->timeout);

         if (n == -2) {
            continue;
         }

         if (SOCK_ERROR(n)) {
            errorno = (int) mg_db_get_last_error(0);
            sprintf(p_page->error_mess, "Connection Error: Cannot Connect to Host (%s:%d): Error Code: %d", (char *) p_page->ip_address, p_page->port, errorno);
            mg_db_disconnect(p_page, *p_chndle, 0);
            continue;
         }
         else {
            connected = 1;
            break;
         }
      }
      if (!connected) {
         mg_db_disconnect(p_page, *p_chndle, 0);
         strcpy(p_page->error_mess, "Connection Error: Failed to find the Host via a DNS Lookup");
         return 0;
      }
   }
   else {

      lp_connection->sockfd = socket(AF_INET, SOCK_STREAM, 0);

      if (INVALID_SOCK(lp_connection->sockfd)) {
         errorno = (int) mg_db_get_last_error(0);
         sprintf(p_page->error_mess, "Connection Error: Invalid Socket: Context=2: Error Code: %d", errorno);
         return 0;
      }

#if !defined(_WIN32)
      BZERO((char *) &cli_addr, sizeof(cli_addr));
      BZERO((char *) &srv_addr, sizeof(srv_addr));
#endif

      cli_addr.sin_family = AF_INET;
      cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      cli_addr.sin_port = htons(0);

      n = bind(lp_connection->sockfd, (xLPSOCKADDR) &cli_addr, sizeof(cli_addr));

      if (SOCK_ERROR(n)) {
         errorno = (int) mg_db_get_last_error(0);
         sprintf(p_page->error_mess, "Connection Error: Cannot bind to Socket: Error Code: %d", errorno);
         mg_db_disconnect(p_page, *p_chndle, 0);
         return 0;
      }

      if (p_page->nagle_algorithm == 0) {
         int flag = 1;
         int result;

         result = setsockopt(lp_connection->sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(int));
         if (result < 0) {
            strcpy(p_page->error_mess, "Connection Error: Unable to disable the Nagle Algorithm");
         }
      }

      srv_addr.sin_port = htons((unsigned short) p_page->port);
      srv_addr.sin_family = AF_INET;
      srv_addr.sin_addr.s_addr = inet_addr(ansi_ip_address);

      n = mg_db_connect_ex(p_page, lp_connection, (xLPSOCKADDR) &srv_addr, sizeof(srv_addr), p_page->timeout);
      if (n == -2) {
         mg_db_disconnect(p_page, *p_chndle, 0);
         return 0;
      }

      if (SOCK_ERROR(n)) {
         errorno = (int) mg_db_get_last_error(0);
         p_page->error_no = errorno;
         sprintf(p_page->error_mess, "Connection Error: Cannot Connect to Host (%s:%d): Error Code: %d", (char *) p_page->ip_address, p_page->port, errorno);
         mg_db_disconnect(p_page, *p_chndle, 0);
         return 0;
      }
   }

   return 1;
}


int mg_db_connect_ex(MGPAGE *p_page, MGCONX *lp_connection, xLPSOCKADDR p_srv_addr, socklen_netx srv_addr_len, int timeout)
{
#if defined(_WIN32)
   int n;
#else
   int flags, n, error;
   socklen_netx len;
   fd_set rset, wset;
   struct timeval tval;
#endif

#if defined(SOLARIS) && BIT64PLAT
   timeout = 0;
#endif

#if defined(SOLARIS)
   timeout = 0;
#endif

   if (timeout != 0) {

#if defined(_WIN32)

      n = connect(lp_connection->sockfd, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);
      return n;

#else
      flags = fcntl(lp_connection->sockfd, F_GETFL, 0);
      n = fcntl(lp_connection->sockfd, F_SETFL, flags | O_NONBLOCK);

      error = 0;

      n = connect(lp_connection->sockfd, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);

      if (n < 0) {

         if (errno != EINPROGRESS) {

#if defined(SOLARIS)

            if (errno != 2 && errno != 146) {
               sprintf((char *) p_page->error_mess, "Diagnostic: Solaris: Initial Connection Error errno=%d; EINPROGRESS=%d", errno, EINPROGRESS);
               return -1;
            }
#else
            return -1;
#endif

         }
      }

      if (n != 0) {

         FD_ZERO(&rset);
         FD_SET(lp_connection->sockfd, &rset);

         wset = rset;
         tval.tv_sec = timeout;
         tval.tv_usec = timeout;

         n = select((int) (lp_connection->sockfd + 1), &rset, &wset, NULL, &tval);

         if (n == 0) {
            close(lp_connection->sockfd);
            errno = ETIMEDOUT;

            return (-2);
         }
         if (NETX_FD_ISSET(lp_connection->sockfd, &rset) || NETX_FD_ISSET(lp_connection->sockfd, &wset)) {

            len = sizeof(error);
            if (getsockopt(lp_connection->sockfd, SOL_SOCKET, SO_ERROR, (void *) &error, (socklen_netx *) &len) < 0) {

               sprintf((char *) p_page->error_mess, "Diagnostic: Solaris: Pending Error %d", errno);

               return (-1);   /* Solaris pending error */
            }
         }
         else {
            ;
         }
      }

      fcntl(lp_connection->sockfd, F_SETFL, flags);      /* Restore file status flags */

      if (error) {
         close(lp_connection->sockfd);
         errno = error;
         return (-1);
      }
      return 1;

#endif
   }
   else {
      n = connect(lp_connection->sockfd, (xLPSOCKADDR) p_srv_addr, (socklen_netx) srv_addr_len);
      return n;
   }

}


int mg_db_disconnect(MGPAGE *p_page, int chndle, short context)
{
   LPMGCONX lp_connection;

   if (p_page->mode == 2) {
      return 1;
   }

   if (!p_page->pcon[chndle])
      return 0;

   if (p_page->mode == 1) {
      p_page->pcon[chndle]->in_use = 0;
      return 1;
   }

   if (context == 1 && p_page->pcon[chndle]->keep_alive) {
      p_page->pcon[chndle]->in_use = 0;
      return 1;
   }

   lp_connection = p_page->pcon[chndle];

#if defined(_WIN32)
   closesocket(lp_connection->sockfd);
   WSACleanup();
#else
   close(lp_connection->sockfd);
#endif

   mg_free((void *) p_page->pcon[chndle]);
   p_page->pcon[chndle] = NULL;

   return 1;
}

int mg_db_send(MGPAGE *p_page, int chndle, MGBUF *p_buf, int mode)
{
   int result, n, n1, len, total;
   char *request;
   unsigned char esize[8];
   LPMGCONX lp_connection;

   result = 1;

   if (mode) {
      len = mg_encode_size(esize, p_buf->data_size - p_page->header_len, MG_CHUNK_SIZE_BASE);
      strncpy((char *) (p_buf->p_buffer + (p_page->header_len - 6) + (5 - len)), (char *) esize, len);
   }

   if (p_page->mode == 2) {
      return 1;
   }

   lp_connection = p_page->pcon[chndle];

   lp_connection->eod = 0;

   request = (char *) p_buf->p_buffer;
   len = p_buf->data_size;

   total = 0;

   n1= 0;
   for (;;) {
      n = send(lp_connection->sockfd, request + total, len - total, 0);
      if (n < 0) {
         result = 0;
         break;
      }

      total += n;

      if (total == len)
         break;

      n1 ++;
      if (n1 > 100000)
         break;

   }

   return result;
}


int mg_db_receive(MGPAGE *p_page, int chndle, MGBUF *p_buf, int size, int mode)
{
   int result, n;
   unsigned long len, total, ssize;
   char s_buffer[16], stype[4];
   char *p;
   LPMGCONX lp_connection;

   if (p_page->mode == 2) {
#ifdef MG_GTM
      char *output;
      gtm_char_t      msgbuf[256];
      gtm_status_t    status;

      output = mg_malloc(MG_BUFSIZE);
      *output = '\0';

      status = (gtm_status_t) mg_gtm_ci("m_gtm_ifc", output, 0, p_buf->p_buffer, "", "", "", "", "");
      strcpy(p_buf->p_buffer, output);
      mg_free((void *) output);
      p_buf->data_size = (int) strlen(p_buf->p_buffer);

      if (status != 0) {
         mg_gtm_zstatus(msgbuf, 256);

         sprintf(p_buf->p_buffer, "00000ce\n%s", msgbuf);
         p_buf->data_size = (int) strlen(p_buf->p_buffer);

         strcpy(p_page->error_mess, msgbuf);
         result = 0;
      }
      else {
         result = p_buf->data_size;
      }
#else
      result = 0;
#endif
      return result;
   }

   lp_connection = p_page->pcon[chndle];

   p = NULL;
   result = 0;
   ssize = 0;
   s_buffer[0] = '\0';
   p_buf->p_buffer[0] = '\0';
   p_buf->data_size = 0;

   if (lp_connection->eod) {
      lp_connection->eod = 0;
      return 0;
   }
   lp_connection->eod = 0;

   len = 0;

   if (mode)
      total = size;
   else
      total = p_buf->size;

   for (;;) {

      n = recv(lp_connection->sockfd, p_buf->p_buffer + len, total - len, 0);

      if (n < 0) {
         result = len;
         lp_connection->eod = 1;
         break;
      }
      if (n < 1) {

         result = len;
         lp_connection->eod = 1;
         break;
      }

      len += n;
      p_buf->data_size += n;
      p_buf->p_buffer[len] = '\0';
      result = len;

      if (!ssize && p_buf->data_size >= MG_RECV_HEAD) {
         ssize = mg_decode_size(p_buf->p_buffer, 5, MG_CHUNK_SIZE_BASE);

         stype[0] = p_buf->p_buffer[5];
         stype[1] = p_buf->p_buffer[6];
         stype[2] = '\0';
         total = ssize + MG_RECV_HEAD;

         if (ssize && (ssize + MG_RECV_HEAD) > total) {
            if (!mg_buf_resize(p_buf, ssize + MG_RECV_HEAD + 32)) {
               p_page->mem_error = 1;
               break;
            }
         }
      }
      if (!ssize || len >= total) {
         p_buf->p_buffer[len] = '\0';
         result = len;
         lp_connection->eod = 1;
         lp_connection->keep_alive = 1;

         break;
      }

   }

   return result;
}


int mg_db_connect_init(MGPAGE *p_page, int chndle)
{
   int result, n, len, buffer_actual_size, child_port;
   char buffer[1024], buffer1[256];
   char *p, *p1;
   MGBUF request;

   if (p_page->mode == 2) {
      return 1;
   }

   result = 0;
   len = 0;

   p_page->pcon[chndle]->child_port = 0;

   mg_buf_init(&request, 1024, 1024);

   sprintf(buffer, "^S^version=%s&timeout=%d&nls=%s&uci=%s\n", MG_VERSION, 0, "", p_page->base_uci);

   mg_buf_cpy(&request, buffer, (int) strlen(buffer));

   n = mg_db_send(p_page, chndle, &request, 0);

   strcpy(buffer, "");
   buffer_actual_size = 0;
   n = mg_db_receive(p_page, chndle, &request, 1024, 0);

   if (n > 0) {
      buffer_actual_size = n;
      request.p_buffer[buffer_actual_size] = '\0';

      strcpy(buffer, (char *) request.p_buffer);

      p = strstr(buffer, "pid=");
      if (!p) {
         return 2;
      }
      if (p) {
         result = 1;
         p +=4;
         p1 = strstr(p, "&");
         if (p1)
            *p1 = '\0';
         strcpy(p_page->pcon[chndle]->mpid, p);
         if (p1)
            *p1 = '&';
      }
      p = strstr(buffer, "uci=");
      if (p) {
         p +=4;
         p1 = strstr(p, "&");
         if (p1)
            *p1 = '\0';
         if (p1)
            *p1 = '&';
      }
      p = strstr(buffer, "server_type=");
      if (p) {
         p +=12;
         p1 = strstr(p, "&");
         if (p1)
            *p1 = '\0';
         strcpy(p_page->pcon[chndle]->dbtype, p);
         if (p1)
            *p1 = '&';
      }
      p = strstr(buffer, "version=");
      if (p) {
         p +=8;
         p1 = strstr(p, "&");
         if (p1)
            *p1 = '\0';
         strcpy(buffer1, p);
         if (p1)
            *p1 = '&';
         p_page->pcon[chndle]->version = (int) strtol(buffer1, NULL, 10);
      }
      p = strstr(buffer, "child_port=");
      if (p) {
         p +=11;
         p1 = strstr(p, "&");
         if (p1)
            *p1 = '\0';
         strcpy(buffer1, p);
         if (p1)
            *p1 = '&';
         child_port = (int) strtol(buffer1, NULL, 10);

         if (child_port == 1)
            child_port = 0;

         if (child_port) {
            p_page->pcon[chndle]->child_port = child_port;
            result = -120;
         }
      }
   }

   return result;
}


int mg_db_ayt(MGPAGE *p_page, int chndle)
{
   int result, n, len, buffer_actual_size;
   char buffer[512];
   MGBUF request;

   if (p_page->mode == 2) {
      return 1;
   }

   result = 0;
   len = 0;
   buffer_actual_size = 0;

   mg_buf_init(&request, 1024, 1024);

   strcpy(buffer, "^A^A0123456789^^^^^\n");
   mg_buf_cpy(&request, buffer, (int) strlen(buffer));

   n = mg_db_send(p_page, chndle, &request, 1);

   strcpy(buffer, "");

   n = mg_db_receive(p_page, chndle, &request, 1024, 0);

   if (n > 0)
      buffer_actual_size += n;

   strcpy(buffer, (char *) request.p_buffer);
   buffer[buffer_actual_size] = '\0';

   if (buffer_actual_size > 0)
      result = 1;

   return result;
}


int mg_db_get_last_error(int context)
{
   int error_code;

#if defined(_WIN32)
   if (context)
      error_code = (int) GetLastError();
   else
      error_code = (int) WSAGetLastError();
#else
   error_code = (int) errno;
#endif

   return error_code;
}


int mg_request_header(MGPAGE *p_page, MGBUF *p_buf, char *command)
{
   char buffer[256];

   sprintf(buffer, "PHPz^P^%s#%s#0#%d#%d#%s#%d^%s^00000\n", p_page->server, p_page->uci, p_page->timeout, p_page->no_retry, MG_VERSION, p_page->storage_mode, command);

   p_page->header_len = (int) strlen(buffer);

   mg_buf_cpy(p_buf, buffer, (int) strlen(buffer));

   return 1;
}


int mg_request_add(MGPAGE *p_page, int chndle, MGBUF *p_buf, unsigned char *element, int size, short byref, short type)
{
#if 1
   int hlen;
   unsigned char head[16];

   if (type == MG_TX_AREC_FORMATTED) {
      mg_buf_cat(p_buf, (char *) element, size);
      return 1;
   }
   hlen = mg_encode_item_header(head, size, byref, type);
   mg_buf_cat(p_buf, (char *) head, hlen);
   if (size)
      mg_buf_cat(p_buf, (char *) element, size);
   return 1;
#else
   unsigned long len;
   char *p;

   len = (int) strlen((char *) element);

   if ((len + p_buf->data_size) < p_buf->size) {
      strcpy((char *) (p_buf->p_buffer + p_buf->data_size), (char *) element);
      p_buf->data_size += len;
   }
   else {
      mg_db_send(p_page, chndle, p_buf, 0);
      p_buf->data_size = 0;

      if (len > (MG_BUFSIZE / 2)) {

         p = p_buf->p_buffer;

         p_buf->p_buffer = element;
         p_buf->data_size = len;

         mg_db_send(p_page, chndle, p_buf, 0);

         p_buf->p_buffer = p;
         p_buf->data_size = 0;
      }
      else {
         mg_buf_cat(p_buf, (char *) element);
      }

   }

   return 1;
#endif
}


int mg_encode_size64(int n10)
{
   if (n10 >= 0 && n10 < 10)
      return (48 + n10);
   if (n10 >= 10 && n10 < 36)
      return (65 + (n10 - 10));
   if (n10 >= 36 && n10 < 62)
      return  (97 + (n10 - 36));

   return 0;
}


int mg_decode_size64(int nxx)
{
   if (nxx >= 48 && nxx < 58)
      return (nxx - 48);
   if (nxx >= 65 && nxx < 91)
      return ((nxx - 65) + 10);
   if (nxx >= 97 && nxx < 123)
      return ((nxx - 97) + 36);

   return 0;
}


int mg_encode_size(unsigned char *esize, int size, short base)
{
   if (base == 10) {
      sprintf((char *) esize, "%d", size);
      return (int) strlen((char *) esize);
   }
   else {
      int n, n1, x;
      char buffer[32];

      n1 = 31;
      buffer[n1 --] = '\0';
      buffer[n1 --] = mg_encode_size64(size  % base);

      for (n = 1;; n ++) {
         x = (size / ((int) pow(base, n)));
         if (!x)
            break;
         buffer[n1 --] = mg_encode_size64(x  % base);
      }
      n1 ++;
      strcpy((char *) esize, buffer + n1);
      return (int) strlen((char *) esize);
   }
}


int mg_decode_size(unsigned char *esize, int len, short base)
{
   int size;
   unsigned char c;

   if (base == 10) {
      c = *(esize + len);
      *(esize + len) = '\0';
      size = (int) strtol((char *) esize, NULL, 10);
      *(esize + len) = c;
   }
   else {
      int n, x;

      size = 0;
      for (n = len - 1; n >= 0; n --) {

         x = (int) esize[n];
         size = size + mg_decode_size64(x) * ((int) pow((double) base, ((double) (len - (n + 1)))));
      }
   }

   return size;
}


int mg_encode_item_header(unsigned char * head, int size, short byref, short type)
{
   int slen, hlen;
   unsigned int code;
   unsigned char esize[16];

   slen = mg_encode_size(esize, size, 10);

   code = slen + (type * 8) + (byref * 64);
   head[0] = (unsigned char) code;
   strncpy((char *) (head + 1), (char *) esize, slen);

   hlen = slen + 1;
   head[hlen] = '0';

   return hlen;
}


int mg_decode_item_header(unsigned char * head, int * size, short * byref, short * type)
{
   int slen, hlen;
   unsigned int code;

   code = (unsigned int) head[0];

   *byref = code / 64;
   *type = (code % 64) / 8;
   slen = code % 8;

   *size = mg_decode_size(head + 1, slen, 10);

   hlen = slen + 1;

   return hlen;
}


int mg_ucase(char *string)
{
#if defined(_WIN32) && defined(_UNICODE)

   CharUpper(string);
   return 1;

#else

   int n, chr;

   n = 0;
   while (string[n] != '\0') {
      chr = (int) string[n];
      if (chr >= 97 && chr <= 122)
         string[n] = (char) (chr - 32);
      n ++;
   }
   return 1;

#endif
}


int mg_lcase(char *string)
{
#if defined(_WIN32) && defined(_UNICODE)

   CharLower(string);
   return 1;

#else

   int n, chr;

   n = 0;
   while (string[n] != '\0') {
      chr = (int) string[n];
      if (chr >= 65 && chr <= 90)
         string[n] = (char) (chr + 32);
      n ++;
   }
   return 1;

#endif
}


int mg_buf_init(MGBUF *p_buf, int size, int increment_size)
{
   int result;

   p_buf->p_buffer = (unsigned char *) mg_malloc(sizeof(char) * (size + 1));
   if (p_buf->p_buffer) {
      *(p_buf->p_buffer) = '\0';
      result = 1;
   }
   else {
      result = 0;
      p_buf->p_buffer = (unsigned char *) mg_malloc(sizeof(char));
      if (p_buf->p_buffer) {
         *(p_buf->p_buffer) = '\0';
         size = 1;
      }
      else
         size = 0;
   }

   p_buf->size = size;
   p_buf->increment_size = increment_size;
   p_buf->data_size = 0;

   return result;
}


int mg_buf_resize(MGBUF *p_buf, unsigned long size)
{
   if (size < MG_BUFSIZE)
      return 1;

   if (size < p_buf->size)
      return 1;

   p_buf->p_buffer = (unsigned char *) mg_realloc((void *) p_buf->p_buffer, sizeof(char) * size);
   p_buf->size = size;

   return 1;
}


int mg_buf_free(MGBUF *p_buf)
{
   if (p_buf->p_buffer)
      mg_free((void *) p_buf->p_buffer);

   p_buf->p_buffer = NULL;
   p_buf->size = 0;
   p_buf->increment_size = 0;
   p_buf->data_size = 0;

   return 1;
}


int mg_buf_cpy(LPMGBUF p_buf, char *buffer, unsigned long size)
{
   unsigned long  result, req_size, csize, increment_size;

   result = 1;

   if (size == 0)
      size = (unsigned long) strlen(buffer);

   if (size == 0) {
      p_buf->data_size = 0;
      p_buf->p_buffer[p_buf->data_size] = '\0';
      return result;
   }

   req_size = size;
   if (req_size > p_buf->size) {
      csize = p_buf->size;
      increment_size = p_buf->increment_size;
      while (req_size > csize)
         csize = csize + p_buf->increment_size;
      mg_buf_free(p_buf);
      result = mg_buf_init(p_buf, (int) size, (int) increment_size);
   }
   if (result) {
      memcpy((void *) p_buf->p_buffer, (void *) buffer, size);
      p_buf->data_size = req_size;
      p_buf->p_buffer[p_buf->data_size] = '\0';
   }

   return result;
}


int mg_buf_cat(LPMGBUF p_buf, char *buffer, unsigned long size)
{
   unsigned long int result, req_size, csize, tsize, increment_size;
   unsigned char *p_temp;

   result = 1;

   if (size == 0)
      size = (unsigned long ) strlen(buffer);

   if (size == 0)
      return result;

   p_temp = NULL;
   req_size = (size + p_buf->data_size);
   tsize = p_buf->data_size;
   if (req_size > p_buf->size) {
      csize = p_buf->size;
      increment_size = p_buf->increment_size;
      while (req_size > csize)
         csize = csize + p_buf->increment_size;
      p_temp = p_buf->p_buffer;
      result = mg_buf_init(p_buf, (int) csize, (int) increment_size);
      if (result) {
         if (p_temp) {
            memcpy((void *) p_buf->p_buffer, (void *) p_temp, tsize);
            p_buf->data_size = tsize;
            mg_free((void *) p_temp);
         }
      }
      else
         p_buf->p_buffer = p_temp;
   }
   if (result) {
      memcpy((void *) (p_buf->p_buffer + tsize), (void *) buffer, size);
      p_buf->data_size = req_size;
      p_buf->p_buffer[p_buf->data_size] = '\0';
   }

   return result;
}


void * mg_malloc(unsigned long size)
{
#ifdef MG_EMALLOC
   return emalloc(size);
#else
#ifdef _WIN32
return (void *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 32);
#else
   return malloc(size);
#endif
#endif

}


void * mg_realloc(void *p_buffer, unsigned long size)
{

#if 0
   char *p;

   p = (char *) mg_malloc(size);
   strcpy(p, p_buffer);
   mg_free(p_buffer);

   return p;

#else

#ifdef MG_EMALLOC
   return erealloc((void *) p_buffer, size);
#else
#ifdef _WIN32
return (void *) HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (LPVOID) p_buffer, size + 32);
#else
   return realloc(p_buffer, size);
#endif
#endif

#endif

}


int mg_free(void *p_buffer)
{
#ifdef MG_EMALLOC
   efree((void *) p_buffer);
#else
#ifdef _WIN32
   HeapFree(GetProcessHeap(), 0, p_buffer);
#else
   free((void *) p_buffer);
#endif
#endif
   return 1;
}


int mg_log_event(char *event, char *title)
{
   int len, n;
   FILE *fp = NULL;
   char timestr[64], heading[256], buffer[2048];
   char *p_buffer;
   time_t now = 0;
#ifdef _WIN32
   HANDLE hLogfile = 0;
   DWORD dwPos = 0, dwBytesWritten = 0;
#endif

#ifdef _WIN32
__try {
#endif

   now = time(NULL);
   sprintf(timestr, "%s", ctime(&now));
   for (n = 0; timestr[n] != '\0'; n ++) {
      if ((unsigned int) timestr[n] < 32) {
         timestr[n] = '\0';
         break;
      }
   }

#ifdef _WIN32
   sprintf(heading, ">>> Time: %s; Build: %s", timestr, MG_VERSION);
#else
   sprintf(heading, ">>> PID=%ld; RN=%ld; Time: %s; Build: %s", (long) getpid(), (long) request_no, timestr, MG_VERSION);
#endif

   len = (int) strlen(heading) + (int) strlen(title) + (int) strlen(event) + 20;

   if (len < 2000)
      p_buffer = buffer;
   else
      p_buffer = (char *) mg_malloc(sizeof(char) * len);

   if (p_buffer == NULL)
      return 0;

   p_buffer[0] = '\0';
   strcpy(p_buffer, heading);
   strcat(p_buffer, "\r\n    ");
   strcat(p_buffer, title);
   strcat(p_buffer, "\r\n    ");
   strcat(p_buffer, event);
   len = ((int) strlen(p_buffer)) * sizeof(char);

#ifdef _WIN32

   strcat(p_buffer, "\r\n");
   len = len + (2 * sizeof(char));
   hLogfile = CreateFile(MG_LOG_FILE, GENERIC_WRITE, FILE_SHARE_WRITE,
                         (LPSECURITY_ATTRIBUTES) NULL, OPEN_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
   dwPos = SetFilePointer(hLogfile, 0, (LPLONG) NULL, FILE_END);
   LockFile(hLogfile, dwPos, 0, dwPos + len, 0);
   WriteFile(hLogfile, (LPTSTR) p_buffer, len, &dwBytesWritten, NULL);
   UnlockFile(hLogfile, dwPos, 0, dwPos + len, 0);
   CloseHandle(hLogfile);

#else /* UNIX or VMS */

   strcat(p_buffer, "\n");
   fp = fopen(MG_LOG_FILE, "a");
   if (fp) {
      fputs(p_buffer, fp);
      fclose(fp);
   }

#endif

   if (p_buffer != buffer) {
      mg_free((void *) p_buffer);
   }

   return 1;

#ifdef _WIN32
}
__except (EXCEPTION_EXECUTE_HANDLER ) {
      return 0;
}

#endif

}


int mg_load_gtm_library(MGPAGE *p_page, short context)
{
   int result;
   char buffer[256], path1[256], path2[256];

   result = 0;

   *buffer = '\0';
   *path1 = '\0';
   *path2 = '\0';
   *(p_page->error_mess) = '\0';
   *(p_page->info) = '\0';

#ifdef MG_GTM

   mg_gtm.gtm = 0;
   mg_gtm.gtmshr.flags = 0;

#ifdef MG_GTM_SO

   /* Try to Load the GT.M library */

   mg_gtm.gtm = 0;

#ifdef _WIN32

   result = 0;

#else

   result = 0;

   strcpy(path2, p_page->gtm_dist);

   sprintf(path1, "%s/libgtmshr.so", path2);
   result = mg_so_load(&(mg_gtm.gtmshr), path1);
   if (!result) {
      sprintf(path1, "%s/libgtmshr.sl", path2);
      result = mg_so_load(&(mg_gtm.gtmshr), path1);
      if (!result) {
         sprintf(path1, "%s/libgtmshr.dylib", path2);
         result = mg_so_load(&(mg_gtm.gtmshr), path1);
      }
   }

   if (!result) {
      strcpy(path1, "libgtmshr.so");
      result = mg_so_load(&(mg_gtm.gtmshr), path1);
      if (!result) {
         strcpy(path1, "libgtmshr.sl");
         result = mg_so_load(&(mg_gtm.gtmshr), path1);
         if (!result) {
            strcpy(path1, "libgtmshr.dylib");
            result = mg_so_load(&(mg_gtm.gtmshr), path1);
         }
      }
   }
#endif


   if (!result) {
      strcpy(p_page->error_mess, "mg_python: Initialization: Information: The GT.M library (libgtmshr) is not available on this system");
      goto gtm_init_end;
   }

   mg_gtm.p_gtm_ci = (LPFN_GTM_CI) mg_so_sym(&(mg_gtm.gtmshr), "gtm_ci");
   if (!mg_gtm.p_gtm_ci) {
      strcpy(buffer,  "gtm_ci");
      goto gtm_init_end;
   }
   mg_gtm.p_gtm_init = (LPFN_GTM_INIT) mg_so_sym(&(mg_gtm.gtmshr), "gtm_init");
   if (!mg_gtm.p_gtm_init) {
      strcpy(buffer,  "gtm_init");
      goto gtm_init_end;
   }
   mg_gtm.p_gtm_exit = (LPFN_GTM_EXIT) mg_so_sym(&(mg_gtm.gtmshr), "gtm_exit");
   if (!mg_gtm.p_gtm_exit) {
      strcpy(buffer,  "gtm_exit");
      goto gtm_init_end;
   }
   mg_gtm.p_gtm_zstatus = (LPFN_GTM_ZSTATUS) mg_so_sym(&(mg_gtm.gtmshr), "gtm_zstatus");
   if (!mg_gtm.p_gtm_zstatus) {
      strcpy(buffer,  "gtm_zstatus");
      goto gtm_init_end;
   }

   result = 1;

gtm_init_end:

   if (result) {
      mg_gtm.gtm = 1;
      sprintf(p_page->info, "mg_python: Initialization: The GT.M library (%s) is loaded", path1);
   }
   else {

      if (strlen(buffer)) {
         sprintf(p_page->error_mess, "mg_python: Initialization: Information: The GT.M library (%s) found on this system is not usable - missing '%s' function", path1, buffer);
      }

   }

#else

   mg_gtm.gtm = 1;
   strcpy(p_page->error_mess, "mg_python: Initialization: The GT.M library (libgtmshr) is incorporated in this distribution");

#endif /* #ifdef MG_GTM_USE_DSO */

   mg_gtm.load_attempted = 1;

#endif /* #ifdef MG_GTM */

   if (*(p_page->error_mess))
      result = 0;
   else
      result = 1;

   return result;

}


int mg_unload_gtm_library(MGPAGE *p_page, short context)
{

#ifdef MG_GTM
   mg_so_unload(&(mg_gtm.gtmshr));
#endif

   return 1;
}


int mg_so_load(MGSO *p_mgso, char * library)
{

#if defined(_WIN32)
   p_mgso->h_library = LoadLibrary(library);
#else
   p_mgso->h_library = dlopen(library, RTLD_NOW);
#endif

   if (p_mgso->h_library)
      return 1;
   else
      return 0;
}


MGPROC mg_so_sym(MGSO *p_mgso, char * symbol)
{
   MGPROC p_proc;

   p_proc = NULL;

#if defined(_WIN32)
   p_proc = GetProcAddress(p_mgso->h_library, symbol);
#else
   p_proc  = (void *) dlsym(p_mgso->h_library, symbol);
#endif

   return p_proc;
}


int mg_so_unload(MGSO *p_mgso)
{

#if defined(_WIN32)
   FreeLibrary(p_mgso->h_library);
#else
   dlclose(p_mgso->h_library); 
#endif

   return 1;
}


