
#ifndef _GUSIDescriptor_
#define _GUSIDescriptor_

#ifdef GUSI_SOURCE

#include "GUSISocket.h"

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSIDescriptorTable {
public:
	enum { SIZE = 64 };
	
	static GUSIDescriptorTable * 	Instance();
	
	int					InstallSocket(GUSISocket * sock, int start = 0);
	int					RemoveSocket(int fd);
	GUSISocket * 		operator[](int fd);
	static GUSISocket *	LookupSocket(int fd);
	
	class iterator;
	friend class iterator;
	
	iterator			begin();
	iterator			end();
	
	~GUSIDescriptorTable();
	
	static void			CloseAllDescriptors();

	static void SetInstance(GUSIDescriptorTable * table);
	
	GUSIDescriptorTable();
	GUSIDescriptorTable(const GUSIDescriptorTable & parent);
private:
	
GUSISocket *	fSocket[SIZE];
int				fInvalidDescriptor;

static	GUSIDescriptorTable	* sGUSIDescriptorTable;

};


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


extern "C" {
void GUSISetupDescriptorTable();
void GUSISetupConsole();
void GUSIDefaultSetupConsole();
void GUSISetupConsoleDescriptors();
void GUSISetupConsoleStdio();
}

extern "C" {
void GUSIStdioClose();	
void GUSIStdioFlush();
}



class GUSIDescriptorTable::iterator {
public:
	iterator(GUSIDescriptorTable * table, int fd = 0) : fTable(table), fFd(fd) {} 	
	GUSIDescriptorTable::iterator & operator++();
	GUSIDescriptorTable::iterator operator++(int);
	int	operator*()				{ return fFd;							}
	bool operator==(const GUSIDescriptorTable::iterator & other) const;
private:
	GUSIDescriptorTable *		fTable;
	int							fFd;
};

inline GUSIDescriptorTable::iterator & GUSIDescriptorTable::iterator::operator++()
{
	while (++fFd < fTable->fInvalidDescriptor && !fTable->fSocket[fFd])
		;
	
	return *this;
}

inline GUSIDescriptorTable::iterator GUSIDescriptorTable::iterator::operator++(int)
{
	int oldFD = fFd;
	
	while (++fFd < fTable->fInvalidDescriptor && !fTable->fSocket[fFd])
		;
	
	return GUSIDescriptorTable::iterator(fTable, oldFD);
}

inline bool GUSIDescriptorTable::iterator::operator==(
				const GUSIDescriptorTable::iterator & other) const
{
	return fFd == other.fFd;
}

inline GUSIDescriptorTable::iterator GUSIDescriptorTable::begin()
{
	return iterator(this);
}

inline GUSIDescriptorTable::iterator GUSIDescriptorTable::end()
{
	return iterator(this, fInvalidDescriptor);
}


#endif /* GUSI_SOURCE */

#endif /* _GUSIDescriptor_ */
