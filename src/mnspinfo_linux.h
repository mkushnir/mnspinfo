#ifndef MNSPINFO_LINUX_H_DEFINED
#define MNSPINFO_LINUX_H_DEFINED

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>


/*
 * Linux specific private interface
 */

#ifdef __cplusplus
extern "C" {
#endif

#define KVP_SETTER_NAME(ty, name) set_##ty##_##name

#define KVP_SETTER_STR(ty, n)                          \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        UNUSED char *key,                              \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    f->n = val;                                        \
    return 0;                                          \
}                                                      \


#define KVP_SETTER_SCANF(ty, fmt, n)                   \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        UNUSED char *key,                              \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    sscanf(val, fmt, &f->n);                           \
    return 0;                                          \
}                                                      \


#define KVP_SETTER_BU(ty, fmt, n)              \
static int                                     \
KVP_SETTER_NAME(ty, n)(                        \
        proc_base_t *proc,                     \
        UNUSED proc_fieldesc_t *fdesc,         \
        UNUSED char *key,                      \
        char *val,                             \
        UNUSED void *udata)                    \
{                                              \
    ty *f = (ty *)proc;                        \
    char buf[16];                              \
    memset(buf, '\0', sizeof(buf));            \
                                               \
    sscanf(val, fmt "%16c", &f->n, buf);       \
    if (strcasecmp(buf, "kb") == 0) {          \
        f->n *= 1024;                          \
    } else if (strcasecmp(buf, "mb") == 0) {   \
        f->n *= 1024 * 1024;                   \
    } else if (strcasecmp(buf, "gb") == 0) {   \
        f->n *= 1024 * 1024 * 1024;            \
    } else {                                   \
    }                                          \
/*                                             \
    TRACE("key=%s val=%s n=%d bu='%s'",        \
          fdesc->name,                         \
          val,                                 \
          f->n,                                \
          buf);                                \
 */                                            \
    return 0;                                  \
}                                              \


#define KVP_SETTER_INT(ty, n) KVP_SETTER_SCANF(ty, "%d", n)
#define KVP_SETTER_INT_BU(ty, n) KVP_SETTER_BU(ty, "%d", n)
#define KVP_SETTER_LONG(ty, n) KVP_SETTER_SCANF(ty, "%ld", n)
#define KVP_SETTER_LONG_BU(ty, n) KVP_SETTER_BU(ty, "%ld", n)



#define KVP_SETTER_BOOL(ty, n)                         \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        UNUSED char *key,                              \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    char buf[16];                                      \
    memset(buf, '\0', sizeof(buf));                    \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    sscanf(val, "%16c", buf);                          \
    if (strcasecmp(buf, "yes") ||                      \
        strcasecmp(buf, "on") ||                       \
        strcasecmp(buf, "true")) {                     \
        f->n = 1;                                      \
    } else {                                           \
        f->n = 0;                                      \
    }                                                  \
    return 0;                                          \
}                                                      \


#define KVP_SETTER_FLOAT(ty, n)                        \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        UNUSED char *key,                              \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    sscanf(val, "%f", &f->n);                          \
    return 0;                                          \
}                                                      \


struct _proc_fieldesc;
struct _proc_base;

typedef int (*parse_field_cb_t)(struct _proc_base *,
                                struct _proc_fieldesc *,
                                char *,
                                char *,
                                void *);
typedef int (*parse_record_cb_t)(struct _proc_base *, void *);

typedef struct _proc_fieldesc {
    char *name;
    parse_field_cb_t cb;
} proc_fieldesc_t;

typedef struct _proc_base {
    int fd;
    char *buf;
} proc_base_t;


#define CPUINFO_KVP_SETTER_NAME(n) KVP_SETTER_NAME(cpuinfo_t, n)
#define CPUINFO_KVP_SETTER_INT(n) KVP_SETTER_INT(cpuinfo_t, n)
#define CPUINFO_KVP_SETTER_INT_BU(n) KVP_SETTER_INT_BU(cpuinfo_t, n)
#define CPUINFO_KVP_SETTER_STR(n) KVP_SETTER_STR(cpuinfo_t, n)
#define CPUINFO_KVP_SETTER_FLOAT(n) KVP_SETTER_FLOAT(cpuinfo_t, n)
#define CPUINFO_KVP_SETTER_BOOL(n) KVP_SETTER_BOOL(cpuinfo_t, n)

typedef struct _cpuinfo {
    proc_base_t base;
    int cpuid;
    char *vendor_id;
    int cpu_family;
    int model;
    char *model_name;
    int stepping;
    float cpu_mhz;
    int cache_size;
    int physical_id;
    int siblings;
    int core_id;
    int cpu_cores;
    int apicid;
    int initial_apicid;
    int fpu;
    int fpu_exception;
    int cpuid_level;
    int wp;
    char *flags;
    float bogomips;
    int clflush_size;
    int cache_alignment;
    char *address_sizes;
    char *power_management;
} cpuinfo_t;


