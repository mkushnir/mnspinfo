#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/traversedir.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"
#include "diag.h"

/*
 * /proc parser
 */
#define ISSPACE(c) (c != '\0' && (c == ' ' || c == '\t'))
static char *
reach_nonspace(char *s)
{
    while (ISSPACE(*s)) {
        ++s;
    }
    return s;
}


static char *
reach_delim(char *s, char delim)
{
    while (*s != delim && *s != '\0') {
        ++s;
    }
    return s;
}


static void
rtrim_space(char *start, char *end)
{
    while (end > start) {
        if (ISSPACE(*(end - 1))) {
            --end;
        } else {
            break;
        }
    }
    *end = '\0';
}


/*
 * one field per line
 */
int parse_kvp(const char *path,
              proc_base_t *proc,
              proc_fieldesc_t *fdesc,
              size_t nfdesc,
              proc_fieldesc_t *eor,
              char fdelim,
              char rdelim,
              parse_record_cb_t rcb,
              void *udata)
{
    int res;
    char *pos;
    ssize_t sz;
    ssize_t nread;

    res = 0;

    if ((proc->fd = open(path, O_RDONLY)) == -1) {
        TRRET(PARSE_KVP + 1);
    }

    proc->buf = NULL;
    sz = 0;
    do {
        proc->buf = realloc(proc->buf, sz + 4096);
        nread = read(proc->fd, proc->buf + sz, 4096);
        sz += nread;
    } while (nread > 0);

    proc->buf[sz - 1] = '\0';

    for (pos = proc->buf; pos < (proc->buf + sz);) {
        char *kstart, *kend;
        char *vstart, *vend;
        size_t i;

        /* key */
        kstart = reach_nonspace(pos);
        if (*kstart == '\0') {
            goto end;
        }
        kend = reach_delim(kstart, fdelim);
        if (kend == kstart) {
            /* empty key */
            res = PARSE_KVP + 4;
            goto end;
        }
        rtrim_space(kstart, kend);
        ++kend;
        /* value */
        vstart = reach_nonspace(kend);
        vend = reach_delim(vstart, rdelim);
        rtrim_space(vstart, vend);
        ++vend;

        for (i = 0; i < nfdesc; ++i) {
            proc_fieldesc_t *f = &fdesc[i];

            if (strcmp(kstart, f->name) == 0 || *(f->name) == '\0') {
                if (f->cb(proc, f, kstart, vstart, udata) != 0) {
                    res = PARSE_KVP + 5;
                    goto end;
                }
                if (eor == f && rcb != NULL) {
                    if (rcb(proc, udata) != 0) {
                        res = PARSE_KVP + 6;
                        goto end;
                    }
                }
                break;
            }
        }
        pos = vend;
    }

end:
    //if (rcb(proc, udata) != 0) {
    //    res = PARSE_KVP + 7;
    //}
    (void)close(proc->fd);
    proc->fd = -1;
    if (proc->buf != NULL) {
        free(proc->buf);
        proc->buf = NULL;
    }
    TRRET(res);
}



/*
 * /proc/cpuinfo -> cpuinfo_t
 */
CPUINFO_KVP_SETTER_INT(cpuid)
CPUINFO_KVP_SETTER_STR(vendor_id)
CPUINFO_KVP_SETTER_INT(cpu_family)
CPUINFO_KVP_SETTER_INT(model)
CPUINFO_KVP_SETTER_STR(model_name)
CPUINFO_KVP_SETTER_INT(stepping)
CPUINFO_KVP_SETTER_FLOAT(cpu_mhz)
CPUINFO_KVP_SETTER_INT_BU(cache_size)
CPUINFO_KVP_SETTER_INT(physical_id)
CPUINFO_KVP_SETTER_INT(siblings)
CPUINFO_KVP_SETTER_INT(core_id)
CPUINFO_KVP_SETTER_INT(cpu_cores)
CPUINFO_KVP_SETTER_INT(apicid)
CPUINFO_KVP_SETTER_INT(initial_apicid)
CPUINFO_KVP_SETTER_BOOL(fpu)
CPUINFO_KVP_SETTER_BOOL(fpu_exception)
CPUINFO_KVP_SETTER_INT(cpuid_level)
CPUINFO_KVP_SETTER_BOOL(wp)
CPUINFO_KVP_SETTER_STR(flags)
CPUINFO_KVP_SETTER_FLOAT(bogomips)
CPUINFO_KVP_SETTER_INT(clflush_size)
CPUINFO_KVP_SETTER_INT(cache_alignment)
CPUINFO_KVP_SETTER_STR(address_sizes)
CPUINFO_KVP_SETTER_STR(power_management)

