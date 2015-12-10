#include <assert.h>
#include <stdlib.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"
#include "diag.h"

#define CPU_INFO_KVP_SETTER_NAME(n) KVP_SETTER_NAME(cpu_info_t, n)
#define CPU_INFO_KVP_SETTER_INT(n) KVP_SETTER_INT(cpu_info_t, n)
#define CPU_INFO_KVP_SETTER_STR(n) KVP_SETTER_STR(cpu_info_t, n)

CPU_INFO_KVP_SETTER_INT(cpuid)
CPU_INFO_KVP_SETTER_STR(vendor_id)
CPU_INFO_KVP_SETTER_INT(cpu_family)
CPU_INFO_KVP_SETTER_INT(model)
CPU_INFO_KVP_SETTER_STR(model_name)

proc_fieldesc_t fdesc[] = {
    {"processor", CPU_INFO_KVP_SETTER_NAME(cpuid)},
    {"vendor_id", CPU_INFO_KVP_SETTER_NAME(vendor_id)},
    {"cpu family", CPU_INFO_KVP_SETTER_NAME(cpu_family)},
    {"model", CPU_INFO_KVP_SETTER_NAME(model)},
    {"model name", CPU_INFO_KVP_SETTER_NAME(model_name)},
};

static int
rcb(UNUSED proc_base_t *proc, UNUSED void *udata)
{
    cpu_info_t *f = (cpu_info_t *)proc;
    struct {
        int ncpu;
    } *params = udata;

    ++(params->ncpu);
    TRACE("[%d]=%s (%s)", f->cpuid, f->vendor_id, f->model_name);
    return 0;
}

int
main(void)
{
    cpu_info_t proc;

    struct {
        int ncpu;
    } params;

    params.ncpu = 0;

    if (parse_kvp("/proc/cpuinfo",
                  (proc_base_t *)&proc,
                  fdesc,
                  countof(fdesc),
                  &fdesc[countof(fdesc) - 1],
                  ':',
                  '\n',
                  rcb,
                  &params) != 0) {
        FAIL("parse_kvp");
    }
    TRACE("ncpu=%d", params.ncpu);
    return 0;
}
