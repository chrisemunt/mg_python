# UNIX makefile for mg_python module (mg_python.pyd)
#
# Inputs:
#    PYTHONVER: Python version (Example: 2.7)
#    PYTHONINC: Location of Python.h (Example: /usr/include/python2.7/)
#    PYTHONMOD: Location for mg_python.pyd (Example: /usr/local/lib/python2.7/site-packages/)
#
# Commands for Linux:
#    Build mg_python module   : make PYTHONVER=2.7 PYTHONINC=/usr/include/python2.7/ PYTHONMOD=/usr/local/lib/python2.7/site-packages/
#    Install mg_python module : make PYTHONVER=2.7 PYTHONINC=/usr/include/python2.7/ PYTHONMOD=/usr/local/lib/python2.7/site-packages/ install
#    Clean mg_python build    : make clean
#
# Commands for Mac OSX:
#    Build mg_python module   : make PYTHONVER=2.7 PYTHONINC=/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
                                                   PYTHONMOD=/Library/Python/2.7/site-packages
#    Install mg_python module : make PYTHONVER=2.7 PYTHONINC=/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
                                                   PYTHONMOD=/Library/Python/2.7/site-packages install
#    Clean mg_python build    : make clean
#

# PYTHONVER=2.7
# PYTHONINC=/usr/include/python2.7/
# PYTHONMOD=/usr/local/lib/python2.7/site-packages/

ifndef PYTHONVER
   PYTHONVER=2.7
endif
ifndef PYTHONINC
   PYTHONINC=/usr/include/python$(PYTHONVER)/
endif
ifndef PYTHONMOD
   PYTHONMOD=/usr/local/lib/python$(PYTHONVER)/site-packages/
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
   PYTHONOS = Linux
   CC = cc
   CCFLAGS += -fpic -DLINUX
   LINK += cc
   LINKFLAGS += -shared -rdynamic
endif
ifeq ($(UNAME_S),Darwin)
   PYTHONOS = OSX
   CC = cc
   CCFLAGS += -fPIC -fno-common -DMACOSX -D_NOTHREADS -DDARWIN
   LINK += cc
   LINKFLAGS += -bundle -flat_namespace -undefined suppress
endif
ifeq ($(UNAME_S),FreeBSD)
   PYTHONOS = FreeBSD
   CC = cc
   CCFLAGS += -DFREEBSD
   LINK = ld
   LINKFLAGS += -G
endif
ifeq ($(UNAME_S),AIX)
   PYTHONOS = AIX
   CC = xlc_r
   CCFLAGS += -DAIX
   LINK = xlc_r
   LINKFLAGS += -G -H512 -T512 -bM:SRE -berok -bexpall -bnoentry
endif

ifndef PYTHONOS
   $(error Cannot determine the OS type)
endif

# Build mg_python module
all : app

# Link the object file and dependent libraries into a binary
app : mg_python.o
	$(LINK) -o mg_python.pyd $(LINKFLAGS) mg_python.o

# Compile the source file into object file
mg_python.o : mg_python.c
	$(CC) -o mg_python.o $(CCFLAGS) -I$(PYTHONINC) -c mg_python.c

# Install mg_python module
install :
	cp mg_python.pyd $(PYTHONMOD)

# Clean target
clean :
	rm -rf mg_python.pyd mg_python.o



