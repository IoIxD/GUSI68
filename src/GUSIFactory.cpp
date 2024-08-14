
#include "GUSIInternal.h"
#include "GUSIFactory.h"
#include "GUSIDiag.h"
#include "GUSIInet.h"

int GUSISocketFactory::socketpair(int, int, int, GUSISocket *[2])
{
	return GUSISetPosixError(EOPNOTSUPP);
}

GUSISocketDomainRegistry::GUSISocketDomainRegistry()
{
	for (int i = 0; i < AF_MAX; ++i)
		factory[i] = nil;
}

GUSISocket *GUSISocketDomainRegistry::socket(int domain, int type, int protocol)
{
	if (!GUSI_CASSERT_CLIENT(domain >= 0 && domain < AF_MAX) || !factory[domain])
		return GUSISetPosixError(EAFNOSUPPORT), static_cast<GUSISocket *>(nil);
	return factory[domain]->socket(domain, type, protocol);
}

int GUSISocketDomainRegistry::socketpair(int domain, int type, int protocol, GUSISocket *s[2])
{
	if (!GUSI_CASSERT_CLIENT(domain >= 0 && domain < AF_MAX) || !factory[domain])
		return GUSISetPosixError(EAFNOSUPPORT);
	return factory[domain]->socketpair(domain, type, protocol, s);
}

GUSISocketFactory *GUSISocketDomainRegistry::AddFactory(int domain, GUSISocketFactory *f)
{
	if (!GUSI_CASSERT_INTERNAL(domain >= 0 && domain < AF_MAX))
		return nil;
	GUSISocketFactory *old = factory[domain];
	factory[domain] = f;

	return old;
}

void GUSISocketTypeRegistry::Initialize()
{
	if (!factory)
	{
		factory = new Entry[maxfactory];
		GUSISocketDomainRegistry::Instance()->AddFactory(domain, this);
	}
}

bool GUSISocketTypeRegistry::Find(int type, int protocol, bool exact, Entry *&found)
{
	Initialize();
	for (Entry *ent = factory; ent < factory + maxfactory; ++ent)
		if (!ent->factory)
		{
			found = ent;
			return false;
		}
		else if (

			(ent->type == type || (!exact && !ent->type))

			&& (ent->protocol == protocol || (!exact && (!ent->protocol || !protocol))))
		{
			found = ent;
			return true;
		}
	found = nil;
	return false;
}

GUSISocket *GUSISocketTypeRegistry::socket(int domain, int type, int protocol)
{
	Entry *ent;
	bool found = Find(type, protocol, false, ent);
	if (!GUSI_CASSERT_CLIENT(found))
		return GUSISetPosixError(EPROTONOSUPPORT), static_cast<GUSISocket *>(nil);
	return ent->factory->socket(domain, type, protocol);
}

int GUSISocketTypeRegistry::socketpair(int domain, int type, int protocol, GUSISocket *s[2])
{
	Entry *ent;
	bool found = Find(type, protocol, false, ent);
	if (!GUSI_CASSERT_CLIENT(found))
		return GUSISetPosixError(EPROTONOSUPPORT);
	return ent->factory->socketpair(domain, type, protocol, s);
}

GUSISocketFactory *GUSISocketTypeRegistry::AddFactory(int type, int protocol, GUSISocketFactory *f)
{
	Entry *ent;
	bool previous = Find(type, protocol, true, ent);
	if (!GUSI_CASSERT_INTERNAL(ent))
		return nil;
	GUSISocketFactory *old = previous ? ent->factory : nil;
	ent->type = type;
	ent->protocol = protocol;
	ent->factory = f;

	return old;
}

GUSISocketFactory *GUSISocketTypeRegistry::RemoveFactory(int type, int protocol)
{
	Entry *ent;
	if (!Find(type, protocol, true, ent))
		return nil;
	GUSISocketFactory *old = ent->factory;
	while (++ent - factory < maxfactory && ent->factory)
		ent[-1] = ent[0];
	ent[-1].factory = nil;

	return old;
}
