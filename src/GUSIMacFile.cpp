
#define GUSI_MESSAGE_LEVEL 5

#include "GUSIInternal.h"
#include "GUSIMacFile.h"
#include "GUSIFSWrappers.h"
#include "GUSISocketMixins.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"
#include "GUSIConfig.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#include <algorithm>
#include <memory>

#include <Aliases.h>
#include <Devices.h>
#include <TextUtils.h>
#include <Resources.h>

GUSI_USING_STD_NAMESPACE

using std::auto_ptr;
using std::min;

class GUSIMacFileSocket;

static void GUSIMFRead(GUSIMacFileSocket *sock);
static pascal void GUSIMFReadDone(ParamBlockRec *pb);
static void GUSIMFWrite(GUSIMacFileSocket *sock);
static pascal void GUSIMFWriteDone(ParamBlockRec *pb);
static pascal void GUSIMFWakeup(ParamBlockRec *pb);

class GUSIMacFileSocket : public GUSISocket,
						  public GUSISMProcess,
						  protected GUSISMAsyncError,
						  protected GUSISMBlocking,
						  protected GUSISMState,
						  protected GUSISMInputBuffer,
						  protected GUSISMOutputBuffer
{
public:
	GUSIMacFileSocket(short fileRef, bool append, int mode);
	~GUSIMacFileSocket();

	virtual ssize_t read(const GUSIScatterer &buffer);

	virtual ssize_t write(const GUSIGatherer &buffer);

	virtual bool select(bool *canRead, bool *canWrite, bool *exception);

	virtual int fsync();

	virtual int fcntl(int cmd, va_list arg);

	virtual int ioctl(unsigned int request, va_list arg);

	virtual int getsockopt(int level, int optname, void *optval, socklen_t *optlen);

	virtual int setsockopt(int level, int optname, void *optval, socklen_t optlen);

	virtual off_t lseek(off_t offset, int whence);

	virtual int ftruncate(off_t offset);

	virtual int fstat(struct stat *buf);

	virtual bool Supports(ConfigOption config);

private:
	short fFileRef;
	bool fAppend;
	bool fWriteBehind;
	bool fReadAhead;
	bool fBeyondEOF;
	long fPosition;
	void SyncRead();
	void SyncWrite();
	size_t fBlockSize;
	ParamBlockRec fReadPB;
	ParamBlockRec fWritePB;
	friend void GUSIMFRead(GUSIMacFileSocket *sock);
	friend pascal void GUSIMFReadDone(ParamBlockRec *pb);
	friend void GUSIMFWrite(GUSIMacFileSocket *sock);
	friend pascal void GUSIMFWriteDone(ParamBlockRec *pb);
	friend pascal void GUSIMFWakeup(ParamBlockRec *pb);
	static IOCompletionUPP sReadProc;
	static IOCompletionUPP sWriteProc;
	static IOCompletionUPP sWakeupProc;
};

class GUSIMacDirectory : public GUSIDirectory
{
public:
	GUSIMacDirectory(const FSSpec &spec);

	virtual dirent *readdir();
	virtual long telldir();
	virtual void seekdir(long pos);
	virtual void rewinddir();

private:
	GUSIFileSpec fDir;
	short fPos;
	dirent fEnt;
};

static pascal void GUSIMFWriteDone(ParamBlockRec *pb)
{
	GUSIMacFileSocket *sock =
		reinterpret_cast<GUSIMacFileSocket *>((char *)pb - offsetof(GUSIMacFileSocket, fWritePB));

	GUSI_CASSERT_INTERNAL(sock->fFileRef == pb->ioParam.ioRefNum);
	GUSIProcess::A5Saver saveA5(sock->Process());
	GUSI_SMESSAGE("Write done.\n");
	if (sock->fOutputBuffer.Locked())
		sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMFWriteDone), pb);
	else
	{
		sock->fOutputBuffer.ClearDefer();
		sock->fOutputBuffer.FreeBuffer(
			sock->fWritePB.ioParam.ioBuffer, sock->fWritePB.ioParam.ioReqCount);
		if (sock->SetAsyncMacError(sock->fWritePB.ioParam.ioResult))
		{
			for (long valid; valid = sock->fOutputBuffer.Valid();)
				sock->fOutputBuffer.FreeBuffer(nil, valid);
			sock->fWriteShutdown = true;
		}
		GUSIMFWrite(sock);
		sock->Wakeup();
	}
}

