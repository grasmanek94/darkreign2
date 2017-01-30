// RAImageDLL.cpp: implementation of the RAImageDLL class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "RAImageDLL.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace WONAPI;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RAImageDLL::RAImageDLL()
{
	pf_dll_Initialize = 0;
	pf_dll_Shutdown = 0;
	pf_dll_SetPaintCallback = 0;
	pf_dll_SetBrowserCallback = 0;
	pf_dll_AdActivated = 0;
	pf_dll_LockBuffer = 0;
	pf_dll_UnlockBuffer = 0;
	pf_dll_GetBufferBitmap = 0;
	pf_dll_GetBufferPalette = 0;
	pf_dll_GetBufferRGB = 0;
	pf_dll_StartMessaging = 0;
	pf_dll_SetOffline = 0;
	pf_dll_SetThrottle = 0;
	pf_dll_SetProxy = 0;

	hLib = LoadLibrary("raimage.dll");

	// bind the DLL's functions
	if (hLib) {
#define BIND(FUNCTION)	pf_dll_##FUNCTION = (dll_##FUNCTION)GetProcAddress(hLib, #FUNCTION)
		BIND(Initialize);
		BIND(Shutdown);
		BIND(SetPaintCallback);
		BIND(SetBrowserCallback);
		BIND(AdActivated);
		BIND(LockBuffer);
		BIND(UnlockBuffer);
		BIND(GetBufferBitmap);
		BIND(GetBufferPalette);
		BIND(GetBufferRGB);
		BIND(StartMessaging);
		BIND(SetOffline);
		BIND(SetThrottle);
		BIND(SetProxy);
#undef BIND
	}
}

RAImageDLL::~RAImageDLL()
{
	pf_dll_Initialize = 0;
	pf_dll_Shutdown = 0;
	pf_dll_SetPaintCallback = 0;
	pf_dll_SetBrowserCallback = 0;
	pf_dll_AdActivated = 0;
	pf_dll_LockBuffer = 0;
	pf_dll_UnlockBuffer = 0;
	pf_dll_GetBufferBitmap = 0;
	pf_dll_GetBufferPalette = 0;
	pf_dll_GetBufferRGB = 0;
	pf_dll_StartMessaging = 0;
	pf_dll_SetOffline = 0;
	pf_dll_SetThrottle = 0;
	pf_dll_SetProxy = 0;

	FreeLibrary(hLib);
}


HANDLE RAImageDLL::Initialize(unsigned long software_id, DWORD version, unsigned long ad_size_id, unsigned long flags)
{
	if (pf_dll_Initialize) {
		return pf_dll_Initialize(software_id, version, ad_size_id, flags);
	} else {
		return 0;
	}
}


void RAImageDLL::Shutdown(HANDLE hData)
{
	if (pf_dll_Shutdown) {
		pf_dll_Shutdown(hData);
	}
}


void RAImageDLL::SetPaintCallback(HANDLE hData, PaintCallbackFunc func, void *data)
{
	if (pf_dll_SetPaintCallback) {
		pf_dll_SetPaintCallback(hData, func, data);
	}
}


void RAImageDLL::SetBrowserCallback(HANDLE hData, BrowserCallbackFunc func)
{
	if (pf_dll_SetBrowserCallback) {
		pf_dll_SetBrowserCallback(hData, func);
	}
}


void RAImageDLL::AdActivated(HANDLE hData)
{
	if (pf_dll_AdActivated) {
		pf_dll_AdActivated(hData);
	}
}


void RAImageDLL::LockBuffer(HANDLE hData)
{
	if (pf_dll_LockBuffer) {
		pf_dll_LockBuffer(hData);
	}
}


void RAImageDLL::UnlockBuffer(HANDLE hData)
{
	if (pf_dll_UnlockBuffer) {
		pf_dll_UnlockBuffer(hData);
	}
}


HBITMAP RAImageDLL::GetBufferBitmap(HANDLE hData)
{
	if (pf_dll_GetBufferBitmap) {
		return pf_dll_GetBufferBitmap(hData);
	} else {
		return 0;
	}
}


HPALETTE RAImageDLL::GetBufferPalette(HANDLE hData)
{
	if (pf_dll_GetBufferPalette) {
		return pf_dll_GetBufferPalette(hData);
	} else {
		return 0;
	}
}


BYTE* RAImageDLL::GetBufferRGB(HANDLE hData, int& x, int& y)
{
	if (pf_dll_GetBufferRGB) {
		return pf_dll_GetBufferRGB(hData, &x, &y);
	} else {
		return 0;
	}
}


BOOL RAImageDLL::StartMessaging(unsigned long software_id)
{
	if (pf_dll_StartMessaging) {
		return pf_dll_StartMessaging(software_id);
	} else {
		return FALSE;
	}
}


BOOL RAImageDLL::SetThrottle(unsigned long throttle)
{
	if (pf_dll_SetThrottle) {
		return pf_dll_SetThrottle(throttle);
	} else {
		return FALSE;
	}
}


BOOL RAImageDLL::SetProxy(const char *address, unsigned long port, const char *username, const char *password)
{
	if (pf_dll_SetProxy) {
		return pf_dll_SetProxy(address, port, username, password);
	} else {
		return FALSE;
	}
}


BOOL RAImageDLL::SetOffline(int offline)
{
	if (pf_dll_SetOffline) {
		return pf_dll_SetOffline(offline);
	} else {
		return FALSE;
	}
}
