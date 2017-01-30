#ifndef ROUTINGAPI_H
#define ROUTINGAPI_H

#include <set>
#include <string>
#include <vector>
#include "Socket/WONWS.h"
#include "common/won.h"
#include "SDKCommon/Completion.h"
#include "Socket/TCPSocket.h"
#include "Authentication/AuthSocket.h"
#include "msg/Routing/RoutingServerTypes.h"
#include "msg/Routing/MMsgRoutingGetClientList.h"

// forwards
namespace WONMsg {
	class BaseMessage;
	class MMsgCommGetNumUsersReply;
	class MMsgRoutingCreateGroupReply;
	class MMsgRoutingGetClientListReply;
	class MMsgRoutingGetGroupListReply;
	class MMsgRoutingGetSimpleClientListReply;
	class MMsgRoutingPeerChat;
	class MMsgRoutingReadDataObjectReply;
	class MMsgRoutingRegisterClientReply;
};

namespace WONAPI {
	
static const WONCommon::RawBuffer gEmptyRawBuffer;
static const std::wstring         gEmptyWString;

	class RoutingServerClient {
	public:
#include "RoutingAPITypes.h"

		RoutingServerClient(bool useTCP =true, bool enableShortcuts =false, bool activateIgnoring =false, unsigned char theLengthFieldSize =2);
		virtual ~RoutingServerClient();

		Error Connect(const TCPSocket::Address& theRoutingAddress, Identity* theIdentityP =NULL, bool isReconnectAttempt =false, long theTimeout =-1, bool async = false, const CompletionContainer<Error>& completion = DEFAULT_COMPLETION );
		void Close();

