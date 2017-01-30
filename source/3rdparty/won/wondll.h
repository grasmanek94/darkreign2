#ifndef _WON_DLL_C_
#define _WON_DLL_C_

#define _WON_IN_DLL_H_C_

#ifdef WIN32
#include <windows.h>
#include <wchar.h>
typedef wchar_t wchar;
#elif defined(_LINUX)
#include "common/linuxGlue.h"
#elif defined(macintosh) && (macintosh == 1)
#include "common/macGlue.h"
#else
#error unknown platform
#endif
#include <time.h>

#undef _WON_IN_DLL_H_C_


// const is not supported in C, so WON_CONST is just a placeholder, so you know the args are const
// change to: #define WON_CONST const
// If you want to enable const.  Be sure to recompile the API library.
#define WON_CONST const

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------------------------------
// Common types

typedef long WONError;

//----------------------------------------------------------------------------------------------------
// Error codes

// Unlisted errors might be server return codes, documented in server docs
#define WONError_Success					(0L)
#define WONError_OutOfMemory				(100001L)
#define WONError_GeneralFailure				(100002L)
#define WONError_InvalidState				(100003L)
#define WONError_NetFailed					(100004L)
#define WONError_NetResourceUnavailable		(100005L)
#define WONError_BadAddress					(100006L)
#define WONError_AddressInUse				(100007L)
#define WONError_HostUnreachable			(100008L)
#define WONError_Timeout					(100009L)
#define WONError_Pending					(100010L)
#define WONError_ConnectionClosed			(100011L)
#define WONError_InvalidMessage				(100012L)
#define WONError_Winsock2Required			(100013L)
#define WONError_InvalidParams				(100014L)
#define WONError_Unimplemented				(100015L)
#define WONError_Aborted					(100016L)
#define WONError_UnableToOpenFile			(100017L)
#define WONError_UnableToWriteToFile		(100018L)
#define WONError_InvalidVerifierKey			(110001L)
#define WONError_GetPubKeysFailed			(110002L)
#define WONError_InvalidPubKeys				(110003L)
#define WONError_FailedToGenerateKey		(110004L)
#define WONError_InvalidAuthLoginReply		(110005L)
#define WONError_InvalidAuthLogin			(110006L)
#define WONError_BadNewPassword				(110007L)
#define WONError_InvalidCDKey				(110008L)
#define WONError_ExpiredKey					(-1500L)
#define WONError_VerifyFailed				(-1501L)
#define WONError_LockedOut					(-1502L)
#define WONError_KeyInUse					(-1503L)
#define WONError_CRCFailed					(-1504L)
#define WONError_UserExists					(-1505L)
#define WONError_UserNotFound				(-1506L)
#define WONError_BadPassword				(-1507L)
#define WONError_DirtyWord					(-1508L)
#define WONError_InvalidSessionKey 			(120001)
#define WONError_InvalidCertificate			(120002)
#define WONError_InvalidRemoteCertificate 	(120003)
#define WONError_InvalidPrivateKey			(120004)
#define WONError_EncryptFailed				(120005)
#define WONError_InvalidSecret				(120006)
#define WONError_Failure					(-1L)
#define WONError_InvalidParameters			(-4L)
#define WONError_NotSupported				(-12L)

WON_CONST char* WONErrorToString(long errorVal);


//----------------------------------------------------------------------------------------------------
// Initialization and termination routines
// 
// Using the WONInitialize and WONTerminate functions explicitly may alleviate problems
// at shutdown.  If you don't call them, they will be called implicitly when the API is
// first used, and at static destruction.
//
void WONInitialize();
BOOL WONTerminate();


#ifndef _MTAPI

//----------------------------------------------------------------------------------------------------
// Single threaded pump function
//
// Makes sure asynchronous routines complete, and trigger completion objects

// timeout is in milliseconds
void WONPump(unsigned long timeout);

#ifdef WIN32
// Can use Windows constant INFINITE as timeout
int WONPumpUntil(HANDLE obj, DWORD timeout);	// return non-zero if obj is signaled
#endif


#endif



//----------------------------------------------------------------------------------------------------
// asynchronous completion notification

typedef struct unnamed_HWONCOMPLETION { int unused; }* HWONCOMPLETION;


typedef void (*WONCompletionCallbackType)(HWONCOMPLETION hCompletion, void* param);
typedef void (*WONCompletionCallbackWithDataType)(HWONCOMPLETION hCompletion, void* param, void* privData);


HWONCOMPLETION WONCreateCallbackCompletion(WONCompletionCallbackType callback);
HWONCOMPLETION WONCreateCallbackWithDataCompletion(WONCompletionCallbackWithDataType callback, void* privData);

#ifdef WIN32
HWONCOMPLETION WONCreateEventCompletion(HANDLE event);

// lParam is defined by the async operation
HWONCOMPLETION WONCreateWindowsSendMessageCompletion(HWND hWindow, UINT msg, WPARAM wParam);
// WARNING: PostMessage completions might not funtion properly if used with an API which 
//          returns a pointer to a block of memory in the lParam which might get deallocated
//          before the message is processed!
HWONCOMPLETION WONCreateWindowsPostMessageCompletion(HWND hWindow, UINT msg, WPARAM wParam);
#endif

void WONCompletionCloseHandle(HWONCOMPLETION hCompletion);

// Shouldn't need to call manually.  Async operations call this.
void WONComplete(HWONCOMPLETION hCompletion, void* param);



//----------------------------------------------------------------------------------------------------
// Address structures

typedef struct {
	long			ipAddress;	// an in_addr
	unsigned short	portNum;
} WONIPAddress;

void WONIPAddressSet(WONIPAddress* addr, unsigned long ipaddr, unsigned short port);
void WONIPAddressSetPort(WONIPAddress* addr, unsigned short port);
void WONIPAddressSetFromString(WONIPAddress* addr, WON_CONST char* hostNameAndPort);
void WONIPAddressSetFromStringAndPort(WONIPAddress* addr, WON_CONST char* hostName, unsigned short portNum);
void WONIPAddressSetFromSockaddr(WONIPAddress* addr, WON_CONST struct sockaddr_in* sockAddr);
void WONIPAddressSetFromInaddr(WONIPAddress* addr, WON_CONST struct in_addr ipaddr, unsigned short port);

void WONIPAddressGetHostName(WON_CONST WONIPAddress* addr, char* strBuf, unsigned short* strBufLength, BOOL includePort);
void WONIPAddressGetDottedIP(WON_CONST WONIPAddress* addr, char* strBuf, unsigned short* strBufLength, BOOL includePort);
void WONIPAddressGetSockaddr(WON_CONST WONIPAddress* addr, struct sockaddr_in* sockAddr);


//----------------------------------------------------------------------------------------------------
// WON Authentication

typedef struct unnamed_HWONAUTH { int unused; }* HWONAUTH;


// Tries to load/save verifier key, returns FALSE on failure
BOOL WONAuthLoadVerifierKeyFromFileA(WON_CONST char* theFile);
BOOL WONAuthLoadVerifierKeyFromFileW(WON_CONST wchar_t* theFile);
BOOL WONAuthWriteVerifierKeyToFileA(WON_CONST char* theFile);
BOOL WONAuthWriteVerifierKeyToFileW(WON_CONST wchar_t* theFile);

#ifdef WIN32
BOOL WONAuthLoadVerifierKeyFromRegistry(WON_CONST char* regPath, WON_CONST char* regKey);
BOOL WONAuthWriteVerifierKeyToRegistry(WON_CONST char* regPath, WON_CONST char* regKey);
#endif

void WONAuthSetVerifierKey(void* verifKey, unsigned long verifKeyLength);

// Fills buffer.  Size of buffer must be specified in bufSize
// Sets bufSize to size of verified key, on return.
// If buffer is too small, bufSize is set, but not data is copied
void WONAuthGetVerifierKey(void* verifierKeyBuf, unsigned short* bufSize);


void WONAuthSetCDKey(WON_CONST char* productName, void* cdKey, unsigned short cdKeySize, BOOL storeInHKeyCurUser);
void WONAuthSetCDKeyString(WON_CONST char* productName, WON_CONST char* cdKeyString, BOOL storeInHKeyCurUser);

BOOL WONAuthHasCDKey();
void WONAuthGetCDKey(void* cdKeyBuf, unsigned short* bufSize);
void WONAuthGetCDKeyAsString(char* cdKeyStrBuf, unsigned short* strBufLength);

BOOL WONAuthIsCDKeyValid();
BOOL WONAuthLoadCDKey(WON_CONST char* productName, BOOL fromHKeyCurUser);
BOOL WONAuthSaveCDKey();


// Specified whether or not a hashed file is used for authentication
BOOL WONAuthWillHash();
void WONAuthSetWillHash(BOOL doHash);

// Set file used for CDKey authentication challenge.  If no hash file is specified,
// the current app will be used instead.
void WONAuthSetHashFileA(WON_CONST char* theFile);
void WONAuthSetHashFileW(WON_CONST wchar_t* theFile);


// You MUST call WONAuthSetLoginKeyFile, and specify a temporary file, in order
// to CDKey authentication.  This file is used to store temporary login information
// so the CDKey can be re-authenticated on a subsequent launch, or after a crash,
// avoiding an "in use" error.

