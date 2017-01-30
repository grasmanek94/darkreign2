#pragma warning(disable : 4786)
#include <list>
#include <vector>
#include <algorithm>
#include "DirectoryAPI.h"
#include "SDKCommon/Pool.h"
#include "Authentication/AuthSocket.h"
#include "msg/Dir/SMsgDirG2AddDirectory.h"
#include "msg/Dir/SMsgDirG2AddService.h"
#include "msg/Dir/DirG2Flags.h"
#include "msg/Dir/SMsgDirG2StatusReply.h"
#include "msg/Dir/SMsgDirG2RemoveEntity.h"
#include "msg/Dir/SMsgDirG2RenewEntity.h"
#include "msg/Dir/SMsgDirG2ModifyDirectory.h"
#include "msg/Dir/SMsgDirG2ModifyService.h"
#include "msg/Dir/SMsgDirG2NameEntity.h"
#include "msg/Dir/SMsgDirG2GetDirContents.h"
#include "msg/Dir/SMsgDirG2GetNumEntities.h"
#include "msg/Dir/SMsgDirG2ClearDataObjects.h"
#include "msg/Dir/SMsgDirG2SetDataObjects.h"
#include "msg/Dir/SMsgDirG2GetEntity.h"
#include "msg/Dir/SMsgDirG2SingleEntityReply.h"
#include "msg/Dir/SMsgDirG2MultiEntityReply.h"
#include "msg/Comm/SMsgCommRegisterRequest.h"
#include "msg/Comm/SMsgCommStatusReply.h"
#include "msg/Dir/SMsgDirG2FindService.h"


using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;
using namespace WONAuth;
using std::map;
using std::string;
using std::wstring;
using std::list;
using std::pair;
using std::vector;


typedef vector<DirEntity> DirEntityVector;
typedef list<IPSocket::Address>	AddressList;

template<class T> void ShuffleArray(T* directoryServers, unsigned int numAddrs)
{
	vector<T> aAddrVec(numAddrs);
	for (int i = 0; i < numAddrs; ++i)
		aAddrVec[i] = directoryServers[i];
	std::random_shuffle(aAddrVec.begin(), aAddrVec.end());
	for (i = 0; i < numAddrs; ++i)
		directoryServers[i] = aAddrVec[i];
}

void WONAPI::ShuffleDirectoryServerArray(IPSocket::Address* directoryServers, unsigned int numAddrs)
{ ShuffleArray<IPSocket::Address>(directoryServers, numAddrs); }
void WONDirShuffleDirectoryServerArray(WONIPAddress* directoryServers, unsigned short numAddrs)
{ ShuffleArray<WONIPAddress>(directoryServers, numAddrs); }

enum OpType {	op_add, op_modify, op_renew, op_remove, op_rename,
				op_getContents, op_getNumEntities, op_setDataObjects,
				op_clearDataObjects, op_get, op_findService };

enum ResultType { status_result, num_result, single_result, multi_result };


class DirectoryData
{
public:
	OpType					opType;
	ResultType				resultType;
	bool					doService;

	Error					error;
	wstring					name;
	wstring					displayName;
	wstring					path;
	unsigned long			lifeSpan;
	bool					uniqueDisplayName;
	bool					overwrite;
	bool					dirVisible;
	bool					autoDelete;
	long					timeout;
	Event					doneEvent;
	CompletionContainer<const DirResult&>			dirResultCompletion;
	CompletionContainer<const DirGetNumResult&>		dirGetNumResultCompletion;
	CompletionContainer<const DirEntityListResult&>	dirEntityListResultCompletion;
	CompletionContainer<const DirEntityResult&>		dirEntityResultCompletion;
	IPSocket::Address		dirAddr;
	IPSocket::Address*		destServer;
	unsigned char*			serviceAddr;
	unsigned short			serviceAddrSize;
	DataObjectTypeSet		dataObjects;

	wstring					newName;
	wstring					newDisplayName;
	unsigned char*			newServiceAddr;
	unsigned short			newServiceAddrSize;

	AddressList				directories;
	AddressList::iterator	curDirectory;
	AuthSocket*				dirSocket;
	Auth1Certificate		cert;

	DirEntityList			tempEntityList;
	DirEntityList*			entityList;

	DirEntity				tempSingleEntity;
	DirEntity*				singleEntity;

	unsigned short			tempNumEntitiesResult;
	unsigned short*			numEntitiesResult;
	GetNumDirEntitiesMode	getNumMode;

	DirEntityCallback		callback;
	void*					callbackPrivData;
	DirG2DataObjectSetMode	setMode;

	long					getFlags;
	unsigned char			findFlags;
	unsigned char			findMatchMode;
	
	void Done()
	{
		IPSocket::Address resultDir;
		if (curDirectory != directories.end())
			resultDir = *curDirectory;
		if (destServer)
			*destServer = resultDir;

		switch (resultType)
		{
		case single_result:
			dirEntityResultCompletion.Complete(DirEntityResult(error, resultDir, singleEntity));
			break;
		case num_result:
			dirGetNumResultCompletion.Complete(DirGetNumResult(error, resultDir, numEntitiesResult));
			break;
		case multi_result:
			dirEntityListResultCompletion.Complete(DirEntityListResult(error, resultDir, entityList));
			break;
		//case status_result:
		default:
			dirResultCompletion.Complete(DirResult(error, resultDir));
			break;
		}

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}

	DirectoryData()
		:	dirSocket(0), serviceAddrSize(0), newServiceAddrSize(0)
	{
	}

	~DirectoryData()
	{
		delete dirSocket;

		if (serviceAddrSize)
			delete serviceAddr;
		if (newServiceAddrSize)
			delete newServiceAddr;
	}

	void TryNextDirectoryServer();
};


static void SessionOpenDone(const Socket::OpenResult& result, DirectoryData* dirData);


