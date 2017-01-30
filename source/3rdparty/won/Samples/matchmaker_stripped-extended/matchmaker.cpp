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

// COMMAND LIST
const char* COMMAND_LOGIN          = "LOGIN";
const char* COMMAND_CREATE_ACCOUNT = "CREATEACCOUNT";
const char* COMMAND_LIST_ROOMS     = "ROOMS";
const char* COMMAND_CREATE_ROOM    = "CREATEROOM";
const char* COMMAND_JOIN_ROOM      = "JOINROOM";
const char* COMMAND_GET_ROOM_NAME  = "ROOMNAME";
const char* COMMAND_LIST_USERS     = "USERS";
const char* COMMAND_IGNORE_USER    = "IGNORE";
const char* COMMAND_UNIGNORE_USER  = "UNIGNORE";
extern const char* COMMAND_HELP;// = "HELP";			// also handled by main.cpp

// UTILITY (main.cpp)
extern int GetCommandToken(string& theDestString, char* theSourceBuffer, 
					int theTokenNum, BOOL copyToEnd);

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
	mProfileServers(NULL)
#ifdef DLLSAMPLE
						,
	mAuthH(NULL)
//#else
//	mGameInfo(mRoutingServer),
#endif // !DLLSAMPLE
{
#ifdef DLLSAMPLE
	mRoutingServer = WONRoutingCreateSession();
	//mGameInfo.Init(mRoutingServer);
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
	WONRoutingInstallASCIIPeerChatCallback(mRoutingServer, OnChatReceived, this);
	WONRoutingInstallUnicodePeerChatCallback(mRoutingServer, OnChatReceived, this);
	WONRoutingInstallReconnectFailureCallback(mRoutingServer, OnReconnectFailure, this);
#else
//	mRoutingServer.InstallClientEnterExCatcherEx((void(*)(const RoutingServerClient::ClientDataWithReason&, Matchmaker*))OnClientEnterChat, this);
	mRoutingServer.InstallGroupEnterExCatcherEx((void(*)(const RoutingServerClient::GroupIdAndClientDataWithReason&, Matchmaker*))OnClientEnterChat, this);
	mRoutingServer.InstallGroupLeaveCatcherEx((void(*)(const RoutingServerClient::GroupIdAndClientIdWithReason&, Matchmaker*))OnClientLeaveChat, this);
	mRoutingServer.InstallASCIIPeerChatCatcherEx((void(*)(const RoutingServerClient::ASCIIChatMessage&, Matchmaker*))OnAsciiChatReceived, this);
	mRoutingServer.InstallUnicodePeerChatCatcherEx((void(*)(const RoutingServerClient::UnicodeChatMessage&, Matchmaker*))OnUnicodeChatReceived, this);
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
#else
	mDirServers = new IPSocket::Address[3];
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.east.won.net", 15101);
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.central.won.net", 15101);
	mDirServers[mNumDirServers++] = IPSocket::Address("wontest.west.won.net", 15101);
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
//	WONAuthSetCDKeyString("Vampire", "ZUS5-JAB4-PEW2-BAB3-9222", FALSE);
//	WONAuthSetLoginKeyFileA("login.ks");
	if (mAuthH) { WONAuthCloseHandle(mAuthH); mAuthH = NULL; }
	mAuthH = WONAuthLoginA(&aError, mAuthServers, mNumAuthServers, theUsernameP, COMMUNITY_SAMPLE, thePasswordP, "", gRequestTimeout);
#else
//	WONCDKey::ClientCDKey aCDKey("Vampire");
//	aCDKey.Init("ZUS5-JAB4-PEW2-BAB3-9222");
//	Identity::SetCDKey(aCDKey);
//	Identity::SetLoginKeyFile("login.ks");
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
			//WONString aWideUserName(reinterpret_cast<const wchar*>(itr->second.mClientName.data()), itr->second.mClientName.size() / 2);
			OutputString( (wchar_t*) itr->second.mClientName.data());
#endif // DLLSAMPLE
		}
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
			for (std::list<WONMsg::MMsgRoutingGetClientListReply::ClientData>::iterator itr = theResultR.mClientList.begin(); itr != theResultR.mClientList.end(); itr++)
			{
				WONMsg::MMsgRoutingGetClientListReply::ClientData aCopiedClient(*itr);
				WONString aWideUserName(reinterpret_cast<const wchar*>(aCopiedClient.mClientName.data()), aCopiedClient.mClientName.size() / 2);

				aCopiedClient.mClientName.assign(reinterpret_cast<const unsigned char*>(aWideUserName.GetUnicodeCString()), aWideUserName.GetUnicodeString().size()*sizeof(wchar_t));
				aCopiedClient.mClientName.append(sizeof(wchar_t), '\0');
				that->mClientMap[aCopiedClient.mClientId] = *itr;
			}
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
void Matchmaker::OnAsciiChatReceived(WONRoutingASCIIChatMessage* theMsgP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	ClientMap::const_iterator itr = that->mClientMap.find(theMsgP->mBaseData.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %s", itr->second.mClientNameW, theMsgP->mChat);
}
#else
void Matchmaker::OnAsciiChatReceived(const RoutingServerClient::ASCIIChatMessage& theMessageR, Matchmaker* that)
{
	ClientMap::const_iterator itr = that->mClientMap.find(theMessageR.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %s", reinterpret_cast<const unsigned short*>(itr->second.mClientName.c_str()), theMessageR.mData.c_str());
}
#endif // DLLSAMPLE
#ifdef DLLSAMPLE
void Matchmaker::OnUnicodeChatReceived(WONRoutingUnicodeChatMessage* theMsgP, void* theMatchmakerP)
{
	Matchmaker* that = reinterpret_cast<Matchmaker*>(theMatchmakerP);
	ClientMap::const_iterator itr = that->mClientMap.find(theMsgP->mBaseData.mSenderId);
	if (itr != that->mClientMap.end())
		OutputStringF("%S: %S", itr->second.mClientNameW, theMsgP->mChat);
}
#else
void Matchmaker::OnUnicodeChatReceived(const RoutingServerClient::UnicodeChatMessage& theMessageR, Matchmaker* that)
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

bool Matchmaker::HandleCommand(string& theCommand, char* theInputBuf)
{

	if (theCommand==COMMAND_CREATE_ACCOUNT)
	{
		string aUsername, aPassword;
		if (GetCommandToken(aUsername,theInputBuf,1,FALSE) == -1 ||
			GetCommandToken(aPassword,theInputBuf,2,FALSE) == -1)
		{
			OutputString("You must specify a username and password for the new account.");
		}
		else
			CreateWONAccount(aUsername.c_str(), aPassword.c_str());
	}
	else if (theCommand==COMMAND_LOGIN)
	{
		string aUsername, aPassword;
		if (GetCommandToken(aUsername,theInputBuf,1,FALSE) == -1 ||
			GetCommandToken(aPassword,theInputBuf,2,FALSE) == -1)
		{
			OutputString ("You must specify the username and password of the account that you want to use.");
		}
		else
			Login(aUsername.c_str(), aPassword.c_str());
	}
	else if (theCommand==COMMAND_LIST_USERS)
		ListUsers();
	else if (theCommand==COMMAND_CREATE_ROOM)
	{
		string aRoomName, aRoomPassword;
		if (GetCommandToken(aRoomName,theInputBuf,1,FALSE) == -1)
		{
			OutputString("You must specify the name of your new room.");	
		}
		else
		{
			// Get the room password (if any)
			GetCommandToken(aRoomPassword,theInputBuf,2,FALSE);

			WONString aUnicodePassword(aRoomPassword);
			CreateRoom(aRoomName.c_str(), aUnicodePassword.GetUnicodeCString());
		}
	}
	else if (theCommand==COMMAND_LIST_ROOMS)
		ListRooms();
	else if (theCommand==COMMAND_JOIN_ROOM)
	{
		string aRoomName, aRoomPassword;
		if (GetCommandToken(aRoomName,theInputBuf,1,FALSE) == -1)
		{
			OutputString("You must specify the name of the room to join.");	
		}
		else
		{
			// Get the room password (if any)
			GetCommandToken(aRoomPassword,theInputBuf,2,FALSE);

			WONString aUnicodePassword(aRoomPassword);
			JoinRoom(aRoomName.c_str(), aUnicodePassword.GetUnicodeCString());
		}
	}
	else if (theCommand==COMMAND_GET_ROOM_NAME) 
	{
		if (GetCurrentChatRoomName().empty())
			OutputString("Not connected to a chat room.");
		else
			OutputStringF("You are currently in room: %S", GetCurrentChatRoomName().GetUnicodeCString());
	}
	else if (theCommand==COMMAND_IGNORE_USER)
	{
		// Isolate the user name from the inputbuf and call IgnoreUser
		string theTargetUser;
		GetCommandToken(theTargetUser,theInputBuf,1,FALSE);
		IgnoreUser(theTargetUser.c_str());
	}
	else if (theCommand==COMMAND_UNIGNORE_USER) 
	{
		// Isolate the user name from the inputbuf and call UnignoreUser
		string theTargetUser;
		GetCommandToken(theTargetUser,theInputBuf,1,FALSE);
		UnignoreUser(theTargetUser.c_str());
	}
	else if (theCommand==COMMAND_HELP)
	{
		OutputStringF("/%s <username> <password> - create a new WON acccount.", COMMAND_CREATE_ACCOUNT);
		OutputStringF("/%s <username> <password> - specify username and password to use.", COMMAND_LOGIN);
		OutputStringF("/%s - get a list of rooms.", COMMAND_LIST_ROOMS);
		OutputStringF("/%s <room name> <password> - create a new chat room.", COMMAND_CREATE_ROOM);
		OutputStringF("/%s <room name> <password> - join the specified chat room.", COMMAND_JOIN_ROOM);
		OutputStringF("/%s - display the room name.", COMMAND_GET_ROOM_NAME);
		OutputStringF("/%s - get a list of users in current chat room.", COMMAND_LIST_USERS);
		OutputStringF("/%s <username> - ignore the specified user.", COMMAND_IGNORE_USER);
		OutputStringF("/%s <username> - unignore the specified user.", COMMAND_UNIGNORE_USER);

		// Always return false from COMMAND_HELP
		return FALSE;
	}
	else 
		return FALSE;	// command not handled

	// command handled
	return TRUE;
}