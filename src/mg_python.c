/*
   ----------------------------------------------------------------------------
   | mg_python: Python Extension for M/Cache/IRIS                             |
   | Author: Chris Munt cmunt@mgateway.com                                    |
   |                    chris.e.munt@gmail.com                                |
   | Copyright (c) 2016-2020 M/Gateway Developments Ltd,                      |
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

Version 2.2.45 17 January 2020:
   Introduce option to connecto the database via its C API
   - mg_python.m_bind_server_api(0, dbtype, path, username, password, envvars, params)

*/

/*
   Building
   ========

   Use the following method:

   1) Build a shared object from first principles:

      Linux
      -----
      gcc -c -fpic -DLINUX -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      gcc -shared -rdynamic -o mg_python.so mg_python.o

      FreeBSD
      -------
      cc -c -DFREEBSD -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      ld -G -o mg_python.so mg_python.o

      AIX
      ---
      xlc_r -c -DAIX -I<path_to_python_dir> -I<path_to_python_dir>/Includ -o mg_python.o mg_python.c
      xlc_r -G -H512 -T512 -bM:SRE mg_python.o -berok -bexpall -bnoentry -o mg_python.so

      Mac OS X
      --------
      gcc -c -fPIC -fno-common -DMACOSX -D_NOTHREADS -DDARWIN -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      gcc -bundle -flat_namespace -undefined suppress mg_python.o -o mg_python.so

      HPUX64
      ------
      cc -c -DHPUX11 -DNET_SSL -D_HPUX_SOURCE -Ae +DA2.0W +z -DMCC_HTTPD -DSPAPI20 -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      ld -b  mg_python.o -o mg_python.so

      Dec UNIX
      --------
      cc -c -DOSF1 -std0 -w -pthread -DIS_64 -ieee_with_inexact -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      ld -all -shared -expect_unresolved "*" -taso mg_python.o -o mg_python.so

      Solaris SPARC32
      ---------------
      cc -c Xa -w -DSOLARIS -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      ld -G mg_python.o -o mg_python.so

      Solaris SPARC64
      ---------------
      cc -c -Xa -w -xarch=v9 -KPIC -DBIT64PLAT -DSOLARIS -I<path_to_python_dir> -I<path_to_python_dir>/Include -o mg_python.o mg_python.c
      ld -G mg_python.o -o mg_python.so
*/


#define MG_VERSION               "2.2.45"

#define MG_MAX_KEY               256
#define MG_MAX_PAGE              256
#define MG_MAX_VARGS             32

#define MG_T_VAR                 0
#define MG_T_STRING              1
#define MG_T_INTEGER             2
#define MG_T_FLOAT               3
#define MG_T_LIST                4

#define MG_PRODUCT               "p"

/*
#define MG_ES_DELIM              0
#define MG_ES_BLOCK              1
*/

/* include standard header */

#include <Python.h>

#define MG_DBA_EMBEDDED          1
#include "mg_dbasys.h"
#include "mg_dba.h"


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
   if (p_page && p_page->p_srv->mem_error == 1) { \
      mg_db_disconnect(p_page->p_srv, chndle, c); \
      PyErr_SetString(PyExc_RuntimeError, (char *) c); \
      return NULL; \
   } \

#define MG_FTRACE(e) \

typedef struct tagMGPTYPE {
   short type;
   short byref;
} MGPTYPE, *LPMGPTYPE;


static int le_mg_user;


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
   MGSRV       srv;
   MGSRV *     p_srv;
} MGPAGE, *LPMGPAGE;


static MGPAGE gpage;
static MGPAGE *tp_page[MG_MAX_PAGE] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static long request_no           = 0;
static char minit[256]           = {'\0'};
static char *mg_empty_string     = "";


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
MGPAGE *                mg_ppage                   (int phndle);
int                     mg_ppage_init              (MGPAGE * p_page);



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
         tp_page[n] = (MGPAGE *) mg_malloc(sizeof(MGPAGE), 0);
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
      mg_free((void *) tp_page[phndle], 0);
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
      p_page->p_srv->storage_mode = smode;
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
      strcpy(p_page->p_srv->ip_address, netname);
      p_page->p_srv->port = port;
      if (username) {
         strcpy(p_page->p_srv->username, username);
      }
      if (password) {
         strcpy(p_page->p_srv->password, password);
      }
      p_page->p_srv->mode = 1;
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
      strcpy(p_page->p_srv->uci, uci);
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
      strcpy(p_page->p_srv->server, server);
      result = 1;
   }

   return Py_BuildValue("i", result);
}


