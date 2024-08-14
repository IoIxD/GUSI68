/*
	File:		GUSIConfig.r

	Contains:	Resources for GUSIConfig.

	Derived from Apple's Appearance SDK.

	$Log: GUSIConfig.r,v $
	Revision 1.1  1999/04/10 03:52:49  neeri
	Initial Version
	
*/

#include "Menus.r"
#include "Dialogs.r"
#include "Controls.r"

#define teFlushRight -1 					/*flush right for all scripts */

resource 'xmnu' (128, purgeable)
{
	versionZero
	{
		{
			dataItem { 'abou', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};

resource 'xmnu' (129, purgeable)
{
	versionZero
	{
		{
			dataItem { 'new ', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'open', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			dataItem { 'clos', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph },
			skipItem {},
			dataItem { 'quit', kMenuNoModifiers, currScript, 0, 0, noHierID, sysFont, naturalGlyph }
		}
	};
};

resource 'DLOG' (1000, "GUSI Configuration") {
	{  0,  0, 265, 500},
	1024,
	visible,
	goAway,
	0x0,
	1000,
	"GUSI Configuration",
	alertPositionParentWindowScreen
};

resource 'dlgx' (1000) {
	versionZero {
		kDialogFlagsUseThemeBackground
	  + kDialogFlagsUseControlHierarchy
	  + kDialogFlagsUseThemeControls
	}
};

resource 'DITL' (1000) {
	{	
		{10, -2, 267, 502}, Control { enabled, 10000 }
	}
};

resource 'CNTL' (10000, "Configuration - Tabs") {
	{  0,  0, 257, 504},
	10000,
	visible,
	0,
	0,
	kControlTabLargeProc,
	0,
	"CNTL"
};

resource 'tab#' (10000, purgeable) 
{
	versionZero
	{ { 
		0, "Sockets",
		0, "Devices",
		0, "Options" 	
	} }
};

resource 'MBAR' (128) {
	{	/* array MenuArray: 2 elements */
		/* [1] */
		128,
		/* [2] */
		129
	}
};

resource 'MENU' (128) {
	128,
	63,
	0x7FFFFFFD,
	enabled,
	apple,
	{	
		"About GUSIConfig", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129) {
	129,
	63,
	0x7FFFFFF7,
	enabled,
	"File",
	{	
		"New…",		noIcon, "N", noMark, plain,
		"Open…",	noIcon, "O", noMark, plain,
		"Close", 	noIcon, "W", noMark, plain,
		"-", 		noIcon, noKey, noMark, plain,
		"Quit", 	noIcon, "Q", noMark, plain
	}
};
