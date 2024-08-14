
#include "GUSIInternal.h"
#include "GUSIPOSIX.h"
#include "GUSISocket.h"
#include "GUSIFactory.h"
#include "GUSIDevice.h"
#include "GUSIDescriptor.h"
#include "GUSIPipe.h"
#include "GUSIConfig.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"
#include "GUSINetDB.h"
#include "GUSITimer.h"

#include <LowMem.h>

int pipe(int *fd)
{
	GUSIErrorSaver saveError;
	GUSISocketFactory *factory = GUSIPipeFactory::Instance();
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();
	GUSISocket *sock[2];

	if (!factory->socketpair(0, 0, 0, sock))
	{
		if ((fd[0] = table->InstallSocket(sock[0])) > -1)
			if ((fd[1] = table->InstallSocket(sock[1])) > -1)
			{
				shutdown(fd[0], 1);
				shutdown(fd[1], 0);

				return 0;
			}
			else
				table->RemoveSocket(fd[0]);
		sock[0]->close();
		sock[1]->close();
	}
	if (!errno)
		return GUSISetPosixError(ENOMEM);
	else
		return -1;
}

int fsync(int s)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->fsync();
}

int close(int s)
{
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

	return table->RemoveSocket(s);
}

ssize_t read(int s, void *buffer, size_t buflen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->read(buffer, buflen);
}

static ssize_t HandleWriteErrors(ssize_t retval)
{
	if (retval == -1)
		switch (errno)
		{
		case EINTR:
		case EWOULDBLOCK:
		case EINPROGRESS:
		case EALREADY:
			break;
		default:
			GUSIConfiguration::Instance()->BrokenPipe();
			break;
		}

	return retval;
}

ssize_t write(int s, const void *buffer, size_t buflen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return HandleWriteErrors(sock->write(buffer, buflen));
}

int fcntl(int s, int cmd, ...)
{
	va_list arglist;
	va_start(arglist, cmd);

	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	if (cmd == F_DUPFD)
	{
		GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

		return table->InstallSocket(sock, va_arg(arglist, int));
	}
	else
		return sock->fcntl(cmd, arglist);
}

int dup(int s)
{
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return table->InstallSocket(sock);
}

int dup2(int s, int s1)
{
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	table->RemoveSocket(s1);
	return table->InstallSocket(sock, s1);
}

int fstat(int s, struct stat *buf)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->fstat(buf);
}

off_t lseek(int s, off_t offset, int whence)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->lseek(offset, whence);
}

int isatty(int s)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->isatty();
}

u_int sleep(u_int seconds)
{
	if (!seconds)
	{
		GUSIContext::Yield(kGUSIYield);
		GUSIContext::Raise();

		return 0;
	}
	GUSITime start(GUSITime::Now());
	GUSITimer *timer = new GUSITimer;
	GUSIContext::CreateCurrent()->ClearWakeups();
	timer->Sleep(GUSITime(static_cast<uint32_t>(seconds), GUSITime::seconds).Get(GUSITime::msecs));
	GUSIContext::Yield(kGUSIBlock);
	delete timer;
	GUSIContext::Raise();

	seconds -= (GUSITime::Now() - start).Get(GUSITime::seconds);

	return (seconds & 0x80000000) ? 0 : seconds;
}

int open(const char *path, int mode, ...)
{
	GUSIErrorSaver saveError;
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();
	GUSISocket *sock;
	int fd;

	if (sock = factory->open(path, mode))
	{
		if ((fd = table->InstallSocket(sock)) > -1)
			return fd;
		sock->close();
	}
	if (!errno)
		return GUSISetPosixError(ENOMEM);
	else
		return -1;
}

int creat(const char *path, ...)
{
	return open(path, O_WRONLY | O_TRUNC | O_CREAT);
}

int remove(const char *path)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->remove(path);
}

int unlink(const char *path)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->remove(path);
}

int rename(const char *oldname, const char *newname)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->rename(oldname, newname);
}

int stat(const char *path, struct stat *buf)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->stat(path, buf, false);
}

int lstat(const char *path, struct stat *buf)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->stat(path, buf, true);
}

int chmod(const char *path, mode_t mode)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->chmod(path, mode);
}

int utime(const char *path, const utimbuf *times)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->utime(path, times);
}

int access(const char *path, int mode)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->access(path, mode);
}

int mkdir(const char *path, ...)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->mkdir(path);
}

int rmdir(const char *path)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->rmdir(path);
}

typedef GUSIDirectory *GUSIDirPtr;

DIR *opendir(const char *path)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	GUSIDirectory *dir = factory->opendir(path);

	return dir ? reinterpret_cast<DIR *>(new GUSIDirPtr(dir)) : 0;
}

