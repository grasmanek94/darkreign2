// WON_EGPrivateKey

// Interface class for the ElGamal implementation of a private key.  Length of each
// portion of the key in bytes can be defined on creation.  Key can be used to decrypt
// binary blocks of arbitrary size and sign binary blocks of arbitrary size.

// This class simply makes use of the EGPrivateKey class.  It eliminates the STL
// and exceptions from the interface.

// Note that the mEGKeyP member is simply a pointer to the allocated key stored in
// the base class.  Ergo, mEGKeyP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mEGKeyP will NEVER be NULL after construction.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "CryptException.h"
#include "EGPublicKey.h"
#include "EGPrivateKey.h"
#include "WON_EGPublicKey.h"
#include "WON_EGPrivateKey.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::EGPrivateKey;
	using WONCrypt::EGPublicKey;
	using WONCrypt::CryptException;

	// Constants
	const char* DECRYPT_FAILED = "Decryption of block failed.";
	const char* SIGN_FAILED    = "Sign of block failed (check params).";
};


// ** Constructors / Destructor

// Constructor - construct new key of specified length if theKeyP is NULL.  Otherwise
// copy existing key from buffer.  If theLen is 0, a new key of default length will be
// constructed.
WON_EGPrivateKey::WON_EGPrivateKey(unsigned short theLen, const unsigned char* theKeyP) :
	WON_CryptKeyBase(),
	mEGKeyP(NULL)
{
	AllocateKey();
	if (theKeyP)
		Create(theLen, theKeyP);
	else
		Create(theLen);
}


// Copy Constructor
WON_EGPrivateKey::WON_EGPrivateKey(const WON_EGPrivateKey& theKeyR) :
	WON_CryptKeyBase(theKeyR),
	mEGKeyP(NULL)
{
	AllocateKey();
	operator=(theKeyR);
}

	
// Destructor - mEGKeyP deleted by base class.
WON_EGPrivateKey::~WON_EGPrivateKey(void)
{}


// ** Private Methods **

// WON_EGPrivateKey::AllocateKey
// Allocates the real EGPublicKey object and sets the key in base class.
void
WON_EGPrivateKey::AllocateKey()
{
	mEGKeyP = new EGPrivateKey;
	delete mKeyP;  mKeyP = mEGKeyP;
}


// ** Public Methods **

// Assignment operator
WON_EGPrivateKey&
WON_EGPrivateKey::operator=(const WON_EGPrivateKey& theKeyR)
{
	if (this != &theKeyR)
	{
		WON_CryptKeyBase::operator=(theKeyR);
		*mEGKeyP = *(theKeyR.mEGKeyP);
	}

	return *this;
}


// WON_EGPrivateKey::Create(len, key)
// Create key from existing raw key of specified length.  Returns true on success,
// false on failure.  Value of lastError is updated.
int
WON_EGPrivateKey::Create(unsigned short theLen, const unsigned char* theKeyP)
{
	try
	{
		mEGKeyP->Create(theLen, theKeyP);
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


// WON_EGPrivateKey::Create(len)
// Create new key specified length.  Returns true on success, false on failure.
// Value of lastError is updated.
int
WON_EGPrivateKey::Create(unsigned short theLen)
{
	try
	{
		mEGKeyP->Create(theLen);
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


// WON_EGPrivateKey::GetPublicKey
// Updates the value of thekeyR to be the public key of this private key.  Returns
// true on success, false on failure.  Value of lastError is updated.  The value of
// theKeyR is indeterminate on failure.
int
WON_EGPrivateKey::GetPublicKey(WON_EGPublicKey& theKeyR) const
{
	try
	{
		*(theKeyR.mEGKeyP) = dynamic_cast<const EGPublicKey&>(mEGKeyP->GetPublicKey());
	}
	catch (CryptException& anExR)
	{
		const_cast<WON_EGPrivateKey*>(this)->SetLastError(anExR.what());
		anExR.SetLog(false);
		return RETURN_FALSE;
	}

	const_cast<WON_EGPrivateKey*>(this)->SetLastError(NULL);
	return RETURN_TRUE;
}


// WON_EGPrivateKey::Decrypt
// Decrypts a binary block of specified length.  Returns (decrypted block, len)
// on success and (NULL, 0) on failure.  Caller is responsible for deleting the
// returned block.  Value of lastError is updated.
WON_EGPrivateKey::CryptReturn
WON_EGPrivateKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen)
{
	try
	{
		EGPrivateKey::CryptReturn aDecrypt(mEGKeyP->Decrypt(theMsgP, theLen));
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


// WON_EGPrivateKey::Sign
// Signs a binary block of specified length.  Returns (signature block, len)
// on success and (NULL, 0) on failure.  Caller is responsible for deleting the
// returned block.  Value of lastError is updated.
WON_EGPrivateKey::CryptReturn
WON_EGPrivateKey::Sign(const void* theMsgP, unsigned long theLen)
{
	try
	{
		EGPrivateKey::CryptReturn aSig(mEGKeyP->Sign(theMsgP, theLen));
		SetLastError(aSig.first ? NULL : SIGN_FAILED);
		return CryptReturn(aSig.first, aSig.second);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
		return CryptReturn();
	}
}
