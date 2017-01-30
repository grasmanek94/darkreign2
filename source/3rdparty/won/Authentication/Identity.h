
#ifndef __WON_AUTHENTICATION_IDENTITY_H__
#define __WON_AUTHENTICATION_IDENTITY_H__


#pragma warning(disable : 4786)


#include <string>
#include <queue>
#include <list>
#include <map>
#include <time.h>
#include <wchar.h>
#include "Socket/TCPSocket.h"
#include "Socket/TMsgSocket.h"
#include "SDKCommon/Completion.h"
#include "common/Event.h"
#include "Errors.h"
#include "auth/Auth1PublicKeyBlock.h"
#include "auth/Auth1Certificate.h"
#include "crypt/BFSymmetricKey.h"
#include "crypt/EGPrivateKey.h"
#include "wondll.h"
#include "ClientCDKey.h"
#include "common/WONString.h"

#ifdef WIN32
#include "common/RegKey.h"
#endif

namespace WONAPI {


class Identity
{
public:

	Identity();
	Identity(const IPSocket::Address* addrs, unsigned int numAddrs);
	explicit Identity(const IPSocket::Address& addr);

	Identity(	const WONCommon::WONString& name, const WONCommon::WONString& community,
				const WONCommon::WONString& pw, const WONCommon::WONString& nickKey,
				const IPSocket::Address& addr);

	Identity(	const WONCommon::WONString& name, const WONCommon::WONString& community,
				const WONCommon::WONString& pw, const WONCommon::WONString& nickKey,
				const IPSocket::Address* addrs, unsigned int numAddrs);

	Identity(	const WONCommon::WONString& name, const WONCommon::WONString& community,
				const WONCommon::WONString& pw, const WONCommon::WONString& nick,
				const WONCommon::WONString& nickKey, const IPSocket::Address& addr);

	Identity(	const WONCommon::WONString& name, const WONCommon::WONString& community,
				const WONCommon::WONString& pw, const WONCommon::WONString& nick,
				const WONCommon::WONString& nickKey, const IPSocket::Address* addrs, unsigned int numAddrs);

	Identity(	const Identity& toCopy);

	Identity& operator=(const Identity& toCopy);

	~Identity();
	
	bool IsNicknameDefault()							{ return isNicknameDefault; }

	void GetLoginName(WONCommon::WONString& s) const	{ s = loginName; }
	void GetCommunityName(WONCommon::WONString& s) const{ s = communityName; }
	void GetPassword(WONCommon::WONString& s) const		{ s = password; }
	void GetNickname(WONCommon::WONString& s) const		{ s = nickname; }
	void GetNicknameKey(WONCommon::WONString& s) const	{ s = nicknameKey; }

	void GetLoginName(std::wstring& s) const		{ s = (const std::wstring&)loginName; }
	void GetCommunityName(std::wstring& s) const	{ s = (const std::wstring&)communityName; }
	void GetPassword(std::wstring& s) const			{ s = (const std::wstring&)password; }
	void GetNickname(std::wstring& s) const			{ s = (const std::wstring&)nickname; }
	void GetNicknameKey(std::wstring& s) const		{ s = (const std::wstring&)nicknameKey; }

	void GetLoginName(std::string& s) const			{ s = (const std::string&)loginName; }
	void GetCommunityName(std::string& s) const		{ s = (const std::string&)communityName; }
	void GetPassword(std::string& s) const			{ s = (const std::string&)password; }
	void GetNickname(std::string& s) const			{ s = (const std::string&)nickname; }
	void GetNicknameKey(std::string& s) const		{ s = (const std::string&)nicknameKey; }

	void GetLoginName(char* s) const			{ strcpy(s, loginName); }
	void GetCommunityName(char* s) const			{ strcpy(s, communityName); }
	void GetPassword(char* s) const				{ strcpy(s, password); }
	void GetNickname(char* s) const				{ strcpy(s, nickname); }
	void GetNicknameKey(char* s) const			{ strcpy(s, nicknameKey); }

	void GetLoginName(wchar_t* s) const			{ wcscpy(s, loginName); }
	void GetCommunityName(wchar_t* s) const			{ wcscpy(s, communityName); }
	void GetPassword(wchar_t* s) const			{ wcscpy(s, password); }
	void GetNickname(wchar_t* s) const			{ wcscpy(s, nickname); }
	void GetNicknameKey(wchar_t* s) const			{ wcscpy(s, nicknameKey); }

	const wchar_t* GetLoginNameW() const			{ return loginName; }
	const wchar_t* GetCommunityNameW() const		{ return communityName; }
	const wchar_t* GetPasswordW() const			{ return password; }
	const wchar_t* GetNicknameW() const			{ return nickname; }
	const wchar_t* GetNicknameKeyW() const			{ return nicknameKey; }
	
