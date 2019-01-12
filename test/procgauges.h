#ifndef MNSPINFO_PROCGAUGES_H_DEFINED
#define MNSPINFO_PROCGAUGES_H_DEFINED

#include <stdbool.h>
#include <unistd.h> //pid_t

#include <mrkcommon/array.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/hash.h>

#include <mnspinfo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mnprocgauges_ctx {
    mnspinfo_ctx_t *spi;
    pid_t pid;
    int fd;
} mnprocgauges_ctx_t;


typedef struct _resolve_names_params {
    pid_t mypid;
    /* mnbytes_t * */
    mnarray_t names;
    /* mnprocgauges_ctx_t *, NULL */
    mnhash_t ctxes;
#define PROCGAUGES_DEFAULT_INTERVAL (2)
    int interval;
    bool error;
} resolve_names_params_t;


#ifdef __cplusplus
}
#endif
#endif /* MNSPINFO_PROCGAUGES_H_DEFINED */
