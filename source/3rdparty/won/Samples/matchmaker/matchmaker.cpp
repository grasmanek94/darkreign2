#pragma warning (disable : 4786)

#include <sys/types.h>
#include <sys/stat.h>
#include <common/won.h>
#ifndef DLLSAMPLE
#include <wondll.h>
#include <WONHTTP.h>
#include <msg/Dir/DirEntity.h>
#include <msg/Profile/SMsgProfileMsgs.h>
#include <Profile/ProfileAPI.h>
#endif // !DLLSAMPLE
#include <msg/ServerStatus.h>
#include "matchmaker.h"
#include "main.h"
#include "game.h"

namespace {
	using namespace std;
	using namespace WONMsg;
	using namespace WONCommon;
#ifndef DLLSAMPLE
	using namespace WONAPI;
#endif // !DLLSAMPLE
};

long gRequestTimeout = -1;

const char*             FILE_VERIFIER_KEY        = "kver.pub";
const char*             ADDR_PATCHSERVER         = "sample.update.won.net";
const unsigned short    PORT_PATCHSERVER         = 80;
const char*             ADDR_MOTDSERVER          = "www.won.net";
const unsigned short    PORT_MOTDSERVER          = 80;
const char*             COMMUNITY_SAMPLE         = "WON";
const char*             CONFIG_SAMPLECHATSERVER  = "RoutingServSampleChat";
const wchar_t*          DIR_TITANSERVER          = L"/TitanServers"; // Parent directory of dirs holding auth, firewall, and event servers.
const wchar_t*          DIR_APISAMPLE            = L"/APISample";
const RawBuffer         OBJ_GAMEPREFIX           = (unsigned char*)"GAME";
const RawBuffer         OBJ_VALIDVERSIONS        = (unsigned char*)"SampleValidVersions";
const RawBuffer         OBJ_CHATROOM_CLIENTCOUNT = (unsigned char*)"__RSClientCount";
const RawBuffer         OBJ_CHATROOM_LOGINACL    = (unsigned char*)"__RSLoginACL";
const RawBuffer         OBJ_CHATROOM_ROOMFLAGS   = (unsigned char*)"__RSRoomFlags";
const wchar_t*          SERVERNAME_AUTH          = L"AuthServer";            // Service name for auth servers
const wchar_t*          SERVERNAME_CHAT          = L"TitanRoutingServer";    // Service name for chat rooms (chat Routing Servers)
const wchar_t*          SERVERNAME_CONTEST       = L"TitanContestServer";    // Service name for contest servers
const wchar_t*          SERVERNAME_EVENT         = L"TitanEventServer";      // Service name for event servers
const wchar_t*          SERVERNAME_FACTORY       = L"TitanFactoryServer";    // Service name for factory servers
const wchar_t*          SERVERNAME_FIREWALL      = L"TitanFirewallDetector"; // Service name for firewall servers
const wchar_t*          SERVERNAME_PROFILE       = L"TitanProfileServer";    // Service name for profile servers
const short             CONTESTPROC_REPORTRESULT = 8; // p_ttt_saveresults
const short             CONTESTPROC_TOPTENLIST   = 9; // p_ttt_gethighscores

Matchmaker::Matchmaker() : 
	mNumAuthServers(0),
	mNumChatServers(0),
	mNumContestServers(0),
	mNumDirServers(0),
	mNumEventServers(0),
	mNumFactoryServers(0),
	mNumFirewallServers(0),
	mNumProfileServers(0),
	mNumProfileServersTried(0),
	mCurProfileServer(0),
	mAuthServers(NULL),
	mContestServers(NULL),
	mDirServers(NULL),
	mEventServers(NULL),
	mFactoryServers(NULL),
	mFirewallServers(NULL),
	mProfileServers(NULL),
#ifdef DLLSAMPLE
	mAuthH(NULL),
#else
	mGameInfo(mRoutingServer),
#endif // !DLLSAMPLE
	mBehindFirewall(true)
{
#ifdef DLLSAMPLE
	mRoutingServer = WONRoutingCreateSession();
	mGameInfo.Init(mRoutingServer);
#endif // DLLSAMPLE
}

Matchmaker::~Matchmaker()
{
#ifdef DLLSAMPLE
	WONRoutingCloseSessionHandle(mRoutingServer);
	if (mAuthH) { WONAuthCloseHandle(mAuthH); }
#endif // DLLSAMPLE

	delete[] mDirServers;
	delete[] mAuthServers;
	delete[] mContestServers;
	delete[] mEventServers;
	delete[] mFactoryServers;
	delete[] mFirewallServers;
	delete[] mProfileServers;
}

bool Matchmaker::Init()
{
	// You can load the verifier key from the registry or from a file.  However, if you load it 
	// from the registry, please load it from your own location rather than using the default.  If you
	// get 110003 errors while authenticating, you probably aren't loading the verifier key.
#ifdef DLLSAMPLE
	if (!WONAuthLoadVerifierKeyFromFileA(FILE_VERIFIER_KEY))
#else
	if (!Identity::LoadVerifierKeyFromFile(FILE_VERIFIER_KEY))
#endif // DLLSAMPLE
	{
		OutputStringF("Failed to load verifier key from %s.", FILE_VERIFIER_KEY);
		return false;
	}
	
#ifdef DLLSAMPLE
//	WONRoutingInstallClientEnterExCallback(mRoutingServer, OnClientEnterChat, this);
	WONRoutingInstallGroupEnterExCallback(mRoutingServer, OnClientEnterChat, this);
	WONRoutingInstallGroupLeaveCallback(mRoutingServer, OnClientLeaveChat, this);
	WONRoutingInstallDataObjectCreationCallback(mRoutingServer, OnDataObjectCreated, this);
	WONRoutingInstallDataObjectDeletionCallback(mRoutingServer, OnDataObjectDeleted, this);
	WONRoutingInstallDataObjectReplacementCallback(mRoutingServer, OnDataObjectReplaced, this);
	WONRoutingInstallPeerDataCallback(mRoutingServer, OnPeerDataReceived, this);
	WONRoutingInstallASCIIPeerChatCallback(mRoutingServer, OnChatReceived, this);
	WONRoutingInstallUnicodePeerChatCallback(mRoutingServer, OnChatReceived, this);
	WONRoutingInstallReconnectFailureCallback(mRoutingServer, OnReconnectFailure, this);
#else
//	mRoutingServer.InstallClientEnterExCatcherEx((void(*)(const RoutingServerClient::ClientDataWithReason&, Matchmaker*))OnClientEnterChat, this);
	mRoutingServer.InstallGroupEnterExCatcherEx((void(*)(const RoutingServerClient::GroupIdAndClientDataWithReason&, Matchmaker*))OnClientEnterChat, this);
	mRoutingServer.InstallGroupLeaveCatcherEx((void(*)(const RoutingServerClient::GroupIdAndClientIdWithReason&, Matchmaker*))OnClientLeaveChat, this);
	mRoutingServer.InstallDataObjectCreationCatcherEx((void(*)(const RoutingServerClient::DataObjectWithLifespan&, Matchmaker*))OnDataObjectCreated, this);
	mRoutingServer.InstallDataObjectDeletionCatcherEx((void(*)(const RoutingServerClient::DataObject&, Matchmaker*))OnDataObjectDeleted, this);
	mRoutingServer.InstallDataObjectReplacementCatcherEx((void(*)(const RoutingServerClient::DataObject&, Matchmaker*))OnDataObjectReplaced, this);
	mRoutingServer.InstallPeerDataCatcherEx((void(*)(const RoutingServerClient::MessageWithClientId&, Matchmaker*))OnPeerDataReceived, this);
	mRoutingServer.InstallASCIIPeerChatCatcherEx((void(*)(const RoutingServerClient::ASCIIChatMessage&, Matchmaker*))OnChatReceived, this);
	mRoutingServer.InstallUnicodePeerChatCatcherEx((void(*)(const RoutingServerClient::UnicodeChatMessage&, Matchmaker*))OnChatReceived, this);
	mRoutingServer.InstallReconnectFailureCatcherEx((void(*)(Matchmaker* that))OnReconnectFailure, this);
#endif // DLLSAMPLE

	GetDirServers();
	GetMOTD();
	GetTitanServerList();

	if (mNumAuthServers == 0)
	{
		OutputString("No Auth Servers found in /TitanServers/Auth");
		return false;
	}

	// initiate firewall detection if there are firewall servers available
	if (mNumFirewallServers)
		DetectFirewall();

	// allow Init() to return
	OutputString("Ready");

	return true;
}

void Matchmaker::GetDirServers()
{
	// NOTE: Put Directory Server addresses in an INI file please!
#ifdef DLLSAMPLE
	mDirServers = new WONIPAddress[3];
	WONIPAddressSetFromStringAndPort(mDirServers + mNumDirServers++, "wontest.east.won.net", 15101);
	WONIPAddressSetFromStringAndPort(mDirServers + mNumDirServers++, "wontest.central.won.net", 15101);
	WONIPAddressSetFromStringAndPort(mDirServers + mNumDirServers++, "wontest.west.won.net", 15101);
	WONDirShuffleDirectoryServerArray(mDirServers, 3);
#else
	mDirServers = new IPSocket::Address[3];
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.east.won.net", 15101);
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.central.won.net", 15101);
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.west.won.net", 15101);
	ShuffleDirectoryServerArray(mDirServers, 3);
#endif // DLLSAMPLE
}

void Matchmaker::GetMOTD()
{
#ifdef DLLSAMPLE
	WONIPAddress proxyServer;
	WONIPAddressSetFromStringAndPort(&proxyServer, ADDR_MOTDSERVER, PORT_MOTDSERVER);
	WONError aError = WONHTTPGetFile(&proxyServer, ADDR_MOTDSERVER, PORT_MOTDSERVER, "/motd/sample/motd.txt", "motd.txt", NULL, NULL, TRUE, NULL, NULL, gRequestTimeout);
#else
	Error aError = HTTPGet(ADDR_MOTDSERVER, PORT_MOTDSERVER, "/motd/sample/motd.txt", "motd.txt", NULL, NULL, true, NULL, NULL, gRequestTimeout, false);
#endif // DLLSAMPLE

	switch (aError)
	{
		case StatusCommon_Success:
		{
			struct stat aStatStruct;
			stat("motd.txt", &aStatStruct);
			char timebuf[22];
			strftime(timebuf, 22, "%b %d, %Y %H:%M:%S", localtime(&aStatStruct.st_mtime));
			OutputStringF("Message of the Day (%s)", timebuf);
			OutputFile("motd.txt");
			break;
		}
		default:
			OutputError("Failed to download Message of the Day", aError);
	}
}

