#ifdef DLLSAMPLE
#include <wondll.h>
#else
#include <Socket/IPSocket.h>
#include <Directory/DirectoryAPI.h>
using namespace WONAPI;
#endif // DLLSAMPLE
#include <msg/ServerStatus.h>
#include "PeerToPeer.h"

using namespace WONMsg;

const long gRequestTimeout = -1;
const char* FILE_VERIFIER_KEY  = "kver.pub";
const wchar_t* DIR_TITANSERVER = L"/TitanServers"; // Parent directory of dirs holding auth, firewall, and event servers.
const wchar_t* SERVERNAME_AUTH = L"AuthServer";    // Service name for auth servers

unsigned int gNumDirServers = 0;
unsigned int gNumAuthServers = 0;
#ifdef DLLSAMPLE
WONIPAddress* gDirServers = NULL;
WONIPAddress* gAuthServers = NULL;
std::list<WONIPAddress> gNewAuthServers;
HWONAUTH gAuthH;
#else
IPSocket::Address* gDirServers = NULL;
IPSocket::Address* gAuthServers = NULL;
Identity gIdentity;
#endif // DLLSAMPLE

void GetDirServers()
{
	// NOTE: Put Directory Server addresses in an INI file please!
#ifdef DLLSAMPLE
	gDirServers = new WONIPAddress[3];
	WONIPAddressSetFromStringAndPort(gDirServers + gNumDirServers++, "wontest.east.won.net", 15101);
	WONIPAddressSetFromStringAndPort(gDirServers + gNumDirServers++, "wontest.central.won.net", 15101);
	WONIPAddressSetFromStringAndPort(gDirServers + gNumDirServers++, "wontest.west.won.net", 15101);
#else
	gDirServers = new IPSocket::Address[3];
	gDirServers[gNumDirServers++] = IPSocket::Address("wontest.east.won.net", 15101);
	gDirServers[gNumDirServers++] = IPSocket::Address("wontest.central.won.net", 15101);
	gDirServers[gNumDirServers++] = IPSocket::Address("wontest.west.won.net", 15101);
#endif // DLLSAMPLE
}

void GetAuthServers()
{
#ifdef DLLSAMPLE
	mNewAuthServers.clear();
	
	HWONDATAOBJECT aDataObjectH = WONDataObjectCreate(OBJ_VALIDVERSIONS.c_str(), OBJ_VALIDVERSIONS.size(), NULL, 0);
	WONError aError = WONDirGetDirectoryW(NULL, gDirServers, gNumDirServers,
	                                      DIR_TITANSERVER, NULL, NULL,
	                                      WONDir_GF_DECOMPROOT | WONDir_GF_DECOMPRECURSIVE | WONDir_GF_DECOMPSERVICES | WONDir_GF_ADDTYPE | WONDir_GF_SERVADDNAME | WONDir_GF_SERVADDNETADDR | WONDir_GF_ADDDOTYPE | WONDir_GF_ADDDODATA,
	                                      &aDataObjectH, 1, AuthServerEntityCallback, this, gRequestTimeout);

	CopySTLAddressListToArray(mNewAuthServers,     &mAuthServers,     &mNumAuthServers);
#else
	WONMsg::DirEntityList entityList;
	Error aError = GetDirectory(NULL, // no identity needed to get TitanServers (after all, the AuthServers are listed in there)
								gDirServers, gNumDirServers,
								NULL,
								DIR_TITANSERVER, 
								&entityList,
								WONMsg::GF_DECOMPROOT | WONMsg::GF_DECOMPRECURSIVE | WONMsg::GF_DECOMPSERVICES | WONMsg::GF_ADDTYPE | WONMsg::GF_SERVADDNAME | WONMsg::GF_SERVADDNETADDR | WONMsg::GF_ADDDOTYPE | WONMsg::GF_ADDDODATA,
								WONCommon::DataObjectTypeSet(),
								0, 0,
								gRequestTimeout);

	switch(aError) 
	{
		case Error_Success:
		{
			delete[] gAuthServers; gNumAuthServers = 0;
			gAuthServers = new IPSocket::Address[entityList.size()];

			WONMsg::DirEntityList::const_iterator aDirEntityListItr = entityList.begin();
			for( ; aDirEntityListItr != entityList.end(); ++aDirEntityListItr)
			{
				if (aDirEntityListItr->mType == WONMsg::DirEntity::ET_SERVICE &&
					aDirEntityListItr->mName == SERVERNAME_AUTH)
					gAuthServers[gNumAuthServers++] = IPSocket::Address(*aDirEntityListItr);
			}

			break;
		}

		case StatusDir_DirNotFound:
//			OutputError("Directory containing Titan servers not found");
			break;
		default:
//			OutputError("Failed to get list of Titan servers!", aError);
			break;
	}
#endif // DLLSAMPLE
}

#ifdef DLLSAMPLE
void AuthServerEntityCallback(HWONDIRENTITY theDirEntityH, void* theMatchmakerP)
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
			gNewAuthServers.push_back(aAddress);
	}
}
#endif // DLLSAMPLE

bool Init()
{
	// load the verifier key
#ifdef DLLSAMPLE
	if (!WONAuthLoadVerifierKeyFromFileA(FILE_VERIFIER_KEY))
#else
	if (!Identity::LoadVerifierKeyFromFile(FILE_VERIFIER_KEY))
#endif // DLLSAMPLE
		return false;

	GetDirServers();
	GetAuthServers();

	// authenticate (get certificate)
#ifdef DLLSAMPLE
	WONError aError;
	HWONAUTH aAuthH;
	aAuthH = WONAuthLoginA(&aError, gAuthServers, gNumAuthServers, "SampleTest", "WON", "JASPTR", "", gRequestTimeout);
	if (aError != WONError_Success)
		return false;
#else
	Error aError;
	gIdentity = Identity("SampleTest", "WON", "JASPTR", "", gAuthServers, gNumAuthServers);
	aError = gIdentity.Authenticate(false, false, gRequestTimeout);
	if (aError != Error_Success)
		return false;
#endif // DLSAMPLE

	return true;
}

