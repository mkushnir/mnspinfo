#ifndef SPINFO_FREEBSD_H_DEFINED
#define SPINFO_FREEBSD_H_DEFINED

#include <time.h>

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/resource.h>
#include <sys/resource.h> /* CPUSTATES */
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/user.h>

#include <libprocstat.h>
#include <kvm.h>

/*
 * FreeBSD specific private interface
 */
/*
 * vm.stats
 *  vm.stats.vm.v_interrupt_free_min
 *  vm.stats.vm.v_cache_count
 *  vm.stats.vm.v_inactive_count
 *  vm.stats.vm.v_active_count
 *  vm.stats.vm.v_wire_count
 *  vm.stats.vm.v_free_count
 *  vm.stats.vm.v_page_count
 *  vm.stats.vm.v_page_size
 *
 * hw.[ncpu|physmem|usermem|pagesize|realmem]
 * dev.cpu.*
 * kern.cp_time(s)
 */
#ifdef __cplusplus
extern "C" {
#endif


#ifdef SPINFO_CTX_T_DEFINED
#error "SPINFO_CTX_T_DEFINED cannot be defined here"
#endif

#define TIMESPEC_TO_NSEC(ts) ((ts).tv_sec * 1000000000 + (ts).tv_nsec)
#define TIMEVAL_TO_USEC(ts) ((ts).tv_sec * 1000000 + (ts).tv_usec)

typedef struct _spinfo_ctx {
    /*
     * public interface
     */
    struct timespec ts0;
    struct timespec ts1;
    uint64_t elapsed; /* msec */

    struct {
        uint64_t realmem;
        uint64_t physmem;
        uint64_t usermem;
        long statclock;
        int pagesize;
        int ncpu;

        uint64_t memused;
        uint64_t memfree;
        uint64_t memavail;
        double mempct;
        double cpupct;
    } sys;

    struct {
        struct rusage ru;
        uint64_t vsz;
        uint64_t rss;
        double cpupct;
        uint64_t nfiles;
        uint64_t nvnodes;
        uint64_t nsockets;
        pid_t pid;
    } proc;

    unsigned flags;

    /*
     * private interface
     */
    uint64_t cp_time0[CPUSTATES];
    uint64_t cp_time1[CPUSTATES];


    int v_cache_count;
    int v_inactive_count;
    int v_active_count;
    int v_wire_count;
    int v_free_count;
    int v_page_count;
    int v_page_size;

    struct procstat *ps;
    struct kinfo_proc *procs;
    //struct filestat_list *files;
    //struct kinfo_kstack *kstack;
    //struct kinfo_vmentry *vmmap;

    unsigned int procsz;
} spinfo_ctx_t;
#define SPINFO_CTX_T_DEFINED


typedef int (*spinfo_proc_cb_t)(spinfo_ctx_t *, struct kinfo_proc *, void *);

#ifdef __cplusplus
}
#endif
#endif /* SPINFO_FREEBSD_H_DEFINED */
