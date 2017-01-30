
#ifndef __WON_COMPLETION_H__
#define __WON_COMPLETION_H__


#include "common/Event.h"

#pragma warning(disable : 4786)


typedef struct unnamed___init_hack { int unused; } _init_hack;


namespace WONAPI {


/*
	This completion mechanism wrappers the behavior invoked when a specific
	operation completes.

	This is particularly useful for receiving notifcation when an operation is
	lengthy, and has been deferred.

	Completion<> is a template.  The template type is the resulting type of the
	operation. The resulting data type is passed as an argument to a virtual
	Complete() member function.  If the template type is void, there is no
	resulting data type, and Complete() receives no arguments.

	Member data can be added to a derived Completion<> object, and would be
	accessibly in a derived Complete() function.  The default Complete()
	function will deallocate the completion object automatically, if autoDelete
	is set to true.

	A CompletionWithContents<> object is like a Completion<> object, but
	piggy-backs additional data, of a second template type, and passes it as a
	second argument to Complete()  (or as the only argument, if the result data
	type is void).  A CompletionWithContents<> can also encapsulates a function
	pointer, which the default implementation of Complete() will execute,
	passing both the result data and piggy-backed data.  Some operations that
	support Completion<>'s internally use a CompletionWithContents<> to also
	provide a function overload that accept both a callback function pointer
	and data to piggy-back.

	A WindowProcSendMessageCompletion<> can be used to generate a WindowProc
	message, sent with the Windows API SendMessage() routine.  Beware,
	completions may be triggered in a separate thread, and may result in your
	WindowProc being called from multiple threads simultanious, or re-entrantly.
	If there is result data, a pointer to it is passed in the lParam.

	A WindowProcPostMessageCompletion<> is similar, but uses the Windows API
	PostMessage() routine to deliver the message.  If there is resulting data,
	a new copy of the data is allocated (with operator new), and passed as the
	lParam.  This is necessary because PostMessage() may defer the message
	until later, and the data must remain valid until the WindowProc message is
	processed.  So, it's the responsibility of the WindowProc function to
	delete the data in the lParam.

	Operations that support this completion machanism usually accept
	CompletionContainer<>'s by reference, then copy them by value.  The
	constructors for CompletionContainer<>s accept Completion<> objects,
	callback functions, or Windows event objects.  Because the constructors are
	non-explicit, you can pass any of these in place of a CompletionContainer<>
	argument.
*/


class CompletionBase
{
private:
	bool autoDelete;

public:
	explicit CompletionBase(bool autoDel = false)
		:	autoDelete(autoDel)				{ }

	virtual ~CompletionBase();

	void clear()							{ if (autoDelete) delete this; }

	bool& GetAutoDelete()					{ return autoDelete; }
	void SetAutoDelete(bool autoDel = true)	{ autoDelete = autoDel; }
};


template <class T>
class Completion : public CompletionBase
{
public:
	explicit Completion(bool autoDel = false)
		:	CompletionBase(autoDel)			{ }
	
	virtual void Complete(T result)			{ clear(); }
};


class Completion<void> : public CompletionBase
{
public:
	explicit Completion(bool autoDel = false)
		:	CompletionBase(autoDel)			{ }
	
	virtual void Complete(void)				{ clear(); }
};


#ifdef WIN32

template <class T>
class WindowProcSendMessageCompletion : public Completion<T>
{
private:
	HWND hWnd;
	UINT msg;
	WPARAM wParameter;
public:
	explicit WindowProcSendMessageCompletion(HWND window, UINT message, WPARAM wParam = 0, bool autoDel = false)
		:	Completion<T>(autoDel),
			hWnd(window),
			msg(message),
			wParameter(wParam)
	{ }
	
	virtual void Complete(T result)
	{
		SendMessage(hWnd, msg, wParameter, (long)(&result));
		clear();
	}
};


class WindowProcSendMessageCompletion<void> : public Completion<void>
{
private:
	HWND hWnd;
	UINT msg;
	WPARAM wParameter;
	LPARAM lParameter;
public:
	explicit WindowProcSendMessageCompletion(HWND window, UINT message, WPARAM wParam = 0, LPARAM lParam = 0, bool autoDel = false)
		:	Completion<void>(autoDel),
			hWnd(window),
			msg(message),
			wParameter(wParam),
			lParameter(lParam)
	{ }
		
