#include "auto_obj.h"


using namespace WONCommon;


// empty default virtual destructor
AutoObjDescriptorBase::~AutoObjDescriptorBase()
{
}


// zero-initializing always occurs before static construction
static AutoObjDescriptorBase* deferredDeallocs = 0;
static CriticalSection* garbageCrit = 0;
static bool stopped = false;


// Starts and stops dealloc defering at static init and destruction
// auto_obj's that are released outside before this object is constructed
//   or after it is destructed, will be deleted immediately without defering
class AutoObjDynamicStatic
{
public:
	AutoObjDynamicStatic()
	{
		garbageCrit = new CriticalSection;
		// this critical section intentionally leaks
		// to handle for post- static destruction race condition issues
	}

	~AutoObjDynamicStatic()
	{
		stopped = true;
		PurgeGarbage();
	}
};
static AutoObjDynamicStatic autoObjDynamicStatic;



void WONCommon::PurgeGarbage(void)
{
	if (garbageCrit)
	{
		garbageCrit->Enter();
		while (deferredDeallocs)
		{
			AutoObjDescriptorBase* deleteMe = deferredDeallocs;
			deferredDeallocs = deferredDeallocs->nextDescriptor;
			garbageCrit->Leave();

			delete deleteMe;

			garbageCrit->Enter();
		}
		garbageCrit->Leave();
	}
}


void AutoObjDescriptorBase::QueueGarbage(void)
{
	if (garbageCrit)	// No dealloc defering pre-static construction
	{
		AutoCrit autoCrit(*garbageCrit);
		if (!stopped)
		{
			nextDescriptor = deferredDeallocs;
			deferredDeallocs = this;
			return;
		}
	}
	delete this;
}