// WARNING: Different products/cd-keys must use different login key files.

void WONAuthSetLoginKeyFileA(WON_CONST char* theFile);
void WONAuthSetLoginKeyFileW(WON_CONST wchar_t* theFile);


// WONAuthLogin

HWONAUTH WONAuthLoginA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
					   WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
					   WON_CONST char* nicknameKey, long timeout);

HWONAUTH WONAuthLoginW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
					   WON_CONST wchar_t* name, WON_CONST wchar_t* community,
					   WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey, long timeout);

	// Passes param of HWONAUTH to the completion handle
void WONAuthLoginAsyncA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
						WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
						WON_CONST char* nicknameKey, long timeout, HWONCOMPLETION hCompletion);

	// Passes param of HWONAUTH to the completion handle
void WONAuthLoginAsyncW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
						WON_CONST wchar_t* name, WON_CONST wchar_t* community,
						WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey,
						long timeout, HWONCOMPLETION hCompletion);

HWONAUTH WONAuthLoginNewAccountA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								 WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
								 WON_CONST char* nicknameKey, long timeout);

HWONAUTH WONAuthLoginNewAccountW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								 WON_CONST wchar_t* name, WON_CONST wchar_t* community,
								 WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey,
								 long timeout);

	// Passes param of HWONAUTH to the completion handle
void WONAuthLoginNewAccountAsyncA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								  WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
								  WON_CONST char* nicknameKey, long timeout, HWONCOMPLETION hCompletion);

	// Passes param of HWONAUTH to the completion handle
void WONAuthLoginNewAccountAsyncW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								  WON_CONST wchar_t* name, WON_CONST wchar_t* community,
								  WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey,
								  long timeout, HWONCOMPLETION hCompletion);

WONError WONAuthChangePasswordA(HWONAUTH hAuth, WON_CONST char* newPassword, long timeout);
WONError WONAuthChangePasswordW(HWONAUTH hAuth, WON_CONST wchar_t* newPassword, long timeout);

	// Passes a WONError as param to completion
void WONAuthChangePasswordAsyncA(HWONAUTH hAuth, WON_CONST char* newPassword, long timeout,
								 HWONCOMPLETION hCompletion);

void WONAuthChangePasswordAsyncW(HWONAUTH hAuth, WON_CONST wchar_t* newPassword, long timeout,
								 HWONCOMPLETION hCompletion);

WONError WONAuthSetNicknameA(HWONAUTH hAuth, WON_CONST char* nicknameKey, WON_CONST char* newNickname,
							 long timeout);

WONError WONAuthSetNicknameW(HWONAUTH hAuth, WON_CONST wchar_t* nicknameKey,
							 WON_CONST wchar_t* newNickname, long timeout);

	// Passes a WONError as param to completion
void WONAuthSetNicknameAsyncA(HWONAUTH hAuth, WON_CONST char* nicknameKey, WON_CONST char* newNickname,
							  long timeout, HWONCOMPLETION hCompletion);

void WONAuthSetNicknameAsyncW(HWONAUTH hAuth, WON_CONST wchar_t* nicknameKey,
							  WON_CONST wchar_t* newNickname, long timeout, HWONCOMPLETION hCompletion);

BOOL WONAuthHasExpired(HWONAUTH hAuth);

time_t WONAuthGetExpiration(HWONAUTH hAuth);

WONError WONAuthRefresh(HWONAUTH hAuth, BOOL forceRefresh, long timeout);

	// Passes a WONError as param to completion
void WONAuthRefreshAsync(HWONAUTH hAuth, BOOL forceRefresh, long timeout, HWONCOMPLETION hCompletion);

void WONAuthCloseHandle(HWONAUTH hAuth);

WON_CONST char*		WONAuthGetNameA(HWONAUTH hAuth);
WON_CONST wchar_t*	WONAuthGetNameW(HWONAUTH hAuth);

WON_CONST char*		WONAuthGetCommunityA(HWONAUTH hAuth);
WON_CONST wchar_t*	WONAuthGetCommunityW(HWONAUTH hAuth);

WON_CONST char*		WONAuthGetPasswordA(HWONAUTH hAuth);
WON_CONST wchar_t*	WONAuthGetPasswordW(HWONAUTH hAuth);

WON_CONST char*		WONAuthGetNicknameA(HWONAUTH hAuth);
WON_CONST wchar_t*	WONAuthGetNicknameW(HWONAUTH hAuth);

WON_CONST char*		WONAuthGetNicknameKeyA(HWONAUTH hAuth);
WON_CONST wchar_t*	WONAuthGetNicknameKeyW(HWONAUTH hAuth);

BOOL WONAuthIsNicknameDefault(HWONAUTH hAuth);

unsigned long WONAuthGetUserID(HWONAUTH hAuth);
unsigned long WONAuthGetCommunityID(HWONAUTH hAuth);
unsigned short WONAuthGetTrustLevel(HWONAUTH hAuth);


// BufSize must contain pointer to size of buf, maximum size to copy, on entry.  Recieves actual size
void WONAuthGetCertificate(HWONAUTH hAuth, void* certificateBuf, unsigned short* bufSize);
void WONAuthGetPrivateKey(HWONAUTH hAuth, void* privateKeyBuf, unsigned short* bufSize);
void WONAuthGetPublicKey(HWONAUTH hAuth, void* publicKeyBuf, unsigned short* bufSize);

// Public key block will not exist until at least one login has completed
void WONAuthGetPublicKeyBlock(void* publicKeyBlockBuf, unsigned short* bufSize);


//----------------------------------------------------------------------------------------------------
// WON Contest API


// On entry, recvSize must contain size of recvData buffer.
// The used size will be returned in recvSize.
// If the buffer is too small, the buffer will be filled, but the actual buffer size will
// be indicated by recvSize.  Beware that the server was contacted, so re-issuing the request
// will communicate with the server again.  If a second request would have a consequence, be
// sure that recvSize is always large enough the first time.
// 
// timeout = timeout in milliseconds, If -1 or <0 means no timeout.
WONError WONContestDBCall(WON_CONST WONIPAddress* contestServerAddr, unsigned short numServers,
						  HWONAUTH hAuth, long timeout, short procedureNum,
						  WON_CONST GUID* GameGUID, unsigned short sendSize, void* sendData,
						  unsigned short* recvSize, void* recvData);

	// Passes param of WONError to the completion handle
void WONContestDBCallAsync(WON_CONST WONIPAddress* contestServerAddr, unsigned short numServers,
						   HWONAUTH hAuth, long timeout, short procedureNum,
						   WON_CONST GUID* GameGUID, unsigned short sendSize, void* sendData,
						   unsigned short* recvSize, void* recvData, HWONCOMPLETION hCompletion);



//----------------------------------------------------------------------------------------------------
// WONDataObject's
//
// A generic data-object, with a variable length type, and variable length data

typedef struct unnamed_HWONDATAOBJECT { int unused; }* HWONDATAOBJECT;

HWONDATAOBJECT WONDataObjectCreate(WON_CONST void* type, BYTE typeLen, void* data, unsigned short dataLen);
void WONDataObjectCloseHandle(HWONDATAOBJECT dataObj);

void WONDataObjectSetType(HWONDATAOBJECT dataObj, WON_CONST void* typeBuf, BYTE typeLen);
void WONDataObjectGetType(HWONDATAOBJECT dataObj, void* typeBuf, BYTE* typeLen);

void WONDataObjectSetData(HWONDATAOBJECT dataObj, WON_CONST void* dataBuf, unsigned short dataLen);
void WONDataObjectGetData(HWONDATAOBJECT dataObj, void* dataBuf, unsigned short* dataLen);

// Lifespan is ignored by Directory API, because Directory entries have their own lifespans
void WONDataObjectSetLifespan(HWONDATAOBJECT dataObj, unsigned long lifeSpan);	// in seconds
unsigned long WONDataObjectGetLifespan(HWONDATAOBJECT dataObj);
BOOL WONDataObjectIsExpired(HWONDATAOBJECT dataObj);


//----------------------------------------------------------------------------------------------------
// WONDirEntity's
//
// An object used to identify an individual entry in the directory server,
// either a directory or service.
//
// You cannot create a HWONDIRENTITY.  They are returned only from certain
// Directory API functions.  However, you must use WONDirEntityCloseHandle()
// to dispose of them.

typedef struct unnamed_HWONDIRENTITY { int unused; }* HWONDIRENTITY;


void WONDirEntityCloseHandle(HWONDIRENTITY hDirEntity);

char WONDirEntityGetType(HWONDIRENTITY hDirEntity);	// Return "D" for directory, or "S" for service
BOOL WONDirEntityIsService(HWONDIRENTITY hDirEntity);
BOOL WONDirEntityIsDirectory(HWONDIRENTITY hDirEntity);