static void GUSIMFWrite(GUSIMacFileSocket *sock)
{
	size_t valid = sock->fOutputBuffer.Valid();
	if (sock->fWriteShutdown && !valid)
		sock->fOutputBuffer.ClearDefer();
	else if (!valid)
		sock->fOutputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMFWrite), sock);
	else
	{
		sock->fOutputBuffer.ClearDefer();
		valid = min(valid, sock->fOutputBuffer.Size() >> 1);
		sock->fWritePB.ioParam.ioBuffer =
			static_cast<Ptr>(sock->fOutputBuffer.ConsumeBuffer(valid));
		long pos = sock->fPosition - sock->fOutputBuffer.Valid();
		size_t align = pos % sock->fBlockSize;
		if (align)
			valid = min(valid, sock->fBlockSize - align);
		else if (valid > sock->fBlockSize)
			valid -= valid % sock->fBlockSize;
		sock->fWritePB.ioParam.ioPosMode = fsAtMark +
												   (sock->fWriteBehind && valid >= sock->fBlockSize)
											   ? noCacheMask
											   : 0;
		sock->fWritePB.ioParam.ioReqCount = valid;
		sock->fWritePB.ioParam.ioActCount = 0;

		GUSI_MESSAGE(("Async write (0x%x bytes).\n", valid));
		PBWriteAsync(&sock->fWritePB);
	}
}

static pascal void GUSIMFReadDone(ParamBlockRec *pb)
{
	GUSIMacFileSocket *sock =
		reinterpret_cast<GUSIMacFileSocket *>((char *)pb - offsetof(GUSIMacFileSocket, fReadPB));
	GUSI_CASSERT_INTERNAL(sock->fFileRef == pb->ioParam.ioRefNum);
	GUSIProcess::A5Saver saveA5(sock->Process());
	GUSI_MESSAGE(("Read done (0x%x/%d).\n", pb->ioParam.ioActCount, pb->ioParam.ioResult));
	if (sock->fInputBuffer.Locked())
		sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMFReadDone), pb);
	else
	{
		sock->fInputBuffer.ClearDefer();
		sock->fInputBuffer.ValidBuffer(
			sock->fReadPB.ioParam.ioBuffer, sock->fReadPB.ioParam.ioActCount);
		if (sock->SetAsyncMacError(sock->fReadPB.ioParam.ioResult))
			sock->fReadShutdown = true;
		else if (!sock->fReadShutdown)
			GUSIMFRead(sock);
		sock->Wakeup();
	}
}

static void GUSIMFRead(GUSIMacFileSocket *sock)
{
	size_t free = sock->fInputBuffer.Free();
	if (free < (sock->fInputBuffer.Size() >> 2))
	{
		if (!sock->fReadShutdown)
			sock->fInputBuffer.Defer(GUSIRingBuffer::Deferred(GUSIMFRead), sock);
		else
			sock->fInputBuffer.ClearDefer();
	}
	else
	{
		sock->fInputBuffer.ClearDefer();
		free = min(free, sock->fInputBuffer.Size() >> 1);
		long pos = sock->fPosition + sock->fInputBuffer.Valid();
		size_t align = pos % sock->fBlockSize;
		if (align)
			free = min(free, sock->fBlockSize - align);
		else if (free > sock->fBlockSize)
			free -= free % sock->fBlockSize;
		sock->fReadPB.ioParam.ioBuffer =
			static_cast<Ptr>(sock->fInputBuffer.ProduceBuffer(free));
		sock->fReadPB.ioParam.ioReqCount = free;
		sock->fReadPB.ioParam.ioActCount = 0;

		GUSI_MESSAGE(("Async read 0x%x 0x%x (0x%x bytes).\n", sock, &sock->fReadPB, free));
		PBReadAsync(&sock->fReadPB);
	}
}

static pascal void GUSIMFWakeup(ParamBlockRec *pb)
{
	GUSIMacFileSocket *sock =
		reinterpret_cast<GUSIMacFileSocket *>((char *)pb - offsetof(GUSIMacFileSocket, fReadPB));
	GUSI_CASSERT_INTERNAL(sock->fFileRef == pb->ioParam.ioRefNum);
	sock->Wakeup();
}

GUSI_COMPLETION_PROC_A0(GUSIMFReadDone, ParamBlockRec)
GUSI_COMPLETION_PROC_A0(GUSIMFWriteDone, ParamBlockRec)
GUSI_COMPLETION_PROC_A0(GUSIMFWakeup, ParamBlockRec)

static auto_ptr<GUSIMacFileDevice> sGUSIMacFileDevice;

GUSIMacFileDevice::~GUSIMacFileDevice()
{
	CleanupTemporaries(true);
}