	const char* GetLoginNameA() const			{ return loginName; }
	const char* GetCommunityNameA() const			{ return communityName; }
	const char* GetPasswordA() const			{ return password; }
	const char* GetNicknameA() const			{ return nickname; }
	const char* GetNicknameKeyA() const			{ return nicknameKey; }

	const WONCommon::WONString& GetLoginName() const	{ return loginName; }
	const WONCommon::WONString& GetCommunityName() const	{ return communityName; }
	const WONCommon::WONString& GetPassword() const		{ return password; }
	const WONCommon::WONString& GetNickname() const		{ return nickname; }
	const WONCommon::WONString& GetNicknameKey() const	{ return nicknameKey; }

	void SetLoginName(const WONCommon::WONString& s)	{ Invalidate(); loginName = s; }
	void SetCommunityName(const WONCommon::WONString& s){ Invalidate(); communityName = s; }
	void SetPassword(const WONCommon::WONString& s)		{ Invalidate(); password = s; }
	void SetNickname(const WONCommon::WONString& s)		{ Invalidate(); nickname = s; }
	void SetNicknameKey(const WONCommon::WONString& s)	{ Invalidate(); nicknameKey = s; }

	void SetServer(const IPSocket::Address& addr)	{ authAddrs.clear(); authAddrs.push_back(addr); }
	void SetServers(const IPSocket::Address* addrs, unsigned int numAddrs);

	static bool LoadVerifierKeyFromFile(const WONCommon::WONString& theFile);
	static bool WriteVerifierKeyToFile(const WONCommon::WONString& theFile);

#ifdef WIN32
	// should eventually add file-based versions of these 
	bool ReadUserNamesFromRegistry(const char* productKey, std::list<WONCommon::WONString>& nameList,
								   const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME);

	bool LoadUserNameAndPasswordFromRegistry(const char* productKey,
											 const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME);

	bool LoadPasswordFromRegistry(const char* productKey,	// name must be set manually first
								  const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME);
	
	bool SaveUserNameAndPasswordToRegistry(const char* productKey,
										   const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME);

	bool SaveUserNameToRegistry(const char* productKey,
								const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME);


	static bool LoadVerifierKeyFromRegistry(const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME,
											const std::string& keyName = REG_CONST::REG_AUTHVERIFIER_NAME);

	static bool WriteVerifierKeyToRegistry(const std::string& regPath = REG_CONST::REG_TITAN_KEY_NAME,
										   const std::string& keyName = REG_CONST::REG_AUTHVERIFIER_NAME);
#endif

	static void SetVerifierKey(const WONCrypt::EGPublicKey& verifKey);
	static void SetVerifierKey(void* verifKey, unsigned long verifKeyLength);

	bool IsAuthenticated()						{ return certificate != 0; }
	void Invalidate();

	void SetAutoReauthenticate(bool autoAuth)	{ autoReauth = autoAuth; }
	bool GetAutoReauthenticate(void)			{ return autoReauth; }

	class AuthResult
	{
	public:
		Error error;
		Identity* identity;

		AuthResult(Identity* ident, Error err)
			:	identity(ident),
				error(err)
		{ }
	};


	Error Authenticate(bool newAcct = false, bool newNick = false, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION )
	{ return Authenticate(newAcct, newNick, std::wstring(), timeout, async, completion); }

	template <class privsType>
	Error AuthenticateEx(bool newAcct, bool newNick, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return Authenticate(newAcct, newNick, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(t, f, true)); }


	Error AuthenticateNewAccount(long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION )
	{ return Authenticate(true, false, timeout, async, completion); }

	template <class privsType>
	Error AuthenticateNewAccountEx(long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return AuthenticateNewAccount(timeout, async, new CompletionWithContents<const AuthResult&, privsType>(t, f, true)); }


	Error AuthenticateNewNickname(long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION )
	{ return Authenticate(false, true, timeout, async, completion); }

	template <class privsType>
	Error AuthenticateNewNicknameEx(long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return AuthenticateNewNickname(timeout, async, new CompletionWithContents<const AuthResult&, privsType>(t, f, true)); }


