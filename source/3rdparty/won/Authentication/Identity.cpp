#include <stdio.h>
#include "Identity.h"
#include "SDKCommon/MemStream.h"
#include "Socket/TMsgSocket.h"
#include "common/won.h"
#include "msg/Auth/TMsgAuth1LoginHW.h"
#include "msg/Auth/TMsgTypesAuth.h"
#include "msg/ServerStatus.h"
#include "Md52.h"

#ifdef WIN32
#include "common/RegKey.h"
#endif


using namespace std;
using namespace WONAPI;
using namespace WONCommon;
using namespace WONAuth;
using namespace WONCrypt;
using namespace WONCDKey;
using namespace WONMsg;


typedef list<Identity::AuthenticationData*> AuthDataList;
static AuthDataList pubKeyQueued;
static CriticalSection globalCrit;
static auto_ptr<Auth1PublicKeyBlock> autoPubKeys;
static auto_ptr<ClientCDKey> autoCDKey;
static Event refreshDoneEvent;
static Error refreshDoneError;
static bool everSetVerifierKey = false;

WONString Identity::hashFile;
WONString Identity::loginKeyFile;
Identity::LoginKeyMap Identity::loginKeys;
bool Identity::doMD5Hash = false;


EGPublicKey Identity::verifierKey;
Auth1PublicKeyBlock* Identity::pubKeys = 0;
ClientCDKey* Identity::CDKey = 0;


//const CompletionContainer<const Identity::AuthResult&> Identity::blankAuthResultCompletion;

// Block of state-data for async authentication request
class Identity::AuthenticationData
{
public:
//	bool hasIncomingLoginKey;
	LoginKey usingLoginKey;
	LoginKey incomingLoginKey;
	Identity* identity;
	long timeout;
	bool getPubKeys;
	bool newAcct;
	bool newNick;
	wstring newPassword;
	bool autoDelete;
	TCPSocket* theSocket;
	TMsgSocket* tMsgSocket;
	Error result;
	CompletionContainer<const Identity::AuthResult&> finalCompletion;
	list<IPSocket::Address>::iterator curAuthAddr;
	BFSymmetricKey sessionKey;
	Event doneEvent;

	void PubKeysDone(Error err);
	void Done(Error err);
	void TryNextAuthServer(Error err);
	void TryNextAuthServerForPubKeys(Error err);
	void GetPubKeys();

	friend class Identity;

//	AuthenticationData() : hasIncomingLoginKey(false)
//	{ }
};


#ifdef WIN32

bool Identity::LoadVerifierKeyFromRegistry(const string& regPath, const string& keyName)
{
	bool   aRet = false;
	RegKey aKey(regPath, HKEY_LOCAL_MACHINE);

	// If key is open, read verifier key and update AuthInfo
	if (aKey.IsOpen())
	{
		unsigned long  aKeyLen = 0;
		unsigned char* aKeyP   = NULL;
		if ((aKey.GetValue(keyName, aKeyP, aKeyLen) == RegKey::Ok) &&
			(aKeyLen > 0))
		{
			SetVerifierKey(aKeyP, aKeyLen);
			aRet = true;
		}
		delete aKeyP;
	}
	return aRet;
}
#endif


bool Identity::LoadVerifierKeyFromFile(const WONString& theFile)
{
	bool result = false;

#if defined(WIN32) && defined(UNICODE)
	FILE *verFile = _wfopen(theFile, L"rb");
#else
	FILE *verFile = fopen(theFile, "rb");
#endif

	if (verFile)
	{
		fseek(verFile, 0, SEEK_END);
		long fileSize = ftell(verFile);
		
		unsigned char* buf = new unsigned char[fileSize];
		if (buf)
		{
			fseek(verFile, 0, SEEK_SET);
			if (fread(buf, 1, fileSize, verFile) == fileSize)
			{
				SetVerifierKey(buf, fileSize);
				result = true;
			}
			delete[] buf;
		}
		fclose(verFile);
	}
	return result;
}


#ifdef WIN32
bool Identity::WriteVerifierKeyToRegistry(const string& regPath, const string& keyName)
{
	bool result = false;
	globalCrit.Enter();
	RegKey aKey(regPath, HKEY_LOCAL_MACHINE, true);
	if (aKey.IsOpen())
	{
		aKey.SetValue(keyName, verifierKey.GetKey(), verifierKey.GetKeyLen());
		result = true;
	}
	globalCrit.Leave();
	return result;
}
#endif


bool Identity::WriteVerifierKeyToFile(const WONString& theFile)
{
	bool result = false;
	long size = verifierKey.GetKeyLen();
	if (size)
	{
#if defined(WIN32) && defined(UNICODE)
		FILE *verFile = _wfopen(theFile, L"wb");
#else
		FILE *verFile = fopen(theFile, "wb");
#endif
		if (verFile && fwrite(verifierKey.GetKey(), 1, size, verFile) == size)
			result = true;
		fclose(verFile);
	}
	return result;
}


void Identity::SetVerifierKey(const WONCrypt::EGPublicKey& verifKey)
{
	globalCrit.Enter();
	everSetVerifierKey = true;
	verifierKey = verifKey;
	globalCrit.Leave();
}


void Identity::SetVerifierKey(void* verifKey, unsigned long verifKeyLength)
{
	if (verifKey && verifKeyLength)
	{
		EGPublicKey tempVerifierKey;
		try {
			tempVerifierKey.Create(verifKeyLength, (unsigned char*)verifKey);
		}
		catch (...)
		{
			return;
		}
		SetVerifierKey(tempVerifierKey);
	}
}


ClientCDKey* Identity::GetCDKey()
{
	ClientCDKey* result = 0;
	globalCrit.Enter();
	if (CDKey)
		result = new ClientCDKey(*CDKey);
	globalCrit.Leave();
	return result;
}


void Identity::SetCDKey(const ClientCDKey& newCDKey)
{
	globalCrit.Enter();
	if (CDKey)
		delete CDKey;
	CDKey = new ClientCDKey(newCDKey);
	globalCrit.Leave();
}


bool Identity::IsCDKeyValid()
{
	AutoCrit autoCrit(globalCrit);
	return hasCDKey() ? CDKey->IsValid() : false;
}

#ifdef WIN32
bool Identity::SaveCDKey()
{
	AutoCrit autoCrit(globalCrit);
	return hasCDKey() ? CDKey->Save() : false;
}


bool Identity::LoadCDKey(const string& productName, bool fromHKeyCurUser)
{
	AutoCrit autoCrit(globalCrit);

	if (!productName.empty())
	{
		ClientCDKey* tmpCDKey = new ClientCDKey(productName, fromHKeyCurUser);
		if (tmpCDKey)
		{
			auto_ptr<ClientCDKey> auto_tmpCDKey(tmpCDKey);
			if (tmpCDKey->Load())
			{
				if (tmpCDKey->IsValid())
				{
					if (hasCDKey())
						delete CDKey;
					CDKey = tmpCDKey;
					auto_tmpCDKey.release();
					return true;
				}
			}
		}
	}
	return false;
}
#endif

Identity::Identity()
	:	privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
}


Identity::Identity(const IPSocket::Address& addr)
	:	privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	authAddrs.push_back(addr);
}


Identity::Identity(const IPSocket::Address* addrs, unsigned int numAddrs)
	:	privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	for (unsigned int i = 0; i < numAddrs; i++)
		authAddrs.push_back(addrs[i]);
}


Identity::Identity(	const WONString& name, const WONString& community,
					const WONString& pw, const WONString& nickKey,
					const IPSocket::Address& addr)
	:	loginName(name),
		communityName(community),
		password(pw),
		nicknameKey(nickKey),
		privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	authAddrs.push_back(addr);
}


Identity::Identity(	const WONString& name, const WONString& community,
					const WONString& pw, const WONString& nickKey,
					const IPSocket::Address* addrs, unsigned int numAddrs)
	:	loginName(name),
		communityName(community),
		password(pw),
		nicknameKey(nickKey),
		privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	for (unsigned int i = 0; i < numAddrs; i++)
		authAddrs.push_back(addrs[i]);
}


Identity::Identity(	const WONString& name, const WONString& community,
					const WONString& pw, const WONString& nick,
					const WONString& nickKey, const IPSocket::Address& addr)
	:	loginName(name),
		communityName(community),
		password(pw),
		nickname(nick),
		nicknameKey(nickKey),
		privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	authAddrs.push_back(addr);
}


Identity::Identity(	const WONString& name, const WONString& community,
					const WONString& pw, const WONString& nick,
					const WONString& nickKey, const IPSocket::Address* addrs, unsigned int numAddrs)
	:	loginName(name),
		communityName(community),
		password(pw),
		nickname(nick),
		nicknameKey(nickKey),
		privateKey(0),
		certificate(0),
		refreshing(false),
		autoReauth(true)
{
	for (unsigned int i = 0; i < numAddrs; i++)
		authAddrs.push_back(addrs[i]);
}


Identity::Identity(const Identity& toCopy)
	:	loginName(toCopy.loginName),
		communityName(toCopy.communityName),
		password(toCopy.password),
		nickname(toCopy.nickname),
		nicknameKey(toCopy.nicknameKey),
		refreshing(false),
		authAddrs(toCopy.authAddrs),
		isNicknameDefault(toCopy.isNicknameDefault),
		certificateExpiration(toCopy.certificateExpiration),
		privateKey(toCopy.GetPrivateKey()),
		certificate(toCopy.GetCertificate()),
		autoReauth(true)
{
}