void Matchmaker::GetPatch()
{
#ifdef DLLSAMPLE
	WONIPAddress proxyServer;
	WONIPAddressSetFromStringAndPort(&proxyServer, ADDR_PATCHSERVER, PORT_PATCHSERVER);
	WONHTTPGetFile(&proxyServer, ADDR_PATCHSERVER, PORT_PATCHSERVER, "SamplePatch.txt", "SamplePatch.txt", NULL, NULL, true, PatchProgressCallback, NULL, gRequestTimeout);
#else
	HTTPGet(ADDR_PATCHSERVER, PORT_PATCHSERVER, "SamplePatch.txt", "SamplePatch.txt", NULL, NULL, true, (ProgressCallback)PatchProgressCallback, NULL, gRequestTimeout, true, (void(*)(Error))PatchDownloadCallback);
#endif // DLLSAMPLE
}

#ifdef DLLSAMPLE
void CopySTLAddressListToArray(const std::list<WONIPAddress>& theListR, WONIPAddress** theArrayPP, unsigned int* theArraySizeP)
{
	delete[] *theArrayPP; *theArraySizeP = 0;
	*theArrayPP = new WONIPAddress[theListR.size()];
	for (std::list<WONIPAddress>::const_iterator itr = theListR.begin(); itr != theListR.end(); ++itr)
		(*theArrayPP)[(*theArraySizeP)++] = *itr;
}
#endif // DLLSAMPLE

void Matchmaker::GetTitanServerList()
{
#ifdef DLLSAMPLE
	mNewAuthServers.clear();
	mNewContestServers.clear();
	mNewEventServers.clear();
	mNewFirewallServers.clear();
	mNewProfileServers.clear();
	
	HWONDATAOBJECT aDataObjectH = WONDataObjectCreate(OBJ_VALIDVERSIONS.c_str(), OBJ_VALIDVERSIONS.size(), NULL, 0);
	WONError aError = WONDirGetDirectoryW(NULL, mDirServers, mNumDirServers,
	                                      DIR_TITANSERVER, NULL, NULL,
	                                      WONDir_GF_DECOMPROOT | WONDir_GF_DECOMPRECURSIVE | WONDir_GF_DECOMPSERVICES | WONDir_GF_ADDTYPE | WONDir_GF_SERVADDNAME | WONDir_GF_SERVADDNETADDR | WONDir_GF_ADDDOTYPE | WONDir_GF_ADDDODATA,
	                                      &aDataObjectH, 1, TitanServerEntityCallback, this, gRequestTimeout);

	CopySTLAddressListToArray(mNewAuthServers,     &mAuthServers,     &mNumAuthServers);
	CopySTLAddressListToArray(mNewContestServers,  &mContestServers,  &mNumContestServers);
	CopySTLAddressListToArray(mNewEventServers,    &mEventServers,    &mNumEventServers);
	CopySTLAddressListToArray(mNewFirewallServers, &mFirewallServers, &mNumFirewallServers);
	CopySTLAddressListToArray(mNewProfileServers,  &mProfileServers,  &mNumProfileServers);
#else
	DataObjectTypeSet aDataObjectSet;
	aDataObjectSet.insert(WONCommon::DataObject(OBJ_VALIDVERSIONS));
	WONMsg::DirEntityList entityList;
	Error aError = GetDirectory(NULL, // no identity needed to get TitanServers (after all, the AuthServers are listed in there)
								mDirServers, mNumDirServers,
								NULL,
								DIR_TITANSERVER, 
								&entityList,
								WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPRECURSIVE | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA,
								aDataObjectSet,
								0, 0,
								gRequestTimeout);

	switch(aError) 
	{
		case Error_Success:
		{
			delete[] mAuthServers; mNumAuthServers = 0;
			mAuthServers = new IPSocket::Address[entityList.size()];
			delete[] mContestServers; mNumContestServers = 0;
			mContestServers = new IPSocket::Address[entityList.size()];
			delete[] mEventServers; mNumEventServers = 0;
			mEventServers = new IPSocket::Address[entityList.size()];
			delete[] mFirewallServers; mNumFirewallServers = 0;
			mFirewallServers = new IPSocket::Address[entityList.size()];
			delete[] mProfileServers; mNumProfileServers = 0;
			mProfileServers = new IPSocket::Address[entityList.size()];

			DirEntityList::const_iterator aDirEntityListItr = entityList.begin();
			for( ; aDirEntityListItr != entityList.end(); ++aDirEntityListItr)
			{
				if (aDirEntityListItr->mType == WONMsg::DirEntity::ET_DIRECTORY)
				{
					DataObjectTypeSet::const_iterator aDataObjectSetItr = aDirEntityListItr->mDataObjects.begin();
					for( ; aDataObjectSetItr != aDirEntityListItr->mDataObjects.end(); ++aDataObjectSetItr)
					{
						if (aDataObjectSetItr->GetDataType() == OBJ_VALIDVERSIONS)
							mValidVersions = reinterpret_cast<const char*>(aDataObjectSetItr->GetData().c_str());
					}
				}
				else if (aDirEntityListItr->mName == SERVERNAME_AUTH)
					mAuthServers[mNumAuthServers++] = IPSocket::Address(*aDirEntityListItr);
				else if (aDirEntityListItr->mName == SERVERNAME_CONTEST)
					mContestServers[mNumContestServers++] = IPSocket::Address(*aDirEntityListItr);
				else if (aDirEntityListItr->mName == SERVERNAME_EVENT)
					mEventServers[mNumEventServers++] = IPSocket::Address(*aDirEntityListItr);
				else if (aDirEntityListItr->mName == SERVERNAME_FIREWALL)
					mFirewallServers[mNumFirewallServers++] = IPSocket::Address(*aDirEntityListItr);
				else if (aDirEntityListItr->mName == SERVERNAME_PROFILE)
					mProfileServers[mNumProfileServers++] = IPSocket::Address(*aDirEntityListItr);
			}
			break;
		}

		case StatusDir_DirNotFound:
			OutputError("Directory containing Titan servers not found");
			break;
		default:
			OutputError("Failed to get list of Titan servers!", aError);
			break;
	}
#endif // DLLSAMPLE
}

bool Matchmaker::IsValidVersion(const char* version)
{
	if (mValidVersions.empty())
		return true;
	
	int lastpos = -1;
	while (TRUE)
	{
		int pos = mValidVersions.find_first_of("\t", lastpos+1);
		if (mValidVersions.substr(lastpos+1,(pos == mValidVersions.npos ? pos : pos-lastpos-1)).compare(version) == 0)
			return true;
		if (pos == mValidVersions.npos)
			return false;
		lastpos = pos;
	}
}

void Matchmaker::CreateWONAccount(const char* theUserName, const char* thePassword)
{
#ifdef DLLSAMPLE
	WONError aError;
	if (mAuthH) { WONAuthCloseHandle(mAuthH); mAuthH = NULL; }
	mAuthH = WONAuthLoginNewAccountA(&aError, mAuthServers, mNumAuthServers, theUserName, COMMUNITY_SAMPLE, thePassword, "", gRequestTimeout);
#else
	mIdentity = Identity(theUserName, COMMUNITY_SAMPLE, thePassword, "", mAuthServers, mNumAuthServers);
	Error aError = mIdentity.AuthenticateNewAccount(gRequestTimeout);
#endif // DLLSAMPLE

	switch (aError)
	{
		case StatusCommon_Success:
			OutputError("Successfully created new account and logged in");

#ifdef DLLSAMPLE
			WONProfileCreate(mAuthH, mProfileServers, mNumProfileServers, "sdk@won.net", gRequestTimeout);
#else
			CreateProfile(&mIdentity, mProfileServers, mNumProfileServers, "sdk@won.net", gRequestTimeout);
#endif

			ListRooms();
			break;
		case StatusAuth_CDKeyInUse:
			OutputError("CD key is already in use");
			break;
		case StatusAuth_CRCFailed:
			OutputError("Invalid version of game");
			break;
		case StatusAuth_UserExists:
			OutputError("User already exists");
			break;
		default:
			OutputError("Account creation failed!", aError);
			break;
	}
}

void Matchmaker::SaveEmailAddressInProfile(bool reset)
{
#ifdef DLLSAMPLE
}
#else
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumProfileServers == 0)
		OutputString("No profile servers available.");
	else
	{
		// if this is an initial call (not a failover call), reset should be true,
		// and we set mCurProfileServer to 0 so we run through all of the profile
		// servers again.
		if (reset)
		{
			mNumProfileServersTried = 0;
			mCurProfileServer = time(NULL) % mNumProfileServers;
		}
		else
		{
			++mNumProfileServersTried;
			++mCurProfileServer;
			mCurProfileServer %= mNumProfileServers;
		}

		// quit when we've failed on all of the profile servers
		if (mNumProfileServersTried >= mNumProfileServers)
		{
			OutputStringF("Failed on all %d Profile Servers.", mNumProfileServers);
			return;
		}

		TCPSocket* aProfileSocketP = new TCPSocket(mProfileServers[mCurProfileServer]);
		AuthSocket* aAuthSocketP = new AuthSocket(&mIdentity, aProfileSocketP, true, 2, false, false);

		// check status of current profile:
		// Case 1: No profile exists for the user.
		// Case 2: Profile exists, but has no email address.
		// Case 3: Profile exists and contains an email address.
		SMsgProfileReadRequest aReadRequest;
		aReadRequest.SetField(PROFILE_EMAIL1);
		aAuthSocketP->SendBaseMsgRequestEx(aReadRequest, NULL, false, true, 2000, -1, true, true, (void(*)(const AuthSocket::AuthRecvBaseMsgResult&, Matchmaker*))ProfileQueryCallback, this);
	}
}

