
#ifndef _GUSIFileSpec_
#define _GUSIFileSpec_

#include <MacTypes.h>
#include <Files.h>
#include <Folders.h>

#include <string.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Construct a FSSpec from...
 */
/* ... the refNum of an open file. 		*/
OSErr GUSIFRefNum2FSp(short fRefNum, FSSpec * desc);
/* ... a working directory & file name. */
OSErr GUSIWD2FSp(short wd, ConstStr31Param name, FSSpec * desc);
/* ... a path name.						*/
OSErr GUSIPath2FSp(const char * path, FSSpec * desc);	
/* ... a special object. 				*/
OSErr GUSISpecial2FSp(OSType object, short vol, FSSpec * desc);	
/* ... a temporary file path.		    */
OSErr GUSIMakeTempFSp(short vol, long dirID, FSSpec * desc);

/*
 * Convert a FSSpec into...
 */
/* ... a full path name.								*/
char * GUSIFSp2FullPath(const FSSpec * desc);
/* ... a relative path name if possible, full if not 	*/
char * GUSIFSp2RelPath(const FSSpec * desc);	
/* ... a path relative to the specified directory 		*/
char * GUSIFSp2DirRelPath(const FSSpec * desc, const FSSpec * dir);
/* ... an GUSI-=specific ASCII encoding. 				*/
char * GUSIFSp2Encoding(const FSSpec * desc);

/*
 * Construct FSSpec of...
 */
/* ... (vRefNum, parID) */
OSErr GUSIFSpUp(FSSpec * desc);
/* ... file (name) in directory denoted by desc */
OSErr GUSIFSpDown(FSSpec * desc, ConstStr31Param name);
/* ... of nth file in directory denoted by (vRefNum, parID) */
OSErr GUSIFSpIndex(FSSpec * desc, short n);

/* Resolve if alias file */
OSErr GUSIFSpResolve(FSSpec * spec);

/* Touch folder containing the object */
OSErr GUSIFSpTouchFolder(const FSSpec * spec);

/* Get catalog information (after resolving leaf aliases) */
OSErr GUSIFSpGetCatInfo(FSSpec * spec, CInfoPBRec * info);

__END_DECLS

#ifdef GUSI_SOURCE

#include "GUSIBasics.h"
#include "GUSIContext.h"

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSICatInfo {
	CInfoPBRec	fInfo;
public:
	bool		IsFile() const;
	bool		IsAlias() const;
	bool 		DirIsExported() const;
	bool	 	DirIsMounted() const;
	bool	 	DirIsShared() const;
	bool	 	HasRdPerm() const;
	bool 		HasWrPerm() const;
	bool 		Locked() const;
	
	CInfoPBRec &				Info()			{	return fInfo; 							}
	operator CInfoPBRec &()						{	return fInfo; 							}
	struct HFileInfo &			FileInfo()		{	return fInfo.hFileInfo;					}
	struct DirInfo &			DirInfo()		{	return fInfo.dirInfo;					}
	struct FInfo &				FInfo()			{	return fInfo.hFileInfo.ioFlFndrInfo;	}
	struct FXInfo &				FXInfo()		{	return fInfo.hFileInfo.ioFlXFndrInfo;	}
	
	const CInfoPBRec &			Info() const	{	return fInfo; 							}
	operator const CInfoPBRec &()	const		{	return fInfo; 							}
	const struct HFileInfo &	FileInfo() const{	return fInfo.hFileInfo;					}
	const struct DirInfo &		DirInfo() const	{	return fInfo.dirInfo;					}
	const struct FInfo &		FInfo() const	{	return fInfo.hFileInfo.ioFlFndrInfo;	}
	const struct FXInfo &		FXInfo() const	{	return fInfo.hFileInfo.ioFlXFndrInfo;	}
};


class GUSIFileSpec {
public:
	
GUSIFileSpec()	{}
GUSIFileSpec(const GUSIFileSpec & spec);

GUSIFileSpec(const FSSpec & spec, bool useAlias = false);

// Construct from volume reference number, directory ID & file name
GUSIFileSpec(short vRefNum, long parID, ConstStr31Param name, bool useAlias = false);

// Construct from working directory & file name
GUSIFileSpec(short wd, ConstStr31Param name, bool useAlias = false);

// Construct from full or relative path
GUSIFileSpec(const char * path, bool useAlias = false);
	
// Construct from open file reference number
explicit GUSIFileSpec(short fRefNum);

GUSIFileSpec(OSType object, short vol = kOnSystemDisk);

	
OSErr		Error() const;
			operator void*() const;
bool		operator!() const;

	
OSErr	SetDefaultDirectory();
OSErr	GetDefaultDirectory();
OSErr	GetVolume(short index = -1);

	
operator const FSSpec &() const;

class pointer {
public:
	pointer(GUSIFileSpec * ptr);
	operator GUSIFileSpec *() const;
	operator const FSSpec *() const;
private:
	GUSIFileSpec * ptr;
};
pointer operator&();

class const_pointer {
public:
	const_pointer(const GUSIFileSpec * ptr);
	operator const GUSIFileSpec *() const;
	operator const FSSpec *() const;
private:
	const GUSIFileSpec * ptr;
};
const_pointer operator&() const;

const FSSpec * operator->() const;

friend class pointer;
friend class const_pointer;

	
const GUSICatInfo * CatInfo(short index);
const GUSICatInfo * DirInfo();
const GUSICatInfo * CatInfo() const;
bool				Exists() const;
void				ResetCatInfo() const;

	
char *	FullPath() const;
char *	RelativePath() const;
char *	RelativePath(const FSSpec & dir) const;

char *	EncodedPath() const;

	
OSErr	Resolve(bool gently = true);

char *	AliasPath() const;

	
GUSIFileSpec &	operator--();

GUSIFileSpec &	operator++();

GUSIFileSpec &	AddPathComponent(const char * name, int length, bool fullSpec);
GUSIFileSpec &	operator+=(ConstStr31Param name);
GUSIFileSpec &	operator+=(const char * name);

//
// These don't need access to the GUSIFileSpec internals
//
// friend GUSIFileSpec operator+(const FSSpec & spec, ConstStr31Param name);
// friend GUSIFileSpec operator+(const FSSpec & spec, const char * name);

GUSIFileSpec & operator[](short index);

void SetVRef(short vref);
void SetParID(long parid);
void SetName(ConstStr63Param nam);
void SetName(const char * nam);

OSErr TouchFolder();

	
friend bool operator==(const GUSIFileSpec & one, const GUSIFileSpec & other);

bool	IsParentOf(const GUSIFileSpec & other) const;

protected:
	
mutable OSErr	fError;

static char *		sScratch;
static long			sScratchSize;

static char *		CScratch(bool extend = false);
static StringPtr	PScratch();

FSSpec							fSpec;
GUSIIOPBWrapper<GUSICatInfo>	fInfo;
mutable bool					fValidInfo;

enum { ROOT_MAGIC_COOKIE = 666 };

OSErr PrependPathComponent(char *&path, ConstStr63Param component, bool colon) const;

};