static void RecvStatusReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 1 == DirStatusReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 1 && srvType == 2)
	{
		try
		{
			SMsgDirG2StatusReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			short status = msg.GetStatus();
			
			if (status != StatusCommon_Success)
			{
				dirData->error = status;
				dirData->TryNextDirectoryServer();
			}
			else
			{
				dirData->error = Error_Success;
				dirData->Done();
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void RecvGetContentsReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 101 == DirG2GetDirectoryContentsReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 101 && srvType == 2)
	{
		try
		{
			SMsgDirG2GetDirectoryContentsReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			short status = msg.GetStatus();
			
			if (status != StatusCommon_Success)
			{
				dirData->error = status;
				dirData->TryNextDirectoryServer();
			}
			else
			{
				DirEntityList* newEntityList = &(msg.Entities());

				if (dirData->callback)
				{
					// An interesting side effect is that 
					DirEntityList::iterator itor = newEntityList->begin();
					while (itor != newEntityList->end())
					{
						dirData->callback(*itor, dirData->callbackPrivData);
						++itor;
					}
				}

				dirData->entityList->splice(dirData->entityList->end(), *newEntityList);

				if (!msg.IsLastReply())
					dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvGetContentsReply, dirData);
				else
				{
					dirData->error = Error_Success;
					dirData->Done();
				}
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void RecvGetNumEntitiesReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 112 == DirG2GetNumEntitiesReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 111 && srvType == 2)
	{
		try
		{
			SMsgDirG2GetNumEntitiesReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			SMsgDirG2GetNumEntitiesReply::NumEntriesList* entries = &(msg.Entries());
			if (entries->size() != 1)
			{
				dirData->error = Error_InvalidMessage;
				dirData->TryNextDirectoryServer();
			}
			else
			{

				SMsgDirG2GetNumEntitiesReply::NumEntriesData* entry = &(entries->front());
				
				short status = entry->first;
				
				if (status != StatusCommon_Success)
				{
					dirData->error = status;
					dirData->TryNextDirectoryServer();
				}
				else
				{
					*(dirData->numEntitiesResult) = entry->second;
					dirData->error = Error_Success;
					dirData->Done();
				}
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void RecvSingleReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 2 == DirG2SingleEntityReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 2 && srvType == 2)
	{
		try
		{
			SMsgDirG2SingleEntityReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();
				
			short status = msg.GetStatus();
				
			if (status != StatusCommon_Success)
			{
				dirData->error = status;
				dirData->TryNextDirectoryServer();
			}
			else
			{
				*(dirData->singleEntity) = msg.Entity();
				dirData->error = Error_Success;
				dirData->Done();
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void RecvMultiReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 3 == DirG2MultiEntityReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 3 && srvType == 2)
	{
		try
		{
			SMsgDirG2MultiEntityReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();

			short status = msg.GetStatus();
			
			if (status != StatusCommon_Success)
			{
				dirData->error = status;
				dirData->TryNextDirectoryServer();
			}
			else
			{
				dirData->entityList->splice(dirData->entityList->end(), msg.Entities());

				if (!msg.IsLastReply())
					dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvMultiReply, dirData);
				else
				{
					// copy the list to a vector so we can shuffle it (entities need to be
					// in a container that supports random access iterators).
					DirEntityList::iterator aListItr;
					DirEntityVector::iterator aVecItr;
					DirEntityVector aEntityVec(dirData->entityList->size());
					for (aVecItr = aEntityVec.begin(), aListItr = dirData->entityList->begin(); aListItr != dirData->entityList->end(); ++aListItr, ++aVecItr)
						*aVecItr = *aListItr;

					// shuffle services around within each directory
					bool haveShuffledOnce = false;
					DirEntityVector::iterator aDirBegin;
					aVecItr = aDirBegin = aEntityVec.begin();
					while (aVecItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
					{
						++aDirBegin; ++aVecItr;
					}
					for ( ; aVecItr != aEntityVec.end(); ++aVecItr)
					{
						if (aVecItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
						{
							std::random_shuffle(aDirBegin, aVecItr);
							aDirBegin = aVecItr;
							++aDirBegin;
							haveShuffledOnce = true;
						}
					}
					std::random_shuffle(aDirBegin, aEntityVec.end());

					// copy the shuffled data back into the list
					dirData->entityList->clear();
					for (aVecItr = aEntityVec.begin(); aVecItr != aEntityVec.end(); ++aVecItr)
						dirData->entityList->push_back(*aVecItr);
					
					if (dirData->callback)
					{
						// An interesting side effect is that 
						DirEntityList::iterator itor = dirData->entityList->begin();
						while (itor != dirData->entityList->end())
						{
							dirData->callback(*itor, dirData->callbackPrivData);
							++itor;
						}
					}

					dirData->error = Error_Success;
					dirData->Done();
				}
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void RecvSingleOrMultiReply(const AuthSocket::AuthRecvBaseMsgResult& result, DirectoryData* dirData)
{
	if (result.authExpired)
	{
		dirData->dirSocket->Close(0);
		dirData->dirSocket->OpenEx(true, dirData->timeout, true, SessionOpenDone, dirData);
		return;
	}
	if (!result.msg)
	{
		dirData->error = result.closed ? Error_ConnectionClosed : Error_Timeout;
		dirData->TryNextDirectoryServer();
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 3 == DirG2MultiEntityReply
	unsigned long srvType = result.msg->GetServiceType();	// 2 == SmallDirServerG2
	if (msgType == 2 && srvType == 2)
	{
		try
		{
			SMsgDirG2SingleEntityReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();
				
			short status = msg.GetStatus();
				
			if (status != StatusCommon_Success)
			{
				dirData->error = status;
				dirData->TryNextDirectoryServer();
			}
			else
			{
				DirEntity entity = msg.Entity();
				
				if (dirData->callback)
					dirData->callback(entity, dirData->callbackPrivData);
					
				dirData->entityList->push_back(entity);

				dirData->error = Error_Success;
				dirData->Done();
			}
		}
		catch (...)
		{
			dirData->error = Error_InvalidMessage;
			dirData->TryNextDirectoryServer();
		}
	}
	else if (msgType == 3 && srvType == 2)
		RecvMultiReply(result, dirData);
	else
	{
		dirData->error = Error_InvalidMessage;
		dirData->TryNextDirectoryServer();
	}
	delete result.msg;
}


static void SessionOpenDone(const Socket::OpenResult& result, DirectoryData* dirData)
{
	if (result.error != Error_Success)
	{
		dirData->error = result.error;
		dirData->TryNextDirectoryServer();
		return;
	}

	switch (dirData->opType)
	{
	case op_get:
		{
			bool extended = ((dirData->getFlags & WONMsg::GF_ADDDATAOBJECTS) || (dirData->dataObjects.size() > 0));
			SMsgDirG2GetEntity msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY, extended);
			if (extended)
				msg.SetGetTypes(dirData->dataObjects);

			msg.SetPath(dirData->path);
			msg.SetFlags(dirData->getFlags);
			
			if (dirData->doService)
			{
				msg.SetName(dirData->name);
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
			}

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, dirData->doService ? RecvSingleReply : RecvMultiReply, dirData);
		}
		break;
	case op_setDataObjects:
		{
			SMsgDirG2SetDataObjects msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY);

			msg.SetDataObjects(dirData->dataObjects);
			msg.SetPath(dirData->path);
			if (dirData->doService)
			{
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
			}
			msg.SetSetMode(dirData->setMode);

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_clearDataObjects:
		{
			SMsgDirG2ClearDataObjects msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY);

			msg.SetClearTypes(dirData->dataObjects);
			msg.SetPath(dirData->path);
			if (dirData->doService)
			{
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
			}

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_getNumEntities:
		{
			SMsgDirG2GetNumEntities::DirGetMode mode;
			switch (dirData->getNumMode)
			{
			case DirServices:
				mode = SMsgDirG2GetNumEntities::ServiceCount;
				break;
			case DirSubDirs:
				mode = SMsgDirG2GetNumEntities::SubDirCount;
				break;
			//case DirAllEntities:
			default:
				mode = SMsgDirG2GetNumEntities::TotalCount;
				break;
			};

			SMsgDirG2GetNumEntities msg;
			SMsgDirG2GetNumEntities::DirPathData dirPathData;
			dirPathData.mPath = dirData->path;
			dirPathData.mMode = mode;

			msg.Paths().push_back(dirPathData);
			
			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvGetNumEntitiesReply, dirData);
		}
		break;
	case op_getContents:
		{
			SMsgDirG2GetDirectoryContents msg;

			msg.SetPath(dirData->path);

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvGetContentsReply, dirData);
		}
		break;
	case op_findService:
		{
			bool extended = dirData->dataObjects.size() > 0;

			SMsgDirG2FindService msg(extended);
			
			if (extended)
				msg.SetSearchObjects(dirData->dataObjects);

			msg.SetFindMode((DirG2FindMatchMode)(dirData->findMatchMode));
			msg.SetFindFlags(dirData->findFlags);
			msg.SetDisplayName(dirData->displayName);
			msg.SetFlags(dirData->getFlags);
			msg.SetPath(dirData->path);
			msg.SetName(dirData->name);
			if (dirData->serviceAddrSize)
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvSingleOrMultiReply, dirData);
		}
		break;
	case op_rename:
		{
			SMsgDirG2NameEntity msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY);

			msg.SetUnique(dirData->uniqueDisplayName);
			msg.SetPath(dirData->path);
			if (dirData->doService)
			{
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
			}
			msg.SetDisplayName(dirData->newDisplayName);

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_modify:
		{
			bool extended = dirData->dataObjects.size() > 0;
			if (dirData->doService)
			{
				SMsgDirG2ModifyService msg(extended);

				if (extended)
					msg.SetDataObjects(dirData->dataObjects);

				unsigned char theFlags = dirData->uniqueDisplayName ? EF_UNIQUEDISPLAYNAME : 0;

				msg.SetEntityFlags(theFlags);
				msg.SetNewLifespan(dirData->lifeSpan);
				msg.SetPath(dirData->path);
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
				msg.SetNewName(dirData->newName);
				msg.SetNewDisplayName(dirData->newDisplayName);
				msg.SetNewNetAddress(RawBuffer(dirData->newServiceAddr, dirData->newServiceAddrSize));

				dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			}
			else
			{
				SMsgDirG2ModifyDirectory msg(extended);

				if (extended)
					msg.SetDataObjects(dirData->dataObjects);

				unsigned char theFlags = dirData->dirVisible ? EF_DIRVISIBLE : EF_DIRINVISIBLE;
				if (dirData->uniqueDisplayName)
					theFlags |= EF_UNIQUEDISPLAYNAME;

				msg.SetEntityFlags(theFlags);
				msg.SetNewLifespan(dirData->lifeSpan);
				msg.SetPath(dirData->path);
				msg.SetNewName(dirData->newName);
				msg.SetNewDisplayName(dirData->newDisplayName);

				dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			}
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_renew:
		{
			SMsgDirG2RenewEntity msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY);
		
			msg.SetLifespan(dirData->lifeSpan);
			msg.SetPath(dirData->path);
			if (dirData->doService)
			{
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
			}

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_remove:
		{
			SMsgDirG2RemoveEntity msg(dirData->doService ? SMsgDirG2KeyedBase::KT_SERVICE : SMsgDirG2KeyedBase::KT_DIRECTORY);

			msg.SetPath(dirData->path);

			if (dirData->doService)
			{
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));
				msg.SetName(dirData->name);
			}

			dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	case op_add:
		{
			bool extended = dirData->dataObjects.size() > 0;
			if (dirData->doService)
			{
				SMsgDirG2AddService msg(extended);

				unsigned char theFlags = dirData->uniqueDisplayName ? EF_UNIQUEDISPLAYNAME : 0;
				
				if (dirData->overwrite)
					theFlags |= EF_OVERWRITE;

				if (extended)
					msg.SetDataObjects(dirData->dataObjects);

				msg.SetEntityFlags(theFlags);
				msg.SetName(dirData->name);
				msg.SetDisplayName(dirData->displayName);
				msg.SetLifespan(dirData->lifeSpan);
				msg.SetPath(dirData->path);
				msg.SetNetAddress(RawBuffer(dirData->serviceAddr, dirData->serviceAddrSize));

				dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			}
			else
			{
				SMsgDirG2AddDirectory msg(extended);

				if (extended)
					msg.SetDataObjects(dirData->dataObjects);

				unsigned char theFlags = dirData->dirVisible ? EF_DIRVISIBLE : EF_DIRINVISIBLE;
				if (dirData->uniqueDisplayName)
					theFlags |= EF_UNIQUEDISPLAYNAME;

				msg.SetEntityFlags(theFlags);
				msg.SetDirName(dirData->name);
				msg.SetDisplayName(dirData->displayName);
				msg.SetLifespan(dirData->lifeSpan);
				msg.SetPath(dirData->path);

				dirData->dirSocket->SendBaseMsg(msg, true, dirData->timeout, true, true);
			}
			dirData->dirSocket->RecvBaseMsgEx(0, false, dirData->timeout, true, RecvStatusReply, dirData);
		}
		break;
	default:
		dirData->error = Error_InvalidParams;
		dirData->Done();
		break;
	}
}


void DirectoryData::TryNextDirectoryServer()
{
	if (++curDirectory == directories.end())
		Done();
	else
	{
		dirSocket->Close(0);
		dirSocket->ReleaseSession();
		dirSocket->GetSocket()->SetRemoteAddress(*curDirectory);
		dirSocket->OpenEx(true, timeout, true, SessionOpenDone, this);
	}
}


Error WONAPI::AddDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						   IPSocket::Address* addedToDir, const WONString& path,
						   const WONString& dirName, const WONString& displayName,
						   unsigned long lifeSpan, bool uniqueDisplayName, bool dirVisible,
						   const DataObjectTypeSet& dataObjects, long timeout, bool async,
						   const CompletionContainer<const DirResult&>& completion)
{
	if (addedToDir)
		*addedToDir = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->error = Error_Timeout;
				dirData->name = dirName.GetUnicodeString();
				dirData->displayName = displayName.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->lifeSpan = lifeSpan;
				dirData->uniqueDisplayName = uniqueDisplayName;
				dirData->dirVisible = dirVisible;
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = false;
				dirData->opType = op_add;
				dirData->resultType = status_result;
				dirData->destServer = addedToDir;
				dirData->dataObjects = dataObjects;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}
				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::AddService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						 IPSocket::Address* addedToDir, const WONString& path,
						 const WONString& name, const WONString& displayName,
						 const void* serviceAddr, unsigned short serviceAddrSize,
						 unsigned long lifeSpan, bool uniqueDisplayName, bool overwrite,
						 const DataObjectTypeSet& dataObjects, long timeout, bool async,
						 const CompletionContainer<const DirResult&>& completion)
{
	if (addedToDir)
		*addedToDir = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->error = Error_Timeout;
				dirData->name = name.GetUnicodeString();
				dirData->displayName = displayName.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->lifeSpan = lifeSpan;
				dirData->uniqueDisplayName = uniqueDisplayName;
				dirData->overwrite = overwrite;
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->opType = op_add;
				dirData->resultType = status_result;
				dirData->doService = true;
				dirData->destServer = addedToDir;
				dirData->dataObjects = dataObjects;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RemoveDirectory(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* removedFromDirServer,
							  const WONString& path, long timeout, bool async,
							  const CompletionContainer<const DirResult&>& completion)
{
	if (removedFromDirServer)
		*removedFromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_remove;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = false;
				dirData->destServer = removedFromDirServer;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RemoveService(Identity* identity, const IPSocket::Address* directoryServers,
							unsigned int numAddrs, IPSocket::Address* removedFromDirServer,
							const WONString& path, const WONString& name,
							const void* serviceAddr, unsigned short serviceAddrSize, long timeout,
							bool async, const CompletionContainer<const DirResult&>& completion)
{
	if (removedFromDirServer)
		*removedFromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_remove;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->name = name.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = true;
				dirData->destServer = removedFromDirServer;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}
				return err;		
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RenewDirectory(Identity* identity, const IPSocket::Address* directoryServers,
							 unsigned int numAddrs, IPSocket::Address* renewedOnDirServer,
							 const WONString& path, unsigned long lifeSpan, long timeout, bool async,
							 const CompletionContainer<const DirResult&>& completion)
{
	if (renewedOnDirServer)
		*renewedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_renew;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = false;
				dirData->destServer = renewedOnDirServer;
				dirData->lifeSpan = lifeSpan;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}
				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RenewService(Identity* identity, const IPSocket::Address* directoryServers,
						   unsigned int numAddrs, IPSocket::Address* renewedOnDirServer,
						   const WONString& path, const WONString& name,
						   const void* serviceAddr, unsigned short serviceAddrSize,
						   unsigned long lifeSpan, long timeout, bool async,
						   const CompletionContainer<const DirResult&>& completion)
{
	if (renewedOnDirServer)
		*renewedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_renew;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->name = name.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = true;
				dirData->destServer = renewedOnDirServer;
				dirData->lifeSpan = lifeSpan;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::ModifyDirectory(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer,
							  const WONString& path,
							  const WONString& newDirName, const WONString& newDisplayName,
							  unsigned long newLifeSpan, bool uniqueDisplayName, bool dirVisible, 
							  const DataObjectTypeSet& dataObjects,  long timeout, bool async,
							  const CompletionContainer<const DirResult&>& completion)
{
	if (modifiedOnDirServer)
		*modifiedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_modify;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = false;
				dirData->destServer = modifiedOnDirServer;
				dirData->newName = newDirName.GetUnicodeString();
				dirData->newDisplayName = newDisplayName.GetUnicodeString();
				dirData->uniqueDisplayName = uniqueDisplayName;
				dirData->dirVisible = dirVisible;
				dirData->lifeSpan = newLifeSpan;
				dirData->dataObjects = dataObjects;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::ModifyService(Identity* identity, const IPSocket::Address* directoryServers,
							unsigned int numAddrs, IPSocket::Address* modifiedOnDirServer,
							const WONString& path, const WONString& name, const void* serviceAddr,
							unsigned short serviceAddrSize, const WONString& newName,
							const WONString& newDisplayName, const void* newServiceAddr,
							unsigned short newServiceAddrSize, unsigned long newLifeSpan,
							bool uniqueDisplayName, const DataObjectTypeSet& dataObjects,
							long timeout, bool async,
							const CompletionContainer<const DirResult&>& completion)
{
	if (modifiedOnDirServer)
		*modifiedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		if (newServiceAddr && newServiceAddrSize)
		{
			dirData->newServiceAddr = new unsigned char[newServiceAddrSize];
			if (!dirData->newServiceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->newServiceAddr, newServiceAddr, newServiceAddrSize);
			dirData->newServiceAddrSize = newServiceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_modify;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->name = name.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = true;
				dirData->destServer = modifiedOnDirServer;
				dirData->newName = newName.GetUnicodeString();
				dirData->newDisplayName = newDisplayName.GetUnicodeString();
				dirData->uniqueDisplayName = uniqueDisplayName;
				dirData->lifeSpan = newLifeSpan;
				dirData->dataObjects = dataObjects;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RenameDirectory(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
							  IPSocket::Address* modifiedOnDirServer, const WONString& path,
							  const WONString& newDisplayName, bool uniqueDisplayName, long timeout,
							  bool async, const CompletionContainer<const DirResult&>& completion)
{
	if (modifiedOnDirServer)
		*modifiedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_rename;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = false;
				dirData->destServer = modifiedOnDirServer;
				dirData->newDisplayName = newDisplayName.GetUnicodeString();
				dirData->uniqueDisplayName = uniqueDisplayName;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::RenameService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
							IPSocket::Address* modifiedOnDirServer, const WONString& path,
							const WONString& name, const void* serviceAddr, unsigned short serviceAddrSize,
							const WONString& newDisplayName, bool uniqueDisplayName, long timeout,
							bool async, const CompletionContainer<const DirResult&>& completion)
{
	if (modifiedOnDirServer)
		*modifiedOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (!tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_rename;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->name = name.GetUnicodeString();
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->doService = true;
				dirData->destServer = modifiedOnDirServer;
				dirData->newDisplayName = newDisplayName.GetUnicodeString();
				dirData->uniqueDisplayName = uniqueDisplayName;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}

	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::GetDirectoryContents(Identity* identity, const IPSocket::Address* directoryServers,
								   unsigned int numAddrs, IPSocket::Address* fromDirServer,
								   const WONString& path, DirEntityList* result,
								   DirEntityCallback callback, void* callbackPrivData, long timeout, bool async,
								   const CompletionContainer<const DirEntityListResult&>& completion)
{
	if (result)
		result->clear();
	
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		DirEntityList tmpList;
		completion.Complete(DirEntityListResult(Error_InvalidParams, IPSocket::Address(), result ? result : &tmpList));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_getContents;
				dirData->resultType = multi_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirEntityListResultCompletion = completion;
				dirData->doService = false;
				dirData->destServer = fromDirServer;
				dirData->callback = callback;
				dirData->callbackPrivData = callbackPrivData;

				dirData->entityList = result ? result : &(dirData->tempEntityList);

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	DirEntityList tmpList;
	completion.Complete(DirEntityListResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmpList));
	return Error_OutOfMemory;
}


Error WONAPI::GetNumDirEntities(Identity* identity, const IPSocket::Address* directoryServers,
								unsigned int numAddrs, IPSocket::Address* fromDirServer,
								const WONString& path, unsigned short* numEntities,
								GetNumDirEntitiesMode mode, long timeout, bool async,
								const CompletionContainer<const DirGetNumResult&>& completion)
{
	if (numEntities)
		*numEntities = 0;
	
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		unsigned short tmpNumEntities = 0;
		completion.Complete(DirGetNumResult(Error_InvalidParams, IPSocket::Address(), numEntities ? numEntities : &tmpNumEntities));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_getNumEntities;
				dirData->resultType = num_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirGetNumResultCompletion = completion;
				//dirData->doService = false;
				dirData->destServer = fromDirServer;

				dirData->getNumMode = mode;
				dirData->numEntitiesResult = numEntities ? numEntities : &(dirData->tempNumEntitiesResult);

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}
				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	unsigned short tmpNumEntities = 0;
	completion.Complete(DirGetNumResult(Error_OutOfMemory, IPSocket::Address(), numEntities ? numEntities : &tmpNumEntities));
	return Error_OutOfMemory;
}


Error WONAPI::ClearDirectoryDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
										unsigned int numAddrs, IPSocket::Address* fromDirServer,
										const WONString& path, const DataObjectTypeSet& clearSet,
										long timeout, bool async,
										const CompletionContainer<const DirResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_clearDataObjects;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				//dirData->doService = false;
				dirData->destServer = fromDirServer;
				dirData->doService = false;
				dirData->dataObjects = clearSet;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;			
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}



Error WONAPI::ClearServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
							  unsigned int numAddrs, IPSocket::Address* fromDirServer,
							  const WONString& path,  const WONString& name,
							  const void* serviceAddr, unsigned short serviceAddrSize,
							  const DataObjectTypeSet& clearSet,
							  long timeout, bool async,
							  const CompletionContainer<const DirResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_clearDataObjects;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->destServer = fromDirServer;
				dirData->doService = true;
				dirData->dataObjects = clearSet;
				dirData->name = name.GetUnicodeString();

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::SetDirectoryDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
									  unsigned int numAddrs, IPSocket::Address* fromDirServer,
									  const WONString& path, const DataObjectTypeSet& dataSet,
									  DirG2DataObjectSetMode mode,
									  long timeout, bool async,
									  const CompletionContainer<const DirResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_setDataObjects;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				//dirData->doService = false;
				dirData->destServer = fromDirServer;
				dirData->doService = false;
				dirData->dataObjects = dataSet;
				dirData->setMode = mode;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::SetServiceDataObjects(Identity* identity, const IPSocket::Address* directoryServers,
									unsigned int numAddrs, IPSocket::Address* fromDirServer,
									const WONString& path, const WONString& name,
									const void* serviceAddr, unsigned short serviceAddrSize,
									const DataObjectTypeSet& dataSet,
									DirG2DataObjectSetMode mode,
									long timeout, bool async,
									const CompletionContainer<const DirResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		completion.Complete(DirResult(Error_InvalidParams, IPSocket::Address()));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_setDataObjects;
				dirData->resultType = status_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirResultCompletion = completion;
				dirData->destServer = fromDirServer;
				dirData->doService = true;
				dirData->dataObjects = dataSet;
				dirData->name = name.GetUnicodeString();
				dirData->setMode = mode;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirResult(Error_OutOfMemory, IPSocket::Address()));
	return Error_OutOfMemory;
}


Error WONAPI::GetDirectory(Identity* identity, const IPSocket::Address* directoryServers,
						   unsigned int numAddrs, IPSocket::Address* fromDirServer,
						   const WONString& path, DirEntityList* result, long getFlags,
						   const DataObjectTypeSet& dataSet, DirEntityCallback callback,
						   void* callbackPrivData, long timeout, bool async,
						   const CompletionContainer<const DirEntityListResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers || !getFlags)
	{
		DirEntityList tmpList;
		completion.Complete(DirEntityListResult(Error_InvalidParams, IPSocket::Address(), result ? result : &tmpList));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_get;
				dirData->resultType = multi_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirEntityListResultCompletion = completion;
				//dirData->doService = false;
				dirData->destServer = fromDirServer;
				dirData->doService = false;
				dirData->dataObjects = dataSet;
				dirData->getFlags = getFlags;
				dirData->entityList = result ? result : &(dirData->tempEntityList);
				dirData->callback = callback;
				dirData->callbackPrivData = callbackPrivData;

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;		
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	DirEntityList tmpList;
	completion.Complete(DirEntityListResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmpList));
	return Error_OutOfMemory;
}


Error WONAPI::GetService(Identity* identity, const IPSocket::Address* directoryServers,
				 unsigned int numAddrs, IPSocket::Address* fromDirServer,
				 const WONString& path, const WONString& name,
				 const void* serviceAddr, unsigned short serviceAddrSize,
				 DirEntity* result, long getFlags,
				 const DataObjectTypeSet& dataSet,
				 long timeout, bool async,
				 const CompletionContainer<const DirEntityResult&>& completion)
{
	if (fromDirServer)
		*fromDirServer = IPSocket::Address();

	DirEntity tmp;

	if (!numAddrs || !directoryServers || !getFlags)
	{
		completion.Complete(DirEntityResult(Error_InvalidParams, IPSocket::Address(), result ? result : &tmp));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				completion.Complete(DirEntityResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmp));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}

		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_get;
				dirData->resultType = single_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->name = name.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirEntityResultCompletion = completion;
				//dirData->doService = false;
				dirData->destServer = fromDirServer;
				dirData->doService = true;
				dirData->dataObjects = dataSet;
				dirData->getFlags = getFlags;
				dirData->singleEntity = result ? result : &(dirData->tempSingleEntity);

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	completion.Complete(DirEntityResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmp));
	return Error_OutOfMemory;
}


class RegisterTitanServerData
{
public:
	Error error;
	SMsgCommRegisterRequest msg;
	CompletionContainer<Error> completion;
	long timeout;
	bool autoDelete;
	Event doneEvent;
	TCPSocket tcpSocket;
	TMsgSocket tmsgSocket;

	RegisterTitanServerData(bool extended, unsigned char lengthSize)
		:	msg(extended),
			tmsgSocket(tcpSocket, lengthSize)
	{ }

	void Done(Error err)
	{
		error = err;

		completion.Complete(err);

		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}
};


static void RecvRegisterReply(const TMsgSocket::RecvBaseMsgResult& result, RegisterTitanServerData* registerData)
{
	if (!result.msg)
	{
		registerData->Done(result.closed ? Error_ConnectionClosed : Error_Timeout);
		return;
	}

	unsigned long msgType = result.msg->GetMessageType();	// 15 == SmallCommStatusReply
	unsigned long srvType = result.msg->GetServiceType();	// 1 == SmallCommonService
	if (msgType == 15 && srvType == 1)
	{
		try
		{
			SMsgCommStatusReply msg(*(SmallMessage*)(result.msg));

			msg.Unpack();
			
			registerData->Done(msg.GetStatus());
		}
		catch (...)
		{
			registerData->Done(Error_InvalidMessage);
		}
	}
	else
		registerData->Done(Error_InvalidMessage);
	delete result.msg;
}


static void RegisterDoneOpen(const TMsgSocket::OpenResult& result, RegisterTitanServerData* registerData)
{
	if (result.error != Error_Success)
	{
		registerData->Done(result.error);
		return;
	}
	registerData->tmsgSocket.SendBaseMsg(registerData->msg, registerData->timeout, true, true);
	registerData->tmsgSocket.RecvBaseMsgEx(0, registerData->timeout, true, RecvRegisterReply, registerData);
}


Error WONAPI::RegisterTitanServer(const IPSocket::Address& titanServerAddr, unsigned char lengthFieldSize,
								  const IPSocket::Address* directoryServers, unsigned int numAddrs,
								  const WONString& path, const WONString& displayName,
								  bool uniqueDisplayName,
								  const WONCommon::DataObjectTypeSet& dataObjects,
								  long timeout, bool async,
								  const CompletionContainer<Error>& completion)
{
	if (!numAddrs || !directoryServers)
	{
		completion.Complete(Error_InvalidParams);
		return Error_InvalidParams;
	}

	bool extended = dataObjects.size() > 0;

	RegisterTitanServerData* registerData = new RegisterTitanServerData(extended, lengthFieldSize);
	if (!registerData)
	{
		completion.Complete(Error_OutOfMemory);
		return Error_OutOfMemory;
	}

	if (extended)
		registerData->msg.SetDataObjects(dataObjects);

	registerData->msg.SetPath(path);
	registerData->msg.SetDisplayName(displayName);
	registerData->msg.SetRequireUniqueDisplayName(uniqueDisplayName);

	for (unsigned int i = 0; i < numAddrs; i++)
		registerData->msg.AddDirServerAddress(directoryServers[i].GetAddressString());

	registerData->timeout = timeout;
	registerData->autoDelete = async;
	registerData->completion = completion;

	registerData->tcpSocket.SetRemoteAddress(titanServerAddr);

	registerData->tmsgSocket.OpenEx(0, timeout, true, RegisterDoneOpen, registerData);

	Error err = Error_Pending;

	if (!async)
	{
		WSSocket::PumpUntil(registerData->doneEvent, timeout);
		//registerData->doneEvent.WaitFor();
		err = registerData->error;
		delete registerData;
	}

	return err;
}


Error WONAPI::FindService(Identity* identity, const IPSocket::Address* directoryServers, unsigned int numAddrs,
						  IPSocket::Address* foundOnDirServer, DirEntityList* result, 
						  long getFlags, unsigned char findMatchMode,
						  unsigned char findFlags, const WONString& path, const WONString& name,
						  const WONString& displayName,
						  const void* serviceAddr, unsigned short serviceAddrSize,
						  const DataObjectTypeSet& dataObjects,
						  DirEntityCallback callback, void* callbackPrivData, long timeout, bool async,
						  const CompletionContainer<const DirEntityListResult&>& completion)
{
	if (result)
		result->clear();
	
	if (foundOnDirServer)
		*foundOnDirServer = IPSocket::Address();

	if (!numAddrs || !directoryServers)
	{
		DirEntityList tmpList;
		completion.Complete(DirEntityListResult(Error_InvalidParams, IPSocket::Address(), result ? result : &tmpList));
		return Error_InvalidParams;
	}

	DirectoryData* dirData = new DirectoryData;
	if (dirData)
	{
		if (serviceAddr && serviceAddrSize)
		{
			dirData->serviceAddr = new unsigned char[serviceAddrSize];
			if (!dirData->serviceAddr)
			{
				delete dirData;
				DirEntityList tmpList;
				completion.Complete(DirEntityListResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmpList));
				return Error_OutOfMemory;
			}
			memcpy(dirData->serviceAddr, serviceAddr, serviceAddrSize);
			dirData->serviceAddrSize = serviceAddrSize;
		}
		TCPSocket* tcpSocket = new TCPSocket(directoryServers[0]);
		if (tcpSocket)
		{
			dirData->dirSocket = new AuthSocket(identity, tcpSocket, true, 4, true);
			if (dirData->dirSocket)
			{
				dirData->opType = op_findService;
				dirData->resultType = multi_result;
				dirData->error = Error_Timeout;
				dirData->path = path.GetUnicodeString();
				dirData->name = name.GetUnicodeString();
				dirData->displayName = displayName.GetUnicodeString();
				dirData->timeout = timeout;
				dirData->autoDelete = async;
				dirData->dirEntityListResultCompletion = completion;
				dirData->doService = true;
				dirData->destServer = foundOnDirServer;
				dirData->callback = callback;
				dirData->callbackPrivData = callbackPrivData;
				dirData->findMatchMode = findMatchMode;
				dirData->findFlags = findFlags;
				dirData->dataObjects = dataObjects;
				dirData->getFlags = getFlags;

				dirData->entityList = result ? result : &(dirData->tempEntityList);

				for (unsigned int i = 0; i < numAddrs; i++)
					dirData->directories.push_back(directoryServers[i]);

				dirData->curDirectory = dirData->directories.begin();

				dirData->dirSocket->OpenEx(true, timeout, true, SessionOpenDone, dirData);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(dirData->doneEvent, timeout);
					//dirData->doneEvent.WaitFor();
					err = dirData->error;
					delete dirData;
				}

				return err;
			}
			delete tcpSocket;
		}
		delete dirData;
	}
	DirEntityList tmpList;
	completion.Complete(DirEntityListResult(Error_OutOfMemory, IPSocket::Address(), result ? result : &tmpList));
	return Error_OutOfMemory;
}


#include "wondll.h"


static WONError ConvertDLLAddrList(const WONIPAddress* serverAddr, unsigned short numServers,
								   IPSocket::Address*& addrs)
{
	addrs = 0;
	if (numServers)
	{
		// build server list
		addrs = new IPSocket::Address[numServers];
		if (addrs)
		{
			array_auto_ptr<IPSocket::Address> autoDelAddrs(addrs);
			unsigned short curServer = 0;
			do {
				addrs[curServer].Set(serverAddr[curServer]);
			} while (++curServer < numServers);
			autoDelAddrs.release();
			return Error_Success;
		}
		return Error_OutOfMemory;
	}
	return Error_Success;
}



WONError WONQueryServerListFromDirServerLL(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										  HWONAUTH hWONAuth, WON_CONST wstring& path,
										  WON_CONST wstring& name, WON_CONST wstring& displayName,
										  long timeout, WONIPAddress* serverAddrBuf,
										  unsigned short* numServerAddrs)
{
	Error err = Error_InvalidParams;
	if (numServerAddrs)
	{
		unsigned short maxServers = *numServerAddrs;
		*numServerAddrs = 0;
		if (maxServers && numDirServers && serverAddrBuf)
		{
			IPSocket::Address* addrs;
			err = ConvertDLLAddrList(dirServerAddr, numDirServers, addrs);
			if (err == Error_Success)
			{
				DirEntityList entityList;
				err = FindService((Identity*)hWONAuth, addrs, numDirServers,
								  0, &entityList, 
								  GF_DECOMPSERVICES | GF_SERVADDNETADDR, FMM_EXACT,
								  FF_MATCHALL, path, name, displayName,
								  0, 0, DataObjectTypeSet(), 0, 0, timeout);
				if (err == Error_Success)
				{
					*numServerAddrs = entityList.size();
					unsigned short curIndex = 0;
					DirEntityList::iterator itor = entityList.begin();
					while (itor != entityList.end())
					{
						const unsigned char* addrBuf = (*itor).mNetAddress.data();

						WONIPAddress* curServer = &(serverAddrBuf[curIndex]);

						*(in_addr*)(&(curServer->ipAddress)) = *(in_addr*)(addrBuf + sizeof(short));
						curServer->portNum = addrBuf[1] + (addrBuf[0] << 8);

						if (++curIndex == maxServers)
							break;
						itor++;
					}
				}
				delete[] addrs;
			}
		}
	}
	return err;
}


class QueryFailoverListData
{
public:
	WONIPAddress* serverAddrBuf;
	unsigned short* numServerAddrs;
	HWONCOMPLETION hCompletion;
	unsigned short maxServers;

	void Done(Error err)
	{
		WONComplete(hCompletion, (void*)err);
		delete this;
	}
};

static void DoneQueryFailoverList(const DirEntityListResult& result, QueryFailoverListData* failoverData)
{
	if (result.error == Error_Success)
	{
		*(failoverData->numServerAddrs) = result.entityList->size();
		unsigned short curIndex = 0;
		DirEntityList::iterator itor = result.entityList->begin();
		while (itor != result.entityList->end())
		{
			const unsigned char* addrBuf = (*itor).mNetAddress.data();

			WONIPAddress* curServer = &(failoverData->serverAddrBuf[curIndex]);

			*(in_addr*)(&(curServer->ipAddress)) = *(in_addr*)(addrBuf + sizeof(short));
			curServer->portNum = addrBuf[1] + (addrBuf[0] << 8);

			if (++curIndex == failoverData->maxServers)
				break;
			itor++;
		}
	}
	failoverData->Done(result.error);
}


static void WONQueryServerListFromDirServerAsyncLL(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
												   HWONAUTH hWONAuth, WON_CONST wstring& path,
												   WON_CONST wstring& name, WON_CONST wstring& displayName,
												   long timeout, WONIPAddress* serverAddrBuf,
												   unsigned short* numServerAddrs, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numServerAddrs)
	{
		IPSocket::Address* addrs;
		err = ConvertDLLAddrList(dirServerAddr, numDirServers, addrs);
		if (err == Error_Success)
		{
			err = Error_InvalidParams;
			QueryFailoverListData* failoverData = new QueryFailoverListData;
			if (!failoverData)
				err = Error_OutOfMemory;
			else
			{
				failoverData->serverAddrBuf = serverAddrBuf;
				failoverData->numServerAddrs = numServerAddrs;
				failoverData->hCompletion = hCompletion;
				failoverData->maxServers = *numServerAddrs;
				*numServerAddrs = 0;
				if (failoverData->maxServers && numDirServers && serverAddrBuf)
				{
					DirEntityList entityList;
					err = FindServiceEx((Identity*)hWONAuth, addrs, numDirServers,
									  0, 0, 
									  GF_DECOMPSERVICES | GF_SERVADDNETADDR, FMM_EXACT,
									  FF_MATCHALL, path, name, displayName,
									  0, 0, DataObjectTypeSet(), 0, 0, timeout, true, DoneQueryFailoverList,
									  failoverData);
					delete[] addrs;
					return;
				}
				delete failoverData;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}

/*
WONError WONQueryServerListFromDirServerW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										  HWONAUTH hWONAuth, WON_CONST wchar* path,
										  WON_CONST wchar* name, WON_CONST wchar* displayName,
										  long timeout, WONIPAddress* serverAddrBuf,
										  unsigned short* numServerAddrs)
{
	return WONQueryServerListFromDirServerLL(dirServerAddr, numDirServers, hWONAuth, path, name,
											 displayName, timeout, serverAddrBuf, numServerAddrs);
}


WONError WONQueryServerListFromDirServerA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										  HWONAUTH hWONAuth, WON_CONST char* path, WON_CONST char* name,
										  WON_CONST char* displayName, long timeout,
										  WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs)
{
	return WONQueryServerListFromDirServerLL(dirServerAddr, numDirServers, hWONAuth, StringToWString(path),
		StringToWString(name), StringToWString(displayName), timeout, serverAddrBuf, numServerAddrs);
}


void WONQueryServerListFromDirServerAsyncW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										   HWONAUTH hWONAuth, WON_CONST wchar* path,
										   WON_CONST wchar* name, WON_CONST wchar* displayName,
										   long timeout, WONIPAddress* serverAddrBuf,
										   unsigned short* numServerAddrs, HWONCOMPLETION hCompletion)
{
	WONQueryServerListFromDirServerAsyncLL(dirServerAddr, numDirServers, hWONAuth, path, name,
								displayName, timeout, serverAddrBuf, numServerAddrs, hCompletion);
}


void WONQueryServerListFromDirServerAsyncA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										   HWONAUTH hWONAuth, WON_CONST char* path, WON_CONST char* name,
										   WON_CONST char* displayName, long timeout,
										   WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs,
										   HWONCOMPLETION hCompletion)
{
	WONQueryServerListFromDirServerAsyncLL(dirServerAddr, numDirServers, hWONAuth, StringToWString(path),
		StringToWString(name), StringToWString(displayName), timeout, serverAddrBuf, numServerAddrs, hCompletion);
}
*/

// renamed them.  :/
WONError WONDirQueryServerListW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										  HWONAUTH hWONAuth, WON_CONST wchar_t* path,
										  WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
										  long timeout, WONIPAddress* serverAddrBuf,
										  unsigned short* numServerAddrs)
{
	return WONQueryServerListFromDirServerLL(dirServerAddr, numDirServers, hWONAuth, wstring(path),
		wstring(name), wstring(displayName), timeout, serverAddrBuf, numServerAddrs);
}


WONError WONDirQueryServerListA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										  HWONAUTH hWONAuth, WON_CONST char* path, WON_CONST char* name,
										  WON_CONST char* displayName, long timeout,
										  WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs)
{
	return WONQueryServerListFromDirServerLL(dirServerAddr, numDirServers, hWONAuth, StringToWString(path),
		StringToWString(name), StringToWString(displayName), timeout, serverAddrBuf, numServerAddrs);
}


void WONDirQueryServerListAsyncW(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										   HWONAUTH hWONAuth, WON_CONST wchar_t* path,
										   WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
										   long timeout, WONIPAddress* serverAddrBuf,
										   unsigned short* numServerAddrs, HWONCOMPLETION hCompletion)
{
	WONQueryServerListFromDirServerAsyncLL(dirServerAddr, numDirServers, hWONAuth, wstring(path),
		wstring(name), wstring(displayName), timeout, serverAddrBuf, numServerAddrs, hCompletion);
}


void WONDirQueryServerListAsyncA(WON_CONST WONIPAddress* dirServerAddr, unsigned short numDirServers,
										   HWONAUTH hWONAuth, WON_CONST char* path, WON_CONST char* name,
										   WON_CONST char* displayName, long timeout,
										   WONIPAddress* serverAddrBuf, unsigned short* numServerAddrs,
										   HWONCOMPLETION hCompletion)
{
	WONQueryServerListFromDirServerAsyncLL(dirServerAddr, numDirServers, hWONAuth, StringToWString(path),
		StringToWString(name), StringToWString(displayName), timeout, serverAddrBuf, numServerAddrs, hCompletion);
}


#define MAXSRVRS 25

class QueryServerListData
{
public:
	unsigned int maxServers;
	unsigned int* numServerAddrs;
	unsigned int tmpInt;
	IPSocket::Address* serverAddrBuf;
	IPSocket::Address tmpSrvrAddrBuf[MAXSRVRS];
	CompletionContainer<const QueryServerListResult&> completion;

	void Done(Error err, const IPSocket::Address& fromDirServer)
	{
		completion.Complete(QueryServerListResult(err, fromDirServer, serverAddrBuf, numServerAddrs));
		delete this;
	}
};


static void DoneQueryServerList(const DirEntityListResult& result, QueryServerListData* queryServerListData)
{
	if (result.error == Error_Success)
	{
		*(queryServerListData->numServerAddrs) = result.entityList->size();
		unsigned short curIndex = 0;
		DirEntityList::iterator itor = result.entityList->begin();
		while (itor != result.entityList->end())
		{
			IPSocket::Address* curServer = &(queryServerListData->serverAddrBuf[curIndex]);
			curServer->Set(*itor);

			if (++curIndex == queryServerListData->maxServers)
				break;
			itor++;
		}
	}
	queryServerListData->Done(result.error, result.directoryServer);
}


Error WONAPI::QueryServerList(Identity* identity, const IPSocket::Address* directoryServers,
					  unsigned int numAddrs, const WONCommon::WONString& path, 
					  const WONCommon::WONString& name, const WONCommon::WONString& displayName,
					  const WONCommon::DataObjectTypeSet& dataObjects,
					  IPSocket::Address* serverAddrBuf, unsigned int* numServerAddrs, long timeout,
					  bool async, const CompletionContainer<const QueryServerListResult&>& completion)
{
	unsigned int maxServers = MAXSRVRS;
	if (numServerAddrs)
	{
		maxServers = *numServerAddrs;
		*numServerAddrs = 0;
	}

	if (!numAddrs || !directoryServers || (!numServerAddrs && serverAddrBuf))
	{
		unsigned int tmpInt = 0;
		completion.Complete(QueryServerListResult(Error_InvalidParams, IPSocket::Address(), serverAddrBuf, numServerAddrs ? numServerAddrs : &tmpInt));
		return Error_InvalidParams;
	}
	
	QueryServerListData* queryServerListData = new QueryServerListData();
	if (!queryServerListData)
	{
		unsigned int tmpInt = 0;
		completion.Complete(QueryServerListResult(Error_OutOfMemory, IPSocket::Address(), serverAddrBuf, numServerAddrs ? numServerAddrs : &tmpInt));
		return Error_OutOfMemory;
	}

	queryServerListData->completion = completion;
	queryServerListData->serverAddrBuf = serverAddrBuf ? serverAddrBuf : queryServerListData->tmpSrvrAddrBuf;
	queryServerListData->maxServers = maxServers;
	queryServerListData->tmpInt = 0;
	queryServerListData->numServerAddrs = numServerAddrs ? numServerAddrs : &(queryServerListData->tmpInt);
	queryServerListData->serverAddrBuf = serverAddrBuf;
	
	return FindServiceEx(identity, directoryServers, numAddrs, 0, 0,
				GF_DECOMPSERVICES | GF_SERVADDNETADDR, FMM_EXACT, FF_MATCHALL, path, name, displayName, 
				0, 0, dataObjects, 0, 0, timeout, async, DoneQueryServerList, queryServerListData);
}


HWONDATAOBJECT WONDataObjectCreate(WON_CONST void* type, BYTE typeLen, void* data, unsigned short dataLen)
{
	return (HWONDATAOBJECT)new DataObject(DataObject::DataType((unsigned char*)type, typeLen), DataObject::Data((unsigned char*)data, dataLen));
}


void WONDataObjectGetType(HWONDATAOBJECT dataObj, void* typeBuf, BYTE* typeLen)
{
	if (typeLen)
	{
		const DataObject* obj = (DataObject*)dataObj;
		const DataObject::DataType* type = &(obj->GetDataType());
		
		if (typeBuf && *typeLen)
		{
			if (type->size() > *typeLen)
				memcpy(typeBuf, type->data(), *typeLen);
			else
				memcpy(typeBuf, type->data(), type->size());
		}

		*typeLen = type->size();
	}
}


void WONDataObjectGetData(HWONDATAOBJECT dataObj, void* dataBuf, unsigned short* dataLen)
{
	if (dataLen)
	{
		const DataObject* obj = (DataObject*)dataObj;
		const DataObject::DataType* data = &(obj->GetData());
		
		if (dataBuf && *dataLen)
		{
			if (data->size() > *dataLen)
				memcpy(dataBuf, data->data(), *dataLen);
			else
				memcpy(dataBuf, data->data(), data->size());
		}

		*dataLen = data->size();
	}
}


void WONDataObjectSetType(HWONDATAOBJECT dataObj, WON_CONST void* typeBuf, BYTE typeLen)
{
	DataObject* obj = (DataObject*)dataObj;

	if (!typeBuf || !typeLen)
		obj->SetDataType(DataObject::DataType());
	else
		obj->SetDataType(DataObject::DataType((unsigned char*)typeBuf, typeLen));
}


void WONDataObjectSetData(HWONDATAOBJECT dataObj, WON_CONST void* dataBuf, unsigned short dataLen)
{
	DataObject* obj = (DataObject*)dataObj;

	if (!dataBuf || !dataLen)
		obj->SetDataType(DataObject::Data());
	else
		obj->SetDataType(DataObject::Data((unsigned char*)dataBuf, dataLen));
}


void WONDataObjectSetLifespan(HWONDATAOBJECT dataObj, unsigned long lifeSpan)
{
	DataObject* obj = (DataObject*)dataObj;
	obj->SetLifespan(lifeSpan);
}


unsigned long WONDataObjectGetLifespan(HWONDATAOBJECT dataObj)
{
	DataObject* obj = (DataObject*)dataObj;
	return obj->GetLifespan();
}


BOOL WONDataObjectIsExpired(HWONDATAOBJECT dataObj)
{
	DataObject* obj = (DataObject*)dataObj;
	return obj->IsExpired() ? TRUE : FALSE;
}


void WONDataObjectCloseHandle(HWONDATAOBJECT dataObj)
{
	delete ((DataObject*)dataObj);
}


WONError WONDirRegisterTitanServerW(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									WON_CONST wchar_t* path, WON_CONST wchar_t* displayName,
									BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									unsigned short numDataObjects, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && titanServerAddr && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = RegisterTitanServer(IPSocket::Address(titanServerAddr->ipAddress, titanServerAddr->portNum),
				lengthFieldSize, dirSrvAddrs, numAddrs, path, displayName, uniqueDisplayName ? true : false,
				dataObjs, timeout, false);
		}
	}
	return err;
}


WONError WONDirRegisterTitanServerA(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									WON_CONST char* path, WON_CONST char* displayName,
									BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									unsigned short numDataObjects, long timeout)
{
	return WONDirRegisterTitanServerW(titanServerAddr, lengthFieldSize, directoryServers, numAddrs,
									WONString(path).GetUnicodeCString(),
									WONString(displayName).GetUnicodeCString(), 
									uniqueDisplayName, dataObjects, numDataObjects, timeout);
}


static void TranslateWONCompletionWithError(Error err, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)err);
}


