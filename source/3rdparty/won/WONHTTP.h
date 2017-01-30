#ifndef __WON_HTTP_H__
#define __WON_HTTP_H__


#include "common/won.h"
#include "Socket/WONWS.h"
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "Socket/IPSocket.h"


namespace WONAPI {


typedef bool (*ProgressCallback)(unsigned long progress, unsigned long size, void* callbackPrivData);
// return false from ProgressCallback to abort download


// Pass in getPath as "/dir/file", or "dir/file".  Do not include "http://"

// If modTime is NULL, and you're using HTTPGet to retrieve a file, the mod time
// of the file will be used to do a conditional GET.
//
// If modTime is NULL, and you're using HTTPGet to recv a file into a buffer, an
// unconditional GET will be performed.
//
// If a modTime is specified, and is 0 on entry, an unconditional GET will be performed,
// and the modTime of the recved file will be returned.
//
// If a modTime is specified, and is non-0 on entry, it will be used to perform
// a conditional GET (overriding the mod time of an existing local file), and the
// mod time of the recved file will be returned.


// w/Proxy, uses file

Error HTTPGet(const IPSocket::Address& proxyAddr,
			  const std::string& hostName, unsigned short httpPort,
			  const std::string& getPath, const std::string& saveAsFile, bool* isNew, 
			  time_t* modTime, bool allowResume, ProgressCallback callback,
			  void* callbackPrivData, long timeout,
			  bool async, const CompletionContainer<Error>& completion);


inline Error HTTPGet(const IPSocket::Address& proxyAddr,
					 const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, const std::string& saveAsFile, bool* isNew = 0, 
					 time_t* modTime = 0, bool allowResume = true, ProgressCallback callback = 0,
					 void* callbackPrivData = 0, long timeout = -1, bool async = false)
{ return HTTPGet(proxyAddr, hostName, httpPort, getPath, saveAsFile, isNew, modTime, allowResume, callback, callbackPrivData, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error HTTPGet(const IPSocket::Address& proxyAddr,
					 const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, const std::string& saveAsFile, bool* isNew, 
					 time_t* modTime, bool allowResume, ProgressCallback callback,
					 void* callbackPrivData, long timeout, bool async,
					 void (*f)(Error, privsType), privsType privs)
{ return HTTPGet(proxyAddr, hostName, httpPort, getPath, saveAsFile, isNew, modTime, allowResume, callback, callbackPrivData, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }



// w/Proxy, uses buffer

Error HTTPGet(const IPSocket::Address& proxyAddr,
			  const std::string& hostName, unsigned short httpPort,
			  const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
			  bool* isNew, time_t* modTime, ProgressCallback callback,
			  void* callbackPrivData, long timeout,
			  bool async, const CompletionContainer<Error>& completion);


inline Error HTTPGet(const IPSocket::Address& proxyAddr,
					 const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					 bool* isNew = 0, time_t* modTime = 0, ProgressCallback callback = 0,
					 void* callbackPrivData = 0, long timeout = -1, bool async = false)
{ return HTTPGet(proxyAddr, hostName, httpPort, getPath, recvBuf, recvBufSize, isNew, modTime, callback, callbackPrivData, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error HTTPGet(const IPSocket::Address& proxyAddr,
					 const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					 bool* isNew, time_t* modTime, ProgressCallback callback,
					 void* callbackPrivData, long timeout, bool async,
					 void (*f)(Error, privsType), privsType privs)
{ return HTTPGet(proxyAddr, hostName, httpPort, getPath, recvBuf, recvBufSize, isNew, modTime, callback, callbackPrivData, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


// w/o Proxy, uses file

inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, const std::string& saveAsFile,
					 bool* isNew, time_t* modTime, bool allowResume,
					 ProgressCallback callback, void* callbackPrivData, long timeout,
					 bool async, const CompletionContainer<Error>& completion)
{ return HTTPGet(IPSocket::Address(hostName, httpPort), hostName, httpPort, getPath, saveAsFile, isNew, modTime, allowResume, callback, callbackPrivData, timeout, async, completion); }


inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, const std::string& saveAsFile,
					 bool* isNew = 0, time_t* modTime = 0, bool allowResume = true,
					 ProgressCallback callback = 0, void* callbackPrivData = 0,
					 long timeout = -1, bool async = false)
{ return HTTPGet(IPSocket::Address(hostName, httpPort), hostName, httpPort, getPath, saveAsFile, isNew, modTime, allowResume, callback, callbackPrivData, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, const std::string& saveAsFile, bool* isNew, 
					 time_t* modTime, bool allowResume, ProgressCallback callback,
					 void* callbackPrivData, long timeout, bool async,
					 void (*f)(Error, privsType), privsType privs)
{ return HTTPGet(hostName, httpPort, getPath, saveAsFile, isNew, modTime, allowResume, callback, callbackPrivData, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


// w/o Proxy, uses buffer

inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					 bool* isNew, time_t* modTime, ProgressCallback callback, void* callbackPrivData,
					 long timeout, bool async, const CompletionContainer<Error>& completion)
{ return HTTPGet(IPSocket::Address(hostName, httpPort), hostName, httpPort, getPath, recvBuf, recvBufSize, isNew, modTime, callback, callbackPrivData, timeout, async, completion); }


inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					 bool* isNew = 0, time_t* modTime = 0, ProgressCallback callback = 0,
					 void* callbackPrivData = 0, long timeout = -1, bool async = false)
{ return HTTPGet(IPSocket::Address(hostName, httpPort), hostName, httpPort, getPath, recvBuf, recvBufSize, isNew, modTime, callback, callbackPrivData, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error HTTPGet(const std::string& hostName, unsigned short httpPort,
					 const std::string& getPath, void* recvBuf, unsigned long* recvBufSize,
					 bool* isNew, time_t* modTime, ProgressCallback callback,
					 void* callbackPrivData, long timeout, bool async,
					 void (*f)(Error, privsType), privsType privs)
{ return HTTPGet(hostName, httpPort, getPath, recvBuf, recvBufSize, isNew, modTime, callback, callbackPrivData, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }



};


#endif
