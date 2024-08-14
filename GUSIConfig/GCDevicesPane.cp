/*
	File:			GCDevicesPane.cp

	Contains:	Pane for manipulating file devices.

	Derived from Apple's Appearance SDK

	$Log: GCDevicesPane.cp,v $
	Revision 1.1  1999/04/10 03:52:47  neeri
	Initial Version
	
*/

#include "GCDevicesPane.h"
#include "GCData.h"

#include "AppearanceHelpers.h"
#include "UDialogUtils.h"

#include <Appearance.h>

enum {
	kEnableNull			= 1,
	kEnableDCon			= 2
};

GCDevicesPane::GCDevicesPane( DialogPtr dialog, SInt16 items ) : GCPane( dialog, items )
{
	AppendDialogItemList( dialog, 1002, overlayDITL );

	UDialogUtils::SetDialogItemValue( fDialog, fOrigItems + kEnableNull, gGCDevices["Null"] );
	UDialogUtils::SetDialogItemValue( fDialog, fOrigItems + kEnableDCon, gGCDevices["DCon"] );
}

GCDevicesPane::~GCDevicesPane()
{
	gGCDevices["Null"] = UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnableNull );
	gGCDevices["DCon"] = UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnableDCon );

	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void
GCDevicesPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	
	localItem = item - fOrigItems;
	
	switch( localItem )
	{
	case kEnableNull:
	case kEnableDCon:
		UDialogUtils::ToggleCheckBox( fDialog, item );
		break;
	}
}
