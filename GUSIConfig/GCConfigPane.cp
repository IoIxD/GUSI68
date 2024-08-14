/*
	File:			GCConfigPane.cp

	Contains:	Pane for manipulating configuration resource settings.

	Derived from Apple's Appearance SDK

	$Log: GCConfigPane.cp,v $
	Revision 1.2  1999/06/24 06:39:33  neeri
	Added SIGINT flag
	
	Revision 1.1  1999/04/10 03:52:45  neeri
	Initial Version
	
*/

#include "AppearanceHelpers.h"
#include "UDialogUtils.h"

#include "GCConfigPane.h"
#include "GCData.h"

#include <stdlib.h>

#include <Appearance.h>

enum {
	kHasConfig		= 1,
	kConfigPane,
	kDefaultType	= 5,
	kDefaultCreator= 7,
	kAutoSpin		= 8,
	kAccurateStat,
	kSharedOpen,
	kAutoInitGraf,
	kHandleAppleEvents,
	kSigPipe,
	kSigInt
};

GCConfigPane::GCConfigPane( DialogPtr dialog, SInt16 items ) : GCPane( dialog, items )
{
	AppendDialogItemList( dialog, 1003, overlayDITL );

	unsigned char str[5];
	str[0] = 4;
	
	UDialogUtils::SetDialogItemValue(fDialog, fOrigItems+kHasConfig, gGCHasConfig);
	AdjustConfig();
	
	memcpy(str+1, &gGCType, 4);
	UDialogUtils::SetDialogItemText(fDialog, fOrigItems+kDefaultType, str);
	memcpy(str+1, &gGCCreator, 4);
	UDialogUtils::SetDialogItemText(fDialog, fOrigItems+kDefaultCreator, str);
	
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kAutoSpin, gGCFlags["AutoSpin"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kAccurateStat, gGCFlags["AccurateStat"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kSharedOpen, gGCFlags["SharedOpen"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kAutoInitGraf, gGCFlags["AutoInitGraf"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kHandleAppleEvents, gGCFlags["HandleAppleEvents"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kSigPipe, gGCFlags["SigPipe"].first);
	UDialogUtils::SetDialogItemValue(fDialog, 
		fOrigItems+kSigInt, gGCFlags["SigInt"].first);
}

GCConfigPane::~GCConfigPane()
{
	Str255	str;

	gGCHasConfig = UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kHasConfig);

	UDialogUtils::GetDialogItemText(fDialog, fOrigItems+kDefaultType, str);
	while (str[0] < 4)
		str[++str[0]] = ' ';
	memcpy(&gGCType, str+1, 4);
	UDialogUtils::GetDialogItemText(fDialog, fOrigItems+kDefaultCreator, str);
	while (str[0] < 4)
		str[++str[0]] = ' ';
	memcpy(&gGCCreator, str+1, 4);

	gGCFlags["AutoSpin"].first 			= 
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kAutoSpin );
	gGCFlags["AccurateStat"].first 		= 
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kAccurateStat );
	gGCFlags["SharedOpen"].first			=
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kSharedOpen );
	gGCFlags["AutoInitGraf"].first		=
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kAutoInitGraf );
	gGCFlags["HandleAppleEvents"].first	= 
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kHandleAppleEvents );
	gGCFlags["SigPipe"].first				=
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kSigPipe );
	gGCFlags["SigInt"].first				=
		UDialogUtils::GetDialogItemValue(fDialog, fOrigItems+kSigInt );
	
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void 
GCConfigPane::AdjustConfig()
{
	ControlHandle	control;

	GetDialogItemAsControl( fDialog, fOrigItems + kConfigPane, &control );
	if ( UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kHasConfig ) == 1 )
		ActivateControl( control );
	else
		DeactivateControl( control );
}

void
GCConfigPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	
	localItem = item - fOrigItems;
	
	switch( localItem ) {
	case kHasConfig:
		UDialogUtils::ToggleCheckBox( fDialog, item );
		AdjustConfig();
		break;		
	default:
		UDialogUtils::ToggleCheckBox( fDialog, item );
		break;
	}
}
