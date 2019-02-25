CLEANFILES = *.core core
#CLEANFILES += *.in
AM_MAKEFLAGS = -s

bin_PROGRAMS = procgauges
noinst_PROGRAMS=testfoo testrusage

diags = ../src/diag.txt
BUILT_SOURCES = diag.c diag.h

noinst_HEADERS = procgauges.h


if FREEBSD
plat_libs = -lprocstat -lkvm
endif

if LINUX
plat_libs =
noinst_PROGRAMS += testparse
endif

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
testfoo_LDFLAGS = -all-static
testrusage_LDFLAGS = -all-static
testparse_LDFLAGS = -all-static
procgauges_LDFLAGS = -all-static
else
testfoo_LDFLAGS =
testrusage_LDFLAGS =
testparse_LDFLAGS =
procgauges_LDFLAGS =
endif

nodist_testfoo_SOURCES = diag.c
testfoo_SOURCES = testfoo.c
testfoo_CFLAGS = $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testfoo_LDFLAGS += $(DEBUG_LD_FLAGS) -L$(top_srcdir)/src/.libs -L$(libdir)
testfoo_LDADD = -lmnspinfo -lmrkcommon -lmndiag $(plat_libs)

nodist_testrusage_SOURCES = diag.c
testrusage_SOURCES = testrusage.c
testrusage_CFLAGS = $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testrusage_LDFLAGS += $(DEBUG_LD_FLAGS) -L$(top_srcdir)/src/.libs -L$(libdir)
testrusage_LDADD = -lmnspinfo -lmrkcommon -lmndiag $(plat_libs)

nodist_procgauges_SOURCES = diag.c
procgauges_SOURCES = procgauges.c
procgauges_CFLAGS = $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
procgauges_LDFLAGS += $(DEBUG_LD_FLAGS) -L$(top_srcdir)/src/.libs -L$(libdir)
procgauges_LDADD = -lmnspinfo -lmrkapp -lmrkcommon -lmndiag $(plat_libs)

if LINUX
nodist_testparse_SOURCES = diag.c
testparse_SOURCES = testparse.c
testparse_CFLAGS = $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ -I$(top_srcdir)/src -I$(top_srcdir) -I$(includedir)
testparse_LDFLAGS += $(DEBUG_LD_FLAGS) -L$(top_srcdir)/src/.libs -L$(libdir) $(plat_libs)
testparse_LDADD = -lmnspinfo -lmrkcommon -lmndiag $(plat_libs)
endif

diag.c diag.h: $(diags)
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -v -S diag.txt.tmp -L mnspinfo -H diag.h -C diag.c ../src/*.[ch] ./*.[ch]

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