bool GUSIMacFileDevice::Want(GUSIFileToken &file)
{
	if (!file.IsFile())
		return false;
	if (!file.Error() && file.Exists())
		return true;
	if (file.Error() == fnfErr)
		switch (file.WhichRequest())
		{
		case GUSIFileToken::kWillOpen:
		case GUSIFileToken::kWillMkdir:
		case GUSIFileToken::kWillSymlink: // A borderline case
			return true;
		}

	return false;
}

GUSISocket *GUSIMacFileDevice::open(short fileRef, int flags)
{
	GUSISocket *sock;

	if (!(sock =
			  new GUSIMacFileSocket(fileRef, (flags & O_APPEND) != 0, flags & 3)))
	{
		FSClose(fileRef);
		GUSISetPosixError(ENOMEM);
	}

	return sock;
}

void GUSIMacFileDevice::CleanupTemporaries(bool giveup)
{
	TempQueue **p = &fTemporaries;

	for (TempQueue *q = *p; q; q = *p)
		if (GUSIFSDelete(&q->fSpec) != fBsyErr || giveup)
		{
			// Delete entry
			*p = q->fNext;
			delete q;
		}
		else
		{
			// Keep entry
			p = &q->fNext;
		}
}

static OSErr VRef2Icon(short vRef, Handle *icon)
{
	OSErr err;
	short cRef;

	GUSIIOPBWrapper<HParamBlockRec> hpb;
	hpb->volumeParam.ioVRefNum = vRef;
	hpb->volumeParam.ioNamePtr = nil;
	hpb->volumeParam.ioVolIndex = 0;
	if (err = GUSIFSHGetVInfo(&hpb))
		return err;
	vRef = hpb->volumeParam.ioVDrvInfo;
	cRef = hpb->volumeParam.ioVDRefNum;
	GUSIIOPBWrapper<ParamBlockRec> pb;
	pb->cntrlParam.ioVRefNum = vRef;
	pb->cntrlParam.ioCRefNum = cRef;
	pb->cntrlParam.csCode = 21;

	if (err = pb.Control())
		return err;

	PtrToHand(*(Ptr *)pb->cntrlParam.csParam, icon, 256);

	return errAborted;
}

typedef OSType AliasTypeMap[2];

static AliasTypeMap sMap[] = {
	{'amnu', 'faam'},
	{'ctrl', 'fact'},
	{'extn', 'faex'},
	{'pref', 'fapf'},
	{'prnt', 'fapn'},
	{'empt', 'trsh'},
	{'trsh', 'trsh'},
	{'strt', 'fast'},
	{'macs', 'fasy'},
	{0, 0}};

static void AliasTypeExpert(
	const GUSIFileSpec &file,
	OSType *fCreator,
	OSType *fType,
	GUSIFileSpec *iconFile,
	short *iconID)
{
	const GUSICatInfo *info = file.CatInfo();

	*fCreator = 'MACS';
	*iconFile = file;
	*iconID = kCustomIconResource;

	if (!info->IsFile())
	{
		GetVolParmsInfoBuffer volParms;

		if (!GUSIFSGetVolParms(file->vRefNum, &volParms) && volParms.vMServerAdr)
			if (file->parID == fsRtParID)
				*fType = 'srvr';
			else if (!info->HasRdPerm())
				*fType = 'fadr';
			else
				*fType = 'faet';
		else if (file->parID == fsRtParID)
			*fType = 'hdsk';
		else if (info->DirIsMounted())
			*fType = 'famn';
		else if (info->DirIsExported())
			*fType = 'fash';
		else if (info->DirIsShared())
			*fType = 'faet';
		else
			*fType = 'fdrp';
		if (file->parID != fsRtParID && info->DirInfo().ioDrDirID < 9)
		{
			short vRef;
			long dirID;

			for (AliasTypeMap *mapp = sMap; **mapp; ++mapp)
				if (FindFolder(file->vRefNum, (*mapp)[0], false, &vRef, &dirID))
					continue;
				else if (dirID == info->DirInfo().ioDrDirID)
				{
					*fType = (*mapp)[1];

					break;
				}
		}
		if (file->parID == fsRtParID)
		{
			iconFile->SetParID(fsRtDirID);
			iconFile->SetName("\pIcon\n");
		}
		else
		{
			*iconFile += "\pIcon\n";
		}
	}
	else
	{
		*fType = info->FInfo().fdType;
		*fCreator = info->FInfo().fdCreator;
	}

	return;
error:
	*fType = 0;
	*fCreator = 0;
}

