
#ifndef _GUSINetDB_
#define _GUSINetDB_

#ifdef GUSI_SOURCE
#include "GUSISpecific.h"

#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSIhostent : public hostent {
public:
	GUSIhostent();
	
	void	Alloc(size_t size);

	char *	fAlias[16];
	char *	fAddressList[16];
	char * 	fName;
	size_t	fAlloc;
	char 	fAddrString[16];
};

extern "C" void GUSIKillHostEnt(void * hostent);



class GUSIservent : public servent {
public:
	GUSIservent();
	
	char *	fAlias[8];
	char 	fName[256];	
};


extern "C" void GUSIKillServiceDBData(void * entry);

class GUSIServiceDB {
public:
	static GUSIServiceDB *	Instance();
	
class iterator {
public:
	inline bool 			operator==(const iterator & other);
	inline bool 			operator!=(const iterator & other);
	inline iterator &		operator++();
	inline servent *		operator*();
};
inline static iterator	begin();
inline static iterator	end();

protected:
	static GUSIServiceDB *	sInstance;
							GUSIServiceDB()		{}
	virtual 				~GUSIServiceDB()	{}
	
	friend void GUSIKillServiceDBData(void * entry);
	
	
friend class iterator;

class Data {
public:
	Data() : fCurrent(0) {}
	
	servent *	fCurrent;
	GUSIservent	fServent;
};
typedef GUSISpecificData<Data, GUSIKillServiceDBData> SpecificData;
static 	SpecificData sData;

virtual void	Reset() = 0;
virtual void	Next()	= 0;

}; 


class GUSINetDB {
public:
	
	static GUSINetDB * 	Instance();

	
virtual hostent *	gethostbyname(const char * name);
virtual hostent *	gethostbyaddr(const void * addr, size_t len, int type);
virtual char *		inet_ntoa(in_addr inaddr);
virtual in_addr_t	inet_addr(const char *address);
virtual long		gethostid();
virtual int			gethostname(char *machname, int buflen);

	
virtual servent *	getservbyname(const char * name, const char * proto);
virtual servent *	getservbyport(int port, const char * proto);
virtual servent *	getservent();
virtual void		setservent(int stayopen);
virtual void		endservent();

	
virtual protoent *	getprotobyname(const char * name);
virtual protoent *	getprotobynumber(int proto);
virtual protoent * 	getprotoent();
virtual void		setprotoent(int stayopen);
virtual void		endprotoent();

protected:
	GUSINetDB();
	virtual ~GUSINetDB()		{}
	
static GUSINetDB *		sInstance;

bool					fServiceOpen;
bool					fServiceValid;
GUSIServiceDB::iterator	fServiceIter;

int				fNextProtocol;
static protoent	sProtocols[2];

}; 


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif

#ifdef GUSI_INTERNAL


GUSIServiceDB::iterator	GUSIServiceDB::begin()
{
	Instance()->Reset();
	Instance()->Next();
	
	return iterator();
}
GUSIServiceDB::iterator	GUSIServiceDB::end()
{
	return iterator();
}
bool GUSIServiceDB::iterator::operator==(const GUSIServiceDB::iterator &)
{
	return !GUSIServiceDB::sData->fCurrent;
}
bool GUSIServiceDB::iterator::operator!=(const GUSIServiceDB::iterator &)
{
	return GUSIServiceDB::sData->fCurrent 
		== static_cast<servent *>(nil);
}
GUSIServiceDB::iterator & GUSIServiceDB::iterator::operator++()
{
	GUSIServiceDB::Instance()->Next();
	return *this;
}
servent * GUSIServiceDB::iterator::operator*()
{
	return GUSIServiceDB::sData->fCurrent;
}


#endif /* GUSI_INTERNAL */

#endif /* GUSI_SOURCE */

#endif /* _GUSINetDB_ */
