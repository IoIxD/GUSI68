/*
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * [�3 Deleted as of 22Jul99, see
 *     ftp://ftp.cs.berkeley.edu/pub/4bsd/README.Impt.License.Change
 *	   for details]
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)time.h	8.1 (Berkeley) 6/2/93
 */

/* Adapted for GUSI by Matthias Neeracher <neeri@iis.ee.ethz.ch> */

#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#include <time.h>
#include <sys/types.h>

struct timezone
{
	int tz_minuteswest; /* minutes west of Greenwich */
	int tz_dsttime;		/* type of dst correction */
};
#define DST_NONE 0 /* not on dst */
#define DST_USA 1  /* USA style dst */
#define DST_AUST 2 /* Australian style dst */
#define DST_WET 3  /* Western European dst */
#define DST_MET 4  /* Middle European dst */
#define DST_EET 5  /* Eastern European dst */
#define DST_CAN 6  /* Canada */

/* Operations on timevals. */
#define timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0
#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
	(((tvp)->tv_sec == (uvp)->tv_sec) ? ((tvp)->tv_usec cmp(uvp)->tv_usec) : ((tvp)->tv_sec cmp(uvp)->tv_sec))

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define ITIMER_REAL 0
#define ITIMER_VIRTUAL 1
#define ITIMER_PROF 2

struct itimerval
{
	struct timeval it_interval; /* timer interval */
	struct timeval it_value;	/* current value */
};

/*
 * Getkerninfo clock information structure
 */
struct clockinfo
{
	int hz;		/* clock frequency */
	int tick;	/* micro-seconds per hz tick */
	int stathz; /* statistics clock frequency */
	int profhz; /* profiling clock frequency */
};

#define NBBY 8 /* number of bits in a byte */

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#include <time.h>

#include <sys/cdefs.h>

__BEGIN_DECLS
int getitimer(int, struct itimerval *);
int gettimeofday(struct timeval *, struct timezone *);
int setitimer(int, const struct itimerval *, struct itimerval *);
int utimes(const char *, const struct timeval *);
int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
__END_DECLS

#endif /* !_SYS_TIME_H_ */
