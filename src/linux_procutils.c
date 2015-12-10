#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <fcntl.h>

#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "spinfo_private.h"
#include "diag.h"

#define ISSPACE(c) (c != '\0' && (c == ' ' || c == '\t'))
static char *
reach_nonspace(char *s)
{
    while (ISSPACE(*s)) {
        ++s;
    }
    return s;
}


static char *
reach_delim(char *s, char delim)
{
    while (*s != delim && *s != '\0') {
        ++s;
    }
    return s;
}


static void
rtrim_space(char *start, char *end)
{
    while (end > start) {
        if (ISSPACE(*(end - 1))) {
            --end;
        } else {
            break;
        }
    }
    *end = '\0';
}


/*
 * one field per line
 */
int parse_kvp(const char *path,
              proc_base_t *proc,
              UNUSED proc_fieldesc_t *fdesc,
              UNUSED size_t nfdesc,
              UNUSED proc_fieldesc_t *eor,
              UNUSED char fdelim,
              UNUSED char rdelim,
              UNUSED parse_record_cb_t rcb,
              UNUSED void *udata)
{
    int res;
    char *pos;
    ssize_t sz;
    ssize_t nread;

    res = 0;

    if ((proc->fd = open(path, O_RDONLY)) == -1) {
        TRRET(PARSE_KVP + 1);
    }

    proc->buf = NULL;
    sz = 0;
    do {
        proc->buf = realloc(proc->buf, sz + 4096);
        nread = read(proc->fd, proc->buf + sz, 4096);
        sz += nread;
    } while (nread > 0);

    proc->buf[sz - 1] = '\0';

    for (pos = proc->buf; pos < (proc->buf + sz);) {
        char *kstart, *kend;
        char *vstart, *vend;
        size_t i;

        /* key */
        kstart = reach_nonspace(pos);
        if (*kstart == '\0') {
            goto end;
        }
        kend = reach_delim(kstart, fdelim);
        if (kend == kstart) {
            /* empty key */
            res = PARSE_KVP + 4;
            goto end;
        }
        rtrim_space(kstart, kend);
        ++kend;
        /* value */
        vstart = reach_nonspace(kend);
        vend = reach_delim(vstart, rdelim);
        rtrim_space(vstart, vend);
        ++vend;

        for (i = 0; i < nfdesc; ++i) {
            proc_fieldesc_t *f = &fdesc[i];

            if (strcmp(kstart, f->name) == 0) {
                if (f->cb(proc, f, vstart, udata) != 0) {
                    res = PARSE_KVP + 5;
                    goto end;
                }
                if (eor == f) {
                    if (rcb(proc, udata) != 0) {
                        res = PARSE_KVP + 6;
                        goto end;
                    }
                }
            }
        }
        pos = vend;
    }

end:
    //if (rcb(proc, udata) != 0) {
    //    res = PARSE_KVP + 7;
    //}
    (void)close(proc->fd);
    proc->fd = -1;
    if (proc->buf != NULL) {
        free(proc->buf);
        proc->buf = NULL;
    }
    TRRET(res);
}
