/*
	File:		BaseDialog.cp

	Contains:	Base class for a dialog.

	Derived from Apple's Appearance SDK

	$Log: BaseDialog.cp,v $
	Revision 1.1  1999/04/10 03:52:45  neeri
	Initial Version
	
*/

#include <Dialogs.h>
#include "BaseDialog.h"

BaseDialog::BaseDialog()
{
}

BaseDialog::BaseDialog( SInt16 resID )
{
	fWindow = GetNewDialog( resID, NULL, (WindowPtr)-1L );
	if ( fWindow )
	{
		((WindowPeek)fWindow)->windowKind = 2000;
		SetWRefCon( fWindow, (long)this );
	}
}

BaseDialog::~BaseDialog()
{
	if ( fWindow )
	{
		DisposeDialog( fWindow );
		fWindow = nil;
	}
}

void
BaseDialog::Update( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;

	((WindowPeek)fWindow)->windowKind = dialogKind;
	DialogSelect( &event, &dialog, &itemHit );
	((WindowPeek)fWindow)->windowKind = 2000;
}

void
BaseDialog::Activate( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	((WindowPeek)fWindow)->windowKind = dialogKind;
	DialogSelect( &event, &dialog, &itemHit );
	((WindowPeek)fWindow)->windowKind = 2000;
}

void
BaseDialog::Deactivate( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	((WindowPeek)fWindow)->windowKind = dialogKind;
	DialogSelect( &event, &dialog, &itemHit );
	((WindowPeek)fWindow)->windowKind = 2000;
}

void
BaseDialog::HandleClick( EventRecord& event )
{
	DialogRef		dialog;
	SInt16			itemHit;
	
	if ( DialogSelect( &event, &dialog, &itemHit ) )
	{
		HandleItemHit( itemHit );
	}
}

void
BaseDialog::HandleKeyDown( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	((WindowPeek)fWindow)->windowKind = dialogKind;
	DialogSelect( &event, &dialog, &itemHit );
	((WindowPeek)fWindow)->windowKind = 2000;
}


void
BaseDialog::HandleItemHit( short /*itemHit*/ )
{
}