#define MEMINFO_KVP_SETTER_NAME(n) KVP_SETTER_NAME(meminfo_t, n)
#define MEMINFO_KVP_SETTER_LONG(n) KVP_SETTER_LONG(meminfo_t, n)
#define MEMINFO_KVP_SETTER_LONG_BU(n) KVP_SETTER_LONG_BU(meminfo_t, n)
#define MEMINFO_KVP_SETTER_STR(n) KVP_SETTER_STR(meminfo_t, n)
#define MEMINFO_KVP_SETTER_FLOAT(n) KVP_SETTER_FLOAT(meminfo_t, n)
#define MEMINFO_KVP_SETTER_BOOL(n) KVP_SETTER_BOOL(meminfo_t, n)

typedef struct _meminfo {
    proc_base_t base;
    long mem_total;
    long mem_free;
    long buffers;
    long cached;
    long swap_cached;
    long active;
    long inactive;
    long active_anon;
    long inactive_anon;
    long active_file;
    long inactive_file;
    long unevictable;
    long mlocked;
    long swap_total;
    long swap_free;
    long dirty;
    long writeback;
    long anon_pages;
    long mapped;
    long shmem;
    long slab;
    long s_reclaimable;
    long s_unreclaim;
    long kernel_stack;
    long page_tables;
    long nfs_unstable;
    long bounce;
    long writeback_tmp;
    long commit_limit;
    long committed_as;
    long vmalloc_total;
    long vmalloc_used;
    long vmalloc_chunk;
    long hardware_corrupted;
    long anon_huge_pages;
    long huge_pages_total;
    long huge_pages_free;
    long huge_pages_reserved;
    long huge_pages_surp;
    long hugepagesize;
    long direct_map_4k;
    long direct_map_2m;
    long direct_map_1g;
} meminfo_t;


typedef struct _proc_stat {
    proc_base_t base;
} proc_stat_t;


typedef struct _proc_stat_cpu {
    long user;
    long nice;
    long system;
    long idle;
    long iowait;
    long irq;
    long softirq;
    long steal;
    long guest;
    long guest_nice;
} proc_stat_cpu_t;


typedef struct _proc_pid_statp {
    proc_base_t base;
    int pid;
    char comm[128];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long priority;
    long nice;
    long num_threads;
    long itrealvalue;
    unsigned long long startitme;
    unsigned long vsize;
    long rss;
    unsigned long rsslim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signals;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;
    unsigned rt_priority;
    unsigned policy;
    unsigned long long delayacct_blkio_ticks;
    unsigned long guest_time;
    long cguest_time;
} proc_pid_statp_t;

typedef struct _proc_pid_statm {
    proc_base_t base;
    long vsize;
    long rss;
    long shared;
    long text;
    long lib;
    long data;
    long dirty;
} proc_pid_statm_t;


#ifdef MNSPINFO_CTX_T_DEFINED
#error "MNSPINFO_CTX_T_DEFINED cannot be defined here"
#endif

typedef struct _mnspinfo_ctx {
    /*
     * public interface
     */
    struct timespec ts0;
    struct timespec ts1;
    uint64_t elapsed; /* msec */

    struct {
        // proc/meminfo MemTotal
        uint64_t realmem;
        // /proc/meminfo DirectMap4k+DirectMap2M
        uint64_t physmem;
        // proc/meminfo MemTotal-[? kernel memory]
        uint64_t usermem;
        long statclock;
        int pagesize;
        int ncpu;

        // proc/meminfo MemTotal-MemFree
        uint64_t memused;
        // proc/meminfo MemFree
        uint64_t memfree;
        // proc/meminfo MemTotal
        uint64_t memavail;
        double mempct;
        double cpupct;
    } sys;

    struct {
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
     * private interface ...
     */
    proc_stat_cpu_t pscpu;
    //proc_pid_statp_t statp;
    //proc_pid_statm_t statm;
    struct rusage ru0;
    struct rusage ru1;
} mnspinfo_ctx_t;
#define MNSPINFO_CTX_T_DEFINED


int parse_kvp(const char *,
                    proc_base_t *,
                    proc_fieldesc_t *,
                    size_t,
                    proc_fieldesc_t *,
                    char,
                    char,
                    parse_record_cb_t,
                    void *);


int parse_cpuinfo(mnspinfo_ctx_t *);
int parse_meminfo(mnspinfo_ctx_t *);
int parse_proc_stat_init(mnspinfo_ctx_t *);
int parse_proc_stat_update(mnspinfo_ctx_t *);
int parse_proc_pid_statp(mnspinfo_ctx_t *);
int parse_proc_pid_statm(mnspinfo_ctx_t *);
int parse_proc_pid_fdinfo(mnspinfo_ctx_t *);
int parse_proc_pid_fd(mnspinfo_ctx_t *);

#ifdef __cplusplus
}
#endif
#endif /* MNSPINFO_LINUX_H_DEFINED */
