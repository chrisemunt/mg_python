# mg_python

A Python Extension for InterSystems **Cache/IRIS** and **YottaDB**.

Chris Munt <cmunt@mgateway.com>  
17 January 2020, M/Gateway Developments Ltd [http://www.mgateway.com](http://www.mgateway.com)

* Current Release: Version: 2.2; Revision 45 - Beta (17 January 2020)
* [Release Notes](#RelNotes) can be found at the end of this document.


## Overview

**mg_python** is an Open Source Python extension developed for InterSystems **Cache/IRIS** and the **YottaDB** database.  It will also work with the **GT.M** database and other **M-like** databases.


## Pre-requisites

Python installation:

       http://www.python.org/

InterSystems **Cache/IRIS** or **YottaDB** (or similar M database):

       https://www.intersystems.com/
       https://yottadb.com/

## Installing mg_python

There are three parts to **mg_python** installation and configuration.

* The Python extension (**mg_python.pyd**).
* The database (or server) side code: **zmgsi**
* A network configuration to bind the former two elements together.

### Building the mg_python extension

**mg_python** is written in standard C.  For Linux systems, the Python installation procedure can use the freely available GNU C compiler (gcc) which can be installed as follows.

Ubuntu:

       apt-get install gcc

Red Hat and CentOS:

       yum install gcc

Apple OS X can use the freely available **Xcode** development environment.

There are two options for Windows, both of which are free:

* Microsoft Visual Studio Community: [https://www.visualstudio.com/vs/community/](https://www.visualstudio.com/vs/community/)
* MinGW: [http://www.mingw.org/](http://www.mingw.org/)

There are built Windows x64 binaries available from:

* [https://github.com/chrisemunt/mg_python/blob/master/bin/winx64](https://github.com/chrisemunt/mg_python/blob/master/bin/winx64)

Having created a suitable development environment, the Python Extension installer can be used to build and deploy **mg_python**.  You will find the setup scripts in the /src directory of the distribution.

UNIX:

       python setup.py install

Windows:

       python setup_win.py install


### InterSystems Cache/IRIS

Log in to the Manager UCI and install the **zmgsi** routines held in either **/m/zmgsi\_cache.xml** or **/m/zmgsi\_iris.xml** as appropriate.

       do $system.OBJ.Load("/m/zmgsi_cache.xml","ck")

Change to your development UCI and check the installation:

       do ^%zmgsi

       M/Gateway Developments Ltd - Service Integration Gateway
       Version: 3.2; Revision 5 (17 January 2020)

### YottaDB

The instructions given here assume a standard 'out of the box' installation of **YottaDB** deployed in the following location:

       /usr/local/lib/yottadb/r122

The primary default location for routines:

       /root/.yottadb/r1.22_x86_64/r

Copy all the routines (i.e. all files with an 'm' extension) held in the GitHub **/yottadb** directory to:

       /root/.yottadb/r1.22_x86_64/r

Change directory to the following location and start a **YottaDB** command shell:

       cd /usr/local/lib/yottadb/r122
       ./ydb

Link all the **zmgsi** routines and check the installation:

       do ylink^%zmgsi

       do ^%zmgsi

       M/Gateway Developments Ltd - Service Integration Gateway
       Version: 3.2; Revision 5 (17 January 2020)


Note that the version of **zmgsi** is successfully displayed.


## Setting up the network service (if required)

The network setup described here is only required if TCP based connectivity is to be used to connect your Python code to the database, as opposed to the API based approach described later.

The default TCP server port for **zmgsi** is **7041**.  If you wish to use an alternative port then modify the following instructions accordingly.

Python code using the **mg_python** methods will, by default, expect the database server to be listening on port **7041** of the local server (localhost).  However, **mg_python** provides the functionality to modify these default settings at run-time.  It is not necessary for the web server/Python installation to reside on the same host as the database server.

### InterSystems Cache/IRIS

Start the Cache/IRIS-hosted concurrent TCP service in the Manager UCI:

       do start^%zmgsi(0) 

To use a server TCP port other than 7041, specify it in the start-up command (as opposed to using zero to indicate the default port of 7041).

### YottaDB

Network connectivity to **YottaDB** is managed via the **xinetd** service.  First create the following launch script (called **zmgsi_ydb** here):

       /usr/local/lib/yottadb/r122/zmgsi_ydb

Content:

       #!/bin/bash
       cd /usr/local/lib/yottadb/r122
       export ydb_dir=/root/.yottadb
       export ydb_dist=/usr/local/lib/yottadb/r122
       export ydb_routines="/root/.yottadb/r1.22_x86_64/o*(/root/.yottadb/r1.22_x86_64/r /root/.yottadb/r) /usr/local/lib/yottadb/r122/libyottadbutil.so"
       export ydb_gbldir="/root/.yottadb/r1.22_x86_64/g/yottadb.gld"
       $ydb_dist/ydb -r xinetd^%zmgsis

Create the **xinetd** script (called **zmgsi_xinetd** here): 

       /etc/xinetd.d/zmgsi_xinetd

Content:

       service zmgsi_xinetd
       {
            disable         = no
            type            = UNLISTED
            port            = 7041
            socket_type     = stream
            wait            = no
            user            = root
            server          = /usr/local/lib/yottadb/r122/zmgsi_ydb
       }

* Note: sample copies of **zmgsi_xinetd** and **zmgsi_ydb** are included in the **/unix** directory.

Edit the services file:

       /etc/services

Add the following line to this file:

       zmgsi_xinetd          7041/tcp                        # zmgsi

Finally restart the **xinetd** service:

       /etc/init.d/xinetd restart

## Using mg_python

Python programs may refer to, and load, the **mg_python** module using the following directive at the top of the script.

       import mg_python

Having added this line, all methods listed provided by the module can be invoked using the following syntax.

       mg_python.<method>

Alternatively, an alias can be assigned to the module name.  For example:

       import mg_python as <alias>

Then methods can be invoked as:

       <alias>.<method>


### Connecting the database.

By default, **mg_python** will connect to the server over TCP - the default parameters for which being the database listening locally on port **7041**. This can be modified using the following function.

       mg_python.m_set_host(<dbhandle>, <netname>, <port>, <username>, <password>)

The first argument refers the server handle; zero being the default handle representing the default server (localhost listening on TCP port 7041).

Example:

       mg_python.m_set_host(0, "localhost", 7041, "", "")

#### Connecting to the database via its API.

As an alternative to connecting to the database using TCP based connectivity, **mg_python** provides the option of high-performance embedded access to a local installation of the database via its API.

##### InterSystems Caché or IRIS.

Use the following functions to bind to the database API.

       mg_python.m_set_uci(<dbhandle>, <namespace>)
       mg_python.m_bind_server_api(<dbhandle>, <dbtype>, <path>, <username>, <password>, <envvars>, <params>)

Where:

* dbhandle: Current server handle.
* namespace: Namespace.
* dbtype: Database type (‘Cache’ or ‘IRIS’).
* path: Path to database manager directory.
* username: Database username.
* password: Database password.
* envvars: List of required environment variables.
* params: Reserved for future use.

Example:

       mg_python.m_set_uci(0, "USER")
       result = mg_python.m_bind_server_api(0, "IRIS", "/usr/iris20191/mgr", "_SYSTEM", "SYS", "", "")

The bind function will return '1' for success and '0' for failure.

Before leaving your Python application, it is good practice to gracefully release the binding to the database:

       mg_python.m_release_server_api(<dbhandle>)

Example:

       mg_python.m_release_server_api(0)

##### YottaDB

Use the following function to bind to the database API.

       mg_python.m_bind_server_api(<dbhandle>, <dbtype>, <path>, <username>, <password>, <envvars>, <params>)

Where:

* dbhandle: Current server handle.
* dbtype: Database type (‘YottaDB’).
* path: Path to the YottaDB installation/library.
* username: Database username.
* password: Database password.
* envvars: List of required environment variables.
* params: Reserved for future use.

Example:

This example assumes that the YottaDB installation is in: **/usr/local/lib/yottadb/r122**. 
This is where the **libyottadb.so** library is found.
Also, in this directory, as indicated in the environment variables, the YottaDB routine interface file resides (**zmgsi.ci** in this example).  The interface file must contain the following line:

       ifc_zmgsis: ydb_char_t * ifc^%zmgsis(I:ydb_char_t*, I:ydb_char_t *, I:ydb_char_t*)

Moving on to the Python code for binding to the YottaDB database.  Modify the values of these environment variables in accordance with your own YottaDB installation.  Note that each line is terminated with a linefeed character, with a double linefeed at the end of the list.

       envvars = "";
       envvars = envvars + "ydb_dir=/root/.yottadb\n"
       envvars = envvars + "ydb_rel=r1.22_x86_64\n"
       envvars = envvars + "ydb_gbldir=/root/.yottadb/r1.22_x86_64/g/yottadb.gld\n"
       envvars =envvars + "ydb_routines=/root/.yottadb/r1.22_x86_64/o*(/root/.yottadb/r1.22_x86_64/r root/.yottadb/r) /usr/local/lib/yottadb/r122/libyottadbutil.so\n"
       envvars = envvars + "ydb_ci=/usr/local/lib/yottadb/r122/zmgsi.ci\n"
       envvars = envvars + "\n"

       result = mg_python.m_bind_server_api(0, "YottaDB", "/usr/local/lib/yottadb/r122", "", "", envvars, "")

The bind function will return '1' for success and '0' for failure.

Before leaving your Python application, it is good practice to gracefully release the binding to the database:

       mg_python.m_release_server_api(<dbhandle>)

Example:

       mg_python.m_release_server_api(0)



## Invoking database commands from Python script

Before invoking database functionality,the following simple script can be used to check that **mg_python** is successfully installed.

       print(m_python.m_ext_version())

This should return something like:

       M/Gateway Developments Ltd. - mg_python: Python Gateway to M - Version 3.0.1

Now consider the following database script:

       Set ^Person(1)="Chris Munt"
       Set name=$Get(^Person(1))

Equivalent Python code:

       mg_python.m_set(0, "^Person", 1, "Chris Munt")
       name = mg_python.m_get(0, "^Person", 1);

In the above examples, the first argument refers the server handle; zero being the default handle representing the default server (localhost listening on TCP port 7041).

**mg_python** provides functions to invoke all database commands and functions.


#### Set a record

       result = mg_python.m_set(<dbhandle>, <global>, <key>, <data>)
      
Example:

       result = mg_python.m_set(0, "^Person", 1, "Chris Munt")

#### Get a record

       result = mg_python.m_get(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_get(0, "^Person", 1)

#### Delete a record

       result = mg_python.m_delete(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_delete(0, "^Person", 1)


#### Check whether a record is defined

       result = mg_python.m_defined(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_defined(0, "^Person", 1)


#### Parse a set of records (in order)

       result = mg_python.m_order(<dbhandle>, <global>, <key>)
      
Example:

       key = mg_python.m_order(0, "^Person", "")
       while (key != ""):
          print(key, " = ", mg_python.m_get(0, "^Person", key))
          key  = mg_python.m_order(0, "^Person", key)


#### Parse a set of records (in reverse order)

       result = mg_python.m_previous(<dbhandle>, <global>, <key>)
      
Example:

       key = mg_python.m_previous(0, "^Person", "")
       while (key != ""):
          print(key, " = ", mg_python.m_get(0, "^Person", key))
          key  = mg_python.m_previous(0, "^Person", key)

## Invocation of database functions

       result = mg_python.m_function(<dbhandle>, <function>, <parameters>)
      
Example:

M routine called 'math':

       add(a, b) ; Add two numbers together
                 quit (a+b)

Python invocation:

      result = mg_python.m_function(0, "add^math", 2, 3);


## Direct access to InterSystems classes (IRIS and Cache)

#### Invocation of a ClassMethod

       result = mg_python.m_classmethod(<dbhandle>, <class_name>, <classmethod_name>, <parameters>);
      
Example (Encode a date to internal storage format):

        result = mg_python.m_classmethod("%Library.Date", "DisplayToLogical", "10/10/2019");

## Resources used by zmgsi

The **zmgsi** server-side code will write to the following global:

* **^zmgsi**: The event Log. 

## License

Copyright (c) 2018-2020 M/Gateway Developments Ltd,
Surrey UK.                                                      
All rights reserved.
 
http://www.mgateway.com                                                  
Email: cmunt@mgateway.com
 
 
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.      


## <a name="RelNotes"></a>Release Notes

### v2.1.44 (12 December 2019)

* Initial Release

### v2.2.45 (17 January 2020)

* Introduce the option to connect to a local installation of the database via its high-performance API.

