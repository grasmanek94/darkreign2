#ifdef UNDELETE
#ifndef _TMsgAuth1Login_H
#define _TMsgAuth1Login_H

// TMsgAuth1Login.h

// AuthServer messages implementing the Auth1 Family login protocol.
// This header/source implements messages:
//	Auth1LoginRequest
//	Auth1NewLoginRequest
// It also includes the Auth1LoginReply which is implemented in its own header
// and source module (it is shared).

// Auth1LoginRequest begins login for an existing user and is sent from the Client to
// the AuthServer.

// Auth1NewLoginRequest begins login for a new user and is sent from the Client to
// the AuthServer.

// Auth1LoginReply is sent in response to both Auth1LoginRequest and
// Auth1NewLoginRequest from the AuthServer to the client and completes the login.


#include <string>
#include "TMsgAuthRawBufferBase.h"

// Forwards from WONSocket
namespace WONMsg {


// TMsgAuth1LoginBase - Base class encapsulating common fields between the Login
// and NewLogin messages.
// Note that this class derives from TMsgAuthRawBufferBase, which implies classes
// derived from this one will include a raw buffer for their use.
class TMsgAuth1LoginBase : public TMsgAuthRawBufferBase
{
public:
	// Default ctor
	TMsgAuth1LoginBase();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1LoginBase(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1LoginBase(const TMsgAuth1LoginBase& theMsgR);

	// Destructor
	virtual ~TMsgAuth1LoginBase();

	// Assignment
	TMsgAuth1LoginBase& operator=(const TMsgAuth1LoginBase& theMsgR);

	// Virtual Duplicate from TMessage
	virtual TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	virtual void* Pack(); 
	virtual void  Unpack();

	// KeyBlock ID access
	unsigned short GetKeyBlockId() const;
	void           SetKeyBlockId(unsigned short theId);

	// NeedKey flag access
	bool GetNeedKey() const;
	void SetNeedKey(bool theFlag);

	// Login name access
	const std::wstring& GetLoginName() const;
	void SetLoginName(const std::wstring& theName);

	// Login name access
	const std::wstring& GetCommunityName() const;
	void SetCommunityName(const std::wstring& theName);

	// Login name access
	const std::wstring& GetNicknameKey() const;
	void SetNicknameKey(const std::wstring& theKey);

private:
	unsigned short mKeyBlockId;     // Id of AuthServ pub key block used to encrypt
	bool           mNeedKey;        // Flag, client requesting to pub/priv key
	std::wstring   mLoginName;      // User login name
	std::wstring   mCommunityName;  // User community name
	std::wstring   mNicknameKey;    // Key for nickname lookup.
};


// TMsgAuth1LoginRequest - begin login for existing user
//   Contains encrypted login data which is expected to contain:
//	   Key Block ID     (2 bytes)  Must match key block Id in message
//     Session Key len  (2 bytes)
//     Session Key      (var bytes == Session Key Len)
//     Password len     (2 bytes)
//     Password         (var bytes == Password Len)
//     NewPassword len  (2 Bytes)
//     NewPassword      (var bytes == newPassword Len) (May be 0)
//   Login data is encrypted with the public key of the AuthServer and stored in the
//   raw buffer from the TMsgAuthRawBufferBase class.
class TMsgAuth1LoginRequest : public TMsgAuth1LoginBase
{
public:
	// Default ctor
	TMsgAuth1LoginRequest();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1LoginRequest(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1LoginRequest(const TMsgAuth1LoginRequest& theMsgR);

	// Destructor
	~TMsgAuth1LoginRequest();

	// Assignment
	TMsgAuth1LoginRequest& operator=(const TMsgAuth1LoginRequest& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

private:
};


// TMsgAuth1NewLoginRequest - begin login for new user
//   Contains encrypted login data which is expected to contain:
//	   Key Block ID     (2 bytes)  Must match key block Id in message
//     Session Key len  (2 bytes)
//     Session Key      (var bytes == Session Key Len)
//     Password len     (2 bytes)
//     Password         (var bytes == Password Len)
//   Login data is encrypted with the public key of the AuthServer and stored in the
//   raw buffer from the TMsgAuthRawBufferBase class.
class TMsgAuth1NewLoginRequest : public TMsgAuth1LoginBase
{
public:
	// Default ctor
	TMsgAuth1NewLoginRequest();

	// TMessage ctor - will throw if TMessage type is not of this type
	explicit TMsgAuth1NewLoginRequest(const TMessage& theMsgR);

	// Copy ctor
	TMsgAuth1NewLoginRequest(const TMsgAuth1NewLoginRequest& theMsgR);

	// Destructor
	~TMsgAuth1NewLoginRequest();

	// Assignment
	TMsgAuth1NewLoginRequest& operator=(const TMsgAuth1NewLoginRequest& theMsgR);

	// Virtual Duplicate from TMessage
	TRawMsg* Duplicate() const;

	// Pack and Unpack the message
	// Unpack will throw a BadMsgException is message is not of this type
	void* Pack(); 
	void  Unpack();

	// Nickname access
	const std::wstring& GetNickname() const;
	void SetNickname(const std::wstring& theName);

private:
	std::wstring mNickname;  // User Nickname.
};


// Inlines
inline TRawMsg*
TMsgAuth1LoginBase::Duplicate(void) const
{ return new TMsgAuth1LoginBase(*this); }

inline unsigned short
TMsgAuth1LoginBase::GetKeyBlockId() const
{ return mKeyBlockId; }

inline void
TMsgAuth1LoginBase::SetKeyBlockId(unsigned short theId)
{ mKeyBlockId = theId; }

inline bool
TMsgAuth1LoginBase::GetNeedKey() const
{ return mNeedKey; }

inline void
TMsgAuth1LoginBase::SetNeedKey(bool theFlag)
{ mNeedKey = theFlag; }

inline const std::wstring&
TMsgAuth1LoginBase::GetLoginName() const
{ return mLoginName; }

inline void
TMsgAuth1LoginBase::SetLoginName(const std::wstring& theName)
{ mLoginName = theName; }

inline const std::wstring&
TMsgAuth1LoginBase::GetCommunityName() const
{ return mCommunityName; }

inline void
TMsgAuth1LoginBase::SetCommunityName(const std::wstring& theName)
{ mCommunityName = theName; }

inline const std::wstring&
TMsgAuth1LoginBase::GetNicknameKey() const
{ return mNicknameKey; }

inline void
TMsgAuth1LoginBase::SetNicknameKey(const std::wstring& theKey)
{ mNicknameKey = theKey; }

inline TRawMsg*
TMsgAuth1LoginRequest::Duplicate(void) const
{ return new TMsgAuth1LoginRequest(*this); }

inline TRawMsg*
TMsgAuth1NewLoginRequest::Duplicate(void) const
{ return new TMsgAuth1NewLoginRequest(*this); }

inline const std::wstring&
TMsgAuth1NewLoginRequest::GetNickname() const
{ return mNickname; }

inline void
TMsgAuth1NewLoginRequest::SetNickname(const std::wstring& theName)
{ mNickname = theName; }

};  // Namespace WONMsg

#endif
#endif