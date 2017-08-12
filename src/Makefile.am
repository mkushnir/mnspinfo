AUTOMAKE_OPTIONS = foreign subdir-objects
AM_MAKEFLAGS = -s

diags = diag.txt
BUILT_SOURCES = diag.c diag.h
EXTRA_DIST = $(diags)
CLEANFILES = $(BUILT_SOURCES) *.core core
#CLEANFILES += *.in

lib_LTLIBRARIES = libspinfo.la

if FREEBSD
plat_sources = spinfo_freebsd.c
plat_headers = spinfo_freebsd.h
plat_libs = -lprocstat -lkvm
endif

if LINUX
plat_sources = spinfo_linux.c linux_procutils.c
plat_headers = spinfo_linux.h
plat_libs =
endif

nobase_include_HEADERS = spinfo.h

noinst_HEADERS = $(plat_headers) spinfo_private.h


libspinfo_la_SOURCES = spinfo.c spinfo_posix.c $(plat_sources)
nodist_libspinfo_la_SOURCES = diag.c

if DEBUG
DEBUG_FLAGS = -g -O0 @CC_DEBUG@
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

libspinfo_la_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @MRKCOMMON_LOCAL_CFLAGS@ @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)

libspinfo_la_LDFLAGS = -version-info 0:0:0 -L$(libdir) -lmrkcommon -lmndiag @MRKCOMMON_LOCAL_LDFLAGS@ $(plat_libs)

SUBDIRS = .

diag.c diag.h: $(diags)
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -S diag.txt.tmp -L spinfo -H diag.h -C diag.c *.[ch]

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun && cd ..; fi; done;
