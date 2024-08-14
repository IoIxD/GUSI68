
#ifndef _GUSIFactory_
#define _GUSIFactory_

#ifdef GUSI_SOURCE

#include "GUSISocket.h"

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSISocketFactory {
public:
	virtual int socketpair(int domain, int type, int protocol, GUSISocket * s[2]);
	virtual GUSISocket * socket(int domain, int type, int protocol) = 0;
protected:
	GUSISocketFactory()				{}
	virtual ~GUSISocketFactory()	{}
};


class GUSISocketDomainRegistry : public GUSISocketFactory {
public:
	
virtual GUSISocket * socket(int domain, int type, int protocol);
virtual int  socketpair(int domain, int type, int protocol, GUSISocket * s[2]);
static  GUSISocketDomainRegistry *	Instance();

	
GUSISocketFactory * AddFactory(int domain, GUSISocketFactory * factory);
GUSISocketFactory * RemoveFactory(int domain);

private:
	
static GUSISocketDomainRegistry *	sInstance;

GUSISocketFactory * factory[AF_MAX];
GUSISocketDomainRegistry();

};


class GUSISocketTypeRegistry : public GUSISocketFactory {
public:
	
GUSISocketTypeRegistry(int domain, int maxfactory);
virtual GUSISocket * socket(int domain, int type, int protocol);
virtual int socketpair(int domain, int type, int protocol, GUSISocket * s[2]);

	
GUSISocketFactory * AddFactory(int type, int protocol, GUSISocketFactory * factory);
GUSISocketFactory * RemoveFactory(int type, int protocol);

private:
	
struct Entry {
	int					type;
	int					protocol;
	GUSISocketFactory * factory;
	Entry()	: type(0), protocol(0), factory(nil) {}
};
Entry 	* 	factory;
int			domain;
int			maxfactory;

void			Initialize();

bool Find(int type, int protocol, bool exact, Entry *&found);

};


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


extern "C" void GUSISetupFactories();



inline GUSISocketDomainRegistry * GUSISocketDomainRegistry::Instance()
{
	if (!sInstance) {
		sInstance = new GUSISocketDomainRegistry();
		GUSISetupFactories();
	}
	
	return sInstance;
}

inline GUSISocketFactory * GUSISocketDomainRegistry::RemoveFactory(int domain)
{
	return AddFactory(domain, nil);
}


inline GUSISocketTypeRegistry::GUSISocketTypeRegistry(int domain, int maxfactory)
 : domain(domain), maxfactory(maxfactory), factory(nil)
{
}


#endif /* GUSI_SOURCE */

#endif /* _GUSIFactory_ */
