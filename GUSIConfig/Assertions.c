/*
	File:		Assertions.c

	Contains:	Assertion routines.

	Version:	Appearance 1.0 SDK

	Copyright:	  1997 by Apple Computer, Inc., all rights reserved.

	$Log: Assertions.c,v $
	Revision 1.1  1999/06/24 06:43:41  neeri
	Added missing files

*/

#define DEBUG_MSG_TYPE 0

#include "Assertions.h"
#include <Types.h>
#include <stdio.h>
#include <TextUtils.h>

void AssertMsg(char *msg, char *file, int line)
{

	char newMsg[256];

	sprintf(newMsg, "%s, File: %s, Line: %d", msg, file, line);
	DebugStr(c2pstr(newMsg));
}