// Matchmaker::ProfileQueryCallback
// Handle SMsgProfileReadResponse
void Matchmaker::ProfileQueryCallback(const AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that)
{
	((AuthSocket*)theResultR.theSocket)->GetSocket()->Close();
	if (theResultR.msg)
	{
		auto_ptr<BaseMessage> autoMsg(theResultR.msg);
		SMsgProfileReadResponse aReadResponse(*(SmallMessage*)theResultR.msg);
		switch (aReadResponse.GetErrorCode())
		{
			case WONMsg::StatusCommon_Success:
			{
				wstring aEmail1;
				aReadResponse.GetField(PROFILE_EMAIL1, aEmail1);
				if (aEmail1.empty())
				{
					// Case 2: Profile exists, but has no email address -- Update the profile
					SMsgProfileUpdateRequest aUpdateRequest;
					aUpdateRequest.SetField(PROFILE_EMAIL1, L"sdk@won.net");
					((AuthSocket*)theResultR.theSocket)->SendBaseMsgRequestEx(aUpdateRequest, NULL, false, true, 2000, -1, true, true, (void(*)(const AuthSocket::AuthRecvBaseMsgResult&, Matchmaker*))ProfileUpdateCallback, that);
				}
				else
				{
					// Case 3: Profile exists and contains an email address -- nothing to do
					delete (AuthSocket*)theResultR.theSocket;
					OutputString("Profile already contains an email address.");
				}
				return;
			}
			case WONMsg::StatusCommon_NotAvailable:
			{
				// Case 1: No profile exists for the user -- Create a profile
				SMsgProfileCreateRequest aCreateRequest;
				aCreateRequest.SetField(PROFILE_EMAIL1, L"sdk@won.net");
				((AuthSocket*)theResultR.theSocket)->SendBaseMsgRequestEx(aCreateRequest, NULL, false, true, 2000, -1, true, true, (void(*)(const AuthSocket::AuthRecvBaseMsgResult&, Matchmaker*))ProfileCreateCallback, that);
				return;
			}
			default:
				OutputError("Profile query failed!", aReadResponse.GetErrorCode());
				break;
		}
	}
	else
		OutputString("Didn't receive a response to SMsgProfileReadRequest.");

	// fail over to next profile server
	delete (AuthSocket*)theResultR.theSocket;
	that->SaveEmailAddressInProfile(false);
}

// Matchmaker::ProfileUpdateCallback
// Handle SMsgProfileUpdateResponse
void Matchmaker::ProfileUpdateCallback(const AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that)
{
	((AuthSocket*)theResultR.theSocket)->GetSocket()->Close();
	if (theResultR.msg)
	{
		auto_ptr<BaseMessage> autoMsg(theResultR.msg);
		SMsgProfileUpdateResponse aUpdateResponse(*(SmallMessage*)theResultR.msg);
		if (aUpdateResponse.GetErrorCode() == StatusCommon_Success)
		{
			OutputString("Successfully updated profile.");
			delete (AuthSocket*)theResultR.theSocket;
			return;
		}
		else
			OutputError("Failed to update profile", aUpdateResponse.GetErrorCode());
	}
	else
		OutputString("Didn't receive a response to SMsgProfileUpdateRequest.");

	// fail over to next profile server
	delete (AuthSocket*)theResultR.theSocket;
	that->SaveEmailAddressInProfile(false);
}

// Matchmaker::ProfileCreateCallback
// Handle SMsgProfileCreateResponse
void Matchmaker::ProfileCreateCallback(const AuthSocket::AuthRecvBaseMsgResult& theResultR, Matchmaker* that)
{
	((AuthSocket*)theResultR.theSocket)->GetSocket()->Close();
	if (theResultR.msg)
	{
		auto_ptr<BaseMessage> autoMsg(theResultR.msg);
		SMsgProfileCreateResponse aCreateResponse(*(SmallMessage*)theResultR.msg);
		if (aCreateResponse.GetErrorCode() == StatusCommon_Success)
		{
			OutputString("Successfully created profile.");
			delete (AuthSocket*)theResultR.theSocket;
			return;
		}
		else
			OutputError("Failed to create profile", aCreateResponse.GetErrorCode());
	}
	else
		OutputString("Didn't receive a response to SMsgProfileCreateRequest.");

	// fail over to next profile server
	delete (AuthSocket*)theResultR.theSocket;
	that->SaveEmailAddressInProfile(false);
}
#endif // DLLSAMPLE

void Matchmaker::Login(const char* theUsernameP, const char* thePasswordP)
{
#ifdef DLLSAMPLE
	WONError aError;
	if (mAuthH) { WONAuthCloseHandle(mAuthH); mAuthH = NULL; }
	mAuthH = WONAuthLoginA(&aError, mAuthServers, mNumAuthServers, theUsernameP, COMMUNITY_SAMPLE, thePasswordP, "", gRequestTimeout);
#else
	mIdentity = Identity(theUsernameP, COMMUNITY_SAMPLE,thePasswordP, "", mAuthServers, mNumAuthServers);
	Error aError = mIdentity.Authenticate(false , false, gRequestTimeout);
#endif // DLLSAMPLE

	switch (aError)
	{
		case StatusCommon_Success:
		{
			OutputError("Successfully logged in");
#ifndef DLLSAMPLE
			SaveEmailAddressInProfile();
#endif
			ListRooms();
			break;
		}
		case StatusAuth_UserNotFound:
			OutputError("Bad username");
			break;
		case StatusAuth_BadPassword:
			OutputError("Incorrect password");
			break;
		case StatusAuth_CDKeyInUse:
			OutputError("CD key is already in use");
			break;
		case StatusAuth_CRCFailed:
			OutputError("Invalid version of game");
			break;
		default:
			OutputError("Login failed!", aError);
			break;
	}
}

void Matchmaker::DetectFirewall()
{
#ifdef DLLSAMPLE
	HWONCOMPLETION aCompletionH = WONCreateCallbackWithDataCompletion(DetectFirewallCallback, this);
	WONFirewallDetectAsync(mFirewallServers, mNumFirewallServers, &this->mBehindFirewall, GAME_PORT, gRequestTimeout, aCompletionH);
#else
	WONAPI::DetectFirewallEx(mFirewallServers, mNumFirewallServers,
	                         &this->mBehindFirewall, GAME_PORT, 
	                         gRequestTimeout, true, (void(*)(const DetectFirewallResult&, Matchmaker*))DetectFirewallCallback, this);
#endif // DLLSAMPLE
}

void Matchmaker::ListRooms()
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else
	{
#ifdef DLLSAMPLE
		mChatServers.clear(); mNumChatServers = 0;
		mNewFactoryServers.clear();
		
		HWONDATAOBJECT aObjects[3];
		aObjects[0] = WONDataObjectCreate(OBJ_CHATROOM_CLIENTCOUNT.c_str(), OBJ_CHATROOM_CLIENTCOUNT.size(), NULL, 0);
		aObjects[1] = WONDataObjectCreate(OBJ_CHATROOM_LOGINACL.c_str(), OBJ_CHATROOM_LOGINACL.size(), NULL, 0);
		aObjects[2] = WONDataObjectCreate(OBJ_CHATROOM_ROOMFLAGS.c_str(), OBJ_CHATROOM_ROOMFLAGS.size(), NULL, 0);
		HWONCOMPLETION aCompletionH = WONCreateCallbackWithDataCompletion(ChatRoomListCallback, this);
		WONDirGetDirectoryAsyncW(mAuthH, mDirServers, mNumDirServers,
		                         DIR_APISAMPLE, NULL, NULL,
		                         WONDir_GF_DECOMPSERVICES | WONDir_GF_ADDTYPE | WONDir_GF_ADDDISPLAYNAME | WONDir_GF_SERVADDNAME | WONDir_GF_SERVADDNETADDR | WONDir_GF_ADDDODATA | WONDir_GF_ADDDOTYPE, 
		                         aObjects, 3,
		                         ChatRoomEntityCallback, this, gRequestTimeout,
		                         aCompletionH);
#else
		WONCommon::DataObjectTypeSet aDataObjectSet;
		aDataObjectSet.insert(DataObject(OBJ_CHATROOM_CLIENTCOUNT));
		aDataObjectSet.insert(DataObject(OBJ_CHATROOM_LOGINACL));
		aDataObjectSet.insert(DataObject(OBJ_CHATROOM_ROOMFLAGS));
		Error aError = GetDirectoryEx(&mIdentity,
		                              mDirServers, mNumDirServers,
		                              NULL, // don't care where data came from
		                              DIR_APISAMPLE, 
		                              NULL,
		                              WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_ADDDISPLAYNAME | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDODATA | WONMsg::GF_ADDDOTYPE, 
		                              aDataObjectSet,
		                              0, 0, // Callback
		                              gRequestTimeout, // timeout (msec)
		                              false, // true,  // change to true to query room list asynchronously
		                              (void(*)(const DirEntityListResult&, Matchmaker*))ChatRoomListCallback, this);
#endif // DLLSAMPLE
	}
}

void Matchmaker::CreateRoom(const char* theRoomName, const wchar_t* thePassword)
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumFactoryServers == 0)
		OutputString("No factory servers available to start room.");
	else
	{
		wstring aCommandLine;
		if (thePassword && thePassword[0] != '\0')
		{
			aCommandLine += L" -Password \"";
			aCommandLine += thePassword;
			aCommandLine += L"\"";
		}
		
		mRoomBeingStarted = theRoomName;
		mPasswordAttempt = thePassword;
		unsigned short startedOnPort;
#ifdef DLLSAMPLE
		WONIPAddress startedOnFactory;
		WONError aError = WONFactoryStartTitanServerW(mAuthH, mFactoryServers, mNumFactoryServers,
									 &startedOnFactory, CONFIG_SAMPLECHATSERVER,
									 &startedOnPort, NULL,
									 aCommandLine.c_str(), false,
									 1, 0, NULL, NULL, 0, gRequestTimeout);

#else
		IPSocket::Address startedOnFactory;
		Error aError = StartTitanServer(&mIdentity, mFactoryServers, mNumFactoryServers, 
										&startedOnFactory, CONFIG_SAMPLECHATSERVER,
										&startedOnPort, NULL,
										aCommandLine, false,
										1, 0, NULL, NULL, 0, gRequestTimeout);
#endif // DLLSAMPLE

		switch (aError) 
		{
			case StatusCommon_Success:
			{
				mCurChatServerAddr = startedOnFactory;
#ifdef DLLSAMPLE
				WONIPAddressSetPort(&mCurChatServerAddr, startedOnPort);
				char aAddressStringBuf[22];
				unsigned short aAddressStringBufLen = 22;
				WONIPAddressGetDottedIP(&mCurChatServerAddr, aAddressStringBuf, &aAddressStringBufLen, TRUE);
#else
				mCurChatServerAddr.SetPort(startedOnPort);
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
				aError = WONDirRegisterTitanServerW(&mCurChatServerAddr, 2, mDirServers, mNumDirServers,
				                                    DIR_APISAMPLE, StringToWString(theRoomName).c_str(), TRUE, NULL, 0, gRequestTimeout);
#else
				aError = RegisterTitanServer(mCurChatServerAddr, 2, mDirServers, mNumDirServers, DIR_APISAMPLE, theRoomName);
#endif // DLLSAMPLE
				switch (aError)
				{
					case StatusCommon_Success:
					{
#ifdef DLLSAMPLE
						OutputStringF("Started chat server %s", aAddressStringBuf);
#else
						OutputStringF("Started chat server %s", mCurChatServerAddr.GetAddressString().c_str());
#endif // DLLSAMPLE 
						wstring aRoomName = WONCommon::StringToWString(theRoomName);
						ChatServerInfo aChatServerInfo;
						aChatServerInfo.mAddress = mCurChatServerAddr;
						aChatServerInfo.mIsPasswordProtected = (thePassword && thePassword[0] != '\0');
						mChatServers[aRoomName] = aChatServerInfo;
						mCurChatServerName = aRoomName;
						JoinChatRoom();
						break;
					}
					case StatusCommon_InvalidParameters:
						OutputError("Request format is invalid");
						break;
					case WONMsg::StatusDir_ServiceExists:
						OutputError("Duplicate chat room name.  Please try again with a different name.");
						break;
					default:
						OutputError("Failed to register chat server!", aError);
						break;
				}
				break;				
			}
			case StatusCommon_InvalidParameters:
				OutputError("Request format is invalid");
				break;
			case StatusCommon_ResourcesLow:
				OutputError("Can not create chat server");
				break;
			default:
				OutputError("Failed to start chat server!", aError);
				break;
		}
	}
}

