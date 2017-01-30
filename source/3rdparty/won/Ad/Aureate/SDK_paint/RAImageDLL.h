// RAImageDLL.h: interface for the RAImageDLL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RAIMAGEDLL_H__DE839B84_AF3F_11D3_A79B_006094B9938E__INCLUDED_)
#define AFX_RAIMAGEDLL_H__DE839B84_AF3F_11D3_A79B_006094B9938E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

#define FLAG_OFFLINE_ONLY		  1							// use offline ads only
#define FLAG_RGB_BUFFER			  0x8000000					// make an RGB buffer instead of an HBITMAP/HPALETTE buffer

class RAImageDLL  
{
private:
	// handle to the DLL
	HMODULE hLib;

	// callback function typedefs
	typedef void (__stdcall *PaintCallbackFunc)(void *data);
	typedef void (__stdcall *BrowserCallbackFunc)(const char *file);

	// typedefs for imported DLL functions
	typedef HANDLE (*dll_Initialize)(unsigned long software_id, DWORD version, unsigned long ad_size_id, unsigned long flags);
	typedef void (*dll_Shutdown)(HANDLE hData);
	typedef void (*dll_SetPaintCallback)(HANDLE hData, PaintCallbackFunc func, void *data);
	typedef void (*dll_SetBrowserCallback)(HANDLE hData, BrowserCallbackFunc func);
	typedef void (*dll_AdActivated)(HANDLE hData);
	typedef void (*dll_LockBuffer)(HANDLE hData);
	typedef void (*dll_UnlockBuffer)(HANDLE hData);
	typedef HBITMAP (*dll_GetBufferBitmap)(HANDLE hData);
	typedef HPALETTE (*dll_GetBufferPalette)(HANDLE hData);
	typedef BYTE* (*dll_GetBufferRGB)(HANDLE hData, int *x, int *y);
	typedef BOOL (*dll_StartMessaging)(unsigned long software_id);
	typedef BOOL (*dll_SetThrottle)(unsigned long throttle);
	typedef BOOL (*dll_SetOffline)(int setting);
	typedef BOOL (*dll_SetProxy)(const char *address, unsigned long port, const char *username, const char *password);

	// function pointers for imported DLL functions
	dll_Initialize			pf_dll_Initialize;
	dll_Shutdown			pf_dll_Shutdown;
	dll_SetPaintCallback	pf_dll_SetPaintCallback;
	dll_SetBrowserCallback	pf_dll_SetBrowserCallback;
	dll_AdActivated			pf_dll_AdActivated;
	dll_LockBuffer			pf_dll_LockBuffer;
	dll_UnlockBuffer		pf_dll_UnlockBuffer;
	dll_GetBufferBitmap		pf_dll_GetBufferBitmap;
	dll_GetBufferPalette	pf_dll_GetBufferPalette;
	dll_GetBufferRGB		pf_dll_GetBufferRGB;
	dll_StartMessaging		pf_dll_StartMessaging;
	dll_SetThrottle			pf_dll_SetThrottle;
	dll_SetProxy			pf_dll_SetProxy;
	dll_SetOffline			pf_dll_SetOffline;
public:
	HANDLE Initialize(unsigned long software_id, DWORD version, unsigned long ad_size_id, unsigned long flags);
	void Shutdown(HANDLE hData);
	void SetPaintCallback(HANDLE hData, PaintCallbackFunc func, void *data);
	void SetBrowserCallback(HANDLE hData, BrowserCallbackFunc func);
	void AdActivated(HANDLE hData);
	void LockBuffer(HANDLE hData);
	void UnlockBuffer(HANDLE hData);
	HBITMAP GetBufferBitmap(HANDLE hData);
	HPALETTE GetBufferPalette(HANDLE hData);
	BYTE* GetBufferRGB(HANDLE hData, int& x, int& y);
	BOOL StartMessaging(unsigned long software_id);
	BOOL SetThrottle(unsigned long throttle);
	BOOL SetOffline(int offline);
	BOOL SetProxy(const char *address, unsigned long port, const char *username, const char *password);
public:
	RAImageDLL();
	virtual ~RAImageDLL();

};

#endif // !defined(AFX_RAIMAGEDLL_H__DE839B84_AF3F_11D3_A79B_006094B9938E__INCLUDED_)
