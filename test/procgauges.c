#include <assert.h>
#include <err.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h> //intmax_t
#include <inttypes.h> //PRId

#include <mncommon/array.h>
#include <mncommon/hash.h>
#include <mncommon/bytes.h>
#include <mncommon/bytestream.h>
#include <mncommon/dumpm.h>
#include <mncommon/util.h>
#include <mncommon/traversedir.h>

#include <mncommand.h>
#include "procgauges.h"

#include "config.h"
#include "diag.h"


/*
 * command line
 */
static int
cmd_help(mncommand_ctx_t *ctx,
         UNUSED mncommand_cmd_t *cmd,
         UNUSED const char *optarg,
         UNUSED void *udata)
{
    mnbytestream_t bs;

    bytestream_init(&bs, 1024);

    mncommand_ctx_format_help(ctx, &bs);
    printf("%s", SDATA(&bs, 0));
    bytestream_fini(&bs);
    exit(0);
}


static int
cmd_version(UNUSED mncommand_ctx_t *ctx,
            UNUSED mncommand_cmd_t *cmd,
            UNUSED const char *optarg,
            UNUSED void *udata)
{
    printf("%s\n", PACKAGE_STRING);
    exit(0);
}


static int
cmd_interval(UNUSED mncommand_ctx_t *ctx,
             UNUSED mncommand_cmd_t *cmd,
             UNUSED const char *optarg,
             void *udata)
{
    resolve_names_params_t *params = udata;
    if ((params->interval = strtol(optarg, NULL, 10)) <= 0) {
        TRACE("invalid interval: %d, forcing default", params->interval);
        params->interval = PROCGAUGES_DEFAULT_INTERVAL;
    }
    return 0;
}


/*
 * mnprocgauges_ctx_t
 */

static mnprocgauges_ctx_t *
mnprocgauges_ctx_new(pid_t pid, mnbytes_t *suffix)
{
    mnprocgauges_ctx_t *res;
    char buf[MAXNAMLEN + 1];

    if (MNUNLIKELY((res = malloc(sizeof(mnprocgauges_ctx_t))) == NULL)) {
        FAIL("malloc");
    }

    if ((res->spi = mnspinfo_new(pid, MNSPINFO_SELF)) == NULL) {
        goto err;
    }

    res->pid = pid;

    if (suffix != NULL) {
        (void)snprintf(buf, sizeof(buf), "procgauges-%s-%d", BDATA(suffix), pid);
    } else {
        (void)snprintf(buf, sizeof(buf), "procgauges-%d", pid);
    }

    if ((res->fd = open(buf, O_WRONLY|O_APPEND|O_CREAT, 0644)) < 0) {
        goto err;
    }

end:
    return res;

err:
    free(res);
    res = NULL;
    goto end;
}


static void
mnprocgauges_ctx_destroy(mnprocgauges_ctx_t **ctx)
{
    if (*ctx != NULL) {
        mnspinfo_destroy(&(*ctx)->spi);
        if ((*ctx)->fd != -1) {
            close((*ctx)->fd);
        }
        free(*ctx);
        *ctx = NULL;
    }
}


static int
mnprocgauges_ctx_hash_item_fini(mnprocgauges_ctx_t *key, UNUSED void *value)
{
    mnprocgauges_ctx_destroy(&key);
    return 0;
}


static uint64_t
mnprocgauges_ctx_hash(mnprocgauges_ctx_t *ctx)
{
    return (uint64_t)ctx->pid;
}


static int
mnprocgauges_ctx_cmp(mnprocgauges_ctx_t *a, mnprocgauges_ctx_t *b)
{
    return MNCMP(a->pid, b->pid);
}


/*
 *
 */
static int
names_fini_item(mnbytes_t **s)
{
    BYTES_DECREF(s);
    return 0;
}


static int
resolve_name_cb(mnbytes_t **s, void *udata)
{
    struct {
        void *p0;
        void *p1;
        ssize_t sz;
        mnbytes_t *hit;
    } *match = udata;
    int res = strstr(match->p0, BCDATA(*s)) != NULL;
    if (res) {
        match->hit = *s;
    }
    return res;
}


