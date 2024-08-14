
#ifndef _GUSIMacFile_
#define _GUSIMacFile_

#ifdef GUSI_INTERNAL

#include "GUSIDevice.h"


class GUSIMacFileDevice : public GUSIDevice {
public:
	static GUSIMacFileDevice *	Instance();
	virtual bool	Want(GUSIFileToken & file);
	
	
virtual GUSISocket * open(GUSIFileToken & file, int flags);

virtual int remove(GUSIFileToken & file);

virtual int rename(GUSIFileToken & file, const char * newname);

virtual int stat(GUSIFileToken & file, struct stat * buf);

virtual int chmod(GUSIFileToken & file, mode_t mode);

virtual int utime(GUSIFileToken & file, const utimbuf * times);

virtual int access(GUSIFileToken & file, int mode);

virtual int mkdir(GUSIFileToken & file);

virtual int rmdir(GUSIFileToken & file);

virtual GUSIDirectory * opendir(GUSIFileToken & file);

virtual int symlink(GUSIFileToken & to, const char * newlink);

virtual int readlink(GUSIFileToken & link, char * buf, int bufsize);

virtual int fgetfileinfo(GUSIFileToken & file, OSType * creator, OSType * type);
virtual int fsetfileinfo(GUSIFileToken & file, OSType creator, OSType type);

virtual int faccess(GUSIFileToken & file, unsigned * cmd, void * arg);

	
	GUSISocket * 	open(short fileRef, int flags);
	
	int 			MarkTemporary(const FSSpec & file);
	void			CleanupTemporaries(bool giveup);
	
	~GUSIMacFileDevice();
protected:
	GUSIMacFileDevice()	: fTemporaries(0)		{}

	
struct TempQueue {
	TempQueue *	fNext;
	FSSpec		fSpec;
} * fTemporaries;

};


#endif /* GUSI_INTERNAL */

#endif /* _GUSIMacFile_ */
