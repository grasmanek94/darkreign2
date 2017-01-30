#ifndef __WON_LINUXGLUE_H__
#define __WON_LINUXGLUE_H__


#ifdef _LINUX

#define __cdecl

typedef bool BOOL;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
typedef unsigned short wchar;

// _MT (and/or _MTAPI) must be defined at compile time

#define _MT					// pthreads
#ifdef _MT
#define _MTAPI
#endif

#ifdef _MTAPI
#define	CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(mutexPtr)	{ pthread_mutex_t tmpMutex = {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, {0, 0}}; *mutexPtr = tmpMutex; }
#define DeleteCriticalSection(mutexPtr)		pthread_mutex_destroy(mutexPtr)
#define EnterCriticalSection(mutexPtr)		pthread_mutex_lock(mutexPtr)
#define LeaveCriticalSection(mutexPtr)		pthread_mutex_unlock(mutexPtr)
#define TryEnterCriticalSection(mutexPtr)	pthread_mutex_trylock(mutexPtr)
#else
#define	CRITICAL_SECTION					int
#define InitializeCriticalSection(mutexPtr)	{}
#define DeleteCriticalSection(mutexPtr)		{}
#define EnterCriticalSection(mutexPtr)		{}
#define LeaveCriticalSection(mutexPtr)		{}
#define TryEnterCriticalSection(mutexPtr)	{}
#endif

#define INFINITE			0xFFFFFFFF
#define TRUE				(1)
#define FALSE				(0)

typedef char* LPCSTR;
typedef char* LPSTR;

#define __int8 char
#define __int16 short
#define __int32 long
#define __int64 long long

#include <pthread.h>

#ifndef _WON_IN_DLL_H_C_

#include <memory>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ostream.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <strstream.h>
#include <endian.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/limits.h>
#include <unistd.h>
#include <netipx/ipx.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <assert.h>
#include <wctype.h>
#include <stdlib.h>
#include <stdio.h>

#define _ASSERT assert
#define HOSTENT hostent
#define _MAX_PATH PATH_MAX
#define MAX_PATH PATH_MAX

#define NSPROTO_IPX		0		// Need to fix to support IPX under linux
#define NSPROTO_SPX		0		// Need to fix to support SPX under linux

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
#endif


#define SOCKET				int
#define INVALID_SOCKET		(-1)
#define SOCKET_ERROR		(-1)
#define closesocket			close
#define ioctlsocket			ioctl
#define SD_RECEIVE			(0)
#define SD_SEND				(1)
#define WSAESHUTDOWN		ESHUTDOWN
#define WSAEINPROGRESS		EINPROGRESS
#define WSAENOTCONN			ENOTCONN
#define WSAEWOULDBLOCK		EWOULDBLOCK
#define WSAEMSGSIZE			EMSGSIZE
#define WSAENOBUFS			ENOBUFS
#define WSAEAFNOSUPPORT		EAFNOSUPPORT
#define WSAEFAULT			EFAULT
#define WSAEINVAL			EINVAL
#define WSAEINTR			EINTR
#define WSAEADDRNOTAVAIL	EADDRNOTAVAIL
#define WSAENETUNREACH		ENETUNREACH
#define WSAECONNREFUSED		ECONNREFUSED
#define WSAEADDRINUSE		EADDRINUSE
#define WSAEMFILE			EMFILE		
#define WSAEOPNOTSUPP		EOPNOTSUPP
#define WSAEHOSTUNREACH		EHOSTUNREACH
#define WSAEDESTADDRREQ		EDESTADDRREQ
#define WSAENOTSOCK			ENOTSOCK

typedef sockaddr SOCKADDR;

#include <string>
namespace std {
typedef basic_string<wchar_t> wstring;
};
/*
inline wchar* wcscpy(wchar* dst, const wchar* src)
{
	const wchar* curSrc = src;
	wchar* curDst = dst;
	for(;;)
	{
		wchar s = *curSrc;
		*curDst = s;
		if (!s)
			break;
		curSrc++; curDst++;
	}
}


inline wchar* wcsncpy(wchar* dst, const wchar* src, size_t sz)
{
	if (sz)
	{
		const wchar* curSrc = src;
		wchar* curDst = dst;
		size_t count = sz;
		do
		{
			wchar s = *curSrc;
			*curDst = s;
			if (!s)
				break;
			curSrc++; curDst++;
		} while (--count);
	}
}


inline int wcslen(const wchar* s)
{
	const wchar* pos = s;
	while (*pos)
		pos++;
	return (int)(pos - s);
}
*/
inline unsigned long timeGetTime()
{
	timeb tp;
	ftime(&tp);
	return ((tp.time * 1000) + tp.millitm);
}

inline unsigned long GetTickCount()
{
	timeb tp;
	ftime(&tp);
	return ((tp.time * 1000) + tp.millitm);
}

inline void timeBeginPeriod(int)	{}
inline void timeEndPeriod(int)		{}

inline int WSAGetLastError()		{ return errno; }

typedef struct _FILETIME { // ft 
    DWORD dwLowDateTime; 
    DWORD dwHighDateTime; 
} FILETIME; 
 

inline int strnicmp(const char *s1, const char *s2, int n)
{
    int i;
    char c1, c2;
    for (i=0; i<n; i++)
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
        if (!c1) return 0;
    }
    return 0;
}

#endif
#endif
#endif
