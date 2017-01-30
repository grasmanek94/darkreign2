// WON_BFSymmetricKey

// Interface class for the Blowfish implementation of a symmetric key.  Length of
// key in bytes can be defined on creation.  Key can be used to encrypt and decrypt
// binary blocks of arbitrary size.

// This class simply makes use of the BFSymmectricKey class.  It eliminates the STL
// and exceptions from the interface.

// Note that the mBFKeyP member is simply a pointer to the allocated key stored in
// the base class.  Ergo, mBFKeyP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mBFKeyP will NEVER be NULL after construction.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "CryptException.h"
#include "BFSymmetricKey.h"
#include "WON_BFSymmetricKey.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::BFSymmetricKey;
	using WONCrypt::CryptException;

	// Constants
	const char* ENCRYPT_FAILED = "Encryption of block failed (check params).";
	const char* DECRYPT_FAILED = "Decryption of block failed.";
};


// ** Constructors / Destructor

// Constructor - construct new key of specified length if theKeyP is NULL.  Otherwise
// copy existing key from buffer.  If theLen is 0, a new key of default length will be
// constructed.
WON_BFSymmetricKey::WON_BFSymmetricKey(unsigned short theLen, const unsigned char* theKeyP) :
	WON_CryptKeyBase(),
	mBFKeyP(NULL)
{
	AllocateKey();
	if (theKeyP)
		Create(theLen, theKeyP);
	else
		Create(theLen);
}


// Copy Constructor
WON_BFSymmetricKey::WON_BFSymmetricKey(const WON_BFSymmetricKey& theKeyR) :
	WON_CryptKeyBase(theKeyR),
	mBFKeyP(NULL)
{
	AllocateKey();
	operator=(theKeyR);
}

	
// Destructor - mBFKeyP deleted by base class.
WON_BFSymmetricKey::~WON_BFSymmetricKey(void)
{}


// ** Private Methods **

// WON_BFSymmetricKey::AllocateKey
// Allocates the real BFSymmetricKey object and sets the key in base class.
void
WON_BFSymmetricKey::AllocateKey()
{
	mBFKeyP = new BFSymmetricKey;
	delete mKeyP;  mKeyP = mBFKeyP;
}


// ** Public Methods **

// Assignment operator
WON_BFSymmetricKey&
WON_BFSymmetricKey::operator=(const WON_BFSymmetricKey& theKeyR)
{
	if (this != &theKeyR)
	{
		WON_CryptKeyBase::operator=(theKeyR);
		*mBFKeyP = *(theKeyR.mBFKeyP);
	}

	return *this;
}


// WON_BFSymmetricKey::Create(len, key)
// Create key from existing raw key of specified length.  Returns true on success,
// false on failure.  Value of lastError is updated.
int
WON_BFSymmetricKey::Create(unsigned short theLen, const unsigned char* theKeyP)
{
	try
	{
		mBFKeyP->Create(theLen, theKeyP);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
		return RETURN_FALSE;
	}

	SetLastError(NULL);
	return RETURN_TRUE;
}


// WON_BFSymmetricKey::Create(len)
// Create new key specified length.  Returns true on success, false on failure.
// Value of lastError is updated.
int
WON_BFSymmetricKey::Create(unsigned short theLen)
{
	try
	{
		mBFKeyP->Create(theLen);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
		return RETURN_FALSE;
	}

	SetLastError(NULL);
	return RETURN_TRUE;
}


// WON_BFSymmetricKey::Encrypt
// Encrypts a binary block of specified length.  Returns (encrypted block, len)
// on success and (NULL, 0) on failure.  Caller is responsible for deleting the
// returned block.  Value of lastError is updated.
WON_BFSymmetricKey::CryptReturn
WON_BFSymmetricKey::Encrypt(const void* theMsgP, unsigned long theLen)
{
	try
	{
		BFSymmetricKey::CryptReturn anEncrypt(mBFKeyP->Encrypt(theMsgP, theLen));
		SetLastError(anEncrypt.first ? NULL : ENCRYPT_FAILED);
		return CryptReturn(anEncrypt.first, anEncrypt.second);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
		return CryptReturn();
	}
}


// WON_BFSymmetricKey::Decrypt
// Decrypts a binary block of specified length.  Returns (decrypted block, len)
// on success and (NULL, 0) on failure.  Caller is responsible for deleting the
// returned block.  Value of lastError is updated.
WON_BFSymmetricKey::CryptReturn
WON_BFSymmetricKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen)
{
	try
	{
		BFSymmetricKey::CryptReturn aDecrypt(mBFKeyP->Decrypt(theMsgP, theLen));
		SetLastError(aDecrypt.first ? NULL : DECRYPT_FAILED);
		return CryptReturn(aDecrypt.first, aDecrypt.second);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
		return CryptReturn();
	}
}
