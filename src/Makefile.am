AM_MAKEFLAGS = -s

diags = diag.txt
BUILT_SOURCES = diag.c diag.h
EXTRA_DIST = $(diags)
CLEANFILES = $(BUILT_SOURCES) *.core core
#CLEANFILES += *.in

lib_LTLIBRARIES = libmnspinfo.la

if FREEBSD
plat_sources = mnspinfo_freebsd.c
plat_headers = mnspinfo_freebsd.h
plat_libs = -lprocstat -lkvm
endif

if LINUX
plat_sources = mnspinfo_linux.c linux_procutils.c
plat_headers = mnspinfo_linux.h
plat_libs =
endif

nobase_include_HEADERS = mnspinfo.h

noinst_HEADERS = $(plat_headers) mnspinfo_private.h


libmnspinfo_la_SOURCES = mnspinfo.c mnspinfo_posix.c $(plat_sources)
nodist_libmnspinfo_la_SOURCES = diag.c

DEBUG_LD_FLAGS =
if DEBUG
DEBUG_CC_FLAGS = -g -O0 @CC_DEBUG@
DEBUG_LD_FLAGS += -no-install
else
DEBUG_CC_FLAGS = -DNDEBUG -O3 -fomit-frame-pointer
if LTO
DEBUG_CC_FLAGS += @CC_LTO@
DEBUG_LD_FLAGS += @LD_LTO@
endif
endif

if ALLSTATIC
libmnspinfo_la_LDFLAGS = -all-static
else
libmnspinfo_la_LDFLAGS =
endif

libmnspinfo_la_CFLAGS = $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)

libmnspinfo_la_LDFLAGS += $(DEBUG_LD_FLAGS) -version-info 0:0:0 -L$(libdir) $(plat_libs)
libmnspinfo_la_LIBADD = -lmrkcommon -lmndiag

SUBDIRS = .

diag.c diag.h: $(diags)
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -v -S diag.txt.tmp -L mnspinfo -H diag.h -C diag.c *.[ch]

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun && cd ..; fi; done;
