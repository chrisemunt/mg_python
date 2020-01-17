/*
   ----------------------------------------------------------------------------
   | mg_dba.so|dll                                                            |
   | Description: An abstraction of the InterSystems Cache/IRIS API           |
   |              and YottaDB API                                             |
   | Author:      Chris Munt cmunt@mgateway.com                               |
   |                         chris.e.munt@gmail.com                           |
   | Copyright (c) 2017-2020 M/Gateway Developments Ltd,                      |
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
   |                                                                          |
   ----------------------------------------------------------------------------
*/

#ifndef MG_DBX_H
#define MG_DBX_H

#if defined(_WIN32)

#define BUILDING_NODE_EXTENSION     1
#if defined(_MSC_VER)
/* Check for MS compiler later than VC6 */
#if (_MSC_VER >= 1400)
#define _CRT_SECURE_NO_DEPRECATE    1
#define _CRT_NONSTDC_NO_DEPRECATE   1
#endif
#endif

#elif defined(__linux__) || defined(__linux) || defined(linux)

#if !defined(LINUX)
#define LINUX                       1
#endif

#elif defined(__APPLE__)

#if !defined(MACOSX)
#define MACOSX                      1
#endif

#endif

#if defined(SOLARIS)
#ifndef __GNUC__
#  define  __attribute__(x)
#endif
#endif

#if defined(_WIN32)
/*
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
*/
#include <stdlib.h>
#define INCL_WINSOCK_API_TYPEDEFS 1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#else
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <dlfcn.h>
#include <signal.h>
#include <pwd.h>
*/

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
#include <math.h>

#endif


/* Cache/IRIS */

#define CACHE_MAXSTRLEN	32767
#define CACHE_MAXLOSTSZ	3641144

typedef char		Callin_char_t;
#define CACHE_INT64 long long
#define CACHESTR	CACHE_ASTR

typedef struct {
   unsigned short len;
   Callin_char_t  str[CACHE_MAXSTRLEN];
} CACHE_ASTR, *CACHE_ASTRP;

typedef struct {
   unsigned int	len;
   union {
      Callin_char_t * ch;
      unsigned short *wch;
      unsigned short *lch;
   } str;
} CACHE_EXSTR, *CACHE_EXSTRP;

#define CACHE_TTALL     1
#define CACHE_TTNEVER   8
#define CACHE_PROGMODE  32

#define CACHE_INT	      1
#define CACHE_DOUBLE	   2
#define CACHE_ASTRING   3

#define CACHE_CHAR      4
#define CACHE_INT2      5
#define CACHE_INT4      6
#define CACHE_INT8      7
#define CACHE_UCHAR     8
#define CACHE_UINT2     9
#define CACHE_UINT4     10
#define CACHE_UINT8     11
#define CACHE_FLOAT     12
#define CACHE_HFLOAT    13
#define CACHE_UINT      14
#define CACHE_WSTRING   15
#define CACHE_OREF      16
#define CACHE_LASTRING  17
#define CACHE_LWSTRING  18
#define CACHE_IEEE_DBL  19
#define CACHE_HSTRING   20
#define CACHE_UNDEF     21

#define CACHE_CHANGEPASSWORD  -16
#define CACHE_ACCESSDENIED    -15
#define CACHE_EXSTR_INUSE     -14
#define CACHE_NORES	         -13
#define CACHE_BADARG	         -12
#define CACHE_NOTINCACHE      -11
#define CACHE_RETTRUNC 	      -10
#define CACHE_ERUNKNOWN	      -9	
#define CACHE_RETTOOSMALL     -8	
#define CACHE_NOCON 	         -7
#define CACHE_INTERRUPT       -6
#define CACHE_CONBROKEN       -4
#define CACHE_STRTOOLONG      -3
#define CACHE_ALREADYCON      -2
#define CACHE_FAILURE	      -1
#define CACHE_SUCCESS 	      0

#define CACHE_ERMXSTR         5
#define CACHE_ERNOLINE        8
#define CACHE_ERUNDEF         9
#define CACHE_ERSYSTEM        10
#define CACHE_ERSUBSCR        16
#define CACHE_ERNOROUTINE     17
#define CACHE_ERSTRINGSTACK   20
#define CACHE_ERUNIMPLEMENTED 22
#define CACHE_ERARGSTACK      25
#define CACHE_ERPROTECT       27
#define CACHE_ERPARAMETER     40
#define CACHE_ERNAMSP         83
#define CACHE_ERWIDECHAR      89
#define CACHE_ERNOCLASS       122
#define CACHE_ERBADOREF       119
#define CACHE_ERNOMETHOD      120
#define CACHE_ERNOPROPERTY    121

#define CACHE_ETIMEOUT        -100
#define CACHE_BAD_STRING      -101
#define CACHE_BAD_NAMESPACE   -102
#define CACHE_BAD_GLOBAL      -103
#define CACHE_BAD_FUNCTION    -104
#define CACHE_BAD_CLASS       -105
#define CACHE_BAD_METHOD      -106