dirent *readdir(DIR *dir)
{
	return GUSIDirPtr(*dir)->readdir();
}

long telldir(DIR *dir)
{
	return GUSIDirPtr(*dir)->telldir();
}

void seekdir(DIR *dir, long pos)
{
	GUSIDirPtr(*dir)->seekdir(pos);
}

void rewinddir(DIR *dir)
{
	GUSIDirPtr(*dir)->seekdir(1);
}

int closedir(DIR *dir)
{
	delete GUSIDirPtr(*dir);
	delete dir;

	return 0;
}

int chdir(const char *dir)
{
	GUSIFileSpec directory(dir);

	if (directory.Error())
		return GUSISetMacError(directory.Error());
	else if (!directory.Exists())
		return GUSISetPosixError(ENOENT);

	(++directory).SetDefaultDirectory();

	return 0;
}

char *getcwd(char *buf, size_t size)
{
	GUSIFileSpec cwd;
	char *res;

	if (cwd.GetDefaultDirectory())
		return GUSISetMacError(cwd.Error()), static_cast<char *>(nil);

	res = (--cwd).FullPath();

	if (size < strlen(res) + 1)
		return GUSISetPosixError(size > 0 ? ERANGE : EINVAL),
			   static_cast<char *>(nil);
	if (!buf && !(buf = (char *)malloc(size)))
		return GUSISetPosixError(ENOMEM), static_cast<char *>(nil);

	strcpy(buf, res);

	return buf;
}

time_t time(time_t *timer)
{
	time_t t;

	if (!timer)
		timer = &t;

	GetDateTime(reinterpret_cast<unsigned long *>(timer));

	return *timer;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	*tv = (timeval)GUSITime::Now();
	if (tz)
		*tz = GUSITime::Zone();

	return 0;
}

extern "C" void GUSIKillTM(void *t)
{
	delete reinterpret_cast<tm *>(t);
}

static tm *get_tm()
{
	static GUSISpecificData<tm, GUSIKillTM> sTM;

	return sTM.get();
}

struct tm *localtime(const time_t *timer)
{
	tm *t = get_tm();
	*t = GUSITime(static_cast<unsigned long>(*timer), GUSITime::seconds);
	t->tm_isdst = GUSITime::Zone().tz_dsttime != 0;
	return t;
}

struct tm *gmtime(const time_t *timer)
{
	tm *t = get_tm();
	*t = GUSITime(static_cast<unsigned long>(*timer), GUSITime::seconds).Local2GMTime();
	t->tm_isdst = 0;
	return t;
}

time_t mktime(struct tm *timeptr)
{
	return static_cast<time_t>(GUSITime(*timeptr).UGet(GUSITime::seconds));
}

int getdtablesize()
{
	return GUSIDescriptorTable::SIZE;
}

int socket(int domain, int type, int protocol)
{
	GUSIErrorSaver saveError;
	GUSISocketFactory *factory = GUSISocketDomainRegistry::Instance();
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();
	GUSISocket *sock;
	int fd;

	if (sock = factory->socket(domain, type, protocol))
	{
		if ((fd = table->InstallSocket(sock)) > -1)
			return fd;
		sock->close();
	}
	if (!errno)
		return GUSISetPosixError(ENOMEM);
	else
		return -1;
}

int socketpair(int domain, int type, int protocol, int *sv)
{
	GUSIErrorSaver saveError;
	GUSISocketFactory *factory = GUSISocketDomainRegistry::Instance();
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();
	GUSISocket *sock[2];

	if (!factory->socketpair(domain, type, protocol, sock))
	{
		if ((sv[0] = table->InstallSocket(sock[0])) > -1)
			if ((sv[1] = table->InstallSocket(sock[1])) > -1)
				return 0;
			else
				table->RemoveSocket(sv[0]);
		sock[0]->close();
		sock[1]->close();
	}
	if (!errno)
		return GUSISetPosixError(ENOMEM);
	else
		return -1;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);
	return sock->bind((void *)name, namelen);
}

int connect(int s, const struct sockaddr *addr, socklen_t addrlen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);
	return sock->connect((void *)addr, addrlen);
}

int listen(int s, int qlen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);
	return sock->listen(qlen);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	if (sock = sock->accept(addr, addrlen))
		if ((s = table->InstallSocket(sock)) != -1)
			return s;
		else
			sock->close();

	return -1;
}

ssize_t readv(int s, const struct iovec *iov, int count)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->read(GUSIScatterer(iov, count));
}

ssize_t recv(int s, void *buffer, size_t buflen, int flags)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	socklen_t fromlen = 0;

	return sock->recvfrom(buffer, buflen, flags, nil, &fromlen);
}

