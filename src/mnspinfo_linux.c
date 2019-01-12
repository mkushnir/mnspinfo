#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "mnspinfo_private.h"
#include "diag.h"

#include "config.h"
#ifndef HAVE_CLOCK_REALTIME_FAST
#   define CLOCK_REALTIME_FAST CLOCK_REALTIME
#endif

/*
 * /proc utils
 */


int
mnspinfo_update0(mnspinfo_ctx_t *ctx)
{
    int res = 0;

    /*
     * /proc/meminfo
     * /proc/cpuinfo
     */
    ctx->sys.statclock = sysconf(_SC_CLK_TCK);
    //TRACE("statclock=%ld", ctx->sys.statclock);
    ctx->sys.pagesize = sysconf(_SC_PAGESIZE);

    if ((res = parse_cpuinfo(ctx)) != 0) {
        goto end;
        //FAIL("parse_cpuinfo");
    }
    if ((res = parse_meminfo(ctx)) != 0) {
        goto end;
        //FAIL("parse_meminfo");
    }

end:
    return res;
}


static void
_mnspinfo_update1(UNUSED mnspinfo_ctx_t *ctx)
{
    /*
     * /proc/stat
     */
    if (parse_proc_stat_init(ctx) != 0) {
        FAIL("parse_proc_stat_init");
    }
}


void
mnspinfo_update1(UNUSED mnspinfo_ctx_t *ctx)
{
    if (parse_proc_stat_update(ctx) != 0) {
        FAIL("parse_proc_stat_update");
    }
}


int
mnspinfo_update3(UNUSED mnspinfo_ctx_t *ctx)
{
    int res = 0;

    /*
     * /proc/PID/stat
     * /proc/PID/statm
     * /proc/PID/fdinfo/
     * /proc/PID/fd/
     */
    if ((res = parse_proc_pid_statm(ctx)) != 0) {
        goto end;
        //FAIL("parse_proc_pid_statm");
    }
    if ((res = parse_proc_pid_fdinfo(ctx)) != 0) {
        goto end;
        //FAIL("parse_proc_pid_fdinfo");
    }
    if ((res = parse_proc_pid_fd(ctx)) != 0) {
        goto end;
        //FAIL("parse_proc_pid_fd");
    }

end:
    return res;
}


int
mnspinfo_update4(UNUSED mnspinfo_ctx_t *ctx)
{
    return 0;
}


int
mnspinfo_init(mnspinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    int res = 0;

    //memset(ctx, 0, sizeof(mnspinfo_ctx_t));

    ctx->ts0.tv_sec = 0;
    ctx->ts0.tv_nsec = 0;

    if (clock_gettime(CLOCK_REALTIME_FAST, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }

    if ((res = mnspinfo_update0(ctx)) != 0) {
        goto end;
    }
    _mnspinfo_update1(ctx);
    _mnspinfo_update2(ctx);

    ctx->flags = flags;
    ctx->proc.pid = pid;
    res = mnspinfo_update3(ctx);

end:
    return res;
}


void
mnspinfo_fini(UNUSED mnspinfo_ctx_t *ctx)
{
}
