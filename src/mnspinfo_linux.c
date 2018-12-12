#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "mnspinfo_private.h"
#include "diag.h"

/*
 * /proc utils
 */


void
mnspinfo_update0(mnspinfo_ctx_t *ctx)
{
    /*
     * /proc/meminfo
     * /proc/cpuinfo
     */
    ctx->sys.statclock = sysconf(_SC_CLK_TCK);
    //TRACE("statclock=%ld", ctx->sys.statclock);
    ctx->sys.pagesize = sysconf(_SC_PAGESIZE);

    if (parse_cpuinfo(ctx) != 0) {
        FAIL("parse_cpuinfo");
    }
    if (parse_meminfo(ctx) != 0) {
        FAIL("parse_meminfo");
    }
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


void
mnspinfo_update3(UNUSED mnspinfo_ctx_t *ctx)
{
    /*
     * /proc/PID/stat
     * /proc/PID/statm
     * /proc/PID/fdinfo/
     * /proc/PID/fd/
     */
    if (parse_proc_pid_statm(ctx) != 0) {
        FAIL("parse_proc_pid_statm");
    }
    if (parse_proc_pid_fdinfo(ctx) != 0) {
        FAIL("parse_proc_pid_fdinfo");
    }
    if (parse_proc_pid_fd(ctx) != 0) {
        FAIL("parse_proc_pid_fd");
    }
}


void
mnspinfo_update4(UNUSED mnspinfo_ctx_t *ctx)
{
}


void
mnspinfo_init(mnspinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    //memset(ctx, 0, sizeof(mnspinfo_ctx_t));

    ctx->ts0.tv_sec = 0;
    ctx->ts0.tv_nsec = 0;

    if (clock_gettime(CLOCK_MONOTONIC, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }

    mnspinfo_update0(ctx);
    _mnspinfo_update1(ctx);
    _mnspinfo_update2(ctx);

    ctx->flags = flags;
    ctx->proc.pid = pid;
    mnspinfo_update3(ctx);

}


void
mnspinfo_fini(UNUSED mnspinfo_ctx_t *ctx)
{
}
