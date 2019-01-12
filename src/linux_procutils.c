#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

//#define TRRET_DEBUG
#include <mrkcommon/dumpm.h>
#include <mrkcommon/traversedir.h>
#include <mrkcommon/util.h>

#include "mnspinfo_private.h"
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
    mnspinfo_ctx_t *ctx = udata;

    ++(ctx->sys.ncpu);
    //TRACE("[%d]=%s (%s)", f->cpuid, f->vendor_id, f->model_name);
    return 0;
}


int
parse_cpuinfo(mnspinfo_ctx_t *ctx)
{
    cpuinfo_t proc;

    ctx->sys.ncpu = 0;
    return parse_kvp("/proc/cpuinfo",
                     &proc.base,
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
    mnspinfo_ctx_t *ctx = udata;

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
parse_meminfo(mnspinfo_ctx_t *ctx)
{
    meminfo_t proc;

    return parse_kvp("/proc/meminfo",
                     &proc.base,
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
    UNUSED mnspinfo_ctx_t *ctx = udata;
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
parse_proc_stat_init(mnspinfo_ctx_t *ctx)
{
    proc_stat_t proc;

    return parse_kvp("/proc/stat",
                     &proc.base,
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
    mnspinfo_ctx_t *ctx = udata;
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
parse_proc_stat_update(mnspinfo_ctx_t *ctx)
{
    proc_stat_t proc;

    return parse_kvp("/proc/stat",
                     &proc.base,
                     proc_stat_fdesc_update,
                     countof(proc_stat_fdesc_update),
                     &proc_stat_fdesc_update[countof(proc_stat_fdesc_update) - 1],
                     ' ',
                     '\n',
                     NULL,
                     ctx);
}


/*
 * /proc/PID/statp -> proc_pid_statp_t
 */
static int
set_proc_pid_statp(UNUSED proc_base_t *proc,
                   UNUSED proc_fieldesc_t *fdesc,
                   char *key,
                   char *val,
                   UNUSED void *udata)
{
    proc_pid_statp_t *statp = (proc_pid_statp_t *)proc;

    if (sscanf(key, "%d", &statp->pid) < 1) {
        perror("sscanf");
        return 1;
    }
    if (sscanf(val, "%128s %1c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld",
               statp->comm,
               &statp->state,
               &statp->ppid,
               &statp->pgrp,
               &statp->session,
               &statp->tty_nr,
               &statp->tpgid,
               &statp->flags,
               &statp->minflt,
               &statp->cminflt,
               &statp->majflt,
               &statp->cmajflt,
               &statp->utime,
               &statp->stime,
               &statp->cutime,
               &statp->cstime) < 16) {
        perror("sscanf");
        return 1;
    }
    return 0;
}


static proc_fieldesc_t proc_pid_statp_fdesc[] = {
    {"", set_proc_pid_statp},
};


int
parse_proc_pid_statp(mnspinfo_ctx_t *ctx)
{
    int res;
    char buf[64];
    proc_pid_statp_t proc;

    (void)snprintf(buf, sizeof(buf), "/proc/%d/stat", ctx->proc.pid);
    if ((res = parse_kvp(
            buf,
            &proc.base,
            proc_pid_statp_fdesc,
            countof(proc_pid_statp_fdesc),
            &proc_pid_statp_fdesc[countof(proc_pid_statp_fdesc) - 1],
            ' ',
            '\n',
            NULL,
            ctx)) == 0) {
         TICKS_TO_TIMEVAL(ctx->ru1.ru_utime, ctx->sys.statclock, proc.utime);
         TICKS_TO_TIMEVAL(ctx->ru1.ru_stime, ctx->sys.statclock, proc.stime);

        if (ctx->elapsed) {
            double udiff, sdiff;
            udiff = (double)(TIMEVAL_TO_USEC(ctx->ru1.ru_utime) -
                             TIMEVAL_TO_USEC(ctx->ru0.ru_utime)) / 1000.0;
            sdiff = (double)(TIMEVAL_TO_USEC(ctx->ru1.ru_stime) -
                             TIMEVAL_TO_USEC(ctx->ru0.ru_stime)) / 1000.0;

            ctx->proc.cpupct = (udiff + sdiff) / (double)ctx->elapsed * 100.0;

        } else {
            ctx->proc.cpupct = 0.0;
        }

        ctx->ru0 = ctx->ru1;
    }

    return res;
}





/*
 * /proc/PID/statm -> proc_pid_statm_t
 */
static int
set_proc_pid_statm(proc_base_t *proc,
                   UNUSED proc_fieldesc_t *fdesc,
                   char *key,
                   char *val,
                   void *udata)
{
    mnspinfo_ctx_t *ctx = udata;
    proc_pid_statm_t *statm = (proc_pid_statm_t *)proc;

    if (sscanf(key, "%ld", &statm->vsize) < 1) {
        perror("sscanf");
        return 1;
    }
    if (sscanf(val, "%ld %ld %ld %ld %ld %ld",
               &statm->rss,
               &statm->shared,
               &statm->text,
               &statm->lib,
               &statm->data,
               &statm->dirty) < 6) {
        perror("sscanf");
        return 1;
    }
    ctx->proc.vsz = statm->vsize * ctx->sys.pagesize;
    ctx->proc.rss = statm->rss * ctx->sys.pagesize;
    return 0;
}


static proc_fieldesc_t proc_pid_statm_fdesc[] = {
    {"", set_proc_pid_statm},
};


int
parse_proc_pid_statm(mnspinfo_ctx_t *ctx)
{
    char buf[64];
    proc_pid_statm_t proc;

    (void)snprintf(buf, sizeof(buf), "/proc/%d/statm", ctx->proc.pid);
    return parse_kvp(buf,
                     &proc.base,
                     proc_pid_statm_fdesc,
                     countof(proc_pid_statm_fdesc),
                     &proc_pid_statm_fdesc[countof(proc_pid_statm_fdesc) - 1],
                     ' ',
                     '\n',
                     NULL,
                     ctx);
}


/*
 * /proc/PID/fdinfo -> proc.nvnodes, proc.nsockets
 */
static int
set_proc_fdinfo_flags(UNUSED proc_base_t *proc,
                      UNUSED proc_fieldesc_t *fdesc,
                      UNUSED char *key,
                      char *val,
                      void *udata)
{
    mnspinfo_ctx_t *ctx = udata;
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
        mnspinfo_ctx_t *ctx = udata;
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
parse_proc_pid_fdinfo(mnspinfo_ctx_t *ctx)
{
    char buf[64];

    ctx->proc.nfiles = 0;
    ctx->proc.nvnodes = 0;
    ctx->proc.nsockets = 0;
    (void)snprintf(buf, sizeof(buf), "/proc/%d/fdinfo", ctx->proc.pid);
    return traverse_dir(buf, fdinfo_cb, ctx);
}


/*
 * /proc/PID/fd -> proc.nsockets
 */
static int
fd_cb(const char *path, struct dirent *de, void *udata)
{
    int res;

    res = 0;
    if (de != NULL) {
        mnspinfo_ctx_t *ctx = udata;
        char *fpath;
        char buf[1024];
        size_t sz;

        ++ctx->proc.nfiles;

        fpath = path_join(path, de->d_name);
        if ((sz = readlink(fpath, buf, sizeof(buf))) > 0) {
            buf[sz] = '\0';
            if (strstr(buf, "socket:") == buf) {
                ++ctx->proc.nsockets;
            }
        }
        free(fpath);
    }
    return res;
}


int
parse_proc_pid_fd(mnspinfo_ctx_t *ctx)
{
    char buf[64];

    //ctx->proc.nfiles = 0;
    //ctx->proc.nvnodes = 0;
    ctx->proc.nsockets = 0;
    (void)snprintf(buf, sizeof(buf), "/proc/%d/fd", ctx->proc.pid);
    return traverse_dir(buf, fd_cb, ctx);
}
