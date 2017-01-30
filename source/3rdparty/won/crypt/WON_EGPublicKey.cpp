// WON_EGPublicKey

// Interface class for the ElGamal implementation of a public key.  ElGamal public keys
// cannot be created by this class.  Use the ElGamal private key class to generate the
// corresponding public key..  Key can be used to encrypt binary blocks of arbitrary
// size and verify signatures on binary blocks of arbitrary size.

// This class simply makes use of the EGPublicKey class.  It eliminates the STL
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
#include "WON_EGPrivateKey.h"
#include "WON_EGPublicKey.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::EGPublicKey;
	using WONCrypt::CryptException;

	// Constants
	const char* ENCRYPT_FAILED = "Encryption of block failed (check params).";
	const char* VERIFY_FAILED  = "Verify of signature block failed.";
};


// ** Constructors / Destructor

// Constructor - copy existing key from raw key.
WON_EGPublicKey::WON_EGPublicKey(unsigned short theLen, const unsigned char* theKeyP) :
	WON_CryptKeyBase(),
	mEGKeyP(NULL)
{
	AllocateKey();
	if ((theLen > 0) && (theKeyP))
		Create(theLen, theKeyP);
}


// Constructor - from private key
WON_EGPublicKey::WON_EGPublicKey(const WON_EGPrivateKey& theKeyR) :
	WON_CryptKeyBase(theKeyR),
	mEGKeyP(NULL)
{
	AllocateKey();
	operator=(theKeyR);
}

	
// Copy Constructor
WON_EGPublicKey::WON_EGPublicKey(const WON_EGPublicKey& theKeyR) :
	WON_CryptKeyBase(theKeyR),
	mEGKeyP(NULL)
{
	AllocateKey();
	operator=(theKeyR);
}

	
// Destructor - mEGKeyP deleted by base class.
WON_EGPublicKey::~WON_EGPublicKey(void)
{}


// ** Private Methods **

// WON_EGPublicKey::AllocateKey
// Allocates the real EGPublicKey object and sets the key in base class.
void
WON_EGPublicKey::AllocateKey()
{
	mEGKeyP = new EGPublicKey;
	delete mKeyP;  mKeyP = mEGKeyP;
}


// ** Public Methods **

// Assignment operator
WON_EGPublicKey&
WON_EGPublicKey::operator=(const WON_EGPublicKey& theKeyR)
{
	if (this != &theKeyR)
	{
		WON_CryptKeyBase::operator=(theKeyR);
		*mEGKeyP = *(theKeyR.mEGKeyP);
	}

	return *this;
}


// Assignment operator - from private key
WON_EGPublicKey&
WON_EGPublicKey::operator=(const WON_EGPrivateKey& theKeyR)
{
	if (! theKeyR.GetPublicKey(*this))
		SetLastError(theKeyR.GetLastError());

	return *this;
}


// WON_EGPublicKey::Create(len, key)
// Create key from existing raw key of specified length.  Returns true on success,
// false on failure.  Value of lastError is updated.
int
WON_EGPublicKey::Create(unsigned short theLen, const unsigned char* theKeyP)
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


// WON_EGPublicKey::Encrypt
// Encrypts a binary block of specified length.  Returns (encrypted block, len)
// on success and (NULL, 0) on failure.  Caller is responsible for deleting the
// returned block.  Value of lastError is updated.
WON_EGPublicKey::CryptReturn
WON_EGPublicKey::Encrypt(const void* theMsgP, unsigned long theLen)
{
	try
	{
		EGPublicKey::CryptReturn anEncrypt(mEGKeyP->Encrypt(theMsgP, theLen));
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


// WON_EGPublicKey::Verify
// Verifies a signature block for a given binary block.  Returns true if signature
// verifies and false if signature fails verification or an error occurs.  Value of
// lastError is updated.
int
WON_EGPublicKey::Verify(const unsigned char* theSigP, unsigned long theSigLen,
	                    const void* theMsgP, unsigned long theMsgLen)
{
	bool aRet = false;
	try
	{
		aRet = mEGKeyP->Verify(theSigP, theSigLen, theMsgP, theMsgLen);
		SetLastError(aRet ? NULL : VERIFY_FAILED);
	}
	catch (CryptException& anExR)
	{
		SetLastError(anExR.what());
		anExR.SetLog(false);
	}

	return (aRet ? RETURN_TRUE : RETURN_FALSE);
}