void WONDirEntityGetPathA(HWONDIRENTITY hDirEntity, char* strBuf, unsigned short* strBufLen);
void WONDirEntityGetPathW(HWONDIRENTITY hDirEntity, wchar_t* strBuf, unsigned short* strBufLen);
void WONDirEntityGetFullPathA(HWONDIRENTITY hDirEntity, char* strBuf, unsigned short* strBufLen);	// adds name
void WONDirEntityGetFullPathW(HWONDIRENTITY hDirEntity, wchar_t* strBuf, unsigned short* strBufLen);	// adds name
void WONDirEntityGetNameA(HWONDIRENTITY hDirEntity, char* strBuf, unsigned short* strBufLen);
void WONDirEntityGetNameW(HWONDIRENTITY hDirEntity, wchar_t* strBuf, unsigned short* strBufLen);
void WONDirEntityGetDisplayNameA(HWONDIRENTITY hDirEntity, char* strBuf, unsigned short* strBufLen);
void WONDirEntityGetDisplayNameW(HWONDIRENTITY hDirEntity, wchar_t* strBuf, unsigned short* strBufLen);

time_t WONDirEntityGetCreationTime(HWONDIRENTITY hDirEntity);
time_t WONDirEntityGetTouchTime(HWONDIRENTITY hDirEntity);

unsigned long WONDirEntityGetLifespan(HWONDIRENTITY hDirEntity);	// in seconds

// Only applicable to directories
BOOL WONDirEntityIsVisible(HWONDIRENTITY hDirEntity);	

// Only applicable to services
void WONDirEntityGetAddress(HWONDIRENTITY hDirEntity, WONIPAddress* addr);	
void WONDirEntityGetAddressEx(HWONDIRENTITY hDirEntity, void* addrBuf, unsigned short* addrBufSize);	

unsigned short WONDirEntityGetNumDataObjects(HWONDIRENTITY hDirEntity);

// If copy is FALSE, the DataObject returned is actually embedded in the HWONDIRENTITY,
// and will become invalid when the HWONDIRENTITY is closed.
// If copy is TRUE, a new DataObject is returned, copied from the HWONDIRENTITY.  You
// must dipose of it yourself, with WONDataObjectCloseHandle().
HWONDATAOBJECT WONDirEntityGetDataObject(HWONDIRENTITY hDirEntity, unsigned short index, BOOL copy);	// index is 0..n




//----------------------------------------------------------------------------------------------------
// WON Directory Server API

// Shuffle the list of Directory Servers.  Other services will be shuffled when retrieved
// from the Directory Server in RecvMultiReply.
void WONDirShuffleDirectoryServerArray(WONIPAddress* directoryServers, unsigned short numAddrs);

// Queries a list of directory server for a list of some other type of server.  The array
// returned can be passed directly to other API functions.
WONError WONDirQueryServerListA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
								HWONAUTH hAuth, WON_CONST char* path, WON_CONST char* name,
								WON_CONST char* displayName, long timeout,
								WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs);

WONError WONDirQueryServerListW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
								HWONAUTH hAuth, WON_CONST wchar_t* path,
								WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
								long timeout, WONIPAddress* serverAddrBuf,
								unsigned short* numServerAddrs);

	// Passes param of WONError to the completion handle
void WONDirQueryServerListAsyncA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
								 HWONAUTH hAuth, WON_CONST char* path, WON_CONST char* name,
								 WON_CONST char* displayName, long timeout,
								 WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs,
								 HWONCOMPLETION hCompletion);

void WONDirQueryServerListAsyncW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
								 HWONAUTH hAuth, WON_CONST wchar_t* path,
								 WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
								 long timeout, WONIPAddress* serverAddrBuf,
								 unsigned short* numServerAddrs, HWONCOMPLETION hCompletion);


// Tells a Titan server to register with a directory server (list).
WONError WONDirRegisterTitanServerA(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									WON_CONST char* path, WON_CONST char* displayName,
									BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									unsigned short numDataObjects, long timeout);

WONError WONDirRegisterTitanServerW(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									WON_CONST wchar_t* path, WON_CONST wchar_t* displayName,
									BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									unsigned short numDataObjects, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRegisterTitanServerAsyncA(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									 WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST char* path, WON_CONST char* displayName,
									 BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									 unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion);

void WONDirRegisterTitanServerAsyncW(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									 WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST wchar_t* path, WON_CONST wchar_t* displayName,
									 BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									 unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion);


// Adds a directory to a directory server
WONError WONDirAddDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							 WON_CONST char* path, WON_CONST char* dirName, WON_CONST char* displayName,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout);

WONError WONDirAddDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* dirName, WON_CONST wchar_t* displayName,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout);

	// Passes param of WONError to the completion handle
void WONDirAddDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							  WON_CONST char* path, WON_CONST char* dirName, WON_CONST char* displayName,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion);

void WONDirAddDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* dirName, WON_CONST wchar_t* displayName,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion);


// Adds a service to a directory server
WONError WONDirAddServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
						   WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
						   BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout);

WONError WONDirAddServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
						   WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
						   BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout);


	// Passes param of WONError to the completion handle
void WONDirAddServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
							BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							long timeout, HWONCOMPLETION hCompletion);

void WONDirAddServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName, 
							BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							long timeout, HWONCOMPLETION hCompletion);


// Allows setting of the service address with a raw buffer of data
WONError WONDirAddServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							 WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);

WONError WONDirAddServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							 WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);


	// Passes param of WONError to the completion handle
void WONDirAddServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							  WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							  HWONCOMPLETION hCompletion);

void WONDirAddServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							  WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							  HWONCOMPLETION hCompletion);


// Renews a directory's lifespan
WONError WONDirRenewDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned long lifeSpan, long timeout);

WONError WONDirRenewDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned long lifeSpan, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenewDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);

void WONDirRenewDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);


// Renews a service's lifespan
WONError WONDirRenewServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							 unsigned long lifeSpan, long timeout);

WONError WONDirRenewServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							 unsigned long lifeSpan, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenewServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);

void WONDirRenewServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);


// Allows setting of the service address with a raw buffer of data
WONError WONDirRenewServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							   unsigned long lifeSpan, long timeout);

WONError WONDirRenewServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							   unsigned long lifeSpan, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenewServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);

void WONDirRenewServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion);


// ModifyDirectory
WONError WONDirModifyDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* newDirName, WON_CONST char* newDisplayName,
								unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);

WONError WONDirModifyDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* newDirName, WON_CONST wchar_t* newDisplayName,
								unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);

	// Passes param of WONError to the completion handle
void WONDirModifyDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* newDirName, WON_CONST char* newDisplayName,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion);

void WONDirModifyDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* newDirName, WON_CONST wchar_t* newDisplayName,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion);


// ModifyService
WONError WONDirModifyServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST char* newName, WON_CONST char* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							  unsigned long newLifeSpan, BOOL uniqueDisplayName,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);

WONError WONDirModifyServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							  unsigned long newLifeSpan, BOOL uniqueDisplayName,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);


	// Passes param of WONError to the completion handle
void WONDirModifyServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST char* newName, WON_CONST char* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							   unsigned long newLifeSpan, BOOL uniqueDisplayName,
							   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							   HWONCOMPLETION hCompletion);

void WONDirModifyServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							   unsigned long newLifeSpan, BOOL uniqueDisplayName,
							   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							   HWONCOMPLETION hCompletion);


// Allows setting of the service address with a raw buffer of data
WONError WONDirModifyServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST char* newName, WON_CONST char* newDisplayName,
								WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								unsigned long newLifeSpan, BOOL uniqueDisplayName,
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);


WONError WONDirModifyServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName,
								WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								unsigned long newLifeSpan, BOOL uniqueDisplayName,
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);


	// Passes param of WONError to the completion handle
void WONDirModifyServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST char* newName, WON_CONST char* newDisplayName,
								 WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName,
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion);


void WONDirModifyServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName,
								 WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName,
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion);


// RenameDirectory
WONError WONDirRenameDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout);

WONError WONDirRenameDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenameDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion);

void WONDirRenameDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion);


// RenameService
WONError WONDirRenameServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout);

WONError WONDirRenameServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenameServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
							   HWONCOMPLETION hCompletion);

void WONDirRenameServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
							   HWONCOMPLETION hCompletion);


// Allows setting of the service address with a raw buffer of data
WONError WONDirRenameServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout);

WONError WONDirRenameServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRenameServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion);

void WONDirRenameServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion);



// RemoveDirectory
WONError WONDirRemoveDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, long timeout);

WONError WONDirRemoveDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRemoveDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, long timeout, HWONCOMPLETION hCompletion);

void WONDirRemoveDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, long timeout, HWONCOMPLETION hCompletion);


// RemoveService
WONError WONDirRemoveServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr, long timeout);

WONError WONDirRemoveServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr, long timeout);


	// Passes param of WONError to the completion handle
void WONDirRemoveServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr, long timeout,
							   HWONCOMPLETION hCompletion);

void WONDirRemoveServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr, long timeout,
							   HWONCOMPLETION hCompletion);


// Allows specifying of the service address with a raw buffer of data
WONError WONDirRemoveServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								long timeout);

WONError WONDirRemoveServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								long timeout);


	// Passes param of WONError to the completion handle
void WONDirRemoveServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 long timeout, HWONCOMPLETION hCompletion);

void WONDirRemoveServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 long timeout, HWONCOMPLETION hCompletion);


enum WONDirGetNumDirEntitiesMode
{
	WONDir_Services = 0,
	WONDir_SubDirs = 1,
	WONDir_AllEntities = 2
};