#define CACHE_INCREMENTAL_LOCK   1

/* End of Cache/IRIS */


/* YottaDB */

#define YDB_DEL_TREE 1

typedef struct {
   unsigned int   len_alloc;
   unsigned int   len_used;
   char		      *buf_addr;
} ydb_buffer_t;

typedef struct {
   unsigned long  length;
   char		      *address;
} ydb_string_t;

typedef struct {
   ydb_string_t   rtn_name;
   void		      *handle;
} ci_name_descriptor;

typedef ydb_buffer_t DBXSTR;


/* End of YottaDB */

/* GT.M call-in interface */

typedef int    gtm_status_t;
typedef char   gtm_char_t;
typedef int    xc_status_t;

/* End of GT.M call-in interface */


#define DBX_DBTYPE_CACHE         1
#define DBX_DBTYPE_IRIS          2
#define DBX_DBTYPE_YOTTADB       5
#define DBX_DBTYPE_GTM           11

#define DBX_MAXCONS              32
#define DBX_MAXARGS              64

#define DBX_ERROR_SIZE           512

#define DBX_DSORT_INVALID        0
#define DBX_DSORT_DATA           1
#define DBX_DSORT_SUBSCRIPT      2
#define DBX_DSORT_GLOBAL         3
#define DBX_DSORT_EOD            9
#define DBX_DSORT_STATUS         10
#define DBX_DSORT_ERROR          11

#define DBX_DSORT_ISVALID(a)     ((a == DBX_DSORT_GLOBAL) || (a == DBX_DSORT_SUBSCRIPT) || (a == DBX_DSORT_DATA) || (a == DBX_DSORT_EOD) || (a == DBX_DSORT_STATUS))

#define DBX_DTYPE_NONE           0
#define DBX_DTYPE_DBXSTR         1
#define DBX_DTYPE_STR            2
#define DBX_DTYPE_INT            4
#define DBX_DTYPE_INT64          5
#define DBX_DTYPE_DOUBLE         6
#define DBX_DTYPE_OREF           7
#define DBX_DTYPE_NULL           10

#define DBX_MAXSIZE              32767
#define DBX_BUFFER               32768

#define DBX_LS_MAXSIZE           3641144
#define DBX_LS_BUFFER            3641145

#if defined(MAX_PATH) && (MAX_PATH>511)
#define DBX_MAX_PATH             MAX_PATH
#else
#define DBX_MAX_PATH             512
#endif

#if defined(_WIN32)
#define DBX_NULL_DEVICE          "//./nul"
#else
#define DBX_NULL_DEVICE          "/dev/null/"
#endif

#if defined(_WIN32)
#define DBX_CACHE_DLL            "cache.dll"
#define DBX_IRIS_DLL             "irisdb.dll"
#define DBX_YDB_DLL              "yottadb.dll"
#define DBX_GTM_DLL              "gtmshr.dll"
#else
#define DBX_CACHE_SO             "libcache.so"
#define DBX_CACHE_DYLIB          "libcache.dylib"
#define DBX_IRIS_SO              "libirisdb.so"
#define DBX_IRIS_DYLIB           "libirisdb.dylib"
#define DBX_YDB_SO               "libyottadb.so"
#define DBX_YDB_DYLIB            "libyottadb.dylib"
#define DBX_GTM_SO               "libgtmshr.so"
#define DBX_GTM_DYLIB            "libgtmshr.dylib"
#endif

#if defined(__linux__) || defined(linux) || defined(LINUX)
#define DBX_MEMCPY(a,b,c)           memmove(a,b,c)
#else
#define DBX_MEMCPY(a,b,c)           memcpy(a,b,c)
#endif

#define DBX_LOCK(RC, TIMEOUT) \
   if (pcon->use_db_mutex) { \
      RC = mg_mutex_lock(pcon->p_db_mutex, TIMEOUT); \
   } \

#define DBX_UNLOCK(RC) \
   if (pcon->use_db_mutex) { \
      RC = mg_mutex_unlock(pcon->p_db_mutex); \
   } \


#define NETX_TIMEOUT             10
#define NETX_IPV6                1
#define NETX_READ_EOF            0
#define NETX_READ_NOCON          -1
#define NETX_READ_ERROR          -2
#define NETX_READ_TIMEOUT        -3
#define NETX_RECV_BUFFER         32768

#if defined(LINUX)
#define NETX_MEMCPY(a,b,c)           memmove(a,b,c)
#else
#define NETX_MEMCPY(a,b,c)           memcpy(a,b,c)
#endif

#if defined(_WIN32)

#if !defined(MG_DBA_EMBEDDED)
#define DBX_EXTFUN(a)    __declspec(dllexport) a __cdecl
#else
#define DBX_EXTFUN(a)    a
#endif

