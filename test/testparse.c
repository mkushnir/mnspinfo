#include <assert.h>
#include <stdlib.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "mnspinfo_private.h"
#include "diag.h"

CPUINFO_KVP_SETTER_INT(cpuid)

proc_fieldesc_t fdesc[] = {
    {"processor", CPUINFO_KVP_SETTER_NAME(cpuid)},
};

static int
cpuinfo_rcb(UNUSED proc_base_t *proc, void *udata)
{
    mnspinfo_ctx_t *ctx = udata;

    ++(ctx->sys.ncpu);
    //TRACE("[%d]=%s (%s)", f->cpuid, f->vendor_id, f->model_name);
    return 0;
}


int
main(void)
{
    cpuinfo_t proc;
    mnspinfo_ctx_t ctx;

    ctx.sys.ncpu = 0;

    if (parse_kvp("/proc/cpuinfo",
                  (proc_base_t *)&proc,
                  fdesc,
                  countof(fdesc),
                  &fdesc[countof(fdesc) - 1],
                  ':',
                  '\n',
                  cpuinfo_rcb,
                  &ctx) != 0) {
        FAIL("parse_kvp");
    }
    TRACE("ncpu=%d", ctx.sys.ncpu);

    ctx.sys.ncpu = 0;

    if (parse_cpuinfo(&ctx) != 0) {
        FAIL("parse_cpuinfo");
    }
    TRACE("ncpu=%d", ctx.sys.ncpu);
    return 0;
}
