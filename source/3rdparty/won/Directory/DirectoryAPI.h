#ifndef __WON_DIRECTORY_API_H__
#define __WON_DIRECTORY_API_H__


#include "Socket/IPSocket.h"
#include "SDKCommon/Completion.h"
#include "Authentication/Identity.h"
#include "msg/Dir/DirEntity.h"
#include "msg/Dir/DirG2Flags.h"
#include "common/DataObject.h"
#include "common/WONString.h"


namespace WONAPI {


//--------------
// Functions in this API:
//--------------
// RegisterTitanServer
// AddDirectory
// AddService
// RenewDirectory
// RenewService
// ModifyDirectory
// ModifyService
// RenameDirectory
// RenameService
// RemoveDirectory
// RemoveService
// GetDirectoryContents
// GetNumDirEntities
// GetNumSubDirs
// GetNumServices
// ClearDirectoryDataObjects
// ClearServiceDataObjects
// SetDirectoryDataObjects
// SetServiceDataObjects
// GetDirectory
// GetService
//
// messages to be implemented :
// not yet supported by dir servers :
//
// find directory
// find service
// modify data object
// explicit set data object
//

struct DirResult
{
	Error error;
	IPSocket::Address directoryServer;

	DirResult(Error err, const IPSocket::Address& addr)
		:	error(err), directoryServer(addr)
	{ }
};



struct DirGetNumResult
{
	Error error;
	IPSocket::Address directoryServer;
	unsigned short* count;

	DirGetNumResult(Error err, const IPSocket::Address& addr, unsigned short* num)
		:	error(err), directoryServer(addr), count(num)
	{ }
};


typedef enum { DirServices = 0, DirSubDirs = 1, DirAllEntities = 2 }	GetNumDirEntitiesMode;


struct DirEntityResult
{
	Error error;
	IPSocket::Address directoryServer;
	WONMsg::DirEntity* entity;

	DirEntityResult(Error err, const IPSocket::Address& addr, WONMsg::DirEntity* dirEntity)
		:	error(err), directoryServer(addr), entity(dirEntity)
	{ }
};


struct DirEntityListResult
{
	Error error;
	IPSocket::Address directoryServer;
	WONMsg::DirEntityList* entityList;

