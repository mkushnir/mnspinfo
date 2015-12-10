#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>

#include "spinfo_private.h"

#include "diag.h"

spinfo_ctx_t *
spinfo_new(pid_t pid, unsigned flags)
{
    spinfo_ctx_t *ctx;

    if ((ctx = malloc(sizeof(spinfo_ctx_t))) == NULL) {
        FAIL("malloc");
    }
    spinfo_init(ctx, pid, flags);
    return ctx;
}


void
spinfo_reinit(spinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    spinfo_fini(ctx);
    spinfo_init(ctx, pid, flags);
}


void
spinfo_update(spinfo_ctx_t *ctx, unsigned what)
{
    if (clock_gettime(CLOCK_MONOTONIC, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }
    ctx->elapsed = ((TIMESPEC_TO_NSEC(ctx->ts1) -
                     TIMESPEC_TO_NSEC(ctx->ts0))) / 1000000;

    if (what & SPINFO_U0) {
        spinfo_update0(ctx);
    }
    if (what & SPINFO_U1) {
        spinfo_update1(ctx);
    }
    if (what & SPINFO_U2) {
        spinfo_update2(ctx);
    }
    if (what & SPINFO_U3) {
        spinfo_update3(ctx);
    }
    ctx->ts0 = ctx->ts1;
}


void
spinfo_destroy(spinfo_ctx_t **pctx)
{
    if (*pctx != NULL) {
        spinfo_fini(*pctx);
        free(*pctx);
        *pctx = NULL;
    }
}