Identity& Identity::operator=(const Identity& toCopy)
{
	if (&toCopy == this)
		return *this;
	loginName = toCopy.loginName;
	communityName = toCopy.communityName;
	password = toCopy.password;
	nickname = toCopy.nickname;
	nicknameKey = toCopy.nicknameKey;
	refreshing = false;
	authAddrs = toCopy.authAddrs;
	isNicknameDefault = toCopy.isNicknameDefault;
	certificateExpiration = toCopy.certificateExpiration;
	if (privateKey)
		delete privateKey;
	privateKey = toCopy.GetPrivateKey();
	if (certificate)
		delete certificate;
	certificate = toCopy.GetCertificate();
	return *this;
}


Identity::~Identity(void)
{
	refreshCrit.Enter();
	if (refreshing)
	{
		refreshCrit.Leave();
		WSSocket::PumpUntil(refreshDoneEvent);
	}
	else
		refreshCrit.Leave();

	delete (EGPrivateKey*)privateKey;
	delete certificate;
}


Error Identity::AuthenticateNewPassword(const WONString& newPassword, long timeout, bool async, const CompletionContainer<const AuthResult&>& completion)
{
	if (newPassword.empty())
	{
		completion.Complete(AuthResult(this, Error_BadNewPassword));
		return Error_BadNewPassword;
	}
	return Authenticate(false, false, newPassword, timeout, async, completion);
}


//-------

Error Identity::SetPubKeyBlock(const unsigned char* buf, unsigned short size)
{
	Error err = Error_Success;

	globalCrit.Enter();

	if (!verifierKey.IsValid())
		err = Error_InvalidVerifierKey;
	else
	{
		Auth1PublicKeyBlock* newPubKeys = new Auth1PublicKeyBlock(buf, size);
		if (!newPubKeys)
			err = Error_OutOfMemory;
		else
		{
			bool success;
			try {
				success = newPubKeys->Verify(verifierKey);
			}
			catch (...)
			{
				success = false;
			}
			if (!success)
			{
				err = Error_InvalidPubKeys;
				delete newPubKeys;
			}
			else
			{
				auto_ptr<Auth1PublicKeyBlock> tempAutoPubKeys(newPubKeys);
				autoPubKeys = tempAutoPubKeys;	// Should delete old one, if any
				pubKeys = newPubKeys;
			}
		}
	}
	globalCrit.Leave();

	return err;
}


/*void Identity::FinishQueued(Error err)
{
	refreshCrit.Enter();
	while (refreshQueue.size())
	{
		CompletionContainer<const AuthResult&> queuedCompletion = refreshQueue.front();
		refreshQueue.pop();
		queuedCompletion.Complete(AuthResult(this, err));
	}
	refreshing = false;
	refreshDoneError = err;
	refreshDoneEvent.Set();
	refreshCrit.Leave();

}

*/


void Identity::AuthenticationData::Done(Error err)
{
	Identity* ident = identity;
	bool doDelete = autoDelete;
	CompletionContainer<const Identity::AuthResult&> tmpFinalCompletion = finalCompletion;

	
	//tMsgSocket->Close(0);	// Force closed, since we recv last
	delete tMsgSocket;

	result = err;

	ident->refreshCrit.Enter();
	queue< CompletionContainer<const AuthResult&> > tmpRefreshQueue = ident->refreshQueue;
	while (ident->refreshQueue.size())
		ident->refreshQueue.pop();
	ident->refreshing = false;
	refreshDoneError = err;
	refreshDoneEvent.Set();
	ident->refreshCrit.Leave();


	tmpFinalCompletion.Complete(AuthResult(ident, err));
	while (tmpRefreshQueue.size())
	{
		CompletionContainer<const AuthResult&> queuedCompletion = tmpRefreshQueue.front();
		tmpRefreshQueue.pop();
		queuedCompletion.Complete(AuthResult(ident, err));
	}

	if (doDelete)
		delete this;
	else
		doneEvent.Set();
}


void Identity::AuthenticationData::TryNextAuthServer(Error err)
{
	curAuthAddr++;

	if (curAuthAddr == identity->authAddrs.end())
		Done(err);
	else
	{
		theSocket->Close(0);
		theSocket->OpenEx(&(*curAuthAddr), timeout, true, OpenCompletion, this);
	}
}


static FILE* hashFileH = 0;


class CloseFile
{
public:
	~CloseFile()
	{
		if (hashFileH)
		{
			fclose(hashFileH);
			hashFileH = 0;
		}
	}
};

static CloseFile closeFile;


static unsigned long GetHashSection(bool restart, unsigned char** theUnhashedBufP, unsigned char digest[MD5_HASH_SIZE])
{
    const unsigned int HASH_CHUNK_SIZE = 16384; // number of writes read from the exe per GetHashSection call

    if (restart)
    {
		WONCommon::WONString fileName = Identity::GetHashFile();
        if (fileName.empty())
        {
#ifdef WIN32
			TCHAR tmpFileName[MAX_PATH];
			if (GetModuleFileName(GetModuleHandle(NULL), tmpFileName, MAX_PATH) == 0)
				return 0;
			Identity::SetHashFile(tmpFileName);
			fileName = tmpFileName;
#else//if defined(_LINUX)
		return 0;
#endif
		}

#if defined(WIN32) && defined(UNICODE)
		hashFileH = _wfopen(fileName, L"rb");
#else
		hashFileH = fopen(fileName, "rb");
#endif

/*		hashFileH = CreateFile(	fileName,
								GENERIC_READ,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_FLAG_SEQUENTIAL_SCAN,
								NULL);
*/
		if (!hashFileH)
			return 0;

        if (*theUnhashedBufP == NULL)
            *theUnhashedBufP = new unsigned char[HASH_CHUNK_SIZE];
    }

	size_t bytesRead = fread(*theUnhashedBufP, 1, HASH_CHUNK_SIZE, hashFileH);
//	if (!bytesRead)
//		return 0;

//    DWORD bytesRead;
//    if (ReadFile(hashFileH, *theUnhashedBufP, HASH_CHUNK_SIZE, &bytesRead, NULL) == 0	|| bytesRead == 0)
//        return 0;

    struct MD5Context Context;
    MD5Init2(&Context);
    MD5Update2(&Context, *theUnhashedBufP, bytesRead);
    MD5Final2(digest, &Context);

    return bytesRead;
}