// GetNumDirEntities
WONError WONDirGetNumDirEntitiesA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								  WON_CONST char* path, unsigned short* numEntities, enum WONDirGetNumDirEntitiesMode mode,
								  long timeout);

WONError WONDirGetNumDirEntitiesW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								  WON_CONST wchar_t* path, unsigned short* numEntities, enum WONDirGetNumDirEntitiesMode mode,
								  long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetNumDirEntitiesAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								   WON_CONST char* path, unsigned short* numEntities, enum WONDirGetNumDirEntitiesMode mode,
								   long timeout, HWONCOMPLETION hCompletion);

void WONDirGetNumDirEntitiesAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								   WON_CONST wchar_t* path, unsigned short* numEntities, enum WONDirGetNumDirEntitiesMode mode,
								   long timeout, HWONCOMPLETION hCompletion);


// GetNumSubDirs
WONError WONDirGetNumSubDirsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, unsigned short* numEntities, long timeout);

WONError WONDirGetNumSubDirsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, unsigned short* numEntities, long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetNumSubDirsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned short* numEntities, long timeout,
							   HWONCOMPLETION hCompletion);

void WONDirGetNumSubDirsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned short* numEntities, long timeout,
							   HWONCOMPLETION hCompletion);


// GetNumServices
WONError WONDirGetNumServicesA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned short* numEntities, long timeout);

WONError WONDirGetNumServicesW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned short* numEntities, long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetNumServicesAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, unsigned short* numEntities, long timeout,
								HWONCOMPLETION hCompletion);

void WONDirGetNumServicesAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, unsigned short* numEntities, long timeout,
								HWONCOMPLETION hCompletion);


// ClearDirectoryDataObjects
WONError WONDirClearDirectoryDataObjectsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										  long timeout);

WONError WONDirClearDirectoryDataObjectsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										  long timeout);


	// Passes param of WONError to the completion handle
void WONDirClearDirectoryDataObjectsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										   long timeout, HWONCOMPLETION hCompletion);

void WONDirClearDirectoryDataObjectsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										   long timeout, HWONCOMPLETION hCompletion);


// ClearServiceDataObjects
WONError WONDirClearServiceDataObjectsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
										HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);

WONError WONDirClearServiceDataObjectsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
										HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout);


	// Passes param of WONError to the completion handle
void WONDirClearServiceDataObjectsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
										 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
										 HWONCOMPLETION hCompletion);

void WONDirClearServiceDataObjectsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
										 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
										 HWONCOMPLETION hCompletion);


// Allows specifying of the service address with a raw buffer of data
WONError WONDirClearServiceDataObjectsExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										  unsigned short numDataObjects, long timeout);

WONError WONDirClearServiceDataObjectsExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										  unsigned short numDataObjects, long timeout);

	// Passes param of WONError to the completion handle
void WONDirClearServiceDataObjectsExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										   unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion);

void WONDirClearServiceDataObjectsExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										   unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion);


// SetDirectoryDataObjects

enum WONDirDataObjectSetMode {
	WONDir_DOSM_ADDREPLACE    = 0,  // Add on not exist, replace on exist
	WONDir_DOSM_ADDIGNORE     = 1,  // Add on not exist, ignore on exist
	WONDir_DOSM_ADDONLY       = 2,  // Add on not exist, error on exist
	WONDir_DOSM_REPLACEIGNORE = 3,  // Replace on exist, ignore on not exist
	WONDir_DOSM_REPLACEONLY   = 4,  // Replace on exist, error on not exist
	WONDir_DOSM_RESETDELETE   = 5   // Clear existing set first, then add all.
};

WONError WONDirSetDirectoryDataObjectsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										enum WONDirDataObjectSetMode mode, long timeout);

WONError WONDirSetDirectoryDataObjectsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										enum WONDirDataObjectSetMode mode, long timeout);


	// Passes param of WONError to the completion handle
void WONDirSetDirectoryDataObjectsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										 enum WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion);

void WONDirSetDirectoryDataObjectsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										 enum WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion);


// SetServiceDataObjects
WONError WONDirSetServiceDataObjectsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
									  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									  enum WONDirDataObjectSetMode mode, long timeout);

WONError WONDirSetServiceDataObjectsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
									  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									  enum WONDirDataObjectSetMode mode, long timeout);


	// Passes param of WONError to the completion handle
void WONDirSetServiceDataObjectsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
									   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									   enum WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion);


void WONDirSetServiceDataObjectsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
									   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									   enum WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion);


// Allows specifying of the service address with a raw buffer of data
WONError WONDirSetServiceDataObjectsExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										unsigned short numDataObjects, enum WONDirDataObjectSetMode mode, long timeout);


WONError WONDirSetServiceDataObjectsExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										unsigned short numDataObjects, enum WONDirDataObjectSetMode mode, long timeout);


	// Passes param of WONError to the completion handle
void WONDirSetServiceDataObjectsExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										 unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										 unsigned short numDataObjects, enum WONDirDataObjectSetMode mode, long timeout,
										 HWONCOMPLETION hCompletion);

void WONDirSetServiceDataObjectsExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										 unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										 unsigned short numDataObjects, enum WONDirDataObjectSetMode mode, long timeout,
										 HWONCOMPLETION hCompletion);


// For routines that return multiple DirEntities, using a WONDirEntityCallback
// allows you to parse them as they are received, rather than after all are
// received.  This can improve the user's experience, if there is a very large
// list of services.
// Note:	Do NOT close the HWONDIRENTITY passed to the callback with WONDirEntityCloseHandle().
//			These dirEntities are still being referenced internally.
typedef void (*WONDirEntityCallback)(HWONDIRENTITY dirEntity, void* callbackPrivData);



// GetDirectoryContents
// On entry, entityBufLen specifies the size of entityBuf as the number of HWONDIRENTITY's
// (and not the byte size).
WONError WONDirGetDirectoryContentsA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									 WONDirEntityCallback callback, void* callbackPrivData, long timeout);

WONError WONDirGetDirectoryContentsW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									 WONDirEntityCallback callback, void* callbackPrivData, long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetDirectoryContentsAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
									  HWONCOMPLETION hCompletion);

void WONDirGetDirectoryContentsAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
									  HWONCOMPLETION hCompletion);


enum WONDirGetFlags
{
	WONDir_GF_NOFLAGS = 0,

	// Bits 0-15 are for decomposition and common flags

	// Decomposition Flags - apply these decompositions for Directories
	WONDir_GF_DECOMPROOT      = 0x00000001,  // Add the dir itself 
	WONDir_GF_DECOMPSERVICES  = 0x00000002,  // Add dir services
	WONDir_GF_DECOMPSUBDIRS   = 0x00000004,  // Add dir subdirs
	WONDir_GF_DECOMPRECURSIVE = 0x00000008,  // Recursive into dir subdirs

	// Common flags - include these attributes for all entities
	WONDir_GF_ADDTYPE         = 0x00000010,  // Add entity types
	WONDir_GF_ADDDISPLAYNAME  = 0x00000020,  // Add display names
	WONDir_GF_ADDCREATED      = 0x00000040,  // Add creation date/time
	WONDir_GF_ADDTOUCHED      = 0x00000080,  // Add touched date/time
	WONDir_GF_ADDLIFESPAN     = 0x00000100,  // Add lifespan
	WONDir_GF_ADDDOTYPE       = 0x00000200,  // Add DataObject types
	WONDir_GF_ADDDODATA       = 0x00000400,  // Add DataObject data
	WONDir_GF_ADDDATAOBJECTS  = 0x00000800,  // Add all DataObjects
	WONDir_GF_ADDPERMISSIONS  = 0x00001000,  // Add permissions
	WONDir_GF_ADDCRC          = 0x00002000,  // Add entity CRC

	// Bits 16-23 are for Directory only fields

	// Directory Flags - include these attributes for directories
	WONDir_GF_DIRADDPATH      = 0x00010000,  // Add dir paths (from root)
	WONDir_GF_DIRADDNAME      = 0x00020000,  // Add service names
	WONDir_GF_DIRADDVISIBLE   = 0x00040000,  // Add directory visibility

	// Bits 24-31 are for Service only fields

	// Service Flags - include these attributes for services
	WONDir_GF_SERVADDPATH     = 0x01000000,  // Add dir paths (from root)
	WONDir_GF_SERVADDNAME     = 0x02000000,  // Add service names
	WONDir_GF_SERVADDNETADDR  = 0x04000000,  // Add service net addresses

	WONDir_GF_ALLFLAGS = 0xffffffff
};


// GetDirectory
// getFlags == mask of WONDirGetFlags
WONError WONDirGetDirectoryA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							 long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout);

WONError WONDirGetDirectoryW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							 long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout);

	// Passes param of WONError to the completion handle
void WONDirGetDirectoryAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							  long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							  HWONCOMPLETION hCompletion);

void WONDirGetDirectoryAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							  long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							  HWONCOMPLETION hCompletion);


// GetService
WONError WONDirGetServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
						   HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout);

WONError WONDirGetServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
						   HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						    WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
						    HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						    long timeout, HWONCOMPLETION hCompletion);

void WONDirGetServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						    WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
						    HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						    long timeout, HWONCOMPLETION hCompletion);


