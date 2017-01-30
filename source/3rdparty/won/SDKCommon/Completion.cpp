#include "Completion.h"
#include "wondll.h"


class C_CompletionBase
{
public:
	virtual ~C_CompletionBase();

	virtual void Complete(void* param) = 0;
};


C_CompletionBase::~C_CompletionBase()
{
}


class C_CallbackCompletion : public C_CompletionBase
{
private:
	WONCompletionCallbackType callback;

public:
	C_CallbackCompletion(WONCompletionCallbackType cb)
		:	callback(cb)
	{ }

	virtual void Complete(void* param);
};


class C_CallbackWithDataCompletion : public C_CompletionBase
{
private:
	WONCompletionCallbackWithDataType callback;
	void* priv;

public:
	C_CallbackWithDataCompletion(WONCompletionCallbackWithDataType cb, void* privData)
		:	callback(cb), priv(privData)
	{ }

	virtual void Complete(void* param);
};


#ifdef WIN32

class C_EventCompletion : public C_CompletionBase
{
private:
	HANDLE event;

public:
	C_EventCompletion(HANDLE evt)
		:	event(evt)
	{ }

	virtual void Complete(void* param);
};


class C_SendMsgCompletion : public C_CompletionBase
{
private:
	HWND hWindow;
	UINT message;
	WPARAM wParam;

public:
	C_SendMsgCompletion(HWND hWnd, UINT msg, WPARAM wPrm)
		:	hWindow(hWnd), message(msg), wParam(wPrm)
	{ }

	virtual void Complete(void* param);
};


class C_PostMsgCompletion : public C_CompletionBase
{
private:
	HWND hWindow;
	UINT message;
	WPARAM wParam;

public:
	C_PostMsgCompletion(HWND hWnd, UINT msg, WPARAM wPrm)
		:	hWindow(hWnd), message(msg), wParam(wPrm)
	{ }

public:
	virtual void Complete(void* param);
};

#endif


void C_CallbackCompletion::Complete(void* param)
{
	callback((HWONCOMPLETION)this, param);
}


void C_CallbackWithDataCompletion::Complete(void* param)
{
	callback((HWONCOMPLETION)this, param, priv);
}


#ifdef WIN32

void C_EventCompletion::Complete(void* param)
{
	SetEvent(event);
}


void C_SendMsgCompletion::Complete(void* param)
{
	SendMessage(hWindow, message, wParam, (long)param);
}


void C_PostMsgCompletion::Complete(void* param)
{
	PostMessage(hWindow, message, wParam, (long)param);
}

#endif

HWONCOMPLETION WONCreateCallbackCompletion(WONCompletionCallbackType callback)
{
	return (HWONCOMPLETION)new C_CallbackCompletion(callback);
}

HWONCOMPLETION WONCreateCallbackWithDataCompletion(WONCompletionCallbackWithDataType callback, void* privData)
{
	return (HWONCOMPLETION)new C_CallbackWithDataCompletion(callback, privData);
}

#ifdef WIN32

HWONCOMPLETION WONCreateEventCompletion(HANDLE event)
{
	return (HWONCOMPLETION)new C_EventCompletion(event);
}


HWONCOMPLETION WONCreateWindowsPostMessageCompletion(HWND hWindow, UINT msg, WPARAM wParam)
{
	return (HWONCOMPLETION)new C_PostMsgCompletion(hWindow, msg, wParam);
}


HWONCOMPLETION WONCreateWindowsSendMessageCompletion(HWND hWindow, UINT msg, WPARAM wParam)
{
	return (HWONCOMPLETION)new C_SendMsgCompletion(hWindow, msg, wParam);
}
#endif


void WONCompletionCloseHandle(HWONCOMPLETION hCompletion)
{
	delete (C_CompletionBase*)hCompletion;
}

// Shouldn't need to call manually.  Async operations call this.
void WONComplete(HWONCOMPLETION hCompletion, void* param)
{
	C_CompletionBase* completion = (C_CompletionBase*)hCompletion;
	if (completion)
		completion->Complete(param);
}


using namespace WONAPI;


//const WONAPI::CompletionContainer<WONAPI::Error>	WONAPI::blankErrorCompletion;


CompletionBase::~CompletionBase()
{
}


CompletionContainerBase::~CompletionContainerBase()
{
}
