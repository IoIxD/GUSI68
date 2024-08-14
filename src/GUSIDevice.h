
#ifndef _GUSIDevice_
#define _GUSIDevice_

#ifdef GUSI_SOURCE

#include "GUSISocket.h"
#include "GUSIFileSpec.h"

#include <dirent.h>
#include <utime.h>

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif

class GUSIDevice;

class GUSIFileToken : public GUSIFileSpec
{
public:
	enum Request
	{

		kWillOpen,

		kWillRemove,

		kWillRename,

		kWillStat,

		kWillChmod,

		kWillUtime,

		kWillAccess,

		kWillMkdir,

		kWillRmdir,

		kWillOpendir,

		kWillSymlink,

		kWillReadlink,

		kWillGetfileinfo,
		kWillSetfileinfo,

		kWillFaccess,

		kNoRequest
	};

	GUSIFileToken(const char *path, Request request, bool useAlias = false);
	GUSIFileToken(const GUSIFileSpec &spec, Request request);
	GUSIFileToken(short fRefNum, Request request);

	bool IsFile() const { return fIsFile; }
	bool IsDevice() const { return !fIsFile; }
	Request WhichRequest() const { return fRequest; }
	GUSIDevice *Device() const { return fDevice; }
	const char *Path() const { return fPath; }

	static bool StrFragEqual(const char *name, const char *frag);
	enum StdStream
	{
		kStdin,
		kStdout,
		kStderr,
		kConsole,
		kNoStdStream = -2
	};
	static StdStream StrStdStream(const char *name);

private:
	GUSIDevice *fDevice;
	const char *fPath;
	bool fIsFile;
	Request fRequest;
};

class GUSIDirectory
{
public:
	virtual ~GUSIDirectory() {}
	virtual dirent *readdir() = 0;
	virtual long telldir() = 0;
	virtual void seekdir(long pos) = 0;
	virtual void rewinddir() = 0;

protected:
	friend class GUSIDevice;
	GUSIDirectory() {}
};

class GUSIDeviceRegistry
{
public:
	static GUSIDeviceRegistry *Instance();

	GUSISocket *open(const char *path, int flags);

	int remove(const char *path);

	int rename(const char *oldname, const char *newname);

	int stat(const char *path, struct stat *buf, bool useAlias);

	int chmod(const char *path, mode_t mode);

	int utime(const char *path, const utimbuf *times);

	int access(const char *path, int mode);

	int mkdir(const char *path);

	int rmdir(const char *path);

	GUSIDirectory *opendir(const char *path);

	int symlink(const char *target, const char *newlink);

	int readlink(const char *path, char *buf, int bufsize);

	int fgetfileinfo(const char *path, OSType *creator, OSType *type);
	int fsetfileinfo(const char *path, OSType creator, OSType type);

	int faccess(const char *path, unsigned *cmd, void *arg);

	void AddDevice(GUSIDevice *device);
	void RemoveDevice(GUSIDevice *device);

	class iterator;

	iterator begin();
	iterator end();

protected:
	friend class GUSIFileToken;

	GUSIDevice *Lookup(GUSIFileToken &file);

private:
	static GUSIDeviceRegistry *sInstance;

	GUSIDevice *fFirstDevice;
	GUSIDeviceRegistry();
};

class GUSIDevice
{
public:
	virtual bool Want(GUSIFileToken &file);

	virtual GUSISocket *open(GUSIFileToken &file, int flags);
	virtual int remove(GUSIFileToken &file);
	virtual int rename(GUSIFileToken &from, const char *newname);
	virtual int stat(GUSIFileToken &file, struct stat *buf);
	virtual int chmod(GUSIFileToken &file, mode_t mode);
	virtual int utime(GUSIFileToken &file, const utimbuf *times);
	virtual int access(GUSIFileToken &file, int mode);
	virtual int mkdir(GUSIFileToken &file);
	virtual int rmdir(GUSIFileToken &file);
	virtual GUSIDirectory *opendir(GUSIFileToken &file);
	virtual int symlink(GUSIFileToken &to, const char *newlink);
	virtual int readlink(GUSIFileToken &link, char *buf, int bufsize);
	virtual int fgetfileinfo(GUSIFileToken &file, OSType *creator, OSType *type);
	virtual int fsetfileinfo(GUSIFileToken &file, OSType creator, OSType type);
	virtual int faccess(GUSIFileToken &file, unsigned *cmd, void *arg);
	int faccess(const char *path, unsigned *cmd, void *arg);

protected:
	friend class GUSIDeviceRegistry;
	friend class GUSIDeviceRegistry::iterator;

	GUSIDevice() : fNextDevice(nil) {}
	virtual ~GUSIDevice() {}

	GUSIDevice *fNextDevice;
};

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

extern "C" void GUSISetupDevices();

inline GUSIDeviceRegistry *GUSIDeviceRegistry::Instance()
{
	if (!sInstance)
	{
		sInstance = new GUSIDeviceRegistry();
		GUSISetupDevices();
	}

	return sInstance;
}

class GUSIDeviceRegistry::iterator
{
public:
	iterator(GUSIDevice *device = 0) : fDevice(device) {}
	GUSIDeviceRegistry::iterator &operator++()
	{
		fDevice = fDevice->fNextDevice;
		return *this;
	}
	GUSIDeviceRegistry::iterator operator++(int)
	{
		GUSIDeviceRegistry::iterator old(*this);
		fDevice = fDevice->fNextDevice;
		return old;
	}
	bool operator==(const GUSIDeviceRegistry::iterator other) const
	{
		return fDevice == other.fDevice;
	}
	GUSIDevice &operator*() { return *fDevice; }
	GUSIDevice *operator->() { return fDevice; }

private:
	GUSIDevice *fDevice;
};

inline GUSIDeviceRegistry::iterator GUSIDeviceRegistry::begin()
{
	return GUSIDeviceRegistry::iterator(fFirstDevice);
}

inline GUSIDeviceRegistry::iterator GUSIDeviceRegistry::end()
{
	return GUSIDeviceRegistry::iterator();
}

#endif /* GUSI_SOURCE */

#endif /* _GUSIDevice_ */