static void
resolve_name(mnbytes_t *cmdline, pid_t pid, resolve_names_params_t *params)
{
    struct {
        void *p0;
        void *p1;
        ssize_t sz;
        mnbytes_t *hit;
    } match;

    //TRACE("cmdline=%s", BDATA(cmdline));
    match.hit = NULL;
    match.sz = BSZ(cmdline);
    for (match.p0 = BDATA(cmdline),
            match.p1 = memchr(match.p0, 0, match.sz);
         match.sz > 0 && match.p1 != NULL;) {
        if (match.p0 == match.p1) {
            break;
        }

        if (array_traverse(&params->names,
                           (array_traverser_t)resolve_name_cb,
                           &match) != 0) {
            mnhash_item_t *hit;

            assert(match.hit != NULL);
            if ((hit = hash_get_item(&params->ctxes,
                                     &(mnprocgauges_ctx_t){.pid = pid})) == NULL) {
                mnprocgauges_ctx_t *ctx;

                if ((ctx = mnprocgauges_ctx_new(pid, match.hit)) == NULL) {
                    TRACE("mnprocgauges_ctx_new failed for pid %d, ignoring",
                            pid);
                } else {
                    //TRACE("added %d", pid);
                    hash_set_add(&params->ctxes, ctx);
                }

            } else {
                TRACE("duplicate pid %d for %s", pid, BDATA(cmdline));
            }
            /*
             * matched, done
             */
            //TRACE("matched: %s", (char *)match.p0);
            break;

        } else {
            //TRACE("didn't match: %s", (char *)match.p0);
        }

        match.sz -= match.p1 - match.p0 + 1;
        match.p0 = match.p1 + 1;
        match.p1 = memchr(match.p0, 0, match.sz);
    }
}


#define RESOLVE_NAMES_CMDLINESZ 1024
static int
resolve_names_cb(const char *d, struct dirent *de, void *udata)
{
    resolve_names_params_t *params = udata;

    //TRACE("d=%s de=%p", d, de);
    if (de != NULL) {
        pid_t pid;
        //TRACE(" name=%s", de->d_name);

        if ((pid = (pid_t)strtol(de->d_name, NULL, 10)) > 0) {
            int fd;
            char buf[MAXNAMLEN + 1];

            if (pid == params->mypid) {
                goto end;
            }

            (void)snprintf(buf, sizeof(buf), "%s/cmdline", d);
            if ((fd = open(buf, O_RDONLY)) >= 0) {
                struct stat sb;

                if (fstat(fd, &sb) == 0) {
                    mnbytes_t *cmdline;

                    cmdline = bytes_new(RESOLVE_NAMES_CMDLINESZ);
                    BYTES_INCREF(cmdline);
                    bytes_memset(cmdline, 0);

                    if (read(fd, BDATA(cmdline), BSZ(cmdline)) >= 0) {
                        resolve_name(cmdline, pid, params);
                    } else {
                        //TRACE("cannot read %d", fd);
                    }
                    BYTES_DECREF(&cmdline);

                } else {
                    //TRACE("cannot fstat %s", buf);
                }
                close(fd);
            } else {
                //TRACE("cannot open %s", buf);
            }
        } else {
            //TRACE("zero pid for %s", de->d_name);
        }
    }

end:
    return 0;
}


static int
resolve_names(resolve_names_params_t *params)
{
    int res;

    res = traverse_dir_no_recurse("/proc", resolve_names_cb, params);
    return res;
}


static int
update_ctxes_cb(UNUSED mnhash_t *hash, mnhash_item_t *hit, void *udata)
{
    UNUSED mnprocgauges_ctx_t *ctx = hit->key;
    UNUSED resolve_names_params_t *params = udata;

    //TRACE("pid=%d", ctx->spi->proc.pid);
    return 0;
}


static void
update_ctxes(resolve_names_params_t *params)
{
    UNUSED int res;

    res = resolve_names(params);
    //TRACE("res=%d", res);
    (void)hash_traverse_item(
            &params->ctxes, (hash_traverser_item_t)update_ctxes_cb, params);
    params->error = false;
}


