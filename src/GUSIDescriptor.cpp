
#include "GUSIInternal.h"
#include "GUSIDescriptor.h"
#include "GUSIBasics.h"
#include "GUSIDiag.h"
#include "GUSINull.h"

#include <errno.h>
#include <fcntl.h>
#include <utility>
#include <memory>

GUSI_USING_STD_NAMESPACE

GUSIDescriptorTable *GUSIDescriptorTable::Instance()
{
	static bool sNeedConsoleSetup = true;

	if (!sGUSIDescriptorTable)
	{
		GUSISetupDescriptorTable();

		if (!sGUSIDescriptorTable)
			sGUSIDescriptorTable = new GUSIDescriptorTable();
	}
	if (sNeedConsoleSetup)
	{
		sNeedConsoleSetup = false;
		GUSISetupConsole();
	}
	return sGUSIDescriptorTable;
}

int GUSIDescriptorTable::InstallSocket(GUSISocket *sock, int start)
{
	if (start < 0 || start >= SIZE)
		return GUSISetPosixError(EINVAL);

	while (start < fInvalidDescriptor)
		if (!fSocket[start])
			goto found;
		else
			++start;
	while (start > fInvalidDescriptor)
		fSocket[fInvalidDescriptor++] = static_cast<GUSISocket *>(nil);
	if (start < SIZE)
		++fInvalidDescriptor;
	else
		return GUSISetPosixError(EMFILE);

found:
	fSocket[start] = sock;

	sock->AddReference();

	return start;
}

int GUSIDescriptorTable::RemoveSocket(int fd)
{
	GUSISocket *sock;

	if (fd < 0 || fd >= fInvalidDescriptor || !(sock = fSocket[fd]))
		return GUSISetPosixError(EBADF);

	fSocket[fd] = nil;

	sock->RemoveReference();

	return 0;
}

GUSISocket *GUSIDescriptorTable::operator[](int fd)
{
	GUSISocket *sock;

	if (fd < 0 || fd >= fInvalidDescriptor || !(sock = fSocket[fd]))
		return GUSISetPosixError(EBADF), static_cast<GUSISocket *>(nil);
	else
		return sock;
}

GUSISocket *GUSIDescriptorTable::LookupSocket(int fd)
{
	GUSIDescriptorTable *table = Instance();
	GUSISocket *sock;

	if (fd < 0 || fd >= table->fInvalidDescriptor || !(sock = table->fSocket[fd]))
		return GUSISetPosixError(EBADF), static_cast<GUSISocket *>(nil);
	else
		return sock;
}

GUSIDescriptorTable::GUSIDescriptorTable(const GUSIDescriptorTable &parent)
	: fInvalidDescriptor(parent.fInvalidDescriptor)
{
	memcpy(fSocket, parent.fSocket, fInvalidDescriptor * sizeof(GUSISocket *));

	iterator e = end();
	for (iterator i = begin(); i != e; ++i)
		if (GUSISocket *s = fSocket[*i])
			s->AddReference();
}
