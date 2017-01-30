#ifndef __AUTO_OBJ_H_
#define __AUTO_OBJ_H_


#include "CriticalSection.h"


namespace WONCommon {


class AutoObjDescriptorBase
{
public:
	AutoObjDescriptorBase* nextDescriptor;

	void QueueGarbage(void);

	virtual ~AutoObjDescriptorBase();
};


/*
	auto_obj<> is like auto_ptr<>, but reference counts.
	It's intended to be copied by value.  When the last
	reference is destructed, the encapsulated pointer is
	deleted.

	Example 1:

	auto_obj<int> = new int;	// you don't need to worry about deleting it

	
	Example 2:

	void foo(const auto_obj<Class*>& i)
	{
		// This function could be passed a Class*, or an existing auto_obj<Class*>
		// Note: If a Class* is passed, it will be automatically deleted when done
	}

	Thread safey and garbage collection (actually just deallocation defering)
	are optional template arguments.  If you use dealloc deferring, be sure to
	purge garbage periodically by calling PurgeGarbage()

*/
template <class T, bool isThreadSafe = true, bool deferDealloc = false>
class auto_obj
{
public:
	bool empty() const				{ return (obj == 0); }
	operator bool() const throw()	{ return (obj != 0); }

	T* get() const throw()			{ return obj; }

/*
	Death to VC6! and all that have come before it!
	This works in auto_ptr<>, but not here...
	Sometimes it compiles, sometimes it internal errors!


	T& operator*() const throw()	{ return (*get()); };
#pragma warning(disable:4284)  
	T *operator->() const throw()	{ return get(); };
*/

protected:
	template <bool useCrit>
	class Descriptor : public AutoObjDescriptorBase
	{
	public:
		unsigned long refCount;
		T* garbageObj;
		bool deleteable;
		CriticalSection crit;

		Descriptor(unsigned long initCount)
			:	refCount(initCount), garbageObj(0), deleteable(true)
		{ }
		
		~Descriptor()	{ delete garbageObj; }
		
		bool release()
		{
			bool result = false;
			if (useCrit)
				crit.Enter();
			if (deleteable)
			{
				result = true;
				deleteable = false;
			}
			if (useCrit)
				crit.Leave();
			return result;
		}

		unsigned long count() const			{ return refCount; }

		void QueueGarbage(T* obj)
		{
			if (deleteable)
				garbageObj = obj;
			AutoObjDescriptorBase::QueueGarbage();
		}
		
		unsigned long inc()
		{
			unsigned long result;
			if (useCrit)
				crit.Enter();
			result = ++refCount;
			if (useCrit)
				crit.Leave();
			return result;
		}

		unsigned long dec()
		{
			unsigned long result;
			if (useCrit)
				crit.Enter();
			result = --refCount;
			if (useCrit)
				crit.Leave();
			return result;
		}
	};


	T* obj;
	Descriptor<isThreadSafe>* desc;
public:
	auto_obj()
		:	obj(0), desc(0)				
	{ }

	auto_obj(T* toTrack)
		:	obj(toTrack), desc(toTrack ? new Descriptor<isThreadSafe>(1) : 0)	
	{ }

	auto_obj(const auto_obj<T>& toCopy)
	{
		if (toCopy.obj)
		{
			obj = toCopy.obj;
			desc = toCopy.desc;
			desc->inc();
		}
		else
		{
			obj = 0;
			desc = 0;
		}
	}

	~auto_obj()
	{
		if (obj)
		{
			if (!desc->dec())
			{
				if (deferDealloc)
					desc->QueueGarbage(obj);
				else
				{
					if (desc->deleteable)
						delete obj;
					delete desc;
				}
			}
		}
		else // if (desc) // implicit
			delete desc;
	}

	auto_obj& operator=(T* toTrack)
	{
		if (obj)
		{
			if (!desc->dec())
			{
				if (deferDealloc)
				{
					desc->QueueGarbage(obj);
					desc = 0;
				}
				else if (desc->deleteable)
					delete obj;
				else
					desc->deleteable = true;
			}
			else
				desc = 0;
		}
		obj = toTrack;
		if (obj)
		{
			if (!desc)
				desc = new Descriptor<isThreadSafe>(1);
			else
				desc->inc();
		}
		return *this;
	}

	auto_obj& operator=(const auto_obj<T>& toCopy)
	{
		if (obj)
		{
			if (!desc->dec())
			{
				if (deferDealloc)
				{
					desc->QueueGarbage(obj);
					desc = 0;
				}
				else if (desc->deleteable)
					delete obj;
				else
					desc->deleteable = true;
			}
			else
				desc = 0;
		}
		obj = toCopy.obj;
		if (obj)
		{
//			if (desc) // implicit
				delete desc;
			desc = toCopy.desc;
			desc->inc();
		}
		return *this;
	}

	void dispose()
	{
		if (obj)
		{
			if (!desc->dec())
			{
				if (deferDealloc)
				{
					desc->QueueGarbage(obj);
					desc = 0;
				}
				else if (desc->deleteable)
					delete obj;
				else
					desc->deleteable = true;
			}
			else
				desc = 0;
			obj = 0;
		}
	}

	// Make sure no-one else deletes it
	// Returns 0 if already released elsewhere, or no obj is encapsulated
	T* release()	// returns false if already released.  Could be bad
	{
		if (obj)
		{
			if (desc->release())	// return false if already deleteable
				return obj;
		}
		return 0;
	}

	unsigned long count()
	{
		if (obj)
			return desc->count();
		return 0;
	}
};



// Warning: if you defer deallocation, and never purge garbage, your app will grow indefinitely!
void PurgeGarbage(void);


};


#endif