	DirEntityListResult(Error err, const IPSocket::Address& addr, WONMsg::DirEntityList* dirEntityList)
		:	error(err), directoryServer(addr), entityList(dirEntityList)
	{ }
};


typedef void (*DirEntityCallback)(const WONMsg::DirEntity& dirEntity, void* callbackPrivData);

// Shuffle the list of Directory Servers.  Other services will be shuffled when retrieved
// from the Directory Server in RecvMultiReply.
void ShuffleDirectoryServerArray(IPSocket::Address* directoryServers, unsigned int numAddrs);

//-----------------------------------------------------------------------------
// RegisterTitanServer

// Does not currently support authentication...  Still ironing our restrictions
// for Titan directory registration requests...

// fail-over directory server addresses

Error RegisterTitanServer(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize,
						  const IPSocket::Address* directoryServers, unsigned int numAddrs,
						  const WONCommon::WONString& path, const WONCommon::WONString& displayName,
						  bool uniqueDisplayName,
						  const WONCommon::DataObjectTypeSet& dataObjects,
						  long timeout, bool async,
						  const CompletionContainer<Error>& completion );

inline WONCommon::DataObjectTypeSet getBlankDOTS()	// Damn VC5
{ return WONCommon::DataObjectTypeSet(); }

inline Error RegisterTitanServer(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize,
						  const IPSocket::Address* directoryServers, unsigned int numAddrs,
						  const WONCommon::WONString& path, const WONCommon::WONString& displayName,
						  bool uniqueDisplayName = true,
						  const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						  long timeout = -1, bool async = false )
{ return RegisterTitanServer(titanServerAddr, lengthFieldSize, directoryServers, numAddrs, path, displayName, uniqueDisplayName, dataObjects, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error RegisterTitanServerEx(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize, const IPSocket::Address* directoryServers, unsigned int numAddrs, const WONCommon::WONString& path, const WONCommon::WONString& displayName, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(Error, privsType), privsType privs)
{ return RegisterTitanServer(titanServerAddr, lengthFieldSize, directoryServers, numAddrs, path, displayName, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }


// 1 directory server address

inline Error RegisterTitanServer(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize,
								 const IPSocket::Address& directoryServer,
								 const WONCommon::WONString& path, const WONCommon::WONString& displayName,
								 bool uniqueDisplayName = true,
								 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
								 long timeout = -1, bool async = false,
								 const CompletionContainer<Error>& completion = DEFAULT_COMPLETION )
{ return RegisterTitanServer(titanServerAddr, lengthFieldSize, &directoryServer, 1, path, displayName, uniqueDisplayName, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error RegisterTitanServerEx(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& displayName, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(Error, privsType), privsType privs)
{ return RegisterTitanServer(titanServerAddr, lengthFieldSize, &directoryServer, 1, path, displayName, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// AddDirectory

// fail-over directory server addresses

Error AddDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
				   IPSocket::Address* addedToDirServer, const WONCommon::WONString& path,
				   const WONCommon::WONString& dirName, const WONCommon::WONString& displayName,
				   unsigned long lifeSpan, bool uniqueDisplayName, bool dirVisible, 
				   const WONCommon::DataObjectTypeSet& dataObjects,
				   long timeout, bool async,
				   const CompletionContainer<const DirResult&>& completion );

inline Error AddDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
				   IPSocket::Address* addedToDirServer, const WONCommon::WONString& path,
				   const WONCommon::WONString& dirName, const WONCommon::WONString& displayName,
				   unsigned long lifeSpan = 0, bool uniqueDisplayName = true, bool dirVisible = true, 
				   const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
				   long timeout = -1, bool async = false )
{ return AddDirectory(identity, directoryServers, numAddrs, addedToDirServer, path, dirName, displayName, lifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error AddDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* addedToDirServer, const WONCommon::WONString& path, const WONCommon::WONString& dirName, const WONCommon::WONString& displayName, unsigned long lifeSpan, bool uniqueDisplayName, bool dirVisible, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddDirectory(identity, directoryServers, numAddrs, addedToDirServer, path, dirName, displayName, lifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address

inline Error AddDirectory(Identity* identity, const IPSocket::Address& directoryServer,
						  const WONCommon::WONString& path, const WONCommon::WONString& dirName, const WONCommon::WONString& displayName,
						  unsigned long lifeSpan = 0, bool uniqueDisplayName = true, bool dirVisible = true, 
						  const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						  long timeout = -1, bool async = false,
						  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return AddDirectory(identity, &directoryServer, 1, 0, path, dirName, displayName, lifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error AddDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& dirName, const WONCommon::WONString& displayName, unsigned long lifeSpan, bool uniqueDisplayName, bool dirVisible, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddDirectory(identity, &directoryServer, 1, 0, path, dirName, displayName, lifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// AddService

// fail-over directory server addresses, and net address buffer

Error AddService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
				 IPSocket::Address* addedToDirServer, const WONCommon::WONString& path,
				 const WONCommon::WONString& name, const WONCommon::WONString& displayName,
				 const void* serviceAddr, unsigned short serviceAddrSize,
				 unsigned long lifeSpan = 0, bool uniqueDisplayName = true, bool overwrite = false,
				 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
				 long timeout = -1, bool async = false,
				 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error AddServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* addedToDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const void* serviceAddr, unsigned short serviceAddrSize, unsigned long lifeSpan, bool uniqueDisplayName, bool overwrite, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddService(identity, directoryServers, numAddrs, addedToDirServer, path, name, displayName, serviceAddr, serviceAddrSize, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// fail-over directory server addresses, and net address obj

inline Error AddService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						IPSocket::Address* addedToDirServer, const WONCommon::WONString& path,
						const WONCommon::WONString& name, const WONCommon::WONString& displayName,
						const IPSocket::Address& serviceAddr,  unsigned long lifeSpan = 0,
						bool uniqueDisplayName = true, bool overwrite = false,
						const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						long timeout = -1, bool async = false,
						const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return AddService(identity, directoryServers, numAddrs, addedToDirServer, path, name, displayName, &(serviceAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error AddServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* addedToDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr, unsigned long lifeSpan, bool uniqueDisplayName, bool overwrite, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddService(identity, directoryServers, numAddrs, addedToDirServer, path, name, displayName, &(serviceAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error AddService(Identity* identity, const IPSocket::Address& directoryServer,
				 const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName,
				 const void* serviceAddr, unsigned short serviceAddrSize,
				 unsigned long lifeSpan = 0, bool uniqueDisplayName = true, bool overwrite = false,
				 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
				 long timeout = -1, bool async = false,
				 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return AddService(identity, &directoryServer, 1, 0, path, name, displayName, serviceAddr, serviceAddrSize, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error AddServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const void* serviceAddr, unsigned short serviceAddrSize, unsigned long lifeSpan, bool uniqueDisplayName, bool overwrite, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddService(identity, &directoryServer, 1, 0, path, name, displayName, serviceAddr, serviceAddrSize, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address obj

inline Error AddService(Identity* identity, const IPSocket::Address& directoryServer,
						const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName,
						const IPSocket::Address& serviceAddr,  unsigned long lifeSpan = 0,
						bool uniqueDisplayName = true, bool overwrite = false,
						const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						long timeout = -1, bool async = false,
						const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return AddService(identity, &directoryServer, 1, 0, path, name, displayName, &(serviceAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error AddServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr, unsigned long lifeSpan, bool uniqueDisplayName, bool overwrite, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return AddService(identity, &directoryServer, 1, 0, path, name, displayName, &(serviceAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RenewDirectory

// fail-over directory server addresses

Error RenewDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					 IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path,
					 unsigned long lifeSpan = 0, long timeout = -1, bool async = false,
					 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RenewDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewDirectory(identity, directoryServers, numAddrs, renewedOnDirServer, path, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address

inline Error RenewDirectory(Identity* identity, const IPSocket::Address& directoryServer,
							const WONCommon::WONString& path, unsigned long lifeSpan = 0,
							long timeout = -1, bool async = false,
							const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenewDirectory(identity, &directoryServer, 1, 0, path, lifeSpan, timeout, async, completion); }

template <class privsType>
inline Error RenewDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewDirectory(identity, &directoryServer, 1, 0, path, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RenewService

// fail-over directory server addresses, and net address buffer

Error RenewService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
				   IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name,
				   const void* serviceAddr, unsigned short serviceAddrSize,
				   unsigned long lifeSpan = 0, long timeout = -1, bool async = false,
				   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RenewServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewService(identity, directoryServers, numAddrs, renewedOnDirServer, path, name, serviceAddr, serviceAddrSize, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// fail-over directory server addresses, and net address obj

inline Error RenewService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						  IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path,
						  const WONCommon::WONString& name, const IPSocket::Address& serviceAddr,
						  unsigned long lifeSpan = 0, long timeout = -1, bool async = false,
						  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenewService(identity, directoryServers, numAddrs, renewedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, lifeSpan, timeout, async, completion); }

template <class privsType>
inline Error RenewServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* renewedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewService(identity, directoryServers, numAddrs, renewedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error RenewService(Identity* identity, const IPSocket::Address& directoryServer,
						  const WONCommon::WONString& path, const WONCommon::WONString& name,
						  const void* serviceAddr, unsigned short serviceAddrSize,
						  unsigned long lifeSpan = 0, long timeout = -1, bool async = false,
						  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenewService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, lifeSpan, timeout, async, completion); }

template <class privsType>
inline Error RenewServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address obj

inline Error RenewService(Identity* identity, const IPSocket::Address& directoryServer,
						  const WONCommon::WONString& path, const WONCommon::WONString& name,
						  const IPSocket::Address& serviceAddr,  unsigned long lifeSpan = 0,
						  long timeout = -1, bool async = false,
						  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenewService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, lifeSpan, timeout, async, completion); }

template <class privsType>
inline Error RenewServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, unsigned long lifeSpan, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenewService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, lifeSpan, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// ModifyDirectory

// fail-over directory server addresses

Error ModifyDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					  IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
					  const WONCommon::WONString& newDirName, const WONCommon::WONString& newDisplayName,
					  unsigned long newLifeSpan = 0, bool uniqueDisplayName = true, bool dirVisible = true, 
					  const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
					  long timeout = -1, bool async = false,
					  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error ModifyDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& newDirName, const WONCommon::WONString& newDisplayName, unsigned long newLifeSpan, bool uniqueDisplayName, bool dirVisible, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyDirectory(identity, directoryServers, numAddrs, modifiedOnDirServer, path, newDirName, newDisplayName, newLifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address

inline Error ModifyDirectory(Identity* identity, const IPSocket::Address& directoryServer,
							 const WONCommon::WONString& path,
							 const WONCommon::WONString& newDirName, const WONCommon::WONString& newDisplayName,
							 unsigned long newLifeSpan = 0, bool uniqueDisplayName = true, bool dirVisible = true, 
							 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
							 long timeout = -1, bool async = false,
							 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ModifyDirectory(identity, &directoryServer, 1, 0, path, newDirName, newDisplayName, newLifeSpan, uniqueDisplayName, dirVisible, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error ModifyDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& newDirName, const WONCommon::WONString& newDisplayName, unsigned long newLifeSpan, bool uniqueDisplayName, bool dirVisible, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyDirectory(identity, &directoryServer, 1, 0, path, newDirName, newDisplayName, newLifeSpan, uniqueDisplayName, disVisible, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// ModifyService

// fail-over directory server addresses, and net address buffer

Error ModifyService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
					const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize,
					const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName,
					const void* newServiceAddr, unsigned short newServiceAddrSize,
					unsigned long newLifeSpan = 0, bool uniqueDisplayName = true,
					const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
					long timeout = -1, bool async = false,
					const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error ModifyServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName, const void* newServiceAddr, unsigned short newServiceAddrSize, unsigned long newLifeSpan, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, serviceAddr, serviceAddrSize, newName, newDisplayName, newServiceAddr, newServiceAddrSize, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// fail-over directory server addresses, and net address obj

inline Error ModifyService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						   IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
						   const WONCommon::WONString& name, const IPSocket::Address& serviceAddr,
						   const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName,
						   const IPSocket::Address& newServiceAddr,  unsigned long newLifeSpan = 0,
						   bool uniqueDisplayName = true,
						   const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ModifyService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, newName, newDisplayName, &(newServiceAddr.Get().sin_port), 6, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error ModifyServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName, const IPSocket::Address& newServiceAddr, unsigned long newLifeSpan, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, newName, newDisplayName, &(newServiceAddr.Get().sin_port), 6, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error ModifyService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const void* serviceAddr, unsigned short serviceAddrSize,
						   const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName,
						   const void* newServiceAddr, unsigned short newServiceAddrSize,
						   unsigned long newLifeSpan = 0, bool uniqueDisplayName = true,
						   const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ModifyService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, newName, newDisplayName, newServiceAddr, newServiceAddrSize, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error ModifyServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName, const void* newServiceAddr, unsigned short newServiceAddrSize, unsigned long newLifeSpan, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, newName, newDisplayName, newServiceAddr, newServiceAddrSize, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address obj

inline Error ModifyService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const IPSocket::Address& serviceAddr,
						   const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName,
						   const IPSocket::Address& newServiceAddr,  unsigned long newLifeSpan = 0,
						   bool uniqueDisplayName = true,
						   const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ModifyService(identity, &directoryServer, 1, 0, path, name, serviceAddr, newName, newDisplayName, newServiceAddr, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, completion); }

template <class privsType>
inline Error ModifyServiceEx(Identity* identity, const IPSocket::Address& directoryServers, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::WONString& newName, const WONCommon::WONString& newDisplayName, const IPSocket::Address& newServiceAddr, unsigned long newLifeSpan, bool uniqueDisplayName, const WONCommon::DataObjectTypeSet& dataObjects, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ModifyService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, newName, newDisplayName, &(newServiceAddr.Get().sin_port), 6, newLifeSpan, uniqueDisplayName, dataObjects, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RenameDirectory

// fail-over directory server addresses

Error RenameDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					  IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
					  const WONCommon::WONString& newDisplayName, bool uniqueDisplayName = true,
					  long timeout = -1, bool async = false,
					  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RenameDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameDirectory(identity, directoryServers, numAddrs, modifiedOnDirServer, path, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address

inline Error RenameDirectory(Identity* identity, const IPSocket::Address& directoryServer,
							 const WONCommon::WONString& path, const WONCommon::WONString& newDisplayName,
							 bool uniqueDisplayName = true, long timeout = -1, bool async = false,
							 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenameDirectory(identity, &directoryServer, 1, 0, path, newDisplayName, uniqueDisplayName, timeout, async, completion); }

template <class privsType>
inline Error RenameDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameDirectory(identity, &directoryServer, 1, 0, path, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RenameService

// fail-over directory server addresses, and net address buffer

Error RenameService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
					const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize,
					const WONCommon::WONString& newDisplayName, bool uniqueDisplayName = true, long timeout = -1,
					bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RenameServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::WONString& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, serviceAddr, serviceAddrSize, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// fail-over directory server addresses, and net address obj

inline Error RenameService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						   IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path,
						   const WONCommon::WONString& name, const IPSocket::Address& serviceAddr,
						   const WONCommon::WONString& newDisplayName, bool uniqueDisplayName = true,
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenameService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, newDisplayName, uniqueDisplayName, timeout, async, completion); }

template <class privsType>
inline Error RenameServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const std::string& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameService(identity, directoryServers, numAddrs, modifiedOnDirServer, path, name, &(serviceAddr.Get().sin_port), 6, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error RenameService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const void* serviceAddr, unsigned short serviceAddrSize,
						   const WONCommon::WONString& newDisplayName, bool uniqueDisplayName = true,
						   long timeout = -1, bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenameService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, newDisplayName, uniqueDisplayName, timeout, async, completion); }

template <class privsType>
inline Error RenameServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::WONString& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address obj

inline Error RenameService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const IPSocket::Address& serviceAddr, const WONCommon::WONString& newDisplayName,
						   bool uniqueDisplayName = true, long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RenameService(identity, &directoryServer, 1, 0, path, name, serviceAddr, newDisplayName, uniqueDisplayName, timeout, async, completion); }

template <class privsType>
inline Error RenameServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const std::string& newDisplayName, bool uniqueDisplayName, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RenameService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, newDisplayName, uniqueDisplayName, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RemoveDirectory

// fail-over directory server addresses

Error RemoveDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					  IPSocket::Address* removedFromDirServer, const WONCommon::WONString& path, long timeout = -1,
					  bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RemoveDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* removedFromDirServer, const WONCommon::WONString& path, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveDirectory(identity, directoryServers, numAddrs, removedFromDirServer, path, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address

inline Error RemoveDirectory(Identity* identity, const IPSocket::Address& directoryServer,
							 const WONCommon::WONString& path, long timeout = -1, bool async = false,
							 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RemoveDirectory(identity, &directoryServer, 1, 0, path, timeout, async, completion); }

template <class privsType>
inline Error RemoveDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveDirectory(identity, &directoryServer, 1, 0, path, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// RemoveService

// fail-over directory server addresses, and net address buffer

Error RemoveService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
					IPSocket::Address* removedFromDirServer,  const WONCommon::WONString& path,
					const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize,
					long timeout = -1, bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error RemoveServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* removedFromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveService(identity, directoryServers, numAddrs, removedFromDirServer, path, name, serviceAddr, serviceAddrSize, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// fail-over directory server addresses, and net address obj

inline Error RemoveService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						   IPSocket::Address* removedFromDirServer, const WONCommon::WONString& path,
						   const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, long timeout = -1,
						   bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RemoveService(identity, directoryServers, numAddrs, removedFromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, timeout, async, completion); }

template <class privsType>
inline Error RemoveServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* removedFromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveService(identity, directoryServers, numAddrs, removedFromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error RemoveService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const void* serviceAddr, unsigned short serviceAddrSize, long timeout = -1,
						   bool async = false, const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RemoveService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, timeout, async, completion); }

template <class privsType>
inline Error RemoveServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address obj

inline Error RemoveService(Identity* identity, const IPSocket::Address& directoryServer,
						   const WONCommon::WONString& path, const WONCommon::WONString& name,
						   const IPSocket::Address& serviceAddr, long timeout = -1, bool async = false,
						   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return RemoveService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, timeout, async, completion); }

template <class privsType>
inline Error RemoveServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return RemoveService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }



//-----------------------------------------------------------------------------
// GetDirectoryContents


// fail-over directory server addresses

Error GetDirectoryContents(Identity* identity, const IPSocket::Address* directoryServers,
						   unsigned int numAddrs, IPSocket::Address* fromDirServer,
						   const WONCommon::WONString& path, WONMsg::DirEntityList* result,
						   DirEntityCallback callback, void* callbackPrivData, long timeout, bool async,
						   const CompletionContainer<const DirEntityListResult&>& completion );

inline Error GetDirectoryContents(Identity* identity, const IPSocket::Address* directoryServers,
						   unsigned int numAddrs, IPSocket::Address* fromDirServer,
						   const WONCommon::WONString& path, WONMsg::DirEntityList* result,
						   DirEntityCallback callback = 0, void* callbackPrivData = 0,
						   long timeout = -1,  bool async = false )
{ return GetDirectoryContents(identity, directoryServers, numAddrs, fromDirServer, path, result, callback, callbackPrivData, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error GetDirectoryContentsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, WONMsg::DirEntityList* result, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return GetDirectoryContents(identity, directoryServers, numAddrs, fromDirServer, path, result, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error GetDirectoryContents(Identity* identity, const IPSocket::Address& directoryServer,
								  const WONCommon::WONString& path, WONMsg::DirEntityList* result,
								  DirEntityCallback callback = 0, void* callbackPrivData = 0,
								  long timeout = -1, bool async = false,
								  const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION )
{ return GetDirectoryContents(identity, &directoryServer, 1, 0, path, result, callback, callbackPrivData, timeout, async, completion); }

template <class privsType>
inline Error GetDirectoryContentsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, WONMsg::DirEntityList* result, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return GetDirectoryContents(identity, &directoryServer, 1, 0, path, result, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// GetNumDirEntities

// fail-over directory server addresses

Error GetNumDirEntities(Identity* identity, const IPSocket::Address* directoryServers,
						unsigned int numAddrs, IPSocket::Address* fromDirServer,
						const WONCommon::WONString& path, unsigned short* numEntities,
						GetNumDirEntitiesMode mode, long timeout, bool async,
						const CompletionContainer<const DirGetNumResult&>& completion );

inline Error GetNumDirEntities(Identity* identity, const IPSocket::Address* directoryServers,
						unsigned int numAddrs, IPSocket::Address* fromDirServer,
						const WONCommon::WONString& path, unsigned short* numEntities,
						GetNumDirEntitiesMode mode = DirAllEntities, long timeout = -1, bool async = false )
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, mode, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error GetNumDirEntitiesEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, unsigned short* numEntities, GetNumDirEntitiesMode mode, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, mode, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error GetNumDirEntities(Identity* identity, const IPSocket::Address& directoryServer,
							   const WONCommon::WONString& path, unsigned short* numEntities,
							   GetNumDirEntitiesMode mode = DirAllEntities, long timeout = -1, bool async = false,
							   const CompletionContainer<const DirGetNumResult&>& completion = DEFAULT_COMPLETION )
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, mode, timeout, async, completion); }

template <class privsType>
inline Error GetNumDirEntitiesEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, unsigned short* numEntities, GetNumDirEntitiesMode mode, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, mode, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// GetNumSubDirs

// fail-over directory server addresses

inline Error GetNumSubDirs(Identity* identity, const IPSocket::Address* directoryServers,
						   unsigned int numAddrs, IPSocket::Address* fromDirServer,
						   const WONCommon::WONString& path, unsigned short* numEntities,
						   long timeout = -1, bool async = false,
						   const CompletionContainer<const DirGetNumResult&>& completion = DEFAULT_COMPLETION )
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, DirSubDirs, timeout, async, completion); }

template <class privsType>
inline Error GetNumSubDirsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, unsigned short* numEntities, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, DirSubDirs, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error GetNumSubDirs(Identity* identity, const IPSocket::Address& directoryServer,
							   const WONCommon::WONString& path, unsigned short* numEntities,
							   long timeout = -1, bool async = false,
							   const CompletionContainer<const DirGetNumResult&>& completion = DEFAULT_COMPLETION )
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, DirSubDirs, timeout, async, completion); }

template <class privsType>
inline Error GetNumSubDirsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, unsigned short* numEntities, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, DirSubDirs, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// GetNumServices

// fail-over directory server addresses

inline Error GetNumServices(Identity* identity, const IPSocket::Address* directoryServers,
							unsigned int numAddrs, IPSocket::Address* fromDirServer,
							const WONCommon::WONString& path, unsigned short* numEntities,
							long timeout = -1, bool async = false,
							const CompletionContainer<const DirGetNumResult&>& completion = DEFAULT_COMPLETION )
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, DirServices, timeout, async, completion); }

template <class privsType>
inline Error GetNumServicesEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, unsigned short* numEntities, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, directoryServers, numAddrs, fromDirServer, path, numEntities, DirServices, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error GetNumServices(Identity* identity, const IPSocket::Address& directoryServer,
							const WONCommon::WONString& path, unsigned short* numEntities,
							long timeout = -1, bool async = false,
							const CompletionContainer<const DirGetNumResult&>& completion = DEFAULT_COMPLETION )
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, DirServices, timeout, async, completion); }

template <class privsType>
inline Error GetNumServicesEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, unsigned short* numEntities, long timeout, bool async, void (*f)(const DirGetNumResult&, privsType), privsType privs)
{ return GetNumDirEntities(identity, &directoryServer, 1, 0, path, numEntities, DirServices, timeout, async, new CompletionWithContents<const DirGetNumResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// ClearDirectoryDataObjects


// fail-over directory server addresses

Error ClearDirectoryDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
								unsigned int numAddrs, IPSocket::Address* fromDirServer,
								const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& clearSet,
								long timeout = -1, bool async = false,
								const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error ClearDirectoryDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearDirectoryDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error ClearDirectoryDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
									   const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& clearSet,
									   long timeout = -1, bool async = false,
									   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ClearDirectoryDataObjects(identity, &directoryServer, 1, 0, path, clearSet, timeout, async, completion); }

template <class privsType>
inline Error ClearDirectoryDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearDirectoryDataObjects(identity, &directoryServer, 1, 0, path, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// ClearServiceDataObjects

// fail-over directory server addresses, and net address buffer

Error ClearServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* fromDirServer,
							  const WONCommon::WONString& path,  const WONCommon::WONString& name,
							  const void* serviceAddr, unsigned short serviceAddrSize,
							  const WONCommon::DataObjectTypeSet& clearSet,
							  long timeout = -1, bool async = false,
							  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
inline Error ClearServiceDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path,  const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, serviceAddr, serviceAddrSize, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// fail-over directory server addresses, and net address obj

inline Error ClearServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
									 unsigned int numAddrs, IPSocket::Address* fromDirServer,
									 const WONCommon::WONString& path,  const WONCommon::WONString& name,
									 const IPSocket::Address& serviceAddr,
									 const WONCommon::DataObjectTypeSet& clearSet,
									 long timeout = -1, bool async = false,
									 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ClearServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, clearSet, timeout, async, completion); }


template <class privsType>
inline Error ClearServiceDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path,  const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address buffer

inline Error ClearServiceDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
									 const WONCommon::WONString& path,  const WONCommon::WONString& name,
									 const void* serviceAddr, unsigned short serviceAddrSize,
									 const WONCommon::DataObjectTypeSet& clearSet,
									 long timeout = -1, bool async = false,
									 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ClearServiceDataObjects(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, clearSet, timeout, async, completion); }


template <class privsType>
inline Error ClearServiceDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path,  const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearServiceDataObjects(identity, &directoryServers, 1, 0, path, name, serviceAddr, serviceAddrSize, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address obj

inline Error ClearServiceDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
									 const WONCommon::WONString& path,  const WONCommon::WONString& name,
									 const IPSocket::Address& serviceAddr,
									 const WONCommon::DataObjectTypeSet& clearSet,
									 long timeout = -1, bool async = false,
									 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return ClearServiceDataObjects(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, clearSet, timeout, async, completion); }


template <class privsType>
inline Error ClearServiceDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path,  const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& clearSet, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return ClearServiceDataObjects(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, clearSet, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// SetDirectoryDataObjects

// fail-over directory server addresses

Error SetDirectoryDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* fromDirServer,
							  const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& dataSet,
							  WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
							  long timeout = -1, bool async = false,
							  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error SetDirectoryDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetDirectoryDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error SetDirectoryDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
									 const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& dataSet,
									 WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
									 long timeout = -1, bool async = false,
									 const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return SetDirectoryDataObjects(identity, &directoryServer, 1, 0, path, dataSet, mode, timeout, async, completion); }

template <class privsType>
inline Error SetDirectoryDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetDirectoryDataObjects(identity, &directoryServer, 1, 0, path, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// SetServiceDataObjects

// fail-over directory server addresses, net address buffer

Error SetServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* fromDirServer,
							  const WONCommon::WONString& path, const WONCommon::WONString& name,
							  const void* serviceAddr, unsigned short serviceAddrSize,
							  const WONCommon::DataObjectTypeSet& dataSet,
							  WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
							  long timeout = -1, bool async = false,
							  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error SetServiceDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, serviceAddr, serviceAddrSize, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// fail-over directory server addresses, net address obj

inline Error SetServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
								   unsigned int numAddrs, IPSocket::Address* fromDirServer,
								   const WONCommon::WONString& path, const WONCommon::WONString& name,
								   const IPSocket::Address& serviceAddr,
								   const WONCommon::DataObjectTypeSet& dataSet,
								   WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
								   long timeout = -1, bool async = false,
								   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return SetServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, dataSet, mode, timeout, async, completion); }

template <class privsType>
inline Error SetServiceDataObjectsEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetServiceDataObjects(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address, net address buffer

inline Error SetServiceDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
							  const WONCommon::WONString& path, const WONCommon::WONString& name,
							  const void* serviceAddr, unsigned short serviceAddrSize,
							  const WONCommon::DataObjectTypeSet& dataSet,
							  WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
							  long timeout = -1, bool async = false,
							  const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return SetServiceDataObjects(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, dataSet, mode, timeout, async, completion); }

template <class privsType>
inline Error SetServiceDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetServiceDataObjects(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


// 1 directory server address, net address obj

inline Error SetServiceDataObjects(Identity* identity, const IPSocket::Address& directoryServer,
								   const WONCommon::WONString& path, const WONCommon::WONString& name,
								   const IPSocket::Address& serviceAddr,
								   const WONCommon::DataObjectTypeSet& dataSet,
								   WONMsg::DirG2DataObjectSetMode mode = WONMsg::DOSM_ADDREPLACE,
								   long timeout = -1, bool async = false,
								   const CompletionContainer<const DirResult&>& completion = DEFAULT_COMPLETION )
{ return SetServiceDataObjects(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, dataSet, mode, timeout, async, completion); }

template <class privsType>
inline Error SetServiceDataObjectsEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& dataSet, WONMsg::DirG2DataObjectSetMode mode, long timeout, bool async, void (*f)(const DirResult&, privsType), privsType privs)
{ return SetServiceDataObjects(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, dataSet, mode, timeout, async, new CompletionWithContents<const DirResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// GetDirectory

// fail-over directory server addresses

Error GetDirectory(Identity* identity, const IPSocket::Address* directoryServers,
				   unsigned int numAddrs, IPSocket::Address* fromDirServer,
				   const WONCommon::WONString& path, WONMsg::DirEntityList* result, long flags, //WONMsg::DirG2GetFlags
				   const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
				   DirEntityCallback callback = 0, void* callbackPrivData = 0,
				   long timeout = -1, bool async = false,
				   const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION );

template <class privsType>
inline Error GetDirectoryEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, WONMsg::DirEntityList* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return GetDirectory(identity, directoryServers, numAddrs, fromDirServer, path, result, flags, dataSet, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error GetDirectory(Identity* identity, const IPSocket::Address& directoryServer,
						  const WONCommon::WONString& path, WONMsg::DirEntityList* result, long flags, //WONMsg::DirG2GetFlags
						  const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
						  DirEntityCallback callback = 0, void* callbackPrivData = 0,
						  long timeout = -1, bool async = false,
						  const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION )
{ return GetDirectory(identity, &directoryServer, 1, 0, path, result, flags, dataSet, callback, callbackPrivData, timeout, async, completion); }

template <class privsType>
inline Error GetDirectoryEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, WONMsg::DirEntityList* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return GetDirectory(identity, &directoryServer, 1, 0, path, result, flags, dataSet, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// GetService

// fail-over directory server addresses, and net address buffer

Error GetService(Identity* identity, const IPSocket::Address* directoryServers,
				 unsigned int numAddrs, IPSocket::Address* fromDirServer,
				 const WONCommon::WONString& path, const WONCommon::WONString& name,
				 const void* serviceAddr, unsigned short serviceAddrSize,
				 WONMsg::DirEntity* result, long flags, //WONMsg::DirG2GetFlags
				 const WONCommon::DataObjectTypeSet& dataSet,
				 long timeout, bool async,
				 const CompletionContainer<const DirEntityResult&>& completion );

inline Error GetService(Identity* identity, const IPSocket::Address* directoryServers,
				 unsigned int numAddrs, IPSocket::Address* fromDirServer,
				 const WONCommon::WONString& path, const WONCommon::WONString& name,
				 const void* serviceAddr, unsigned short serviceAddrSize,
				 WONMsg::DirEntity* result, long flags, //WONMsg::DirG2GetFlags
				 const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
				 long timeout = -1, bool async = false )
{ return GetService(identity, directoryServers, numAddrs, fromDirServer, path, name, serviceAddr, serviceAddrSize, result, flags, dataSet, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error GetServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, WONMsg::DirEntity* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, long timeout, bool async, void (*f)(const DirEntityResult&, privsType), privsType privs)
{ return GetService(identity, directoryServers, numAddrs, fromDirServer, path, name, serviceAddr, serviceAddrSize, result, flags, dataSet, timeout, async, new CompletionWithContents<const DirEntityResult&, privsType>(f, privs, true)); }


// fail-over directory server addresses, and net address obj

inline Error GetService(Identity* identity, const IPSocket::Address* directoryServers,
						unsigned int numAddrs, IPSocket::Address* fromDirServer,
						const WONCommon::WONString& path, const WONCommon::WONString& name,
						const IPSocket::Address& serviceAddr, WONMsg::DirEntity* result, long flags, //WONMsg::DirG2GetFlags
						const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
						long timeout = -1, bool async = false,
						const CompletionContainer<const DirEntityResult&>& completion = DEFAULT_COMPLETION )
{ return GetService(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, result, flags, dataSet, timeout, async, completion); }

template <class privsType>
inline Error GetServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* fromDirServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, WONMsg::DirEntity* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, long timeout, bool async, void (*f)(const DirEntityResult&, privsType), privsType privs)
{ return GetService(identity, directoryServers, numAddrs, fromDirServer, path, name, &(serviceAddr.Get().sin_port), 6, result, flags, dataSet, timeout, async, new CompletionWithContents<const DirEntityResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address buffer

inline Error GetService(Identity* identity, const IPSocket::Address& directoryServer,
						const WONCommon::WONString& path, const WONCommon::WONString& name,
						const void* serviceAddr, unsigned short serviceAddrSize,
						WONMsg::DirEntity* result, long flags, //WONMsg::DirG2GetFlags
						const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
						long timeout = -1, bool async = false,
						const CompletionContainer<const DirEntityResult&>& completion = DEFAULT_COMPLETION )
{ return GetService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, result, flags, dataSet, timeout, async, completion); }

template <class privsType>
inline Error GetServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const void* serviceAddr, unsigned short serviceAddrSize, WONMsg::DirEntity* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, long timeout, bool async, void (*f)(const DirEntityResult&, privsType), privsType privs)
{ return GetService(identity, &directoryServer, 1, 0, path, name, serviceAddr, serviceAddrSize, result, flags, dataSet, timeout, async, new CompletionWithContents<const DirEntityResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address obj

inline Error GetService(Identity* identity, const IPSocket::Address& directoryServer,
						const WONCommon::WONString& path, const WONCommon::WONString& name,
						const IPSocket::Address& serviceAddr, WONMsg::DirEntity* result, long flags, //WONMsg::DirG2GetFlags
						const WONCommon::DataObjectTypeSet& dataSet = getBlankDOTS(),
						long timeout = -1, bool async = false,
						const CompletionContainer<const DirEntityResult&>& completion = DEFAULT_COMPLETION )
{ return GetService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, result, flags, dataSet, timeout, async, completion); }

template <class privsType>
inline Error GetServiceEx(Identity* identity, const IPSocket::Address& directoryServer, const WONCommon::WONString& path, const WONCommon::WONString& name, const IPSocket::Address& serviceAddr, WONMsg::DirEntity* result, long flags, const WONCommon::DataObjectTypeSet& dataSet, long timeout, bool async, void (*f)(const DirEntityResult&, privsType), privsType privs)
{ return GetService(identity, &directoryServer, 1, 0, path, name, &(serviceAddr.Get().sin_port), 6, result, flags, dataSet, timeout, async, new CompletionWithContents<const DirEntityResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// FindService
//
// findMatchMode == DirG2FindMatchMode
// findFlags == DirG2FindFlags

// fail-over directory server addresses, and net address buffer

Error FindService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
				  IPSocket::Address* foundOnDirServer, WONMsg::DirEntityList* result, long getFlags,
				  unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name,
				  const WONCommon::WONString& displayName, const void* serviceAddr = 0, unsigned short serviceAddrSize = 0,
				  const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
				  DirEntityCallback callback = 0, void* callbackPrivData = 0,
				  long timeout = -1, bool async = false,
				  const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION );


template <class privsType>
inline Error FindServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* foundOnDirServer, WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& dataObjects, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return FindService(identity, directoryServers, numAddrs, foundOnDirServer, result, getFlags, findMatchMode, findFlags, path, name, displayName, serviceAddr, serviceAddrSize, dataObjects, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


// fail-over directory server addresses, and net address obj

inline Error FindService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						 IPSocket::Address* foundOnDirServer, WONMsg::DirEntityList* result, long getFlags,
						 unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name,
						 const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr,
						 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						 DirEntityCallback callback = 0, void* callbackPrivData = 0, long timeout = -1, bool async = false,
						 const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION )
{ return FindService(identity, directoryServers, numAddrs, foundOnDirServer, result, getFlags, findMatchMode, findFlags, path, name, displayName, &(serviceAddr.Get().sin_port), 6, dataObjects, callback, callbackPrivData, timeout, async, completion); }


template <class privsType>
inline Error FindServiceEx(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs, IPSocket::Address* foundOnDirServer, WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& dataObjects, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return FindService(identity, directoryServers, numAddrs, foundOnDirServer, result, getFlags, findMatchMode, findFlags, path, name, displayName, &(serviceAddr.Get().sin_port), 6, dataObjects, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }



// 1 directory server address, and net address buffer

inline Error FindService(Identity* identity, const IPSocket::Address& directoryServer,
						 WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode, unsigned char findFlags,
						 const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName,
						 const void* serviceAddr = 0, unsigned short serviceAddrSize = 0,
						 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						 DirEntityCallback callback = 0, void* callbackPrivData = 0, long timeout = -1, bool async = false,
						 const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION )
{ return FindService(identity, &directoryServer, 1, 0, result, getFlags, findMatchMode, findFlags, path, name, displayName, serviceAddr, serviceAddrSize, dataObjects, callback, callbackPrivData, timeout, async, completion); }


template <class privsType>
inline Error FindServiceEx(Identity* identity, const IPSocket::Address& directoryServer, WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const void* serviceAddr, unsigned short serviceAddrSize, const WONCommon::DataObjectTypeSet& dataObjects, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return FindService(identity, &directoryServer, 1, 0, result, getFlags, findMatchMode, findFlags, path, name, displayName, serviceAddr, serviceAddrSize, dataObjects, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


// 1 directory server address, and net address obj

inline Error FindService(Identity* identity, const IPSocket::Address& directoryServer,
						 WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode,
						 unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name,
						 const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr,
						 const WONCommon::DataObjectTypeSet& dataObjects = getBlankDOTS(),
						 DirEntityCallback callback = 0, void* callbackPrivData = 0, long timeout = -1, bool async = false,
						 const CompletionContainer<const DirEntityListResult&>& completion = DEFAULT_COMPLETION )
{ return FindService(identity, &directoryServer, 1, 0, result, getFlags, findMatchMode, findFlags, path, name, displayName, &(serviceAddr.Get().sin_port), 6, dataObjects, callback, callbackPrivData, timeout, async, completion); }


template <class privsType>
inline Error FindServiceEx(Identity* identity, const IPSocket::Address& directoryServer, WONMsg::DirEntityList* result, long getFlags, unsigned char findMatchMode, unsigned char findFlags, const WONCommon::WONString& path, const WONCommon::WONString& name, const WONCommon::WONString& displayName, const IPSocket::Address& serviceAddr, const WONCommon::DataObjectTypeSet& dataObjects, DirEntityCallback callback, void* callbackPrivData, long timeout, bool async, void (*f)(const DirEntityListResult&, privsType), privsType privs)
{ return FindService(identity, &directoryServer, 1, 0, result, getFlags, findMatchMode, findFlags, path, name, displayName, &(serviceAddr.Get().sin_port), 6, dataObjects, callback, callbackPrivData, timeout, async, new CompletionWithContents<const DirEntityListResult&, privsType>(f, privs, true)); }


//-----------------------------------------------------------------------------
// QueryServerList

struct QueryServerListResult
{
	Error error;
	IPSocket::Address directoryServer;
	IPSocket::Address* serverAddres;
	unsigned int* numServerAddrs;

	QueryServerListResult(Error err, const IPSocket::Address& addr, IPSocket::Address* srvrs, unsigned int* numSrvrs)
		:	error(err), directoryServer(addr), serverAddres(srvrs), numServerAddrs(numSrvrs)
	{ }
};


// fail-over directory server addresses

Error QueryServerList(Identity* identity, const IPSocket::Address* directoryServers,
					  unsigned int numAddrs, const WONCommon::WONString& path, 
					  const WONCommon::WONString& name, const WONCommon::WONString& displayName,
					  const WONCommon::DataObjectTypeSet& dataObjects,
					  IPSocket::Address* serverAddrBuf, unsigned int* numServerAddrs, long timeout,
					  bool async, const CompletionContainer<const QueryServerListResult&>& completion );

inline Error QueryServerList(Identity* identity, const IPSocket::Address* directoryServers,
					  unsigned int numAddrs, const WONCommon::WONString& path, 
					  const WONCommon::WONString& name, const WONCommon::WONString& displayName,
					  const WONCommon::DataObjectTypeSet& dataObjects,
					  IPSocket::Address* serverAddrBuf, unsigned int* numServerAddrs, long timeout = -1,
					  bool async = false )
{ return QueryServerList(identity, directoryServers, numAddrs, path, name, displayName, dataObjects, serverAddrBuf, numServerAddrs, timeout, async, DEFAULT_COMPLETION); }

template <class privsType>
inline Error QueryServerListEx(Identity* identity, const IPSocket::Address* directoryServers,
					  unsigned int numAddrs, const WONCommon::WONString& path, 
					  const WONCommon::WONString& name, const WONCommon::WONString& displayName,
					  const WONCommon::DataObjectTypeSet& dataObjects,
					  IPSocket::Address* serverAddrBuf, unsigned int* numServerAddrs, long timeout,
					  bool async, void (*f)(const QueryServerListResult&, privsType), privsType privs)
{ return QueryServerList(identity, directoryServers, numAddrs, path, name, displayName, dataObjects, serverAddrBuf, numServersAddrs, timeout, async, new CompletionWithContents<const QueryServerListResult&, privsType>(f, privs, true)); }


// 1 directory server address

inline Error QueryServerList(Identity* identity, const IPSocket::Address& directoryServer,
							 const WONCommon::WONString& path,  const WONCommon::WONString& name,
							 const WONCommon::WONString& displayName, IPSocket::Address* serverAddrBuf,
							 const WONCommon::DataObjectTypeSet& dataObjects,
							 unsigned int* numServerAddrs, long timeout = -1, bool async = false,
							 const CompletionContainer<const QueryServerListResult&>& completion = DEFAULT_COMPLETION )
{ return QueryServerList(identity, &directoryServer, 1, path, name, displayName, dataObjects, serverAddrBuf, numServerAddrs, timeout, async, completion); }

template <class privsType>
inline Error QueryServerListEx(Identity* identity, const IPSocket::Address& directoryServer,
							   const WONCommon::WONString& path, const WONCommon::WONString& name,
							   const WONCommon::WONString& displayName,
							   const WONCommon::DataObjectTypeSet& dataObjects,
							   IPSocket::Address* serverAddrBuf, unsigned int* numServerAddrs, long timeout,
							   bool async, void (*f)(const QueryServerListResult&, privsType), privsType privs)
{ return QueryServerList(identity, &directoryServer, 1, path, name, displayName, dataObjects, serverAddrBuf, numServersAddrs, timeout, async, new CompletionWithContents<const QueryServerListResult&, privsType>(f, privs, true)); }

};

#endif