void Matchmaker::JoinRoom(const char* theRoomName, const wchar_t* thePassword)
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else
	{
		wstring aRoomName = StringToWString(theRoomName);
		if (aRoomName == mCurChatServerName.GetUnicodeString())
			OutputError("You are already in the specified room");
		else
		{
			map<wstring, ChatServerInfo>::const_iterator itr = mChatServers.find(aRoomName);
			if (itr == mChatServers.end())
				OutputError("Specified room not found");
			else
			{
				Close();
				mCurChatServerName = aRoomName;
				mCurChatServerAddr = itr->second.mAddress;
				mPasswordAttempt = thePassword;
				JoinChatRoom();	
			}
		}
	}
}

void Matchmaker::JoinChatRoom()
{
	// cleanup previous Routing Server connection (if any)
	mClientMap.clear();
#ifdef DLLSAMPLE
	WONRoutingCloseConnection(mRoutingServer);
#else
	mRoutingServer.Close();
#endif // DLLSAMPLE

	// connect to the new server
#ifdef DLLSAMPLE
	WONError aError = WONRoutingConnect(mRoutingServer, &mCurChatServerAddr, mAuthH, gRequestTimeout);
#else
	Error aError = mRoutingServer.Connect(mCurChatServerAddr, &mIdentity, false, gRequestTimeout);//, true, (void(*)(const Socket::OpenResult&, Matchmaker*))ChatConnectCallback, this);
#endif // DLLSAMPLE

	switch (aError)
	{
		case StatusCommon_Success:
			{
				OutputError("Connected to chat room");

				// login
#ifdef DLLSAMPLE
				WONRoutingRegisterW(mRoutingServer, WONAuthGetNameW(mAuthH), mPasswordAttempt.c_str(), FALSE, FALSE, TRUE, ChatRoomLoginCallback, this);
#else
				RawBuffer aClientName((unsigned char*)mIdentity.GetLoginName().GetTitanUnicodeString().data(), mIdentity.GetLoginName().GetTitanUnicodeString().size() * 2);
				mRoutingServer.RegisterEx(aClientName, mPasswordAttempt, false, false, true /* setup chat */, (void(*)(const RoutingServerClient::RegisterClientResult&, Matchmaker*))ChatRoomLoginCallback, this);
#endif // DLLSAMPLE
			}
			break;
#ifdef DLLSAMPLE
		case WONError_HostUnreachable:
#else
		case Error_HostUnreachable:
#endif // DLLSAMPLE
			OutputError("Chat server unreachable");
			Close();
			break;
		default:
			OutputServerError("Failed to connect to chat server", mCurChatServerAddr, aError);
			Close();
	}

}

void Matchmaker::ListUsers()
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else
	{
		ClientMap::const_iterator itr = mClientMap.begin();
		for ( ; itr != mClientMap.end(); ++itr)
		{
			// There's only one NULL character on the end of the string rather than two.  If we leave it like that, 
			// code further down stream won't be able to accurately determine its length.
#ifdef DLLSAMPLE
			OutputString(itr->second.mClientNameW);
#else
			WONString aWideUserName(reinterpret_cast<const wchar*>(itr->second.mClientName.data()), itr->second.mClientName.size() / 2);
			OutputString(aWideUserName.GetUnicodeCString());
#endif // DLLSAMPLE
		}
	}
}

void Matchmaker::CreateGame(const char* theGameName, const wchar_t* thePassword)
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (!mGameInfo.mMyGameName.empty())
		OutputError("You're already in a game.");
	else
	{
		if (thePassword != NULL && thePassword[0] != '\0')
			OutputString("FYI: Game passwords are not currently supported");

		RawBuffer aObjectName(OBJ_GAMEPREFIX + (unsigned char*)theGameName);

		RawBuffer aObject;
#ifdef DLLSAMPLE
		mGameInfo.mPlayersInGame[0] = WONRoutingGetClientId(mRoutingServer);
#else
		mGameInfo.mPlayersInGame[0] = mRoutingServer.GetClientId();
#endif // DLLSAMPLE
		mGameInfo.mNumPlayersInGame = 1;
		mGameInfo.mMyGameName = theGameName;
		aObject.assign((unsigned char*)mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
			sizeof(WONRoutingClientId));
#else
			sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
		WONRoutingCreateDataObjectAndGetResult(mRoutingServer, GROUPID_ALLUSERS, aObjectName.data(), aObjectName.size(), WONRoutingGetClientId(mRoutingServer), 0, aObject.data(), aObject.size(), CreateDataObjectCallback, this);
#else
		mRoutingServer.CreateDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aObjectName, mRoutingServer.GetClientId(), 0, aObject, (void(*)(short, Matchmaker*))CreateDataObjectCallback, this);
#endif // DLLSAMPLE

		// initialize score info
		mScoreSoFar.Clear(true);
#ifdef DLLSAMPLE
		mScoreSoFar.mPlayer1.mUserSeq = WONAuthGetUserID(mAuthH);
#else
		mScoreSoFar.mPlayer1.mUserSeq = mIdentity.GetUserId();
#endif // DLLSAMPLE
	}
}

void Matchmaker::JoinGame(const char* theGameName, const wchar_t* thePassword)
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else
	{
		if (thePassword != NULL && thePassword[0] != '\0')
			OutputString("FYI: Game passwords are not currently supported");

		GameMap::const_iterator itr = mGameMap.find(theGameName);
		if (itr != mGameMap.end())
		{
			OutputString("Sending join request.");
			JoinRequestMsg aJoinRequest;
			RawBuffer aMsg;
            aMsg.assign((unsigned char*)&aJoinRequest, sizeof(aJoinRequest));
#ifdef DLLSAMPLE
			WONRoutingSendData(mRoutingServer, &(itr->second.mOwnerId), 1, TRUE, aMsg.data(), aMsg.size());
#else
            mRoutingServer.SendData(&(itr->second.mOwnerId), 1, true, aMsg, false);		
#endif // DLLSAMPLE
		}
		else
			OutputError("Specified game not found");
	}
}

void Matchmaker::LeaveGame()
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (!IsInAGame())
		OutputError("Not currently in a game.");
	else if (mGameInfo.IAmCaptain())
	{
		// remove the game from the game list
		RawBuffer aObjectType(OBJ_GAMEPREFIX + (unsigned char*)mGameInfo.mMyGameName.c_str());
#ifdef DLLSAMPLE
		WONRoutingDeleteDataObject(mRoutingServer, GROUPID_ALLUSERS, aObjectType.data(), aObjectType.size());
#else
		mRoutingServer.DeleteDataObject(RoutingServerClient::GROUPID_ALLUSERS, aObjectType);
#endif // DLLSAMPLE

		// tell current players that the game has been dissolved
		GameDissolvedMsg aGameDissolved;
		RawBuffer aMsg;
		aMsg.assign((unsigned char*)&aGameDissolved, sizeof(aGameDissolved));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
		mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
		mGameInfo.Reset();

		// save results via Contest Server
		ReportResults();

		OutputString("Game dissolved.");
	}
	else
	{
		LeaveGameMsg aLeaveGame;
		RawBuffer aMsg;
        aMsg.assign((unsigned char*)&aLeaveGame, sizeof(aLeaveGame));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, &mGameInfo.mCaptainId, 1, TRUE, aMsg.data(), aMsg.size());
#else
        mRoutingServer.SendData(&mGameInfo.mCaptainId, 1, true, aMsg, false);
#endif // DLLSAMPLE
		mGameInfo.Reset();
		OutputString("Ok");
	}
}

void Matchmaker::ListGames()
{
#ifdef DLLSAMPLE
	if (!WONRoutingIsRegistered(mRoutingServer))
#else
	if (!mRoutingServer.IsRegistered())
#endif // DLLSAMPLE
		OutputError("Not connected to a chat room.");
	else if (mGameMap.size() == 0)
		OutputError("No games available.");
	else
	{
		GameMap::const_iterator itr = mGameMap.begin();
		OutputString("Name\tPlayer Count");
		for ( ; itr != mGameMap.end(); ++itr)
#ifdef DLLSAMPLE
			OutputStringF("%s\t%d", itr->first.c_str(), itr->second.mDataLen/2);
#else
			OutputStringF("%s\t%d", itr->first.c_str(), itr->second.mData.size()/2);
#endif // DLLSAMPLE
	}
}

