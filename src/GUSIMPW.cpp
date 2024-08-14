
#include "GUSIInternal.h"
#include "GUSIMPW.h"
#include "GUSIDevice.h"
#include "GUSIDescriptor.h"
#include "GUSIMacFile.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"
#include "GUSITimer.h"
#include "GUSIConfig.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <stdio.h>

// #include <CursorCtl.h>

#include <CodeFragments.h>

extern "C"
{
#if !defined(powerc) && !defined(__powerc)
#define DECL_stdlib(name, pname, ret, args) ret name args;
#ifdef __MWERKS__
#pragma pointers_in_D0
#endif

	DECL_stdlib(MPW_open, "\popen", int, (const char *name, int flags))
		DECL_stdlib(MPW_close, "\pclose", int, (int s))
			DECL_stdlib(MPW_read, "\pread", int, (int s, char *buffer, unsigned buflen))
				DECL_stdlib(MPW_write, "\pwrite", int, (int s, char *buffer, unsigned buflen))
					DECL_stdlib(MPW_fcntl, "\pfcntl", int, (int s, unsigned int cmd, int arg))
						DECL_stdlib(MPW_ioctl, "\pioctl", int, (int d, unsigned int request, long *argp))
							DECL_stdlib(MPW_lseek, "\plseek", long, (int fd, long offset, int whence))
								DECL_stdlib(MPW_faccess, "\pfaccess", int, (char *fileName, unsigned int cmd, long *arg))
									DECL_stdlib(MPW_getenv, "\pgetenv", char *, (const char *env))

#ifdef __MWERKS__
#pragma pointers_in_A0
#endif
										static bool ConnectToMPWLibrary()
	{
		return true;
	}
#ifdef __MWERKS__
	extern int _mpwerrno;
	static void UpdateMPWErrno() { errno = _mpwerrno; }
#else
	static void UpdateMPWErrno() {}
#endif
#else
#define DECL_stdlib(name, pname, ret, args) ret(*name) args;

	DECL_stdlib(MPW_open, "\popen", int, (const char *name, int flags))
		DECL_stdlib(MPW_close, "\pclose", int, (int s))
			DECL_stdlib(MPW_read, "\pread", int, (int s, char *buffer, unsigned buflen))
				DECL_stdlib(MPW_write, "\pwrite", int, (int s, char *buffer, unsigned buflen))
					DECL_stdlib(MPW_fcntl, "\pfcntl", int, (int s, unsigned int cmd, int arg))
						DECL_stdlib(MPW_ioctl, "\pioctl", int, (int d, unsigned int request, long *argp))
							DECL_stdlib(MPW_lseek, "\plseek", long, (int fd, long offset, int whence))
								DECL_stdlib(MPW_faccess, "\pfaccess", int, (char *fileName, unsigned int cmd, long *arg))
									DECL_stdlib(MPW_getenv, "\pgetenv", char *, (const char *env))

										int *MPW_errno;
	static void UpdateMPWErrno() { errno = *MPW_errno; }

	static bool sConnected;

	static void DoConnectToMPWLibrary()
	{
		CFragConnectionID StdCLib;
		CFragSymbolClass symClass;
		Ptr whoCares;
		Str255 error;

		if (GetSharedLibrary(
				StringPtr("\pStdCLib"), kPowerPCCFragArch, kLoadCFrag, &StdCLib, &whoCares, error))
			return;

		if (FindSymbol(StdCLib, "\perrno", (Ptr *)&MPW_errno, &symClass))
			goto failed;
#undef DECL_stdlib
#define DECL_stdlib(name, pname, ret, args)                  \
	if (FindSymbol(StdCLib, pname, (Ptr *)&name, &symClass)) \
		goto failed;

		DECL_stdlib(MPW_open, "\popen", int, (const char *name, int flags))
			DECL_stdlib(MPW_close, "\pclose", int, (int s))
				DECL_stdlib(MPW_read, "\pread", int, (int s, char *buffer, unsigned buflen))
					DECL_stdlib(MPW_write, "\pwrite", int, (int s, char *buffer, unsigned buflen))
						DECL_stdlib(MPW_fcntl, "\pfcntl", int, (int s, unsigned int cmd, int arg))
							DECL_stdlib(MPW_ioctl, "\pioctl", int, (int d, unsigned int request, long *argp))
								DECL_stdlib(MPW_lseek, "\plseek", long, (int fd, long offset, int whence))
									DECL_stdlib(MPW_faccess, "\pfaccess", int, (char *fileName, unsigned int cmd, long *arg))
										DECL_stdlib(MPW_getenv, "\pgetenv", char *, (const char *env))

											sConnected = true;

		return;

	failed:
#undef DECL_stdlib
#define DECL_stdlib(name, pname, ret, args) name = 0;

		DECL_stdlib(MPW_open, "\popen", int, (const char *name, int flags))
			DECL_stdlib(MPW_close, "\pclose", int, (int s))
				DECL_stdlib(MPW_read, "\pread", int, (int s, char *buffer, unsigned buflen))
					DECL_stdlib(MPW_write, "\pwrite", int, (int s, char *buffer, unsigned buflen))
						DECL_stdlib(MPW_fcntl, "\pfcntl", int, (int s, unsigned int cmd, int arg))
							DECL_stdlib(MPW_ioctl, "\pioctl", int, (int d, unsigned int request, long *argp))
								DECL_stdlib(MPW_lseek, "\plseek", long, (int fd, long offset, int whence))
									DECL_stdlib(MPW_faccess, "\pfaccess", int, (char *fileName, unsigned int cmd, long *arg))
										DECL_stdlib(MPW_getenv, "\pgetenv", char *, (const char *env))
	}

	static bool ConnectToMPWLibrary()
	{
		if (!sConnected)
			DoConnectToMPWLibrary();
		return sConnected;
	}

#endif
}

