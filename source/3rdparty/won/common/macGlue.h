
#ifndef __WON_MACGLUE_H__
#define __WON_MACGLUE_H__

#define WON_BIG_ENDIAN

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
#include <stdlib.h>
#include <ctime>
#define wchar wchar_t

#define INFINITE			0xFFFFFFFF
#define TRUE				(1)
#define FALSE				(0)


typedef char* LPCSTR;
typedef char* LPSTR;

#define MAX_PATH (255)

#define __int8 char
#define __int16 short
#define __int32 long
#define __int64 long long

typedef struct _FILETIME {
	unsigned long dwLowDateTime;
	unsigned long dwHighDateTime;
} FILETIME;

#ifndef _WON_IN_DLL_H_C_

#include "unistd.h"	// should find in GUSI
#include "fcntl.h"	// should find in GUSI
#include <stddef.h>
#include <cstdlib>
#include <GUSISocket.h>
#include <Socket.h>
#include <Events.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <sstream>
#include <cstring>

#define _ASSERT assert

#define HOSTENT hostent
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

inline int WSAGetLastError()		{ return errno; }

inline void timeBeginPeriod(int)	{}
inline void timeEndPeriod(int)		{}

inline unsigned long timeGetTime()
{
	return ((TickCount() * (long long)50) / 3);
}

inline unsigned long GetTickCount()
{
	return ((TickCount() * (long long)50) / 3);
}

inline char* strdup(const char* src)
{
	if (src)
	{
		size_t sLen = strlen(src);
		char* result = (char*)malloc(sLen+1);
		if (result)
		{
			strcpy(result, src);
			return result;
		}
	}
	return 0;
}

inline wchar_t* wcsdup(const wchar_t* src)
{
	if (src)
	{
		size_t sLen = wcslen(src);
		wchar_t* result = (wchar_t*)malloc((sLen+1)*2);
		if (result)
		{
			wcscpy(result, src);
			return result;
		}
	}
	return 0;
}

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
