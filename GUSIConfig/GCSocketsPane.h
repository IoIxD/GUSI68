/*
	File:		GCSocketsPane.h

	Contains:	Pane for manipulating socket families.

	Derived from Apple's Appearance SDK

	$Log: GCSocketsPane.h,v $
	Revision 1.1  1999/04/10 03:52:48  neeri
	Initial Version
	
*/

#pragma once

#include "GCPane.h"

class GCSocketsPane : public GCPane
{
public:
		GCSocketsPane( DialogPtr dialog, SInt16 items );
	virtual ~GCSocketsPane();
	
	virtual void	ItemHit( SInt16 item );
	
	void AdjustINET();
};