static proc_fieldesc_t cpuinfo_fdesc[] = {
    {"processor", CPUINFO_KVP_SETTER_NAME(cpuid)},
    {"vendor_id", CPUINFO_KVP_SETTER_NAME(vendor_id)},
    {"cpu family", CPUINFO_KVP_SETTER_NAME(cpu_family)},
    {"model", CPUINFO_KVP_SETTER_NAME(model)},
    {"model name", CPUINFO_KVP_SETTER_NAME(model_name)},
    {"stepping", CPUINFO_KVP_SETTER_NAME(stepping)},
    {"cpu MHz", CPUINFO_KVP_SETTER_NAME(cpu_mhz)},
    {"cache size", CPUINFO_KVP_SETTER_NAME(cache_size)},
    {"physical id", CPUINFO_KVP_SETTER_NAME(physical_id)},
    {"siblings", CPUINFO_KVP_SETTER_NAME(siblings)},
    {"core id", CPUINFO_KVP_SETTER_NAME(core_id)},
    {"cpu cores", CPUINFO_KVP_SETTER_NAME(cpu_cores)},
    {"apicid", CPUINFO_KVP_SETTER_NAME(apicid)},
    {"initial apicid", CPUINFO_KVP_SETTER_NAME(initial_apicid)},
    {"fpu", CPUINFO_KVP_SETTER_NAME(fpu)},
    {"fpu_exception", CPUINFO_KVP_SETTER_NAME(fpu_exception)},
    {"cpuid level", CPUINFO_KVP_SETTER_NAME(cpuid_level)},
    {"wp", CPUINFO_KVP_SETTER_NAME(wp)},
    {"flags", CPUINFO_KVP_SETTER_NAME(flags)},
    {"bogomips", CPUINFO_KVP_SETTER_NAME(bogomips)},
    {"clflush size", CPUINFO_KVP_SETTER_NAME(clflush_size)},
    {"cache_alignment", CPUINFO_KVP_SETTER_NAME(cache_alignment)},
    {"address_sizes", CPUINFO_KVP_SETTER_NAME(address_sizes)},
    {"power management", CPUINFO_KVP_SETTER_NAME(power_management)},
};


static int
cpuinfo_rcb(UNUSED proc_base_t *proc, void *udata)
{
    spinfo_ctx_t *ctx = udata;

    ++(ctx->sys.ncpu);
    //TRACE("[%d]=%s (%s)", f->cpuid, f->vendor_id, f->model_name);
    return 0;
}


int
parse_cpuinfo(spinfo_ctx_t *ctx)
{
    cpuinfo_t proc;

    ctx->sys.ncpu = 0;
    return parse_kvp("/proc/cpuinfo",
                     (proc_base_t *)&proc,
                     cpuinfo_fdesc,
                     countof(cpuinfo_fdesc),
                     &cpuinfo_fdesc[countof(cpuinfo_fdesc) - 1],
                     ':',
                     '\n',
                     cpuinfo_rcb,
                     ctx);
}


/*
 * /proc/meminfo -> meminfo_t
 */
MEMINFO_KVP_SETTER_LONG_BU(mem_total)
MEMINFO_KVP_SETTER_LONG_BU(mem_free)
MEMINFO_KVP_SETTER_LONG_BU(direct_map_4k)
MEMINFO_KVP_SETTER_LONG_BU(direct_map_2m)

static proc_fieldesc_t meminfo_fdesc[] = {
    {"MemTotal", MEMINFO_KVP_SETTER_NAME(mem_total)},
    {"MemFree", MEMINFO_KVP_SETTER_NAME(mem_free)},
    {"DirectMap4k", MEMINFO_KVP_SETTER_NAME(direct_map_4k)},
    {"DirectMap2M", MEMINFO_KVP_SETTER_NAME(direct_map_2m)},
};


