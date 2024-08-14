
#ifndef _GUSIPOSIX_
#define _GUSIPOSIX_

#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utime.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <MacTypes.h>

__BEGIN_DECLS
int remove(const char * path);
int rename(const char *oldname, const char *newname);
int fgetfileinfo(const char * path, OSType * creator, OSType * type);
void fsetfileinfo(const char * path, OSType creator, OSType type);
time_t time(time_t * timer);
struct tm * localtime(const time_t * timer);
struct tm * gmtime(const time_t * timer);
time_t mktime(struct tm *timeptr);
__END_DECLS

#endif /* _GUSIPOSIX_ */
