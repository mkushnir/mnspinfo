#ifndef SPINFO_PRIVATE_H_DEFINED
#define SPINFO_PRIVATE_H_DEFINED

#include <unistd.h>

#ifdef __FreeBSD__
#include "spinfo_freebsd.h"
#endif

#ifdef __linux__
#include "spinfo_linux.h"
#endif

#define TIMESPEC_TO_NSEC(ts) ((ts).tv_sec * 1000000000 + (ts).tv_nsec)
#define TIMEVAL_TO_USEC(ts) ((ts).tv_sec * 1000000 + (ts).tv_usec)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * platform independent private interface
 */

void spinfo_init(spinfo_ctx_t *, pid_t, unsigned);
void spinfo_update0(spinfo_ctx_t *);
void spinfo_update1(spinfo_ctx_t *);
void spinfo_update2(spinfo_ctx_t *);
void spinfo_update3(spinfo_ctx_t *);
void spinfo_update4(spinfo_ctx_t *);
void spinfo_fini(spinfo_ctx_t *);

#ifdef __cplusplus
}
#endif

#include <spinfo.h>
#endif /* SPINFO_PRIVATE_H_DEFINED */
