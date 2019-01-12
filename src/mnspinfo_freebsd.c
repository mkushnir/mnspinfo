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

#include "mnspinfo_private.h"
#include "diag.h"

static int
testfiles(mnspinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    int res = 0;

    struct filestat_list *files;
    struct filestat *fs;

    if ((files = procstat_getfiles(ctx->ps, proc, 1)) == NULL) {
        res = -1;
        goto end;
        //FAIL("procstat_getfiles");
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

    //TRACE("proc=%p", proc);
    //TRACE("ki_fd=%p", proc->ki_fd);
    //TRACE("fd_files=%p", proc->ki_fd->fd_files);
    //TRACE("fdt_nfiles=%d", proc->ki_fd->fd_files->fdt_nfiles);

end:
    return res;
}


UNUSED static int
testkstack(mnspinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    int res = 0;
    unsigned int i, sz;
    struct kinfo_kstack *kstack;

    if ((kstack = procstat_getkstack(ctx->ps, proc, &sz)) == NULL) {
        res = -1;
        goto end;
        //FAIL("procstat_getkstack");
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
end:
    return res;
}


UNUSED static int
testvmmap(mnspinfo_ctx_t *ctx, struct kinfo_proc *proc, UNUSED void *udata)
{
    int res = 0;
    unsigned int i, sz;
    uint64_t vsz_total, res_total;
    struct kinfo_vmentry *vmmap;

    if ((vmmap = procstat_getvmmap(ctx->ps, proc, &sz)) == NULL) {
        res = -1;
        goto end;
        //FAIL("procstat_getvmmap");
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

end:
    return res;
}


static int
traverse_procs(mnspinfo_ctx_t *ctx, mnspinfo_proc_cb_t cb, void *udata)
{
    unsigned int i;

    for (i = 0; i < ctx->procsz; ++i) {
        if (cb(ctx, &ctx->procs[i], udata) != 0) {
            TRRET(TRAVERSE_PROCS + 1);
        }
    }
    return 0;
}

int
mnspinfo_update0(mnspinfo_ctx_t *ctx)
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
    return 0;
}

#define CPUSTATE_STR(s)        \
    (s) == CP_USER ? "USER":   \
    (s) == CP_NICE ? "NICE":   \
    (s) == CP_SYS ? "SYS":     \
    (s) == CP_INTR ? "INTR":   \
    (s) == CP_IDLE ? "IDLE":   \
    "<unknown>"                \


static void
_mnspinfo_update1(mnspinfo_ctx_t *ctx)
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
#define SPIU1_VM(n)                                                    \
    sz = sizeof(ctx->n);                                               \
    if (sysctlbyname("vm.stats.vm." #n,                                \
                     &ctx->n,                                          \
                     &sz,                                              \
                     NULL,                                             \
                     0) != 0) {                                        \
        ctx->n = 0;                                                    \
        /* TRACE("no such sysctl: %s, ignoring", "vm.stats.vm." #n); */\
    }                                                                  \


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


int
mnspinfo_update1(mnspinfo_ctx_t *ctx)
{
    int i;
    uint64_t cp_time[CPUSTATES];

    /*
     * kern.cp_time(s)
     */
    for (i = 0; i < CPUSTATES; ++i) {
        cp_time[i] = ctx->cp_time0[i];
    }

    _mnspinfo_update1(ctx);

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
    return 0;
}


static int
_mnspinfo_update3(mnspinfo_ctx_t *ctx)
{
    int res = 0;

    ctx->ru1 = ctx->procs->ki_rusage;

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

    //ctx->proc.vsz = 0;
    //ctx->proc.rss = 0;
    //if ((res = traverse_procs(ctx, testvmmap, NULL)) != 0) {
    //    goto end;
    //    //FAIL("traverse_procs");
    //}

    ctx->proc.vsz = ctx->procs->ki_size;
    ctx->proc.rss = ctx->procs->ki_rssize * ctx->v_page_size;

    ctx->proc.nfiles = 0;
    ctx->proc.nvnodes = 0;
    ctx->proc.nsockets = 0;
    if ((res = traverse_procs(ctx, testfiles, NULL)) != 0) {
        goto end;
        //FAIL("traverse_procs");
    }

end:
    return res;
}


int
mnspinfo_update3(mnspinfo_ctx_t *ctx)
{
    int res = 0;

    ctx->procsz = 0;
    if ((ctx->procs = procstat_getprocs(ctx->ps,
                                        KERN_PROC_PID,
                                        (int)ctx->proc.pid,
                                        &ctx->procsz)) == NULL) {
        res = -1;
        goto end;
    }

    res = _mnspinfo_update3(ctx);

    procstat_freeprocs(ctx->ps, ctx->procs);
    ctx->procs = NULL;

end:
    return res;
}


int
mnspinfo_update4(UNUSED mnspinfo_ctx_t *ctx)
{
    return 0;
}


int
mnspinfo_init(mnspinfo_ctx_t *ctx, pid_t pid, unsigned flags)
{
    int res = 0;

    //memset(ctx, 0, sizeof(mnspinfo_ctx_t));

    ctx->elapsed = 0;

    ctx->proc.pid = pid;
    if (ctx->proc.pid == 0) {
        ctx->proc.pid = getpid();
    }

    ctx->ts0.tv_sec = 0;
    ctx->ts0.tv_nsec = 0;

    ctx->flags = flags;

    if (clock_gettime(CLOCK_REALTIME_FAST, &ctx->ts1) != 0) {
        FAIL("clock_gettime");
    }

    if ((res = mnspinfo_update0(ctx)) != 0) {
        goto end;
    }

    _mnspinfo_update1(ctx);

    if ((ctx->ps = procstat_open_sysctl()) == NULL) {
        FAIL("procstat_open_sysctl");
    }

    res = mnspinfo_update3(ctx);

end:
    return res;
}


void
mnspinfo_fini(mnspinfo_ctx_t *ctx)
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
