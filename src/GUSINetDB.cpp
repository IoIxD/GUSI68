
#include "GUSIInternal.h"
#include "GUSINetDB.h"
#include "GUSIFileSpec.h"
#include "GUSIFactory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <Resources.h>
#include <Memory.h>


hostent * GUSINetDB::gethostbyname(const char *)
{
	return (h_errno = NO_RECOVERY), static_cast<hostent *>(nil);
}
hostent * GUSINetDB::gethostbyaddr(const void *, size_t, int)
{
	return (h_errno = NO_RECOVERY), static_cast<hostent *>(nil);
}
char * GUSINetDB::inet_ntoa(in_addr)
{
	return (h_errno = NO_RECOVERY), static_cast<char *>(nil);
}
in_addr_t GUSINetDB::inet_addr(const char * address)
{
	int a[4];
	if (sscanf(address, "%d.%d.%d.%d", a, a+1, a+2, a+3) != 4)
		return static_cast<in_addr_t>(GUSISetHostError(NO_RECOVERY));
	else if ((a[0] & 0xFFFFFF00) || (a[1] & 0xFFFFFF00) 
	      || (a[2] & 0xFFFFFF00) || (a[3] & 0xFFFFFF00))
		return static_cast<in_addr_t>(GUSISetHostError(NO_RECOVERY));
	else
		return (in_addr_t)((a[0] << 24) | (a[1] << 16) | (a[2] <<  8) | a[3]);
}
long GUSINetDB::gethostid()
{
	return 0;
}

int GUSINetDB::gethostname(char *machname, int buflen)
{
	static char * sHostName = nil;
	
	if (!sHostName) {
		in_addr ipaddr;
		if (!(ipaddr.s_addr = static_cast<in_addr_t>(gethostid()))) {
			
Handle 	hostString 	= GetResource('STR ', -16413);
char 	hsState 	= HGetState(hostString);
HLock(hostString);
sHostName = new char[**hostString+1];
memcpy(sHostName, *hostString+1, **hostString);
sHostName[**hostString] = 0;
HSetState(hostString, hsState);

		} else {
			
hostent * ent	= gethostbyaddr((char *) &ipaddr, 0, 0);
char * name 	= ent ? ent->h_name : inet_ntoa(ipaddr);
sHostName   	= new char[strlen(name)+1];
strcpy(sHostName, name);

		}
	}
	strncpy(machname, sHostName, unsigned(buflen));
	machname[buflen-1] = 0;  /* extra safeguard */

	return 0;
}

servent * GUSINetDB::getservent()
{
	if (!fServiceOpen)
		setservent(0);
	else if (!fServiceValid)
		++fServiceIter;
	if (fServiceIter == GUSIServiceDB::end())
		endservent();
	fServiceValid	= false;	// Refetch next time we call
	return *fServiceIter;
}
void GUSINetDB::setservent(int)
{
	fServiceIter 	= GUSIServiceDB::begin();
	fServiceOpen 	= true;
	fServiceValid	= true;	// Don't refetch first time getservent is called
}
void	GUSINetDB::endservent()
{
	fServiceOpen = false;
}

servent * GUSINetDB::getservbyname(const char * name, const char * proto)
{
	servent * ent;
	
	for (setservent(0); ent = getservent(); ) {
		if (!strcmp(name, ent->s_name))
			goto haveName;
		for (char ** al = ent->s_aliases; *al; ++al)
			if (!strcmp(name, *al))
				goto haveName;
		continue;
haveName:
		if (!proto || !strcmp(proto, ent->s_proto))
			break;
	}
	return ent;
}

servent * GUSINetDB::getservbyport(int port, const char * proto)
{
	servent * ent;
	
	for (setservent(0); ent = getservent(); )
		if (port == ent->s_port && (!proto || !strcmp(proto, ent->s_proto)))
			break;
	
	return ent;
}

protoent GUSINetDB::sProtocols[] = {
	{ "udp", {NULL}, IPPROTO_UDP},
	{ "tcp", {NULL}, IPPROTO_TCP}
};

protoent * GUSINetDB::getprotoent()
{
	fNextProtocol = !fNextProtocol;
	return sProtocols+!fNextProtocol;
}
void GUSINetDB::setprotoent(int)
{
	fNextProtocol  = 0;
}
void GUSINetDB::endprotoent()
{
	fNextProtocol  = 0;
}

protoent * GUSINetDB::getprotobyname(const char * name)
{
	for (int i = 0; i<2; ++i)
		if (!strcmp(name, sProtocols[i].p_name))
			return sProtocols+i;
	return static_cast<protoent *>(nil);
}
protoent * GUSINetDB::getprotobynumber(int proto)
{
	for (int i = 0; i<2; ++i)
		if (proto == sProtocols[i].p_proto)
			return sProtocols+i;
	return static_cast<protoent *>(nil);
}



class GUSIFileServiceDB : public GUSIServiceDB {
public:
	static GUSIFileServiceDB * Instance();
protected:
	FILE *			fFile;
	pthread_mutex_t	fLock;
	
	GUSIFileServiceDB(FILE * file) 
		: fFile(file), fLock(0) { }
	
	virtual void	Reset();
	virtual void 	Next();
};



extern "C" void GUSIKillBuiltinServiceDBEntry(void * entry);

class GUSIBuiltinServiceDB : public GUSIServiceDB {
public:
	static GUSIBuiltinServiceDB * Instance() { return new GUSIBuiltinServiceDB; }
protected:
	friend void GUSIKillBuiltinServiceDBEntry(void * entry);
	
