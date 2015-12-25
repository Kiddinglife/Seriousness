#ifndef __JACKIESPINLOCK_H__
#define __JACKIESPINLOCK_H__
#include "JACKIE_Atomic.h"
class JackieSpinLock
{
	public:
	JackieAtomicLong flag;
	JackieSpinLock();
	~JackieSpinLock();
	void lock(void)
	{
		while( flag.GetValue() > 0 ) { };
		flag.Increment();
	}

	void unlock()
	{
		flag.Decrement();
	}
};
#endif  //__JACKIESPINLOCK_H__

