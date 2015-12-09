#ifndef SPINFO_LINUX_H_DEFINED
#define SPINFO_LINUX_H_DEFINED

#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/resource.h>


#ifdef __cplusplus
extern "C" {
#endif

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