static int
meminfo_rcb(proc_base_t *proc, void *udata)
{
    meminfo_t *meminfo = (meminfo_t *)proc;
    spinfo_ctx_t *ctx = udata;

    ctx->sys.realmem = meminfo->mem_total * ctx->sys.pagesize;
    ctx->sys.physmem = (meminfo->direct_map_4k + meminfo->direct_map_2m) *
        ctx->sys.pagesize;
    ctx->sys.usermem = meminfo->mem_total * ctx->sys.pagesize;

    ctx->sys.memused = (meminfo->mem_total - meminfo->mem_free) * ctx->sys.pagesize;
    ctx->sys.memfree = meminfo->mem_free * ctx->sys.pagesize;
    ctx->sys.memavail = meminfo->mem_total * ctx->sys.pagesize;

    ctx->sys.mempct = (double)ctx->sys.memused /
                      (double)ctx->sys.memavail * 100.0;

    return 0;
}


int
parse_meminfo(spinfo_ctx_t *ctx)
{
    meminfo_t proc;

    return parse_kvp("/proc/meminfo",
                     (proc_base_t *)&proc,
                     meminfo_fdesc,
                     countof(meminfo_fdesc),
                     &meminfo_fdesc[countof(meminfo_fdesc) - 1],
                     ':',
                     '\n',
                     meminfo_rcb,
                     ctx);
}



/*
 * /proc/stat -> proc_stat_t
 */
static int
set_proc_stat_cpu(proc_base_t *proc,
                  UNUSED proc_fieldesc_t *fdesc,
                  UNUSED char *key,
                  UNUSED char *val,
                  UNUSED void *udata)
{
    UNUSED proc_stat_t *f = (proc_stat_t *)proc;
    UNUSED spinfo_ctx_t *ctx = udata;
    //TRACE("val='%s'", val);
    if (sscanf(val, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
        &ctx->pscpu.user,
        &ctx->pscpu.nice,
        &ctx->pscpu.system,
        &ctx->pscpu.idle,
        &ctx->pscpu.iowait,
        &ctx->pscpu.irq,
        &ctx->pscpu.softirq,
        &ctx->pscpu.steal,
        &ctx->pscpu.guest,
        &ctx->pscpu.guest_nice) != 10) {
        perror("sscanf");
        return 1;
    }
    return 0;
}


static proc_fieldesc_t proc_stat_fdesc_init[] = {
    {"cpu", set_proc_stat_cpu},
};


int
parse_proc_stat_init(spinfo_ctx_t *ctx)
{
    proc_stat_t proc;

    return parse_kvp("/proc/stat",
                     (proc_base_t *)&proc,
                     proc_stat_fdesc_init,
                     countof(proc_stat_fdesc_init),
                     &proc_stat_fdesc_init[countof(proc_stat_fdesc_init) - 1],
                     ' ',
                     '\n',
                     NULL,
                     ctx);
}


static int
update_proc_stat_cpu(UNUSED proc_base_t *proc,
                     UNUSED proc_fieldesc_t *fdesc,
                     UNUSED char *key,
                     char *val,
                     void *udata)
{
    spinfo_ctx_t *ctx = udata;
    proc_stat_cpu_t pscpu;
    double user, nice, system, idle, steal, guest, guest_nice, total;

    //TRACE("val='%s'", val);
    if (sscanf(val, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
        &pscpu.user,
        &pscpu.nice,
        &pscpu.system,
        &pscpu.idle,
        &pscpu.iowait,
        &pscpu.irq,
        &pscpu.softirq,
        &pscpu.steal,
        &pscpu.guest,
        &pscpu.guest_nice) != 10) {
        perror("sscanf");
        return 1;
    }
    user = pscpu.user - ctx->pscpu.user;
    nice = pscpu.nice - ctx->pscpu.nice;
    system = pscpu.system - ctx->pscpu.system;
    idle = pscpu.idle - ctx->pscpu.idle;
    steal = pscpu.steal - ctx->pscpu.steal;
    guest = pscpu.guest - ctx->pscpu.guest;
    guest_nice = pscpu.guest_nice - ctx->pscpu.guest_nice;
    total = user + nice + system + idle + steal + guest + guest_nice;
    ctx->sys.cpupct = (total - idle) / total * 100.0;
    ctx->pscpu = pscpu;
    return 0;
}