		// Registration
		void Register(const ClientName& theClientNameR, const Password& thePasswordR, bool becomeHost =false, bool becomeSpectator =false, bool joinChat =false, const CompletionContainer<const RegisterClientResult&>& completion = DEFAULT_COMPLETION );
		void Register(const ClientName& theClientNameR, const std::string& thePasswordR, bool becomeHost =false, bool becomeSpectator =false, bool joinChat =false, const CompletionContainer<const RegisterClientResult&>& completion = DEFAULT_COMPLETION );
		void Disconnect(bool isPermanent,                              const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void Reconnect(bool getMissedMessages,                         const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		ClientId GetClientId() const;
		bool IsRegistered() const;

		// Client lists
		// FIX: add overloads that take a non-STL completion
		void GetClientList(bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag,                         const CompletionContainer<const GetClientListResult&>& completion = DEFAULT_COMPLETION );
		void GetClientInfo(ClientId theClientId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag,   const CompletionContainer<const GetClientInfoResult&>& completion = DEFAULT_COMPLETION );
		void GetGroupList(                                                                                                const CompletionContainer<const GetGroupListResult&>& completion = DEFAULT_COMPLETION );
		void GetMembersOfGroup(GroupId theGroupId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag, const CompletionContainer<const GetClientListResult&>& completion = DEFAULT_COMPLETION );
		void GetSimpleClientList(unsigned char theClientListType,                                                         const CompletionContainer<const GetSimpleClientListResult&>& completion = DEFAULT_COMPLETION );

		// Send raw data
		void BroadcastData(const Message& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BroadcastData(const WONCommon::RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SendData(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SendData(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const WONCommon::RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SendData(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const WONCommon::RawBuffer& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SendDataMultiple(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message theMessages[], unsigned short theNumMessages, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SendDataMultiple(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const std::list<WONCommon::RawBuffer>& theMessageListR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );

		// Send chat
		void BroadcastChat(const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BroadcastChat(const string& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BroadcastChat(const wstring& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void WhisperChat(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const string& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const wstring& theMessageR, bool shouldSendReply, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );

		// Ignore
		void IgnoreClient(const ClientName& theClientNameR);
		void UnignoreClient(const ClientName& theClientNameR);

		// Enter/exit chat group
		void JoinChat(const CompletionContainer<short>& completion = DEFAULT_COMPLETION);
		void LeaveChat(const CompletionContainer<short>& completion = DEFAULT_COMPLETION);

		// administrative stuff
//		void AcceptUser(const UserName& theUserNameR, const std::wstring& theCommentR =gEmptyWString) { AcceptUser(true, theUserNameR, theCommentR); }
//		void RejectUser(const UserName& theUserNameR, const std::wstring& theCommentR =gEmptyWString) { AcceptUser(false, theUserNameR, theCommentR); }
		
		// Administrative
		void OpenRegistration(                                         const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void CloseRegistration(                                        const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void SetPassword(const Password& thePasswordR,                 const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BecomeModerator(bool flagOnOrOff,                         const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void AddSuccessor(ClientId theClientId,                        const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void RemoveSuccessor(ClientId theClientId,                     const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		
		// Moderator calls
		void BootClient(ClientId theClientId,                          const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BanClientName(const ClientName& theClientNameR,           const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BanClientId(ClientId theClientId,                         const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void BanWONUserId(unsigned long theWONUserId,                  const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void UnbanClientName(const ClientName& theClientNameR,         const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void UnbanClientId(ClientId theClientId,                       const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void UnbanWONUserId(unsigned long theWONUserId,                const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void InviteClient(const ClientName& theClientNameR,            const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void UninviteClient(const ClientName& theClientNameR,          const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void MuteClient(ClientId theClientId,                          const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void UnmuteClient(ClientId theClientId,                        const CompletionContainer<short>& completion = DEFAULT_COMPLETION );

		// Group management
		void CreateGroup(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const ClientId* theInitialMembersP =NULL, unsigned short theNumInitialMembers =0, const CompletionContainer<const CreateGroupResult&>& completion = DEFAULT_COMPLETION );
		void CreateGroup(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const std::list<ClientId>& theInitialMembers, const CompletionContainer<const CreateGroupResult&>& completion = DEFAULT_COMPLETION );
		void DeleteGroup(GroupId theGroupId,                           const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void AddToGroup(ClientId theClientId, GroupId theGroupId,      const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void RemoveFromGroup(ClientId theClientId, GroupId theGroupId, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );

		// Dataobjects
		void CreateDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, ClientOrGroupId theOwnerId, unsigned short theLifespan, const WONCommon::RawBuffer& theDataR, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void DeleteDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void ModifyDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, unsigned short theOffset, bool isInsert, const WONCommon::RawBuffer& theDataR, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void ReplaceDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void RenewDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, unsigned short theLifespan, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );
		void ReadDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<const ReadDataObjectResult&>& completion = DEFAULT_COMPLETION );
		void SubscribeDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<const ReadDataObjectResult&>& completion = DEFAULT_COMPLETION );
		void UnsubscribeDataObject(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers, const CompletionContainer<short>& completion = DEFAULT_COMPLETION );

		void IsUserPresent(const wstring& theUserName, bool caseSensitive =false, bool shouldSendNACK =true, unsigned short theTag =0, const CompletionContainer<bool>& completion = DEFAULT_COMPLETION);
		void GetNumUsers(unsigned short theTag =0, const CompletionContainer<const GetNumUsersResult&>& completion = DEFAULT_COMPLETION);

		// completion routines for async messages
		CompletionContainer<const ClientName&>                     mClientJoinAttemptCatcher;
		CompletionContainer<const ClientIdWithReason&>             mClientEnterCatcher;
		CompletionContainer<const ClientDataWithReason&>           mClientEnterExCatcher;
		CompletionContainer<const ClientIdWithReason&>             mClientLeaveCatcher;
		CompletionContainer<ClientId>                              mHostChangeCatcher;
		CompletionContainer<unsigned short>                        mSpectatorCountCatcher;
		CompletionContainer<const GroupIdAndClientIdWithReason&>   mGroupEnterCatcher;
		CompletionContainer<const GroupIdAndClientDataWithReason&> mGroupEnterExCatcher;
		CompletionContainer<const GroupIdAndClientIdWithReason&>   mGroupLeaveCatcher;
		CompletionContainer<const GroupIdAndSpectatorCount&>       mGroupSpectatorCountCatcher;
		CompletionContainer<const ClientIdWithFlag&>               mMuteClientCatcher;
		CompletionContainer<const ClientIdWithFlag&>               mBecomeModeratorCatcher;
		CompletionContainer<const DataObjectWithLifespan&>         mDataObjectCreationCatcher;
		CompletionContainer<const DataObject&>                     mDataObjectDeletionCatcher;
		CompletionContainer<const DataObjectModification&>         mDataObjectModificationCatcher;
		CompletionContainer<const DataObject&>                     mDataObjectReplacementCatcher;
		CompletionContainer<void>                                  mKeepAliveCatcher;
		CompletionContainer<const MessageWithClientId&>            mPeerDataCatcher;
		CompletionContainer<const RawChatMessage&>                 mRawPeerChatCatcher;
		CompletionContainer<const ASCIIChatMessage&>               mASCIIPeerChatCatcher;
		CompletionContainer<const UnicodeChatMessage&>             mUnicodePeerChatCatcher;
		CompletionContainer<void>                                  mReconnectFailureCatcher;

		void InstallClientJoinAttemptCatcher(const CompletionContainer<const ClientName&>& completion)                  { mClientJoinAttemptCatcher = completion;      mClientJoinAttemptCatcher.OwnCompletion(); }
		void InstallClientEnterCatcher(const CompletionContainer<const ClientIdWithReason&>& completion)                { mClientEnterCatcher = completion;            mClientEnterCatcher.OwnCompletion(); }
		void InstallClientEnterExCatcher(const CompletionContainer<const ClientDataWithReason&>& completion)            { mClientEnterExCatcher = completion;          mClientEnterExCatcher.OwnCompletion(); }
		void InstallClientLeaveCatcher(const CompletionContainer<const ClientIdWithReason&>& completion)                { mClientLeaveCatcher = completion;            mClientLeaveCatcher.OwnCompletion(); }
		void InstallSpectatorCountCatcher(const CompletionContainer<unsigned short>& completion)                        { mSpectatorCountCatcher = completion;         mSpectatorCountCatcher.OwnCompletion(); }
		void InstallGroupEnterCatcher(const CompletionContainer<const GroupIdAndClientIdWithReason&>& completion)       { mGroupEnterCatcher = completion;             mGroupEnterCatcher.OwnCompletion(); }
		void InstallGroupEnterExCatcher(const CompletionContainer<const GroupIdAndClientDataWithReason&>& completion)   { mGroupEnterExCatcher = completion;           mGroupEnterExCatcher.OwnCompletion(); }
		void InstallGroupLeaveCatcher(const CompletionContainer<const GroupIdAndClientIdWithReason&>& completion)       { mGroupLeaveCatcher = completion;             mGroupLeaveCatcher.OwnCompletion(); }
		void InstallGroupSpectatorCountCatcher(const CompletionContainer<const GroupIdAndSpectatorCount&>& completion)  { mGroupSpectatorCountCatcher = completion;    mGroupSpectatorCountCatcher.OwnCompletion(); }
		void InstallMuteClientCatcher(const CompletionContainer<const ClientIdWithFlag&> completion)                     { mMuteClientCatcher = completion;             mMuteClientCatcher.OwnCompletion(); }
		void InstallBecomeModeratorCatcher(const CompletionContainer<const ClientIdWithFlag&> completion)                { mBecomeModeratorCatcher = completion;        mBecomeModeratorCatcher.OwnCompletion(); }
		void InstallHostChangeCatcher(const CompletionContainer<ClientId>& completion)                                  { mHostChangeCatcher = completion;             mHostChangeCatcher.OwnCompletion(); }
		void InstallDataObjectCreationCatcher(const CompletionContainer<const DataObjectWithLifespan&>& completion)     { mDataObjectCreationCatcher = completion;     mDataObjectCreationCatcher.OwnCompletion(); }
		void InstallDataObjectDeletionCatcher(const CompletionContainer<const DataObject&>& completion)                 { mDataObjectDeletionCatcher = completion;     mDataObjectDeletionCatcher.OwnCompletion(); }
		void InstallDataObjectModificationCatcher(const CompletionContainer<const DataObjectModification&>& completion) { mDataObjectModificationCatcher = completion; mDataObjectModificationCatcher.OwnCompletion(); }
		void InstallDataObjectReplacementCatcher(const CompletionContainer<const DataObject&>& completion)              { mDataObjectReplacementCatcher = completion;  mDataObjectReplacementCatcher.OwnCompletion(); }
		void InstallKeepAliveCatcher(const CompletionContainer<void>& completion)                                       { mKeepAliveCatcher = completion;              mKeepAliveCatcher.OwnCompletion(); }
		void InstallPeerDataCatcher(const CompletionContainer<const MessageWithClientId&>& completion)                  { mPeerDataCatcher = completion;               mPeerDataCatcher.OwnCompletion(); }
		void InstallRawPeerChatCatcher(const CompletionContainer<const RawChatMessage&>& completion)                    { mRawPeerChatCatcher = completion;            mRawPeerChatCatcher.OwnCompletion(); }
		void InstallASCIIPeerChatCatcher(const CompletionContainer<const ASCIIChatMessage&>& completion)                { mASCIIPeerChatCatcher = completion;          mASCIIPeerChatCatcher.OwnCompletion(); }
		void InstallUnicodePeerChatCatcher(const CompletionContainer<const UnicodeChatMessage&>& completion)            { mUnicodePeerChatCatcher = completion;        mUnicodePeerChatCatcher.OwnCompletion(); }
		void InstallReconnectFailureCatcher(const CompletionContainer<void>& completion)                                { mReconnectFailureCatcher = completion;       mReconnectFailureCatcher.OwnCompletion(); }

		WSSocket* GetSocket() { return mSocketP->GetSocket(); }
		static void FillMsgFromResult(WONMsg::BaseMessage& theMsg, const TMsgSocket::RecvBaseMsgResult& result);
	protected:
		virtual void OnBecomeModerator(ClientId theClientId, bool flagOnOrOff) {}
		virtual void OnChatFromPeer() {}
		virtual void OnClientEnter(ClientId theClientId, const ClientName& theClientNameR =gEmptyRawBuffer, unsigned long theIPAddress =0, unsigned long theWONUserId =0, unsigned long theCommunityId =0, unsigned short theTrustLevel =0) {}
		virtual void OnClientLeave(ClientId theClientId) {}
		virtual void OnClientJoinAttempt(const ClientName& theClientNameR) {}
		virtual void OnDataFromPeer() {}
		virtual void OnDataObjectCreated() {}
		virtual void OnDataObjectDeleted() {}
		virtual void OnDataObjectModified() {}
		virtual void OnDataObjectReplaced() {}
		virtual void OnGroupEnter(GroupId theGroupId, ClientId theClientId, const ClientName& theClientNameR =gEmptyRawBuffer, unsigned long theIPAddress =0, unsigned long theWONUserId =0, unsigned long theCommunityId =0, unsigned short theTrustLevel =0) {}
		virtual void OnGroupLeave(GroupId theGroupId, ClientId theClientId) {}
		virtual void OnGroupSpectatorCount(GroupId theGroupId, unsigned short theSpectatorCount) {}
		virtual void OnHostChange(ClientId theNewHostId) {}
		virtual void OnKeepAlive() {}
		virtual void OnMuteClient(ClientId theClientId, bool flagMuteOrUnmute) {}
		virtual void OnReconnectFailure() {}
		virtual void OnSpectatorCount(unsigned short theSpectatorCount) {}
	private:
//		void AcceptUser(bool accept, const UserName& theUserNameR, const std::wstring& theCommentR);

		// completion routines
		struct ConnectData {
			RoutingServerClient*                            mRoutingServerClientP;
			CompletionContainer<Error>						mFinalCompletion;
			long                                            mTimeout;
			Error*                                          mStatusP;
			WONCommon::Event                                mDoneEvent;
			bool											mAutoDelete;
			ConnectData() : mRoutingServerClientP(NULL), mFinalCompletion(), mTimeout(0), mStatusP(NULL), mAutoDelete(false) {}
		};
		static void OpenCompletion(const Socket::OpenResult& theResultR, ConnectData* theConnectDataP);
		static void AuthCompletion(const AuthSocket::AuthResult& theResultR, ConnectData* theConnectDataP);
		static void SocketClosed(const Socket::Result& result, RoutingServerClient* that);
		static void ReconnectAttemptCallback(Error error, RoutingServerClient* that);
		static void ReconnectRequestCallback(short theStatus, RoutingServerClient* that);
		static void ReceiveLoop(const AuthSocket::AuthRecvBaseMsgResult& result, RoutingServerClient* that);//ClientBoolPair* theClientBoolPair);
		
		void LoadIgnoredUsersList();
		void SaveIgnoredUsersList() const;
		bool IsClientIgnored(WONMsg::ClientId theClientId) const;
		bool IsClientIgnored(const ClientName& theClientNameR) const;

		bool IsAsyncMessage(unsigned char theMessageType);
		bool IsReplyMessage(unsigned char theMessageType);
		void SendMessageToServer(const WONMsg::BaseMessage& theMsgR, unsigned char theMessageType =0, CompletionContainerBase* completion =NULL);

		void BroadcastChat(const WONCommon::RawBuffer& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion);
		void WhisperChat(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const WONCommon::RawBuffer& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, const CompletionContainer<short>& completion);
		void BanOrUnbanClientId(bool banOrUnban, ClientId theClientId, const CompletionContainer<short>& completion);
		void BanOrUnbanClientName(bool banOrUnban, const ClientName& theClientNameR, const CompletionContainer<short>& completion);
		void BanOrUnbanWONUserId(bool banOrUnban, long theWONUserId, const CompletionContainer<short>& completion);

		// message handlers
		void InitMessageHandlers();
//		void HandleAcceptUser(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleBecomeModerator(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleClientChange(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleClientChangeEx(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleCreateDataObject(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleDeleteDataObject(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleGroupChange(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleGroupChangeEx(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleGroupSpectatorCount(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleHostChange(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleKeepAlive(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleModifyDataObject(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleMuteClient(const TMsgSocket::RecvBaseMsgResult& result);
		void HandlePeerData(const TMsgSocket::RecvBaseMsgResult& result);
		void HandlePeerChat(const TMsgSocket::RecvBaseMsgResult& result);
		void HandlePeerDataMultiple(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleReplaceDataObject(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleSpectatorCount(const TMsgSocket::RecvBaseMsgResult& result);

		void HandleGetClientInfoReply(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleGetClientListReply(const TMsgSocket::RecvBaseMsgResult& result);
		void HandleRegisterClientReply(const TMsgSocket::RecvBaseMsgResult& result);

		typedef std::pair<unsigned char, CompletionContainerBase*> RequestCompletionPair;
		typedef std::list<RequestCompletionPair> RequestCompletionList;
		typedef std::map<ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData>  ClientMap;
		typedef std::map<ClientId, WONMsg::MMsgRoutingGetClientListReply::ClientData>::value_type  ClientMapPair;
		typedef std::map<ClientName, ClientId> ClientNameToIdMap;
		typedef std::map<ClientName, ClientId>::value_type ClientNameToIdMapPair;
		typedef std::set<ClientName> ClientNameSet;
		typedef std::set<ClientId> ClientIdSet;
		
		typedef void (RoutingServerClient::*Handler) (const TMsgSocket::RecvBaseMsgResult& result);
		typedef std::vector<Handler> MessageHandlerVector;

		bool                  mIsTCP;
		bool                  mEnableShortcuts;
		bool                  mIgnoringIsOn;
		unsigned char         mLengthFieldSize;
		IPSocket::Address     mRoutingAddress;
		AuthSocket*           mSocketP;
		Identity              mCurIdentity;
		MessageHandlerVector  mReplyHandlerVector;
		MessageHandlerVector  mAsyncMsgHandlerVector;
		RequestCompletionList mRequestCompletionList;
		WONCommon::CriticalSection mRequestCompletionListCrit;
		
		ClientName mClientName;
		ClientId   mClientId;

		ClientId          mHostId;
		ClientMap         mClientMap;
		ClientNameToIdMap mClientNameToIdMap;

		ClientNameSet mIgnoredClientNames;
		ClientIdSet   mIgnoredClientIds;		
	public:
		template <class privsType> Error ConnectEx(const TCPSocket::Address& theRoutingAddress, Identity* theIdentityP, bool isReconnectAttempt, long theTimeout, bool async, void (*f)(Error, privsType), privsType t)   { return Connect(theRoutingAddress, theIdentityP, isReconnectAttempt, theTimeout, async, new CompletionWithContents<Error, privsType>(t, f, true)); }
		template <class privsType> void  RegisterEx(const ClientName& theClientNameR, const Password& thePasswordR, bool becomeHost, bool becomeSpectator, bool joinChat,	  void (*f)(const RegisterClientResult&, privsType), privsType t) { Register(theClientNameR, thePasswordR, becomeHost, becomeSpectator, joinChat, new CompletionWithContents<const RegisterClientResult&, privsType>(t, f, true)); }
		template <class privsType> void  RegisterEx(const ClientName& theClientNameR, const std::string& thePasswordR, bool becomeHost, bool becomeSpectator, bool joinChat,  void (*f)(const RegisterClientResult&, privsType), privsType t) { Register(theClientNameR, thePasswordR, becomeHost, becomeSpectator, joinChat, new CompletionWithContents<const RegisterClientResult&, privsType>(t, f, true)); }
		template <class privsType> void  DisconnectEx(bool isPermanent,                                                                                                       void (*f)(short, privsType), privsType t)                       { Disconnect(isPermanent, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  ReconnectEx(bool getMissedMessages,                                                                                                  void (*f)(short, privsType), privsType t)                       { Reconnect(getMissedMessages, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  GetClientListEx(bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag,                                               void (*f)(const GetClientListResult&, privsType), privsType t)  { GetClientList(requestAuthInfo, requestMuteFlag, requestModeratorFlag, new CompletionWithContents<const GetClientListResult&, privsType>(t, f, true)); }
		template <class privsType> void  GetClientInfoEx(ClientId theClientId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag,                         void (*f)(const GetClientInfoResult&, privsType), privsType t)  { GetClientInfo(theClientId, requestAuthInfo, requestMuteFlag, requestModeratorFlag, new CompletionWithContents<const GetClientInfoResult&, privsType>(t, f, true)); }
		template <class privsType> void  GetGroupListEx(                                                                                                                      void (*f)(const GetGroupListResult&, privsType), privsType t)   { GetGroupList(new CompletionWithContents<const GetGroupListResult&, privsType>(t, f, true)); }
		template <class privsType> void  GetMembersOfGroupEx(GroupId theGroupId, bool requestAuthInfo, bool requestMuteFlag, bool requestModeratorFlag,                       void (*f)(const GetClientListResult&, privsType), privsType t)  { GetMembersOfGroup(theGroupId, requestAuthInfo, requestMuteFlag, requestModeratorFlag, new CompletionWithContents<const GetClientListResult&, privsType>(t, f, true)); }
		template <class privsType> void  GetSimpleClientList(unsigned char theClientListType,                                                                                 void (*f)(const GetSimpleClientListResult&, privsType), privsType t) { GetSimpleClientList(theClientListType, new CompletionWithContents<const GetSimpleClientListResult&, privsType>(t, f, true)); }
		template <class privsType> void  BroadcastDataEx(const Message& theMessageR, bool shouldSendReply,                                                                    void (*f)(short, privsType), privsType t)                       { BroadcastData(theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BroadcastDataEx(const WONCommon::RawBuffer& theMessageR, bool shouldSendReply,                                                                  void (*f)(short, privsType), privsType t)                       { BroadcastData(theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SendDataEx(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, const Message& theMessageR, bool shouldSendReply,       void (*f)(short, privsType), privsType t)                       { SendData(theRecipients, theNumRecipients, theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SendDataEx(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const WONCommon::RawBuffer& theMessageR, bool shouldSendReply, void (*f)(short, privsType), privsType t)  { SendData(theRecipients, theNumRecipients, flagIncludeExclude, theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SendDataEx(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const WONCommon::RawBuffer& theMessageR, bool shouldSendReply, void (*f)(short, privsType), privsType t)               { SendData(theRecipientListR, flagIncludeExclude, theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BroadcastChatEx(const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ BroadcastChat(theMessageR, theChatType, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BroadcastChatEx(const string& theMessageR, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ BroadcastChat(theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BroadcastChatEx(const wstring& theMessageR, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ BroadcastChat(theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  WhisperChatEx(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message& theMessageR, WONMsg::ChatType theChatType, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ WhisperChat(theRecipients, theNumRecipients, flagIncludeExclude, theMessageR, theChatType, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  WhisperChatEx(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const string& theMessageR, bool shouldSendReply,  void (*f)(short, privsType), privsType t)		{ WhisperChat(theRecipientListR, flagIncludeExclude, theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  WhisperChatEx(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const wstring& theMessageR, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ WhisperChat(theRecipientListR, flagIncludeExclude, theMessageR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SendDataMultipleEx(const ClientOrGroupId theRecipients[], unsigned short theNumRecipients, bool flagIncludeExclude, const Message theMessages[], unsigned short theNumMessages, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ SendDataMultiple(theRecipients, theNumRecipients, flagIncludeExclude, theMessages, theNumMessages, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SendDataMultipleEx(const std::list<ClientOrGroupId>& theRecipientListR, bool flagIncludeExclude, const std::list<WONCommon::RawBuffer>& theMessageListR, bool shouldSendReply, void (*f)(short, privsType), privsType t)		{ SendDataMultiple(theRecipientListR, flagIncludeExclude, theMessageListR, shouldSendReply, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  JoinChatEx(                                                                                                                          void (*f)(short, privsType), privsType t)		{ JoinChat(new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  LeaveChatEx(                                                                                                                         void (*f)(short, privsType), privsType t)		{ LeaveChat(new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  OpenRegistrationEx(                                                                                                                  void (*f)(short, privsType), privsType t)		{ OpenRegistration(new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  CloseRegistrationEx(                                                                                                                 void (*f)(short, privsType), privsType t)		{ CloseRegistration(new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  SetPasswordEx(const Password& thePasswordR,                                                                                          void (*f)(short, privsType), privsType t)		{ SetPassword(thePasswordR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BecomeModeratorEx(bool flagOnOrOff,                                                                                                  void (*f)(short, privsType), privsType t)		{ BecomeModerator(flagOnOrOff, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BootClientEx(ClientId theClientId,																									  void (*f)(short, privsType), privsType t)		{ BootClient(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BanClientNameEx(const ClientName& theClientNameR,                                                                                    void (*f)(short, privsType), privsType t)		{ BanClientName(theClientNameR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BanClientIdEx(ClientId theClientId,                                                                                                  void (*f)(short, privsType), privsType t)		{ BanClientId(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  BanWONUserIdEx(unsigned long theWONUserId,                                                                                           void (*f)(short, privsType), privsType t)		{ BanWONUserId(theWONUserId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  UnbanClientNameEx(const ClientName& theClientNameR,                                                                                  void (*f)(short, privsType), privsType t)		{ UnbanClientName(theClientNameR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  UnbanClientIdEx(ClientId theClientId,                                                                                                void (*f)(short, privsType), privsType t)		{ UnbanClientId(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  UnbanWONUserIdEx(unsigned long theWONUserId,                                                                                         void (*f)(short, privsType), privsType t)		{ UnbanWONUserId(theWONUserId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  InviteClientEx(const ClientName& theClientNameR,                                                                                     void (*f)(short, privsType), privsType t)		{ InviteClient(theClientNameR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  UninviteClientEx(const ClientName& theClientNameR,                                                                                   void (*f)(short, privsType), privsType t)		{ UninviteClient(theClientNameR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  MuteClientEx(ClientId theClientId,                                                                                                   void (*f)(short, privsType), privsType t)		{ MuteClient(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  UnmuteClientEx(ClientId theClientId,                                                                                                 void (*f)(short, privsType), privsType t)		{ UnmuteClient(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  AddSuccessorEx(ClientId theClientId,                                                                                                 void (*f)(short, privsType), privsType t)		{ AddSuccessor(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  RemoveSuccessorEx(ClientId theClientId,                                                                                              void (*f)(short, privsType), privsType t)		{ RemoveSuccessor(theClientId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  CreateGroupEx(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const ClientId* theInitialMembersP, unsigned short theNumInitialMembers, void (*f)(const CreateGroupResult&, privsType), privsType t)		{ CreateGroup(theGroupNameR, isPublic, announceChanges, theInitialMembersP, theNumInitialMembers, new CompletionWithContents<const CreateGroupResult&, privsType>(t, f, true)); }
		template <class privsType> void  CreateGroupEx(const GroupName& theGroupNameR, bool isPublic, bool announceChanges, const std::list<ClientId>& theInitialMembers,     void (*f)(const CreateGroupResult&, privsType), privsType t)		{ CreateGroup(theGroupNameR, isPublic, announceChanges, theInitialMembers, new CompletionWithContents<const CreateGroupResult&, privsType>(t, f, true)); }
		template <class privsType> void  DeleteGroupEx(GroupId theGroupId,                                                                                                    void (*f)(short, privsType), privsType t)		{ DeleteGroup(theGroupId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  AddToGroupEx(ClientId theClientId, GroupId theGroupId,                                                                               void (*f)(short, privsType), privsType t)		{ AddToGroup(theClientId, theGroupId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  RemoveFromGroupEx(ClientId theClientId, GroupId theGroupId,                                                                          void (*f)(short, privsType), privsType t)		{ RemoveFromGroup(theClientId, theGroupId, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  CreateDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, ClientOrGroupId theOwnerId, unsigned short theLifespan, const WONCommon::RawBuffer& theDataR, void (*f)(short, privsType), privsType t)		{ CreateDataObject(theLinkId, theDataTypeR, theOwnerId, theLifespan, theDataR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  DeleteDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR,                                                         void (*f)(short, privsType), privsType t)		{ DeleteDataObject(theLinkId, theDataTypeR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  ModifyDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, unsigned short theOffset, bool isInsert, const WONCommon::RawBuffer& theDataR, void (*f)(short, privsType), privsType t)		{ ModifyDataObject(theLinkId, theDataTypeR, theOffset, isInsert, theDataR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  ReplaceDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, const WONCommon::RawBuffer& theDataR,                             void (*f)(short, privsType), privsType t)		{ ReplaceDataObject(theLinkId, theDataTypeR, theDataR, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  RenewDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, unsigned short theLifespan,                              void (*f)(short, privsType), privsType t)		{ RenewDataObject(theLinkId, theDataTypeR, theLifespan, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  ReadDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers,       void (*f)(const ReadDataObjectResult&, privsType), privsType t)		{ ReadDataObject(theLinkId, theDataTypeR, flagExactOrRecursive, flagGroupOrMembers, new CompletionWithContents<const ReadDataObjectResult&, privsType>(t, f, true)); }
		template <class privsType> void  SubscribeDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers,  void (*f)(const ReadDataObjectResult&, privsType), privsType t)		{ SubscribeDataObject(theLinkId, theDataTypeR, flagExactOrRecursive, flagGroupOrMembers, new CompletionWithContents<const ReadDataObjectResult&, privsType>(t, f, true)); }
		template <class privsType> void  UnsubscribeDataObjectEx(ClientOrGroupId theLinkId, const WONCommon::RawBuffer& theDataTypeR, bool flagExactOrRecursive, bool flagGroupOrMembers,void (*f)(short, privsType), privsType t)		{ UnsubscribeDataObject(theLinkId, theDataTypeR, flagExactOrRecursive, flagGroupOrMembers, new CompletionWithContents<short, privsType>(t, f, true)); }
		template <class privsType> void  IsUserPresentEx(const wstring& theUserName, bool caseSensitive, bool shouldSendNACK, unsigned short theTag,									 void (*f)(bool, privsType), privsType t)       { IsUserPresent(theUserName, caseSensitive, shouldSendNACK, theTag, new CompletionWithContents<bool,privsType>(t, f, true)); }
		template <class privsType> void  GetNumUsersEx(unsigned short theTag,																											 void (*f)(const GetNumUsersResult&, privsType t), privsType t) { GetNumUsers(theTag, new CompletionWithContents<const GetNumUsersResult&, privsType>(t, f, true)); }

		template <class privsType> void  InstallClientJoinAttemptCatcherEx(     void (*f)(const ClientName&, privsType), privsType t)                     { InstallClientJoinAttemptCatcher(new CompletionWithContents<const ClientName&, privsType>(t, f, true)); }
		template <class privsType> void  InstallClientEnterCatcherEx(           void (*f)(const ClientIdWithReason&, privsType), privsType t)             { InstallClientEnterCatcher(new CompletionWithContents<const ClientIdWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallClientEnterExCatcherEx(         void (*f)(const ClientDataWithReason&, privsType), privsType t)           { InstallClientEnterExCatcher(new CompletionWithContents<const ClientDataWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallClientLeaveCatcherEx(           void (*f)(const ClientIdWithReason&, privsType), privsType t)             { InstallClientLeaveCatcher(new CompletionWithContents<const ClientIdWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallHostChangeCatcherEx(            void (*f)(ClientId, privsType), privsType t)                              { InstallHostChangeCatcher(new CompletionWithContents<ClientId, privsType>(t, f, true)); }
		template <class privsType> void  InstallGroupEnterCatcherEx(            void (*f)(const GroupIdAndClientIdWithReason&, privsType), privsType t)   { InstallGroupEnterCatcher(new CompletionWithContents<const GroupIdAndClientIdWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallGroupLeaveCatcherEx(            void (*f)(const GroupIdAndClientIdWithReason&, privsType), privsType t)   { InstallGroupLeaveCatcher(new CompletionWithContents<const GroupIdAndClientIdWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallGroupEnterExCatcherEx(          void (*f)(const GroupIdAndClientDataWithReason&, privsType), privsType t) { InstallGroupEnterExCatcher(new CompletionWithContents<const GroupIdAndClientDataWithReason&, privsType>(t, f, true)); }
		template <class privsType> void  InstallSpectatorCountCatcherEx(        void (*f)(unsigned short, privsType), privsType t)                        { InstallSpectatorCountCatcher(new CompletionWithContents<unsigned short, privsType>(t, f, true)); }
		template <class privsType> void  InstallGroupSpectatorCountCatcherEx(   void (*f)(const GroupIdAndSpectatorCount&, privsType), privsType t)       { InstallGroupSpectatorCountCatcher(new CompletionWithContents<const GroupIdAndSpectatorCount&, privsType>(t, f, true)); }
		template <class privsType> void  InstallMuteClientCatcherEx(            void (*f)(const ClientIdWithFlag&, privsType), privsType t)               { InstallMuteClientCatcher(new CompletionWithContents<const ClientIdWithFlag&, privsType>(t, f, true)); }
		template <class privsType> void  InstallBecomeModeratorCatcherEx(       void (*f)(const ClientIdWithFlag&, privsType), privsType t)               { InstallBecomeModeratorCatcher(new CompletionWithContents<const ClientIdWithFlag&, privsType>(t, f, true)); }
		template <class privsType> void  InstallDataObjectCreationCatcherEx(    void (*f)(const DataObjectWithLifespan&, privsType), privsType t)         { InstallDataObjectCreationCatcher(new CompletionWithContents<const DataObjectWithLifespan&, privsType>(t, f, true)); }
		template <class privsType> void  InstallDataObjectDeletionCatcherEx(    void (*f)(const DataObject&, privsType), privsType t)                     { InstallDataObjectDeletionCatcher(new CompletionWithContents<const DataObject&, privsType>(t, f, true)); }
		template <class privsType> void  InstallDataObjectModificationCatcherEx(void (*f)(const DataObjectModification&, privsType), privsType t)         { InstallDataObjectModificationCatcher(new CompletionWithContents<const DataObjectModification&, privsType>(t, f, true)); }
		template <class privsType> void  InstallDataObjectReplacementCatcherEx( void (*f)(const DataObject&, privsType), privsType t)                     { InstallDataObjectReplacementCatcher(new CompletionWithContents<const DataObject&, privsType>(t, f, true)); }
		template <class privsType> void  InstallKeepAliveCatcherEx(             void (*f)(privsType), privsType t)                                        { InstallKeepAliveCatcher(new CompletionVoidWithContents<privsType>(t, f, true)); }
		template <class privsType> void  InstallPeerDataCatcherEx(              void (*f)(const MessageWithClientId&, privsType), privsType t)            { InstallPeerDataCatcher(new CompletionWithContents<const MessageWithClientId&, privsType>(t, f, true)); }
		template <class privsType> void  InstallRawPeerChatCatcherEx(           void (*f)(const RawChatMessage&, privsType), privsType t)                 { InstallRawPeerChatCatcher(new CompletionWithContents<const RawChatMessage&, privsType>(t, f, true)); }
		template <class privsType> void  InstallASCIIPeerChatCatcherEx(         void (*f)(const ASCIIChatMessage&, privsType), privsType t)               { InstallASCIIPeerChatCatcher(new CompletionWithContents<const ASCIIChatMessage&, privsType>(t, f, true)); }
		template <class privsType> void  InstallUnicodePeerChatCatcherEx(       void (*f)(const UnicodeChatMessage&, privsType), privsType t)             { InstallUnicodePeerChatCatcher(new CompletionWithContents<const UnicodeChatMessage&, privsType>(t, f, true)); }
		template <class privsType> void  InstallReconnectFailureCatcherEx(      void (*f)(privsType), privsType t)                                        { InstallReconnectFailureCatcher(new CompletionVoidWithContents<privsType>(t, f, true)); }
	};
	
}; // namespace WONAPI

#endif // ROUTINGAPI_H