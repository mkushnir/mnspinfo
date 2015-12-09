#ifndef SPINFO_H_DEFINED
#define SPINFO_H_DEFINED

#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPINFO_CTX_T_DEFINED
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
#endif

#define SPINFO_SELF 1
#define SPINFO_TREE 2
spinfo_ctx_t *spinfo_new(pid_t, unsigned);
void spinfo_reinit(spinfo_ctx_t *, pid_t, unsigned);

#define SPINFO_U0 0x01 /* system constants */
#define SPINFO_U1 0x02 /* system cpu and memory usage */
#define SPINFO_U2 0x04 /* process resource usage */
#define SPINFO_U3 0x08 /* process files and memory */
#define SPINFO_U4 0x10 /* process limits */
#define SPINFO_UZ 0xffffffff
void spinfo_update(spinfo_ctx_t *, unsigned);

void spinfo_destroy(spinfo_ctx_t **);

#ifdef __cplusplus
}
#endif
#endif /* SPINFO_H_DEFINED */