static int
run_ctxes_cb(mnhash_t *hash, mnhash_item_t *hit, UNUSED void *udata)
{
    mnprocgauges_ctx_t *ctx = hit->key;

    assert(ctx->pid > 0);
    assert(ctx->fd >= 0);

    //if (mnspinfo_update(ctx->spi, 0) != 0) {
    if (mnspinfo_update(ctx->spi, MNSPINFO_U2|MNSPINFO_U3) != 0) {
        TRACE("mnspinfo_update failed");
        hash_delete_pair(hash, hit);

    } else {
        char buf[1024];
        int nwritten;

        if ((nwritten = snprintf(
                buf,
                sizeof(buf),
                "%jd %lf %"PRIu64" %"PRIu64" %"PRIu64"\n",
                (intmax_t)ctx->spi->ts1.tv_sec,
                ctx->spi->proc.cpupct,
                ctx->spi->proc.rss,
                ctx->spi->proc.nfiles,
                ctx->spi->proc.nsockets)) > 0) {
            ssize_t n;

            if ((n = write(ctx->fd, buf, nwritten)) <= 0) {
                TRACE("failed to write to %d", ctx->fd);
            }
        }

        TRACE("%d %jd %lf %"PRIu64" %"PRIu64" %"PRIu64,
              ctx->pid,
              (intmax_t)ctx->spi->ts1.tv_sec,
              ctx->spi->proc.cpupct,
              ctx->spi->proc.rss,
              ctx->spi->proc.nfiles,
              ctx->spi->proc.nsockets);

    }
    return 0;
}


static void
run_ctxes(resolve_names_params_t *params)
{
    (void)hash_traverse_item(
            &params->ctxes, (hash_traverser_item_t)run_ctxes_cb, params);
    if (hash_get_elnum(&params->ctxes) == 0) {
        params->error = true;
    }
}


int
main(int argc, char *argv[static argc])
{
    int i;
    int optind;
    mncommand_ctx_t cmdctx;
    resolve_names_params_t params = {
        .interval = PROCGAUGES_DEFAULT_INTERVAL,
        .error = false,
    };

    if (mncommand_ctx_init(&cmdctx) != 0) {
        FAIL("mncommand_ctx_init");
    }

    (void)mncommand_ctx_add_cmd(&cmdctx, BYTES_REF("help"), 'h', 0,
                                BYTES_REF("Print this message and exit."),
                                cmd_help, NULL);
    (void)mncommand_ctx_add_cmd(&cmdctx, BYTES_REF("version"), 'V', 0,
                                BYTES_REF("Print version and exit."),
                                cmd_version, NULL);
    (void)mncommand_ctx_add_cmd(&cmdctx, BYTES_REF("interval"), 't', 1,
                                BYTES_REF("Print version and exit."),
                                cmd_interval, NULL);

    if ((optind = mncommand_ctx_getopt(&cmdctx, argc, argv, &params)) < 0) {
        errx(optind, "error see ^^^");
    }

    argc -= optind;
    argv += optind;

    /* params */
    hash_init(&params.ctxes, 31,
              (hash_hashfn_t)mnprocgauges_ctx_hash,
              (hash_item_comparator_t)mnprocgauges_ctx_cmp,
              (hash_item_finalizer_t)mnprocgauges_ctx_hash_item_fini);

    if (array_init(&params.names,
                   sizeof(mnbytes_t *),
                   0,
                   NULL,
                   (array_finalizer_t)names_fini_item) != 0) {
        FAIL("array_init");
    }
    params.mypid = getpid();

    for (i = 0; i < argc; ++i) {
        mnprocgauges_ctx_t probe;

        if ((probe.pid = strtol(argv[i], NULL, 10)) > 0) {
            mnhash_item_t *hit;

            if ((hit = hash_get_item(&params.ctxes, &probe)) == NULL) {
                mnprocgauges_ctx_t *ctx;

                if ((ctx = mnprocgauges_ctx_new(probe.pid, NULL)) == NULL) {
                    TRACE("mnprocgauges_ctx_new failed for pid %d, ignoring",
                            probe.pid);
                    continue;
                }
                hash_set_add(&params.ctxes, ctx);

            } else {
                TRACE("duplicate pid %d, ignoring", probe.pid);
                continue;
            }

        } else {
            mnbytes_t **name;

            if ((name = array_incr(&params.names)) == NULL) {
                FAIL("array_incr");
            }
            *name = bytes_new_from_str(argv[i]);
            BYTES_INCREF(*name);
        }
    }

    update_ctxes(&params);

    while (true) {
        if (params.error) {
            sleep(5);
            update_ctxes(&params);
        } else {
            sleep(params.interval);
            run_ctxes(&params);
        }
    }

    hash_fini(&params.ctxes);
    (void)array_fini(&params.names);
    (void)mncommand_ctx_fini(&cmdctx);

    return 0;
}