// Allows specifying of the service address with a raw buffer of data
WONError WONDirGetServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout);

WONError WONDirGetServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout);


	// Passes param of WONError to the completion handle
void WONDirGetServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion);

void WONDirGetServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion);


// FindMatchMode (byte) - Mode for find queries
enum WONDir_FindMatchMode
{
	WONDir_FMM_EXACT   = 0,  // Compared value must equal search value
	WONDir_FMM_BEGIN   = 1,  // Compared value must begin with search value
	WONDir_FMM_END     = 2,  // Compared value must end with search value
	WONDir_FMM_CONTAIN = 3,  // Compared value must contain search value

	WONDir_FMM_MAX
};


// FindFlags (byte) - Control flags for find queries
enum WONDir_FindFlags
{
	WONDir_FF_NOFLAGS   = 0,

	WONDir_FF_MATCHALL  = 0x01,  // Return all valid matches
	WONDir_FF_FULLKEY   = 0x02,  // Match only if all search field match
	WONDir_FF_RECURSIVE = 0x04,  // Search directories recursively for matches

	WONDir_FF_ALLFLAGS  = 0xff
};


// FindService
WONError WONDirFindServiceA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							WON_CONST char* name, WON_CONST char* displayName, WON_CONST WONIPAddress* serviceAddr,
							HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							WONDirEntityCallback callback, void* callbackPrivData, long timeout);

WONError WONDirFindServiceW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST WONIPAddress* serviceAddr,
							HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							WONDirEntityCallback callback, void* callbackPrivData, long timeout);


void WONDirFindServiceAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							 unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							 WON_CONST char* name, WON_CONST char* displayName, WON_CONST WONIPAddress* serviceAddr,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							 HWONCOMPLETION hCompletion);

void WONDirFindServiceAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							 unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							 WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST WONIPAddress* serviceAddr,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							 HWONCOMPLETION hCompletion);




WONError WONDirFindServiceExA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							  unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							  WON_CONST char* name, WON_CONST char* displayName, WON_CONST void* serviceAddr,
							  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							  unsigned short numDataObjects, WONDirEntityCallback callback,
							  void* callbackPrivData, long timeout);

WONError WONDirFindServiceExW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							  unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							  WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST void* serviceAddr,
							  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							  unsigned short numDataObjects, WONDirEntityCallback callback,
							  void* callbackPrivData, long timeout);


	// Passes param of WONError to the completion handle
void WONDirFindServiceExAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							   unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							   WON_CONST char* name, WON_CONST char* displayName, WON_CONST void* serviceAddr,
							   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							   unsigned short numDataObjects, WONDirEntityCallback callback,
							   void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion);

void WONDirFindServiceExAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							   unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							   WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST void* serviceAddr,
							   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							   unsigned short numDataObjects, WONDirEntityCallback callback,
							   void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion);




//----------------------------------------------------------------------------------------------------
// Event Server event logging API


// HWONEVENT
typedef struct unnamed_HWONEVENT { int unused; }* HWONEVENT;

HWONEVENT WONEventCreate(unsigned short activityType);
void WONEventCloseHandle(HWONEVENT hEvent);

void WONEventSetActivityType(HWONEVENT hEvent, unsigned short activityType);
void WONEventSetActivityDataTime(HWONEVENT hEvent, time_t activityDateTime);

void WONEventSetUserA(HWONEVENT hEvent, unsigned short authMethod, unsigned long userId, WON_CONST char* userName);
void WONEventSetUserW(HWONEVENT hEvent, unsigned short authMethod, unsigned long userId, WON_CONST wchar_t* userName);
void WONEventSetClientA(HWONEVENT hEvent, WON_CONST char* clientName, WON_CONST char* clientAddr);
void WONEventSetClientW(HWONEVENT hEvent, WON_CONST wchar_t* clientName, WON_CONST char* clientAddr);
void WONEventSetServerA(HWONEVENT hEvent, unsigned short serverType, WON_CONST char* logicalName, WON_CONST char* serverAddr);
void WONEventSetServerW(HWONEVENT hEvent, unsigned short serverType, WON_CONST wchar_t* logicalName, WON_CONST char* serverAddr);

void WONEventAddDetailStringA(HWONEVENT hEvent, unsigned short detailType, WON_CONST char* str);
void WONEventAddDetailStringW(HWONEVENT hEvent, unsigned short detailType, WON_CONST wchar_t* str);
void WONEventAddDetailNumber(HWONEVENT hEvent, unsigned short detailType, double d);

void WONEventAddAttachmentA(HWONEVENT hEvent, WON_CONST char* desc, BYTE contentType, void* body, unsigned short size);
void WONEventAddAttachmentW(HWONEVENT hEvent, WON_CONST wchar_t* desc, BYTE contentType, void* body, unsigned short size);

WONError WONEventReport(HWONAUTH hAuth, WON_CONST WONIPAddress* evtServers, unsigned int numAddrs,
						HWONEVENT* events, unsigned int numEvts, BOOL reliable, BOOL useUDP,
						long timeout, long perUDPtimeout);	// good perUDPtimeout = 2000

	// Passes param of WONError to the completion handle
void WONEventReportAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* evtServers, unsigned int numAddrs,
						 HWONEVENT* events, unsigned int numEvts, BOOL reliable, BOOL useUDP,
						 long timeout, long perUDPtimeout, HWONCOMPLETION hCompletion);// good perUDPtimeout = 2000


//----------------------------------------------------------------------------------------------------
// Peer-to-peer Authentication
//		Simple API for 1-off peer-to-peer authentication.

WONError WONPeerAuthListen(HWONAUTH hAuth, unsigned short listenPort, WON_CONST void* sendAfter,
						   unsigned short sendAfterSize);

WONError WONPeerAuthAccept(void* certificateBuf, unsigned short* certificateBufSize,
						   void* recvedAfterBuf, unsigned short* recvedAfterBufSize, long timeout);

	// Passes param of WONError to the completion handle
void WONPeerAuthAcceptAsync(void* certificateBuf, unsigned short* certificateBufSize,
							void* recvedAfterBuf, unsigned short* recvedAfterBufSize,
							long timeout, HWONCOMPLETION hCompletion);

void WONPeerAuthStopListening();

WONError WONPeerAuthenticate(HWONAUTH hAuth, WON_CONST WONIPAddress* host, void* certificateBuf,
							 unsigned short* certificateBufSize, WON_CONST void* sendAfter,
							 unsigned short sendAfterSize, void* recvedAfterBuf,
							 unsigned short* recvedAfterBufSize, long timeout);

	// Passes param of WONError to the completion handle
void WONPeerAuthenticateAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* host, void* certificateBuf,
							  unsigned short* certificateBufSize, WON_CONST void* sendAfter,
							  unsigned short sendAfterSize, void* recvedAfterBuf,
							  unsigned short* recvedAfterBufSize, long timeout, HWONCOMPLETION hCompletion);


// Peer-to-peer 1-off authentication.
// Uses two state machines, and doesn't do any IO.
// Allow games to implement peer-to-peer authentication using own transport layer
typedef struct unnamed_HWONPEERAUTHCLIENT { int unused; }* HWONPEERAUTHCLIENT;
typedef struct unnamed_HWONPEERAUTHSERVER { int unused; }* HWONPEERAUTHSERVER;

HWONPEERAUTHCLIENT WONPeerAuthClientStateCreate();
void WONPeerAuthClientStateCloseHandle(HWONPEERAUTHCLIENT hState);

HWONPEERAUTHSERVER WONPeerAuthServerStateCreate();
void WONPeerAuthServerStateCloseHandle(HWONPEERAUTHSERVER hState);


	// void** arguments return pointers to an internal buffer.  No need to dipose of them.
	// Beware that if you delete the state object, the buffer will also be released.


	// stage 1, call Init() and send req to the server
	// (You can get the certificate and private key out of an identity object.
	// Be sure it hasn't expired.)
WONError WONPeerAuthClientInit(HWONPEERAUTHCLIENT hState, HWONAUTH hWONAuth, WON_CONST void** req, unsigned short* reqSize);

WONError WONPeerAuthClientInitEx(HWONPEERAUTHCLIENT hState, WON_CONST void* certificate, unsigned short certificateSize,
								 WON_CONST void* privKey, unsigned short privKeySize, WON_CONST void** req, unsigned short* reqSize, time_t theAuthDelta);

	// stage 2, server recvs req, and must send challenge #1 to client
	// (You can get the certificate and private key out of an identity object.
	// Be sure it hasn't expired.)
WONError WONPeerAuthServerRecvAuth1Request(HWONPEERAUTHSERVER hState, WON_CONST void* req, unsigned short reqSize,
										   HWONAUTH hWONAuth, WON_CONST void** challenge1, unsigned short* challenge1Size);

WONError WONPeerAuthServerRecvAuth1RequestEx(HWONPEERAUTHSERVER hState, WON_CONST void* req, unsigned short reqSize,
											 WON_CONST void* serverCertificate, unsigned short serverCertificateSize,
											 WON_CONST void* serverPrivateKey, unsigned short serverPrivateKeySize,
											 WON_CONST void** challenge1, unsigned short* challenge1Size, time_t theAuthDelta);

	// stage 3, client recvs challenge #1, and must send challenge #2 to server
