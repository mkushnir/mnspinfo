#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <mncommon/dumpm.h>
#include <mncommon/util.h>
#include <mncommon/unittest.h>

#include "diag.h"

#include <mnspinfo.h>

#ifndef NDEBUG
const char *_malloc_options = "AJ";
#endif

static void
bar(void)
{
    static char *s;
    size_t sz, i;

    if (s != NULL) {
        free(s);
    }
    //sz = random() % (1024 * 1024 * 1024);
    //sz = 1024*1024*100 + (random() % (1024 * 1024));
    sz = 1024*100 + (random() % (1024 * 1024));

    TRACE("sz=%ld", sz);
    s = malloc(sz);
    for (i = 0; i < sz; ++i) {
        if (i % 4096 == 0) {
            s[i] = (char)(random() % 256);
        }
    }
}


static void
run(pid_t pid)
{
    mnspinfo_ctx_t *sp;
    sp = mnspinfo_new(pid, MNSPINFO_SELF);

    while (1) {
        mnspinfo_update(sp, MNSPINFO_U1|MNSPINFO_U2|MNSPINFO_U3);
        TRACE("SYS real=%ld phys=%ld user=%ld used=%ld/%lf %%cpu=%lf "
              "PROC vsz=%ld rss=%ld %%mem=%lf %%cpu=%lf files=%ld/%ld/%ld",
              sp->sys.realmem,
              sp->sys.physmem,
              sp->sys.usermem,
              sp->sys.memused,
              sp->sys.mempct,
              sp->sys.cpupct,
              sp->proc.vsz,
              sp->proc.rss,
              (double)sp->proc.vsz / (double)sp->sys.memused * 100.0,
              sp->proc.cpupct,
              sp->proc.nfiles,
              sp->proc.nvnodes,
              sp->proc.nsockets);

        sleep(2);
        bar();
    }
    mnspinfo_destroy(&sp);
}


UNUSED static void
test0(void)
{
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        //TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }

    run(getpid());
    //run(-1);
}


UNUSED static void
test1(void)
{
    pid_t ppid, pid;

    ppid = getpid();
    if ((pid = fork()) == 0) {
        run(ppid);
    } else if (pid == -1) {
        FAIL("fork");
    } else {
        int status;

        if (wait(&status) == pid) {
            TRACE("finished: %d", pid);
        } else {
            FAIL("wait");
        }
    }
}

int
main(void)
{
    test0();
    //test1();
    return 0;
}
