/*
	File:			GUSISocketsPane.r

	Contains:	Resources for socket family pane.

	Derived from Apple's Appearance SDK.

	$Log: GCSocketsPane.r,v $
	Revision 1.1  1999/04/10 03:52:49  neeri
	Initial Version
	
*/

#include "Dialogs.r"
#include "Controls.r"

resource 'DITL' (1001) {
	{	{ 40,  15, 255, 229}, Control 	  { enabled, 10010},
		{ 64,  21, 250, 220}, Control 	  { enabled, 10011},
		{ 67,  24, 152, 200}, Control 	  {disabled, 10012},
		{ 82,  24, 147, 200}, Control 	  { enabled, 10013},
		{ 85,  26, 103, 195}, RadioButton { enabled, "Open Transport"},
		{104,  26, 122, 195}, RadioButton { enabled, "MacTCP"},
		{123,  26, 141, 195}, RadioButton { enabled, "Both"},
		{160,  24, 245, 200}, Control 	  {disabled, 10014},
		{175,  24, 240, 200}, Control 	  { enabled, 10015},
		{178,  26, 196, 195}, RadioButton { enabled, "Stream (TCP)"},
		{197,  26, 215, 195}, RadioButton { enabled, "Datagram (UDP)"},
		{216,  26, 234, 195}, RadioButton { enabled, "Both"},
		{ 50, 250,  68, 450}, CheckBox	  { enabled, "Enable PPC Sockets"},
		{ 74, 250,  92, 450}, CheckBox	  { enabled, "Enable Local Socket Pairs"},
	}
};

resource 'dftb' (1001, purgeable)
{
	versionZero
	{
		{
			skipItem {},
			skipItem {},
			dataItem { kDialogFontUseFontMask, kControlFontSmallBoldSystemFont, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" },
			skipItem {},
			skipItem {},
			skipItem {},
			skipItem {},
			dataItem { kDialogFontUseFontMask, kControlFontSmallBoldSystemFont, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "" }
		}
	}
};

/* TCP/IP: Enable Checkbox Group */

resource 'CNTL' (10010, "") {
	{  0,   0, 215, 214},
	0,
	visible,
	1,
	0,
	kControlGroupBoxCheckBoxProc,
	0,
	"Enable Internet Sockets"
};

/* TCP/IP: Contents Group */

resource 'CNTL' (10011, "") {
	{  0,   0, 186, 199},
	2,
	visible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};

resource 'CNTL' (10012, "") {
	{  0,   0,  75, 176},
	0,
	visible,
	0,
	0,
	kControlGroupBoxSecondaryTextTitleProc,
	0,
	"Implementation"
};

/* OT/MacTCP Radio Buttons */
resource 'CNTL' (10013, "") {
	{  0,   0,  65, 176},
	0,
	visible,
	0,
	0,
	kControlRadioGroupProc,
	0,
	""
};

resource 'CNTL' (10014, "") {
	{  0,   0,  75, 176},
	0,
	visible,
	0,
	0,
	kControlGroupBoxSecondaryTextTitleProc,
	0,
	"Socket Types"
};

/* TCP/UDP Radio Buttons */
resource 'CNTL' (10015, "") {
	{  0,   0,  65, 176},
	0,
	visible,
	0,
	0,
	kControlRadioGroupProc,
	0,
	""
};
