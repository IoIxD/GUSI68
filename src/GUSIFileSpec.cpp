
#include "GUSIInternal.h"
#include "GUSIFileSpec.h"
#include "GUSIFSWrappers.h"

#include <PLStringFuncs.h>
#include <Errors.h>
#include <TextUtils.h>
#include <Resources.h>
#include <Memory.h>
#include <Aliases.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


bool ReadHex(const char * path, int bytes, char * result) 
{
	char hexbyte[3];
	hexbyte[2] = 0;
	while (bytes--) {
		hexbyte[0] = *path++; hexbyte[1] = *path++;
		if (isxdigit(hexbyte[0]) && isxdigit(hexbyte[1]))
			*result++ = (char) strtol(hexbyte, nil, 16);
		else
			return false;
	}
	return true;
}


GUSIFileSpec::GUSIFileSpec(const FSSpec & spec, bool useAlias)							
	: fValidInfo(false), fSpec(spec), fError(noErr)
{
	if (!useAlias)
		Resolve();
}

GUSIFileSpec::GUSIFileSpec(short vRefNum, long parID, ConstStr31Param name, bool useAlias)
	: fValidInfo(false)
{
	OSErr	err;
	
	if ((err = FSMakeFSSpec(vRefNum, parID, name, &fSpec)) && (err != fnfErr)) {
		fSpec.vRefNum	=	vRefNum;
		fSpec.parID		=	parID;
		memcpy(fSpec.name, name, *name+1);
	}
	fError	= noErr;
	
	if (!useAlias)
		Resolve();	
		
	if (EqualString(fSpec.name, name, false, true))
		memcpy(fSpec.name, name, *name+1);		
}

GUSIFileSpec::GUSIFileSpec(short wd, ConstStr31Param name, bool useAlias)
	: fValidInfo(false)
{
	if ((fError = FSMakeFSSpec(wd, 0, name, &fSpec)) && (fError != fnfErr))
		return;
	
	if (!useAlias)
		Resolve();
		
	if (EqualString(fSpec.name, name, false, true))
		memcpy(fSpec.name, name, *name+1);		
}

GUSIFileSpec::GUSIFileSpec(OSType object, short vol)
	: fValidInfo(false)
{
	fError = FindFolder(vol, object, true, &fSpec.vRefNum, &fSpec.parID);
}

GUSIFileSpec::GUSIFileSpec(short fRefNum)
	: fValidInfo(false)
{
	GUSIIOPBWrapper<FCBPBRec>	fcb;

	fcb->ioNamePtr	= 	fSpec.name;
	fcb->ioRefNum	=	fRefNum;
	fcb->ioFCBIndx	= 	0;

	if (fError = GUSIFSGetFCBInfo(&fcb))
		return;

	fSpec.vRefNum 	=	fcb->ioFCBVRefNum;
	fSpec.parID		=	fcb->ioFCBParID;
}

GUSIFileSpec::GUSIFileSpec(const char * path, bool useAlias)
	: fValidInfo(false)
{
	if (!*path) {
		fError = dirNFErr;
		return;
	}
	const char *	nextPath;
	bool			fullSpec	= 	false;
	
	fSpec.vRefNum 	= 0;
	fSpec.parID		= 0;
	
	<<Try decoding the path as an encoded [[FSSpec]]>>
	<<Try converting the path with [[FSMakeFSSpec]] and [[return]]>>
	<<Determine the starting directory of the path>>

	fError = noErr;
	while (*path && !fError) {
		if (*path == ':')	{
			<<Walk directories upwards>>
		} else {
			if (nextPath = strchr(path, ':')) {
				AddPathComponent(path, nextPath-path, fullSpec);
				fullSpec 	= true;
				path 		= nextPath+1;
			} else {
				AddPathComponent(path, strlen(path), fullSpec);
				fullSpec 	= true;
				break;
			}
		}
	}
	if (!fError) {
		if (!fullSpec)
			--(*this);
		if (!useAlias)
			Resolve();
	} else if (*path && fError == fnfErr) {
		fError = dirNFErr;
	}
}

OSErr GUSIFileSpec::GetVolume(short index)
{
	fValidInfo	=	false;
	if (fSpec.name[0] || index>=0) {
		GUSIIOPBWrapper<ParamBlockRec>	vol;
		
		vol->volumeParam.ioVRefNum	=	fSpec.vRefNum;
		vol->volumeParam.ioNamePtr	=	fSpec.name;
		vol->volumeParam.ioVolIndex	=	index;
		
		if (index < 0 && fSpec.name[fSpec.name[0]] != ':')
			fSpec.name[++fSpec.name[0]] = ':';
	
		if (fError = GUSIFSGetVInfo(&vol))
			return fError;
		
		fSpec.vRefNum	=	vol->volumeParam.ioVRefNum;
	} else {
		fError 			= 	noErr;
		fSpec.vRefNum	= 	0;
	}
	fSpec.parID			=	fsRtParID;
	
	return fError;
}

