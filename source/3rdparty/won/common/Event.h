/********************************************************************************

  WON::Event

	A simple Event object to encapsulate the behavior of Windows EVENT objects.

	12/30/98	-	Colen Garoutte-Carson	- Initial functionality

********************************************************************************/


#ifndef __WON_EVENT_H__
#define __WON_EVENT_H__


#ifdef _LINUX
#include <pthread.h>
#elif defined(WIN32)
#include <windows.h>
#pragma warning(disable : 4786)
#elif defined(macintosh) && (macintosh == 1)
#include "macGlue.h"
#else
#error unknown platform
#endif

#include "CriticalSection.h"
#include "wondll.h"

namespace WONCommon {


#ifndef _MT
	// || (defined(macintosh) && (macintosh == 1))
	// Mac version of lib doesn't support threading

class Event
{
private:
	bool state;
	bool manual_reset;

	Event* refersTo;

	bool WaitFor(DWORD timeout = INFINITE) const { return true; }

public:
	explicit Event(bool manualReset = true, bool initialState = false)
		:	manual_reset(manualReset), state(initialState), refersTo(0)
	{ }

	Event(Event& evt)
	{
		if (evt.refersTo)
			refersTo = evt.refersTo;
		else
			refersTo = &evt;
	}
	
	~Event()
	{ }

	Event& operator=(Event& evt)
	{
		if (evt.refersTo)
			refersTo = evt.refersTo;
		else
			refersTo = &evt;
		return *this;
	}

	bool Set()
	{
		if (refersTo)
			return refersTo->Set();
		state = true;
		return true;
	}

	bool Reset()
	{
		if (refersTo)
			return refersTo->Reset();
		state = false;
		return true;
	}

	bool Test()
	{
		bool result = state;
		if (manual_reset)
			state = false;
		return result;
	}
};


#elif defined(WIN32)

class Event
{
private:
	HANDLE mEvent;
	bool owns;

public:
	explicit Event(bool manualReset = true, bool initialState = false) : owns(true)
	{
		mEvent = CreateEvent(0, manualReset, initialState, 0); 
	}

	// If constructed with existing HANDLE causes Event object not to close it
	Event(HANDLE event) : owns(false), mEvent(event)		{ }
	Event(Event& evt) : owns(false), mEvent(evt.mEvent)		{ }

	~Event()
	{
		if (mEvent && owns)
			CloseHandle(mEvent);
	}

	Event& operator=(Event& evt)
	{
		if (mEvent && owns)
			CloseHandle(mEvent);
		owns = false;
		mEvent = evt.mEvent;
		return *this;
	}

	bool Set()
	{
		return mEvent ? SetEvent(mEvent) == TRUE : false;
	}

	bool Reset()
	{
		return mEvent ? ResetEvent(mEvent) == TRUE : false;
	}

	bool WaitFor(DWORD timeout = INFINITE)
	{
		return mEvent ? WaitForSingleObject(mEvent, timeout) == WAIT_OBJECT_0 : false;
	}

	bool Test()
	{
		return WaitFor(0);
	}

	HANDLE GetHandle() const
	{
		return mEvent;
	}

};


#elif defined(_LINUX)

class Event
{
private:
	mutable CriticalSection crit;
	mutable pthread_cond_t cond;
	mutable bool state;
	bool manual_reset;
	
	Event* refersTo;

public:
	explicit Event(bool manualReset = true, bool initialState = false)
		:	manual_reset(manualReset), state(initialState), refersTo(0)
	{
		pthread_cond_init(&cond, 0);
	}

	Event(Event& evt)
	{
		if (evt.refersTo)
			refersTo = evt.refersTo;
		else
			refersTo = &evt;
	}
	
	~Event()
	{
		if (!refersTo)
			pthread_cond_destroy(&cond);
	}

	Event& operator=(Event& evt)
	{
		if (!refersTo)
			pthread_cond_destroy(&cond);
		if (evt.refersTo)
			refersTo = evt.refersTo;
		else
			refersTo = &evt;
		return *this;
	}

	bool Set()
	{
		if (refersTo)
			return refersTo->Set();
		AutoCrit autoCrit(crit);
		if (!state)
		{
			if (manual_reset)
			{
				state = true;
				pthread_cond_broadcast(&cond);
			}
			else
				pthread_cond_signal(&cond);
		}
		return true;
	}

	bool Reset()
	{
		if (refersTo)
			return refersTo->Reset();
		AutoCrit autoCrit(crit);
		state = false;
		return true;
	}

	bool WaitFor(DWORD timeout = INFINITE)
	{
		if (refersTo)
			return refersTo->WaitFor(timeout);
		AutoCrit autoCrit(crit);
		if (state)
		{
			if (!manual_reset)
				state = false;
			return true;
		}
		if (timeout == INFINITE)
		{
			pthread_cond_wait(&cond, &(crit.mCrit));
			return true;
		}
		struct timespec waitTime = { (timeout / 1000), (timeout % 1000) * 1000000 };
		return pthread_cond_timedwait(&cond, &(crit.mCrit), &waitTime) != ETIMEDOUT;
	}

	bool Test()			{ return WaitFor(0); }
};


#endif



};

#endif
