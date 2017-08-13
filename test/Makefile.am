AUTOMAKE_OPTIONS = foreign subdir-objects
CLEANFILES = *.core core
#CLEANFILES += *.in
AM_MAKEFLAGS = -s

noinst_PROGRAMS=testfoo testrusage

diags = ../src/diag.txt
BUILT_SOURCES = diag.c diag.h

noinst_HEADERS = ../src/spinfo.h unittest.h


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

nodist_testfoo_SOURCES = diag.c
testfoo_SOURCES = testfoo.c
testfoo_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testfoo_LDFLAGS = -L$(libdir) -lmrkcommon -L$(top_srcdir)/src/.libs -lspinfo $(plat_libs)

nodist_testrusage_SOURCES = diag.c
testrusage_SOURCES = testrusage.c
testrusage_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testrusage_LDFLAGS = -L$(libdir) -lmrkcommon -L$(top_srcdir)/src/.libs -lspinfo $(plat_libs)

if LINUX
nodist_testparse_SOURCES = diag.c
testparse_SOURCES = testparse.c
testparse_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testparse_LDFLAGS = -L$(top_srcdir)/src/.libs -lspinfo -L$(libdir) -lmrkcommon -lmndiag $(plat_libs)
endif

diag.c diag.h: $(diags)
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -v -S diag.txt.tmp -L spinfo -H diag.h -C diag.c ../*.[ch] ./*.[ch]

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