void WONDirRegisterTitanServerAsyncW(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									 WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST wchar_t* path, WON_CONST wchar_t* displayName,
									 BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									 unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && titanServerAddr && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			RegisterTitanServerEx(IPSocket::Address(titanServerAddr->ipAddress, titanServerAddr->portNum),
				lengthFieldSize, dirSrvAddrs, numAddrs, path, displayName, uniqueDisplayName ? true : false,
				dataObjs, timeout, true, TranslateWONCompletionWithError, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRegisterTitanServerAsyncA(WON_CONST WONIPAddress* titanServerAddr, unsigned char lengthFieldSize,
									 WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST char* path, WON_CONST char* displayName,
									 BOOL uniqueDisplayName, HWONDATAOBJECT* dataObjects,
									 unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirRegisterTitanServerAsyncW(titanServerAddr, lengthFieldSize, directoryServers, numAddrs,
		WONString(path).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), uniqueDisplayName, dataObjects,
		numDataObjects, timeout, hCompletion);
}


WONError WONDirAddDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* dirName, WON_CONST wchar_t* displayName,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && dirServerAddr)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(dirServerAddr[i].ipAddress, dirServerAddr[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = AddDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dirName, displayName,
				lifeSpan, uniqueDisplayName ? true : false, dirVisible ? true : false, dataObjs, timeout, false);
		}
	}
	return err;
}