class GUSITempFileSpec : public GUSIFileSpec {
public:
	GUSITempFileSpec(short vRefNum = kOnSystemDisk);
	GUSITempFileSpec(short vRefNum, long parID);
	GUSITempFileSpec(ConstStr31Param basename);
	GUSITempFileSpec(short vRefNum, ConstStr31Param basename);
	GUSITempFileSpec(short vRefNum, long parID, ConstStr31Param basename);
private:
	void TempName();
	void TempName(ConstStr31Param basename);
	
	static int	sID;
};


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


inline bool GUSICatInfo::IsFile() const
{
	return !(DirInfo().ioFlAttrib & 0x10);
}

inline bool GUSICatInfo::IsAlias() const
{
	return
		!(FileInfo().ioFlAttrib & 0x10) &&
		(FInfo().fdFlags & (1 << 15));
}

inline bool GUSICatInfo::DirIsExported() const
{
	return (FileInfo().ioFlAttrib & 0x20) != 0;
}

inline bool GUSICatInfo::DirIsMounted() const
{
	return (FileInfo().ioFlAttrib & 0x08) != 0;
}

inline bool GUSICatInfo::DirIsShared() const
{
	return (FileInfo().ioFlAttrib & 0x04) != 0;
}

inline bool GUSICatInfo::HasRdPerm() const
{
	return !(DirInfo().ioACUser & 0x02) != 0;
}

inline bool GUSICatInfo::HasWrPerm() const
{
	return !(DirInfo().ioACUser & 0x04) != 0;
}

inline bool GUSICatInfo::Locked() const
{
	return (FileInfo().ioFlAttrib & 0x11) == 0x01;
}


inline StringPtr GUSIFileSpec::PScratch()
{
	return (StringPtr) CScratch();
}

inline OSErr GUSIFileSpec::SetDefaultDirectory()
{
	return fError = HSetVol(nil, fSpec.vRefNum, fSpec.parID);
}

inline OSErr GUSIFileSpec::GetDefaultDirectory()
{
	fSpec.name[0] 	= 0;
	fValidInfo		= false;
	return fError 	= HGetVol(nil, &fSpec.vRefNum, &fSpec.parID);
}

inline GUSIFileSpec &	GUSIFileSpec::operator+=(ConstStr31Param name)
{
	return AddPathComponent((char *) name+1, name[0], true);
}

inline GUSIFileSpec &	GUSIFileSpec::operator+=(const char * name)
{
	return AddPathComponent(name, strlen(name), true);
}

inline const GUSICatInfo * GUSIFileSpec::CatInfo() const
{
	return const_cast<GUSIFileSpec *>(this)->CatInfo(0);
}

inline const GUSICatInfo * GUSIFileSpec::DirInfo()
{
	if (CatInfo(-1)) {
		fSpec.parID = fInfo->DirInfo().ioDrParID;
		fValidInfo	= true;
		
		return &fInfo.fPB;
	} else
		return nil;
}

inline bool GUSIFileSpec::Exists() const
{
	return CatInfo() != nil;
}

inline void GUSIFileSpec::ResetCatInfo() const
{
	GUSI_MUTABLE(GUSIFileSpec, fValidInfo) = false;
}

inline GUSIFileSpec::operator const FSSpec &() const
{
	return fSpec;
}
inline const FSSpec * GUSIFileSpec::operator->() const
{
	return &fSpec;
}

inline GUSIFileSpec::const_pointer::const_pointer(const GUSIFileSpec * ptr)
	: ptr(ptr)
{
}
inline GUSIFileSpec::const_pointer::operator const GUSIFileSpec *() const
{
	return ptr;
}
inline GUSIFileSpec::const_pointer::operator const FSSpec *() const
{
	return &ptr->fSpec;
}
inline GUSIFileSpec::const_pointer GUSIFileSpec::operator&() const
{
	return const_pointer(this);
}

inline GUSIFileSpec::pointer::pointer(GUSIFileSpec * ptr)
	: ptr(ptr)
{
}
inline GUSIFileSpec::pointer::operator GUSIFileSpec *() const
{
	return ptr;
}
inline GUSIFileSpec::pointer::operator const FSSpec *() const
{
	return &ptr->fSpec;
}
inline GUSIFileSpec::pointer GUSIFileSpec::operator&()
{
	return pointer(this);
}


#endif /* GUSI_SOURCE */

#endif /* GUSIFileSpec */
