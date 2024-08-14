/*
	File:		GCDevicesPane.h

	Contains:	Pane for manipulating file devices.

	Derived from Apple's Appearance SDK

	$Log: GCDevicesPane.h,v $
	Revision 1.1  1999/04/10 03:52:47  neeri
	Initial Version
	
*/

#pragma once

#include "GCPane.h"

class GCDevicesPane : public GCPane
{
public:
		GCDevicesPane( DialogPtr dialog, SInt16 items );
	virtual ~GCDevicesPane();
	
	virtual void	ItemHit( SInt16 item );
};