WONError WONDirAddDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							 WON_CONST char* path, WON_CONST char* dirName, WON_CONST char* displayName,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout)
{
	return WONDirAddDirectoryW(hWONAuth, dirServerAddr, numAddrs,
		WONString(path).GetUnicodeCString(),
		WONString(dirName).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), lifeSpan, uniqueDisplayName,
		dirVisible, dataObjects, numDataObjects, timeout);
}


static void TranslateWONCompletionWithDirResult(const DirResult& result, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)result.error);
}


void WONDirAddDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* dirName, WON_CONST wchar_t* displayName,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && dirServerAddr)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(dirServerAddr[i].ipAddress, dirServerAddr[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			AddDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dirName, displayName,
				lifeSpan, uniqueDisplayName ? true : false, dirVisible ? true : false, dataObjs, timeout, true,
				TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirAddDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* dirServerAddr, unsigned short numAddrs,
							  WON_CONST char* path, WON_CONST char* dirName, WON_CONST char* displayName,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion)
{
	WONDirAddDirectoryAsyncW(hWONAuth, dirServerAddr, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(dirName).GetUnicodeCString(), WONString(displayName).GetUnicodeCString(), lifeSpan, uniqueDisplayName,
		dirVisible, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirAddServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							 WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = AddService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, displayName, serviceAddr,
				serviceAddrSize, lifeSpan, uniqueDisplayName ? true : false, overwrite ? true : false, dataObjs,
				timeout, false);
		}
	}
	return err;
}


