#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/user.h>

#include <libprocstat.h>
#include <kvm.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"
#include "diag.h"

static int
testfiles(spinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    struct filestat_list *files;
    struct filestat *fs;

    if ((files = procstat_getfiles(ctx->ps, proc, 1)) == NULL) {
        FAIL("procstat_getfiles");
    }

    STAILQ_FOREACH(fs, files, next) {
        if (fs->fs_fd == -1) {
            continue;
        }
        ++ctx->proc.nfiles;
        switch (fs->fs_type) {
        case PS_FST_TYPE_VNODE:
            ++ctx->proc.nvnodes;
            break;
        case PS_FST_TYPE_SOCKET:
            ++ctx->proc.nsockets;
            break;
        default:
            break;
        }
        //TRACE("type=%08x "
        //      "flags=%08x "
        //      "fflags=%08x "
        //      "uflags=%08x "
        //      "fd=%d "
        //      "ref_count=%d "
        //      "offset=%ld "
        //      "path=%s",
        //      fs->fs_type,
        //      fs->fs_flags,
        //      fs->fs_fflags,
        //      fs->fs_uflags,
        //      fs->fs_fd,
        //      fs->fs_ref_count,
        //      fs->fs_offset,
        //      fs->fs_path);
    }
    procstat_freefiles(ctx->ps, files);
    //TRACE("KI vsz=%ld rss=%ld-%ld=%ld %%cpu=%u",
    //      proc->ki_size,
    //      proc->ki_rssize * 4096,
    //      ctx->proc.rss,
    //      proc->ki_rssize * 4096 - ctx->proc.rss,
    //      proc->ki_pctcpu);
    return 0;
}


UNUSED static int
testkstack(spinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    unsigned int i, sz;
    struct kinfo_kstack *kstack;

    if ((kstack = procstat_getkstack(ctx->ps, proc, &sz)) == NULL) {
        FAIL("procstat_getkstack");
    }

    for (i = 0; i < sz; ++i) {
        struct kinfo_kstack *k;

        k = &kstack[i];
        //TRACE("tid=%d state=%d trace=\n%s",
        //      k->kkst_tid,
        //      k->kkst_state,
        //      k->kkst_trace);
    }
    procstat_freekstack(ctx->ps, kstack);
    return 0;
}


static int
testvmmap(spinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    unsigned int i, sz;
    uint64_t vsz_total, res_total;
    struct kinfo_vmentry *vmmap;

    if ((vmmap = procstat_getvmmap(ctx->ps, proc, &sz)) == NULL) {
        FAIL("procstat_getvmmap");
    }

    vsz_total = 0;
    res_total = 0;
    for (i = 0; i < sz; ++i) {
        struct kinfo_vmentry *v;
        uint64_t vsz;

        v = &vmmap[i];
        //if (v->kve_type != 1) {
        //    continue;
        //}
        vsz = v->kve_end - v->kve_start;
        //TRACE("type=%d "
        //      "start=%016lx "
        //      "end=%016lx "
        //      "sz=%ld(%ld) "
        //      "off=%016lx "
        //      "flags=%08x "
        //      "res=%d "
        //      "pres=%d "
        //      "path=%s",
        //      v->kve_type,
        //      v->kve_start,
        //      v->kve_end,
        //      vsz, vsz/4096,
        //      v->kve_offset,
        //      v->kve_flags,
        //      v->kve_resident,
        //      v->kve_private_resident,
        //      v->kve_path);
        vsz_total += vsz;
        res_total += v->kve_resident;
    }
    //TRACE("vsz=%ld(%ld) rss=%ld(%ld)",
    //      vsz_total,
    //      vsz_total / 4096,
    //      res_total * ctx->sys.pagesize,
    //      res_total);
    ctx->proc.vsz = vsz_total;
    ctx->proc.rss = res_total * ctx->sys.pagesize;
    procstat_freevmmap(ctx->ps, vmmap);
    return 0;
}


static int
traverse_procs(spinfo_ctx_t *ctx, spinfo_proc_cb_t cb, void *udata)
{
    unsigned int i;

    for (i = 0; i < ctx->procsz; ++i) {
        if (cb(ctx, &ctx->procs[i], udata) != 0) {
            TRRET(TRAVERSE_PROCS + 1);
        }
    }
    return 0;
}

