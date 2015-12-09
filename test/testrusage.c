#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>


#include <mrkcommon/dumpm.h>
#include <mrkcommon/util.h>

#include "diag.h"
#include "unittest.h"

static void
bar(void)
{
    static char *s;
    size_t sz, i;

    if (s != NULL) {
        free(s);
    }
    sz = random() % (1024 * 1024 * 1024);
    TRACE("sz=%ld", sz);
    s = malloc(sz);
    for (i = 0; i < sz; ++i) {
        if (i % 4096 == 0) {
            s[i] = (char)(random() % 256);
        }
    }
}


int
main(void)
{
    while (1) {
        struct rusage ru;
        double utime, stime;

        if (getrusage(RUSAGE_SELF, &ru) != 0) {
            FAIL("getrusage");
        }
        utime = (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000.;
        stime = (double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000.;
        TRACE("%lf %lf %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
              utime,
              stime,
              ru.ru_maxrss * 4,
              ru.ru_ixrss * 4,
              ru.ru_idrss * 4,
              ru.ru_isrss * 4,
              ru.ru_minflt,
              ru.ru_majflt,
              ru.ru_nswap,
              ru.ru_inblock,
              ru.ru_oublock,
              ru.ru_nvcsw,
              ru.ru_nivcsw
                );
        if (utime == 0.0) {
            sleep(1);
        } else {
            sleep(1);
        }
        bar();
    }
    return 0;
}