WONError WONDirAddServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							 WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	return WONDirAddServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), WONString(displayName).GetUnicodeCString(), serviceAddr, serviceAddrSize,
		lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects, timeout);
}


void WONDirAddServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							  WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							  HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			AddServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, displayName, serviceAddr,
				serviceAddrSize, lifeSpan, uniqueDisplayName ? true : false, overwrite ? true : false, dataObjs,
				timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirAddServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							  WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  unsigned long lifeSpan, BOOL uniqueDisplayName, BOOL overwrite,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							  HWONCOMPLETION hCompletion)
{
	WONDirAddServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), WONString(displayName).GetUnicodeCString(), serviceAddr, serviceAddrSize,
		lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirAddServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
						   WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
						   BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	return WONDirAddServiceExW(hWONAuth, directoryServers, numAddrs, path, name, displayName,
		&(srvAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects,
		timeout);
}


WONError WONDirAddServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
						   WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
						   BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	return WONDirAddServiceExA(hWONAuth, directoryServers, numAddrs, path, name, displayName,
		&(srvAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects,
		timeout);
}


void WONDirAddServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST wchar_t* displayName,
							WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName, 
							BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	WONDirAddServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name, displayName,
		&(srvAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects,
		timeout, hCompletion);
}


void WONDirAddServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							WON_CONST char* path, WON_CONST char* name, WON_CONST char* displayName,
							WON_CONST WONIPAddress* serviceAddr, unsigned long lifeSpan, BOOL uniqueDisplayName,
							BOOL overwrite, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	WONDirAddServiceExAsyncA(hWONAuth, directoryServers, numAddrs, path, name, displayName,
		&(srvAddr.Get().sin_port), 6, lifeSpan, uniqueDisplayName, overwrite, dataObjects, numDataObjects,
		timeout, hCompletion);
}


WONError WONDirRenewDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned long lifeSpan, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RenewDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, lifeSpan, timeout, false);
		}
	}
	return err;
}


WONError WONDirRenewDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned long lifeSpan, long timeout)
{
	return WONDirRenewDirectoryW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), lifeSpan, timeout);
}


void WONDirRenewDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RenewDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, lifeSpan, timeout, true,
				TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRenewDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirRenewDirectoryAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), lifeSpan,
		timeout, hCompletion);
}


WONError WONDirRenewServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							   unsigned long lifeSpan, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RenewService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, lifeSpan, timeout, false);
		}
	}
	return err;
}


