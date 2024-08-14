
#include "GUSIInternal.h"
#include "GUSIBuffer.h"

#include <algorithm>
#include <stdlib.h>
#include <Memory.h>

#include <algorithm>

using std::min;

GUSIScattGath::GUSIScattGath(const iovec *iov, int cnt, bool gather)
	: fIo(iov), fCount(cnt), fScratch(nil), fGather(gather)
{
	if (fCount < 1)
	{

		fBuf = nil;
		fLen = 0;
	}
	else if (fCount == 1)
	{
		fBuf = (void *)iov->iov_base;
		fLen = (int)iov->iov_len;
	}
	else
	{

		fBuf = nil;
		for (fLen = 0; cnt--; ++iov)
			fLen += (int)iov->iov_len;
	}
}

GUSIScattGath::GUSIScattGath(void *buffer, size_t length, bool gather)
	: fIo(&fTrivialIo), fCount(1), fScratch(nil), fGather(gather)
{
	fTrivialIo.iov_base = static_cast<caddr_t>(fBuf = buffer);
	fTrivialIo.iov_len = fLen = length;
}

void *GUSIScattGath::Buffer() const
{
	if (!fScratch && fCount > 1)
	{
		if (GUSI_MUTABLE(GUSIScattGath, fScratch) = NewHandle(fLen))
		{
			HLock(fScratch);
			GUSI_MUTABLE(GUSIScattGath, fBuf) = (void *)*fScratch;
			if (fGather)
			{

				char *buffer = static_cast<char *>(fBuf);
				const iovec *io = fIo;
				for (int count = fCount; count--; ++io)
				{
					memcpy(buffer, io->iov_base, io->iov_len);

					buffer += io->iov_len;
				}
			}
		}
		else
		{
			GUSI_MUTABLE(GUSIScattGath, fCount) = 0;
			GUSI_MUTABLE(GUSIScattGath, fBuf) = nil;
		}
	}

	return fBuf;
}

GUSIScattGath::~GUSIScattGath()
{
	if (fScratch)
	{
		if (!fGather)
		{

			char *buffer = static_cast<char *>(fBuf);
			const iovec *io = fIo;
			int length = fLen;
			for (int count = fCount; count-- && length; ++io)
			{
				int sect = min(length, int(io->iov_len));

				memcpy(io->iov_base, buffer, sect);

				buffer += sect;
				length -= sect;
			}
		}
		DisposeHandle(fScratch);
	}
}

void GUSIScattGath::operator=(const GUSIScattGath &other)
{
	if (fScratch)
		DisposeHandle(fScratch);

	fIo = other.fIo;
	fCount = other.fCount;
	fLen = other.fLen;
	fGather = other.fGather;

	fScratch = nil;
	fBuf = other.fScratch ? nil : other.fBuf;
}

GUSIScattGath::GUSIScattGath(const GUSIScattGath &other)
{

	fIo = other.fIo;
	fCount = other.fCount;
	fLen = other.fLen;
	fGather = other.fGather;

	fScratch = nil;
	fBuf = other.fScratch ? nil : other.fBuf;
}

size_t GUSIRingBuffer::Distance(Ptr from, Ptr to)
{
	if (from > to)
		return (fEnd - from) + (to - fBuffer);
	else
		return to - from;
}

bool GUSIRingBuffer::Invariant()
{
	Lock();

	bool invariant =
		GUSI_CASSERT_INTERNAL(fProduce >= fBuffer && fProduce < fEnd) && GUSI_CASSERT_INTERNAL(fConsume >= fBuffer && fConsume < fEnd) && GUSI_CASSERT_INTERNAL(fFree + fValid + fSpare == fEnd - fBuffer) && GUSI_CASSERT_INTERNAL(Distance(fConsume, fProduce) == (fValid + fSpare) % (fEnd - fBuffer)) && GUSI_CASSERT_INTERNAL(Distance(fProduce, fConsume) == fFree % (fEnd - fBuffer));

	Release();

	return invariant;
}

GUSIRingBuffer::GUSIRingBuffer(size_t bufsiz)
{

	fBuffer = fConsume = fProduce = bufsiz ? NewPtr(bufsiz) : 0;
	fEnd = fBuffer + bufsiz;

	fValid = fSpare = 0;
	fFree = bufsiz;

	fInUse = false;
	fLocked = 0;
	fDeferred = nil;
	fDeferredArg = nil;
	fNewBuffer = nil;
	fOldBuffer = nil;
	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::GUSIRingBuffer()!\n");
}

void GUSIRingBuffer::ObsoleteBuffer()
{
	Ptr oldBuffer = fBuffer;
	fBuffer = fNewBuffer->fBuffer;
	fEnd = fNewBuffer->fEnd;
	fConsume = fNewBuffer->fConsume;
	fProduce = fNewBuffer->fProduce;
	fFree = fNewBuffer->fFree;
	fValid = fNewBuffer->fValid;
	fSpare = fNewBuffer->fSpare;
	fInUse = fNewBuffer->fInUse;
	fNewBuffer->fOldBuffer = fOldBuffer;
	fOldBuffer = fNewBuffer;
	fNewBuffer = fOldBuffer->fNewBuffer;
	fOldBuffer->fBuffer = oldBuffer;
	fOldBuffer->fNewBuffer = nil;
}

