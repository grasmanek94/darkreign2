#ifndef MAIN_H
#define MAIN_H

#ifdef DLLSAMPLE
#include <wondll.h>
#else
#include <Socket/IPSocket.h>
#endif // DLLSAMPLE

#ifdef MAIN_CPP
#define MAYBE_EXTERN
#else
#define MAYBE_EXTERN extern
#endif

MAYBE_EXTERN void OutputString(const char* string);
MAYBE_EXTERN void OutputString(const wchar_t* string);
MAYBE_EXTERN void OutputStringF(const char* format, ...);
MAYBE_EXTERN void OutputError(const char* string);
MAYBE_EXTERN void OutputError(const char* string, int error);
#ifdef DLLSAMPLE
MAYBE_EXTERN void OutputServerError(const char* string, const WONIPAddress& theAddrR, int error);
#else
MAYBE_EXTERN void OutputServerError(const char* string, const WONAPI::IPSocket::Address& theAddrR, int error);
#endif // DLLSAMPLE
MAYBE_EXTERN void OutputFile(const char* theFilePath);

#endif // MAIN_H