void Identity::LoginCompletion(const TMsgSocket::RecvRawMsgResult& result, AuthenticationData* authData)
{
	if (!result.msg)
	{
//		if (authData->hasIncomingLoginKey)
//			AddLoginKey(*(authData->curAuthAddr), authData->incomingLoginKey, false);
//		else
//			WriteLoginKeys();	// ??
		authData->TryNextAuthServer(result.closed ? Error_ConnectionClosed : Error_Timeout);
		return;
	}
	
	auto_ptr<unsigned char> autoMsg(result.msg);

	unsigned long serverType = *((unsigned long*)result.msg);
	makeLittleEndian(serverType);
	if (serverType != Auth1Login)	// 201
	{
		authData->Done(Error_InvalidAuthLoginReply);
		return;
	}

	unsigned long msgType = *((unsigned long*)result.msg + 1);
	makeLittleEndian(msgType);
	if (msgType != Auth1LoginReply)	// 4
	{
		// Handle cd key challenge
		if (msgType == Auth1LoginChallengeHW && (Identity::hasCDKey() || Identity::willHash())) // 32
		{
			TMsgAuth1ChallengeHW msg(TMessage(result.msgLength, result.msg));

			CryptKeyBase::CryptReturn cryptResult(NULL,0);
			cryptResult = authData->sessionKey.Decrypt(msg.GetRawBuf(), msg.GetRawBufLen());

			if (cryptResult.first == NULL)
			{
				authData->TryNextAuthServer(Error_InvalidMessage);
				return;
			}

			auto_ptr<unsigned char> deleteCryptResult(cryptResult.first);

			if(cryptResult.second != 16)
			{
				authData->TryNextAuthServer(Error_InvalidMessage);
				return;
			}
			
			TMsgAuth1ConfirmHW reply;

			unsigned char aChallengeResponse[32];
				
			struct MD5Context ContextEntireFile; // hash without seed
			struct MD5Context ContextKeyedHash;  // hash with seed
			if (Identity::willHash())
			{
				MD5Init2(&ContextEntireFile);
				MD5Init2(&ContextKeyedHash);

				MD5Update2(&ContextKeyedHash, (BYTE*)cryptResult.first, cryptResult.second);

				bool firsttime = true;
				unsigned long aBytesRead = 0;
				unsigned char* aUnhashedBuf = NULL;
				while ((aBytesRead = GetHashSection(firsttime, &aUnhashedBuf, reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE)) != 0)
				{
					MD5Update2(&ContextEntireFile, aUnhashedBuf, aBytesRead);
					MD5Update2(&ContextKeyedHash, reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE, MD5_HASH_SIZE);
					firsttime = false;
				}
				if (hashFileH)
				{
					fclose(hashFileH);
					hashFileH = 0;
				}

				MD5Final2(reinterpret_cast<unsigned char*>(aChallengeResponse), &ContextEntireFile);
				MD5Final2(reinterpret_cast<unsigned char*>(aChallengeResponse) + MD5_HASH_SIZE, &ContextKeyedHash);

				delete aUnhashedBuf;
				aUnhashedBuf = NULL;
			}
			CryptKeyBase::CryptReturn aReplyCrypt(NULL,0);
			aReplyCrypt = authData->sessionKey.Encrypt((const unsigned char*)aChallengeResponse, MD5_HASH_SIZE*2);

			if (!aReplyCrypt.first)
			{
				authData->Done(Error_EncryptFailed);
				return;
			}
			auto_ptr<unsigned char> aDeleteReplyCrypt(aReplyCrypt.first);

			//authData->hasIncomingLoginKey = true;
			memcpy(&(authData->incomingLoginKey), cryptResult.first, 8);
			authData->incomingLoginKey.peered = false;
			AddLoginKey(*(authData->curAuthAddr), authData->incomingLoginKey, false);
			
			reply.SetRawBuf(aReplyCrypt.first,aReplyCrypt.second,true);

			authData->tMsgSocket->SendBaseMsg(reply, -1, true, true);
			authData->tMsgSocket->RecvRawMsgEx(0, NULL, authData->timeout, true, LoginCompletion, authData);
		}
		else
			authData->Done(Error_InvalidAuthLoginReply);
		return;
	}

	short status = *(short*)((unsigned long*)result.msg + 2);
	makeLittleEndian(status);
	if (status != 0)
	{
		Error err;
		
		if (status == StatusAuth_KeyInUse) //-1503
		{
			if (hasCDKey())
			{
				// Invalidate both the key that we sent up in the login request and the key
				// that the Auth Server gave us in the challenge.  They're both wrong.
				bool b = InvalidateLoginKey(*(authData->curAuthAddr), authData->incomingLoginKey, authData->usingLoginKey);
				if (b)
				{
					authData->theSocket->Close(0);
					authData->theSocket->OpenEx(&*(authData->curAuthAddr), authData->timeout, true, OpenCompletion, authData);
					return;
				}
			}
			err = Error_KeyInUse;
		}
		else
		{
			// Invalidate only the key that the Auth Server gave us in the challenge.  It's not
			// correct (Auth Server only switches to new key on success).
			if (hasCDKey())
				InvalidateLoginKey(*(authData->curAuthAddr), authData->incomingLoginKey, authData->incomingLoginKey);

			switch (status)
			{
			case StatusCommon_InvalidParameters:	//-4
				err = Error_InvalidAuthLogin;
				break;
			case StatusAuth_ExpiredKey:				//-1500
				err = Error_ExpiredKey;
				break;
			case StatusAuth_VerifyFailed:			//-1501
				err = Error_VerifyFailed;
				break;
			case StatusAuth_LockedOut:				//-1502
				err = Error_LockedOut;
				break;
			case StatusAuth_CRCFailed:				//-1504
				err = Error_CRCFailed;
				break;
			case StatusAuth_UserExists:				//-1505
				err = Error_UserExists;
				break;
			case StatusAuth_UserNotFound:			//-1506
				err = Error_UserNotFound;
				break;
			case StatusAuth_BadPassword:			//-1507
				err = Error_BadPassword;
				break;
			case StatusAuth_DirtyWord:				//-1508
				err = Error_DirtyWord;
				break;
			case StatusAuth_BadCommunity:			//-1509
				err = Error_BadCommunity;			
				break;
			case StatusAuth_InvalidCDKey:			//-1510
				err = Error_InvalidCDKey;			
				break;
			case StatusAuth_NotInCommunity:			//-1511
				err = Error_NotInCommunity;			
				break;
			case StatusCommon_Failure:				//-1
				authData->TryNextAuthServer(Error_GeneralFailure);
				return;
			default:
				err = Error_InvalidAuthLoginReply;
			};
		}
		authData->Done(err);
		return;
	}
	
	if (!authData->newPassword.empty())
		authData->identity->password = authData->newPassword;

	unsigned char numErrors;
	MemStream memStream(((unsigned char*)result.msg)+10, result.msgLength - 10);
	if (!memStream.Read(1, &numErrors))
	{
		authData->Done(Error_InvalidAuthLoginReply);
		return;
	}
	
	authData->identity->errStrings.clear();
	while (numErrors--)
	{
		unsigned short errLength;
		if (memStream.Read(2, &errLength) != 2)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
		makeLittleEndian(errLength);
		
		wchar_t* errString = new wchar_t[errLength];
		if (!errString)
		{
			authData->Done(Error_OutOfMemory);
			return;
		}
		if (memStream.Read(errLength*2, errString) != errLength*2)
		{
			delete errString;
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
		wstring tmpS(errString, errLength);
		makeLittleEndianWString(tmpS);
		authData->identity->errStrings.push_back(tmpS);
		delete errString;	// chuck it, for now...
	}

	unsigned char numClearEntries;

	if (!memStream.Read(1, &numClearEntries))
	{
		authData->Done(Error_InvalidAuthLoginReply);
		return;
	}
	
	while (numClearEntries--)
	{
		unsigned char blockType;
		if (!memStream.Read(1, &blockType))
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}

		unsigned short blockLen;
		if ((memStream.Read(2, &blockLen) != 2) || !blockLen)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
		makeLittleEndian(blockLen);

		unsigned char* block = memStream.GetBufferAtPos();
		if (memStream.Read(blockLen, 0) != blockLen)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}

		switch (blockType)
		{

	//	Never in the clear:
	//		ALClientPrivateKey = 2,  // Client's Private Key
	//		ALSecretConfirm    = 4,  // Client secret confirmation
	//		ALNicknameInfo     = 5,  // Nickname information

		case 1:	// ALCertificate:
			{
					// Construct from raw representation (calls Unpack())
				Auth1Certificate* cert = new Auth1Certificate(block, blockLen);
				if (!cert)
				{
					authData->Done(Error_OutOfMemory);
					return;
				}

				globalCrit.Enter();
				bool valid = pubKeys->VerifyFamilyBuffer(*cert);
				globalCrit.Leave();
				
				if (!valid)
				{
					delete cert;
					authData->Done(Error_InvalidAuthLoginReply);
					return;
				}
				if (authData->identity->certificate)
					delete authData->identity->certificate;
				authData->identity->certificate = cert;

				time_t delta = cert->GetIssueTime() - time(NULL);
				authData->identity->expirationDelta = delta;
				authData->identity->certificateExpiration = (cert->GetExpireTime() - delta) - 120; 	// 2 minutes padding
			}
			break;
		case 3: // ALPublicKeyBlock:
			{
				Error err = SetPubKeyBlock(block, blockLen);
				if (err != Error_Success)
				{
					authData->Done(err);
					return;
				}
			}
			break;
		default:
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
	}

	BFSymmetricKey::CryptReturn decryptedReply = authData->sessionKey.Decrypt((unsigned char*)memStream.GetBufferAtPos(), memStream.GetBufferSizeLeft());
			
	if (!decryptedReply.second)
	{
		authData->Done(Error_InvalidAuthLoginReply);
		return;
	}
	if (!decryptedReply.first)
	{
		authData->Done(Error_OutOfMemory);
		return;
	}
	auto_ptr<unsigned char> decryptedReplyAuthPtr(decryptedReply.first);

	MemStream decryptedStream((unsigned char*)(decryptedReply.first), decryptedReply.second);
	unsigned char decryptedBlocks;
	if (decryptedStream.Read(1, &decryptedBlocks) != 1)
	{
		authData->Done(Error_InvalidAuthLoginReply);
		return;
	}
	bool gotSecret = false;
	while (decryptedBlocks--)
	{
		unsigned char blockType;
		if (decryptedStream.Read(1, &blockType) != 1)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}

		unsigned short blockLen;
		if ((decryptedStream.Read(2, &blockLen) != 2) || !blockLen)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
		makeLittleEndian(blockLen);

		unsigned char* block = decryptedStream.GetBufferAtPos();
		if (decryptedStream.Read(blockLen, 0) != blockLen)
		{
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}

		switch (blockType)
		{
		case 4:	// ALSecretConfirm
			{
				unsigned long keyLen = authData->sessionKey.GetKeyLen();
				if (keyLen == blockLen-2)	// skip random pad
				{
					void* ourSecret = (void*)(authData->sessionKey.GetKey());
					if (!memcmp(ourSecret, block+2, blockLen-2))	// skip random pad
					{
						gotSecret = true;
						break;
					}
				}
				authData->Done(Error_InvalidAuthLoginReply);
				return;
			}
		case 5:	// ALNicknameInfo
			{
				if (*block == 0xFF)
				{
					authData->identity->isNicknameDefault = true;
					authData->identity->nickname = authData->identity->loginName;
				}
				else
				{
					authData->identity->isNicknameDefault = *block == '1';
					WONString tmpS(((wchar*)(block+3)), (blockLen-3)/2);
					//wstring tmpS(((wchar_t*)(block+3)), (blockLen-3)/2);
					//makeLittleEndianWString(tmpS);
					authData->identity->nickname = tmpS.GetUnicodeString();
				}
			}
			break;
		case 2:	// ALClientPrivateKey
			{
				EGPrivateKey* newPrivateKey = new EGPrivateKey;
				if (!newPrivateKey)
				{
					authData->Done(Error_OutOfMemory);
					return;
				}
				try {
					newPrivateKey->Create(blockLen, block);
				}
				catch (...)
				{
					delete newPrivateKey;
					authData->Done(Error_InvalidAuthLoginReply);
					return;
				}
				if (authData->identity->privateKey)
					delete (EGPrivateKey*)authData->identity->privateKey;
				authData->identity->privateKey = newPrivateKey;
			}
			break;
		default:
			authData->Done(Error_InvalidAuthLoginReply);
			return;
		}
	}
	if (gotSecret)
	{
		if (authData->identity->hasCDKey())
			AddLoginKey(*(authData->curAuthAddr), authData->incomingLoginKey, true);
	}
	authData->Done(gotSecret ? Error_Success : Error_InvalidAuthLoginReply);
}