#define NETX_WSASOCKET               netx_so.p_WSASocket
#define NETX_WSAGETLASTERROR         netx_so.p_WSAGetLastError
#define NETX_WSASTARTUP              netx_so.p_WSAStartup
#define NETX_WSACLEANUP              netx_so.p_WSACleanup
#define NETX_WSAFDISET               netx_so.p_WSAFDIsSet
#define NETX_WSARECV                 netx_so.p_WSARecv
#define NETX_WSASEND                 netx_so.p_WSASend

#define NETX_WSASTRINGTOADDRESS      netx_so.p_WSAStringToAddress
#define NETX_WSAADDRESSTOSTRING      netx_so.p_WSAAddressToString
#define NETX_GETADDRINFO             netx_so.p_getaddrinfo
#define NETX_FREEADDRINFO            netx_so.p_freeaddrinfo
#define NETX_GETNAMEINFO             netx_so.p_getnameinfo
#define NETX_GETPEERNAME             netx_so.p_getpeername
#define NETX_INET_NTOP               netx_so.p_inet_ntop
#define NETX_INET_PTON               netx_so.p_inet_pton

#define NETX_CLOSESOCKET             netx_so.p_closesocket
#define NETX_GETHOSTNAME             netx_so.p_gethostname
#define NETX_GETHOSTBYNAME           netx_so.p_gethostbyname
#define NETX_SETSERVBYNAME           netx_so.p_getservbyname
#define NETX_GETHOSTBYADDR           netx_so.p_gethostbyaddr
#define NETX_HTONS                   netx_so.p_htons
#define NETX_HTONL                   netx_so.p_htonl
#define NETX_NTOHL                   netx_so.p_ntohl
#define NETX_NTOHS                   netx_so.p_ntohs
#define NETX_CONNECT                 netx_so.p_connect
#define NETX_INET_ADDR               netx_so.p_inet_addr
#define NETX_INET_NTOA               netx_so.p_inet_ntoa
#define NETX_SOCKET                  netx_so.p_socket
#define NETX_SETSOCKOPT              netx_so.p_setsockopt
#define NETX_GETSOCKOPT              netx_so.p_getsockopt
#define NETX_GETSOCKNAME             netx_so.p_getsockname
#define NETX_SELECT                  netx_so.p_select
#define NETX_RECV                    netx_so.p_recv
#define NETX_SEND                    netx_so.p_send
#define NETX_SHUTDOWN                netx_so.p_shutdown
#define NETX_BIND                    netx_so.p_bind
#define NETX_LISTEN                  netx_so.p_listen
#define NETX_ACCEPT                  netx_so.p_accept

#define  NETX_FD_ISSET(fd, set)              netx_so.p_WSAFDIsSet((SOCKET)(fd), (fd_set *)(set))

typedef int (WINAPI * LPFN_WSAFDISSET)       (SOCKET, fd_set *);

typedef DWORD           DBXTHID;
typedef HINSTANCE       DBXPLIB;
typedef FARPROC         DBXPROC;

typedef LPSOCKADDR      xLPSOCKADDR;
typedef u_long          *xLPIOCTL;
typedef const char      *xLPSENDBUF;
typedef char            *xLPRECVBUF;

#ifdef _WIN64
typedef int             socklen_netx;
#else
typedef size_t          socklen_netx;
#endif

#define SOCK_ERROR(n)   (n == SOCKET_ERROR)
#define INVALID_SOCK(n) (n == INVALID_SOCKET)
#define NOT_BLOCKING(n) (n != WSAEWOULDBLOCK)

#define BZERO(b,len) (memset((b), '\0', (len)), (void) 0)

#else /* #if defined(_WIN32) */

#define DBX_EXTFUN(a)    a

#define NETX_WSASOCKET               WSASocket
#define NETX_WSAGETLASTERROR         WSAGetLastError
#define NETX_WSASTARTUP              WSAStartup
#define NETX_WSACLEANUP              WSACleanup
#define NETX_WSAFDIsSet              WSAFDIsSet
#define NETX_WSARECV                 WSARecv
#define NETX_WSASEND                 WSASend

#define NETX_WSASTRINGTOADDRESS      WSAStringToAddress
#define NETX_WSAADDRESSTOSTRING      WSAAddressToString
#define NETX_GETADDRINFO             getaddrinfo
#define NETX_FREEADDRINFO            freeaddrinfo
#define NETX_GETNAMEINFO             getnameinfo
#define NETX_GETPEERNAME             getpeername
#define NETX_INET_NTOP               inet_ntop
#define NETX_INET_PTON               inet_pton

#define NETX_CLOSESOCKET             closesocket
#define NETX_GETHOSTNAME             gethostname
#define NETX_GETHOSTBYNAME           gethostbyname
#define NETX_SETSERVBYNAME           getservbyname
#define NETX_GETHOSTBYADDR           gethostbyaddr
#define NETX_HTONS                   htons
#define NETX_HTONL                   htonl
#define NETX_NTOHL                   ntohl
#define NETX_NTOHS                   ntohs
#define NETX_CONNECT                 connect
#define NETX_INET_ADDR               inet_addr
#define NETX_INET_NTOA               inet_ntoa
#define NETX_SOCKET                  socket
#define NETX_SETSOCKOPT              setsockopt
#define NETX_GETSOCKOPT              getsockopt
#define NETX_GETSOCKNAME             getsockname
#define NETX_SELECT                  select
#define NETX_RECV                    recv
#define NETX_SEND                    send
#define NETX_SHUTDOWN                shutdown
#define NETX_BIND                    bind
#define NETX_LISTEN                  listen
#define NETX_ACCEPT                  accept