WONError WONDirRenewServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							   unsigned long lifeSpan, long timeout)
{
	return WONDirRenewServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), 
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, lifeSpan, timeout);
}


void WONDirRenewServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RenewServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, lifeSpan, timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRenewServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirRenewServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), 
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, lifeSpan, timeout, hCompletion);

}


WONError WONDirRenewServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							 unsigned long lifeSpan, long timeout)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	return WONDirRenewServiceExW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port), 6,
		lifeSpan, timeout);
}


WONError WONDirRenewServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							 unsigned long lifeSpan, long timeout)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	return WONDirRenewServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), &(srvAddr.Get().sin_port), 6, lifeSpan, timeout);
}


void WONDirRenewServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	WONDirRenewServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, lifeSpan, timeout, hCompletion);

}


void WONDirRenewServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  unsigned long lifeSpan, long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;
	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	WONDirRenewServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), &(srvAddr.Get().sin_port), 6, lifeSpan, timeout, hCompletion);
}



WONError WONDirModifyDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* newDirName, WON_CONST wchar_t* newDisplayName,
								unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = ModifyDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, newDirName, newDisplayName,
				newLifeSpan, uniqueDisplayName ? true : false, dirVisible ? true : false, dataObjs,
				timeout, false);
		}
	}
	return err;
}


WONError WONDirModifyDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* newDirName, WON_CONST char* newDisplayName,
								unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	return WONDirModifyDirectoryW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(newDirName).GetUnicodeCString(), WONString(newDisplayName).GetUnicodeCString(),
		newLifeSpan, uniqueDisplayName, dirVisible, dataObjects, numDataObjects, timeout);
}


void WONDirModifyDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* newDirName, WON_CONST wchar_t* newDisplayName,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			ModifyDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, newDirName, newDisplayName,
				newLifeSpan, uniqueDisplayName ? true : false, dirVisible ? true : false, dataObjs,
				timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirModifyDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* newDirName, WON_CONST char* newDisplayName,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName, BOOL dirVisible, 
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion)
{
	WONDirModifyDirectoryAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(newDirName).GetUnicodeCString(), WONString(newDisplayName).GetUnicodeCString(), newLifeSpan, uniqueDisplayName,
		dirVisible, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirModifyServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName,
								WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								unsigned long newLifeSpan, BOOL uniqueDisplayName,
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = ModifyService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, newName, newDisplayName, newServiceAddr, newServiceAddrSize, newLifeSpan,
				uniqueDisplayName ? true : false, dataObjs, timeout, false);
		}
	}
	return err;
}


WONError WONDirModifyServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST char* newName, WON_CONST char* newDisplayName,
								WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								unsigned long newLifeSpan, BOOL uniqueDisplayName,
								HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	return WONDirModifyServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, WONString(newName).GetUnicodeCString(), 
		WONString(newDisplayName).GetUnicodeCString(), newServiceAddr, newServiceAddrSize, newLifeSpan,
		uniqueDisplayName, dataObjects, numDataObjects, timeout);
}


void WONDirModifyServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName,
								 WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName,
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			ModifyServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, newName, newDisplayName, newServiceAddr, newServiceAddrSize, newLifeSpan,
				uniqueDisplayName ? true : false, dataObjs, timeout, true, TranslateWONCompletionWithDirResult, 
				hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirModifyServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST char* newName, WON_CONST char* newDisplayName,
								 WON_CONST void* newServiceAddr, unsigned short newServiceAddrSize,
								 unsigned long newLifeSpan, BOOL uniqueDisplayName,
								 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
								 HWONCOMPLETION hCompletion)
{
	WONDirModifyServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, WONString(newName).GetUnicodeCString(),
		WONString(newDisplayName).GetUnicodeCString(), newServiceAddr, newServiceAddrSize, newLifeSpan,
		uniqueDisplayName, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirModifyServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							  unsigned long newLifeSpan, BOOL uniqueDisplayName,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	IPSocket::Address srvAddr;
	IPSocket::Address newSrvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	if (newServiceAddr)
		newSrvAddr.Set(newServiceAddr->ipAddress, newServiceAddr->portNum);

	return WONDirModifyServiceExW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, newName, newDisplayName, &(newSrvAddr.Get().sin_port), 6,
		newLifeSpan, uniqueDisplayName, dataObjects, numDataObjects, timeout);
}


WONError WONDirModifyServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST char* newName, WON_CONST char* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							  unsigned long newLifeSpan, BOOL uniqueDisplayName,
							  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	return WONDirModifyServiceW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, WONString(newName).GetUnicodeCString(),
		WONString(newDisplayName).GetUnicodeCString(), newServiceAddr, newLifeSpan,
		uniqueDisplayName, dataObjects, numDataObjects, timeout);
}


void WONDirModifyServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST wchar_t* newName, WON_CONST wchar_t* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							   unsigned long newLifeSpan, BOOL uniqueDisplayName,
							   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							   HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;
	IPSocket::Address newSrvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);
	if (newServiceAddr)
		newSrvAddr.Set(newServiceAddr->ipAddress, newServiceAddr->portNum);

	WONDirModifyServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, newName, newDisplayName, &(newSrvAddr.Get().sin_port),
		6, newLifeSpan, uniqueDisplayName, dataObjects, numDataObjects, timeout, hCompletion);

}


void WONDirModifyServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST char* newName, WON_CONST char* newDisplayName, WON_CONST WONIPAddress* newServiceAddr,
							   unsigned long newLifeSpan, BOOL uniqueDisplayName,
							   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
							   HWONCOMPLETION hCompletion)
{
	WONDirModifyServiceAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, WONString(newName).GetUnicodeCString(),
		WONString(newDisplayName).GetUnicodeCString(), newServiceAddr, newLifeSpan,
		uniqueDisplayName, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirRenameDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RenameDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, newDisplayName,
				uniqueDisplayName ? true : false, timeout, false);
		}
	}
	return err;
}


WONError WONDirRenameDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	return WONDirRenameDirectoryW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(newDisplayName).GetUnicodeCString(), uniqueDisplayName, timeout);
}


void WONDirRenameDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RenameDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, newDisplayName,
				uniqueDisplayName ? true : false, timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRenameDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion)
{
	WONDirRenameDirectoryAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(newDisplayName).GetUnicodeCString(), uniqueDisplayName, timeout, hCompletion);

}



WONError WONDirRenameServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RenameService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, newDisplayName, uniqueDisplayName ? true : false, timeout, false);
		}
	}
	return err;
}


WONError WONDirRenameServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	return WONDirRenameServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, WONString(newDisplayName).GetUnicodeCString(),
		uniqueDisplayName, timeout);
}


void WONDirRenameServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RenameServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, newDisplayName, uniqueDisplayName ? true : false, timeout, true,
				TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRenameServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
								 HWONCOMPLETION hCompletion)
{
	WONDirRenameServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, WONString(newDisplayName).GetUnicodeCString(),
		uniqueDisplayName, timeout, hCompletion);
}


WONError WONDirRenameServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirRenameServiceExW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port), 6,
		newDisplayName, uniqueDisplayName, timeout);
}


WONError WONDirRenameServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							  WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout)
{
	return WONDirRenameServiceW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, WONString(newDisplayName).GetUnicodeCString(), uniqueDisplayName,
		timeout);
}


void WONDirRenameServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST wchar_t* newDisplayName, BOOL uniqueDisplayName, long timeout,
							   HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirRenameServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port),
		6, newDisplayName, uniqueDisplayName, timeout, hCompletion);
}


void WONDirRenameServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
							   WON_CONST char* newDisplayName, BOOL uniqueDisplayName, long timeout,
							   HWONCOMPLETION hCompletion)
{
	WONDirRenameServiceAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, WONString(newDisplayName).GetUnicodeCString(), uniqueDisplayName,
		timeout, hCompletion);
}


WONError WONDirRemoveDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RemoveDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, timeout, false);
		}
	}
	return err;
}


WONError WONDirRemoveDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, long timeout)
{
	return WONDirRemoveDirectoryW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), timeout);
}


void WONDirRemoveDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RemoveDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, timeout, true,
				TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRemoveDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirRemoveDirectoryAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		timeout, hCompletion);
}


WONError WONDirRemoveServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = RemoveService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, timeout, false);
		}
	}
	return err;
}


WONError WONDirRemoveServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								long timeout)
{
	return WONDirRemoveServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), 
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, timeout);
}


void WONDirRemoveServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			RemoveServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirRemoveServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
								 long timeout, HWONCOMPLETION hCompletion)
{
	WONDirRemoveServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(), 
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, timeout, hCompletion);
}


WONError WONDirRemoveServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr, long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirRemoveServiceExW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, timeout);
}


WONError WONDirRemoveServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr, long timeout)
{
	return WONDirRemoveServiceW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, timeout);
}


void WONDirRemoveServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr, long timeout,
							   HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirRemoveServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port),
		6, timeout, hCompletion);
}


void WONDirRemoveServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr, long timeout,
							   HWONCOMPLETION hCompletion)
{
	WONDirRemoveServiceAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, timeout, hCompletion);
}


WONError WONDirGetNumDirEntitiesW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								  WON_CONST wchar_t* path, unsigned short* numEntities, WONDirGetNumDirEntitiesMode mode,
								  long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = GetNumDirEntities((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				(GetNumDirEntitiesMode)mode, timeout, false);
		}
	}
	return err;
}


WONError WONDirGetNumDirEntitiesA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								  WON_CONST char* path, unsigned short* numEntities, WONDirGetNumDirEntitiesMode mode,
								  long timeout)
{
	return WONDirGetNumDirEntitiesW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, mode, timeout);
}



static void TranslateWONCompletionWithDirGetNumResult(const DirGetNumResult& result, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)result.error);
}


void WONDirGetNumDirEntitiesAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								   WON_CONST wchar_t* path, unsigned short* numEntities, WONDirGetNumDirEntitiesMode mode,
								   long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			GetNumDirEntitiesEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				(GetNumDirEntitiesMode)mode, timeout, true, TranslateWONCompletionWithDirGetNumResult,
				hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetNumDirEntitiesAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								   WON_CONST char* path, unsigned short* numEntities, WONDirGetNumDirEntitiesMode mode,
								   long timeout, HWONCOMPLETION hCompletion)
{
	WONDirGetNumDirEntitiesAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, mode, timeout, hCompletion);
}


WONError WONDirGetNumSubDirsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, unsigned short* numEntities, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = GetNumSubDirs((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				timeout, false);
		}
	}
	return err;
}


WONError WONDirGetNumSubDirsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, unsigned short* numEntities, long timeout)
{
	return WONDirGetNumSubDirsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, timeout);
}


void WONDirGetNumSubDirsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned short* numEntities, long timeout,
							   HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			GetNumSubDirsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				timeout, true, TranslateWONCompletionWithDirGetNumResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetNumSubDirsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned short* numEntities, long timeout,
							   HWONCOMPLETION hCompletion)
{
	WONDirGetNumSubDirsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, timeout, hCompletion);
}


WONError WONDirGetNumServicesW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST wchar_t* path, unsigned short* numEntities, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			err = GetNumServices((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				timeout, false);
		}
	}
	return err;
}


WONError WONDirGetNumServicesA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   WON_CONST char* path, unsigned short* numEntities, long timeout)
{
	return WONDirGetNumServicesW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, timeout);
}


void WONDirGetNumServicesAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST wchar_t* path, unsigned short* numEntities, long timeout,
								HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			GetNumServicesEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, numEntities,
				timeout, true, TranslateWONCompletionWithDirGetNumResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetNumServicesAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
								WON_CONST char* path, unsigned short* numEntities, long timeout,
								HWONCOMPLETION hCompletion)
{
	WONDirGetNumServicesAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		numEntities, timeout, hCompletion);
}


WONError WONDirClearDirectoryDataObjectsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										  long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = ClearDirectoryDataObjects((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dataObjs,
				timeout, false);
		}
	}
	return err;
}


WONError WONDirClearDirectoryDataObjectsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										  long timeout)
{
	return WONDirClearDirectoryDataObjectsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		dataObjects, numDataObjects, timeout);
}


void WONDirClearDirectoryDataObjectsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										   long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			ClearDirectoryDataObjectsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dataObjs,
				timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirClearDirectoryDataObjectsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										   long timeout, HWONCOMPLETION hCompletion)
{
	WONDirClearDirectoryDataObjectsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirClearServiceDataObjectsExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										  unsigned short numDataObjects, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = ClearServiceDataObjects((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name,
				serviceAddr, serviceAddrSize, dataObjs, timeout, false);
		}
	}
	return err;
}


WONError WONDirClearServiceDataObjectsExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										  WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										  unsigned short numDataObjects, long timeout)
{
	return WONDirClearServiceDataObjectsExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects, timeout);
}


void WONDirClearServiceDataObjectsExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										   unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			ClearServiceDataObjectsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name,
				serviceAddr, serviceAddrSize, dataObjs, timeout, true, TranslateWONCompletionWithDirResult,
				hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirClearServiceDataObjectsExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										   WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										   unsigned short numDataObjects, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirClearServiceDataObjectsExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects, timeout,
		hCompletion);
}


WONError WONDirClearServiceDataObjectsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
										HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirClearServiceDataObjectsExW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, dataObjects, numDataObjects, timeout);
}


WONError WONDirClearServiceDataObjectsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
										HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout)
{
	return WONDirClearServiceDataObjectsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, timeout);
}


void WONDirClearServiceDataObjectsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
										 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
										 HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirClearServiceDataObjectsExAsyncW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, dataObjects, numDataObjects, timeout, hCompletion);
}


void WONDirClearServiceDataObjectsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
										 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects, long timeout,
										 HWONCOMPLETION hCompletion)
{
	WONDirClearServiceDataObjectsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, timeout, hCompletion);
}


WONError WONDirSetDirectoryDataObjectsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										WONDirDataObjectSetMode mode, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = SetDirectoryDataObjects((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dataObjs,
				(DirG2DataObjectSetMode)mode, timeout, false);
		}
	}
	return err;
}


WONError WONDirSetDirectoryDataObjectsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										WONDirDataObjectSetMode mode, long timeout)
{
	return WONDirSetDirectoryDataObjectsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		dataObjects, numDataObjects, mode, timeout);
}


void WONDirSetDirectoryDataObjectsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										 WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			SetDirectoryDataObjectsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, dataObjs,
				(DirG2DataObjectSetMode)mode, timeout, true, TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirSetDirectoryDataObjectsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
										 WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirSetDirectoryDataObjectsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		dataObjects, numDataObjects, mode, timeout, hCompletion);
}



WONError WONDirSetServiceDataObjectsExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										unsigned short numDataObjects, WONDirDataObjectSetMode mode, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = SetServiceDataObjects((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, dataObjs, (DirG2DataObjectSetMode)mode, timeout, false);
		}
	}
	return err;
}


WONError WONDirSetServiceDataObjectsExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										unsigned short numDataObjects, WONDirDataObjectSetMode mode, long timeout)
{
	return WONDirSetServiceDataObjectsExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects, mode, timeout);
}


void WONDirSetServiceDataObjectsExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr,
										 unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										 unsigned short numDataObjects, WONDirDataObjectSetMode mode, long timeout,
										 HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (!dirSrvAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			SetServiceDataObjectsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name, serviceAddr,
				serviceAddrSize, dataObjs, (DirG2DataObjectSetMode)mode, timeout, true,
				TranslateWONCompletionWithDirResult, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirSetServiceDataObjectsExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
										 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr,
										 unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
										 unsigned short numDataObjects, WONDirDataObjectSetMode mode, long timeout,
										 HWONCOMPLETION hCompletion)
{
	WONDirSetServiceDataObjectsExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects, mode, timeout,
		hCompletion);
}


WONError WONDirSetServiceDataObjectsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
									  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									  WONDirDataObjectSetMode mode, long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirSetServiceDataObjectsExW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, dataObjects, numDataObjects, mode, timeout);
}


WONError WONDirSetServiceDataObjectsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
									  HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									  WONDirDataObjectSetMode mode, long timeout)
{
	return WONDirSetServiceDataObjectsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, mode, timeout);
}


void WONDirSetServiceDataObjectsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
									   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									   WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirSetServiceDataObjectsExAsyncW(hWONAuth, directoryServers, numAddrs, path, name,
		&(srvAddr.Get().sin_port), 6, dataObjects, numDataObjects, mode, timeout, hCompletion);
}


void WONDirSetServiceDataObjectsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
									   HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
									   WONDirDataObjectSetMode mode, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirSetServiceDataObjectsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, mode, timeout, hCompletion);
}


void WONDirEntityCloseHandle(HWONDIRENTITY hWONDirEntity)
{
	delete (DirEntity*)hWONDirEntity;
}


char WONDirEntityGetType(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return (char)dirEntity->mType;
}


BOOL WONDirEntityIsService(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return dirEntity->mType == DirEntity::ET_SERVICE;
}


BOOL WONDirEntityIsDirectory(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return dirEntity->mType == DirEntity::ET_DIRECTORY;
}


void WONDirEntityGetPathA(HWONDIRENTITY hWONDirEntity, char* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = dirEntity->mPath.size() + 1;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, WStringToString(dirEntity->mPath).c_str(), copySize);
		}
	}
}


void WONDirEntityGetPathW(HWONDIRENTITY hWONDirEntity, wchar_t* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = dirEntity->mPath.size();
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize - 1)
				copySize = bufSize - 1;
			memcpy(strBuf, dirEntity->mPath.data(), copySize * sizeof(wchar_t));
			strBuf[copySize] = 0;
		}
	}
}


void WONDirEntityGetFullPathA(HWONDIRENTITY hWONDirEntity, char* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		wstring fullPath(dirEntity->GetFullPath());
		unsigned long copySize = fullPath.size() + 1;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, WStringToString(fullPath).c_str(), copySize);
		}
	}
}


void WONDirEntityGetFullPathW(HWONDIRENTITY hWONDirEntity, wchar_t* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		wstring fullPath(dirEntity->GetFullPath());
		unsigned long copySize = fullPath.size();
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize - 1)
				copySize = bufSize - 1;
			memcpy(strBuf, fullPath.data(), copySize * sizeof(wchar_t));
			strBuf[copySize] = 0;
		}
	}
}


void WONDirEntityGetNameA(HWONDIRENTITY hWONDirEntity, char* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = dirEntity->mName.size() + 1;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, WStringToString(dirEntity->mName).c_str(), copySize);
		}
	}
}


void WONDirEntityGetNameW(HWONDIRENTITY hWONDirEntity, wchar_t* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = (dirEntity->mName.size() + 1) * 2;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, dirEntity->mName.data(), copySize);
			strBuf[(copySize - 1) / 2] = 0;
		}
	}
}


void WONDirEntityGetDisplayNameA(HWONDIRENTITY hWONDirEntity, char* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = dirEntity->mDisplayName.size() + 1;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, WStringToString(dirEntity->mDisplayName).c_str(), copySize);
		}
	}
}


void WONDirEntityGetDisplayNameW(HWONDIRENTITY hWONDirEntity, wchar_t* strBuf, unsigned short* strBufLen)
{
	if (strBufLen)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *strBufLen;
		unsigned long copySize = (dirEntity->mDisplayName.size() + 1) * 2;
		*strBufLen = copySize;
		if (strBuf && *strBufLen)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(strBuf, dirEntity->mDisplayName.data(), copySize);
			strBuf[(copySize - 1) / 2] = 0;
		}
	}
}


time_t WONDirEntityGetCreationTime(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return (time_t)dirEntity->mCreated;
}


time_t WONDirEntityGetTouchTime(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return (time_t)dirEntity->mTouched;
}


unsigned long WONDirEntityGetLifespan(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return dirEntity->mLifespan;
}


BOOL WONDirEntityIsVisible(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return dirEntity->mVisible ? TRUE : FALSE;
}


void WONDirEntityGetAddress(HWONDIRENTITY hWONDirEntity, WONIPAddress* addr)
{
	if (addr)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		IPSocket::Address tmpAddr(*dirEntity);
		addr->ipAddress = *(unsigned long*)&(tmpAddr.GetAddress());
		addr->portNum = tmpAddr.GetPort();
	}
}


void WONDirEntityGetAddressEx(HWONDIRENTITY hWONDirEntity, void* addrBuf, unsigned short* addrBufSize)
{
	if (addrBufSize)
	{
		DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
		unsigned short bufSize = *addrBufSize;
		unsigned long copySize = dirEntity->mNetAddress.size();
		*addrBufSize = copySize;
		if (addrBuf && *addrBufSize)
		{
			if (copySize > bufSize)
				copySize = bufSize;
			memcpy(addrBuf, dirEntity->mNetAddress.data(), copySize);
		}
	}
}


unsigned short WONDirEntityGetNumDataObjects(HWONDIRENTITY hWONDirEntity)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;
	return dirEntity->mDataObjects.size();
}


HWONDATAOBJECT WONDirEntityGetDataObject(HWONDIRENTITY hWONDirEntity, unsigned short index, BOOL copy)
{
	DirEntity* dirEntity = (DirEntity*)hWONDirEntity;

	DataObjectTypeSet::iterator itor = dirEntity->mDataObjects.begin();

	unsigned short i = index;
	while (i)
	{
		itor++;
		i--;
	}
	if (copy)
		return (HWONDATAOBJECT)new DataObject(*itor);
	return (HWONDATAOBJECT)&*itor;
}


class WONDirEntityCallbackData
{
public:
	WONDirEntityCallback callback;
	HWONDIRENTITY* entityBuf;
	unsigned short* entityBufLen;
	HWONCOMPLETION hCompletion;
	void* callbackPrivData;
};


static void TranslateDirEntityCallback(const WONMsg::DirEntity& dirEntity, void* callbackPrivData)
{
	WONDirEntityCallbackData* callbackData = (WONDirEntityCallbackData*)callbackPrivData;

	callbackData->callback((HWONDIRENTITY)&dirEntity, callbackData->callbackPrivData);
}


WONError WONDirGetDirectoryContentsW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									 WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				auto_ptr<WONDirEntityCallbackData> autoCallbackData(callbackData);

				callbackData->callback = callback;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				DirEntityList result;

				err = GetDirectoryContents((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, &result,
					TranslateDirEntityCallback, callbackData, timeout, false);

				if (err == Error_Success && entityBuf && entityBufLen && *entityBufLen)
				{
					unsigned short curEntity = 0;
					DirEntityList::iterator itor = result.begin();
					while (itor != result.end())
					{
						entityBuf[curEntity] = (HWONDIRENTITY)new DirEntity(*itor);
						itor++;
						if (++curEntity == *entityBufLen)
							break;
					}
					*entityBufLen = result.size();
				}
			}
		}
	}
	return err;
}


WONError WONDirGetDirectoryContentsA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									 WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									 WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	return WONDirGetDirectoryContentsW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		entityBuf, entityBufLen, callback, callbackPrivData, timeout);
}


static void TranslateWONCompletionWithDirEntityListResult(const DirEntityListResult& result, WONDirEntityCallbackData* callbackData)
{
	if (result.error == Error_Success && callbackData->entityBuf && callbackData->entityBufLen && *(callbackData->entityBufLen))
	{
		unsigned short curEntity = 0;
		DirEntityList::iterator itor = result.entityList->begin();
		while (itor != result.entityList->end())
		{
			callbackData->entityBuf[curEntity] = (HWONDIRENTITY)new DirEntity(*itor);
			itor++;
			if (++curEntity == *(callbackData->entityBufLen))
				break;
		}
		*(callbackData->entityBufLen) = result.entityList->size();
	}

	WONComplete(callbackData->hCompletion, (void*)result.error);
	delete callbackData;
}


void WONDirGetDirectoryContentsAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
									  HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				callbackData->hCompletion = hCompletion;
				callbackData->callback = callback;
				callbackData->entityBuf = entityBuf;
				callbackData->entityBufLen = entityBufLen;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				GetDirectoryContentsEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, 0,
					TranslateDirEntityCallback, callbackData, timeout, true, TranslateWONCompletionWithDirEntityListResult,
					callbackData);
				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetDirectoryContentsAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
									  WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
									  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
									  HWONCOMPLETION hCompletion)
{
	WONDirGetDirectoryContentsAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		entityBuf, entityBufLen, callback, callbackPrivData, timeout, hCompletion);
}