class GUSIMPWSocket : public GUSISocket
{
public:
	~GUSIMPWSocket();

	ssize_t read(const GUSIScatterer &buffer);

	ssize_t write(const GUSIGatherer &buffer);

	virtual off_t lseek(off_t offset, int whence);

	virtual int fcntl(int cmd, va_list arg);

	virtual int ioctl(unsigned int request, va_list arg);

	virtual int ftruncate(off_t offset);

	virtual int fstat(struct stat *buf);

	virtual int isatty();

	virtual bool select(bool *canRead, bool *canWrite, bool *exception);

protected:
	friend class GUSIMPWDevice;

	GUSIMPWSocket(int fd);

private:
	int fFD;
};

class GUSIMPWDevice : public GUSIDevice
{
public:
	static GUSIMPWDevice *Instance();

	virtual bool Want(GUSIFileToken &file);

	virtual GUSISocket *open(GUSIFileToken &, int flags);

	GUSISocket *stdopen(int fd, int flags);

private:
	GUSIMPWDevice() {}

	static GUSIMPWDevice *sInstance;
};

GUSIMPWSocket::GUSIMPWSocket(int fd)
	: fFD(fd)
{
}
GUSIMPWSocket::~GUSIMPWSocket()
{
	MPW_close(fFD);
	UpdateMPWErrno();
}

GUSIMPWDevice *GUSIMPWDevice::sInstance;

extern int StandAlone;

#define MPW_O_RDONLY 0			 /* Bits 0 and 1 are used internally */
#define MPW_O_WRONLY 1			 /* Values 0..2 are historical */
#define MPW_O_RDWR 2			 /* NOTE: it goes 0, 1, 2, *!* 8, 16, 32, ... */
#define MPW_O_APPEND (1 << 3)	 /* append (writes guaranteed at the end) */
#define MPW_O_RSRC (1 << 4)		 /* Open the resource fork */
#define MPW_O_ALIAS (1 << 5)	 /* Open alias file */
#define MPW_O_CREAT (1 << 8)	 /* Open with file create */
#define MPW_O_TRUNC (1 << 9)	 /* Open with truncation */
#define MPW_O_EXCL (1 << 10)	 /* w/ O_CREAT:  Exclusive "create-only" */
#define MPW_O_BINARY (1 << 11)	 /* Open as a binary stream */
#define MPW_O_NRESOLVE (1 << 14) /* Don't resolve any aliases */

static int TranslateOpenFlags(int mode)
{
	int mpwMode;

	switch (mode & 3)
	{
	case O_RDWR:
		mpwMode = MPW_O_RDWR;
		break;
	case O_RDONLY:
		mpwMode = MPW_O_RDONLY;
		break;
	case O_WRONLY:
		mpwMode = MPW_O_WRONLY;
		break;
	}
	if (mode & O_APPEND)
		mpwMode |= MPW_O_APPEND;
	if (mode & O_CREAT)
		mpwMode |= MPW_O_CREAT;
	if (mode & O_EXCL)
		mpwMode |= MPW_O_EXCL;
	if (mode & O_TRUNC)
		mpwMode |= MPW_O_TRUNC;

	return mpwMode;
}

GUSISocket *GUSIMPWDevice::stdopen(int fd, int flags)
{
	if (!ConnectToMPWLibrary())
		return GUSISetPosixError(ENOEXEC), static_cast<GUSISocket *>(nil);

	short fRef;

	if (!StandAlone && MPW_ioctl(fd, FIOINTERACTIVE, nil) == -1 && MPW_ioctl(fd, FIOREFNUM, (long *)&fRef) != -1)
	{
		static short sOutFRef = 0;
		static GUSISocket *sOutSocket;

		MPW_close(fd);
		if (fd == 1)
		{
			sOutFRef = fRef;
			return sOutSocket = GUSIMacFileDevice::Instance()->open(fRef, flags);
		}
		else if (fd == 2 && fRef == sOutFRef)
		{
			// Standard output and error redirected to same file
			return sOutSocket;
		}
		else
			return GUSIMacFileDevice::Instance()->open(fRef, flags);
	}

	GUSISocket *sock = new GUSIMPWSocket(fd);

	return sock ? sock : (GUSISetPosixError(ENOMEM), static_cast<GUSISocket *>(nil));
}

char *GUSIGetEnv(const char *name)
{
	if (!ConnectToMPWLibrary())
		return static_cast<char *>(nil);
	return MPW_getenv(name);
}

void GUSIMPWSpin(bool wait)
{
	static GUSITimer sSpinDue(false);

	GUSIConfiguration::Instance()->AutoInitGraf();

	if (sSpinDue.Expired())
	{
		// RotateCursor(32);
		sSpinDue.Sleep(125, true);
	}
	else if (wait)
		GUSIHandleNextEvent(600);
}

void GUSISetupConsoleDescriptors()
{
	GUSIMPWDevice *mpw = GUSIMPWDevice::Instance();
	GUSIDescriptorTable *table = GUSIDescriptorTable::Instance();

	GUSIDeviceRegistry::Instance()->AddDevice(mpw);

	if (!(*table)[0])
	{
		table->InstallSocket(mpw->stdopen(0, O_RDONLY));
		table->InstallSocket(mpw->stdopen(1, O_WRONLY));
		table->InstallSocket(mpw->stdopen(2, O_WRONLY));
	}

	GUSISetHook(GUSI_EventHook + mouseDown, (GUSIHook)-1);
	GUSISetHook(GUSI_SpinHook, (GUSIHook)GUSIMPWSpin);
}
