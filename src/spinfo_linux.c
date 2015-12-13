#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"
#include "diag.h"

/*
 * /proc utils
 */


void
spinfo_update0(spinfo_ctx_t *ctx)
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
_spinfo_update1(UNUSED spinfo_ctx_t *ctx)
{
    /*
     * /proc/stat
     */
    if (parse_proc_stat_init(ctx) != 0) {
        FAIL("parse_proc_stat_init");
    }
}


void
spinfo_update1(UNUSED spinfo_ctx_t *ctx)
{
    if (parse_proc_stat_update(ctx) != 0) {
        FAIL("parse_proc_stat_update");
    }
}


void
spinfo_update3(UNUSED spinfo_ctx_t *ctx)
{
    /*
     * /proc/PID/stat
     * /proc/PID/statm
     * /proc/PID/fdinfo/
     */
    if (parse_proc_pid_statm(ctx) != 0) {
        FAIL("parse_proc_pid_statm");
    }
    if (parse_proc_pid_fdinfo(ctx) != 0) {
        FAIL("parse_proc_pid_fdinfo");
    }
}


void
spinfo_update4(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_init(spinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    //memset(ctx, 0, sizeof(spinfo_ctx_t));

    ctx->ts0.tv_sec = 0;
    ctx->ts0.tv_nsec = 0;

    if (clock_gettime(CLOCK_MONOTONIC, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }

    spinfo_update0(ctx);
    _spinfo_update1(ctx);
    _spinfo_update2(ctx);

    ctx->flags = flags;
    ctx->proc.pid = pid;
    spinfo_update3(ctx);

}


void
spinfo_fini(UNUSED spinfo_ctx_t *ctx)
{
}
