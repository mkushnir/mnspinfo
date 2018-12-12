#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>

#include "mnspinfo_private.h"

#include "diag.h"

mnspinfo_ctx_t *
mnspinfo_new(pid_t pid, unsigned flags)
{
    mnspinfo_ctx_t *ctx;

    if ((ctx = malloc(sizeof(mnspinfo_ctx_t))) == NULL) {
        FAIL("malloc");
    }
    mnspinfo_init(ctx, pid, flags);
    return ctx;
}


void
mnspinfo_reinit(mnspinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    mnspinfo_fini(ctx);
    mnspinfo_init(ctx, pid, flags);
}


void
mnspinfo_update(mnspinfo_ctx_t *ctx, unsigned what)
{
    if (clock_gettime(CLOCK_MONOTONIC, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }
    ctx->elapsed = ((TIMESPEC_TO_NSEC(ctx->ts1) -
                     TIMESPEC_TO_NSEC(ctx->ts0))) / 1000000;

    if (what & MNSPINFO_U0) {
        mnspinfo_update0(ctx);
    }
    if (what & MNSPINFO_U1) {
        mnspinfo_update1(ctx);
    }
    if (what & MNSPINFO_U2) {
        mnspinfo_update2(ctx);
    }
    if (what & MNSPINFO_U3) {
        mnspinfo_update3(ctx);
    }
    ctx->ts0 = ctx->ts1;
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
