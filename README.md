# mg_python

A Python Extension for InterSystems **Cache/IRIS** and **YottaDB**.

Chris Munt <cmunt@mgateway.com>  
14 March 2021, M/Gateway Developments Ltd [http://www.mgateway.com](http://www.mgateway.com)

* Current Release: Version: 2.3; Revision 47.
* Two connectivity models to the InterSystems or YottaDB database are provided: High performance via the local database API or network based.
* [Release Notes](#RelNotes) can be found at the end of this document.

Contents

* [Overview](#Overview") 
* [Pre-requisites](#PreReq") 
* [Installing mg\_python](#Install)
* [Using mg\_python](#Using)
* [Connecting to the database](#Connect)
* [Invocation of database commands](#DBCommands)
* [Invocation of database functions](#DBFunctions)
* [Transaction Processing](#TProcessing)
* [Direct access to InterSystems classes (IRIS and Cache)](#DBClasses)
* [License](#License)


## <a name="Overview"></a> Overview

**mg_python** is an Open Source Python extension developed for InterSystems **Cache/IRIS** and the **YottaDB** database.  It will also work with the **GT.M** database and other **M-like** databases.


## <a name="PreReq"></a> Pre-requisites 

Python installation:

       http://www.python.org/

InterSystems **Cache/IRIS** or **YottaDB** (or similar M database):

       https://www.intersystems.com/
       https://yottadb.com/


## <a name="Install"></a> Installing mg\_python

There are three parts to **mg_python** installation and configuration.

* The Python extension (**mg_python.pyd**).
* The DB Superserver: the **%zmgsi** routines.
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


### Installing the DB Superserver

The DB Superserver is required for:

* Network based access to databases.

Two M routines need to be installed (%zmgsi and %zmgsis).  These can be found in the *Service Integration Gateway* (**mgsi**) GitHub source code repository ([https://github.com/chrisemunt/mgsi](https://github.com/chrisemunt/mgsi)).  Note that it is not necessary to install the whole *Service Integration Gateway*, just the two M routines held in that repository.

#### Installation for InterSystems Cache/IRIS

Log in to the %SYS Namespace and install the **zmgsi** routines held in **/isc/zmgsi\_isc.ro**.

       do $system.OBJ.Load("/isc/zmgsi_isc.ro","ck")

Change to your development Namespace and check the installation:

       do ^%zmgsi

       M/Gateway Developments Ltd - Service Integration Gateway
       Version: 4.0; Revision 16 (11 February 2021)


#### Installation for YottaDB

The instructions given here assume a standard 'out of the box' installation of **YottaDB** (version 1.30) deployed in the following location:

       /usr/local/lib/yottadb/r130

The primary default location for routines:

       /root/.yottadb/r1.30_x86_64/r

Copy all the routines (i.e. all files with an 'm' extension) held in the GitHub **/yottadb** directory to:

       /root/.yottadb/r1.30_x86_64/r

Change directory to the following location and start a **YottaDB** command shell:

       cd /usr/local/lib/yottadb/r130
       ./ydb

Link all the **zmgsi** routines and check the installation:

       do ylink^%zmgsi

       do ^%zmgsi

       M/Gateway Developments Ltd - Service Integration Gateway
       Version: 4.0; Revision 16 (11 February 2021)

Note that the version of **zmgsi** is successfully displayed.

Finally, add the following lines to the interface file (**zmgsi.ci** in the example used in the db.open() method).

       sqlemg: ydb_string_t * sqlemg^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t *)
       sqlrow: ydb_string_t * sqlrow^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t *)
       sqldel: ydb_string_t * sqldel^%zmgsis(I:ydb_string_t*, I:ydb_string_t *)
       ifc_zmgsis: ydb_string_t * ifc^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t*)

A copy of this file can be downloaded from the **/unix** directory of the  **mgsi** GitHub repository [here](https://github.com/chrisemunt/mgsi)


### Starting the DB Superserver

The default TCP server port for **zmgsi** is **7041**.  If you wish to use an alternative port then modify the following instructions accordingly.

* For InterSystems DB servers the concurrent TCP service should be started in the **%SYS** Namespace.

Start the DB Superserver using the following command:

       do start^%zmgsi(0) 

To use a server TCP port other than 7041, specify it in the start-up command (as opposed to using zero to indicate the default port of 7041).

* For YottaDB, as an alternative to starting the DB Superserver from the command prompt, Superserver processes can be started via the **xinetd** daemon.  Instructions for configuring this option can be found in the **mgsi** repository [here](https://github.com/chrisemunt/mgsi)

Python code using the **mg\_python** functions will, by default, expect the database server to be listening on port **7041** of the local server (localhost).  However, **mg\_python** provides the functionality to modify these default settings at run-time.  It is not necessary for the Python installation to reside on the same host as the database server.


### Resources used by the DB Superserver (%zmgsi)

The **zmgsi** server-side code will write to the following global:

* **^zmgsi**: The event Log. 


## <a name="Using"></a> Using mg\_python

Python programs may refer to, and load, the **mg_python** module using the following directive at the top of the script.

       import mg_python

Having added this line, all methods listed provided by the module can be invoked using the following syntax.

       mg_python.<method>

Alternatively, an alias can be assigned to the module name.  For example:

       import mg_python as <alias>

Then methods can be invoked as:

       <alias>.<method>


## <a name="Connect"></a> Connecting to the database

By default, **mg_python** will connect to the server over TCP - the default parameters for which being the database listening locally on port **7041**. This can be modified using the following function.

       mg_python.m_set_host(<dbhandle>, <netname>, <port>, <username>, <password>)

The first argument refers the server handle; zero being the default handle representing the default server (localhost listening on TCP port 7041).

Example:

       mg_python.m_set_host(0, "localhost", 7041, "", "")

### Connecting to the database via its API.

As an alternative to connecting to the database using TCP based connectivity, **mg_python** provides the option of high-performance embedded access to a local installation of the database via its API.

#### InterSystems Caché or IRIS.

Use the following functions to bind to the database API.

       mg_python.m_set_uci(<dbhandle>, <namespace>)
       mg_python.m_bind_server_api(<dbhandle>, <dbtype>, <path>, <username>, <password>, <envvars>, <params>)

Where:

* dbhandle: Current server handle.
* namespace: Namespace.
* dbtype: Database type ('Cache' or 'IRIS').
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

#### YottaDB

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

This example assumes that the YottaDB installation is in: **/usr/local/lib/yottadb/r130**. 
This is where the **libyottadb.so** library is found.
Also, in this directory, as indicated in the environment variables, the YottaDB routine interface file resides (**zmgsi.ci** in this example).  The interface file must contain the following lines:

       sqlemg: ydb_string_t * sqlemg^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t *)
       sqlrow: ydb_string_t * sqlrow^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t *)
       sqldel: ydb_string_t * sqldel^%zmgsis(I:ydb_string_t*, I:ydb_string_t *)
       ifc_zmgsis: ydb_string_t * ifc^%zmgsis(I:ydb_string_t*, I:ydb_string_t *, I:ydb_string_t*)

Moving on to the Python code for binding to the YottaDB database.  Modify the values of these environment variables in accordance with your own YottaDB installation.  Note that each line is terminated with a linefeed character, with a double linefeed at the end of the list.

       envvars = "";
       envvars = envvars + "ydb_dir=/root/.yottadb\n"
       envvars = envvars + "ydb_rel=r1.30_x86_64\n"
       envvars = envvars + "ydb_gbldir=/root/.yottadb/r1.30_x86_64/g/yottadb.gld\n"
       envvars = envvars + "ydb_routines=/root/.yottadb/r1.30_x86_64/o*(/root/.yottadb/r1.30_x86_64/r root/.yottadb/r) /usr/local/lib/yottadb/r130/libyottadbutil.so\n"
       envvars = envvars + "ydb_ci=/usr/local/lib/yottadb/r130/zmgsi.ci\n"
       envvars = envvars + "\n"

       result = mg_python.m_bind_server_api(0, "YottaDB", "/usr/local/lib/yottadb/r130", "", "", envvars, "")

The bind function will return '1' for success and '0' for failure.

Before leaving your Python application, it is good practice to gracefully release the binding to the database:

       mg_python.m_release_server_api(<dbhandle>)

Example:

       mg_python.m_release_server_api(0)


## <a name="DBCommands"></a> Invocation of database commands

Before invoking database functionality,the following simple script can be used to check that **mg_python** is successfully installed.

       print(m_python.m_ext_version())

This should return something like:

       M/Gateway Developments Ltd. - mg_python: Python Gateway to M - Version 2.3.46

Now consider the following database script:

       Set ^Person(1)="Chris Munt"
       Set name=$Get(^Person(1))

Equivalent Python code:

       mg_python.m_set(0, "^Person", 1, "Chris Munt")
       name = mg_python.m_get(0, "^Person", 1);

In the above examples, the first argument refers the server handle; zero being the default handle representing the default server (localhost listening on TCP port 7041).

**mg_python** provides functions to invoke all database commands and functions.


### Set a record

       result = mg_python.m_set(<dbhandle>, <global>, <key>, <data>)
      
Example:

       result = mg_python.m_set(0, "^Person", 1, "Chris Munt")

### Get a record

       result = mg_python.m_get(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_get(0, "^Person", 1)

### Delete a record

       result = mg_python.m_delete(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_delete(0, "^Person", 1)


### Check whether a record is defined

       result = mg_python.m_defined(<dbhandle>, <global>, <key>)
      
Example:

       result = mg_python.m_defined(0, "^Person", 1)


### Parse a set of records (in order)

       result = mg_python.m_order(<dbhandle>, <global>, <key>)
      
Example:

       key = mg_python.m_order(0, "^Person", "")
       while (key != ""):
          print(key, " = ", mg_python.m_get(0, "^Person", key))
          key  = mg_python.m_order(0, "^Person", key)


### Parse a set of records (in reverse order)

       result = mg_python.m_previous(<dbhandle>, <global>, <key>)
      
Example:

       key = mg_python.m_previous(0, "^Person", "")
       while (key != ""):
          print(key, " = ", mg_python.m_get(0, "^Person", key))
          key  = mg_python.m_previous(0, "^Person", key)

### Increment the value of a global node

       result = mg_python.m_increment(<dbhandle>, <global>, <key>, <increment_value>)
      
Example:

       result = mg_python.m_increment(0, "^Global", "counter", 1)

This will increment the value of global node ^Global("counter"), by 1 and return the new value.
 

## <a name="DBFunctions"> Invocation of database functions

       result = mg_python.m_function(<dbhandle>, <function>, <parameters>)
      
Example:

M routine called 'math':

       add(a, b) ; Add two numbers together
                 quit (a+b)

Python invocation:

      result = mg_python.m_function(0, "add^math", 2, 3)


## <a name="TProcessing"></a> Transaction Processing

M DB Servers implement Transaction Processing by means of the methods described in this section.

### Start a Transaction

       result = mg_python.m_tstart(<dbhandle>)

* On successful completion this method will return zero, or an error code on failure.

Example:

       result = mg_python.m_tstart(0)


### Determine the Transaction Level

       result = mg_python.m_tlevel(<dbhandle>)

* Transactions can be nested and this method will return the level of nesting.  If no Transaction is active this method will return zero.  Otherwise a positive integer will be returned to represent the current depth of Transaction nesting.

Example:

       tlevel = mg_python.m_tlevel(0)


### Commit a Transaction

       result = mg_python.m_tcommit(<dbhandle>)

* On successful completion this method will return zero, or an error code on failure.

Example:

       result = mg_python.m_tcommit(0)


### Rollback a Transaction

       result = mg_python.m_trollback(<dbhandle>)

* On successful completion this method will return zero, or an error code on failure.

Example:

       result = mg_python.m_trollback(0)


## <a name="DBClasses"> Direct access to InterSystems classes (IRIS and Cache)

### Invocation of a ClassMethod

       result = mg_python.m_classmethod(<dbhandle>, <class_name>, <classmethod_name>, <parameters>)
      
Example (Encode a date to internal storage format):

       result = mg_python.m_classmethod("%Library.Date", "DisplayToLogical", "10/10/2019")


## <a name="License"></a> License

Copyright (c) 2018-2021 M/Gateway Developments Ltd,
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

### v2.2.45a (20 January 2021)

* Restructure and update the documentation.

### v2.3.46 (15 February 2021)

* Introduce support for M transaction processing: tstart, $tlevel, tcommit, trollback.
	* Available with DB Superserver v4 and later. 
* Introduce support for the M increment function.
* Allow the DB server response timeout to be modified via the mg\_python.m\_set\_timeout() function.
	* mg_python.m\_set\_timeout([dbhandle],[timeout])

### v2.3.47 (14 March 2021)

* Introduce support for YottaDB Transaction Processing over API based connectivity.
	* This functionality was previously only available over network-based connectivity to YottaDB.
