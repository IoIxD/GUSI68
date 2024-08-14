
#include "GUSIInternal.h"
#include "GUSIConfig.h"
#include "GUSIDiag.h"
#include "GUSIContext.h"
#include "GUSIBasics.h"
#include "GUSIFSWrappers.h"

#include <signal.h>
#include <sys/signal.h>

#include <PLStringFuncs.h>
#include <Resources.h>
#include <Quickdraw.h>
#include <LowMem.h>
#include <Script.h>


#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#endif
struct GUSIConfigRsrc {
	OSType			fDefaultType;
	OSType			fDefaultCreator;
	
	bool			fAutoSpin;
	unsigned char	fFlags;
	
	OSType			fVersion;
	
	short							fNumSuffices;
	GUSIConfiguration::FileSuffix 	fSuffices[1];
};
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#endif


GUSIConfiguration * GUSIConfiguration::sInstance = nil;

//
// Prevent inlining to allow override
//
#ifdef __MWERKS__
#pragma dont_inline on
#endif

void GUSISetupConfig()
{
	GUSIConfiguration::CreateInstance();
}

#ifdef __MWERKS__
#pragma dont_inline reset
#endif

GUSIConfiguration::GUSIConfiguration(short resourceID)
{
	
fDefaultType	=	'TEXT';
fDefaultCreator	=	'MPS ';

fAutoSpin = true;

fAutoInitGraf		= true;

fSigPipe		= false;

fSigInt		= true;

fAccurateStat		= false;
fSharedOpen			= false;

fHandleAppleEvents 	= true;

	
	if (resourceID == kNoResource)
		return;
	
	GUSIConfigRsrc ** 	rsrc = 
		reinterpret_cast<GUSIConfigRsrc **>(Get1Resource('GU I', resourceID));
	long				size = GetHandleSize(Handle(rsrc));
	
	if (!rsrc || !size)
		return;
		
	OSType version = '0102';
	if (size >= 14)
		version = rsrc[0]->fVersion;
		
	
if (version >= '0120' && size >= 16)
	fNumSuffices = rsrc[0]->fNumSuffices;
if (fNumSuffices && (fSuffices = new FileSuffix[fNumSuffices])) {
	fWeOwnSuffices = true;
	memcpy(fSuffices, rsrc[0]->fSuffices, fNumSuffices*sizeof(FileSuffix));
	for (int i=0; i<fNumSuffices; i++)
		for (int j=0; j<4; j++)
			if (fSuffices[i].suffix[j] == ' ')
				fSuffices[i].suffix[j] = 0;
}

}

void GUSIConfiguration::SetDefaultFType(const GUSIFileSpec & name) const
{
	FInfo	info;	

	if (GUSIFSGetFInfo(&name, &info))
		return;
	info.fdType 	=	fDefaultType;
	info.fdCreator	=	fDefaultCreator;

	#warning: unhandled macro "definitions[mat]"
determined:	
	GUSIFSSetFInfo(&name, &info);
}

void GUSIConfiguration::DoAutoSpin() const
{
	GUSIContext::Yield(kGUSIYield);
	GUSIContext::Raise();
}

void GUSIConfiguration::DoAutoInitGraf()
{
	Ptr curA5 = LMGetCurrentA5();
	
	if (!(reinterpret_cast<long>(curA5) & 1) 
	 && curA5 > reinterpret_cast<Ptr>(ApplicationZone()) 
	 && curA5 < LMGetBufPtr()
	)
		if (*reinterpret_cast<GrafPtr **>(curA5) != &qd.thePort)
			InitGraf(&qd.thePort);
	fAutoInitGraf	=	false;	
}

void GUSIConfiguration::BrokenPipe()
{
	if (fSigPipe)
		raise(SIGPIPE);
}

bool GUSIConfiguration::CmdPeriod(const EventRecord * event)
{
	#warning: unhandled macro "definitions[mat]"
	
	if ((event->what == keyDown) || (event->what == autoKey)) {
		// see if the command key is down.  If it is, find out the ASCII
		// equivalent for the accompanying key.

		if (event->modifiers & cmdKey ) {
			
short 	virtualKey 	= (event->message & kMaskVirtualKey) >> kShiftWord;
short 	keyCode    	= (event->modifiers & kMaskModifiers) | virtualKey;
UInt32	state      	= 0;
Handle	hKCHR 		= nil; 
Ptr		KCHRPtr 	= reinterpret_cast<Ptr>(GetScriptManagerVariable(smKCHRCache));
if (!KCHRPtr) {
	short script 	= GetScriptManagerVariable(smKeyScript);
	short kcID 		= GetScriptVariable(script, smScriptKeys);
	hKCHR   		= GetResource('KCHR', kcID);
	KCHRPtr 		= *hKCHR;
}
short	keyInfo;
if (KCHRPtr) {
	keyInfo = KeyTranslate(KCHRPtr, keyCode, &state);
	if (hKCHR)
		ReleaseResource(hKCHR);
} else
	keyInfo = event->message;

			
			if ((keyInfo & kMaskASCII2) == '.' || (keyInfo & kMaskASCII1) == ('.' << 16))
				return true;
		}  // end the command key is down
	}  // end key down event	
	
	return false;
}

void GUSIConfiguration::ConfigureHandleAppleEvents(bool handleAppleEvents)
{
	if (fHandleAppleEvents != handleAppleEvents) {
		fHandleAppleEvents = handleAppleEvents;
		GUSISetHook(GUSI_EventHook+kHighLevelEvent, (GUSIHook) (fHandleAppleEvents ? 0 : -1));
	}
}

