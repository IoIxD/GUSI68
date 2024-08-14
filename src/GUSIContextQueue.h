
#ifndef _GUSIContextQueue_
#define _GUSIContextQueue_

#ifndef GUSI_SOURCE

typedef struct GUSIContextQueue GUSIContextQueue;
#else

#include <stdlib.h>


class GUSIContext;


#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSIContextQueue {
public:
	GUSIContextQueue();
	~GUSIContextQueue();
	
	bool			empty();
	GUSIContext *	front() const;
	GUSIContext *	back() const;
	void			push_front(GUSIContext * context);
	void			push_back(GUSIContext * context);
	void			push(GUSIContext * context) 		{ push_back(context); }
	void			pop_front();
	void			pop() 								{ pop_front(); }
	void			remove(GUSIContext * context);
	
	void			Wakeup();

	
struct element;
class iterator {
	friend class GUSIContextQueue;
public:
	iterator & operator++();
	iterator operator++(int);
	bool operator==(const iterator other) const;
	GUSIContext * operator*();
	GUSIContext * operator->();
private:
	
element *	fCurrent;

iterator(element * elt) : fCurrent(elt) {}
iterator()				: fCurrent(0)   {}

};

iterator 		begin();
iterator 		end();

private:
	
struct element {
	GUSIContext *	fContext;
	element *		fNext;
	
	element(GUSIContext * context, element * next = 0) 
		: fContext(context), fNext(next) {}
	void * operator new(size_t);
	void   operator delete(void *, size_t);
private:
	
struct header {
	short	fFree;
	short 	fMax;
	header *fNext;
};
static header *	sBlocks;

};

element *	fFirst;
element *	fLast;

};


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


inline GUSIContextQueue::GUSIContextQueue() 
	: fFirst(0), fLast(0)
{
}

inline bool GUSIContextQueue::empty()
{
	return !fFirst;
}

inline GUSIContext * GUSIContextQueue::front() const
{
	return fFirst ? fFirst->fContext : reinterpret_cast<GUSIContext *>(0);
}

inline GUSIContext * GUSIContextQueue::back() const
{
	return fLast ? fLast->fContext : reinterpret_cast<GUSIContext *>(0);
}

inline void GUSIContextQueue::push_front(GUSIContext * context)
{
	fFirst = new element(context, fFirst);
	if (!fLast)
		fLast = fFirst;
}

inline void GUSIContextQueue::pop_front()
{
	if (element * e = fFirst) {
		if (!(fFirst = fFirst->fNext))
			fLast = 0;
		delete e;
	}
}

inline GUSIContextQueue::iterator GUSIContextQueue::begin()
{
	return iterator(fFirst);
}

inline GUSIContextQueue::iterator GUSIContextQueue::end()
{
	return iterator();
}

inline GUSIContextQueue::iterator & GUSIContextQueue::iterator::operator++()
{
	fCurrent = fCurrent->fNext;
	
	return *this;
}

inline GUSIContextQueue::iterator GUSIContextQueue::iterator::operator++(int)
{
	GUSIContextQueue::iterator it(*this);
	fCurrent = fCurrent->fNext;
	
	return it;
}

inline bool GUSIContextQueue::iterator::operator==(const iterator other) const
{
	return fCurrent == other.fCurrent;
}

inline GUSIContext * GUSIContextQueue::iterator::operator*()
{
	return fCurrent->fContext;
}

inline GUSIContext * GUSIContextQueue::iterator::operator->()
{
	return fCurrent->fContext;
}

#endif /* GUSI_SOURCE */

#endif /* _GUSIContextQueue_ */