static PyObject * ex_m_bind_server_api(PyObject *self, PyObject *args)
{
   int result, phndle;
   char *dbtype_name, *path, *username, *password, *env, *params;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "issssss", &phndle, &dbtype_name, &path, &username, &password, &env, &params))
      return NULL;

   result = 0;
   p_page = mg_ppage(phndle);

   strcpy(p_page->p_srv->dbtype_name, dbtype_name);
   strcpy(p_page->p_srv->shdir, path);
   strcpy(p_page->p_srv->username, password);
   strcpy(p_page->p_srv->password, password);

   p_page->p_srv->p_env = (MGBUF *) mg_malloc(sizeof(MGBUF), 0);
   mg_buf_init(p_page->p_srv->p_env, MG_BUFSIZE, MG_BUFSIZE);
   mg_buf_cpy(p_page->p_srv->p_env, env, (int) strlen(env));

   result = mg_bind_server_api(p_page->p_srv, 0);

   if (!result) {
      if (!strlen(p_page->p_srv->error_mess)) {
         strcpy(p_page->p_srv->error_mess, "The server API is not available on this host");
      }
      MG_ERROR(p_page->p_srv->error_mess);
      return NULL;
   }

   return Py_BuildValue("i", result);

}


static PyObject * ex_m_release_server_api(PyObject *self, PyObject *args)
{
   int result, phndle;
   MGPAGE *p_page;

   if (!PyArg_ParseTuple(args, "i", &phndle))
      return NULL;

   result = 0;
   p_page = mg_ppage(phndle);

   result = mg_release_server_api(p_page->p_srv, 0);

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
      error = p_page->p_srv->error_mess;
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);

   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "S", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "S", MG_PRODUCT);

   max = mg_get_keys(key, nkey, py_nkey, NULL);
   data = mg_get_string(py_data, &p, &data_len);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }
   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) data, data_len, (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);


   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "G", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "G", MG_PRODUCT);
   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);


   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "K", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "K", MG_PRODUCT);

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "D", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "D", MG_PRODUCT);

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "O", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "O", MG_PRODUCT);

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "P", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "P", MG_PRODUCT);

   max = mg_get_keys(key, nkey, py_nkey, NULL);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "M", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   ifc[0] = 0;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   n = 0;
   for (rn = 0; rn < mrec; rn ++) {
      p = PyList_GetItem(records, rn);
      ps = mg_get_string(p, &temp, &len);

      if (rn == 0) {
         ifc[0] = 0;
         ifc[1] = MG_TX_AREC_FORMATTED;
      }
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) ps, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[0] = 0;
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) options, (int) strlen((char *) options), (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "m", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) global, (int) strlen((char *) global), (short) ifc[0], (short) ifc[1]);

   for (n = 1; n <= max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) nkey[n].ps, nkey[n].size, (short) ifc[0], (short) ifc[1]);
   }

   anybyref = 1;

   ifc[0] = 1;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);
   ifc[0] = 0;
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) options, (int) strlen((char *) options), (short) ifc[0], (short) ifc[1]);

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "X", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "X", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) fun, (int) strlen((char *) fun), (short) ifc[0], (short) ifc[1]);

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
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);
            ifc[1] = MG_TX_DATA;
            mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