GUSIFileSpec & GUSIFileSpec::operator--()
{
	if (fSpec.parID == fsRtParID) {
		fSpec.vRefNum 	= ROOT_MAGIC_COOKIE;
		fSpec.parID   	= 0;
		fSpec.name[0] 	= 0;
		fError			= noErr;
		fValidInfo		= false;
	} else 
		DirInfo();
	
	return *this;
}

GUSIFileSpec & GUSIFileSpec::operator++()
{
	if (!fSpec.parID && fSpec.vRefNum == ROOT_MAGIC_COOKIE) {
		fSpec.vRefNum	=	0;
		fSpec.parID		=	fsRtParID;
		fSpec.name[0]	=	0;
		fValidInfo		= 	false;
		
		goto punt;
	} 
			
   	if (!CatInfo())
		goto punt;
	if (fInfo->IsAlias())
		if (Resolve() || !CatInfo())
			goto punt;
	if (fInfo->IsFile()) {
		fError = afpObjectTypeErr;
		
		goto punt;
	}
	
	fSpec.parID		= fInfo->DirInfo().ioDrDirID;
	fSpec.name[0] 	= 0;
	fValidInfo		= false;

punt:	
	return *this;
}

GUSIFileSpec & GUSIFileSpec::AddPathComponent(const char * name, int length, bool fullSpec)
{
	if (length > 63) {
		fError = bdNamErr;
		
		goto punt;
	}

	if (fullSpec) 
		if (!++(*this))
			goto punt;
	
	memcpy(fSpec.name+1, name, fSpec.name[0] = length);
	fValidInfo = false;
		
	if (fSpec.parID == fsRtParID)
		GetVolume();

punt:	
	return *this;
}

GUSIFileSpec & GUSIFileSpec::operator[](short index)
{
	if (fSpec.parID == fsRtParID) 
		GetVolume(index);
	else 
		CatInfo(index);
	
	return *this;
}

void GUSIFileSpec::SetVRef(short vref)
{
	fSpec.vRefNum 	= vref;
	fValidInfo		= false;
}

void GUSIFileSpec::SetParID(long parid)
{
	fSpec.parID	 	= parid;
	fValidInfo		= false;
}

void GUSIFileSpec::SetName(ConstStr63Param name)
{
	PLstrcpy(fSpec.name, name);
	fValidInfo	= false;
}

void GUSIFileSpec::SetName(const char * name)
{
	memcpy(fSpec.name+1, name, fSpec.name[0] = strlen(name));
	fValidInfo	= false;
}

OSErr GUSIFileSpec::TouchFolder()
{
	GUSIFileSpec	folder(*this, true);
	
	if (!folder.DirInfo())
		return fError = folder.Error();
	
	GetDateTime(&folder.fInfo->DirInfo().ioDrMdDat);
	folder.fInfo->DirInfo().ioDrDirID = folder.fInfo->DirInfo().ioDrParID;
	
	return fError = GUSIFSSetCatInfo(&folder.fInfo);
}

const GUSICatInfo * GUSIFileSpec::CatInfo(short index)
{
	if (fValidInfo && !index)	// Valid already
		return &fInfo.fPB;
		
	fInfo->DirInfo().ioVRefNum 		= fSpec.vRefNum;
	fInfo->DirInfo().ioDrDirID 		= fSpec.parID;
	fInfo->DirInfo().ioNamePtr 		= (StringPtr) fSpec.name;
	fInfo->DirInfo().ioFDirIndex 	= index;
	fInfo->DirInfo().ioACUser 		= 0;
		
	fValidInfo = !(fError = GUSIFSGetCatInfo(&fInfo)) && index>=0;
		
	return fError ? nil : &fInfo.fPB;
}

char * GUSIFileSpec::FullPath() const
{
	char * 					path = CScratch();
	GUSIFileSpec 			current(*this);
	const GUSICatInfo * 	info 		= current.CatInfo();
	bool					directory	= info && !info->IsFile();

	if (!path) 
		return nil;
	*(path += sScratchSize-1) = 0;
	
	for (;;) {
		if (PrependPathComponent(path, current.fSpec.name, directory))
			return nil;
		directory = current.fSpec.name[0] != 0;
		if (current.fSpec.parID == fsRtParID)
			return path;
		if (!--current)
			return nil;
	}
}

char *	GUSIFileSpec::RelativePath(const FSSpec & dir) const
{
	GUSIFileSpec current(dir);
	if (current.fSpec.name[0])
		++current;
	long	relDirID= current.fSpec.parID;
	if (current.GetVolume(0))
		return FullPath();
	short	relVRef = current.fSpec.vRefNum;
	current = *this;
	if (current.GetVolume(0) || current.fSpec.vRefNum != relVRef)
		return FullPath();

	current = *this;
		
	char * 					path 		= CScratch();
	const GUSICatInfo * 	info 		= current.CatInfo();
	bool					directory	= info && !info->IsFile();

	if (!path) 
		return nil;
	if (directory && info->DirInfo().ioDrDirID == relDirID)
		return strcpy(path, ":");
	*(path += sScratchSize-1) = 0;
	
	for (;;) {
		if (PrependPathComponent(path, current.fSpec.name, directory))
			return nil;
		if (current.fSpec.parID == relDirID) {
			if (directory)
				*--path = ':';
			return path;
		}
		directory = current.fSpec.name[0] != 0;
		if (current.fSpec.parID == fsRtParID)
			return path;
			if (!--current)
			return nil;
	}
}