void Identity::OpenCompletion(const Socket::OpenResult& result, AuthenticationData* authData)
{
	if (result.error != Error_Success)
	{
		authData->TryNextAuthServer(result.error);
		return;
	}

	size_t bufSize = 14 + ((
		authData->identity->loginName.size()
	+	authData->identity->communityName.size()
	+	authData->identity->nicknameKey.size()
	+	authData->identity->password.size()
	+	authData->newPassword.size()
		) * 2);

	if (hasCDKey() || willHash())
		bufSize += 20;	// 2+8+2+8

	if (authData->newNick)
		bufSize += 2 + (authData->identity->nickname.size() * 2);

	char* buffer = new char[bufSize];
	
	if (!buffer)
	{
		authData->Done(Error_OutOfMemory);
		return;
	}
	auto_ptr<char> autoBuffer(buffer);

	unsigned short blockId = pubKeys->GetBlockId();
	EGPublicKey::CryptReturn encryptedSessionKey = pubKeys->EncryptRawBuffer(authData->sessionKey.GetKey(), authData->sessionKey.GetKeyLen());

	if (!encryptedSessionKey.first)
	{
		authData->Done(Error_OutOfMemory);
		return;
	}
	auto_ptr<unsigned char> AuthEncryptedSessionKeyPtr(encryptedSessionKey.first);
	
	MemStream bufferStream(buffer, bufSize);
	
	bufferStream << blockId;
	bufferStream << (!authData->identity->certificate);	// needKey
	bufferStream << authData->newAcct;
	
	bufferStream << (unsigned short)authData->identity->loginName.size();
	bufferStream << authData->identity->loginName.GetUnicodeString();
	bufferStream << (unsigned short)authData->identity->communityName.size();
	bufferStream << authData->identity->communityName.GetUnicodeString();
	bufferStream << (unsigned short)authData->identity->nicknameKey.size();
	bufferStream << authData->identity->nicknameKey.GetUnicodeString();
	bufferStream << (unsigned short)authData->identity->password.size();
	bufferStream << authData->identity->password.GetUnicodeString();
	bufferStream << (unsigned short)authData->newPassword.size();
	bufferStream << authData->newPassword;
	if (hasCDKey() || willHash())
	{
		__int64 rawCDKey = hasCDKey() ? CDKey->AsRaw() : 0; // message is encrypted, so key doesn't need to be

		bufferStream << (unsigned short)8;
		bufferStream.Write(8, &rawCDKey);

		bufferStream << (unsigned short)8;
		
		ReadLoginKey(*(authData->curAuthAddr), authData->usingLoginKey);
		bufferStream.Write(8, &(authData->usingLoginKey));
	}

	if (authData->newNick)
	{
		bufferStream << (unsigned short)authData->identity->nickname.size();
		bufferStream << authData->identity->nickname.GetUnicodeString();
	}

	// Encrypt a block.  Returns allocated output block and length of output block
	// (CryptReturn).  Caller must delete the returned block (return.first).
	BFSymmetricKey::CryptReturn encryptedDataBlock = authData->sessionKey.Encrypt(buffer, bufSize);

	autoBuffer.release();
	delete buffer;

	if (!encryptedDataBlock.first)
	{
		authData->Done(Error_OutOfMemory);
		return;
	}
	auto_ptr<unsigned char> AuthEncryptedDataBlockPtr(encryptedDataBlock.first);

	size_t sendBufSize = 12 + encryptedSessionKey.second + encryptedDataBlock.second;

	buffer = new char[sendBufSize];

	if (!buffer)
	{
		authData->Done(Error_OutOfMemory);
		return;
	}
	auto_ptr<char> autoBuffer2(buffer);
	
	MemStream sendBufferStream(buffer, sendBufSize);
	
	sendBufferStream << (unsigned long)Auth1Login;	// 201
	if (hasCDKey() || willHash())
		sendBufferStream << (unsigned long)Auth1LoginRequestHW;	// 30
	else
		sendBufferStream << (unsigned long)Auth1LoginRequest2;	// 3
	sendBufferStream << blockId;
	sendBufferStream << (unsigned short)encryptedSessionKey.second;
	sendBufferStream.Write(encryptedSessionKey.second, encryptedSessionKey.first);
	sendBufferStream.Write(encryptedDataBlock.second, encryptedDataBlock.first);
	
	AuthEncryptedSessionKeyPtr.release();
	delete encryptedSessionKey.first;
	AuthEncryptedDataBlockPtr.release();
	delete encryptedDataBlock.first;
		
	authData->tMsgSocket->SendRawMsg(sendBufSize, buffer, -1, true, true);
	autoBuffer2.release();
	delete buffer;
	authData->tMsgSocket->RecvRawMsgEx(0, NULL, authData->timeout, true, LoginCompletion, authData);
}



void Identity::AuthenticationData::PubKeysDone(Error err)
{
	globalCrit.Enter();

	AuthDataList::iterator itor = pubKeyQueued.begin();

	while (itor != pubKeyQueued.end())
	{
		AuthenticationData* authData = *itor;
		if (err != Error_Success)
			authData->Done(err);
		else
		{
			if (authData == this)
				OpenCompletion(Socket::OpenResult(theSocket, Error_Success), authData);
			else
			{
				authData->theSocket->Close(0);
				authData->theSocket->OpenEx(&*(authData->curAuthAddr), timeout, true, OpenCompletion, authData);
			}
		}
		itor++;
	}
	
	pubKeyQueued.clear();

	globalCrit.Leave();
}


void Identity::AuthenticationData::TryNextAuthServerForPubKeys(Error err)
{
	curAuthAddr++;

	if (curAuthAddr == identity->authAddrs.end())
		PubKeysDone(err);
	else
	{
		theSocket->Close(0);
		theSocket->OpenEx(&*curAuthAddr, timeout, true, Identity::PubKeysOpenCompletion, this);
	}
}


void Identity::PubKeysCompletion(const TMsgSocket::RecvRawMsgResult& result, AuthenticationData* authData)
{
	if (!result.msg)
	{
		authData->TryNextAuthServerForPubKeys(result.closed ? Error_ConnectionClosed : Error_Timeout);
		return;
	}
	
	auto_ptr<unsigned char> autoMsg(result.msg);

	if ((getLittleEndian(*(unsigned long*)result.msg) != Auth1Login)					// 201
	  || (getLittleEndian(*((unsigned long*)result.msg+1)) != Auth1GetPubKeysReply)	// 2
	  || (getLittleEndian(*(unsigned short*)((unsigned long*)result.msg+2)) != 0))// 0 == common success status
	{
		authData->TryNextAuthServerForPubKeys(Error_GetPubKeysFailed);
		return;
	}
	Error err;
	unsigned short pubKeySize = *(unsigned short*)(((unsigned char*)result.msg) + 10);
	makeLittleEndian(pubKeySize);
	if (pubKeySize != result.msgLength - 12)
		err = Error_InvalidPubKeys;
	else
		err = SetPubKeyBlock(((unsigned char*)result.msg) + 12, pubKeySize);
	
	if (err == Error_InvalidPubKeys)
		authData->TryNextAuthServerForPubKeys(Error_InvalidPubKeys);
	else
		authData->PubKeysDone(err);
}


void Identity::PubKeysOpenCompletion(const Socket::OpenResult& result, AuthenticationData* authData)
{
	if (result.error != Error_Success)
		authData->TryNextAuthServerForPubKeys(result.error);
	else
	{
		struct {
			long serviceType;
			long messageType;
		} msg = { Auth1Login, Auth1GetPubKeys };	// 201, 1
		
		makeLittleEndian(msg.serviceType);
		makeLittleEndian(msg.messageType);

		authData->tMsgSocket->SendRawMsg(sizeof(msg), &msg, -1, true, true);
		authData->tMsgSocket->RecvRawMsgEx(NULL, NULL, authData->timeout, true, PubKeysCompletion, authData);
	}
}


void Identity::AuthenticationData::GetPubKeys()
{
	globalCrit.Enter();
	if (pubKeys)
		theSocket->OpenEx(&*curAuthAddr, timeout, true, Identity::OpenCompletion, this);
	else
	{
		pubKeyQueued.push_back(this);
		if (pubKeyQueued.size() == 1)
			theSocket->OpenEx(&*curAuthAddr, timeout, true, Identity::PubKeysOpenCompletion, this);
	}
	globalCrit.Leave();
}


Error Identity::DoCompletion(Error err, const CompletionContainer<const AuthResult&>& completion)
{
	refreshCrit.Enter();
	queue< CompletionContainer<const AuthResult&> > tmpRefreshQueue = refreshQueue;
	while (refreshQueue.size())
		refreshQueue.pop();
	refreshing = false;
	refreshDoneError = err;
	refreshDoneEvent.Set();
	refreshCrit.Leave();

	completion.Complete(AuthResult(this, err));
	while (tmpRefreshQueue.size())
	{
		CompletionContainer<const AuthResult&> queuedCompletion = tmpRefreshQueue.front();
		tmpRefreshQueue.pop();
		queuedCompletion.Complete(AuthResult(this, err));
	}
	
	//	FinishQueued(err);
	return err;
}