	Error AuthenticateNewPassword(const WONCommon::WONString& newPassword, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	Error AuthenticateNewPasswordEx(const WONCommon::WONString& newPassword, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return AuthenticateNewPassword(newPassword, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(t, f, true)); }


	Error Refresh(bool force = false, long timeout = -1, bool async = false, const CompletionContainer<const AuthResult&>& completion = DEFAULT_COMPLETION );

	template <class privsType>
	Error RefreshEx(bool force, long timeout, bool async, void (*f)(const AuthResult&, privsType), privsType t)
	{ return Refresh(force, timeout, async, new CompletionWithContents<const AuthResult&, privsType>(t, f, true)); }


	// These return NEW objects, due to race condition issues.  Don't forget to delete them
	WONAuth::Auth1Certificate* GetCertificate() const;
	void GetCertificate(void* certBuf, unsigned short* certBufSize) const;

	WONCrypt::EGPrivateKey* GetPrivateKey() const;
	void GetPrivateKey(void* keyBuf, unsigned short* keyBufSize) const;

	WONCrypt::EGPublicKey* GetPublicKey() const;
	void GetPublicKey(void* keyBuf, unsigned short* keyBufSize) const;

	static WONAuth::Auth1PublicKeyBlock* GetPublicKeyBlock();
	static void GetPublicKeyBlock(void* keyBlockBuf, unsigned short* keyBlockBufSize);

	static WONCrypt::EGPublicKey* GetVerifierKey();
	static void GetVerifierKey(void* keyBuf, unsigned short* keyBufSize);

	static WONCDKey::ClientCDKey* GetCDKey();
	static void GetCDKey(void* keyBug, unsigned short keyBufSize);
	
	static bool hasCDKey()										{ return CDKey != 0; }
	static void SetCDKey(const WONCDKey::ClientCDKey& newCDKey);

	static bool IsCDKeyValid();

#ifdef WIN32
	static bool SaveCDKey();
	static bool LoadCDKey(const string& productName, bool fromHKeyCurUser = false);
#endif

	time_t GetExpiration() const								{ return certificateExpiration; }
	bool IsExpired() const;

	unsigned long  GetUserId() const;
	unsigned long  GetCommunityId() const;
	unsigned short GetTrustLevel() const;

	static void SetLoginKeyFile(const WONCommon::WONString& fileName)	{ loginKeys.clear(); loginKeyFile = fileName; }

	time_t GetExpirationDelta() const									{ return expirationDelta; }

	static bool willHash()												{ return doMD5Hash; }
	static void SetWillHash(bool doHash)								{ doMD5Hash = doHash; }
	static void SetHashFile(const WONCommon::WONString& fileName)		{ hashFile = fileName; }
	static const WONCommon::WONString& GetHashFile()					{ return hashFile; }

protected:
	WONCommon::WONString loginName;
	WONCommon::WONString communityName;
	WONCommon::WONString password;
	WONCommon::WONString nickname;
	WONCommon::WONString nicknameKey;
	bool isNicknameDefault;
	bool autoReauth;

	static WONCommon::WONString hashFile;
	static bool doMD5Hash;

private:
	WONAuth::Auth1Certificate* certificate;
	WONCrypt::EGPrivateKey* privateKey;
	static WONCrypt::EGPublicKey verifierKey;
	static WONAuth::Auth1PublicKeyBlock* pubKeys;
	static WONCDKey::ClientCDKey* CDKey;


	struct LoginKey 
	{
		char keyData[8];
		bool peered;
	};

	struct LoginKeyStruct
	{
		unsigned long authAddr;
		unsigned short authPort;
		unsigned short numLoginKeys;
	};


	typedef std::list<LoginKey>													LoginKeyList;
	typedef std::map<IPSocket::Address, std::pair<LoginKeyStruct, LoginKeyList> >	LoginKeyMap;

	static WONCommon::WONString loginKeyFile;
	static LoginKeyMap loginKeys;

	static bool InvalidateLoginKey(const IPSocket::Address& forAddr, const LoginKey& oldKey, LoginKey& newKey);
	static bool ReadLoginKey(const IPSocket::Address& forAddr, LoginKey& key);
	static void AddLoginKey(const IPSocket::Address& forAddr, const LoginKey& key, bool validated);
	static void WriteLoginKeys();

	WONCommon::CriticalSection refreshCrit;
	bool refreshing;

	typedef std::queue<CompletionContainer<const AuthResult&> > RefreshQueue;
	RefreshQueue refreshQueue;

	std::list<std::wstring> errStrings;
	std::list<IPSocket::Address> authAddrs;

	time_t certificateExpiration;
	time_t expirationDelta;

	// Private, since option combination doesn't make sense for an interface
	Error Authenticate(bool newAcct, bool newNick, const WONCommon::WONString& newPassword, long timeout, bool async, const CompletionContainer<const AuthResult&>& completion);

	// Block of state-data for async authentication request
	class AuthenticationData;
	friend class AuthenticationData;
//	void FinishQueued(Error err);

	static void LoginCompletion(const TMsgSocket::RecvRawMsgResult& result, AuthenticationData* authData);
	static void OpenCompletion(const Socket::OpenResult& result, AuthenticationData* authData);
	static void PubKeysCompletion(const TMsgSocket::RecvRawMsgResult& result, AuthenticationData* authData);
	static void PubKeysOpenCompletion(const Socket::OpenResult& result, AuthenticationData* authData);
	Error DoCompletion(Error err, const CompletionContainer<const AuthResult&>& completion);
	static Error SetPubKeyBlock(const unsigned char* buf, unsigned short size);
};


};


#endif