WONError WONPeerAuthClientRecvChallenge1(HWONPEERAUTHCLIENT hState, WON_CONST void* challenge1, unsigned short challenge1Size,
										 WON_CONST void** challenge2, unsigned short* challenge2Size);


	// stage 4, server recvs challenge #2, and must return final result to client.
	// Server side of peer-to-peer authentication is complete.
WONError WONPeerAuthServerRecvChallenge2(HWONPEERAUTHSERVER hState, WON_CONST void* challenge2, unsigned short challenge2Size,
										 WON_CONST void** result, unsigned short* resultSize);

	// stage 5, client recvs final result from server.
	// If it checks out, client side of peer-to-peer authentication is complete.
WONError WONPeerAuthClientRecvResult(HWONPEERAUTHCLIENT hState, WON_CONST void* result, unsigned short resultSize);


void WONPeerAuthServerGetSessionKey(HWONPEERAUTHSERVER hState, void* keyBuf, unsigned short* keyBufSize);
void WONPeerAuthClientGetSessionKey(HWONPEERAUTHCLIENT hState, void* keyBuf, unsigned short* keyBufSize);


//----------------------------------------------------------------------------------------------------
// Firewall Detector Server API
WONError WONFirewallDetect(WON_CONST WONIPAddress* firewallDetectorServers, unsigned int numAddrs,
						   BOOL* behindFirewall, unsigned short listenPort, long timeout);

	// Passes param of WONError to the completion handle
void WONFirewallDetectAsync(WON_CONST WONIPAddress* firewallDetectorServers, unsigned int numAddrs,
							BOOL* behindFirewall, unsigned short listenPort, long timeout,
							HWONCOMPLETION hCompletion);


//----------------------------------------------------------------------------------------------------
// Factory Server API
WONError WONFactoryStartTitanServerA(HWONAUTH hAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
									 WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
									 unsigned short* startedOnPorts, unsigned short* numStartedPorts,
									 WON_CONST char* commandLineFragment, BOOL replaceCommandLine,
									 unsigned char numPortsRequested, unsigned char numSpecificPorts,
									 unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
									 unsigned short numAuthorizedIPs, long timeout);

WONError WONFactoryStartTitanServerW(HWONAUTH hAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
									 WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
									 unsigned short* startedOnPorts, unsigned short* numStartedPorts,
									 WON_CONST wchar_t* commandLineFragment, BOOL replaceCommandLine,
									 unsigned char numPortsRequested, unsigned char numSpecificPorts,
									 unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
									 unsigned short numAuthorizedIPs, long timeout);

void WONFactoryStartTitanServerAsyncA(HWONAUTH hAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
									  WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
									  unsigned short* startedOnPorts, unsigned short* numStartedPorts,
									  WON_CONST char* commandLineFragment, BOOL replaceCommandLine,
									  unsigned char numPortsRequested, unsigned char numSpecificPorts,
									  unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
									  unsigned short numAuthorizedIPs, long timeout, HWONCOMPLETION hCompletion);

void WONFactoryStartTitanServerAsyncW(HWONAUTH hAuth, WON_CONST WONIPAddress* factories, unsigned int numFactories,
									  WONIPAddress* startedOnFactory, WON_CONST char* configurationName,
									  unsigned short* startedOnPorts, unsigned short* numStartedPorts,
									  WON_CONST wchar_t* commandLineFragment, BOOL replaceCommandLine,
									  unsigned char numPortsRequested, unsigned char numSpecificPorts,
									  unsigned short* specificPortsArray, WON_CONST WONIPAddress* authorizedIPs,
									  unsigned short numAuthorizedIPs, long timeout, HWONCOMPLETION hCompletion);

WONError WONFactoryStopTitanServer(HWONAUTH hAuth, WON_CONST WONIPAddress* factory, WON_CONST char* configurationName,
								   unsigned short processPort, long timeout);

void WONFactoryStopTitanServerAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* factory, WON_CONST char* configurationName,
									unsigned short processPort, long timeout, HWONCOMPLETION hCompletion);


//----------------------------------------------------------------------------------------------------
// Ping API

WONError WONPingIcmpOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout);
void WONPingIcmpOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						  HWONCOMPLETION hCompletion);


WONError WONPingIcmp(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					 unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage);
void WONPingIcmpAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
					  unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage,
					  HWONCOMPLETION hCompletion);


WONError WONPingTitanTCPOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
							 unsigned char lengthFieldSize, BOOL includeConnectTime);
void WONPingTitanTCPOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
							  unsigned char lengthFieldSize, BOOL includeConnectTime, HWONCOMPLETION hCompletion);


WONError WONPingTitanTCP(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						 unsigned char lengthFieldSize, BOOL includeConnectTime, unsigned int numPings,
						 unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage);
void WONPingTitanTCPAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						  unsigned char lengthFieldSize, BOOL includeConnectTime, unsigned int numPings,
						  unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage, HWONCOMPLETION hCompletion);


WONError WONPingTitanUDPOnce(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout);
void WONPingTitanUDPOnceAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
							  HWONCOMPLETION hCompletion);


WONError WONPingTitanUDP(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						 unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage);
void WONPingTitanUDPAsync(WON_CONST WONIPAddress* addr, unsigned int* pingTime, unsigned long timeout,
						  unsigned int numPings, unsigned long timeoutPerPing, BOOL setPingTimeArgToAverage,
						  HWONCOMPLETION hCompletion);


WONError WONPingOrderServersByIcmp(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
								   unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
								   unsigned int numPingsPerServer, unsigned long timeoutPerServer,
								   unsigned long timeoutPerPing, BOOL useAveragePing);
void WONPingOrderServersByIcmpAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion);


WONError WONPingOrderServersByTitanTCP(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									   unsigned char lengthFieldSize, BOOL includeConnectTime, 
									   unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									   unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									   unsigned long timeoutPerPing, BOOL useAveragePing);
void WONPingOrderServersByTitanTCPAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
										unsigned char lengthFieldSize, BOOL includeConnectTime, 
										unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
										unsigned int numPingsPerServer, unsigned long timeoutPerServer,
										unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion);


WONError WONPingOrderServersByTitanUDP(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
									   unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
									   unsigned int numPingsPerServer, unsigned long timeoutPerServer,
									   unsigned long timeoutPerPing, BOOL useAveragePing);
void WONPingOrderServersByTitanUDPAsync(WONIPAddress* addresses, unsigned int numAddresses, unsigned int* pingArray,
										unsigned int* numGoodAddrs, unsigned int maxSimultaniousPings,
										unsigned int numPingsPerServer, unsigned long timeoutPerServer,
										unsigned long timeoutPerPing, BOOL useAveragePing, HWONCOMPLETION hCompletion);

//----------------------------------------------------------------------------------------------------
// Profile API
WONError WONProfileCreate(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
                          WON_CONST char* emailAddress, long timeout);

	// Passes param of WONError to the completion handle
void WONProfileCreateAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
                           WON_CONST char* emailAddress, long timeout, HWONCOMPLETION hCompletion);

//----------------------------------------------------------------------------------------------------
// HTTP GET API

typedef BOOL (*WONProgressCallback)(unsigned long progress, unsigned long size, void* callbackPrivData);


WONError WONHTTPGetFile(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName, unsigned short httpPort, 
						WON_CONST char* getPath, WON_CONST char* saveAsFile, BOOL* isNew, time_t* modTime, 
						BOOL allowResume, WONProgressCallback callback, void* callbackPrivData, 
						long timeout);

void WONHTTPGetFileAsync(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName, unsigned short httpPort, 
						 WON_CONST char* getPath, WON_CONST char* saveAsFile, BOOL* isNew, time_t* modTime, 
						 BOOL allowResume, WONProgressCallback callback, void* callbackPrivData, 
						 long timeout, HWONCOMPLETION hCompletion);

WONError WONHTTPGetData(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName, unsigned short httpPort, 
						WON_CONST char* getPath, void* recvBuf, unsigned long* recvBufSize, 
						BOOL* isNew, time_t* modTime, WONProgressCallback callback, void* callbackPrivData, 
						long timeout);

void WONHTTPGetDataAsync(WON_CONST WONIPAddress* proxyServer, WON_CONST char* hostName, unsigned short httpPort, 
						 WON_CONST char* getPath, void* recvBuf, unsigned long* recvBufSize, 
						 BOOL* isNew, time_t* modTime, WONProgressCallback callback, void* callbackPrivData, 
						 long timeout, HWONCOMPLETION hCompletion);


//----------------------------------------------------------------------------------------------------
// Routing API
#include "Routing/CRoutingAPI.h"


