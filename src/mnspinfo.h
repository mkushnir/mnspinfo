#ifndef MNSPINFO_H_DEFINED
#define MNSPINFO_H_DEFINED

#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MNSPINFO_CTX_T_DEFINED
typedef struct _mnspinfo_ctx {
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
} mnspinfo_ctx_t;
#define MNSPINFO_CTX_T_DEFINED
#endif

#define MNSPINFO_SELF 1
#define MNSPINFO_TREE 2
mnspinfo_ctx_t *mnspinfo_new(pid_t, unsigned);
int mnspinfo_reinit(mnspinfo_ctx_t *, pid_t, unsigned);

#define MNSPINFO_U0 0x01 /* system constants */
#define MNSPINFO_U1 0x02 /* system cpu and memory usage */
#define MNSPINFO_U2 0x04 /* process resource usage */
#define MNSPINFO_U3 0x08 /* process files and memory */
#define MNSPINFO_U4 0x10 /* process limits */
#define MNSPINFO_UZ 0xffffffff
int mnspinfo_update(mnspinfo_ctx_t *, unsigned);

void mnspinfo_destroy(mnspinfo_ctx_t **);

#ifdef __cplusplus
}
#endif
#endif /* MNSPINFO_H_DEFINED */