#define NETX_FD_ISSET(fd, set) FD_ISSET(fd, set)

typedef pthread_t       DBXTHID;
typedef void            *DBXPLIB;
typedef void            *DBXPROC;

typedef unsigned long   DWORD;
typedef unsigned long   WORD;
typedef int             WSADATA;
typedef int             SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr * LPSOCKADDR;
typedef struct hostent  HOSTENT;
typedef struct hostent  * LPHOSTENT;
typedef struct servent  SERVENT;
typedef struct servent  * LPSERVENT;

#ifdef NETX_BS_GEN_PTR
typedef const void      * xLPSOCKADDR;
typedef void            * xLPIOCTL;
typedef const void      * xLPSENDBUF;
typedef void            * xLPRECVBUF;
#else
typedef LPSOCKADDR      xLPSOCKADDR;
typedef char            * xLPIOCTL;
typedef const char      * xLPSENDBUF;
typedef char            * xLPRECVBUF;
#endif /* #ifdef NETX_BS_GEN_PTR */

#if defined(OSF1) || defined(HPUX) || defined(HPUX10) || defined(HPUX11)
typedef int             socklen_netx;
#elif defined(LINUX) || defined(AIX) || defined(AIX5) || defined(MACOSX)
typedef socklen_t       socklen_netx;
#else
typedef size_t          socklen_netx;
#endif

#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif

#define SOCK_ERROR(n)   (n < 0)
#define INVALID_SOCK(n) (n < 0)
#define NOT_BLOCKING(n) (n != EWOULDBLOCK && n != 2)

#define BZERO(b, len)   (bzero(b, len))

#endif /* #if defined(_WIN32) */


typedef struct tagNETXSOCK {

   unsigned char                 winsock_ready;
   short                         sock;
   short                         load_attempted;
   short                         nagle_algorithm;
   short                         winsock;
   short                         ipv6;
   DBXPLIB                       plibrary;

   char                          libnam[256];

#if defined(_WIN32)
   WSADATA                       wsadata;
   int                           wsastartup;
   WORD                          version_requested;
   LPFN_WSASOCKET                p_WSASocket;
   LPFN_WSAGETLASTERROR          p_WSAGetLastError; 
   LPFN_WSASTARTUP               p_WSAStartup;
   LPFN_WSACLEANUP               p_WSACleanup;
   LPFN_WSAFDISSET               p_WSAFDIsSet;
   LPFN_WSARECV                  p_WSARecv;
   LPFN_WSASEND                  p_WSASend;

#if defined(NETX_IPV6)
   LPFN_WSASTRINGTOADDRESS       p_WSAStringToAddress;
   LPFN_WSAADDRESSTOSTRING       p_WSAAddressToString;
   LPFN_GETADDRINFO              p_getaddrinfo;
   LPFN_FREEADDRINFO             p_freeaddrinfo;
   LPFN_GETNAMEINFO              p_getnameinfo;
   LPFN_GETPEERNAME              p_getpeername;
   LPFN_INET_NTOP                p_inet_ntop;
   LPFN_INET_PTON                p_inet_pton;
#else
   LPVOID                        p_WSAStringToAddress;
   LPVOID                        p_WSAAddressToString;
   LPVOID                        p_getaddrinfo;
   LPVOID                        p_freeaddrinfo;
   LPVOID                        p_getnameinfo;
   LPVOID                        p_getpeername;
   LPVOID                        p_inet_ntop;
   LPVOID                        p_inet_pton;
#endif

   LPFN_CLOSESOCKET              p_closesocket;
   LPFN_GETHOSTNAME              p_gethostname;
   LPFN_GETHOSTBYNAME            p_gethostbyname;
   LPFN_GETHOSTBYADDR            p_gethostbyaddr;
   LPFN_GETSERVBYNAME            p_getservbyname;

   LPFN_HTONS                    p_htons;
   LPFN_HTONL                    p_htonl;
   LPFN_NTOHL                    p_ntohl;
   LPFN_NTOHS                    p_ntohs;
   LPFN_CONNECT                  p_connect;
   LPFN_INET_ADDR                p_inet_addr;
   LPFN_INET_NTOA                p_inet_ntoa;

   LPFN_SOCKET                   p_socket;
   LPFN_SETSOCKOPT               p_setsockopt;
   LPFN_GETSOCKOPT               p_getsockopt;
   LPFN_GETSOCKNAME              p_getsockname;
   LPFN_SELECT                   p_select;
   LPFN_RECV                     p_recv;
   LPFN_SEND                     p_send;
   LPFN_SHUTDOWN                 p_shutdown;
   LPFN_BIND                     p_bind;
   LPFN_LISTEN                   p_listen;
   LPFN_ACCEPT                   p_accept;
#endif /* #if defined(_WIN32) */

} NETXSOCK, *PNETXSOCK;


