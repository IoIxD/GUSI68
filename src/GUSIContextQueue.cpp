
#include "GUSIInternal.h"

#define GUSI_DIAG GUSI_DIAG_CAUTIOUS
#include "GUSIDiag.h"

#include "GUSIContextQueue.h"
#include "GUSIContext.h"

#include <string.h>

#include <Memory.h>

void *GUSIContextQueue::element::operator new(size_t)
{
	header *b;

	for (b = sBlocks; b; b = b->fNext)
		if (b->fFree)
			break;

	if (!b)
	{
		short max = sBlocks ? (sBlocks->fMax << 1) : 64;
		b = reinterpret_cast<header *>(NewPtr(max << 3));
		if (!b)
			return 0;
		memset(b, 0, max << 3);
		b->fMax = max;
		b->fFree = max - 1;
		b->fNext = sBlocks;
		sBlocks = b;
	}

	element *e = reinterpret_cast<element *>(b);
	while ((++e)->fContext)
		;
	--b->fFree;

	return e;
}

void GUSIContextQueue::element::operator delete(void *elem, size_t)
{
	header *h = static_cast<header *>(elem);
	header *b;
	header *p = 0;

	for (b = sBlocks; b; b = b->fNext)
		if (h > b && h < b + b->fMax)
		{
			memset(h, 0, sizeof(header));
			if (++b->fFree == b->fMax - 1)
			{
				int sum = 0;
				for (header *s = sBlocks; s; s = s->fNext)
					if (s != b)
						if ((sum += s->fFree) > 32)
						{
							if (p)
								p->fNext = b->fNext;
							else
								sBlocks = b->fNext;
							DisposePtr(reinterpret_cast<Ptr>(b));

							break;
						}
			}
			return;
		}
	// Can't reach
}

GUSIContextQueue::~GUSIContextQueue()
{
	while (!empty())
		pop_front();
}

void GUSIContextQueue::remove(GUSIContext *context)
{
	if (fFirst)
		if (fFirst->fContext == context)
			pop_front();
		else
		{
			element *prev = fFirst;
			for (element *cur = prev->fNext; cur; cur = cur->fNext)
				if (cur->fContext == context)
				{
					if (!(prev->fNext = cur->fNext)) // Delete last element
						fLast = prev;
					delete cur;

					return;
				}
				else
					prev = cur;
		}
}

void GUSIContextQueue::push_back(GUSIContext *context)
{
	if (element *e = new element(context))
	{
		if (fLast)
			fLast->fNext = e;
		else
			fFirst = e;
		fLast = e;
	}
}

void GUSIContextQueue::Wakeup()
{
	GUSI_MESSAGE(("Wakeup #%h\n", this));
	for (element *cur = fFirst; cur; cur = cur->fNext)
		cur->fContext->Wakeup();
}
