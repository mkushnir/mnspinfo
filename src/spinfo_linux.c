#include <assert.h>

#include <time.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"

#include "diag.h"

/*
 *
 */


void
spinfo_update0(spinfo_ctx_t *ctx)
{
    /*
     * /proc/meminfo
     * /proc/cpuinfo
     */
    ctx->sys.realmem = 0;
    ctx->sys.physmem = 0;
    ctx->sys.usermem = 0;
    ctx->sys.statclock = sysconf(_SC_CLK_TCK);
    //TRACE("statclock=%ld", ctx->sys.statclock);
    ctx->sys.pagesize = 0;
    ctx->sys.ncpu = 0;
}


void
spinfo_update1(UNUSED spinfo_ctx_t *ctx)
{
    /*
     * /proc/meminfo
     */
    /*
     * /proc/stat
     */
}


//void
//_spinfo_update2(spinfo_ctx_t *ctx)
//{
//    /*
//     * getrusage(2), times(3)
//     */
//    if (getrusage(RUSAGE_SELF, &ctx->proc.ru) != 0) {
//        FAIL("getrusage");
//    }
//}
//
//
//void
//spinfo_update2(spinfo_ctx_t *ctx)
//{
//    double ticks;
//    struct rusage ru;
//    double udiff, sdiff;
//    UNUSED double irss;
//
//    /*
//     * getrusage(2), times(3)
//     */
//    if (getrusage(RUSAGE_SELF, &ru) != 0) {
//        FAIL("getrusage");
//    }
//
//    ticks = (double)ctx->elapsed * (double)ctx->sys.statclock / 1000.0;
//
//#define U2DIFF(name) ((double)(ru.name - ctx->proc.ru.name))
//
//    irss = (U2DIFF(ru_ixrss) + U2DIFF(ru_idrss) + U2DIFF(ru_isrss)) * 1024.0 / ticks;
//
//    udiff = (double)(TIMEVAL_TO_USEC(ru.ru_utime) -
//                     TIMEVAL_TO_USEC(ctx->proc.ru.ru_utime)) / 1000.0;
//    sdiff = (double)(TIMEVAL_TO_USEC(ru.ru_stime) -
//                     TIMEVAL_TO_USEC(ctx->proc.ru.ru_stime)) / 1000.0;
//
//    ctx->proc.cpupct = (udiff + sdiff) / (double)ctx->elapsed * 100.0;
//    TRACE("%%PCPU=%lf IRSS=%lf", ctx->proc.cpupct, irss);
//
//    ctx->proc.ru = ru;
//}


void
spinfo_update3(UNUSED spinfo_ctx_t *ctx)
{
    /*
     * /proc/PID/stat
     * /proc/PID/statm
     */
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
    // 1
    _spinfo_update2(ctx);

    ctx->flags = flags;
    ctx->proc.pid = pid;
    // 3

}


void
spinfo_fini(UNUSED spinfo_ctx_t *ctx)
{
}
