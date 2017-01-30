// WON_AuthPublicKeyBlock1

// Interface class for the Auth PublicKeyBlock for WON.  Encapsulates a list of one
// or more ElGamal public keys.  

// The first key in the key block is the most recent and should be considered the
// only valid key.  Other keys in the block are still valid but are being
// obsoleted.  Always use the first key for encryption.  Singature verification
// should be performed first with the first key and, if the verify fails, any
// additional keys in the block.

// **IMPORTANT NOTE**
// The Auth keyBlock is meant to only be gnerated by the WON AuthServer.  Thus there
// is not interface to generate a new key block in this class.  A key block may be
// constructed from raw data and its members read.  No setting or buuilding.

// Note that the mBlockP member is simply a pointer to the allocated block stored
// in the base class.  Ergo, mBlockP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mBlockP will NEVER be NULL after construction.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "crypt/EGPublicKey.h"
#include "Auth1Certificate.h"
#include "Auth1PublicKeyBlock.h"
#include "WON_AuthCertificate1.h"
#include "WON_AuthPublicKeyBlock1.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::EGPublicKey;
	using WONAuth::Auth1PublicKeyBlock;
};


// ** Constructors / Destructor

// Default Constructor
WON_AuthPublicKeyBlock1::WON_AuthPublicKeyBlock1() :
	WON_AuthFamilyBuffer(),
	mBlockP(NULL),
	mItrP(NULL)
{
	mBlockP = new Auth1PublicKeyBlock;
	mBufP   = mBlockP;
	mItrP   = new Auth1PublicKeyBlock::PublicKeyList::iterator(mBlockP->KeyList().begin());
}


// Construct from raw data- Construct a new key block and then unpack it.
WON_AuthPublicKeyBlock1::WON_AuthPublicKeyBlock1(const unsigned char* theRawP,
                                                 unsigned short theLen) :
	WON_AuthFamilyBuffer(),
	mBlockP(NULL),
	mItrP(NULL)
{
	mBlockP = new Auth1PublicKeyBlock(theRawP, theLen);
	mBufP   = mBlockP;
	mItrP   = new Auth1PublicKeyBlock::PublicKeyList::iterator(mBlockP->KeyList().begin());
}


// Copy Constructor
WON_AuthPublicKeyBlock1::WON_AuthPublicKeyBlock1(const WON_AuthPublicKeyBlock1& theBlockR) :
	WON_AuthFamilyBuffer(theBlockR),
	mBlockP(NULL),
	mItrP(NULL)
{
	mBlockP = new Auth1PublicKeyBlock(*theBlockR.mBlockP);
	mBufP   = mBlockP;
	mItrP   = new Auth1PublicKeyBlock::PublicKeyList::iterator(mBlockP->KeyList().begin());
}

	
// Destructor - mBlockP deleted by base class.
WON_AuthPublicKeyBlock1::~WON_AuthPublicKeyBlock1()
{
	delete mItrP;
}


// ** Public Methods **

// Assignment operator
WON_AuthPublicKeyBlock1&
WON_AuthPublicKeyBlock1::operator=(const WON_AuthPublicKeyBlock1& theBlockR)
{
	if (this != &theBlockR)
	{
		WON_AuthFamilyBuffer::operator=(theBlockR);
		*mBlockP = *(theBlockR.mBlockP);
		*mItrP   = mBlockP->KeyList().begin();
	}

	return *this;
}


// Certificate verifier method
int
WON_AuthPublicKeyBlock1::VerifyCertificate(const WON_AuthCertificate1* theCertP) const
{
	bool aRet = (theCertP->mCertP ? mBlockP->VerifyFamilyBuffer(*(theCertP->mCertP)) : false);
	return (aRet ? RETURN_TRUE : RETURN_FALSE);
}


// ** Accessor Methods **
// All methods simply pass through to real certificate object

unsigned short
WON_AuthPublicKeyBlock1::GetBlockId() const
{
	return mBlockP->GetBlockId();
}

int
WON_AuthPublicKeyBlock1::GetNumKeys() const
{
	return mBlockP->KeyList().size();
}

WON_AuthPublicKeyBlock1::PubKeyReturn
WON_AuthPublicKeyBlock1::GetFirstKey() const
{
	*mItrP = mBlockP->KeyList().begin();
	return GetNextKey();
}

WON_AuthPublicKeyBlock1::PubKeyReturn
WON_AuthPublicKeyBlock1::GetNextKey() const
{
	PubKeyReturn aRet;
	if (*mItrP != mBlockP->KeyList().end())
	{
		aRet.mKeyP   = (*mItrP)->GetKey();
		aRet.mKeyLen = (*mItrP)->GetKeyLen();
		(*mItrP)++;
	}

	return aRet;
}