char *	GUSIFileSpec::RelativePath() const
{
	GUSIFileSpec	here;
	here.GetDefaultDirectory();
	return RelativePath(here);
}

char * GUSIFileSpec::EncodedPath() const
{
	if (!CScratch())
		return nil;

	GUSI_sprintf(sScratch, "\021%04hX%08X:%#s", fSpec.vRefNum, fSpec.parID, fSpec.name);
	
	return sScratch;
}

OSErr GUSIFileSpec::Resolve(bool gently)
{
	const GUSICatInfo * info = CatInfo();
	
	if (!info || (!info->IsAlias() && (fError = resFNotFound)))
		if (gently)
			return fError = noErr;
		else
			return fError;

	Boolean		isFolder;
	Boolean		wasAlias;
	
	fValidInfo	=	false;
	
	return fError = ResolveAliasFile(&fSpec, true, &isFolder, &wasAlias);
}

char * GUSIFileSpec::AliasPath() const
{
	const GUSICatInfo * info = CatInfo();
	if (!info || (!info->IsAlias() && (GUSI_MUTABLE(GUSIFileSpec, fError) = resFNotFound)))
		return nil;

	char * 			path 		= CScratch();
	if (!path) 
		return nil;
	*(path += sScratchSize-1) = 0;

	AliasHandle	alias;
	<<Get the [[AliasHandle]] for the alias file and detach it>>
	bool directory;
	<<Find out if alias points at a file or a directory>>
	Str63 component;
	
	/*
		Build path from target up to root.  Separate with colons.
	*/
	for (short index = 0; 
		!(GUSI_MUTABLE(GUSIFileSpec, fError) = GetAliasInfo(alias, index, component));
		++index
	)
		if (!*component || PrependPathComponent(path, component, directory)) 
			break;
		else
			directory = true;
	
	if (!fError 
	 && !(GUSI_MUTABLE(GUSIFileSpec, fError) = GetAliasInfo(alias, asiVolumeName, component))
	)
		PrependPathComponent(path, component, true);

	DisposeHandle((Handle) alias);
	
	return fError ? nil : path;
}

bool operator==(const GUSIFileSpec & one, const GUSIFileSpec & other)
{
	if (one.fSpec.parID != other.fSpec.parID || !EqualString(one.fSpec.name, other.fSpec.name, false, true))
		return false;
	if (one.fSpec.vRefNum == other.fSpec.vRefNum)
		return true;
	GUSIFileSpec current;
	current = one;
	current.GetVolume(0);
	short vRef1 = current.fSpec.vRefNum;
	current = other;
	current.GetVolume(0);
	short vRef2 = current.fSpec.vRefNum;
	
	return vRef1 == vRef2;
}

bool	GUSIFileSpec::IsParentOf(const GUSIFileSpec & other) const
{
	for (GUSIFileSpec current(other); !(--current).Error();)
		if (current == *this)
			return true;
	
	return false;
}

GUSITempFileSpec::GUSITempFileSpec(short vRefNum)
 : GUSIFileSpec(kTemporaryFolderType, vRefNum)
{
	TempName();
}
GUSITempFileSpec::GUSITempFileSpec(short vRefNum, long parID)
{
	SetVRef(vRefNum);
	SetParID(parID);
	
	TempName();
}

GUSITempFileSpec::GUSITempFileSpec(ConstStr63Param basename)
 : GUSIFileSpec(kTemporaryFolderType, kOnSystemDisk)
{
	TempName(basename);
}
GUSITempFileSpec::GUSITempFileSpec(short vRefNum, ConstStr31Param basename)
 : GUSIFileSpec(kTemporaryFolderType, vRefNum)
{
	TempName(basename);
}
GUSITempFileSpec::GUSITempFileSpec(short vRefNum, long parID, ConstStr31Param basename)
{
	SetVRef(vRefNum);
	SetParID(parID);
	
	TempName(basename);
}

int GUSITempFileSpec::sID = 0;

void GUSITempFileSpec::TempName()
{
	for (;;) {
		char	name[8];

		GUSI_sprintf(name, "tmp%04d", sID++);		
		SetName(name);
		
		sID 		%= 10000;
		fValidInfo   = false;
		if (!Exists()) {
			if (fError == fnfErr)
				fError = noErr;
			return;
		}
	}
}

void GUSITempFileSpec::TempName(ConstStr31Param basename)
{
	for (int id = 0;;++id) {
		Str32	name;
		int 	len = 2;

		if (id < 10)
			;
		else if (id < 100)
			len = 3;
		else if (id < 1000)
			len = 4;
		else if (id < 10000)
			len = 5;
		else {
			fError = fnfErr;
			return;
		}
		
		<<Insert [[id]] at the appropriate place into [[basename]]>>
		SetName(name);
		
		fValidInfo   = false;
		if (!Exists()) {
			if (fError == fnfErr)
				fError = noErr;
			return;
		}
	}
}

<<C API functions for [[GUSIFileSpec]]>>
