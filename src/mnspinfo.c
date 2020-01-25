#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <mncommon/dumpm.h>

#include "mnspinfo_private.h"

#include "diag.h"

#include "config.h"
#ifndef HAVE_CLOCK_REALTIME_FAST
#   define CLOCK_REALTIME_FAST CLOCK_REALTIME
#endif

mnspinfo_ctx_t *
mnspinfo_new(pid_t pid, unsigned flags)
{
    mnspinfo_ctx_t *ctx;

    if ((ctx = malloc(sizeof(mnspinfo_ctx_t))) == NULL) {
        goto end;
    }
    if (mnspinfo_init(ctx, pid, flags) != 0) {
        goto err;
    }

end:
    return ctx;

err:
    free(ctx);
    ctx = NULL;
    goto end;
}


int
mnspinfo_reinit(mnspinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    mnspinfo_fini(ctx);
    return mnspinfo_init(ctx, pid, flags);
}


int
mnspinfo_update(mnspinfo_ctx_t *ctx, unsigned what)
{
    int res = 0;
    if (clock_gettime(CLOCK_REALTIME_FAST, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }
    ctx->elapsed = ((TIMESPEC_TO_NSEC(ctx->ts1) -
                     TIMESPEC_TO_NSEC(ctx->ts0))) / 1000000;

    if (what & MNSPINFO_U0) {
        if ((res = mnspinfo_update0(ctx)) != 0) {
            goto end;
        }
    }
    if (what & MNSPINFO_U1) {
        if ((res = mnspinfo_update1(ctx)) != 0) {
            goto end;
        }
    }
    if (what & MNSPINFO_U2) {
        if ((res = mnspinfo_update2(ctx)) != 0) {
            goto end;
        }
    }
    if (what & MNSPINFO_U3) {
        if ((res = mnspinfo_update3(ctx)) != 0) {
            goto end;
        }
    }
    ctx->ts0 = ctx->ts1;

end:
    return res;
}


void
mnspinfo_destroy(mnspinfo_ctx_t **pctx)
{
    if (*pctx != NULL) {
        mnspinfo_fini(*pctx);
        free(*pctx);
        *pctx = NULL;
    }
}
