#include <assert.h>

#include <time.h>
#include <unistd.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"

#include "diag.h"

void
spinfo_update0(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_update1(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_update2(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_update3(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_update4(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_init(UNUSED spinfo_ctx_t *ctx, UNUSED pid_t pid, UNUSED unsigned flags)
{
    memset(ctx, 0, sizeof(spinfo_ctx_t));
}


void
spinfo_fini(UNUSED spinfo_ctx_t *ctx)
{
}
