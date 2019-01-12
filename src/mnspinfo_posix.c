#include <assert.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "mnspinfo_private.h"
#include "diag.h"

void
_mnspinfo_update2(mnspinfo_ctx_t *ctx)
{
    /*
     * getrusage(2), times(3)
     */
    if (getrusage(RUSAGE_SELF, &ctx->proc.ru) != 0) {
        FAIL("getrusage");
    }
}


int
mnspinfo_update2(mnspinfo_ctx_t *ctx)
{
    double ticks;
    struct rusage ru;
    double udiff, sdiff;
    UNUSED double irss;

    /*
     * getrusage(2), times(3)
     */
    if (getrusage(RUSAGE_SELF, &ru) != 0) {
        FAIL("getrusage");
    }

    ticks = (double)ctx->elapsed * (double)ctx->sys.statclock / 1000.0;

#define U2DIFF(name) ((double)(ru.name - ctx->proc.ru.name))

    irss = (U2DIFF(ru_ixrss) + U2DIFF(ru_idrss) + U2DIFF(ru_isrss)) * 1024.0 / ticks;

    udiff = (double)(TIMEVAL_TO_USEC(ru.ru_utime) -
                     TIMEVAL_TO_USEC(ctx->proc.ru.ru_utime)) / 1000.0;
    sdiff = (double)(TIMEVAL_TO_USEC(ru.ru_stime) -
                     TIMEVAL_TO_USEC(ctx->proc.ru.ru_stime)) / 1000.0;

    ctx->proc.cpupct = (udiff + sdiff) / (double)ctx->elapsed * 100.0;
    //TRACE("%%PCPU=%lf IRSS=%lf", ctx->proc.cpupct, irss);

    ctx->proc.ru = ru;
    return 0;
}


