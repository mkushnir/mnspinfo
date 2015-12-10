#ifndef SPINFO_LINUX_H_DEFINED
#define SPINFO_LINUX_H_DEFINED

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>


/*
 * Linux specific private interface
 */

#ifdef __cplusplus
extern "C" {
#endif

#define KVP_SETTER_NAME(ty, name) set_##ty##_##name

#define KVP_SETTER_INT(ty, n)                          \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    sscanf(val, "%d", &f->n);                          \
    return 0;                                          \
}                                                      \


#define KVP_SETTER_STR(ty, n)                          \
static int                                             \
KVP_SETTER_NAME(ty, n)(                                \
        proc_base_t *proc,                             \
        UNUSED proc_fieldesc_t *fdesc,                 \
        char *val,                                     \
        UNUSED void *udata)                            \
{                                                      \
    ty *f = (ty *)proc;                                \
/*    TRACE("key=%s val=%s", fdesc->name, val); */     \
    f->n = val;                                        \
    return 0;                                          \
}                                                      \


struct _proc_fieldesc;
struct _proc_base;

typedef int (*parse_field_cb_t)(struct _proc_base *, struct _proc_fieldesc *, char *, void *);
typedef int (*parse_record_cb_t)(struct _proc_base *, void *);

typedef struct _proc_fieldesc {
    char *name;
    parse_field_cb_t cb;
} proc_fieldesc_t;

typedef struct _proc_base {
    int fd;
    char *buf;
} proc_base_t;

typedef struct _proc_meminfo {
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
} proc_meminfo_t;

typedef struct _cpu_info {
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
    int cpiid_level;
    int wp;
    char *flags;
    float bogomips;
    int clflush_size;
    int cache_alignment;
    char *address_sizes;
    char *power_management;
} cpu_info_t;


int parse_kvp(const char *,
                    proc_base_t *,
                    proc_fieldesc_t *,
                    size_t,
                    proc_fieldesc_t *,
                    char,
                    char,
                    parse_record_cb_t,
                    void *);


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
     * private interface ...
     */
} spinfo_ctx_t;
#define SPINFO_CTX_T_DEFINED

#ifdef __cplusplus
}
#endif
#endif /* SPINFO_LINUX_H_DEFINED */
