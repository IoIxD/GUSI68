/*
	File:			GCData.h

	Contains:	Configuration data.

	$Log: GCData.h,v $
	Revision 1.1  1999/04/10 03:52:46  neeri
	Initial Version
	
*/

#pragma once

#include <MacTypes.h>

#include <utility>
#include <map>
#include <iostream>
#include <string>

typedef std::pair<bool, bool> 			GCFlag;
struct GCTypeCreator {
	OSType	fType;
	OSType	fCreator;
};
typedef std::map<std::string, GCFlag>			GCFlagTable;
typedef GCFlagTable::iterator 						GCFlagIterator;
typedef std::map<std::string, bool>				GCOptionTable;
typedef GCOptionTable::iterator 						GCOptionIterator;
typedef std::map<std::string, GCTypeCreator>	GCFileTable;
typedef GCFileTable::iterator 						GCFileIterator;

extern GCOptionTable	gGCSockets;
extern GCOptionTable	gGCDevices;
extern GCFlagTable	gGCFlags;
extern GCFileTable	gGCFiles;
extern OSType			gGCType;
extern OSType			gGCCreator;
extern Boolean			gGCHasConfig;
extern Boolean			gQuit;

void DefaultFlags(bool hasConfigSection);
void DefaultSockets(bool hasConfigSection);
void DefaultDevices(bool hasConfigSection);
Boolean ReadData(std::istream & s, Handle header, Handle footer);
void WriteData(std::ostream & s, Handle header, Handle footer);
