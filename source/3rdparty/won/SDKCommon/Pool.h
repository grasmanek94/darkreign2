
#ifndef __WON_POOL_H__
#define __WON_POOL_H__


namespace WONAPI {


template <class T>
class defaultPoolElemAllocator
{
	T* allocate()	{ return new T; }
};


// Warning!  pool<> is not inherently thread safe!
// You might need to use a critical section to protect access to it
template <class T, class allocator = defaultPoolElemAllocator<T> >
class pool
{
private:
	std::queue<T*> elems;
	unsigned int cap;
	unsigned int growBy;

public:
	pool(unsigned int initialElems = 0, unsigned int elemsCap = 0, unsigned int ElemsToGrowBy = 0)
		:	cap(elemsCap)
	{
		growBy = (elemsCap && ElemsToGrowBy > elemsCap) ? elemsCap : ElemsToGrowBy;
		
		unsigned int i = initialElems;
		while (i)
		{
			T* t = allocator().allocate();
			if (!t)
				break;
			elems.push(t);
			i--;
		}
	}

	~pool()
	{
		while (!elems.empty())
		{
			delete elems.front();
			elems.pop();
		}
	}

	T* get()
	{
		T* t;
		if (elems.empty())
		{
			t = allocator().allocate();
			if (t)
			{
				unsigned int i = growBy;
				while (i)
				{
					T* t2 = allocator().allocate();
					if (!t2)
						break;
					elems.push(t2);
					i--;
				}
			}
		}
		else
		{
			t = elems.front();
			elems.pop();
		}
		return t;
	}

	void release(T* t)
	{
		if (!cap || elems.size() != cap)
			elems.push(t);
		else
			delete t;
	}
};


};


#endif
