
#include "GUSIInternal.h"
#include "GUSINull.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"

#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>

#include <Devices.h>
#include "GUSIPOSIX.h"

class GUSINullSocket : public GUSISocket
{
public:
	GUSINullSocket();

	virtual ssize_t read(const GUSIScatterer &buffer);

	virtual ssize_t write(const GUSIGatherer &buffer);

	virtual int fstat(struct stat *buf);

	virtual bool Supports(ConfigOption config);
};

extern "C" void GUSIwithNullSockets()
{
	GUSIDeviceRegistry::Instance()->AddDevice(GUSINullDevice::Instance());
}

GUSINullDevice *GUSINullDevice::sInstance = nil;

bool GUSINullDevice::Want(GUSIFileToken &file)
{
	if (!file.IsDevice())
		return false;

	const char *path = file.Path();

	switch (file.WhichRequest())
	{
	case GUSIFileToken::kWillOpen:
	case GUSIFileToken::kWillStat:
		return file.StrFragEqual(path + 4, "null") && !path[8];
	default:
		return false;
	}
}

GUSISocket *GUSINullDevice::open(GUSIFileToken &, int)
{
	GUSISocket *sock = new GUSINullSocket;
	if (!sock)
		GUSISetPosixError(ENOMEM);
	return sock;
}

GUSISocket *GUSINullDevice::open()
{
	GUSISocket *sock = new GUSINullSocket;
	if (!sock)
		GUSISetPosixError(ENOMEM);
	return sock;
}

static int GUSINullStat(struct stat *buf)
{
	buf->st_dev = 'dev ';
	buf->st_ino = 'null';
	buf->st_mode = S_IFCHR | 0666;
	buf->st_nlink = 1;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_rdev = 0;
	buf->st_size = 1;
	buf->st_atime = time(NULL);
	buf->st_mtime = time(NULL);
	buf->st_ctime = time(NULL);
	buf->st_blksize = 0;
	buf->st_blocks = 1;

	return 0;
}

int GUSINullDevice::stat(GUSIFileToken &, struct stat *buf)
{
	return GUSINullStat(buf);
}

GUSINullSocket::GUSINullSocket()
{
}