Error Identity::Authenticate(bool newAcct, bool newNick, const WONString& newPassword, long timeout, bool async, const CompletionContainer<const AuthResult&>& completion)
{
	if (authAddrs.empty())
		return DoCompletion(Error_BadAddress, completion);

	refreshCrit.Enter();

#ifdef WIN32
	if (!everSetVerifierKey)
		LoadVerifierKeyFromRegistry();
#endif

	if (CDKey && !CDKey->IsValid())
		return DoCompletion(Error_InvalidCDKey, completion);

	if (refreshing)
	{
		if (newAcct || newNick || !newPassword.empty())
		{
			refreshCrit.Leave();
			return DoCompletion(Error_InvalidState, completion);
		}
		
		// Already refreshing...  Add this req to a queue, and wait if synch
		refreshQueue.push(completion);
		refreshCrit.Leave();		

		if (async)
			return Error_Pending;

		WSSocket::PumpUntil(refreshDoneEvent, timeout);
		//refreshDoneEvent.WaitFor();
		return refreshDoneError;
	}
	else
	{
		refreshing = true;
		refreshDoneEvent.Reset();
	}
	refreshCrit.Leave();


	TCPSocket* tcpSocket = new TCPSocket;
	if (!tcpSocket)
		return DoCompletion(Error_OutOfMemory, completion);

	TMsgSocket* tMsgSocket = new TMsgSocket(tcpSocket, true, 4);
	if (!tMsgSocket)
	{
		delete tcpSocket;
		return DoCompletion(Error_OutOfMemory, completion);
	}

	AuthenticationData* authData = new AuthenticationData();
	if (!authData)
	{
		delete tMsgSocket;
		return DoCompletion(Error_OutOfMemory, completion);
	}

	try {
		authData->sessionKey.Create(8);
	}
	catch (...)
	{
		delete authData;
		delete tMsgSocket;
		return DoCompletion(Error_FailedToGenerateKey, completion);
	}

	const unsigned char* sessionKeyVal = authData->sessionKey.GetKey();


	
	authData->curAuthAddr = authAddrs.begin();
	authData->theSocket = tcpSocket;
	authData->tMsgSocket = tMsgSocket;
	authData->timeout = timeout;
	authData->identity = this;
	authData->finalCompletion = completion;
	authData->autoDelete = async;
	authData->newPassword = newPassword.GetUnicodeString();
	authData->newNick = newNick;
	authData->newAcct = newAcct;

	authData->GetPubKeys();
				
	if (async)
		return Error_Pending;

	WSSocket::PumpUntil(authData->doneEvent, timeout);
	//WSSocket::PumpUntil(refreshDoneEvent);
	//refreshDoneEvent.WaitFor();

	refreshCrit.Enter();
	refreshCrit.Leave();

	Error result = authData->result;

	delete authData;

	return result;
}


bool Identity::IsExpired() const
{
	if (!certificate)
		return true;

	time_t now;
	time(&now);
	return (now > certificateExpiration);
}


Error Identity::Refresh(bool force, long timeout, bool async, const CompletionContainer<const AuthResult&>& completion)
{
	if (!force)
	{
		if (!IsExpired())
			return DoCompletion(Error_Success, completion);
	}
	return Authenticate(false, false, timeout, async, completion);
}


void Identity::Invalidate()
{
	globalCrit.Enter();
	if (certificate)
		delete certificate;
	certificate = 0;
	globalCrit.Leave();
}


Auth1Certificate* Identity::GetCertificate() const
{
	AutoCrit autoCrit(globalCrit);

	Auth1Certificate* result = 0;
	if (certificate)
		result = new Auth1Certificate(*certificate);
	return result;
}


void Identity::GetCertificate(void* certificateBuf, unsigned short* bufSize) const
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (certificateBuf)
		{
			AutoCrit autoCrit(globalCrit);

			if (certificate)
			{
				const unsigned char* rawBuf = certificate->GetRaw();
				unsigned short rawLen = certificate->GetRawLen();
				*bufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(certificateBuf, rawBuf, rawLen);
			}
		}
	}
}



EGPrivateKey* Identity::GetPrivateKey() const
{
	AutoCrit autoCrit(globalCrit);

	EGPrivateKey* result = 0;
	if (privateKey)
		result = new EGPrivateKey(*privateKey);
	return result;
}


void Identity::GetPrivateKey(void* privateKeyBuf, unsigned short* bufSize) const
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (privateKeyBuf)
		{
			AutoCrit autoCrit(globalCrit);
			if (privateKey)
			{
				const unsigned char* rawBuf = privateKey->GetKey();
				unsigned short rawLen = privateKey->GetKeyLen();
				*bufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(privateKeyBuf, rawBuf, rawLen);
			}
		}
	}
}



EGPublicKey* Identity::GetPublicKey() const
{
	AutoCrit autoCrit(globalCrit);
	EGPublicKey* result = 0;
	if (certificate)
		result = new EGPublicKey(certificate->GetPubKey());
	return result;
}


void Identity::GetPublicKey(void* publicKeyBuf, unsigned short* bufSize) const
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (publicKeyBuf)
		{
			AutoCrit autoCrit(globalCrit);
			if (certificate)
			{
				const unsigned char* rawBuf = certificate->GetPubKey().GetKey();
				unsigned short rawLen = certificate->GetPubKey().GetKeyLen();
				*bufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(publicKeyBuf, rawBuf, rawLen);
			}
		}
	}
}


EGPublicKey* Identity::GetVerifierKey()
{
	AutoCrit autoCrit(globalCrit);
	EGPublicKey* result = 0;
	if (verifierKey.GetKeyLen())
		result = new EGPublicKey(verifierKey);
	return result;
}


void Identity::GetVerifierKey(void* verifierKeyBuf, unsigned short* bufSize)
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (verifierKeyBuf)
		{
			AutoCrit autoCrit(globalCrit);

			unsigned short rawLen = verifierKey.GetKeyLen();
			if (rawLen)
			{
				const unsigned char* rawBuf = verifierKey.GetKey();
				*bufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(verifierKeyBuf, rawBuf, rawLen);
			}
		}
	}
}


Auth1PublicKeyBlock* Identity::GetPublicKeyBlock()
{
	AutoCrit autoCrit(globalCrit);
	Auth1PublicKeyBlock* result = 0;
	if (pubKeys)
		result = new Auth1PublicKeyBlock(*pubKeys);
	return result;
}


void Identity::GetPublicKeyBlock(void* publicKeyBlockBuf, unsigned short* bufSize)
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (publicKeyBlockBuf)
		{
			AutoCrit autoCrit(globalCrit);
			if (pubKeys)
			{
				const unsigned char* rawBuf = pubKeys->GetRaw();
				unsigned short rawLen = pubKeys->GetRawLen();
				*bufSize = rawLen;
				if (maxSize >= rawLen)
					memcpy(publicKeyBlockBuf, rawBuf, rawLen);
			}
		}
	}
}



void Identity::SetServers(const IPSocket::Address* addrs, unsigned int numAddrs)
{
	authAddrs.clear();
	for (unsigned int i = 0; i < numAddrs; i++)
		authAddrs.push_back(addrs[i]);
}


unsigned long Identity::GetUserId() const
{
	if (!certificate)
		return 0;
	return certificate->GetUserId();
}


unsigned long Identity::GetCommunityId() const
{
	if (!certificate)
		return 0;
	return certificate->GetCommunityId();
}


unsigned short Identity::GetTrustLevel() const
{
	if (!certificate)
		return 0;
	return certificate->GetTrustLevel();
}


void Identity::WriteLoginKeys()
{
	AutoCrit gCrit(globalCrit);

	// If the file hasn't been specified open a default file (see ReadLoginKeys())
	if (loginKeyFile.size() == 0)
		Identity::SetLoginKeyFile("login.ks");

	if (loginKeyFile.size() > 0)	/* probably not needed but I'll leave it in */
	{
#if defined(WIN32) && defined(UNICODE)
		FILE *keyFile = _wfopen(loginKeyFile, L"wb");
#else
		FILE *keyFile = fopen(loginKeyFile, "wb");
#endif
		if (keyFile)
		{
			unsigned char numKeys = loginKeys.size();
			fputc(numKeys, keyFile);

			LoginKeyMap::iterator itor = loginKeys.begin();
			while (itor != loginKeys.end())
			{
				LoginKeyStruct* keyStruct = &((*itor).second.first);
				LoginKeyList* keyList = &((*itor).second.second);
				keyStruct->numLoginKeys = keyList->size();

				size_t bufSize = fwrite(keyStruct, sizeof(LoginKeyStruct), 1, keyFile);
				
				LoginKeyList::iterator itor2 = keyList->begin();
				while (itor2 != keyList->end())
				{
					LoginKey key = *itor2;
					bufSize = fwrite(&key, sizeof(LoginKey), 1, keyFile);
					++itor2;
				}
				++itor;
			}
			fclose(keyFile);
		}
	}
}


