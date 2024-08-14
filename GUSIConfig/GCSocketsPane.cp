/*
	File:			GCSocketsPane.cp

	Contains:	Pane to manipulate socket families.

	Derived from Apple's Appearance SDK

	$Log: GCSocketsPane.cp,v $
	Revision 1.1  1999/04/10 03:52:48  neeri
	Initial Version
	
*/

#include "GCSocketsPane.h"
#include "AppearanceHelpers.h"
#include "UDialogUtils.h"
#include "GCData.h"

#include <Appearance.h>

enum {
	kEnableINET			= 1,
	kINETPane			= 2,
	kINETAPI				= 4,
	kINETAPI_OT,
	kINETAPI_MT,
	kINETAPI_Both,
	kINETType			= 9,
	kINETType_Tcp,
	kINETType_Udp,
	kINETType_Both,
	kEnablePPC			= 13,
	kEnableLocal		= 14
};
enum {
	kWithOpenTransport	= 1,
	kWithMacTCP				= 2,
	
	kWithTCP					= 1,
	kWithUDP					= 2
};

struct {
	const char *	fName;
	int				fImplMask;
	int				fTypeMask;
} sInetOptions[] =
{
	{ "Inet",		kWithOpenTransport | kWithMacTCP,	kWithTCP | kWithUDP	},
	{ "MTInet",		kWithMacTCP,								kWithTCP | kWithUDP	},
	{ "MTTcp",		kWithMacTCP,								kWithTCP					},
	{ "MTUdp",		kWithMacTCP,								kWithUDP					},
	{ "OTInet",		kWithOpenTransport,						kWithTCP | kWithUDP	},
	{ "OTTcp",		kWithOpenTransport,						kWithTCP					},
	{ "OTUdp",		kWithOpenTransport,						kWithUDP					},
	{ 0, 0, 0 }
};

GCSocketsPane::GCSocketsPane( DialogPtr dialog, SInt16 items ) : GCPane( dialog, items )
{
	AppendDialogItemList( dialog, 1001, overlayDITL );
	
	int impl = 0;
	int type = 0;
	
	for (int i = 0; sInetOptions[i].fName; ++i)
		if (gGCSockets[sInetOptions[i].fName]) {
			impl |= sInetOptions[i].fImplMask;
			type |= sInetOptions[i].fTypeMask;
		}
	
	UDialogUtils::SetDialogItemValue( dialog, fOrigItems + kEnableINET, impl != 0 );
	AdjustINET();
	
	UDialogUtils::SetDialogItemValue( dialog, fOrigItems + kINETAPI,  impl ? impl : 3 );
	UDialogUtils::SetDialogItemValue( dialog, fOrigItems + kINETType, type ? type : 3 );

	UDialogUtils::SetDialogItemValue( dialog, fOrigItems + kEnablePPC,  gGCSockets["PPC"] );
	UDialogUtils::SetDialogItemValue( dialog, fOrigItems + kEnableLocal,gGCSockets["Local"] );
}

GCSocketsPane::~GCSocketsPane()
{
	int impl = UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kINETAPI );
	int type = UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kINETType );
	
	if (!UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnableINET))
		impl = type = 0;

	for (int i = 0; sInetOptions[i].fName; ++i)
		gGCSockets[sInetOptions[i].fName] = 
			(impl == sInetOptions[i].fImplMask && type == sInetOptions[i].fTypeMask);
	
	gGCSockets["PPC"]		= UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnablePPC);
	gGCSockets["Local"]	= UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnableLocal);
	
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void 
GCSocketsPane::AdjustINET()
{
	ControlHandle	control;

	GetDialogItemAsControl( fDialog, fOrigItems + kINETPane, &control );
	if ( UDialogUtils::GetDialogItemValue( fDialog, fOrigItems + kEnableINET ) == 1 )
		ActivateControl( control );
	else
		DeactivateControl( control );
}

void
GCSocketsPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	
	localItem = item - fOrigItems;
	
	switch( localItem ) {
		case kEnableINET:
			UDialogUtils::ToggleCheckBox( fDialog, item );
			AdjustINET();
			break;
		case kEnablePPC:
		case kEnableLocal:
			UDialogUtils::ToggleCheckBox( fDialog, item );
			break;
	}
}