WONError WONDirGetDirectoryW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							 long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				auto_ptr<WONDirEntityCallbackData> autoCallbackData(callbackData);

				callbackData->callback = callback;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				DataObjectTypeSet dataObjs;
				if (dataObjects && numDataObjects)
				{
					for (int i2 = 0; i2 < numDataObjects; i2++)
						dataObjs.insert(*((DataObject**)dataObjects)[i2]);
				}

				DirEntityList result;

				err = GetDirectory((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, &result,
					getFlags, dataObjs, TranslateDirEntityCallback, callbackData, timeout, false);

				if (err == Error_Success && entityBuf && entityBufLen && *entityBufLen)
				{
					unsigned short curEntity = 0;
					DirEntityList::iterator itor = result.begin();
					while (itor != result.end())
					{
						entityBuf[curEntity] = (HWONDIRENTITY)new DirEntity(*itor);
						itor++;
						if (++curEntity == *entityBufLen)
							break;
					}
					*entityBufLen = result.size();
				}
			}
		}
	}
	return err;
}


WONError WONDirGetDirectoryA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							 long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	return WONDirGetDirectoryW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		entityBuf, entityBufLen, getFlags, dataObjects, numDataObjects, callback, callbackPrivData, timeout);
}


void WONDirGetDirectoryAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							  long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							  HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				callbackData->hCompletion = hCompletion;
				callbackData->callback = callback;
				callbackData->entityBuf = entityBuf;
				callbackData->entityBufLen = entityBufLen;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				DataObjectTypeSet dataObjs;
				if (dataObjects && numDataObjects)
				{
					for (int i2 = 0; i2 < numDataObjects; i2++)
						dataObjs.insert(*((DataObject**)dataObjects)[i2]);
				}

				GetDirectoryEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, 0,
					getFlags, dataObjs, TranslateDirEntityCallback, callbackData, timeout, true,
					TranslateWONCompletionWithDirEntityListResult, callbackData);

				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetDirectoryAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, HWONDIRENTITY* entityBuf, unsigned short* entityBufLen,
							  long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							  HWONCOMPLETION hCompletion)
{
	WONDirGetDirectoryAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		entityBuf, entityBufLen, getFlags, dataObjects, numDataObjects, callback, callbackPrivData,
		timeout, hCompletion);
}


WONError WONDirGetServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			DirEntity* resultEntity = 0;
			if (result)
			{
				resultEntity = new DirEntity();
				if (!resultEntity)
					return err;
				*result = (HWONDIRENTITY)resultEntity;
			}

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			err = GetService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name,
				serviceAddr, serviceAddrSize, resultEntity, getFlags, dataObjs, timeout, false);
			if (err != Error_Success)
				delete resultEntity;
		}
	}
	return err;
}


WONError WONDirGetServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							 HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 long timeout)
{
	return WONDirGetServiceExW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, result, getFlags, dataObjects,
		numDataObjects, timeout);
}


static void TranslateWONCompletionWithDirEntityResult(const DirEntityResult& result, pair<HWONCOMPLETION, DirEntity*> p)
{
	if (result.error != Error_Success)
		delete p.second;
	WONComplete(p.first, (void*)result.error);
}


void WONDirGetServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			DirEntity* resultEntity = 0;
			if (result)
			{
				resultEntity = new DirEntity();
				if (!resultEntity)
				{
					WONComplete(hCompletion, (void*)err);
					return;
				}
				*result = (HWONDIRENTITY)resultEntity;
			}

			for (int i = 0; i < numAddrs; i++)
				dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

			DataObjectTypeSet dataObjs;
			if (dataObjects && numDataObjects)
			{
				for (int i2 = 0; i2 < numDataObjects; i2++)
					dataObjs.insert(*((DataObject**)dataObjects)[i2]);
			}

			GetServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, path, name,
				serviceAddr, serviceAddrSize, resultEntity, getFlags, dataObjs, timeout, true,
				TranslateWONCompletionWithDirEntityResult, pair<HWONCOMPLETION, DirEntity*>(hCompletion, resultEntity));
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirGetServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  WON_CONST char* path, WON_CONST char* name, WON_CONST void* serviceAddr, unsigned short serviceAddrSize,
							  HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							  long timeout, HWONCOMPLETION hCompletion)
{
	WONDirGetServiceExAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, serviceAddrSize, result, getFlags, dataObjects,
		numDataObjects, timeout, hCompletion);
}


WONError WONDirGetServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
						   HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirGetServiceExW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port), 6,
		result, getFlags, dataObjects, numDataObjects, timeout);
}


WONError WONDirGetServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						   WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
						   HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						   long timeout)
{
	return WONDirGetServiceW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, result, getFlags, dataObjects, numDataObjects, timeout);
}


void WONDirGetServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						    WON_CONST wchar_t* path, WON_CONST wchar_t* name, WON_CONST WONIPAddress* serviceAddr,
						    HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						    long timeout, HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirGetServiceExAsyncW(hWONAuth, directoryServers, numAddrs, path, name, &(srvAddr.Get().sin_port), 6,
		result, getFlags, dataObjects, numDataObjects, timeout, hCompletion);
}


void WONDirGetServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
						    WON_CONST char* path, WON_CONST char* name, WON_CONST WONIPAddress* serviceAddr,
						    HWONDIRENTITY* result, long getFlags, HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
						    long timeout, HWONCOMPLETION hCompletion)
{
	WONDirGetServiceAsyncW(hWONAuth, directoryServers, numAddrs, WONString(path).GetUnicodeCString(),
		WONString(name).GetUnicodeCString(), serviceAddr, result, getFlags, dataObjects, numDataObjects,
		timeout, hCompletion);
}


WONError WONDirFindServiceExW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							  unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							  WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST void* serviceAddr,
							  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							  unsigned short numDataObjects, WONDirEntityCallback callback,
							  void* callbackPrivData, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				auto_ptr<WONDirEntityCallbackData> autoCallbackData(callbackData);

				callbackData->callback = callback;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				DataObjectTypeSet dataObjs;
				if (dataObjects && numDataObjects)
				{
					for (int i2 = 0; i2 < numDataObjects; i2++)
						dataObjs.insert(*((DataObject**)dataObjects)[i2]);
				}

				DirEntityList result;

				err = FindService((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, &result, getFlags,
					findMatchMode, findFlags, path, name, displayName, serviceAddr, serviceAddrSize, dataObjs,
					TranslateDirEntityCallback, callbackData, timeout, false);

				if (err == Error_Success && entityBuf && entityBufLen && *entityBufLen)
				{
					unsigned short curEntity = 0;
					DirEntityList::iterator itor = result.begin();
					while (itor != result.end())
					{
						entityBuf[curEntity] = (HWONDIRENTITY)new DirEntity(*itor);
						itor++;
						if (++curEntity == *entityBufLen)
							break;
					}
					*entityBufLen = result.size();
				}
			}
		}
	}
	return err;
}


WONError WONDirFindServiceExA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							  HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							  unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							  WON_CONST char* name, WON_CONST char* displayName, WON_CONST void* serviceAddr,
							  unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							  unsigned short numDataObjects, WONDirEntityCallback callback,
							  void* callbackPrivData, long timeout)
{
	return WONDirFindServiceExW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen,
		getFlags, findMatchMode, findFlags, WONString(path).GetUnicodeCString(), WONString(name).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects,
		callback, callbackPrivData, timeout);
}


void WONDirFindServiceExAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							   unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							   WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST void* serviceAddr,
							   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							   unsigned short numDataObjects, WONDirEntityCallback callback,
							   void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && directoryServers)
	{
		err = Error_OutOfMemory;
		IPSocket::Address* dirSrvAddrs = new IPSocket::Address[numAddrs];
		if (dirSrvAddrs)
		{
			array_auto_ptr<IPSocket::Address> autoDirSrvAddrs(dirSrvAddrs);

			WONDirEntityCallbackData* callbackData = new WONDirEntityCallbackData;
			if (callbackData)
			{
				callbackData->hCompletion = hCompletion;
				callbackData->callback = callback;
				callbackData->entityBuf = entityBuf;
				callbackData->entityBufLen = entityBufLen;
				callbackData->callbackPrivData = callbackPrivData;

				for (int i = 0; i < numAddrs; i++)
					dirSrvAddrs[i].Set(directoryServers[i].ipAddress, directoryServers[i].portNum);

				DataObjectTypeSet dataObjs;
				if (dataObjects && numDataObjects)
				{
					for (int i2 = 0; i2 < numDataObjects; i2++)
						dataObjs.insert(*((DataObject**)dataObjects)[i2]);
				}

				FindServiceEx((Identity*)hWONAuth, dirSrvAddrs, numAddrs, 0, 0, getFlags,
					findMatchMode, findFlags, path, name, displayName, serviceAddr, serviceAddrSize, dataObjs,
					TranslateDirEntityCallback, callbackData, timeout, true,
					TranslateWONCompletionWithDirEntityListResult, callbackData);
				return;
			}
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONDirFindServiceExAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							   HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags, 
							   unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							   WON_CONST char* name, WON_CONST char* displayName, WON_CONST void* serviceAddr,
							   unsigned short serviceAddrSize, HWONDATAOBJECT* dataObjects,
							   unsigned short numDataObjects, WONDirEntityCallback callback,
							   void* callbackPrivData, long timeout, HWONCOMPLETION hCompletion)
{
	WONDirFindServiceExAsyncW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen, getFlags, 
		findMatchMode, findFlags, WONString(path).GetUnicodeCString(), WONString(name).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), serviceAddr, serviceAddrSize, dataObjects, numDataObjects,
		callback, callbackPrivData, timeout, hCompletion);
}


WONError WONDirFindServiceW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST WONIPAddress* serviceAddr,
							HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	return WONDirFindServiceExW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen, getFlags, 
		findMatchMode, findFlags, path, name, displayName, &(srvAddr.Get().sin_port), 6, dataObjects,
		numDataObjects, callback, callbackPrivData, timeout);
}


WONError WONDirFindServiceA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							WON_CONST char* name, WON_CONST char* displayName, WON_CONST WONIPAddress* serviceAddr,
							HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							WONDirEntityCallback callback, void* callbackPrivData, long timeout)
{
	return WONDirFindServiceW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen, getFlags,
		findMatchMode, findFlags, WONString(path).GetUnicodeCString(), WONString(name).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, callback,
		callbackPrivData, timeout);
}


void WONDirFindServiceAsyncW(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							 unsigned char findMatchMode, unsigned char findFlags, WON_CONST wchar_t* path,
							 WON_CONST wchar_t* name, WON_CONST wchar_t* displayName, WON_CONST WONIPAddress* serviceAddr,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							 HWONCOMPLETION hCompletion)
{
	IPSocket::Address srvAddr;

	if (serviceAddr)
		srvAddr.Set(serviceAddr->ipAddress, serviceAddr->portNum);

	WONDirFindServiceExAsyncW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen, getFlags,
		findMatchMode, findFlags, path, name, displayName, &(srvAddr.Get().sin_port), 6, dataObjects,
		numDataObjects, callback, callbackPrivData, timeout, hCompletion);
}


void WONDirFindServiceAsyncA(HWONAUTH hWONAuth, WON_CONST WONIPAddress* directoryServers, unsigned int numAddrs,
							 HWONDIRENTITY* entityBuf, unsigned short* entityBufLen, long getFlags,
							 unsigned char findMatchMode, unsigned char findFlags, WON_CONST char* path,
							 WON_CONST char* name, WON_CONST char* displayName, WON_CONST WONIPAddress* serviceAddr,
							 HWONDATAOBJECT* dataObjects, unsigned short numDataObjects,
							 WONDirEntityCallback callback, void* callbackPrivData, long timeout,
							 HWONCOMPLETION hCompletion)
{
	WONDirFindServiceAsyncW(hWONAuth, directoryServers, numAddrs, entityBuf, entityBufLen, getFlags,
		findMatchMode, findFlags, WONString(path).GetUnicodeCString(), WONString(name).GetUnicodeCString(),
		WONString(displayName).GetUnicodeCString(), serviceAddr, dataObjects, numDataObjects, callback,
		callbackPrivData, timeout, hCompletion);
}



