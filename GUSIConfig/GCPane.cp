/*
	File:		GCPane.cp

	Contains:	Base class for panes in our GCDialog.

	Derived from Apple's Appearance SDK

	$Log: GCPane.cp,v $
	Revision 1.1  1999/04/10 03:52:48  neeri
	Initial Version
	
*/

#include "GCPane.h"

GCPane::GCPane( DialogPtr dialog, SInt16 items )
{
	fDialog = dialog;
	fOrigItems = items;
}

GCPane::~GCPane()
{
}

void
GCPane::ItemHit( SInt16 )
{
}

void
GCPane::Idle()
{
}