	struct Entry {
		Entry() : fValue(0) {}
		
		int	fValue;
	};
	typedef GUSISpecificData<Entry, GUSIKillBuiltinServiceDBEntry>	SpecificEntry;
	static SpecificEntry sEntry;
	
	virtual void	Reset();
	virtual void 	Next();
	
	static servent	sServices[];
};



GUSIFileServiceDB * GUSIFileServiceDB::Instance()
{
	GUSIFileSpec services(kPreferencesFolderType, kOnSystemDisk);
	
	if (services.Error())
		return static_cast<GUSIFileServiceDB *>(nil);
	
	services.SetName("\p/etc/services");
	
	FILE * f = fopen(services.FullPath(), "r");
	
	return f ? new GUSIFileServiceDB(f) : static_cast<GUSIFileServiceDB *>(nil);
}

void GUSIFileServiceDB::Reset()
{
	rewind(fFile);
}

void GUSIFileServiceDB::Next()
{
	GUSIservent & service = sData->fServent;

	pthread_mutex_lock(&fLock);
	sData->fCurrent = static_cast<servent *>(nil);
	while (fgets(service.fName, 128, fFile)) {
		char *	p;
		if (p = strpbrk(service.fName, "#\n\r")) // Line terminated at newline or '#'
			*p = 0;
		if (!(service.s_name = strtok(service.fName, " \t"))
		 || !(p = strtok(NULL, " \t"))
		 || !(service.s_proto = strpbrk(p, "/,"))
		)
			continue;
		
		*service.s_proto++ 	= 0;
		service.s_port 		= atoi(p);
	
		int aliascount;
		for (aliascount = 0; aliascount < 15; ) 
			if (!(service.s_aliases[aliascount++] = strtok(NULL, " \t")))
				break;
		service.s_aliases[aliascount] = NULL;
		sData->fCurrent = &service;
		
		break;
	}
	pthread_mutex_unlock(&fLock);
}


GUSIBuiltinServiceDB::SpecificEntry	GUSIBuiltinServiceDB::sEntry;

void GUSIKillBuiltinServiceDBEntry(void * entry)
{
	delete reinterpret_cast<GUSIBuiltinServiceDB::Entry *>(entry);
}

static char *	sNoAliases[1] = {NULL};

servent GUSIBuiltinServiceDB::sServices[] =
{
	{ "echo", 		sNoAliases,   7, "udp"},
	{ "discard", 	sNoAliases,   9, "udp"},
	{ "time", 		sNoAliases,  37, "udp"},
	{ "domain", 	sNoAliases,  53, "udp"},
	{ "sunrpc", 	sNoAliases, 111, "udp"},
	{ "tftp", 		sNoAliases,  69, "udp"},
	{ "biff", 		sNoAliases, 512, "udp"},
	{ "who", 		sNoAliases, 513, "udp"},
	{ "talk", 		sNoAliases, 517, "udp"},
	
	{ "ftp-data", 	sNoAliases,  20, "tcp"},
	{ "ftp", 		sNoAliases,  21, "tcp"},
    { "ssh",        sNoAliases,  22, "tcp"},
	{ "telnet", 	sNoAliases,  23, "tcp"},
	{ "smtp", 		sNoAliases,  25, "tcp"},
	{ "time", 		sNoAliases,  37, "tcp"},
	{ "whois", 		sNoAliases,  43, "tcp"},
	{ "domain", 	sNoAliases,  53, "tcp"},
	{ "hostnames", 	sNoAliases, 101, "tcp"},
	{ "nntp", 		sNoAliases, 119, "tcp"},
	{ "finger", 	sNoAliases,  79, "tcp"},
	{ "http", 		sNoAliases,  80, "tcp"},
	{ "ntp", 		sNoAliases, 123, "tcp"},
	{ "uucp", 		sNoAliases, 540, "tcp"},
	{ NULL, NULL, -1, NULL}
};

void GUSIBuiltinServiceDB::Reset()
{
	sEntry->fValue = 0;
}
void GUSIBuiltinServiceDB::Next()
{
	if (!sServices[sEntry->fValue].s_name)
		sData->fCurrent = static_cast<servent *>(nil);
	else
		sData->fCurrent = sServices + sEntry->fValue++;
}


GUSIServiceDB::SpecificData	GUSIServiceDB::sData;

void GUSIKillServiceDBData(void * data)
{
	delete reinterpret_cast<GUSIServiceDB::Data *>(data);
}

GUSIServiceDB * GUSIServiceDB::sInstance;

GUSIServiceDB * GUSIServiceDB::Instance()
{
	if (!sInstance) 
		if (!(sInstance = GUSIFileServiceDB::Instance()))
			sInstance = GUSIBuiltinServiceDB::Instance();
	return sInstance;
}


GUSIhostent::GUSIhostent()
 : fName(nil), fAlloc(0)
{
	h_aliases 	= fAlias;
	h_addr_list	= fAddressList;
}
void GUSIhostent::Alloc(size_t size)
{
	if (size > fAlloc) {
		if (fName)
			delete[] fName;
		h_name = fName = new char[fAlloc = size];
	}
}
void GUSIKillHostEnt(void * hostent)
{
	delete reinterpret_cast<GUSIhostent *>(hostent);
}


GUSIservent::GUSIservent()
{
	s_aliases	= fAlias;
}