static OSType sIconTypes[] = {
	'ICN#',
	'ics#',
	'icl4',
	'ics4',
	'icl8',
	'ics8',
	0};

static bool CopyIconFamily(
	short srcResFile, short srcID, short dstResFile, short dstID)
{
	Handle icon;
	bool success = false;

	for (OSType *types = sIconTypes; *types; ++types)
	{
		UseResFile(srcResFile);
		if (icon = Get1Resource(*types, srcID))
		{
			UseResFile(dstResFile);
			DetachResource(icon);
			AddResource(icon, *types, dstID, "\p");

			success = success || !ResError();
		}
	}

	return success;
}

static bool AddIconsToFile(
	const GUSIFileSpec &origFile, short aliasFile,
	const GUSIFileSpec &iconFile, short iconID,
	bool plainDisk)
{
	bool hasCustomIcons = false;
	short iFile = FSpOpenResFile(&iconFile, fsRdPerm);

	if (iFile != -1)
	{
		hasCustomIcons =
			CopyIconFamily(iFile, iconID, aliasFile, kCustomIconResource);
		CloseResFile(iFile);
	}
	if (!hasCustomIcons && plainDisk)
	{
		Handle icon;
		if (!VRef2Icon(origFile->vRefNum, &icon))
		{
			AddResource(icon, 'ICN#', kCustomIconResource, "\p");

			hasCustomIcons = !ResError();
		}
	}

	return hasCustomIcons;
}

int GUSIMacFileDevice::symlink(GUSIFileToken &to, const char *newlink)
{
	if (!to.Exists())
		return GUSISetPosixError(EIO);
	if (to.IsDevice())
		return GUSISetPosixError(EINVAL);

	OSType fileType;
	OSType fileCreator;
	short iconID;
	short aliasFile;
	AliasHandle alias;
	bool customIcon;
	GUSIFileSpec iconFile;
	FInfo info;

	GUSIFileSpec newnm(newlink, true);

	if (newnm.Error())
		return GUSISetPosixError(EIO);
	if (newnm.Exists())
		return GUSISetPosixError(EEXIST);

	AliasTypeExpert(to, &fileCreator, &fileType, &iconFile, &iconID);
	FSpCreateResFile(&newnm, fileCreator, fileType, smSystemScript);

	if (ResError())
		return GUSISetPosixError(EIO);

	aliasFile = FSpOpenResFile(&newnm, fsRdWrPerm);
	if (aliasFile == -1)
		goto deleteFile;

	if (NewAlias(nil, &to, &alias))
		goto closeFile;

	AddResource((Handle)alias, 'alis', 0, to->name);
	if (ResError())
		goto deleteAlias;

	customIcon = AddIconsToFile(to, aliasFile, iconFile, iconID,
								fileType == 'hdsk' && fileCreator == 'MACS');

	CloseResFile(aliasFile);

	GUSIFSGetFInfo(&newnm, &info);
	info.fdFlags |= kIsAlias | (customIcon ? kHasCustomIcon : 0);
	info.fdFlags &= ~kHasBeenInited;
	GUSIFSSetFInfo(&newnm, &info);
	newnm.TouchFolder();

	return 0;

deleteAlias:
	DisposeHandle((Handle)alias);
closeFile:
	CloseResFile(aliasFile);
deleteFile:
	GUSIFSDelete(&newnm);

	return GUSISetPosixError(EIO);
}

int GUSIMacFileDevice::fsetfileinfo(GUSIFileToken &file, OSType creator, OSType type)
{
	FInfo info;

	if (file.IsDevice())
		return GUSISetPosixError(EINVAL);
	if (file.Error() || !file.Exists() || GUSIFSGetFInfo(&file, &info))
		return GUSISetPosixError(EIO);

	info.fdType = type;
	info.fdCreator = creator;
	info.fdFlags &= ~kHasBeenInited;

	if (GUSIFSSetFInfo(&file, &info))
		return GUSISetPosixError(EIO);
	return 0;
}

GUSIMacFileSocket::~GUSIMacFileSocket()
{
	fsync();
	if (fFileRef)
		FSClose(fFileRef);
	GUSIMacFileDevice::Instance()->CleanupTemporaries(false);
}

IOCompletionUPP GUSIMacFileSocket::sReadProc = 0;
IOCompletionUPP GUSIMacFileSocket::sWriteProc = 0;
IOCompletionUPP GUSIMacFileSocket::sWakeupProc = 0;

GUSIMacDirectory::GUSIMacDirectory(const FSSpec &spec)
	: fDir(spec)
{
	fPos = 1;
}