// Returns true if was previously saved
bool Identity::ReadLoginKey(const IPSocket::Address& forAddr, LoginKey& key)
{
	AutoCrit gCrit(globalCrit);

	if (loginKeys.size() == 0)
	{
		// If the file hasn't been specified open a default file (see WriteLoginKeys())
		if (loginKeyFile.size() == 0)
			SetLoginKeyFile("login.ks");

		if (loginKeyFile.size() != 0)	/* probably not needed but I'll leave it in */
		{
#if defined(WIN32) && defined(UNICODE)
			FILE *keyFile = _wfopen(loginKeyFile, L"rb");
#else
			FILE *keyFile = fopen(loginKeyFile, "rb");
#endif
			if (keyFile)
			{
				unsigned char numServers = fgetc(keyFile);
				while (numServers--)
				{
					LoginKeyStruct keyStruct;

					size_t bufSize = fread(&keyStruct, sizeof(LoginKeyStruct), 1, keyFile);
					if (bufSize == 0)
						break;
					
					LoginKeyList keyList;

					unsigned short numLoginKeys = keyStruct.numLoginKeys;
					while (numLoginKeys--)
					{
						LoginKey key;
						bufSize = fread(&key, sizeof(LoginKey), 1, keyFile);
						if (bufSize == 0)
							break;

						keyList.push_back(key);
					}

					loginKeys.insert(LoginKeyMap::value_type(IPSocket::Address(keyStruct.authAddr, keyStruct.authPort), pair<LoginKeyStruct, LoginKeyList>(keyStruct, keyList)));
				}
				fclose(keyFile);
			}
		}
	}

	LoginKeyMap::iterator itor = loginKeys.find(forAddr);

	if (itor != loginKeys.end())
	{
		LoginKeyList* keyList = &((*itor).second.second);
		if (keyList->size() != 0)
		{
			key = keyList->front();
			key.peered = false;
			return true;
		}
	}
	key.keyData[0] = key.keyData[1] = key.keyData[2] = key.keyData[3] = 0;
	key.keyData[4] = key.keyData[5] = key.keyData[6] = key.keyData[7] = 0;
	key.peered = false;
	return false;
}


// return true if any left
bool Identity::InvalidateLoginKey(const IPSocket::Address& forAddr, const LoginKey& oldKey, LoginKey& newKey)
{
	bool result = false;
	LoginKeyMap::iterator itor = loginKeys.find(forAddr);

	if (itor != loginKeys.end())
	{
		LoginKeyList* keyList = &((*itor).second.second);

		if (!keyList->empty())
		{
			while (TRUE)
			{
				LoginKey copyKey = keyList->front();
				if ((memcmp(&newKey, &copyKey, 8) == 0) ||
					(memcmp(&oldKey, &copyKey, 8) == 0))
				{
					keyList->pop_front();
					if (keyList->empty())
						break;
				}
				else
				{
					newKey = copyKey;
					result = true;
					break;
				}
			}
		}
	}
	WriteLoginKeys();
	return result;
}


void Identity::AddLoginKey(const IPSocket::Address& forAddr, const LoginKey& key, bool validated)
{
	LoginKeyMap::iterator forAddrItor = loginKeys.find(forAddr);

	if (forAddrItor == loginKeys.end())
	{
		LoginKeyStruct keyStruct;
		in_addr addr = forAddr.GetAddress();
		keyStruct.authAddr = *(long*)(&addr);
		keyStruct.authPort = forAddr.GetPort();
		forAddrItor = loginKeys.insert(LoginKeyMap::value_type(forAddr, pair<LoginKeyStruct, LoginKeyList>(keyStruct, LoginKeyList()))).first;
	}
	LoginKeyList* keyList = &((*forAddrItor).second.second);
	
	if (validated)
		keyList->clear();
	keyList->push_front(key);

	if (!validated)	// else, it's already been added to the other lists
	{
		// Any valid login key needs to be added to the list of login keys for all other auth servers
		// to support auth server peering

		LoginKey peerKeyCopy = key;
		peerKeyCopy.peered = true;

		LoginKeyMap::iterator itor = loginKeys.begin();
		while (itor != loginKeys.end())
		{
			if (itor != forAddrItor)
			{
				keyList = &((*itor).second.second);

				// Need to add this peered key after the non-peered keys, but before other peered keys.
				// This way, the peered keys are tried after server-specific keys, and the newest peered key
				// is the first peered key tried.

				LoginKeyList::iterator keyItor = keyList->begin();
				while (keyItor != keyList->end())
				{
					if ((*keyItor).peered)
					{
						keyList->insert(keyItor, peerKeyCopy);

						// Max list out at 1000 login Keys (about 10K).  This is rediculously large, anyway
						if (keyList->size() > 1000)
							keyList->pop_back();

						// put before
						break;
					}
					keyItor++;
				}
			}
			itor++;
		}
	}

	WriteLoginKeys();
}


#ifdef WIN32


struct UserInfo
{
	wstring name;
	wstring pw;

	UserInfo(const wstring& _name, const wstring& _pw)
		:	name(_name), pw(_pw)
	{ }
};


static bool ReadAllUsersFromRegistry(const std::string& regPath, const char* productKey,
									 deque<UserInfo>& userInfo)
{
	// not yet implemented
	return false;

	// hash product name for keyName
	// use CD-Key if available, or constant blow-fish key to encrypt block

/*
	string keyName(productKey);
	keyName += "_cache";

	bool   aRet = false;
	RegKey aKey(regPath, HKEY_LOCAL_MACHINE);

	// If key is open, read verifier key and update AuthInfo
	if (aKey.IsOpen())
	{
		unsigned long  aKeyLen = 0;
		unsigned char* aKeyP   = NULL;
		if ((aKey.GetValue(keyName, aKeyP, aKeyLen) == RegKey::Ok) &&
			(aKeyLen > 0))
		{
			SetVerifierKey(aKeyP, aKeyLen);
			aRet = true;
		}
		delete aKeyP;
	}
	return aRet;
	*/

}


static bool WriteAllUsersFromRegistry(const std::string& regPath, const char* productKey,
									  deque<UserInfo>& userInfo)
{
	// not yet implemented
	return false;
}


// read all from productkey section into nameList
bool Identity::ReadUserNamesFromRegistry(const char* productKey, list<WONCommon::WONString>& nameList,
							   const std::string& regPath)
{
	nameList.clear();

	deque<UserInfo> userInfo;

	bool result = ReadAllUsersFromRegistry(regPath, productKey, userInfo);
	if (result)
	{
		deque<UserInfo>::iterator itor = userInfo.begin();
		while (itor != userInfo.end())
		{
			nameList.push_back((*itor).name);
			itor++;
		}
	}
	return result;
}


// load last saved name and password
bool Identity::LoadUserNameAndPasswordFromRegistry(const char* productKey, const std::string& regPath)
{
	deque<UserInfo> userInfo;

	bool result = ReadAllUsersFromRegistry(regPath, productKey, userInfo);
	if (result)
	{
		deque<UserInfo>::iterator itor = userInfo.begin();
		if (itor == userInfo.end())
			result = false;
		else
		{	// first entry is newest
			loginName = (*itor).name;
			password = (*itor).pw;
		}
	}
	return result;
}


// load password for name currently in identity object
bool Identity::LoadPasswordFromRegistry(const char* productKey, const std::string& regPath)
{
	deque<UserInfo> userInfo;

	bool result = ReadAllUsersFromRegistry(regPath, productKey, userInfo);
	if (result)
	{
		deque<UserInfo>::iterator itor = userInfo.begin();
		result = false;
		while (itor != userInfo.end())
		{
			if (loginName == (*itor).name)
			{
				password = (*itor).pw;
				result = true;
				break;
			}
			itor++;
		}
	}
	return result;
}


// save both username and password, move to head of list
// so it's found first when LoadUserNameAndPasswordFromRegistry is called next
bool Identity::SaveUserNameAndPasswordToRegistry(const char* productKey, const std::string& regPath)
{
	deque<UserInfo> userInfo;

	bool result = ReadAllUsersFromRegistry(regPath, productKey, userInfo);
	if (result)
	{
		deque<UserInfo>::iterator itor = userInfo.begin();
		result = false;
		while (itor != userInfo.end())
		{
			if (loginName == (*itor).name)
			{
				userInfo.erase(itor);
				userInfo.push_front(UserInfo(loginName, password));
				result = WriteAllUsersFromRegistry(regPath, productKey, userInfo);
				break;
			}
			itor++;
		}
	}
	return result;
}


// Same as SaveUserNameAndPasswordToRegistry(), except stores a blank password
bool Identity::SaveUserNameToRegistry(const char* productKey, const std::string& regPath)
{
	WONString tmpPassword = password;
	password.erase();
	bool result = SaveUserNameToRegistry(productKey, regPath);
	password = tmpPassword;
	return result;
}

#endif


// C API routines
#include "wondll.h"

BOOL WONAuthLoadVerifierKeyFromFileA(WON_CONST char* theFile)
{
	return Identity::LoadVerifierKeyFromFile(theFile) ? 1 : 0;
}


BOOL WONAuthLoadVerifierKeyFromFileW(WON_CONST wchar_t* theFile)
{
	return Identity::LoadVerifierKeyFromFile(theFile) ? 1 : 0;
}


BOOL WONAuthWriteVerifierKeyToFileA(WON_CONST char* theFile)
{
	return Identity::WriteVerifierKeyToFile(theFile) ? 1 : 0;
}