void
spinfo_update0(spinfo_ctx_t *ctx)
{
    size_t sz;

    /*
     * hw.realmem
     * hw.physmem
     * hw.usermem
     * hw.pagesize
     * hw.ncpu
     */
    ctx->sys.realmem = 0;
    ctx->sys.physmem = 0;
    ctx->sys.usermem = 0;
    ctx->sys.statclock = sysconf(_SC_CLK_TCK);
    ctx->sys.pagesize = 0;
    ctx->sys.ncpu = 0;

    sz = sizeof(ctx->sys.realmem);
    if (sysctlbyname("hw.realmem", &ctx->sys.realmem, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname hw.realmem");
    }

    sz = sizeof(ctx->sys.realmem);
    if (sysctlbyname("hw.physmem", &ctx->sys.physmem, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname hw.physmem");
    }

    sz = sizeof(ctx->sys.usermem);
    if (sysctlbyname("hw.usermem", &ctx->sys.usermem, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname hw.usermem");
    }

    sz = sizeof(ctx->sys.pagesize);
    if (sysctlbyname("hw.pagesize", &ctx->sys.pagesize, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname hw.pagesize");
    }

    sz = sizeof(ctx->sys.ncpu);
    if (sysctlbyname("hw.ncpu", &ctx->sys.ncpu, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname hw.ncpu");
    }
}

#define CPUSTATE_STR(s)        \
    (s) == CP_USER ? "USER":   \
    (s) == CP_NICE ? "NICE":   \
    (s) == CP_SYS ? "SYS":     \
    (s) == CP_INTR ? "INTR":   \
    (s) == CP_IDLE ? "IDLE":   \
    "<unknown>"                \


static void
_spinfo_update1(spinfo_ctx_t *ctx)
{
    size_t sz;
    /*
     * memory
     *  vm.stats.vm.v_interrupt_free_min
     *  vm.stats.vm.v_cache_count
     *  vm.stats.vm.v_inactive_count
     *  vm.stats.vm.v_active_count
     *  vm.stats.vm.v_wire_count
     *  vm.stats.vm.v_free_count
     *  vm.stats.vm.v_page_count
     *  vm.stats.vm.v_page_size
     */
#define SPIU1_VM(n)                            \
    sz = sizeof(ctx->n);                       \
    if (sysctlbyname("vm.stats.vm." #n,        \
                     &ctx->n,                  \
                     &sz,                      \
                     NULL,                     \
                     0) != 0) {                \
        FAIL("sysctlbyname vm.stats.vm." #n);  \
    }                                          \


    SPIU1_VM(v_cache_count)
    SPIU1_VM(v_inactive_count)
    SPIU1_VM(v_active_count)
    SPIU1_VM(v_wire_count)
    SPIU1_VM(v_free_count)
    SPIU1_VM(v_page_count)
    SPIU1_VM(v_page_size)

    //TRACE("%d+%d+%d+%d+%d=%d(%d)",
    //      ctx->v_cache_count,
    //      ctx->v_inactive_count,
    //      ctx->v_active_count,
    //      ctx->v_wire_count,
    //      ctx->v_free_count,
    //      ctx->v_page_count,
    //      ctx->v_cache_count +
    //        ctx->v_inactive_count +
    //        ctx->v_active_count +
    //        ctx->v_wire_count +
    //        ctx->v_free_count);

    ctx->sys.memused = (uint64_t)(ctx->v_cache_count +
                                  ctx->v_inactive_count +
                                  ctx->v_active_count +
                                  ctx->v_wire_count) *
                       (uint64_t)ctx->v_page_size;

    ctx->sys.memfree = (uint64_t)ctx->v_free_count *
                       (uint64_t)ctx->v_page_size;

    ctx->sys.memavail = (uint64_t)ctx->v_page_count *
                        (uint64_t)ctx->v_page_size;

    ctx->sys.mempct = (double)ctx->sys.memused /
                      (double)ctx->sys.memavail * 100.0;

    //TRACE("used=%ld(%lf%%) free=%ld(%lf%%) avail=%ld",
    //      ctx->sys.memused,
    //      ctx->sys.mempct,
    //      ctx->sys.memfree,
    //      (double)ctx->sys.memfree / (double)ctx->sys.memavail * 100.0,
    //      ctx->sys.memavail);

    /*
     * CPU
     *
     * kern.cp_time(s)
     */
    sz = sizeof(ctx->cp_time0);
    if (sysctlbyname("kern.cp_time", &ctx->cp_time0, &sz, NULL, 0) != 0) {
        FAIL("sysctlbyname kern.cp_time");
    }
    ctx->sys.cpupct = 0.;

}


void
spinfo_update1(spinfo_ctx_t *ctx)
{
    int i;
    uint64_t cp_time[CPUSTATES];

    /*
     * kern.cp_time(s)
     */
    for (i = 0; i < CPUSTATES; ++i) {
        cp_time[i] = ctx->cp_time0[i];
    }

    _spinfo_update1(ctx);

    //TRACE("elapsed=%ld", ctx->elapsed);
    if (ctx->elapsed > 0) {
        for (i = 0; i < CPUSTATES; ++i) {
            ctx->cp_time1[i] = ctx->cp_time0[i] - cp_time[i];
            //TRACE("%s=%ld(%lf%%, %lf)",
            //      CPUSTATE_STR(i),
            //      ctx->cp_time1[i],
            //      (double)MIN(ctx->cp_time1[i], ctx->elapsed) /
            //      (double)MAX(ctx->cp_time1[i], ctx->elapsed) *
            //      100.0,
            //      ((double)(ctx->cp_time1[i] * 100) / (double)ctx->elapsed));
        }
    }
    ctx->sys.cpupct = (double)(ctx->cp_time1[CP_USER] +
                               ctx->cp_time1[CP_NICE] +
                               ctx->cp_time1[CP_SYS] +
                               ctx->cp_time1[CP_INTR]) /
        (double)ctx->elapsed * 100.0;
    //TRACE("%%CPU=%lf", ctx->sys.cpupct);
}


static void
_spinfo_update3(spinfo_ctx_t *ctx)
{
    int op;

    op = KERN_PROC_PID;
    if (ctx->proc.pid == 0) {
        ctx->proc.pid = getpid();
    }
    //TRACE("querying pid %d", ctx->proc.pid);
    ctx->procsz = 0;
    if ((ctx->procs = procstat_getprocs(ctx->ps,
                                        op,
                                        (int)ctx->proc.pid,
                                        &ctx->procsz)) == NULL) {
        FAIL("procstat_getprocs");
    }
    ctx->proc.vsz = 0;
    ctx->proc.rss = 0;
    if (traverse_procs(ctx, testvmmap, NULL) != 0) {
        FAIL("traverse_procs");
    }
    ctx->proc.nfiles = 0;
    ctx->proc.nvnodes = 0;
    ctx->proc.nsockets = 0;
    if (traverse_procs(ctx, testfiles, NULL) != 0) {
        FAIL("traverse_procs");
    }
}


void
spinfo_update3(spinfo_ctx_t *ctx)
{
    spinfo_fini(ctx);
    if ((ctx->ps = procstat_open_sysctl()) == NULL) {
        FAIL("procstat_open_sysctl");
    }
    _spinfo_update3(ctx);
}


void
spinfo_update4(UNUSED spinfo_ctx_t *ctx)
{
}


void
spinfo_init(spinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    ctx->ts0.tv_sec = 0;
    ctx->ts0.tv_nsec = 0;

    if (clock_gettime(CLOCK_UPTIME_FAST, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }

    spinfo_update0(ctx);
    _spinfo_update1(ctx);
    _spinfo_update2(ctx);

    ctx->flags = flags;
    ctx->proc.pid = pid;
    if ((ctx->ps = procstat_open_sysctl()) == NULL) {
        FAIL("procstat_open_sysctl");
    }
    _spinfo_update3(ctx);
}

void
spinfo_fini(spinfo_ctx_t *ctx)
{
    if (ctx->ps != NULL) {
        if (ctx->procs != NULL) {
            procstat_freeprocs(ctx->ps, ctx->procs);
            ctx->procs = NULL;
        }
        procstat_close(ctx->ps);
        ctx->ps = NULL;
    }
}

