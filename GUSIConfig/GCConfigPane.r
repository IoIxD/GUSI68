/*
	File:			GUSIConfigPane.r

	Contains:	Resources for configuration pane.

	Derived from Apple's Appearance SDK.

	$Log: GCConfigPane.r,v $
	Revision 1.2  1999/06/24 06:39:33  neeri
	Added SIGINT flag
	
	Revision 1.1  1999/04/10 03:52:46  neeri
	Initial Version
	
*/

#include "Dialogs.r"
#include "Controls.r"

resource 'DITL' (1003) {
	{	{ 40,  15, 255, 485}, Control 	  { enabled, 10030},
		{ 55,  20, 250, 480}, Control 	  {disabled, 10031},
	 	{ 60,  25, 245, 240}, Control 	  {disabled, 10032},
	 	{ 80,	 35, 106, 130}, StaticText	  {disabled, "Default Type"},
	 	{ 82, 160,  98, 210}, EditText     {disabled, "TEXT"},
	 	{113,	 35, 129, 130}, StaticText	  {disabled, "Default Creator"},
	 	{115, 160, 131, 210}, EditText     {disabled, "MPS "},
		{ 70, 250,  88, 475}, CheckBox	  { enabled, "Spin Automatically"},
		{ 94, 250, 112, 475}, CheckBox	  { enabled, "Accurate Subdirectory Count"},
		{118, 250, 136, 475}, CheckBox	  { enabled, "Open Files in Shared Mode"},
		{142, 250, 160, 475}, CheckBox	  { enabled, "Automatically Initialize QuickDraw"},
		{166, 250, 184, 475}, CheckBox	  { enabled, "Automatically Handle AppleEvents"},
		{190, 250, 208, 475}, CheckBox	  { enabled, "Raise SIGPIPE on broken pipes"},
		{214, 250, 232, 475}, CheckBox	  { enabled, "Raise SIGINT on command-period"}
	}
};

/* TCP/IP: Enable Checkbox Group */

resource 'CNTL' (10030, "") {
	{  0,   0, 215, 470},
	0,
	visible,
	1,
	0,
	kControlGroupBoxCheckBoxProc,
	0,
	"Enable Hardcoded Configuration"
};

/* Config Contents Group */

resource 'CNTL' (10031, "") {
	{  0,   0, 195, 460},
	2,
	visible,
	0,
	0,
	kControlUserPaneProc,
	0,
	""
};

resource 'CNTL' (10032, "") {
	{  0,   0, 185, 215},
	0,
	visible,
	0,
	0,
	kControlGroupBoxSecondaryTextTitleProc,
	0,
	"File Type Mapping"
};