typedef struct tagDBXDEBUG {
   unsigned char  debug;
   FILE *         p_fdebug;
} DBXDEBUG, *PDBXDEBUG;


typedef struct tagDBXZV {
   unsigned char  product;
   double         mg_version;
   int            majorversion;
   int            minorversion;
   int            mg_build;
   unsigned long  vnumber; /* yymbbbb */
   char           version[64];
} DBXZV, *PDBXZV;

typedef struct tagDBXMUTEX {
   unsigned char     created;
   int               stack;
#if defined(_WIN32)
   HANDLE            h_mutex;
#else
   pthread_mutex_t   h_mutex;
#endif /* #if defined(_WIN32) */
   DBXTHID           thid;
} DBXMUTEX, *PDBXMUTEX;


typedef struct tagDBXCVAL {
   void           *pstr;
   CACHE_EXSTR    zstr;
} DBXCVAL, *PDBXCVAL;


typedef struct tagDBXVAL {
   short          type;
   union {
      int            int32;
      long long      int64;
      double         real;
      unsigned int   oref;
   } num;
   unsigned long  offset;
   DBXSTR         svalue;
   DBXCVAL cvalue;
} DBXVAL, *PDBXVAL;


typedef struct tagDBXFUN {
   unsigned int   rflag;
   int            label_len;
   char *         label;
   int            routine_len;
   char *         routine;
   char           buffer[128];
} DBXFUN, *PDBXFUN;


typedef struct tagDBXISCSO {

   short             loaded;
   short             iris;
   short             merge_enabled;
   char              funprfx[8];
   char              libdir[256];
   char              libnam[256];
   char              dbname[32];
   DBXPLIB           p_library;

   int               (* p_CacheSetDir)                   (char * dir);
   int               (* p_CacheSecureStartA)             (CACHE_ASTRP username, CACHE_ASTRP password, CACHE_ASTRP exename, unsigned long flags, int tout, CACHE_ASTRP prinp, CACHE_ASTRP prout);
   int               (* p_CacheEnd)                      (void);

   unsigned char *   (* p_CacheExStrNew)                 (CACHE_EXSTRP zstr, int size);
   unsigned short *  (* p_CacheExStrNewW)                (CACHE_EXSTRP zstr, int size);
   wchar_t *         (* p_CacheExStrNewH)                (CACHE_EXSTRP zstr, int size);
   int               (* p_CachePushExStr)                (CACHE_EXSTRP sptr);
   int               (* p_CachePushExStrW)               (CACHE_EXSTRP sptr);
   int               (* p_CachePushExStrH)               (CACHE_EXSTRP sptr);
   int               (* p_CachePopExStr)                 (CACHE_EXSTRP sstrp);
   int               (* p_CachePopExStrW)                (CACHE_EXSTRP sstrp);
   int               (* p_CachePopExStrH)                (CACHE_EXSTRP sstrp);
   int               (* p_CacheExStrKill)                (CACHE_EXSTRP obj);
   int               (* p_CachePushStr)                  (int len, Callin_char_t * ptr);
   int               (* p_CachePushStrW)                 (int len, short * ptr);
   int               (* p_CachePushStrH)                 (int len, wchar_t * ptr);
   int               (* p_CachePopStr)                   (int * lenp, Callin_char_t ** strp);
   int               (* p_CachePopStrW)                  (int * lenp, short ** strp);
   int               (* p_CachePopStrH)                  (int * lenp, wchar_t ** strp);
   int               (* p_CachePushDbl)                  (double num);
   int               (* p_CachePushIEEEDbl)              (double num);
   int               (* p_CachePopDbl)                   (double * nump);
   int               (* p_CachePushInt)                  (int num);
   int               (* p_CachePopInt)                   (int * nump);
   int               (* p_CachePushInt64)                (CACHE_INT64 num);
   int               (* p_CachePopInt64)                 (CACHE_INT64 * nump);

   int               (* p_CachePushGlobal)               (int nlen, const Callin_char_t * nptr);
   int               (* p_CachePushGlobalX)              (int nlen, const Callin_char_t * nptr, int elen, const Callin_char_t * eptr);
   int               (* p_CacheGlobalGet)                (int narg, int flag);
   int               (* p_CacheGlobalSet)                (int narg);
   int               (* p_CacheGlobalData)               (int narg, int valueflag);
   int               (* p_CacheGlobalKill)               (int narg, int nodeonly);
   int               (* p_CacheGlobalOrder)              (int narg, int dir, int valueflag);
   int               (* p_CacheGlobalQuery)              (int narg, int dir, int valueflag);
   int               (* p_CacheGlobalIncrement)          (int narg);
   int               (* p_CacheGlobalRelease)            (void);

   int               (* p_CacheAcquireLock)              (int nsub, int flg, int tout, int * rval);
   int               (* p_CacheReleaseAllLocks)          (void);
   int               (* p_CacheReleaseLock)              (int nsub, int flg);
   int               (* p_CachePushLock)                 (int nlen, const Callin_char_t * nptr);

   int               (* p_CacheAddGlobal)                (int num, const Callin_char_t * nptr);
   int               (* p_CacheAddGlobalDescriptor)      (int num);
   int               (* p_CacheAddSSVN)                  (int num, const Callin_char_t * nptr);
   int               (* p_CacheAddSSVNDescriptor)        (int num);
   int               (* p_CacheMerge)                    (void);

   int               (* p_CachePushFunc)                 (unsigned int * rflag, int tlen, const Callin_char_t * tptr, int nlen, const Callin_char_t * nptr);
   int               (* p_CacheExtFun)                   (unsigned int flags, int narg);
   int               (* p_CachePushRtn)                  (unsigned int * rflag, int tlen, const Callin_char_t * tptr, int nlen, const Callin_char_t * nptr);
   int               (* p_CacheDoFun)                    (unsigned int flags, int narg);
   int               (* p_CacheDoRtn)                    (unsigned int flags, int narg);

   int               (* p_CacheCloseOref)                (unsigned int oref);
   int               (* p_CacheIncrementCountOref)       (unsigned int oref);
   int               (* p_CachePopOref)                  (unsigned int * orefp);
   int               (* p_CachePushOref)                 (unsigned int oref);
   int               (* p_CacheInvokeMethod)             (int narg);
   int               (* p_CachePushMethod)               (unsigned int oref, int mlen, const Callin_char_t * mptr, int flg);
   int               (* p_CacheInvokeClassMethod)        (int narg);
   int               (* p_CachePushClassMethod)          (int clen, const Callin_char_t * cptr, int mlen, const Callin_char_t * mptr, int flg);
   int               (* p_CacheGetProperty)              (void);
   int               (* p_CacheSetProperty)              (void);
   int               (* p_CachePushProperty)             (unsigned int oref, int plen, const Callin_char_t * pptr);

   int               (* p_CacheType)                     (void);

   int               (* p_CacheEvalA)                    (CACHE_ASTRP volatile expr);
   int               (* p_CacheExecuteA)                 (CACHE_ASTRP volatile cmd);
   int               (* p_CacheConvert)                  (unsigned long type, void * rbuf);

   int               (* p_CacheErrorA)                   (CACHE_ASTRP, CACHE_ASTRP, int *);
   int               (* p_CacheErrxlateA)                (int, CACHE_ASTRP);

   int               (* p_CacheEnableMultiThread)        (void);

} DBXISCSO, *PDBXISCSO;


