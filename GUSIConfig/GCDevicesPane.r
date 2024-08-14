/*
	File:			GUSIDevicesPane.r

	Contains:	Resources for device family pane.

	Derived from Apple's Appearance SDK.

	$Log: GCDevicesPane.r,v $
	Revision 1.1  1999/04/10 03:52:47  neeri
	Initial Version
	
*/

#include "Dialogs.r"
#include "Controls.r"

resource 'DITL' (1002) {
	{	
		{ 50,  15,  68, 200}, CheckBox	  { enabled, "Enable Null Device"},
		{ 74,  15,  92, 200}, CheckBox	  { enabled, "Enable DCon Device"},
	}
};