ssize_t recvfrom(
	int s, void *buffer, size_t buflen, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->recvfrom(buffer, buflen, flags, from, fromlen);
}

ssize_t recvmsg(int s, struct msghdr *msg, int flags)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->recvmsg(msg, flags);
}

ssize_t writev(int s, const struct iovec *iov, int count)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return HandleWriteErrors(sock->write(GUSIGatherer(iov, count)));
}

ssize_t send(int s, const void *buffer, size_t buflen, int flags)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return HandleWriteErrors(sock->sendto(buffer, buflen, flags, nil, 0));
}

ssize_t sendto(
	int s, const void *buffer, size_t buflen, int flags, const struct sockaddr *to, socklen_t tolen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return HandleWriteErrors(sock->sendto(buffer, buflen, flags, to, tolen));
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return HandleWriteErrors(sock->sendmsg(msg, flags));
}

static int select_once(int width,
					   fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
					   fd_set *readres, fd_set *writeres, fd_set *exceptres)
{
	bool r, w, e;
	bool *canRead;
	bool *canWrite;
	bool *canExcept;
	int count = 0;

	for (int s = 0; s < width; ++s)
	{
		canRead = (readfds && FD_ISSET(s, readfds)) ? &r : nil;
		canWrite = (writefds && FD_ISSET(s, writefds)) ? &w : nil;
		canExcept = (exceptfds && FD_ISSET(s, exceptfds)) ? &e : nil;
		if (canRead || canWrite || canExcept)
		{
			GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

			if (!GUSI_ASSERT_EXTERNAL(sock, ("Socket %d closed in select\n", s)))
				return count ? count : -1;

			r = w = e = false;
			if (sock->select(canRead, canWrite, canExcept))
				count += (canRead && *canRead) + (canWrite && *canWrite) + (canExcept && *canExcept);
			if (r)
				FD_SET(s, readres);
			if (w)
				FD_SET(s, writeres);
			if (e)
				FD_SET(s, exceptres);
		}
	}

	return count;
}

static bool select_sleep(bool canSleep)
{
	if (canSleep)
	{
		return GUSIContext::Yield(kGUSIBlock);
	}
	else
	{
		return GUSIContext::Yield(kGUSIPoll);
	}
}

static int select_forever(bool canSleep, int width,
						  fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
						  fd_set *readres, fd_set *writeres, fd_set *exceptres)
{
	int count;

	for (;;)
	{
		count =
			select_once(width,
						readfds, writefds, exceptfds,
						readres, writeres, exceptres);
		if (count)
			break;
		if (select_sleep(canSleep))
			return GUSISetPosixError(EINTR);
	}

	return count;
}

static int select_timed(bool canSleep, int width,
						fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
						fd_set *readres, fd_set *writeres, fd_set *exceptres,
						struct timeval *timeout)
{
	int count;
	GUSITimer timer;

	timer.MicroSleep(GUSITime(*timeout).Get(GUSITime::usecs));
	for (;;)
	{
		count =
			select_once(width,
						readfds, writefds, exceptfds,
						readres, writeres, exceptres);
		if (count || timer.Expired())
			break;
		if (select_sleep(canSleep))
			return GUSISetPosixError(EINTR);
	}

	return count;
}

int select(int width, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	bool canSleep = true;
	int count = 0;
	fd_set readres;
	FD_ZERO(&readres);
	fd_set writeres;
	FD_ZERO(&writeres);
	fd_set exceptres;
	FD_ZERO(&exceptres);

	for (int s = 0; s < width; ++s)
		if ((readfds && FD_ISSET(s, readfds)) || (writefds && FD_ISSET(s, writefds)) || (exceptfds && FD_ISSET(s, exceptfds)))
			if (!GUSIDescriptorTable::LookupSocket(s))
				return -1;
	for (int s = 0; s < width; ++s)
		if (GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s))
		{
			bool r = readfds && FD_ISSET(s, readfds);
			bool w = writefds && FD_ISSET(s, writefds);
			bool e = exceptfds && FD_ISSET(s, exceptfds);

			if (r || w || e)
				canSleep = sock->pre_select(r, w, e) && canSleep;
		}
	if (!timeout)
	{
		count =
			select_forever(canSleep, width,
						   readfds, writefds, exceptfds,
						   &readres, &writeres, &exceptres);
	}
	else if (timeout->tv_sec || timeout->tv_usec)
	{
		count =
			select_timed(canSleep, width,
						 readfds, writefds, exceptfds,
						 &readres, &writeres, &exceptres,
						 timeout);
	}
	else
	{
		count =
			select_once(width,
						readfds, writefds, exceptfds,
						&readres, &writeres, &exceptres);
		GUSIContext::Yield(kGUSIYield);
	}
	int saveErrno = errno;
	for (int s = 0; s < width; ++s)
		if (GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s))
		{
			bool r = readfds && FD_ISSET(s, readfds);
			bool w = writefds && FD_ISSET(s, writefds);
			bool e = exceptfds && FD_ISSET(s, exceptfds);

			if (r || w || e)
				sock->post_select(r, w, e);
		}
	errno = saveErrno;
	int nwords = ((width + 31) >> 3) & ~3;
	if (readfds)
		memcpy(readfds, &readres, nwords);
	if (writefds)
		memcpy(writefds, &writeres, nwords);
	if (exceptfds)
		memcpy(exceptfds, &exceptres, nwords);
	GUSIContext::Raise();

	return count;
}

int getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->getsockname(name, namelen);
}

int getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->getpeername(name, namelen);
}

int shutdown(int s, int how)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->shutdown(how);
}

int ioctl(int s, unsigned long request, ...)
{
	va_list arglist;
	va_start(arglist, request);

	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->ioctl(request, arglist);
}

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->getsockopt(level, optname, optval, optlen);
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock->setsockopt(level, optname, (void *)optval, optlen);
}

int ftruncate(int s, off_t offset)
{
	GUSISocket *sock = GUSIDescriptorTable::LookupSocket(s);

	return sock ? sock->ftruncate(offset) : -1;
}

int truncate(const char *path, off_t offset)
{
	int fd = open(path, O_RDWR);
	if (fd < 0)
		return fd;

	int res = ftruncate(fd, offset);

	close(fd);

	return res;
}

int usleep(u_int useconds)
{
	if (!useconds)
	{
		GUSIContext::Yield(kGUSIYield);
		GUSIContext::Raise();

		return 0;
	}
	GUSITimer *timer = new GUSITimer;
	GUSIContext::CreateCurrent()->ClearWakeups();
	timer->MicroSleep(useconds);
	GUSIContext::Yield(kGUSIBlock);
	delete timer;
	GUSIContext::Raise();

	return 0;
}

int symlink(const char *path, const char *linkto)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->symlink(path, linkto);
}

int readlink(const char *path, char *buf, int bufsize)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->readlink(path, buf, bufsize);
}

int inet_aton(const char *addr, struct in_addr *ina)
{
	in_addr_t a = inet_addr(addr);

	if (a == INADDR_NONE)
		return 0;
	if (ina)
		ina->s_addr = a;

	return 1;
}

in_addr_t inet_addr(const char *addr)
{
	return GUSINetDB::Instance()->inet_addr(addr);
}

char *inet_ntoa(in_addr addr)
{
	return GUSINetDB::Instance()->inet_ntoa(addr);
}

long gethostid()
{
	return GUSINetDB::Instance()->gethostid();
}

int gethostname(char *machname, int buflen)
{
	return GUSINetDB::Instance()->gethostname(machname, buflen);
}

void setprotoent(int stayopen)
{
	GUSINetDB::Instance()->setprotoent(stayopen);
}

void setservent(int stayopen)
{
	GUSINetDB::Instance()->setservent(stayopen);
}

void endprotoent()
{
	GUSINetDB::Instance()->endprotoent();
}

void endservent()
{
	GUSINetDB::Instance()->endservent();
}

hostent *gethostbyaddr(const void *addr, size_t size, int family)
{
	return GUSINetDB::Instance()->gethostbyaddr(addr, size, family);
}

hostent *gethostbyname(const char *name)
{
	return GUSINetDB::Instance()->gethostbyname(name);
}

protoent *getprotobyname(const char *name)
{
	return GUSINetDB::Instance()->getprotobyname(name);
}

protoent *getprotobynumber(int proto)
{
	return GUSINetDB::Instance()->getprotobynumber(proto);
}

protoent *getprotoent()
{
	return GUSINetDB::Instance()->getprotoent();
}

servent *getservbyname(const char *name, const char *proto)
{
	return GUSINetDB::Instance()->getservbyname(name, proto);
}

servent *getservbyport(int port, const char *proto)
{
	return GUSINetDB::Instance()->getservbyport(port, proto);
}

servent *getservent()
{
	return GUSINetDB::Instance()->getservent();
}

int fgetfileinfo(const char *path, OSType *creator, OSType *type)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->fgetfileinfo(path, creator, type);
}
int faccess(const char *path, unsigned *cmd, void *arg)
{
	GUSIDeviceRegistry *factory = GUSIDeviceRegistry::Instance();

	return factory->faccess(path, cmd, arg);
}