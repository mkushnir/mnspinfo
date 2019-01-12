#include <assert.h>
#include <err.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include <mrkcommon/array.h>
#include <mrkcommon/hash.h>
#include <mrkcommon/bytes.h>
#include <mrkcommon/bytestream.h>
#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>
#include <mrkcommon/traversedir.h>

#include <mncommand.h>
#include <mnspinfo.h>

#include "config.h"
#include "diag.h"


typedef struct _mnprocgauges_ctx {
    mnspinfo_ctx_t *ctx;
} mnprocgauges_ctx_t;


struct _resolve_names_params {
    pid_t mypid;
    /* mnprocgauges_ctx_t */
    mnarray_t ctxes;
    //mnhash_t ctxes;
    /* mnbytes_t * */
    mnarray_t names;
    bool error;
};


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
    printf("%s\n", VERSION);
    exit(0);
}


static int
names_fini_item(mnbytes_t **s)
{
    BYTES_DECREF(s);
    return 0;
}


static int
resolve_name_cb(mnbytes_t **s, const char *p0)
{
    return strstr(p0, (char *)BDATA(*s)) != NULL;
}


static int
mnprocgauges_ctx_fini_item(mnprocgauges_ctx_t *ctx)
{
    mnspinfo_destroy(&ctx->ctx);
    return 0;
}


static void
resolve_name(mnbytes_t *cmdline, pid_t pid, struct _resolve_names_params *params)
{
    void *p0, *p1;
    ssize_t sz = BSZ(cmdline);

    for (p0 = BDATA(cmdline), p1 = memchr(p0, 0, sz); sz > 0 && p1 != NULL;) {
        if (p0 == p1) {
            break;
        }
        //TRACE("a=%s", (char *)p0);
        if (array_traverse(&params->names, (array_traverser_t)resolve_name_cb, p0) != 0) {
            mnprocgauges_ctx_t *ctx;
            if ((ctx = array_incr(&params->ctxes)) == NULL) {
                FAIL("array_incr");
            }
            if ((ctx->ctx = mnspinfo_new(pid, MNSPINFO_SELF)) == NULL) {
                TRACE("mnspinfo_new failed for pid %d", pid);
                (void)array_decr_fast(&params->ctxes);
            }
            //TRACE("added pid %d for %s", pid, p0);
            break;
        }
        sz -= p1 - p0 + 1;
        p0 = p1 + 1;
        p1 = memchr(p0, 0, sz);
    }
}


#define RESOLVE_NAMES_CMDLINESZ 1024
static int
resolve_names_cb(const char *d, struct dirent *de, void *udata)
{
    struct _resolve_names_params *params = udata;

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
resolve_names(struct _resolve_names_params *params)
{
    int res;

    res = traverse_dir_no_recurse("/proc", resolve_names_cb, params);
    return res;
}


static int
update_ctxes_cb(mnprocgauges_ctx_t *ctx, void *udata)
{
    UNUSED struct _resolve_names_params *params = udata;
    TRACE("pid=%d", ctx->ctx->proc.pid);
    return 0;
}


static void
update_ctxes(struct _resolve_names_params *params)
{
    int res;

    res = resolve_names(params);
    TRACE("res=%d", res);
    (void)array_traverse(
            &params->ctxes, (array_traverser_t)update_ctxes_cb, params);
    params->error = false;
}


static int
run_ctxes_cb(mnprocgauges_ctx_t *ctx, void *udata)
{
    UNUSED struct _resolve_names_params *params = udata;
    TRACE("pid=%d", ctx->ctx->proc.pid);
    return 0;
}


static void
run_ctxes(struct _resolve_names_params *params)
{
    (void)array_traverse(
            &params->ctxes, (array_traverser_t)run_ctxes_cb, params);
    if (params->ctxes.elnum == 0) {
        params->error = true;
    }
}


int
main(int argc, char *argv[static argc])
{
    int i;
    int optind;
    mncommand_ctx_t cmdctx;
    struct _resolve_names_params params;

    BYTES_ALLOCA(_help, "help");
    BYTES_ALLOCA(_help_description, "Print this message and exit");
    BYTES_ALLOCA(_version, "version");
    BYTES_ALLOCA(_version_description, "Print version and exit");

    if (mncommand_ctx_init(&cmdctx) != 0) {
        FAIL("mncommand_ctx_init");
    }

    (void)mncommand_ctx_add_cmd(&cmdctx, _help, 'h', 0,
                                _help_description, cmd_help);
    (void)mncommand_ctx_add_cmd(&cmdctx, _version, 'V', 0,
                                _version_description, cmd_version);

    if ((optind = mncommand_ctx_getopt(&cmdctx, argc, argv, NULL)) < 0) {
        errx(optind, "error see ^^^");
    }

    argc -= optind;
    argv += optind;

    /* params */
    if (array_init(&params.ctxes,
                   sizeof(mnprocgauges_ctx_t),
                   0,
                   NULL,
                   (array_finalizer_t)mnprocgauges_ctx_fini_item) != 0) {
        FAIL("array_init");
    }
    if (array_init(&params.names,
                   sizeof(mnbytes_t *),
                   0,
                   NULL,
                   (array_finalizer_t)names_fini_item) != 0) {
        FAIL("array_init");
    }
    params.mypid = getpid();

    for (i = 0; i < argc; ++i) {
        pid_t pid;

        if ((pid = strtol(argv[i], NULL, 10)) > 0) {
            mnprocgauges_ctx_t *ctx;

            if ((ctx = array_incr(&params.ctxes)) == NULL) {
                FAIL("array_incr");
            }
            if ((ctx->ctx = mnspinfo_new(pid, MNSPINFO_SELF)) == NULL) {
                TRACE("mnspinfo_new failed for pid %d", pid);
                (void)array_decr_fast(&params.ctxes);
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
            sleep(2);
            run_ctxes(&params);
        }
    }

    (void)array_fini(&params.ctxes);
    (void)array_fini(&params.names);
    (void)mncommand_ctx_fini(&cmdctx);

    return 0;
}