typedef struct tagDBXYDBSO {
   short             loaded;
   char              libdir[256];
   char              libnam[256];
   char              funprfx[8];
   char              dbname[32];
   DBXPLIB           p_library;

   int               (* p_ydb_init)                      (void);
   int               (* p_ydb_exit)                      (void);
   int               (* p_ydb_malloc)                    (size_t size);
   int               (* p_ydb_free)                      (void *ptr);
   int               (* p_ydb_data_s)                    (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, unsigned int *ret_value);
   int               (* p_ydb_delete_s)                  (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int deltype);
   int               (* p_ydb_set_s)                     (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *value);
   int               (* p_ydb_get_s)                     (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_subscript_next_s)          (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_subscript_previous_s)      (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *ret_value);
   int               (* p_ydb_node_next_s)               (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int *ret_subs_used, ydb_buffer_t *ret_subsarray);
   int               (* p_ydb_node_previous_s)           (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, int *ret_subs_used, ydb_buffer_t *ret_subsarray);
   int               (* p_ydb_incr_s)                    (ydb_buffer_t *varname, int subs_used, ydb_buffer_t *subsarray, ydb_buffer_t *increment, ydb_buffer_t *ret_value);
   int               (* p_ydb_ci)                        (const char *c_rtn_name, ...);
   int               (* p_ydb_cip)                       (ci_name_descriptor *ci_info, ...);
} DBXYDBSO, *PDBXYDBSO;


typedef struct tagDBXGTMSO {
   short             loaded;
   char              libdir[256];
   char              libnam[256];
   char              funprfx[8];
   char              dbname[32];
   DBXPLIB           p_library;
   xc_status_t       (* p_gtm_ci)       (const char *c_rtn_name, ...);
   xc_status_t       (* p_gtm_init)     (void);
   xc_status_t       (* p_gtm_exit)     (void);
   void              (* p_gtm_zstatus)  (char* msg, int len);
} DBXGTMSO, *PDBXGTMSO;


