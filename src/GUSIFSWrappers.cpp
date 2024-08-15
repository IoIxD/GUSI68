
#include "GUSIInternal.h"
#include "GUSIFSWrappers.h"
#include "GUSIContext.h"

#include <PLStringFuncs.h>
#include <Devices.h>
#include <Script.h>
#include <StringCompare.h>
#include <Traps.h>
#include <FSM.h>
#include <Gestalt.h>

OSErr GUSIFSOpen(GUSIIOPBWrapper<ParamBlockRec> *pb)
{
	pb->StartIO();
	PBOpenSync(&pb->fPB);
	return pb->FinishIO();
}
OSErr GUSIFSOpenDriver(StringPtr name, short *refNum)
{
	GUSIIOPBWrapper<ParamBlockRec> pb;

	pb->ioParam.ioNamePtr = name;
	pb->ioParam.ioPermssn = fsCurPerm;

	OSErr err = GUSIFSOpen(&pb);
	*refNum = pb->ioParam.ioRefNum;

	return err;
}

OSErr GUSIFSHGetFInfo(GUSIIOPBWrapper<HParamBlockRec> *pb)
{
	pb->StartIO();
	PBHGetFInfoAsync(&pb->fPB);
	return pb->FinishIO();
}
OSErr GUSIFSHSetFInfo(GUSIIOPBWrapper<HParamBlockRec> *pb)
{
	pb->StartIO();
	PBHSetFInfoAsync(&pb->fPB);
	return pb->FinishIO();
}
OSErr GUSIFSGetFInfo(const FSSpec *spec, FInfo *info)
{
	GUSIIOPBWrapper<HParamBlockRec> pb;

	pb->fileParam.ioVRefNum = spec->vRefNum;
	pb->fileParam.ioDirID = spec->parID;
	pb->fileParam.ioNamePtr = const_cast<StringPtr>(spec->name);
	pb->fileParam.ioFDirIndex = 0;

	OSErr err = GUSIFSHGetFInfo(&pb);
	if (!err)
		*info = pb->fileParam.ioFlFndrInfo;

	return err;
}
OSErr GUSIFSSetFInfo(const FSSpec *spec, const FInfo *info)
{
	GUSIIOPBWrapper<HParamBlockRec> pb;

	pb->fileParam.ioVRefNum = spec->vRefNum;
	pb->fileParam.ioDirID = spec->parID;
	pb->fileParam.ioNamePtr = const_cast<StringPtr>(spec->name);
	pb->fileParam.ioFDirIndex = 0;

	OSErr err = GUSIFSHGetFInfo(&pb);
	if (!err)
	{
		pb->fileParam.ioDirID = spec->parID; /* Gets overwritten by PBHGetInfo */
		pb->fileParam.ioFlFndrInfo = *info;
		err = GUSIFSHSetFInfo(&pb);
	}

	return err;
}

OSErr GUSIFSHGetVolParms(GUSIIOPBWrapper<HParamBlockRec> *pb)
{
	pb->StartIO();
	PBHGetVolParmsAsync(&pb->fPB);
	return pb->FinishIO();
}
OSErr GUSIFSGetVolParms(short vRefNum, GetVolParmsInfoBuffer *volParms)
{
	GUSIIOPBWrapper<HParamBlockRec> pb;

	pb->ioParam.ioNamePtr = nil;
	pb->ioParam.ioVRefNum = vRefNum;
	pb->ioParam.ioBuffer = Ptr(&volParms);
	pb->ioParam.ioReqCount = sizeof(GetVolParmsInfoBuffer);

	return GUSIFSHGetVolParms(&pb);
}

OSErr GUSIFSCreate(const FSSpec *spec)
{
	GUSIIOPBWrapper<HParamBlockRec> pb;

	pb->fileParam.ioVRefNum = spec->vRefNum;
	pb->fileParam.ioDirID = spec->parID;
	pb->fileParam.ioNamePtr = const_cast<StringPtr>(spec->name);

	pb.StartIO();
	PBHCreateAsync(&pb.fPB);
	return pb.FinishIO();
}

OSErr GUSIFSCreate(const FSSpec *spec, OSType creator, OSType type, ScriptCode script)
{
	OSErr err;

	if (err = GUSIFSCreate(spec))
		return err;

	GUSIIOPBWrapper<GUSICatInfo> info;
	info->FileInfo().ioVRefNum = spec->vRefNum;
	info->FileInfo().ioDirID = spec->parID;
	info->FileInfo().ioNamePtr = const_cast<StringPtr>(spec->name);

	if (err = GUSIFSGetCatInfo(&info))
		goto nuke;

	info->FInfo().fdCreator = creator;
	info->FInfo().fdType = type;

	if (script == smSystemScript)
		info->FXInfo().fdScript = 0;
	else
		info->FXInfo().fdScript = script | 0x80;

	if (err = GUSIFSSetCatInfo(&info))
		goto nuke;

	return noErr;
nuke:
	GUSIFSDelete(spec);

	return err;
}

