
#ifndef _GUSITimer_
#define _GUSITimer_

#ifndef GUSI_SOURCE

typedef struct GUSITimer GUSITimer;

#else
#include "GUSISpecific.h"

#include <errno.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <inttypes.h>

#include <MacTypes.h>
#include <Timer.h>
#include <Math64.h>

#include <ConditionalMacros.h>

#if PRAGMA_STRUCT_ALIGN
#pragma options align=native
#endif


class GUSITime {
public:
	enum Format {seconds, ticks, msecs, usecs, nsecs};
	
#if !TYPE_LONGLONG
	GUSITime(int32_t  val, Format format);
	GUSITime(uint32_t val, Format format);
#endif
	GUSITime(int64_t val, Format format=nsecs) 	{ Construct(val, format); }
	GUSITime(const timeval & tv);
	GUSITime(const timespec & ts);
	GUSITime(const tm & t);
	GUSITime() {}
	
	int32_t		Get(Format format)		{ return S32Set(Get64(format)); 		}
	uint32_t	UGet(Format format)		
							{ return U32SetU(SInt64ToUInt64(Get64(format)));	}
	int64_t		Get64(Format format);
	
	operator int64_t()		{	return fTime;	}
	operator timeval();
	operator timespec();
	operator tm();
	
	GUSITime GM2LocalTime();
	GUSITime Local2GMTime();
	
	GUSITime & operator +=(const GUSITime & other) 
		{ fTime = S64Add(fTime, other.fTime); return *this; }
	GUSITime & operator -=(const GUSITime & other) 
		{ fTime = S64Subtract(fTime, other.fTime); return *this; }
	
	
	static GUSITime 	Now();
	static timezone	&	Zone();
private:
	void	Construct(int64_t val, Format format);
	time_t	Deconstruct(int64_t & remainder);
	
	int64_t	fTime;
	
	static	int64_t		sTimeOffset;
	static	timezone	sTimeZone;
};

inline GUSITime operator+(const GUSITime & a, const GUSITime & b)
					{ GUSITime t(a); return t+=b; }
inline GUSITime operator-(const GUSITime & a, const GUSITime & b)
					{ GUSITime t(a); return t-=b; }


#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
class GUSIContext;

extern "C" void GUSIKillTimers(void * timers);

class GUSITimer : public TMTask {
public:
	GUSITimer(bool wakeup = true, GUSIContext * context = 0);
	virtual ~GUSITimer();
	
	void			Sleep(long ms, bool driftFree = false);
	void			MicroSleep(long us, bool driftFree = false)
										{ Sleep(-us, driftFree);				}
	GUSIContext *	Context()			{ return fQueue->fContext;				}
	GUSITimer *		Next()				{ return fNext;							}
	bool			Primed()			{ return (qType&kTMTaskActive) != 0; 	}
	bool			Expired()			{ return !(qType&kTMTaskActive); 		}
	virtual void	Wakeup();
	void			Kill();
	void			Reset();
	
	struct Queue {
		GUSITimer 	*	fTimer;
		GUSIContext *	fContext;
		
		Queue() : fTimer(0) {}
	};
	
	QElem *	Elem()		{	return reinterpret_cast<QElem *>(&this->qLink); }
protected:
	Queue *			fQueue;
	GUSITimer * 	fNext;
	
	class TimerQueue : public GUSISpecificData<Queue,GUSIKillTimers> {
	public:
		~TimerQueue();
	};
	
	static TimerQueue	sTimerQueue;
	static TimerUPP		sTimerProc;
};
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif


#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif

#endif /* GUSI_SOURCE */

#endif /* _GUSITimer_ */