static proc_fieldesc_t proc_stat_fdesc_update[] = {
    {"cpu", update_proc_stat_cpu},
};


int
parse_proc_stat_update(spinfo_ctx_t *ctx)
{
    proc_stat_t proc;

    return parse_kvp("/proc/stat",
                     (proc_base_t *)&proc,
                     proc_stat_fdesc_update,
                     countof(proc_stat_fdesc_update),
                     &proc_stat_fdesc_update[countof(proc_stat_fdesc_update) - 1],
                     ' ',
                     '\n',
                     NULL,
                     ctx);
}


static int
set_proc_pid_statm(UNUSED proc_base_t *proc,
                   UNUSED proc_fieldesc_t *fdesc,
                   char *key,
                   char *val,
                   void *udata)
{
    spinfo_ctx_t *ctx = udata;

    if (sscanf(key, "%ld", &ctx->statm.vsize) < 1) {
        perror("sscanf");
        return 1;
    }
    if (sscanf(val, "%ld %ld %ld %ld %ld %ld",
               &ctx->statm.rss,
               &ctx->statm.shared,
               &ctx->statm.text,
               &ctx->statm.lib,
               &ctx->statm.data,
               &ctx->statm.dirty) < 6) {
        perror("sscanf");
        return 1;
    }
    ctx->proc.vsz = ctx->statm.vsize * ctx->sys.pagesize;
    ctx->proc.rss = ctx->statm.rss * ctx->sys.pagesize;
    return 0;
}


static proc_fieldesc_t proc_pid_statm_fdesc[] = {
    {"", set_proc_pid_statm},
};


int
parse_proc_pid_statm(spinfo_ctx_t *ctx)
{
    proc_pid_statm_t proc;

    return parse_kvp("/proc/self/statm",
                     (proc_base_t *)&proc,
                     proc_pid_statm_fdesc,
                     countof(proc_pid_statm_fdesc),
                     &proc_pid_statm_fdesc[countof(proc_pid_statm_fdesc) - 1],
                     ' ',
                     '\n',
                     NULL,
                     ctx);
}


static int
set_proc_fdinfo_flags(UNUSED proc_base_t *proc,
                      UNUSED proc_fieldesc_t *fdesc,
                      UNUSED char *key,
                      char *val,
                      void *udata)
{
    spinfo_ctx_t *ctx = udata;
    unsigned long flags;

    if (sscanf(val, "%lo", &flags) != 1) {
        perror("sscanf");
        return 1;
    }
    if (S_ISREG(flags)) {
        ++ctx->proc.nvnodes;
    } else if (S_ISSOCK(flags)) {
        ++ctx->proc.nsockets;
    }
    //TRACE("val=%s flags=%lo S_ISREG=%d S_ISSOCK=%d", val, flags, S_ISREG(flags), S_ISSOCK(flags));
    return 0;
}


static proc_fieldesc_t proc_pid_fdinfo_fdesc[] = {
    {"flags", set_proc_fdinfo_flags}
};


static int
fdinfo_cb(const char *path, struct dirent *de, void *udata)
{
    int res;
    proc_base_t proc;

    res = 0;
    if (de != NULL) {
        spinfo_ctx_t *ctx = udata;
        char *fpath;

        ++ctx->proc.nfiles;

        //TRACE("d_name=%p", de->d_name);
        fpath = path_join(path, de->d_name);

        //TRACE("fpath=%s", fpath);
        res = parse_kvp(fpath,
                         &proc,
                         proc_pid_fdinfo_fdesc,
                         countof(proc_pid_fdinfo_fdesc),
                         &proc_pid_fdinfo_fdesc[
                            countof(proc_pid_fdinfo_fdesc) - 1],
                         ':',
                         '\n',
                         NULL,
                         ctx);
        free(fpath);
    }
    return res;
}


int
parse_proc_pid_fdinfo(spinfo_ctx_t *ctx)
{
    ctx->proc.nfiles = 0;
    ctx->proc.nvnodes = 0;
    ctx->proc.nsockets = 0;
    return traverse_dir("/proc/self/fdinfo", fdinfo_cb, ctx);
}