#ifdef UNICODE
#define WONAuthLoadVerifierKeyFromFile		WONAuthLoadVerifierKeyFromFileW
#define WONAuthWriteVerifierKeyToFile		WONAuthWriteVerifierKeyToFileW
#define WONAuthSetLoginKeyFile		WONAuthSetLoginKeyFileW
#define WONAuthSetHashFile			WONAuthSetHashFileW
#define WONAuthLogin				WONAuthLoginW
#define WONAuthLoginAsync			WONAuthLoginAsyncW
#define WONAuthLoginNewAccount		WONAuthLoginNewAccountW
#define WONAuthLoginNewAccountAsync	WONAuthLoginNewAccountAsyncW
#define WONAuthChangePassword		WONAuthChangePasswordW
#define WONAuthChangePasswordAsync	WONAuthChangePasswordAsyncW
#define WONAuthSetNickname			WONAuthSetNicknameW
#define WONAuthSetNicknameAsync		WONAuthSetNicknameAsyncW
#define WONAuthGetName				WONAuthGetNameW
#define WONAuthGetCommunity			WONAuthGetCommunityW
#define WONAuthGetPassword			WONAuthGetPasswordW
#define WONAuthGetNickname			WONAuthGetNicknameW
#define WONAuthGetNicknameKey		WONAuthGetNicknameKeyW
//#define WONQueryServerListFromDirServer			WONQueryServerListFromDirServerW	// obsolete, but linkable
//#define WONQueryServerListFromDirServerAsync	WONQueryServerListFromDirServerAsyncW
#define WONDirEntityGetPath			WONDirEntityGetPathW
#define WONDirEntityGetFullPath		WONDirEntityGetFullPathW
#define WONDirEntityGetName			WONDirEntityGetNameW
#define WONDirEntityGetDisplayName	WONDirEntityGetDisplayNameW
#define WONDirQueryServerList		WONDirQueryServerListW
#define WONDirQueryServerListAsync	WONDirQueryServerListAsyncW
#define WONDirRegisterTitanServer	WONDirRegisterTitanServerW
#define WONDirRegisterTitanServerAsync	WONDirRegisterTitanServerAsyncW
#define WONDirAddDirectory			WONDirAddDirectoryW
#define WONDirAddDirectoryAsync		WONDirAddDirectoryAsyncW
#define WONDirAddService			WONDirAddServiceW
#define WONDirAddServiceAsync		WONDirAddServiceAsyncW
#define WONDirAddServiceEx			WONDirAddServiceExW
#define WONDirAddServiceExAsync		WONDirAddServiceExAsyncW
#define WONDirRenewDirectory		WONDirRenewDirectoryW
#define WONDirRenewDirectoryAsync	WONDirRenewDirectoryAsyncW
#define WONDirRenewService			WONDirRenewServiceW
#define WONDirRenewServiceAsync		WONDirRenewServiceAsyncW
#define WONDirRenewServiceEx		WONDirRenewServiceExW
#define WONDirRenewServiceExAsync	WONDirRenewServiceExAsyncW
#define WONDirModifyDirectory		WONDirModifyDirectoryW
#define WONDirModifyDirectoryAsync	WONDirModifyDirectoryAsyncW
#define WONDirModifyService			WONDirModifyServiceW
#define WONDirModifyServiceAsync	WONDirModifyServiceAsyncW
#define WONDirModifyServiceEx		WONDirModifyServiceExW
#define WONDirModifyServiceExAsync	WONDirModifyServiceExAsyncW
#define WONDirRenameDirectory		WONDirRenameDirectoryW
#define WONDirRenameDirectoryAsync	WONDirRenameDirectoryAsyncW
#define WONDirRenameService			WONDirRenameServiceW
#define WONDirRenameServiceAsync	WONDirRenameServiceAsyncW
#define WONDirRenameServiceEx		WONDirRenameServiceExW
#define WONDirRenameServiceExAsync	WONDirRenameServiceExAsyncW
#define WONDirRemoveDirectory		WONDirRemoveDirectoryW
#define WONDirRemoveDirectoryAsync	WONDirRemoveDirectoryAsyncW
#define WONDirRemoveService			WONDirRemoveServiceW
#define WONDirRemoveServiceAsync	WONDirRemoveServiceAsyncW
#define WONDirRemoveServiceEx		WONDirRemoveServiceExW
#define WONDirRemoveServiceExAsync	WONDirRemoveServiceExAsyncW
#define WONDirGetNumDirEntities		WONDirGetNumDirEntitiesW
#define WONDirGetNumDirEntitiesAsync	WONDirGetNumDirEntitiesAsyncW
#define WONDirGetNumSubDirs			WONDirGetNumSubDirsW
#define WONDirGetNumSubDirsAsync	WONDirGetNumSubDirsAsyncW
#define WONDirGetNumServices		WONDirGetNumServicesW
#define WONDirGetNumServicesAsync	WONDirGetNumServicesAsyncW
#define WONDirClearDirectoryDataObjects			WONDirClearDirectoryDataObjectsW
#define WONDirClearDirectoryDataObjectsAsync	WONDirClearDirectoryDataObjectsAsyncW
#define WONDirClearServiceDataObjects			WONDirClearServiceDataObjectsW
#define WONDirClearServiceDataObjectsAsync		WONDirClearServiceDataObjectsAsyncW
#define WONDirClearServiceDataObjectsEx			WONDirClearServiceDataObjectsExW
#define WONDirClearServiceDataObjectsExAsync	WONDirClearServiceDataObjectsExAsyncW
#define WONDirSetDirectoryDataObjects			WONDirSetDirectoryDataObjectsW
#define WONDirSetDirectoryDataObjectsAsync		WONDirSetDirectoryDataObjectsAsyncW
#define WONDirSetServiceDataObjects				WONDirSetServiceDataObjectsW
#define WONDirSetServiceDataObjectsAsync		WONDirSetServiceDataObjectsAsyncW
#define WONDirSetServiceDataObjectsEx			WONDirSetServiceDataObjectsExW
#define WONDirSetServiceDataObjectsExAsync		WONDirSetServiceDataObjectsExAsyncW
#define WONDirGetDirectoryContents				WONDirGetDirectoryContentsW
#define WONDirGetDirectoryContentsAsync			WONDirGetDirectoryContentsAsyncW
#define WONDirGetDirectory						WONDirGetDirectoryW
#define WONDirGetDirectoryAsync					WONDirGetDirectoryAsyncW
#define WONDirGetService						WONDirGetServiceW
#define WONDirGetServiceAsync					WONDirGetServiceAsyncW
#define WONDirGetServiceEx						WONDirGetServiceExW
#define WONDirGetServiceExAsync					WONDirGetServiceExAsyncW
#define WONDirFindService						WONDirFindServiceW
#define WONDirFindServiceAsync					WONDirFindServiceAsyncW
#define WONDirFindServiceEx						WONDirFindServiceExW
#define WONDirFindServiceExAsync				WONDirFindServiceExAsyncW
#define WONEventSetUser							WONEventSetUserW
#define WONEventSetClient						WONEventSetClientW
#define WONEventSetServer						WONEventSetServerW
#define WONEventAddDetailString					WONEventAddDetailStringW
#define WONEventAddAttachment					WONEventAddAttachmentW
#define WONFactoryStartTitanServer				WONFactoryStartTitanServerW
#define WONFactoryStartTitanServerAsync			WONFactoryStartTitanServerAsyncW
#define WONProfileSetGender						WONProfileSetGenderW
#define WONProfileSetFirstName					WONProfileSetFirstNameW
#define WONProfileSetLastName					WONProfileSetLastNameW
#define WONProfileSetMiddleName					WONProfileSetMiddleNameW
#define WONProfileSetCompanyName				WONProfileSetCompanyNameW
#define WONProfileSetAddress1					WONProfileSetAddress1W
#define WONProfileSetAddress2					WONProfileSetAddress2W
#define WONProfileSetCity						WONProfileSetCityW
#define WONProfileSetPostalCode					WONProfileSetPostalCodeW
#define WONProfileSetState						WONProfileSetStateW
#define WONProfileSetPhone1						WONProfileSetPhone1W
#define WONProfileSetPhone1Ext					WONProfileSetPhone1ExtW
#define WONProfileSetPhone2						WONProfileSetPhone2W
#define WONProfileSetPhone2Ext					WONProfileSetPhone2ExtW
#define WONProfileSetEmail1						WONProfileSetEmail1W
#define WONProfileSetEmail2						WONProfileSetEmail2W
#define WONProfileGetGender						WONProfileGetGenderW
#define WONProfileGetFirstName					WONProfileGetFirstNameW
#define WONProfileGetLastName					WONProfileGetLastNameW
#define WONProfileGetMiddleName					WONProfileGetMiddleNameW
#define WONProfileGetCompanyName				WONProfileGetCompanyNameW
#define WONProfileGetAddress1					WONProfileGetAddress1W
#define WONProfileGetAddress2					WONProfileGetAddress2W
#define WONProfileGetCity						WONProfileGetCityW
#define WONProfileGetPostalCode					WONProfileGetPostalCodeW
#define WONProfileGetState						WONProfileGetStateW
#define WONProfileGetPhone1						WONProfileGetPhone1W
#define WONProfileGetPhone1Ext					WONProfileGetPhone1ExtW
#define WONProfileGetPhone2						WONProfileGetPhone2W
#define WONProfileGetPhone2Ext					WONProfileGetPhone2ExtW
#define WONProfileGetEmail1						WONProfileGetEmail1W
#define WONProfileGetEmail2						WONProfileGetEmail2W
#else
#define WONAuthLoadVerifierKeyFromFile		WONAuthLoadVerifierKeyFromFileA
#define WONAuthWriteVerifierKeyToFile		WONAuthWriteVerifierKeyToFileA
#define WONAuthSetLoginKeyFile		WONAuthSetLoginKeyFileA
#define WONAuthSetHashFile			WONAuthSetHashFileA
#define WONAuthLogin				WONAuthLoginA
#define WONAuthLoginAsync			WONAuthLoginAsyncA
#define WONAuthLoginNewAccount		WONAuthLoginNewAccountA
#define WONAuthLoginNewAccountAsync	WONAuthLoginNewAccountAsyncA
#define WONAuthChangePassword		WONAuthChangePasswordA
#define WONAuthChangePasswordAsync	WONAuthChangePasswordAsyncA
#define WONAuthSetNickname			WONAuthSetNicknameA
#define WONAuthSetNicknameAsync		WONAuthSetNicknameAsyncA
#define WONAuthGetName				WONAuthGetNameA
#define WONAuthGetCommunity			WONAuthGetCommunityA
#define WONAuthGetPassword			WONAuthGetPasswordA
#define WONAuthGetNickname			WONAuthGetNicknameA
#define WONAuthGetNicknameKey		WONAuthGetNicknameKeyA
//#define WONQueryServerListFromDirServer			WONQueryServerListFromDirServerA	// obsolete, but linkable
//#define WONQueryServerListFromDirServerAsync	WONQueryServerListFromDirServerAsyncA
#define WONDirEntityGetPath			WONDirEntityGetPathA
#define WONDirEntityGetFullPath		WONDirEntityGetFullPathA
#define WONDirEntityGetName			WONDirEntityGetNameA
#define WONDirEntityGetDisplayName	WONDirEntityGetDisplayNameA
#define WONDirQueryServerList		WONDirQueryServerListA
#define WONDirQueryServerListAsync	WONDirQueryServerListAsyncA
#define WONDirRegisterTitanServer	WONDirRegisterTitanServerA
#define WONDirRegisterTitanServerAsync	WONDirRegisterTitanServerAsyncA
#define WONDirAddDirectory			WONDirAddDirectoryA
#define WONDirAddDirectoryAsync		WONDirAddDirectoryAsyncA
#define WONDirAddService			WONDirAddServiceA
#define WONDirAddServiceAsync		WONDirAddServiceAsyncA
#define WONDirAddServiceEx			WONDirAddServiceExA
#define WONDirAddServiceExAsync		WONDirAddServiceExAsyncA
#define WONDirRenewDirectory		WONDirRenewDirectoryA
#define WONDirRenewDirectoryAsync	WONDirRenewDirectoryAsyncA
#define WONDirRenewService			WONDirRenewServiceA
#define WONDirRenewServiceAsync		WONDirRenewServiceAsyncA
#define WONDirRenewServiceEx		WONDirRenewServiceExA
#define WONDirRenewServiceExAsync	WONDirRenewServiceExAsyncA
#define WONDirModifyDirectory		WONDirModifyDirectoryA
#define WONDirModifyDirectoryAsync	WONDirModifyDirectoryAsyncA
#define WONDirModifyService			WONDirModifyServiceA
#define WONDirModifyServiceAsync	WONDirModifyServiceAsyncA
#define WONDirModifyServiceEx		WONDirModifyServiceExA
#define WONDirModifyServiceExAsync	WONDirModifyServiceExAsyncA
#define WONDirRenameDirectory		WONDirRenameDirectoryA
#define WONDirRenameDirectoryAsync	WONDirRenameDirectoryAsyncA
#define WONDirRenameService			WONDirRenameServiceA
#define WONDirRenameServiceAsync	WONDirRenameServiceAsyncA
#define WONDirRenameServiceEx		WONDirRenameServiceExA
#define WONDirRenameServiceExAsync	WONDirRenameServiceExAsyncA
#define WONDirRemoveDirectory		WONDirRemoveDirectoryA
#define WONDirRemoveDirectoryAsync	WONDirRemoveDirectoryAsyncA
#define WONDirRemoveService			WONDirRemoveServiceA
#define WONDirRemoveServiceAsync	WONDirRemoveServiceAsyncA
#define WONDirRemoveServiceEx		WONDirRemoveServiceExA
#define WONDirRemoveServiceExAsync	WONDirRemoveServiceExAsyncA
#define WONDirGetNumDirEntities		WONDirGetNumDirEntitiesA
#define WONDirGetNumDirEntitiesAsync	WONDirGetNumDirEntitiesAsyncA
#define WONDirGetNumSubDirs			WONDirGetNumSubDirsA
#define WONDirGetNumSubDirsAsync	WONDirGetNumSubDirsAsyncA
#define WONDirGetNumServices		WONDirGetNumServicesA
#define WONDirGetNumServicesAsync	WONDirGetNumServicesAsyncA
#define WONDirClearDirectoryDataObjects			WONDirClearDirectoryDataObjectsA
#define WONDirClearDirectoryDataObjectsAsync	WONDirClearDirectoryDataObjectsAsyncA
#define WONDirClearServiceDataObjects			WONDirClearServiceDataObjectsA
#define WONDirClearServiceDataObjectsAsync		WONDirClearServiceDataObjectsAsyncA
#define WONDirClearServiceDataObjectsEx			WONDirClearServiceDataObjectsExA
#define WONDirClearServiceDataObjectsExAsync	WONDirClearServiceDataObjectsExAsyncA
#define WONDirSetDirectoryDataObjects			WONDirSetDirectoryDataObjectsA
#define WONDirSetDirectoryDataObjectsAsync		WONDirSetDirectoryDataObjectsAsyncA
#define WONDirSetServiceDataObjects				WONDirSetServiceDataObjectsA
#define WONDirSetServiceDataObjectsAsync		WONDirSetServiceDataObjectsAsyncA
#define WONDirSetServiceDataObjectsEx			WONDirSetServiceDataObjectsExA
#define WONDirGetDirectoryContents				WONDirGetDirectoryContentsA
#define WONDirGetDirectoryContentsAsync			WONDirGetDirectoryContentsAsyncA
#define WONDirGetDirectory						WONDirGetDirectoryA
#define WONDirGetDirectoryAsync					WONDirGetDirectoryAsyncA
#define WONDirGetService						WONDirGetServiceA
#define WONDirGetServiceAsync					WONDirGetServiceAsyncA
#define WONDirGetServiceEx						WONDirGetServiceExA
#define WONDirGetServiceExAsync					WONDirGetServiceExAsyncA
#define WONDirFindService						WONDirFindServiceA
#define WONDirFindServiceAsync					WONDirFindServiceAsyncA
#define WONDirFindServiceEx						WONDirFindServiceExA
#define WONDirFindServiceExAsync				WONDirFindServiceExAsyncA
#define WONEventSetUser							WONEventSetUserA
#define WONEventSetClient						WONEventSetClientA
#define WONEventSetServer						WONEventSetServerA
#define WONEventAddDetailString					WONEventAddDetailStringA
#define WONEventAddAttachment					WONEventAddAttachmentA
#define WONFactoryStartTitanServer				WONFactoryStartTitanServerA
#define WONFactoryStartTitanServerAsync			WONFactoryStartTitanServerAsyncA
#define WONProfileSetGender						WONProfileSetGenderA
#define WONProfileSetFirstName					WONProfileSetFirstNameA
#define WONProfileSetLastName					WONProfileSetLastNameA
#define WONProfileSetMiddleName					WONProfileSetMiddleNameA
#define WONProfileSetCompanyName				WONProfileSetCompanyNameA
#define WONProfileSetAddress1					WONProfileSetAddress1A
#define WONProfileSetAddress2					WONProfileSetAddress2A
#define WONProfileSetCity						WONProfileSetCityA
#define WONProfileSetPostalCode					WONProfileSetPostalCodeA
#define WONProfileSetState						WONProfileSetStateA
#define WONProfileSetPhone1						WONProfileSetPhone1A
#define WONProfileSetPhone1Ext					WONProfileSetPhone1ExtA
#define WONProfileSetPhone2						WONProfileSetPhone2A
#define WONProfileSetPhone2Ext					WONProfileSetPhone2ExtA
#define WONProfileSetEmail1						WONProfileSetEmail1A
#define WONProfileSetEmail2						WONProfileSetEmail2A
#define WONProfileGetGender						WONProfileGetGenderA
#define WONProfileGetFirstName					WONProfileGetFirstNameA
#define WONProfileGetLastName					WONProfileGetLastNameA
#define WONProfileGetMiddleName					WONProfileGetMiddleNameA
#define WONProfileGetCompanyName				WONProfileGetCompanyNameA
#define WONProfileGetAddress1					WONProfileGetAddress1A
#define WONProfileGetAddress2					WONProfileGetAddress2A
#define WONProfileGetCity						WONProfileGetCityA
#define WONProfileGetPostalCode					WONProfileGetPostalCodeA
#define WONProfileGetState						WONProfileGetStateA
#define WONProfileGetPhone1						WONProfileGetPhone1A
#define WONProfileGetPhone1Ext					WONProfileGetPhone1ExtA
#define WONProfileGetPhone2						WONProfileGetPhone2A
#define WONProfileGetPhone2Ext					WONProfileGetPhone2ExtA
#define WONProfileGetEmail1						WONProfileGetEmail1A
#define WONProfileGetEmail2						WONProfileGetEmail2A
#endif


#ifdef __cplusplus
}
#endif

#endif