typedef struct tagDBXCON {
   short          dbtype;
   int            argc;
   unsigned long  pid;
   char           shdir[256];
   char           username[64];
   char           password[64];
   char           nspace[64];
   char           input_device[64];
   char           output_device[64];
   char           debug_str[64];
   DBXSTR         input_str;
   DBXVAL         output_val;
   int            offset;
   DBXVAL         args[DBX_MAXARGS];
   ydb_buffer_t   yargs[DBX_MAXARGS];
   int            error_code;
   char           error[DBX_ERROR_SIZE];
   short          use_db_mutex;
   DBXMUTEX       *p_db_mutex;
   DBXMUTEX       db_mutex;
   DBXZV          *p_zv;
   DBXZV          zv;
   DBXDEBUG       *p_debug;
   DBXDEBUG       debug;
   DBXISCSO       *p_isc_so;
   DBXYDBSO       *p_ydb_so;
   DBXGTMSO       *p_gtm_so;
   short          increment;

   short          connected;
   int            port;
   char           ip_address[128];
   int            error_no;
   int            timeout;
   int            eof;
   SOCKET         cli_socket;
   char           info[256];

   /* Old MGWSI protocol */

   short          eod;
   short          keep_alive;
   short          in_use;
   int            chndle;
   int            base_port;
   int            child_port;
   char           command[4];
   char           mpid[128];
   char           server[64];
   char           server_software[64];
   char           zmgsi_version[8];
   void *         p_srv;

} DBXCON, *PDBXCON;


#define MG_HOST                  "127.0.0.1"
/*
#define MG_PORT                  7040
*/
#define MG_PORT                  7041
#define MG_SERVER                "LOCAL"
#define MG_UCI                   "USER"

#if !defined(MG_DBA_EMBEDDED)
#define MG_PRODUCT               "g"
#endif

#define MG_MAXCON                32

#define MG_TX_DATA               0
#define MG_TX_AKEY               1
#define MG_TX_AREC               2
#define MG_TX_EOD                3
#define MG_TX_AREC_FORMATTED     9

#define MG_RECV_HEAD             8

#define MG_CHUNK_SIZE_BASE       62

#define MG_BUFSIZE               32768
#define MG_BUFMAX                32767

#define MG_ES_DELIM              0
#define MG_ES_BLOCK              1

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

typedef struct tagMGSRV {
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
   char        shdir[256];
   char        ip_address[64];
   int         port;
   int         timeout;
   int         no_retry;
   int         nagle_algorithm;
   char        username[64];
   char        password[256];
   char        dbtype_name[32];
   MGBUF *     p_env;
   MGBUF *     p_params;
   PDBXCON     pcon[MG_MAXCON];
} MGSRV, *LPMGSRV;