#ifdef DLLSAMPLE
void Matchmaker::AcceptMove(unsigned char thePlayer, WONRoutingClientId theClientId, unsigned int theX, unsigned int theY)
#else
void Matchmaker::AcceptMove(unsigned char thePlayer, RoutingServerClient::ClientId theClientId, unsigned int theX, unsigned int theY)
#endif // DLLSAMPLE
{
	MoveAcceptedMsg aMoveAccepted(theClientId, thePlayer, theX, theY);
	RawBuffer aMsg;
    aMsg.assign((unsigned char*)&aMoveAccepted, sizeof(aMoveAccepted));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
    mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
	
	mGameInfo.mWhoseTurnIsIt = (++mGameInfo.mWhoseTurnIsIt) % mGameInfo.mNumPlayersInGame;
	YourTurnMsg aYourTurn;
	aMsg.assign((unsigned char*)&aYourTurn, sizeof(aYourTurn));
#ifdef DLLSAMPLE
	WONRoutingSendData(mRoutingServer, &mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt], 1, TRUE, aMsg.data(), aMsg.size());
#else
    mRoutingServer.SendData(&mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt], 1, true, aMsg, false);
#endif // DLLSAMPLE
}

void Matchmaker::StartGame()
{
	if (!mGameInfo.IAmCaptain())
		OutputError("You're not the captain of a/the game");
	else if (mGameInfo.mNumPlayersInGame != TicTacToe::MAX_PLAYERS_PER_GAME)
		OutputError("Waiting for more players...");
	else if (mGameInfo.mGameHasStarted)
		OutputError("Game has already been started.");
	else
	{
		mGameInfo.mGameHasStarted = true;
		mGameInfo.mWhoseTurnIsIt = 0;
		StartGameMsg aStartGame;
		RawBuffer aMsg;
        aMsg.assign((unsigned char*)&aStartGame, sizeof(aStartGame));
#ifdef DLLSAMPLE
		WONRoutingSendData(mRoutingServer, mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
        mRoutingServer.SendData(mGameInfo.mPlayersInGame, mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE
	}
}

void Matchmaker::Move(const char* theMove)
{
	if (!IsInAGame())
		OutputString("Not currently in a game.");
	else if (strlen(theMove) != 2)
		OutputError("Invalid move.  Try again");
	else
	{
		char aColumn = toupper(theMove[0]);
		char aRow = theMove[1];

		if (aColumn < 'A' || aColumn > 'Z' || 
			aRow < '1'    || aRow > '9')
			OutputError("Invalid move.  Try again");
		else
		{
			unsigned int aX = aColumn - 'A';
			unsigned int aY = aRow - '1';
			if (!mGameInfo.IAmCaptain())
			{
				MoveRequestMsg aMoveRequest(aX, aY);
				RawBuffer aMsg;
				aMsg.assign((unsigned char*)&aMoveRequest, sizeof(aMoveRequest));
#ifdef DLLSAMPLE
				WONRoutingSendData(mRoutingServer, &mGameInfo.mCaptainId, 1, TRUE, aMsg.data(), aMsg.size());
#else
				mRoutingServer.SendData(&mGameInfo.mCaptainId, 1, true, aMsg, false);		
#endif // DLLSAMPLE
			}
			else
			{
#ifdef DLLSAMPLE
				if (mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt] != WONRoutingGetClientId(mRoutingServer) || !mGameInfo.ValidateMove(aX, aY))
#else
				if (mGameInfo.mPlayersInGame[mGameInfo.mWhoseTurnIsIt] != mRoutingServer.GetClientId() || !mGameInfo.ValidateMove(aX, aY))
#endif // DLLSAMPLE
				{
					OutputString("Invalid move (or maybe it's not your turn?)");
					return;
				}
#ifdef DLLSAMPLE
				AcceptMove(PLAYER_O, WONRoutingGetClientId(mRoutingServer), aX, aY);
#else
				AcceptMove(PLAYER_O, mRoutingServer.GetClientId(), aX, aY);
#endif // DLLSAMPLE
			}
		}
	}
}

void Matchmaker::DisplayTopTen()
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumFactoryServers == 0)
		OutputString("No contest servers available.");
	else
	{
		TopTenList aTopTenList;
		memset(&aTopTenList, 0, sizeof(aTopTenList));
		unsigned short aRecvSize = sizeof(aTopTenList);

		// retrieve top ten list
		GUID aGuid;	memset(&aGuid,0,sizeof(aGuid));
#ifdef DLLSAMPLE
		WONError aError = WONContestDBCall(mContestServers, mNumContestServers, mAuthH,
										   gRequestTimeout, CONTESTPROC_TOPTENLIST, &aGuid, 
										   0, NULL, &aRecvSize, &aTopTenList);
#else
		Error aError = ContestDBCall(&mIdentity, mContestServers, mNumContestServers, 
 					  CONTESTPROC_TOPTENLIST, &aGuid,
					  0, NULL, &aRecvSize, &aTopTenList, 
					  gRequestTimeout, false);
#endif // DLLSAMPLE

		switch (aError)
		{
			case StatusCommon_Success:
			{
				// display the list
				OutputString ("    Name            Wins Losses Ties");
				for (int i = 0; i < aTopTenList.mNumRecords; ++i)
					OutputStringF("%2d. %-15s %4d  %4d  %4d", i+1, aTopTenList.mRecords[i].mName, aTopTenList.mRecords[i].mWins, aTopTenList.mRecords[i].mLosses, aTopTenList.mRecords[i].mTies);
				break;
			}
			default:
				OutputError("Failed to get top ten list", aError);
				break;
		}
	}
}

void Matchmaker::ReportResults()
{
	if (!IsLoggedIn())
		OutputError("Need username and password.");
	else if (mNumFactoryServers == 0)
		OutputString("No contest servers available.");
	else
	{
		GUID aGuid;	memset(&aGuid,0,sizeof(aGuid));
#ifdef DLLSAMPLE
		HWONCOMPLETION aCompletionH = WONCreateCallbackWithDataCompletion(ReportResultsCallback, this);
		WONContestDBCallAsync(mContestServers, mNumContestServers, mAuthH, 
							  gRequestTimeout, CONTESTPROC_REPORTRESULT,
							  &aGuid, sizeof(ScoreReport), &mScoreSoFar,
							  NULL, NULL, aCompletionH);
#else
		Error aError = ContestDBCallEx(&mIdentity, mContestServers, mNumContestServers, 
 					  CONTESTPROC_REPORTRESULT, &aGuid,
					  sizeof(ScoreReport), &mScoreSoFar, NULL, NULL, 
					  gRequestTimeout, true, (void(*)(const ContestDBResult&, Matchmaker*))ReportResultsCallback, this);
#endif // DLLSAMPLE
	}
}

void Matchmaker::IgnoreUser(const char* theUserName)
{
#ifdef DLLSAMPLE
		WONRoutingIgnoreClientA(mRoutingServer, theUserName);
#else
		mRoutingServer.IgnoreClient((unsigned char*)theUserName);
#endif // DLLSAMPLE
}

void Matchmaker::UnignoreUser(const char* theUserName)
{
#ifdef DLLSAMPLE
		WONRoutingUnignoreClientA(mRoutingServer, theUserName);
#else
		mRoutingServer.UnignoreClient((unsigned char*)theUserName);
#endif // DLLSAMPLE
}

//
// CALLBACKS
//
#ifdef DLLSAMPLE
void Matchmaker::DetectFirewallCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP)
{
	WONError aError = reinterpret_cast<WONError>(param);
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
#else
void Matchmaker::DetectFirewallCallback(const DetectFirewallResult& theResultR, Matchmaker* that)
{
	Error aError = theResultR.error;
#endif // DLLSAMPLE
	if (aError == StatusCommon_Success)
	{
		if (that->mBehindFirewall)
			OutputString("You are behind a firewall.");
		else
			OutputString("You are not behind a firewall.");
	}
	else
		OutputError("Failed to complete firewall detection!", aError);
}

#ifdef DLLSAMPLE
BOOL Matchmaker::PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData)
#else
bool Matchmaker::PatchProgressCallback(unsigned long progress, unsigned long size, void* callbackPrivData)
#endif
{
	if (size > 0)
		OutputStringF("%d%% (%d/%d)", progress * 100 / size, progress, size);
#ifdef DLLSAMPLE
	return TRUE;
#else
	return true;
#endif // DLLSAMPLE
}

