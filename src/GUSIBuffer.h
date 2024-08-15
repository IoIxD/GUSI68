
#ifndef _GUSIBuffer_
#define _GUSIBuffer_

#ifdef GUSI_SOURCE

#include <sys/types.h>
#include <sys/uio.h>

#include <MacTypes.h>

#include "GUSIDiag.h"
#include "GUSIBasics.h"

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align = native
#endif

class GUSIScattGath
{
protected:
	GUSIScattGath(const iovec *iov, int count, bool gather);
	GUSIScattGath(void *buffer, size_t length, bool gather);
	virtual ~GUSIScattGath();

public:
	const iovec *IOVec() const;
	int Count() const;
	void *Buffer() const;
	operator void *() const;
	int Length() const;
	int SetLength(int len) const;
	void operator=(const GUSIScattGath &other);
	GUSIScattGath(const GUSIScattGath &other);

private:
	const iovec *fIo;
	iovec fTrivialIo;
	mutable int fCount;
	mutable Handle fScratch;
	mutable void *fBuf;
	mutable int fLen;
	bool fGather;
};

class GUSIScatterer : public GUSIScattGath
{
public:
	GUSIScatterer(const iovec *iov, int count)
		: GUSIScattGath(iov, count, false) {}
	GUSIScatterer(void *buffer, size_t length)
		: GUSIScattGath(buffer, length, false) {}

	GUSIScatterer &operator=(const GUSIScatterer &other)
	{
		*static_cast<GUSIScattGath *>(this) = other;
		return *this;
	}
};

class GUSIGatherer : public GUSIScattGath
{
public:
	GUSIGatherer(const struct iovec *iov, int count)
		: GUSIScattGath(iov, count, true) {}
	GUSIGatherer(const void *buffer, size_t length)
		: GUSIScattGath(const_cast<void *>(buffer), length, true) {}

	GUSIGatherer &operator=(const GUSIGatherer &other)
	{
		*static_cast<GUSIScattGath *>(this) = other;
		return *this;
	}
};

class GUSIRingBuffer
{
public:
	GUSIRingBuffer(size_t bufsiz);
	~GUSIRingBuffer();
	operator void *();

	void Produce(void *from, size_t &len);
	void Produce(const GUSIGatherer &gather, size_t &len, size_t &offset);
	void Produce(const GUSIGatherer &gather, size_t &len);
	void Consume(void *to, size_t &len);
	void Consume(const GUSIScatterer &scatter, size_t &len, size_t &offset);
	void Consume(const GUSIScatterer &scatter, size_t &len);
	size_t Free();
	size_t Valid();

	void *ProduceBuffer(size_t &len);
	void *ConsumeBuffer(size_t &len);
	void ValidBuffer(void *buffer, size_t len);
	void FreeBuffer(void *buffer, size_t len);

	void Lock();
	void Release();
	bool Locked();
	typedef void (*Deferred)(void *);
	void Defer(Deferred def, void *ar);
	void ClearDefer();

	void SwitchBuffer(size_t bufsiz);
	size_t Size();
	void PurgeBuffers();

	class Peeker
	{
	public:
		Peeker(GUSIRingBuffer &buffer);
		~Peeker();

		void Peek(void *to, size_t &len);
		void Peek(const GUSIScatterer &scatter, size_t &len);

	private:
		GUSIRingBuffer &fTopBuffer;
		GUSIRingBuffer *fCurBuffer;
		Ptr fPeek;

		void *PeekBuffer(size_t &len);
	};
	friend class Peeker;

	void Peek(void *to, size_t &len);
	void Peek(const GUSIScatterer &scatter, size_t &len);

private:
	Ptr fBuffer;
	Ptr fEnd;
	Ptr fConsume;
	Ptr fProduce;
	size_t fFree;
	size_t fValid;
	size_t fSpare;
	bool fInUse;

	bool Invariant();
	size_t Distance(Ptr from, Ptr to);

	int fLocked;
	Deferred fDeferred;
	void *fDeferredArg;

	GUSIRingBuffer *fNewBuffer;
	GUSIRingBuffer *fOldBuffer;
	void ObsoleteBuffer();

	void IterateIOVec(const GUSIScattGath &sg, size_t &len, size_t &offset, bool produce);
};

#if PRAGMA_STRUCT_ALIGN
#pragma options align = reset
#endif

inline const iovec *GUSIScattGath::IOVec() const
{
	return fIo;
}
inline int GUSIScattGath::Count() const
{
	return fCount;
}
inline GUSIScattGath::operator void *() const
{
	return Buffer();
}
inline int GUSIScattGath::Length() const
{
	return fLen;
}
inline int GUSIScattGath::SetLength(int len) const
{
	return GUSI_MUTABLE(GUSIScattGath, fLen) = len;
}

inline void GUSIRingBuffer::Produce(const GUSIGatherer &gather, size_t &len, size_t &offset)
{
	IterateIOVec(gather, len, offset, true);
}

inline void GUSIRingBuffer::Consume(const GUSIScatterer &scatter, size_t &len, size_t &offset)
{
	IterateIOVec(scatter, len, offset, false);
}

inline void GUSIRingBuffer::Produce(const GUSIGatherer &gather, size_t &len)
{
	size_t offset = 0;

	IterateIOVec(gather, len, offset, true);
}

inline void GUSIRingBuffer::Consume(const GUSIScatterer &scatter, size_t &len)
{
	size_t offset = 0;

	IterateIOVec(scatter, len, offset, false);
}

inline void GUSIRingBuffer::Lock() { ++fLocked; }
inline bool GUSIRingBuffer::Locked() { return (fLocked != 0); }
inline void GUSIRingBuffer::ClearDefer() { fDeferred = nil; }
inline void GUSIRingBuffer::Release()
{
	GUSI_CASSERT_INTERNAL(fLocked > 0);
	if (--fLocked <= 0 && fDeferred)
		fDeferred(fDeferredArg);
}
inline void GUSIRingBuffer::Defer(Deferred def, void *ar)
{
	fDeferred = def;
	fDeferredArg = ar;
}

inline size_t GUSIRingBuffer::Size() { return fEnd - fBuffer; }

inline void GUSIRingBuffer::Peek(void *to, size_t &len)
{
	Peeker peeker(*this);

	peeker.Peek(to, len);
}

inline void GUSIRingBuffer::Peek(const GUSIScatterer &scatter, size_t &len)
{
	Peeker peeker(*this);

	peeker.Peek(scatter, len);
}

#endif /* GUSI_SOURCE */

#endif /* _GUSIBuffer_ */
