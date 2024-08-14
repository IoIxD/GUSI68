/*
	File:		GCPane.h

	Contains:	Base class for dialog panes.

	Derived from Apple's Appearance SDK

	$Log: GCPane.h,v $
	Revision 1.1  1999/04/10 03:52:48  neeri
	Initial Version
	
*/

#pragma once

#include <Dialogs.h>

class GCPane
{
	public:
			GCPane( DialogPtr dialog, SInt16 origItems );
		virtual ~GCPane();
		
		virtual void	ItemHit( SInt16 item );
		virtual void	Idle();
	
	protected:
		DialogPtr		fDialog;
		SInt16			fOrigItems;
};