DBX_EXTFUN(int)         dbx_init                      ();
DBX_EXTFUN(int)         dbx_version                   (int index, char *output, int output_len);
DBX_EXTFUN(int)         dbx_open                      (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_close                     (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_set                       (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_get                       (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_next                      (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_previous                  (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_delete                    (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_defined                   (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_increment                 (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_function                  (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_classmethod               (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_method                    (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_getproperty               (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_setproperty               (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_closeinstance             (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_getnamespace              (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_setnamespace              (unsigned char *input, unsigned char *output);
DBX_EXTFUN(int)         dbx_sleep                     (int period_ms);
DBX_EXTFUN(int)         dbx_benchmark                 (unsigned char *inputstr, unsigned char *outputstr);

int                     isc_load_library              (DBXCON *pcon);
int                     isc_authenticate              (DBXCON *pcon);
int                     isc_open                      (DBXCON *pcon);
int                     isc_parse_zv                  (char *zv, DBXZV * p_isc_sv);
int                     isc_change_namespace          (DBXCON *pcon, char *nspace);
int                     isc_pop_value                 (DBXCON *pcon, DBXVAL *value, int required_type);
int                     isc_error_message             (DBXCON *pcon, int error_code);

int                     ydb_load_library              (DBXCON *pcon);
int                     ydb_open                      (DBXCON *pcon);
int                     ydb_parse_zv                  (char *zv, DBXZV * p_ydb_sv);
int                     ydb_error_message             (DBXCON *pcon, int error_code);
int                     ydb_function                  (DBXCON *pcon, DBXFUN *pfun);

int                     gtm_load_library              (DBXCON *pcon);
int                     gtm_open                      (DBXCON *pcon);
int                     gtm_parse_zv                  (char *zv, DBXZV * p_gtm_sv);
int                     gtm_error_message             (DBXCON *pcon, int error_code);

DBXCON *                mg_unpack_header              (unsigned char *input, unsigned char *output);
int                     mg_unpack_arguments           (DBXCON *pcon);
int                     mg_global_reference           (DBXCON *pcon);
int                     mg_function_reference         (DBXCON *pcon, DBXFUN *pfun);
int                     mg_class_reference            (DBXCON *pcon, short context);
int                     mg_add_block_size             (DBXSTR *block, unsigned long offset, unsigned long data_len, int dsort, int dtype);
unsigned long           mg_get_block_size             (DBXSTR *block, unsigned long offset, int *dsort, int *dtype);
int                     mg_set_size                   (unsigned char *str, unsigned long data_len);
unsigned long           mg_get_size                   (unsigned char *str);

int                     mg_buf_init                   (MGBUF *p_buf, int size, int increment_size);
int                     mg_buf_resize                 (MGBUF *p_buf, unsigned long size);
int                     mg_buf_free                   (MGBUF *p_buf);
int                     mg_buf_cpy                    (MGBUF *p_buf, char * buffer, unsigned long size);
int                     mg_buf_cat                    (MGBUF *p_buf, char * buffer, unsigned long size);

void *                  mg_realloc                    (void *p, int curr_size, int new_size, short id);
void *                  mg_malloc                     (int size, short id);
int                     mg_free                       (void *p, short id);

int                     mg_ucase                      (char *string);
int                     mg_lcase                      (char *string);
int                     mg_create_string              (DBXCON *pcon, void *data, short type);
int                     mg_buffer_dump                (DBXCON *pcon, void *buffer, unsigned int len, char *title, short mode);
int                     mg_log_event                  (DBXDEBUG *p_debug, char *message, char *title, int level);
int                     mg_pause                      (int msecs);
DBXPLIB                 mg_dso_load                   (char *library);
DBXPROC                 mg_dso_sym                    (DBXPLIB p_library, char *symbol);
int                     mg_dso_unload                 (DBXPLIB p_library);
DBXTHID                 mg_current_thread_id          (void);
unsigned long           mg_current_process_id         (void);
int                     mg_error_message              (DBXCON *pcon, int error_code);
int                     mg_set_error_message          (DBXCON *pcon);
int                     mg_set_error_message_ex       (unsigned char *output, char *error_message);
int                     mg_cleanup                    (DBXCON *pcon);

int                     mg_mutex_create               (DBXMUTEX *p_mutex);
int                     mg_mutex_lock                 (DBXMUTEX *p_mutex, int timeout);
int                     mg_mutex_unlock               (DBXMUTEX *p_mutex);
int                     mg_mutex_destroy              (DBXMUTEX *p_mutex);
int                     mg_enter_critical_section     (void *p_crit);
int                     mg_leave_critical_section     (void *p_crit);

int                     mg_sleep                      (unsigned long msecs);

int                     netx_load_winsock             (DBXCON *pcon, int context);
int                     netx_tcp_connect              (DBXCON *pcon, int context);
int                     netx_tcp_handshake            (DBXCON *pcon, int context);
int                     netx_tcp_command              (DBXCON *pcon, int context);
int                     netx_tcp_connect_ex           (DBXCON *pcon, xLPSOCKADDR p_srv_addr, socklen_netx srv_addr_len, int timeout);
int                     netx_tcp_disconnect           (DBXCON *pcon, int context);
int                     netx_tcp_write                (DBXCON *pcon, unsigned char *data, int size);
int                     netx_tcp_read                 (DBXCON *pcon, unsigned char *data, int size, int timeout, int context);
int                     netx_get_last_error           (int context);
int                     netx_get_error_message        (int error_code, char *message, int size, int context);
int                     netx_get_std_error_message    (int error_code, char *message, int size, int context);


int                     mg_db_command                 (DBXCON *pcon, int context);
int                     mg_db_connect                 (MGSRV *p_srv, int *chndle, short context);
int                     mg_db_disconnect              (MGSRV *p_srv, int chndle, short context);
int                     mg_db_send                    (MGSRV *p_srv, int chndle, MGBUF *p_buf, int mode);
int                     mg_db_receive                 (MGSRV *p_srv, int chndle, MGBUF *p_buf, int size, int mode);
int                     mg_db_connect_init            (MGSRV *p_srv, int chndle);
int                     mg_db_ayt                     (MGSRV *p_srv, int chndle);
int                     mg_db_get_last_error          (int context);

int                     mg_request_header             (MGSRV *p_srv, MGBUF *p_buf, char *command, char *product);
int                     mg_request_add                (MGSRV *p_srv, int chndle, MGBUF *p_buf, unsigned char *element, int size, short byref, short type);

int                     mg_encode_size64              (int n10);
int                     mg_decode_size64              (int nxx);
int                     mg_encode_size                (unsigned char *esize, int size, short base);
int                     mg_decode_size                (unsigned char *esize, int len, short base);
int                     mg_encode_item_header         (unsigned char * head, int size, short byref, short type);
int                     mg_decode_item_header         (unsigned char * head, int * size, short * byref, short * type);
int                     mg_get_error                  (MGSRV *p_srv, char *buffer);

int                     mg_extract_substrings         (MGSTR * records, char* buffer, int tsize, char delim, int offset, int no_tail, short type);
int                     mg_compare_keys               (MGSTR * key, MGSTR * rkey, int max);
int                     mg_replace_substrings         (char * tbuffer, char *fbuffer, char * replace, char * with);

int                     mg_bind_server_api            (MGSRV *p_srv, short context);
int                     mg_release_server_api         (MGSRV *p_srv, short context);
int                     mg_invoke_server_api          (MGSRV *p_srv, int chndle, MGBUF *p_buf, int size, int mode);

#endif


