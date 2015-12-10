AUTOMAKE_OPTIONS = foreign subdir-objects
CLEANFILES = *.core
#CLEANFILES += *.in
AM_MAKEFLAGS = -s

noinst_PROGRAMS=testfoo testrusage

distdir = $(top_srcdir)/$(PACKAGE)-$(VERSION)/test
dist_HEADERS = unittest.h

BUILT_SOURCES = ../src/diag.c ../src/diag.h

noinst_HEADERS = ../src/spinfo.h

if FREEBSD
plat_libs = -lprocstat -lkvm
endif

if LINUX
plat_libs =
noinst_PROGRAMS += testparse
endif

if DEBUG
DEBUG_FLAGS = -g -O0 @CC_DEBUG@
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

nodist_testfoo_SOURCES = ../src/diag.c
testfoo_SOURCES = testfoo.c
testfoo_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testfoo_LDFLAGS = -L$(libdir) -lmrkcommon -L$(top_srcdir)/src/.libs -lspinfo $(plat_libs)

nodist_testrusage_SOURCES = ../src/diag.c
testrusage_SOURCES = testrusage.c
testrusage_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testrusage_LDFLAGS = -L$(libdir) -lmrkcommon -L$(top_srcdir)/src/.libs -lspinfo $(plat_libs)

if LINUX
nodist_testparse_SOURCES = ../src/diag.c
testparse_SOURCES = testparse.c
testparse_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testparse_LDFLAGS = -L$(libdir) -lmrkcommon -L$(top_srcdir)/src/.libs -lspinfo $(plat_libs)
endif

../src/diag.c ../src/diag.h: ../src/diag.txt
	 $(AM_V_GEN) /bin/sh ./gen-diag spinfo

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