GUSIRingBuffer::~GUSIRingBuffer()
{
	Lock();
	Release();
	if (fBuffer)
		DisposePtr(fBuffer);
	PurgeBuffers();
	if (fNewBuffer)
		delete fNewBuffer;
}

void *GUSIRingBuffer::ProduceBuffer(size_t &len)
{
	Lock();

	while (fNewBuffer)
	{
		if (!fValid)
			ObsoleteBuffer();
		else
		{
			void *buf = fNewBuffer->ProduceBuffer(len);
			Release();
			return buf;
		}
	}

	size_t requested_length = len;

	if (!fValid)
	{
		fProduce = fConsume = fBuffer;
		fSpare = 0;
		fFree = fEnd - fBuffer;
	}
	size_t streak = fEnd - fProduce;
	if (streak >= fFree)
		streak = fFree;
	else

		if (streak < (fFree >> 1) && streak < len)
	{
		fSpare = streak;
		fProduce = fBuffer;
		fFree -= fSpare;
		streak = fFree;
	}

	if (len > streak)
		len = streak;

	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::ProduceBuffer()!\n");
	GUSI_CASSERT_INTERNAL(len <= requested_length);
	void *result = fProduce;
	fInUse = true;
	Release();
	return result;
}

void *GUSIRingBuffer::ConsumeBuffer(size_t &len)
{
	Lock();
	size_t requested_length = len;

	while (fNewBuffer && !fValid && !fInUse)
		ObsoleteBuffer();

	size_t streak = fEnd - fConsume - fSpare;
	if (streak > fValid)
		streak = fValid;
	if (len > streak)
		len = streak;
	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::ConsumeBuffer()!\n");
	GUSI_CASSERT_INTERNAL(len <= requested_length);
	void *result = fConsume;
	Release();
	return result;
}

void GUSIRingBuffer::ValidBuffer(void *buffer, size_t len)
{
	Lock();
	if (fNewBuffer && (buffer < fBuffer || buffer >= fEnd))
	{
		fNewBuffer->ValidBuffer(buffer, len);
		Release();
		return;
	}
	GUSI_CASSERT_INTERNAL(len <= fFree);
	GUSI_CASSERT_INTERNAL(fProduce + len <= fEnd);
	fInUse = false;
	fValid += len;
	fFree -= len;
	fProduce += len;
	if (fProduce == fEnd)
		fProduce = fBuffer;

	while (fNewBuffer && !fValid && !fInUse)
		ObsoleteBuffer();

	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::ValidBuffer()!\n");
	Release();
}

void GUSIRingBuffer::FreeBuffer(void *, size_t len)
{
	Lock();
	fFree += len;
	fValid -= len;
	fConsume += len;

	if (fConsume == fEnd - fSpare)
	{
		fConsume = fBuffer;
		fFree += fSpare;
		fSpare = 0;
	}

	while (fNewBuffer && !fValid && !fInUse)
		ObsoleteBuffer();

	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::FreeBuffer()!\n");
	Release();
}

void GUSIRingBuffer::Produce(void *from, size_t &len)
{
	size_t part;
	size_t rest;
	void *buf;

	PurgeBuffers();
	for (rest = len; (part = rest) && fFree; rest -= part)
	{
		buf = ProduceBuffer(part);
		BlockMoveData(from, buf, part);
		ValidBuffer(buf, part);

		from = static_cast<char *>(from) + part;
	}
	len -= rest;
	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::Produce()!\n");
}

void GUSIRingBuffer::Consume(void *to, size_t &len)
{
	size_t part;
	size_t rest;
	void *buf;

	PurgeBuffers();
	for (rest = len; (part = rest) && fValid; rest -= part)
	{
		buf = ConsumeBuffer(part);
		if (to)
		{
			BlockMoveData(buf, to, part);
			to = static_cast<char *>(to) + part;
		}
		FreeBuffer(buf, part);
	}

	len -= rest;
	GUSI_SASSERT_INTERNAL(
		Invariant(), "Invariant violated in GUSIRingBuffer::Consume()!\n");
}

size_t GUSIRingBuffer::Free()
{
	if (fNewBuffer)
		return fNewBuffer->Free();
	else
		return fFree;
}

size_t GUSIRingBuffer::Valid()
{
	if (fNewBuffer)
		return fNewBuffer->Valid() + fValid;
	else
		return fValid;
}

void GUSIRingBuffer::Peeker::Peek(void *to, size_t &len)
{
	size_t part;
	size_t rest;
	void *buf;

	for (rest = len; (part = rest) && (buf = PeekBuffer(part)); rest -= part)
	{
		BlockMoveData(buf, to, part);
		to = static_cast<char *>(to) + part;
	}

	len -= rest;
}

void GUSIRingBuffer::Peeker::Peek(const GUSIScatterer &scatter, size_t &len)
{
	const iovec *vec = scatter.IOVec();
	iovec io = vec[0];

	while (!io.iov_len)
		io = *++vec;

	size_t part;
	size_t rest = len;
	while (part = min(rest, io.iov_len))
	{
		size_t donepart = part;
		Peek(io.iov_base, donepart);
		rest -= donepart;
		if (donepart != part)
			break;
		do
		{
			io = *++vec;
		} while (!io.iov_len);
	}
	len -= rest;
}