BOOL WONAuthWriteVerifierKeyToFileW(WON_CONST wchar_t* theFile)
{
	return Identity::WriteVerifierKeyToFile(theFile) ? 1 : 0;
}


#ifdef WIN32
BOOL WONAuthLoadVerifierKeyFromRegistry(WON_CONST char* regPath, WON_CONST char* regKey)
{
	return Identity::LoadVerifierKeyFromRegistry(regPath, regKey) ? 1 : 0;
}


BOOL WONAuthWriteVerifierKeyToRegistry(WON_CONST char* regPath, WON_CONST char* regKey)
{
	return Identity::WriteVerifierKeyToRegistry(regPath, regKey) ? 1 : 0;
}
#endif


void WONAuthSetVerifierKey(void* verifKey, unsigned long verifKeyLength)
{
	Identity::SetVerifierKey(verifKey, verifKeyLength);
}


static HWONAUTH WONAuthLoginWLL(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								WON_CONST wstring& name, WON_CONST wstring& community,
								WON_CONST wstring& password, WON_CONST wstring& nicknameKey,
								long timeout, bool createAccount)
{
	Error err = Error_InvalidParams;
	Identity* ident = 0;
	
	if (numServers)
	{
		err = Error_OutOfMemory;

		// build server list
		IPSocket::Address* addrs = new IPSocket::Address[numServers];
		if (addrs)
		{
			array_auto_ptr<IPSocket::Address> autoDelAddrs(addrs);
			unsigned short curServer = 0;
			do {
				addrs[curServer].Set(authServers[curServer]);
			} while (++curServer < numServers);

			ident = new Identity(name, community, password, nicknameKey, addrs, numServers);
			if (ident)
			{
				err = ident->Authenticate(createAccount, false, timeout);
				if (err != Error_Success)
				{
					delete ident;
					ident = 0;
				}
			}
		}
	}
	if (errorCode)
		*errorCode = err;

	return (HWONAUTH)ident;
}


HWONAUTH WONAuthLoginA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
					   WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
					   WON_CONST char* nicknameKey, long timeout)
{
	return WONAuthLoginWLL(errorCode, authServers, numServers, StringToWString(name), StringToWString(community),
		StringToWString(password), StringToWString(nicknameKey), timeout, false);
}


HWONAUTH WONAuthLoginW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
					   WON_CONST wchar_t* name, WON_CONST wchar_t* community,
					   WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey, long timeout)
{
	return WONAuthLoginWLL(errorCode, authServers, numServers, wstring(name), wstring(community),
		wstring(password), wstring(nicknameKey), timeout, false);
}


HWONAUTH WONAuthLoginNewAccountA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								 WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
								 WON_CONST char* nicknameKey, long timeout)
{
	return WONAuthLoginWLL(errorCode, authServers, numServers, StringToWString(name), StringToWString(community),
		StringToWString(password), StringToWString(nicknameKey), timeout, true);
}


HWONAUTH WONAuthLoginNewAccountW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								 WON_CONST wchar_t* name, WON_CONST wchar_t* community,
								 WON_CONST wchar_t* password, WON_CONST wchar_t* nicknameKey,
								 long timeout)
{
	return WONAuthLoginWLL(errorCode, authServers, numServers, wstring(name), wstring(community),
		wstring(password), wstring(nicknameKey), timeout, true);
}


void WONAuthCloseHandle(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	delete ident;
}


WONError WONAuthSetNicknameW(HWONAUTH hWONAuth, WON_CONST wchar_t* nicknameKey,
							 WON_CONST wchar_t* newNickname, long timeout)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		Identity identCopy(*ident);
		identCopy.SetNickname(newNickname);
		identCopy.SetNicknameKey(nicknameKey);
		err = identCopy.AuthenticateNewNickname(timeout);
		if (err == Error_Success)
			*ident = identCopy;
	}
	return err;
}


WONError WONAuthSetNicknameA(HWONAUTH hWONAuth, WON_CONST char* nicknameKey, WON_CONST char* newNickname,
							 long timeout)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		Identity identCopy(*ident);
		identCopy.SetNickname(newNickname);
		identCopy.SetNicknameKey(nicknameKey);
		err = identCopy.AuthenticateNewNickname(timeout);
		if (err == Error_Success)
			*ident = identCopy;
	}
	return err;
}


WONError WONAuthChangePasswordW(HWONAUTH hWONAuth, WON_CONST wchar_t* newPassword, long timeout)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		Identity identCopy(*ident);
		err = identCopy.AuthenticateNewPassword(newPassword, timeout);
		if (err == Error_Success)
			*ident = identCopy;
	}
	return err;
}


WONError WONAuthChangePasswordA(HWONAUTH hWONAuth, WON_CONST char* newPassword, long timeout)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		Identity identCopy(*ident);
		err = identCopy.AuthenticateNewPassword(newPassword, timeout);
		if (err == Error_Success)
			*ident = identCopy;
	}
	return err;
}


BOOL WONAuthHasExpired(HWONAUTH hWONAuth)
{
	return time(NULL) >= WONAuthGetExpiration(hWONAuth) ? TRUE : FALSE;
}


time_t WONAuthGetExpiration(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return ident ? ident->GetExpiration() : 0;
}


WONError WONAuthRefresh(HWONAUTH hWONAuth, BOOL forceRefresh, long timeout)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		Identity identCopy(*ident);
		err = identCopy.Refresh(forceRefresh == TRUE ? true : false, timeout);
		if (err == Error_Success)
			*ident = identCopy;
	}
	return err;
}


WON_CONST char*		WONAuthGetNameA(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST char*)ident->GetLoginNameA();
}


WON_CONST wchar_t*	WONAuthGetNameW(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST wchar_t*)ident->GetLoginNameW();
}

WON_CONST char*		WONAuthGetCommunityA(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST char*)ident->GetCommunityNameA();
}


WON_CONST wchar_t*	WONAuthGetCommunityW(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST wchar_t*)ident->GetCommunityNameW();
}


WON_CONST char*		WONAuthGetPasswordA(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST char*)ident->GetPasswordA();
}


WON_CONST wchar_t*	WONAuthGetPasswordW(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST wchar_t*)ident->GetPasswordW();
}


WON_CONST char*		WONAuthGetNicknameA(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST char*)ident->GetNicknameA();
}


WON_CONST wchar_t*	WONAuthGetNicknameW(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST wchar_t*)ident->GetNicknameW();
}


WON_CONST char*		WONAuthGetNicknameKeyA(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST char*)ident->GetNicknameKeyA();
}


WON_CONST wchar_t*	WONAuthGetNicknameKeyW(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return (WON_CONST wchar_t*)ident->GetNicknameKeyW();
}


BOOL WONAuthIsNicknameDefault(HWONAUTH hWONAuth)
{
	Identity* ident = (Identity*)hWONAuth;
	return ident ? (ident->IsNicknameDefault() ? TRUE : FALSE) : FALSE;
}


unsigned long WONAuthGetUserID(HWONAUTH hWONAuth)
{
	unsigned long userID = 0;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
		return ident->GetUserId();
	return userID;
}


unsigned long WONAuthGetCommunityID(HWONAUTH hWONAuth)
{
	unsigned long userID = 0;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
		return ident->GetCommunityId();
	return userID;
}


unsigned short WONAuthGetTrustLevel(HWONAUTH hWONAuth)
{
	unsigned long userID = 0;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
		return ident->GetTrustLevel();
	return userID;
}


void WONAuthGetCertificate(HWONAUTH hWONAuth, void* certificateBuf, unsigned short* bufSize)
{
	Identity* ident = (Identity*)hWONAuth;
	ident->GetCertificate(certificateBuf, bufSize);
}


void WONAuthGetPrivateKey(HWONAUTH hWONAuth, void* privateKeyBuf, unsigned short* bufSize)
{
	Identity* ident = (Identity*)hWONAuth;
	ident->GetPrivateKey(privateKeyBuf, bufSize);
}


void WONAuthGetPublicKey(HWONAUTH hWONAuth, void* publicKeyBuf, unsigned short* bufSize)
{
	Identity* ident = (Identity*)hWONAuth;
	ident->GetPublicKey(publicKeyBuf, bufSize);
}


void WONAuthGetPublicKeyBlock(void* publicKeyBlockBuf, unsigned short* bufSize)
{
	Identity::GetPublicKeyBlock(publicKeyBlockBuf, bufSize);
}


void WONAuthGetVerifierKey(void* verifierKeyBuf, unsigned short* bufSize)
{
	Identity::GetVerifierKey(verifierKeyBuf, bufSize);
}


class WONAuthLoginAsyncWLLData
{
public:
	HWONCOMPLETION completion;
	WONError* errorCode;
	Identity* oldIdent;
};


static void WONAuthLoginAsyncWLLComplete(const Identity::AuthResult& result, WONAuthLoginAsyncWLLData* data)
{
	if (data->errorCode)
		*(data->errorCode) = result.error;

	if (result.error != Error_Success)
	{
		delete result.identity;
		WONComplete(data->completion, 0);
	}
	else
		WONComplete(data->completion, (HWONAUTH)result.identity);
	delete data;
}