#ifdef DLLSAMPLE
void Matchmaker::PatchDownloadCallback(HWONCOMPLETION hCompletion, void* param)
{
	WONError theError = reinterpret_cast<WONError>(param);
#else
void Matchmaker::PatchDownloadCallback(Error theError)
{
#endif // DLLSAMPLE
	switch(theError)
	{
		case StatusCommon_Success:
			OutputString("Patch download complete.");
			break;
		default:
			OutputError("Patch download failed!", theError);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::TitanServerEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);

	if (WONDirEntityIsDirectory(theDirEntityH))
	{
		unsigned short aNumObjects = WONDirEntityGetNumDataObjects(theDirEntityH);
		for (int iObject = 0; iObject < aNumObjects; ++iObject)
		{
			HWONDATAOBJECT aObjectH = WONDirEntityGetDataObject(theDirEntityH, iObject, FALSE);
			unsigned char aTypeLen;
			WONDataObjectGetType(aObjectH, NULL, &aTypeLen);
			char* aTypeBufP = new char[aTypeLen];
			WONDataObjectGetType(aObjectH, aTypeBufP, &aTypeLen);

			unsigned short aDataLen;
			WONDataObjectGetData(aObjectH, NULL, &aDataLen);
			char* aDataBufP = new char[aDataLen];
			WONDataObjectGetData(aObjectH, aDataBufP, &aDataLen);
			
			if (memcmp(aTypeBufP, OBJ_VALIDVERSIONS.c_str(), min((unsigned long)aTypeLen, OBJ_VALIDVERSIONS.size())) == 0)
				that->mValidVersions.assign(aDataBufP, aDataLen);

			delete aTypeBufP;
			delete aDataBufP;
		}
	}
	else
	{
		// get the server's address
		WONIPAddress aAddress;
		WONDirEntityGetAddress(theDirEntityH, &aAddress);

		// get the server's name (i.e. type)
		unsigned short aBufLen = 0;
		WONDirEntityGetNameW(theDirEntityH, NULL, &aBufLen);
		wchar* aBufP = new wchar[aBufLen + 1];
		WONDirEntityGetNameW(theDirEntityH, aBufP, &aBufLen);
	
		// store the info
		if (wcscmp(aBufP, SERVERNAME_AUTH) == 0)
			that->mNewAuthServers.push_back(aAddress);
		else if (wcscmp(aBufP, SERVERNAME_CONTEST) == 0)
			that->mNewContestServers.push_back(aAddress);
		else if (wcscmp(aBufP, SERVERNAME_EVENT) == 0)
			that->mNewEventServers.push_back(aAddress);
		else if (wcscmp(aBufP, SERVERNAME_FIREWALL) == 0)
			that->mNewFirewallServers.push_back(aAddress);
		else if (wcscmp(aBufP, SERVERNAME_PROFILE) == 0)
			that->mNewProfileServers.push_back(aAddress);

		delete aBufP;
	}
}
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
void Matchmaker::ChatRoomEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);

	// get the server's name (i.e. type)
	unsigned short aNameBufLen = 0;
	WONDirEntityGetNameW(theDirEntityH, NULL, &aNameBufLen);
	wchar* aNameBufP = new wchar[aNameBufLen + 1];
	WONDirEntityGetNameW(theDirEntityH, aNameBufP, &aNameBufLen);

	if (wcscmp(aNameBufP, SERVERNAME_CHAT) == 0)
	{
		ChatServerInfo aChatServerInfo;
		WONDirEntityGetAddress(theDirEntityH, &aChatServerInfo.mAddress);

		unsigned short aNumObjects = WONDirEntityGetNumDataObjects(theDirEntityH);
		for (int iObject = 0; iObject < aNumObjects; ++iObject)
		{
			HWONDATAOBJECT aObjectH = WONDirEntityGetDataObject(theDirEntityH, iObject, FALSE);
			
			unsigned char aTypeLen;
			WONDataObjectGetType(aObjectH, NULL, &aTypeLen);
			char* aTypeBufP = new char[aTypeLen];
			WONDataObjectGetType(aObjectH, aTypeBufP, &aTypeLen);
			
			unsigned short aDataLen;
			WONDataObjectGetData(aObjectH, NULL, &aDataLen);
			char* aDataBufP = new char[aDataLen];
			WONDataObjectGetData(aObjectH, aDataBufP, &aDataLen);

			if (memcmp(aTypeBufP, OBJ_CHATROOM_CLIENTCOUNT.c_str(), min((unsigned long)aTypeLen, OBJ_CHATROOM_CLIENTCOUNT.size())) == 0)
				aChatServerInfo.mClientCount = *(unsigned long*)aDataBufP;
			else if (memcmp(aTypeBufP, OBJ_CHATROOM_LOGINACL.c_str(), min((unsigned long)aTypeLen, OBJ_CHATROOM_LOGINACL.size())) == 0)
				;
			else if (memcmp(aTypeBufP, OBJ_CHATROOM_ROOMFLAGS.c_str(), min((unsigned long)aTypeLen, OBJ_CHATROOM_ROOMFLAGS.size())) == 0)
				aChatServerInfo.mIsPasswordProtected = ((*aDataBufP) & 0x1 != 0);

			delete aTypeBufP;
			delete aDataBufP;
		}
		
		// get the server's display name
		unsigned short aDisplayNameBufLen = 0;
		WONDirEntityGetDisplayNameW(theDirEntityH, NULL, &aDisplayNameBufLen);
		wchar_t* aDisplayNameBufP = new wchar[aDisplayNameBufLen + 1];
		WONDirEntityGetDisplayNameW(theDirEntityH, aDisplayNameBufP, &aDisplayNameBufLen);

		that->mChatServers[aDisplayNameBufP] = aChatServerInfo;
		++(that->mNumChatServers);

		delete aDisplayNameBufP;
	}
	else if (wcscmp(aNameBufP, SERVERNAME_FACTORY) == 0)
	{
		// get the server's address
		WONIPAddress aAddress;
		WONDirEntityGetAddress(theDirEntityH, &aAddress);
		that->mNewFactoryServers.push_back(aAddress);
	}

	delete aNameBufP;
}
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
void Matchmaker::ChatRoomListCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	WONError aError = reinterpret_cast<WONError>(param);
#else
void Matchmaker::ChatRoomListCallback(const DirEntityListResult& theResultR, Matchmaker* that)
{
	Error aError = theResultR.error;
#endif // DLLSAMPLE
	switch(aError)
	{
		case StatusCommon_Success:
		{
#ifdef DLLSAMPLE
			CopySTLAddressListToArray(that->mNewFactoryServers, &that->mFactoryServers, &that->mNumFactoryServers);
#else
			that->mChatServers.clear(); that->mNumChatServers = 0;
			delete[] that->mFactoryServers; that->mNumFactoryServers = 0;
			that->mFactoryServers = new IPSocket::Address[theResultR.entityList->size()];

			DirEntityList::const_iterator aDirEntityListItr = theResultR.entityList->begin();
			for( ; aDirEntityListItr != theResultR.entityList->end(); ++aDirEntityListItr)
			{
				if (aDirEntityListItr->mName == SERVERNAME_CHAT)
				{
					ChatServerInfo aChatServerInfo;
					aChatServerInfo.mAddress = IPSocket::Address(*aDirEntityListItr);

					DataObjectTypeSet::const_iterator aDataObjectSetItr = aDirEntityListItr->mDataObjects.begin();
					for( ; aDataObjectSetItr != aDirEntityListItr->mDataObjects.end(); ++aDataObjectSetItr)
					{
						if (aDataObjectSetItr->GetDataType() == OBJ_CHATROOM_CLIENTCOUNT)
							aChatServerInfo.mClientCount = *(unsigned long*)aDataObjectSetItr->GetData().c_str();
						else if (aDataObjectSetItr->GetDataType() == OBJ_CHATROOM_LOGINACL)
							;
						else if (aDataObjectSetItr->GetDataType() == OBJ_CHATROOM_ROOMFLAGS)
							aChatServerInfo.mIsPasswordProtected = ((*aDataObjectSetItr->GetData().c_str()) & 0x1 != 0);
					}
					
					WONString aDisplayName(aDirEntityListItr->mDisplayName);
					that->mChatServers[aDisplayName] = aChatServerInfo;
//					that->mChatServers[(const wchar_t*)aDirEntityListItr->mDisplayName().c_str()] = aChatServerInfo;
					++(that->mNumChatServers);
				}
				else if (aDirEntityListItr->mName == SERVERNAME_FACTORY)
					that->mFactoryServers[that->mNumFactoryServers++] = IPSocket::Address(*aDirEntityListItr);
			}
#endif // !DLLSAMPLE

			if (that->mNumChatServers == 0)
				OutputString("No rooms available.");
			else
			{
				OutputString("Room Name       Users Password?");
				ChatServerMap::const_iterator itr;
				for (itr = that->mChatServers.begin(); itr != that->mChatServers.end(); ++itr)
				{
					WONString aDisplayName(itr->first);
					OutputStringF("%-15S %3d       %c", aDisplayName.GetUnicodeCString(), itr->second.mClientCount, itr->second.mIsPasswordProtected ? 'y' : 'n');
				}
			}

			break;
		}

		case StatusDir_DirNotFound:
			OutputError("Unable to access server at this time");
			break;
		default:
			OutputError("Room list request failed!", aError);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::ChatRoomLoginCallback(WONRoutingRegisterClientResult* theResultP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	short aStatus = theResultP->mStatus;
#else
void Matchmaker::ChatRoomLoginCallback(const RoutingServerClient::RegisterClientResult& theResultR, Matchmaker* that)
{
	short aStatus = theResultR.mStatus;
#endif // DLLSAMPLE
	switch (aStatus)
	{
		case StatusRouting_ClientAlreadyRegistered:
			OutputError("Software error: already logged in");
			
			// fall through
		case StatusCommon_Success:
			OutputError("Logged in to chat room successfully");
#ifdef DLLSAMPLE
			WONRoutingGetClientList(that->mRoutingServer,
			                        FALSE, // no auth info
			                        FALSE, // no mute flag
			                        FALSE, // no moderator flag
			                        ClientListCallback, that);
#else
			that->mRoutingServer.GetClientListEx(false, // no auth info
			                                     false, // no mute flag
			                                     false, // no moderator flag
			                                     (void(*)(const RoutingServerClient::GetClientListResult&, Matchmaker*))ClientListCallback, that);
#endif // DLLSAMPLE
			break;
		case StatusRouting_ClientAlreadyExists:
			OutputError("Someone is already using the account");
			that->Close();
			break;
		case StatusRouting_InvalidPassword:
			OutputError("Incorrect password");
			that->Close();
			break;
		case StatusRouting_ServerFull:
			OutputError("This chat room is full.  Please try another one.");
			that->Close();
			break;
		default:
			OutputServerError("Failed to login the chat room", that->mCurChatServerAddr, aStatus);
			that->Close();
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::ClientListCallback(WONRoutingClientListResult* theListP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	short aStatus = theListP->mStatus;
#else
void Matchmaker::ClientListCallback(const RoutingServerClient::GetClientListResult& theResultR, Matchmaker* that)
{
	short aStatus = theResultR.mStatus;
#endif // DLLSAMPLE
	switch (aStatus)
	{
		case StatusCommon_Success:
		{
			OutputError("Got client list successfully");
#ifdef DLLSAMPLE
			for (int iClient = 0; iClient < theListP->mNumClients; ++iClient)
			{
				WONRoutingClientData aClientData;
				CopyWONRoutingClientData(&theListP->mClients[iClient], &aClientData);
				that->mClientMap[theListP->mClients[iClient].mClientId] = aClientData;
			}
#else
			for (std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::const_iterator itr = theResultR.mClientList.begin(); itr != theResultR.mClientList.end(); itr++)
				that->mClientMap[itr->mClientId] = *itr;
#endif // DLLSAMPLE

#ifdef DLLSAMPLE
			WONRoutingSubscribeDataObject(that->mRoutingServer, GROUPID_ALLUSERS, OBJ_GAMEPREFIX.data(), OBJ_GAMEPREFIX.size(), FALSE, TRUE, SubscribeDataObjectCallback, that);
#else
			that->mRoutingServer.SubscribeDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, OBJ_GAMEPREFIX, false, true, (void(*)(const RoutingServerClient::ReadDataObjectResult& result, Matchmaker*))SubscribeDataObjectCallback, that);
#endif // DLLSAMPLE
			break;
		}
		case StatusRouting_MustBeClient:
			OutputError("Failed to retrieve client list: not logged in");
			break;
		default:
			OutputError("Failed to retrieve client list", aStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::SubscribeDataObjectCallback(WONRoutingReadDataObjectResult* theObjectsP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	short aStatus = theObjectsP->mStatus;
#else
	void Matchmaker::SubscribeDataObjectCallback(const RoutingServerClient::ReadDataObjectResult& theResultR, Matchmaker* that)
{
	short aStatus = theResultR.mStatus;
#endif // DLLSAMPLE
	switch (aStatus)
	{
		case StatusCommon_Success:
		{
			OutputError("Subscribed to game list successfully");
#ifdef DLLSAMPLE
			for (int iObject = 0; iObject < theObjectsP->mNumObjects; ++iObject)
			{
				string gamename = (char*)theObjectsP->mObjects[iObject].mDataType + OBJ_GAMEPREFIX.size();
				that->mGameMap[gamename] = theObjectsP->mObjects[iObject];
			}
#else
			for (std::list<RoutingServerClient::DataObject>::const_iterator itr = theResultR.mDataObjectList.begin(); itr != theResultR.mDataObjectList.end(); itr++)
			{
				string gamename = (char*)itr->mDataType.data() + OBJ_GAMEPREFIX.size();
				that->mGameMap[gamename] = *itr; 
			}
#endif // DLLSAMPLE
			break;
		}
		case StatusRouting_SubscriptionAlreadyExists:
			OutputError("Can not get game list again");
			break;
		default:
			OutputError("Fail to retrieve game list", aStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::CreateDataObjectCallback(short theStatus, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
#else
void Matchmaker::CreateDataObjectCallback(short theStatus, Matchmaker* that)
{
#endif // DLLSAMPLE
	switch(theStatus)
	{
		case StatusCommon_Success:
			OutputError("Created game successfully");
#ifdef DLLSAMPLE
			that->mGameInfo.mCaptainId = WONRoutingGetClientId(that->mRoutingServer);
#else
			that->mGameInfo.mCaptainId = that->mRoutingServer.GetClientId();
#endif // DLLSAMPLE
			break;
		case StatusRouting_ObjectAlreadyExists:
			OutputError("Game with that name already exists.  Choose another name.");
			break;
		case StatusRouting_ObjectTooLarge:
			OutputError("Too much game data");
			break;
		default:
			OutputError("Fail to create game", theStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::ReplaceDataObjectCallback(short theStatus, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
#else
void Matchmaker::ReplaceDataObjectCallback(short theStatus, Matchmaker* that)
{
#endif // DLLSAMPLE
	switch(theStatus)
	{
		case StatusCommon_Success:
		{
			OutputError("Successfully updated game object");
			if (that->mGameInfo.mJoiningClientId)
			{
				JoinReplyMsg aJoinReply(true);
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aJoinReply, sizeof(aJoinReply));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &that->mGameInfo.mJoiningClientId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&that->mGameInfo.mJoiningClientId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			break;
		}
		default:
			OutputError("Failed to update game object!", theStatus);
			break;
	}
}

#ifdef DLLSAMPLE
void Matchmaker::ReportResultsCallback(HWONCOMPLETION hCompletion, void* param, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	WONError aError = reinterpret_cast<WONError>(param);
#else
void Matchmaker::ReportResultsCallback(const ContestDBResult& theResultR, Matchmaker* that)
{
	Error aError = theResultR.error;
#endif // DLLSAMPLE
	switch(aError)
	{
		case StatusCommon_Success:
			OutputError("Successfully recorded scores");
			break;
		default:
			OutputError("Failed to record scores via Contest Server!", aError);
			break;
	}
}

//
// CATCHERS
//

#ifdef DLLSAMPLE
void Matchmaker::OnClientEnterChat(WONRoutingGroupIdAndClientDataWithReason *theDataP, void* theMatchmakerP)
{
	if (theDataP->mGroupId == GROUPID_CHAT)
	{
		Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
		OutputStringF("%S entered room", reinterpret_cast<const unsigned short*>(theDataP->mData.mClientNameW));
		WONRoutingClientData aClientData;
		CopyWONRoutingClientData(&theDataP->mData, &aClientData);
		that->mClientMap[theDataP->mData.mClientId] = aClientData;
	}
}
#else
void Matchmaker::OnClientEnterChat(const RoutingServerClient::GroupIdAndClientDataWithReason& theDataR, Matchmaker* that)
{
	if (theDataR.mGroupId == GROUPID_CHAT)
	{
		OutputStringF("%S entered room", reinterpret_cast<const unsigned short*>(theDataR.mClientName.c_str()));
		that->mClientMap[theDataR.mClientId] = theDataR;
	}
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnClientLeaveChat(WONRoutingGroupIdAndClientIdWithReason* theDataP, void* theMatchmakerP)
{
	if (theDataP->mGroupId == GROUPID_CHAT)
	{
		Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
		OutputStringF("%S left room", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theDataP->mClientId)->second.mClientNameW));
		ClientMap::iterator itr = that->mClientMap.find(theDataP->mClientId);
		if (itr != that->mClientMap.end()) // should never not find the client, but...
		{
			DeleteWONRoutingClientData(&(itr->second));
			that->mClientMap.erase(itr);
		}
	}
}
#else
void Matchmaker::OnClientLeaveChat(const RoutingServerClient::GroupIdAndClientIdWithReason& theDataR, Matchmaker* that)
{
	if (theDataR.mGroupId == GROUPID_CHAT)
	{
		OutputStringF("%S left room", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theDataR.mClientId)->second.mClientName.c_str()));
		if (theDataR.mGroupId == RoutingServerClient::GROUPID_CHAT)
			that->mClientMap.erase(theDataR.mClientId);
	}
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnDataObjectCreated(WONRoutingDataObjectWithLifespan* theDataP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	char* gamename = (char*)theDataP->mObject.mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("New game created: %s", gamename);
	that->mGameMap[gamename] = theDataP->mObject;
}
#else
void Matchmaker::OnDataObjectCreated(const RoutingServerClient::DataObjectWithLifespan& theObjectR, Matchmaker* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("New game created: %s", gamename);
	that->mGameMap[gamename] = theObjectR;
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnDataObjectDeleted(WONRoutingDataObject* theDataP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	char* gamename = (char*)theDataP->mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("Game deleted: %s", gamename);
	that->mGameMap.erase(gamename);
}
#else
void Matchmaker::OnDataObjectDeleted(const RoutingServerClient::DataObject& theObjectR, Matchmaker* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("Game deleted: %s", gamename);
	that->mGameMap.erase(gamename);
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnDataObjectReplaced(WONRoutingDataObject* theDataP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	char* gamename = (char*)theDataP->mDataType + OBJ_GAMEPREFIX.size();
	OutputStringF("Game data updated: %s", gamename);
	that->mGameMap[gamename].mData = theDataP->mData; 
}
#else
void Matchmaker::OnDataObjectReplaced(const RoutingServerClient::DataObject& theObjectR, Matchmaker* that)
{
	char* gamename = (char*)theObjectR.mDataType.c_str() + OBJ_GAMEPREFIX.size();
	OutputStringF("Game data updated: %s", gamename);
	that->mGameMap[gamename].mData = theObjectR.mData; 
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnPeerDataReceived(WONRoutingMessageWithClientId* theMsgP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);

	WONRoutingClientId theSenderId = theMsgP->mClientId;
	const char* theBufP = theMsgP->mData;
	unsigned short theBufLen = theMsgP->mDataLen;
#else
void Matchmaker::OnPeerDataReceived(const RoutingServerClient::MessageWithClientId& theMessageR, Matchmaker* that)
{
	RoutingServerClient::ClientId theSenderId = theMessageR.mClientId;
	const unsigned char* theBufP = theMessageR.mBufP;
	unsigned short theBufLen = theMessageR.mBufLen;
#endif // DLLSAMPLE
	if (theBufLen == 0)
		return;

	MessageType theGameMessageType = theBufP[0];
	switch (theGameMessageType)
	{
		case MSGTYPE_JOIN_REQUEST:
		{
			if (theBufLen < sizeof(JoinRequestMsg))
				return;
#ifdef DLLSAMPLE
			OutputStringF("Received join request from %S", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientNameW));
#else
			OutputStringF("Received join request from %S", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientName.c_str()));
#endif // DLLSAMPLE
			if (that->mGameInfo.mNumPlayersInGame >= TicTacToe::MAX_PLAYERS_PER_GAME)
			{
				JoinReplyMsg aJoinReply(false);
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aJoinReply, sizeof(aJoinReply));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &theSenderId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&theSenderId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			else
			{
				that->mGameInfo.mPlayersInGame[that->mGameInfo.mNumPlayersInGame++] = theSenderId;
				that->mGameInfo.mJoiningClientId = theSenderId;
				RawBuffer aDataType(OBJ_GAMEPREFIX + (unsigned char*)that->mGameInfo.mMyGameName.c_str());
				RawBuffer aData;
                aData.assign((unsigned char*)that->mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
				             that->mGameInfo.mNumPlayersInGame*sizeof(WONRoutingClientId));
#else
				             that->mGameInfo.mNumPlayersInGame*sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
				WONRoutingReplaceDataObjectAndGetResult(that->mRoutingServer, GROUPID_ALLUSERS, aDataType.data(), aDataType.size(), aData.data(), aData.size(), ReplaceDataObjectCallback, that);
#else
				that->mRoutingServer.ReplaceDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aDataType, aData, (void(*)(short, Matchmaker*))ReplaceDataObjectCallback, that);
#endif // DLLSAMPLE

				// set new player's id in score info
				ClientMap::const_iterator itr = that->mClientMap.find(theSenderId);
				that->mScoreSoFar.mPlayer2.mUserSeq = itr->second.mWONUserId;
				that->mScoreSoFar.Clear();
			}
			break;
		}
		case MSGTYPE_JOIN_REPLY:
		{
			if (theBufLen < sizeof(JoinReplyMsg))
				return;
			const JoinReplyMsg* aJoinReplyP = reinterpret_cast<const JoinReplyMsg*>(theBufP);
			if (aJoinReplyP->mAcceptRejectFlag)
			{
				OutputString("Captain accepted you into game");
				that->mGameInfo.mCaptainId = theSenderId;
			}
			else
				OutputString("Captain rejected your join request for game");
			break;
		}
		case MSGTYPE_LEAVE_GAME:
		{
			if (theBufLen < sizeof(LeaveGameMsg))
				return;
			for (unsigned int i = 0; i < that->mGameInfo.mNumPlayersInGame; ++i)
			{
				if (that->mGameInfo.mPlayersInGame[i] == theSenderId)
				{
					memmove(&that->mGameInfo.mPlayersInGame[i], &that->mGameInfo.mPlayersInGame[i+1], 
#ifdef DLLSAMPLE
					        sizeof(WONRoutingClientId) * (that->mGameInfo.mNumPlayersInGame - i - 1));
#else
					        sizeof(RoutingServerClient::ClientId) * (that->mGameInfo.mNumPlayersInGame - i - 1));
#endif // DLLSAMPLE
					--that->mGameInfo.mNumPlayersInGame;
					that->mGameInfo.mJoiningClientId = 0;
					RawBuffer aDataType(OBJ_GAMEPREFIX + (unsigned char*)that->mGameInfo.mMyGameName.c_str());
					RawBuffer aData;
                    aData.assign((unsigned char*)that->mGameInfo.mPlayersInGame, 
#ifdef DLLSAMPLE
					             that->mGameInfo.mNumPlayersInGame*sizeof(WONRoutingClientId));
#else
					             that->mGameInfo.mNumPlayersInGame*sizeof(RoutingServerClient::ClientId));
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
					WONRoutingReplaceDataObjectAndGetResult(that->mRoutingServer, GROUPID_ALLUSERS, aDataType.data(), aDataType.size(), aData.data(), aData.size(), ReplaceDataObjectCallback, that);
#else
                    that->mRoutingServer.ReplaceDataObjectEx(RoutingServerClient::GROUPID_ALLUSERS, aDataType, aData, (void(*)(short, Matchmaker*))ReplaceDataObjectCallback, that);
#endif // DLLSAMPLE

					// save results via Contest Server
					that->ReportResults();

					break;
				}
			}
			if (that->mGameInfo.mNumPlayersInGame == 1)
				that->mGameInfo.mGameHasStarted = false;

			break;
		}
		case MSGTYPE_START_GAME:
		{
			if (theBufLen < sizeof(StartGameMsg))
				return;

			OutputString("Game starting...");
			that->mGameInfo.mGameHasStarted = true;
			that->mGameInfo.ResetBoard();
			that->mGameInfo.DisplayBoard();
			break;
		}
		case MSGTYPE_YOUR_TURN:
		{
			if (theBufLen < sizeof(YourTurnMsg))
				return;
			OutputString("It's your turn");
			break;
		}
		case MSGTYPE_MOVE_REQUEST:
		{
			if (theBufLen < sizeof(MoveRequestMsg))
				return;
			const MoveRequestMsg* aMoveRequestP = reinterpret_cast<const MoveRequestMsg*>(theBufP);
			
			if (that->mGameInfo.mPlayersInGame[that->mGameInfo.mWhoseTurnIsIt] == theSenderId && that->mGameInfo.ValidateMove(aMoveRequestP->mX, aMoveRequestP->mY))
				that->AcceptMove(PLAYER_X, theSenderId, aMoveRequestP->mX, aMoveRequestP->mY);
			else
			{
				MoveRejectedMsg aMoveRejected;
				RawBuffer aMsg;
                aMsg.assign((unsigned char*)&aMoveRejected, sizeof(aMoveRejected));
#ifdef DLLSAMPLE
				WONRoutingSendData(that->mRoutingServer, &theSenderId, 1, TRUE, aMsg.data(), aMsg.size());
#else
                that->mRoutingServer.SendData(&theSenderId, 1, true, aMsg, false);
#endif // DLLSAMPLE
			}
			break;
		}
		case MSGTYPE_MOVE_ACCEPTED:
		{
			if (theBufLen < sizeof(MoveAcceptedMsg))
				return;
			const MoveAcceptedMsg* aMoveAcceptedP = reinterpret_cast<const MoveAcceptedMsg*>(theBufP);
#ifdef DLLSAMPLE
			if (aMoveAcceptedP->mClientId != WONRoutingGetClientId(that->mRoutingServer))
				OutputStringF("%S took square %c%d", reinterpret_cast<const unsigned short*>(that->mClientMap.find(aMoveAcceptedP->mClientId)->second.mClientNameW), 'A' + aMoveAcceptedP->mX, aMoveAcceptedP->mY + 1);
#else
			if (aMoveAcceptedP->mClientId != that->mRoutingServer.GetClientId())
				OutputStringF("%S took square %c%d", reinterpret_cast<const unsigned short*>(that->mClientMap.find(aMoveAcceptedP->mClientId)->second.mClientName.c_str()), 'A' + aMoveAcceptedP->mX, aMoveAcceptedP->mY + 1);
#endif // DLLSAMPLE
			that->mGameInfo.mGameBoard[aMoveAcceptedP->mX][aMoveAcceptedP->mY] = aMoveAcceptedP->mPlayer;
			that->mGameInfo.DisplayBoard();
			if (that->mGameInfo.IAmCaptain())
			{
				if (that->mGameInfo.IsWinningMove(aMoveAcceptedP->mPlayer, aMoveAcceptedP->mX, aMoveAcceptedP->mY))
				{
					GameOverMsg aGameOver(aMoveAcceptedP->mClientId);
					RawBuffer aMsg;
                    aMsg.assign((unsigned char*)&aGameOver, sizeof(aGameOver));
#ifdef DLLSAMPLE
					WONRoutingSendData(that->mRoutingServer, that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
                    that->mRoutingServer.SendData(that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE

					// update score info
#ifdef DLLSAMPLE
					if (aMoveAcceptedP->mClientId == WONRoutingGetClientId(that->mRoutingServer))
#else
					if (aMoveAcceptedP->mClientId == that->mRoutingServer.GetClientId())
#endif // DLLSAMPLE
					{
						++that->mScoreSoFar.mPlayer1.mWins;
						++that->mScoreSoFar.mPlayer2.mLosses;
					}
					else
					{
						++that->mScoreSoFar.mPlayer1.mLosses;
						++that->mScoreSoFar.mPlayer2.mWins;
					}
				}
				else if (that->mGameInfo.IsBoardFull())
				{
					GameOverMsg aGameOver;
					RawBuffer aMsg;
                    aMsg.assign((unsigned char*)&aGameOver, sizeof(aGameOver));
#ifdef DLLSAMPLE
					WONRoutingSendData(that->mRoutingServer, that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, TRUE, aMsg.data(), aMsg.size());
#else
                    that->mRoutingServer.SendData(that->mGameInfo.mPlayersInGame, that->mGameInfo.mNumPlayersInGame, true, aMsg, false);
#endif // DLLSAMPLE

					// update score info
					++that->mScoreSoFar.mPlayer1.mTies;
					++that->mScoreSoFar.mPlayer2.mTies;
				}
			}
			break;
		}
		case MSGTYPE_MOVE_REJECTED:
		{
			if (theBufLen < sizeof(MoveRejectedMsg))
				return;
			const MoveRejectedMsg* aMoveRejectedP = reinterpret_cast<const MoveRejectedMsg*>(theBufP);
			OutputString("Invalid move (or maybe it's not your turn?)");
			break;
		}
		case MSGTYPE_GAME_OVER:
		{
			if (theBufLen < sizeof(GameOverMsg))
				return;
			const GameOverMsg* aGameOverP = reinterpret_cast<const GameOverMsg*>(theBufP);
			that->mGameInfo.mGameHasStarted = false;
			if (aGameOverP->mClientId)
			{
#ifdef DLLSAMPLE
				if (aGameOverP->mClientId == WONRoutingGetClientId(that->mRoutingServer))
#else
				if (aGameOverP->mClientId == that->mRoutingServer.GetClientId())
#endif // DLLSAMPLE
					OutputString("GAME OVER: You win!!! :)");
				else
#ifdef DLLSAMPLE
					OutputStringF("GAME OVER: %S wins. :(", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientNameW));
#else
					OutputStringF("GAME OVER: %S wins. :(", reinterpret_cast<const unsigned short*>(that->mClientMap.find(theSenderId)->second.mClientName.c_str()));
#endif // DLLSAMPLE
			}
			else
				OutputString("GAME OVER: It's a draw.");
			break;
		}
		case MSGTYPE_GAME_DISSOLVED:
		{
			if (theBufLen < sizeof(GameDissolvedMsg))
				return;
			that->mGameInfo.Reset();
			OutputString("Captain has dissolved the game");
		}
	}
}
#ifdef DLLSAMPLE
void Matchmaker::OnChatReceived(WONRoutingASCIIChatMessage* theMsgP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	ClientMap::const_iterator itr = that->mClientMap.find(theMsgP->mBaseData.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %s", itr->second.mClientNameW, theMsgP->mChat);
}
#else
void Matchmaker::OnChatReceived(const RoutingServerClient::ASCIIChatMessage& theMessageR, Matchmaker* that)
{
	ClientMap::const_iterator itr = that->mClientMap.find(theMessageR.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %s", reinterpret_cast<const unsigned short*>(itr->second.mClientName.c_str()), theMessageR.mData.c_str());
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnChatReceived(WONRoutingUnicodeChatMessage* theMsgP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	ClientMap::const_iterator itr = that->mClientMap.find(theMsgP->mBaseData.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %S", itr->second.mClientNameW, theMsgP->mChat);
}
#else
void Matchmaker::OnChatReceived(const RoutingServerClient::UnicodeChatMessage& theMessageR, Matchmaker* that)
{
	ClientMap::const_iterator itr = that->mClientMap.find(theMessageR.mSenderId);
	if (itr != that->mClientMap.end())
	{
		WONString aChatText(theMessageR.mData);
		OutputStringF("%S: %S", reinterpret_cast<const unsigned short*>(itr->second.mClientName.c_str()), aChatText.GetUnicodeCString());
	}
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnReconnectFailure(void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
#else
void Matchmaker::OnReconnectFailure(Matchmaker* that)
{
#endif // DLLSAMPLE
	OutputString("Connection to chat failed unexpectedly");
	that->Close();
}


void Matchmaker::Close()
{
#ifdef DLLSAMPLE
	WONRoutingCloseConnection(mRoutingServer);
#else
	mRoutingServer.Close();
#endif // DLLSAMPLE
	mCurChatServerName.erase();
}

bool Matchmaker::IsLoggedIn() const
{
#ifdef DLLSAMPLE
	return (mAuthH != NULL && WONAuthGetNameA(mAuthH) != NULL && WONAuthGetNameA(mAuthH) != '\0');
#else
	return (mIdentity.GetLoginName().size() != 0 );
#endif // DLLSAMPLE
}

bool Matchmaker::IsInAGame() const
{ return mGameInfo.mCaptainId != 0; }