	virtual void Complete(void)
	{
		SendMessage(hWnd, msg, wParameter, lParameter);
		clear();
	}
};


/*
	Important:  It's the responsibility of the WindowProc function to
	delete the data in the lParam.  (see above)
*/
template <class T>
class WindowProcPostMessageCompletion : public Completion<T>
{
private:
	HWND hWnd;
	UINT msg;
	WPARAM wParameter;

	template <class T2>
	T2* hack(const T2* arg)
	{
		return new T2(*arg);
	}
public:
	explicit WindowProcPostMessageCompletion(HWND window, UINT message, WPARAM wParam = 0, bool autoDel = false)
		:	Completion<T>(autoDel),
			hWnd(window),
			msg(message),
			wParameter(wParam)
	{ }
	
	virtual void Complete(T result)
	{
		PostMessage(hWnd, msg, wParameter, (long)(hack(&result)));
		clear();
	}
};


class WindowProcPostMessageCompletion<void> : public Completion<void>
{
private:
	HWND hWnd;
	UINT msg;
	WPARAM wParameter;
	LPARAM lParameter;
public:
	explicit WindowProcPostMessageCompletion(HWND window, UINT message, WPARAM wParam = 0, LPARAM lParam = 0, bool autoDel = false)
		:	Completion<void>(autoDel),
			hWnd(window),
			msg(message),
			wParameter(wParam),
			lParameter(lParam)
	{ }
		
	virtual void Complete(void)
	{
		PostMessage(hWnd, msg, wParameter, lParameter);
		clear();
	}
};
#endif


class CompletionContainerBase
{
public:
	virtual ~CompletionContainerBase();

	virtual bool empty() const	= 0;
};


template <class T>
class CompletionContainer : public CompletionContainerBase
{
private:
	void (*f)(T result);
	Completion<T>* completion;
	bool ownsCompletion;
	bool useEvent;
	mutable WONCommon::Event doneEvent;

public:
	CompletionContainer()
		:	f(NULL), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(_init_hack*)
		:	f(NULL), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(void (*func)(T result))
		:	f(func), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(Completion<T>* cmplt)
		:	f(NULL), completion(cmplt), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(Completion<T>* cmplt, bool ownCompletion)
		:	f(NULL), completion(cmplt), ownsCompletion(ownCompletion), useEvent(false)
	{ }

#ifdef WIN32
	CompletionContainer(HANDLE evt)
		:	f(NULL), completion(NULL), ownsCompletion(false), doneEvent(evt), useEvent(evt != NULL)
	{ }
#endif

	CompletionContainer(WONCommon::Event& evt)
		:	f(NULL), completion(NULL), ownsCompletion(false), doneEvent(evt), useEvent(true)
	{ }

	CompletionContainer(const CompletionContainer<T>& toCopy)
		:	f(toCopy.f), completion(toCopy.completion), ownsCompletion(false), doneEvent(toCopy.doneEvent), useEvent(toCopy.useEvent)
	{ }


	~CompletionContainer()
	{
		if (ownsCompletion)
			delete completion;
	}

	CompletionContainer<T>& operator=(const CompletionContainer<T>& toCopy)
	{
		if (ownsCompletion || (completion && completion->GetAutoDelete()))
		{
			ownsCompletion = false;
			delete completion;
		}
		completion = toCopy.completion;
		f = toCopy.f;
		useEvent = toCopy.useEvent;
		doneEvent = toCopy.doneEvent;
		return *this;
	}

	void Complete(T result) const
	{
		try
		{
			if (useEvent)
				doneEvent.Set();
			if (f)
				f(result);
			if (completion)
				completion->Complete(result);
		}
		catch (...)
		{
		}
	}

	void OwnCompletion(bool own = true)
	{
		if (own)
		{
			if (completion && completion->GetAutoDelete())
			{
				completion->SetAutoDelete(false);
				ownsCompletion = true;
			}
		}
		else if (ownsCompletion)
		{
			if (completion)
				completion->SetAutoDelete(true);
			ownsCompletion = false;
		}
	}
	bool OwnsCompletion() const	{ return ownsCompletion; }

	bool empty() const			{ return (!useEvent && !f && !completion); }
	void clear()
	{
		if (ownsCompletion || (completion && completion->GetAutoDelete()))
		{
			ownsCompletion = false;
			delete completion;
			completion = NULL;
		}
	}
};


class CompletionContainer<void> : public CompletionContainerBase
{
private:
	void (*f)(void);
	Completion<void>* completion;
	bool ownsCompletion;
	bool useEvent;
	mutable WONCommon::Event doneEvent;

public:
	CompletionContainer()
		:	f(NULL), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(_init_hack*)
		:	f(NULL), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(void (*func)(void))
		:	f(func), completion(NULL), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(Completion<void>* cmplt)
		:	f(NULL), completion(cmplt), ownsCompletion(false), useEvent(false)
	{ }

