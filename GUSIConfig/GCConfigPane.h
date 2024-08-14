/*
	File:		GCSocketsPane.h

	Contains:	Pane for manipulating configuration resource settings.

	Derived from Apple's Appearance SDK

	$Log: GCConfigPane.h,v $
	Revision 1.1  1999/04/10 03:52:46  neeri
	Initial Version
	
*/

#pragma once

#include "GCPane.h"

class GCConfigPane : public GCPane
{
public:
		GCConfigPane( DialogPtr dialog, SInt16 items );
	virtual ~GCConfigPane();
	
	virtual void	ItemHit( SInt16 item );
	
	void AdjustConfig();
};