static void WONAuthLoginAsyncWLL(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								 WON_CONST wstring& name, WON_CONST wstring& community, WON_CONST wstring& password,
								 WON_CONST wstring& nicknameKey, long timeout, HWONCOMPLETION completion, bool createAccount)
{
	Error err = Error_InvalidParams;
	Identity* ident = 0;
	
	if (numServers)
	{
		err = Error_OutOfMemory;

		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			auto_ptr<WONAuthLoginAsyncWLLData> autoDelData;
			data->completion = completion;
			data->errorCode = errorCode;

			// build server list
			IPSocket::Address* addrs = new IPSocket::Address[numServers];
			if (addrs)
			{
				array_auto_ptr<IPSocket::Address> autoDelAddrs(addrs);
				unsigned short curServer = 0;
				do {
					addrs[curServer].Set(authServers[curServer]);
				} while (++curServer < numServers);

				ident = new Identity(name, community, password, nicknameKey, addrs, numServers);
				if (ident)
				{
					autoDelData.release();
					ident->AuthenticateEx(createAccount, false, timeout, true, WONAuthLoginAsyncWLLComplete, data);
					return;
				}
			}
		}
	}
	if (errorCode)
		*errorCode = err;
	
	WONComplete(completion, 0);
}


void WONAuthLoginAsyncA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
						WON_CONST char* name, WON_CONST char* community, WON_CONST char* password, WON_CONST char* nicknameKey,
						long timeout, HWONCOMPLETION hCompletion)
{
	WONAuthLoginAsyncWLL(errorCode, authServers, numServers, StringToWString(name),
		StringToWString(community), StringToWString(password), StringToWString(nicknameKey),
		timeout, hCompletion, false);
}


void WONAuthLoginAsyncW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
						WON_CONST wchar_t* name, WON_CONST wchar_t* community, WON_CONST wchar_t* password,
						WON_CONST wchar_t* nicknameKey, long timeout, HWONCOMPLETION hCompletion)
{
	WONAuthLoginAsyncWLL(errorCode, authServers, numServers, wstring(name), wstring(community),
		wstring(password), wstring(nicknameKey), timeout, hCompletion, false);
}


void WONAuthLoginNewAccountAsyncA(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								  WON_CONST char* name, WON_CONST char* community, WON_CONST char* password,
								  WON_CONST char* nicknameKey, long timeout, HWONCOMPLETION hCompletion)
{
	WONAuthLoginAsyncWLL(errorCode, authServers, numServers, StringToWString(name),
		StringToWString(community), StringToWString(password), StringToWString(nicknameKey),
		timeout, hCompletion, true);
}


void WONAuthLoginNewAccountAsyncW(WONError* errorCode, WON_CONST WONIPAddress* authServers, unsigned short numServers,
								  WON_CONST wchar_t* name, WON_CONST wchar_t* community, WON_CONST wchar_t* password,
								  WON_CONST wchar_t* nicknameKey, long timeout, HWONCOMPLETION hCompletion)
{
	WONAuthLoginAsyncWLL(errorCode, authServers, numServers, wstring(name), wstring(community),
		wstring(password), wstring(nicknameKey), timeout, hCompletion, true);
}

static void ChangePassSetNickRefreshCompletion(const Identity::AuthResult& result, WONAuthLoginAsyncWLLData* data)
{
	if (result.error == Error_Success)
		*(data->oldIdent) = *(result.identity);
	WONComplete(data->completion, (void*)result.error);
	delete data;
}


void WONAuthChangePasswordAsyncA(HWONAUTH hWONAuth, WON_CONST char* newPassword, long timeout, HWONCOMPLETION hCompletion)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		err = Error_OutOfMemory;
		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			data->completion = hCompletion;
			data->oldIdent = ident;
			
			Identity identCopy(*ident);
			identCopy.AuthenticateNewPasswordEx(newPassword, timeout, true, ChangePassSetNickRefreshCompletion, data);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONAuthChangePasswordAsyncW(HWONAUTH hWONAuth, WON_CONST wchar_t* newPassword, long timeout, HWONCOMPLETION hCompletion)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		err = Error_OutOfMemory;
		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			data->completion = hCompletion;
			data->oldIdent = ident;
			
			Identity identCopy(*ident);
			identCopy.AuthenticateNewPasswordEx(newPassword, timeout, true, ChangePassSetNickRefreshCompletion, data);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONAuthSetNicknameAsyncA(HWONAUTH hWONAuth, WON_CONST char* nicknameKey, WON_CONST char* newNickname,
							  long timeout, HWONCOMPLETION hCompletion)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		err = Error_OutOfMemory;
		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			data->completion = hCompletion;
			data->oldIdent = ident;
			
			Identity identCopy(*ident);
			identCopy.SetNickname(newNickname);
			identCopy.SetNicknameKey(nicknameKey);
			identCopy.AuthenticateNewNicknameEx(timeout, true, ChangePassSetNickRefreshCompletion, data);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONAuthSetNicknameAsyncW(HWONAUTH hWONAuth, WON_CONST wchar_t* nicknameKey, WON_CONST wchar_t* newNickname,
							  long timeout, HWONCOMPLETION hCompletion)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		err = Error_OutOfMemory;
		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			data->completion = hCompletion;
			data->oldIdent = ident;
			
			Identity identCopy(*ident);
			identCopy.SetNickname(newNickname);
			identCopy.SetNicknameKey(nicknameKey);
			identCopy.AuthenticateNewNicknameEx(timeout, true, ChangePassSetNickRefreshCompletion, data);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONAuthRefreshAsync(HWONAUTH hWONAuth, BOOL forceRefresh, long timeout, HWONCOMPLETION hCompletion)
{
	WONError err = Error_InvalidParams;
	Identity* ident = (Identity*)hWONAuth;
	if (ident)
	{
		err = Error_OutOfMemory;
		WONAuthLoginAsyncWLLData* data = new WONAuthLoginAsyncWLLData;
		if (data)
		{
			data->completion = hCompletion;
			data->oldIdent = ident;

			Identity identCopy(*ident);
			identCopy.RefreshEx(forceRefresh == TRUE ? true : false, timeout, true, ChangePassSetNickRefreshCompletion, data);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}


void WONAuthSetCDKey(WON_CONST char* productName, void* cdKeyBuf, unsigned short cdKeySize, BOOL storeInHKeyCurUser)
{
	WONCDKey::ClientCDKey cdKey(productName,storeInHKeyCurUser?true:false);
	cdKey.Init(WONCommon::RawBuffer((unsigned char*)cdKeyBuf, cdKeySize));

	Identity::SetCDKey(cdKey);
}

void WONAuthSetCDKeyString(WON_CONST char* productName, WON_CONST char* cdKeyString, BOOL storeInHKeyCurUser)
{
	WONCDKey::ClientCDKey cdKey(productName, storeInHKeyCurUser?true:false);
	cdKey.Init(cdKeyString);

	Identity::SetCDKey(cdKey);
}


BOOL WONAuthHasCDKey()
{
	return Identity::hasCDKey();
}


void WONAuthGetCDKey(void* cdKeyBuf, unsigned short* bufSize)
{
	if (bufSize)
	{
		unsigned short maxSize = *bufSize;
		*bufSize = 0;
		if (maxSize >= sizeof(__int64))
		{
			WONCDKey::ClientCDKey* cdKey = Identity::GetCDKey();
			if (cdKey)
			{
				__int64 keyVal = cdKey->AsRaw();
				memcpy(cdKeyBuf, &keyVal, sizeof(__int64));
				*bufSize = sizeof(__int64);
				delete cdKey;
			}
		}
	}
}


void WONAuthGetCDKeyAsString(char* cdKeyStrBuf, unsigned short* strBufLength)
{
	if (strBufLength)
	{
		unsigned short maxSize = *strBufLength;
		*strBufLength = 0;
		if (cdKeyStrBuf && maxSize > 1)
		{
			WONCDKey::ClientCDKey* cdKey = Identity::GetCDKey();
			if (cdKey)
			{
				string keyStr = cdKey->AsString();
				cdKeyStrBuf[maxSize - 1] = 0;
				unsigned short copySize = keyStr.size() > maxSize-2 ? maxSize-2 : keyStr.size();
				strncpy(cdKeyStrBuf, keyStr.c_str(), copySize + 1);
				*strBufLength = copySize;
				delete cdKey;
			}
		}
	}
}


void WONAuthSetLoginKeyFileA(WON_CONST char* theFile)
{
	Identity::SetLoginKeyFile(theFile);
}


void WONAuthSetLoginKeyFileW(WON_CONST wchar_t* theFile)
{
	Identity::SetLoginKeyFile(theFile);
}


void WONAuthSetHashFileA(WON_CONST char* theFile)
{
	Identity::SetHashFile(theFile);
}

void WONAuthSetHashFileW(WON_CONST wchar_t* theFile)
{
	Identity::SetHashFile(theFile);
}


BOOL WONAuthWillHash()
{
	return Identity::willHash() ? TRUE : FALSE;
}


void WONAuthSetWillHash(BOOL doHash)
{
	Identity::SetWillHash(doHash ? true : false);
}


BOOL WONAuthIsCDKeyValid()
{
	return Identity::IsCDKeyValid() ? TRUE : FALSE;
}

#ifdef WIN32
BOOL WONAuthLoadCDKey(WON_CONST char* productName, BOOL fromHKeyCurUser)
{
	return Identity::LoadCDKey(productName, fromHKeyCurUser?true:false) ? TRUE : FALSE;
}


BOOL WONAuthSaveCDKey()
{
	return Identity::SaveCDKey() ? TRUE : FALSE;
}
#endif