static OSErr GUSIFSCatMove1(GUSIIOPBWrapper<CMovePBRec> *pb)
{
	pb->StartIO();
	PBCatMoveAsync(&pb->fPB);
	return pb->FinishIO();
}
OSErr GUSIFSCatMove(const FSSpec *spec, long dest)
{
	GUSIIOPBWrapper<CMovePBRec> pb;

	pb->ioVRefNum = spec->vRefNum;
	pb->ioDirID = spec->parID;
	pb->ioNamePtr = const_cast<StringPtr>(spec->name);
	pb->ioNewName = nil;
	pb->ioNewDirID = dest;

	return GUSIFSCatMove1(&pb);
}
OSErr GUSIFSCatMove(const FSSpec *spec, const FSSpec *dest)
{
	GUSIIOPBWrapper<CMovePBRec> pb;

	pb->ioVRefNum = spec->vRefNum;
	pb->ioDirID = spec->parID;
	pb->ioNamePtr = const_cast<StringPtr>(spec->name);
	pb->ioNewName = const_cast<StringPtr>(dest->name);
	pb->ioNewDirID = dest->parID;

	return GUSIFSCatMove1(&pb);
}

OSErr GUSIFSGetCatInfo(GUSIIOPBWrapper<GUSICatInfo> *info)
{
	info->StartIO();
	PBGetCatInfoAsync(&info->fPB.Info());
	return info->FinishIO();
}

OSErr GUSIFSGetVInfo(GUSIIOPBWrapper<ParamBlockRec> *pb)
{
	pb->StartIO();
	PBGetVInfoAsync(&pb->fPB);
	return pb->FinishIO();
}

#if TARGET_API_MAC_CARBON || !TARGET_RT_MAC_CFM
// Carbon builds and 68K builds don't need this glue
#define CallPBXGetVolInfoAsync PBXGetVolInfoAsync
#else  //	TARGET_API_MAC_CARBON || !TARGET_RT_MAC_CFM
/* This is exactly like the simple mixed mode glue in InterfaceLib in Mac OS 8.5 and 8.6 */
static pascal OSErr PBXGetVolInfoAsyncGlue(XVolumeParamPtr paramBlock)
{
	enum
	{
		uppFSDispatchProcInfo = kRegisterBased | REGISTER_RESULT_LOCATION(kRegisterD0) | RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(long))) /* selector */
								| REGISTER_ROUTINE_PARAMETER(2, kRegisterD1, SIZE_CODE(sizeof(long)))																								 /* trap word */
								| REGISTER_ROUTINE_PARAMETER(3, kRegisterA0, SIZE_CODE(sizeof(XVolumeParamPtr)))
	};

	static UniversalProcPtr fsDispatchTrapAddress = NULL;

	/* Is this the first time we've been called? */
	if (fsDispatchTrapAddress == NULL)
	{
		/* Yes - Get the trap address of _FSDispatch */
		fsDispatchTrapAddress = NGetTrapAddress(_FSDispatch, OSTrap);
	}
	return (CallOSTrapUniversalProc(fsDispatchTrapAddress,
									uppFSDispatchProcInfo,
									kFSMXGetVolInfo,
									_FSDispatch | kAsyncMask,
									paramBlock));
}

/*
** PBXGetVolInfoSync was added to the File Manager in System software 7.5.2.
** However, PBXGetVolInfoSync wasn't added to InterfaceLib until Mac OS 8.5.
** This wrapper calls PBXGetVolInfoSync if it is found in InterfaceLib;
** otherwise, it calls PBXGetVolInfoSyncGlue. This ensures that your program
** is calling the latest implementation of PBXGetVolInfoSync.
*/
static pascal OSErr CallPBXGetVolInfoAsync(XVolumeParamPtr paramBlock)
{
	typedef pascal OSErr (*PBXGetVolInfoProcPtr)(XVolumeParamPtr paramBlock);

	OSErr result;
	CFragConnectionID connID;
	static PBXGetVolInfoProcPtr PBXGetVolInfoAsyncPtr = NULL;

	//* Is this the first time we've been called? */
	if (PBXGetVolInfoAsyncPtr == NULL)
	{
		/* Yes - Get our connection ID to InterfaceLib */
		result = GetSharedLibrary("\pInterfaceLib", kPowerPCCFragArch, kLoadCFrag, &connID, NULL, NULL);
		if (result == noErr)
		{
			/* See if PBXGetVolInfoSync is in InterfaceLib */
			if (FindSymbol(connID, "\pPBXGetVolInfoAsync", &(Ptr)PBXGetVolInfoAsyncPtr, NULL) != noErr)
			{
				/* Use glue code if symbol isn't found */
				PBXGetVolInfoAsyncPtr = PBXGetVolInfoAsyncGlue;
			}
		}
	}
	/* Call PBXGetVolInfoAsync if present; otherwise, call PBXGetVolInfoAsyncGlue */
	return ((*PBXGetVolInfoAsyncPtr)(paramBlock));
}
#endif //	TARGET_API_MAC_CARBON || !TARGET_RT_MAC_CFM