#if 0
{
   mg_db_disconnect(p_page->p_srv, chndle, 1);
   return MG_MAKE_PYSTRINGN(p_buf->p_buffer, (int) strlen(p_buf->p_buffer));
   /* return MG_MAKE_PYSTRINGN(buffer, (int) strlen(buffer)); */
}
#endif

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "x", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.global, (int) strlen((char *) vargs.global), (short) ifc[0], (short) ifc[1]);

   for (n = 0; n < max; n ++) {
      ifc[0] = 0;
      ifc[1] = MG_TX_DATA;
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) vargs.cvars[n].ps, vargs.cvars[n].size, (short) ifc[0], (short) ifc[1]);
   }

   MG_MEMCHECK("Insufficient memory to process request", 1);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);

   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "x", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) cclass, (int) strlen((char *) cclass), (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) cmethod, (int) strlen((char *) cmethod), (short) ifc[0], (short) ifc[1]);

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
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "H", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) fun, (int) strlen((char *) fun), (short) ifc[0], (short) ifc[1]);

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
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "y", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) cclass, (int) strlen((char *) cclass), (short) ifc[0], (short) ifc[1]);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) cmethod, (int) strlen((char *) cmethod), (short) ifc[0], (short) ifc[1]);

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
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

            ifc[1] = MG_TX_AREC_FORMATTED;

            max = (int) PyList_Size(pstr);
            for (n = 0; n < max; n ++) {
               a = PyList_GetItem(pstr, n);
               str = mg_get_string(a, &p, &len);
               mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
            }

            ifc[1] = MG_TX_EOD;
            mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

         }
         else {
            str = mg_get_string(pstr, &p, &n);

            ifc[1] = MG_TX_DATA;

            mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);
         }
      }
   }

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "h", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[1] = MG_TX_AREC_FORMATTED;

   max = (int) PyList_Size(py_cgi);

   for (n = 0; n < max; n ++) {
      a = PyList_GetItem(py_cgi, n);
      str = mg_get_string(a, &p, &len);
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   str = mg_get_string(py_content, &p, &n);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, (int) strlen((char *) str), (short) ifc[0], (short) ifc[1]);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
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

   n = mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   if (n < 1) {
      strcpy((char *) p_buf->p_buffer, "");
      mg_db_disconnect(p_page->p_srv, chndle, 0);
      output = Py_BuildValue("s", p_buf->p_buffer);
      mg_buf_free(p_buf);
      return output;
   }

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

   n = mg_db_connect(p_page->p_srv, &chndle, 1);
   if (!n) {
      MG_ERROR(p_page->p_srv->error_mess);
      mg_buf_free(p_buf);
      return NULL;
   }

   mg_request_header(p_page->p_srv, p_buf, "X", MG_PRODUCT);

   ifc[0] = 0;
   ifc[1] = MG_TX_DATA;
   strcpy(buffer, "sort^%ZMGS");
   mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) buffer, (int) strlen((char *) buffer), (short) ifc[0], (short) ifc[1]);


   ifc[0] = 1;
   ifc[1] = MG_TX_AREC;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   ifc[1] = MG_TX_AREC_FORMATTED;

   max = (int) PyList_Size(records);
   for (n = 0; n < max; n ++) {
      a = PyList_GetItem(records, n);
      str = mg_get_string(a, &p, &len);
      mg_request_add(p_page->p_srv, chndle, p_buf, (unsigned char *) str, len, (short) ifc[0], (short) ifc[1]);
   }
   ifc[1] = MG_TX_EOD;
   mg_request_add(p_page->p_srv, chndle, p_buf, NULL, 0, (short) ifc[0], (short) ifc[1]);

   mg_db_send(p_page->p_srv, chndle, p_buf, 1);
   mg_db_receive(p_page->p_srv, chndle, p_buf, MG_BUFSIZE, 0);

   MG_MEMCHECK("Insufficient memory to process response", 0);

   mg_db_disconnect(p_page->p_srv, chndle, 1);

   if ((n = mg_get_error(p_page->p_srv, (char *) p_buf->p_buffer))) {
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

	{"m_bind_server_api", ex_m_bind_server_api, METH_VARARGS, "m_bind_server_api() doc string"},
	{"m_release_server_api", ex_m_release_server_api, METH_VARARGS, "m_release_server_api() doc string"},

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

   dbx_init();

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

   p_page->p_srv = &(p_page->srv);
   p_page->p_srv->mem_error = 0;
   p_page->p_srv->mode = 0;
   p_page->p_srv->storage_mode = 0;
   p_page->p_srv->timeout = 0;
   strcpy(p_page->p_srv->server, "");
   strcpy(p_page->p_srv->uci, "");

   strcpy(p_page->p_srv->ip_address, MG_HOST);
   p_page->p_srv->port = MG_PORT;
   strcpy(p_page->p_srv->uci, MG_UCI);

   strcpy(p_page->p_srv->username, "");
   strcpy(p_page->p_srv->password, "");

   for (n = 0; n < MG_MAXCON; n ++) {
      p_page->p_srv->pcon[n] = NULL;
   }

   return 1;
}

