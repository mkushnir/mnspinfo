AUTOMAKE_OPTIONS = foreign subdir-objects

distdir = $(top_srcdir)/$(PACKAGE)-$(VERSION)/src
dist_HEADERS =

BUILT_SOURCES = diag.c diag.h
EXTRA_DIST = diag.txt gen-diag
CLEANFILES = $(BUILT_SOURCES) *.core
#CLEANFILES += *.in

lib_LTLIBRARIES = libspinfo.la

if FREEBSD
plat_sources = spinfo_freebsd.c
plat_headers = spinfo_freebsd.h
plat_libs = -lprocstat -lkvm
endif

if LINUX
plat_sources = spinfo_linux.c
plat_headers = spinfo_linux.h
plat_libs =
endif

nobase_include_HEADERS = spinfo.h

noinst_HEADERS = $(plat_headers)

libspinfo_la_SOURCES = spinfo.c $(plat_sources)
nodist_libspinfo_la_SOURCES = diag.c

if DEBUG
DEBUG_FLAGS = -g -O0 @CC_DEBUG@
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

libspinfo_la_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)

libspinfo_la_LDFLAGS = -version-info 0:0:0 -L$(libdir) -lmrkcommon $(plat_libs)

SUBDIRS = .

diag.c diag.h: diag.txt
	 $(AM_V_GEN) /bin/sh ./gen-diag spinfo

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun && cd ..; fi; done;