	CompletionContainer(Completion<void>* cmplt, bool ownCompletion)
		:	f(NULL), completion(cmplt), ownsCompletion(ownCompletion), useEvent(false)
	{ }

#ifdef _MTAPI
#ifdef WIN32
	CompletionContainer(HANDLE evt)
		:	f(NULL), completion(NULL), ownsCompletion(false), doneEvent(evt), useEvent(evt != NULL)
	{ }
#endif
#endif

	CompletionContainer(WONCommon::Event& evt)
		:	f(NULL), completion(NULL), ownsCompletion(false), doneEvent(evt), useEvent(true)
	{ }

	CompletionContainer(const CompletionContainer<void>& toCopy)
		:	f(toCopy.f), completion(toCopy.completion), ownsCompletion(false), doneEvent(toCopy.doneEvent), useEvent(toCopy.useEvent)
	{ }

	~CompletionContainer()
	{
		if (ownsCompletion)
			delete completion;
	}

	CompletionContainer<void>& operator=(const CompletionContainer<void>& toCopy)
	{
		if (ownsCompletion || (completion && completion->GetAutoDelete()))
		{
			ownsCompletion = false;
			delete completion;
		}
		completion = toCopy.completion;
		f = toCopy.f;
		useEvent = toCopy.useEvent;
		doneEvent = toCopy.doneEvent;
		return *this;
	}

	void Complete() const
	{
		try
		{
			if (useEvent)
				doneEvent.Set();
			if (f)
				f();
			if (completion)
				completion->Complete();
		}
		catch (...)
		{
		}
	}

	void OwnCompletion(bool own = true)
	{
		if (own)
		{
			if (completion && completion->GetAutoDelete())
			{
				completion->SetAutoDelete(false);
				ownsCompletion = true;
			}
		}
		else if (ownsCompletion)
		{
			completion->SetAutoDelete(true);
			ownsCompletion = false;
		}
	}
	bool OwnsCompletion() const	{ return ownsCompletion; }

	bool empty() const			{ return (!useEvent && !f && !completion); }
	void clear()
	{
		if (ownsCompletion || (completion && completion->GetAutoDelete()))
		{
			ownsCompletion = false;
			delete completion;
			completion = NULL;
		}
	}
};



template <class T, class privsType>
class CompletionWithContents : public Completion<T>
{
private:
	privsType privs;
	void (*f)(T result, privsType prvs);

public:

	explicit CompletionWithContents(privsType prvs, bool autoDel = false)
		:	Completion<T>(autoDel),
			f(NULL),
			privs(prvs)
	{ }

	CompletionWithContents(privsType prvs, void (*func)(T result, privsType prvs), bool autoDel = false)
		:	Completion<T>(autoDel),
			f(func),
			privs(prvs)
	{ }

	CompletionWithContents(void (*func)(T result, privsType prvs), privsType prvs, bool autoDel = false)
		:	Completion<T>(autoDel),
			f(func),
			privs(prvs)
	{ }
	
	virtual void Complete(T result)
	{
		Complete(result, privs);
		Completion<T>::Complete(result);
	}

	virtual void Complete(T result, privsType prvs)
	{
		if (f)
			f(result, prvs);
	}

};


template <class privsType>
class CompletionVoidWithContents  : public Completion<void>
{
private:
	privsType privs;
	void (*f)(privsType prvs);

public:

	explicit CompletionVoidWithContents(privsType prvs, bool autoDel = false)
		:	Completion<void>(autoDel),
			privs(prvs)
	{ }

	CompletionVoidWithContents(privsType prvs, void (*func)(privsType prvs), bool autoDel = false)
		:	Completion<void>(0),
			f(func),
			privs(prvs)
	{ }

	CompletionVoidWithContents(void (*func)(privsType prvs), privsType prvs, bool autoDel = false)
		:	Completion<void>(0, autoDel, hEvent),
			f(func),
			privs(prvs)
	{ }
	
	virtual void Complete()
	{
		Complete(privs);
		Completion<void>::Complete();
	}

	virtual void Complete(privsType prvs)
	{
		if (f)
			f(prvs);
	}
};


#define DEFAULT_COMPLETION				((_init_hack*)0)


};





#endif
