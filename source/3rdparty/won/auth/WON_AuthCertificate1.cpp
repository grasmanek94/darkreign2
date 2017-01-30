// WON_AuthCertificate1

// Interface class for the Auth Certificate1 for WON.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.

// Note that the mCertP member is simply a pointer to the allocated certificate stored
// in the base class.  Ergo, mCertP does not need to be deleted.

// *IMPORTANT**
// Implementation relies on the fact that mCertP will NEVER be NULL after construction.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "crypt/EGPublicKey.h"
#include "Auth1Certificate.h"
#include "WON_AuthCertificate1.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::EGPublicKey;
	using WONAuth::Auth1Certificate;
};


// ** Constructors / Destructor

// Default Constructor - construct from optional user data.
WON_AuthCertificate1::WON_AuthCertificate1(unsigned long theUserId,
                                           unsigned long theCommunityId,
                                           unsigned short theTrustLevel) :
	WON_AuthFamilyBuffer(),
	mCertP(NULL)
{
	mCertP = new Auth1Certificate(theUserId, theCommunityId, theTrustLevel);
	mBufP  = mCertP;
}


// Construct from raw data- Construct a new certificate and then unpack it.
WON_AuthCertificate1::WON_AuthCertificate1(const unsigned char* theRawP, unsigned short theLen) :
	WON_AuthFamilyBuffer(),
	mCertP(NULL)
{
	mCertP = new Auth1Certificate(theRawP, theLen);
	mBufP  = mCertP;
}


// Copy Constructor
WON_AuthCertificate1::WON_AuthCertificate1(const WON_AuthCertificate1& theCertR) :
	WON_AuthFamilyBuffer(theCertR),
	mCertP(NULL)
{
	mCertP = new Auth1Certificate(*theCertR.mCertP);
	mBufP  = mCertP;
}

	
// Destructor - mCertP deleted by base class.
WON_AuthCertificate1::~WON_AuthCertificate1()
{}


// ** Public Methods **

// Assignment operator
WON_AuthCertificate1&
WON_AuthCertificate1::operator=(const WON_AuthCertificate1& theCertR)
{
	if (this != &theCertR)
	{
		WON_AuthFamilyBuffer::operator=(theCertR);
		*mCertP = *(theCertR.mCertP);
	}

	return *this;
}


// ** Accessor Methods **
// All methods simply pass through to real certificate object

unsigned long
WON_AuthCertificate1::GetUserId() const
{
	return mCertP->GetUserId();
}

unsigned long
WON_AuthCertificate1::GetCommunityId() const
{
	return mCertP->GetCommunityId();
}

unsigned short
WON_AuthCertificate1::GetTrustLevel() const
{
	return mCertP->GetTrustLevel();
}

const unsigned char*
WON_AuthCertificate1::GetPubKey() const
{
	return mCertP->GetPubKey().GetKey();
}

unsigned short
WON_AuthCertificate1::GetPubKeyLen() const
{
	return mCertP->GetPubKey().GetKeyLen();
}

void
WON_AuthCertificate1::SetUserId(unsigned long theId)
{
	mCertP->SetUserId(theId);
}

void
WON_AuthCertificate1::SetCommunityId(unsigned long theId)
{
	mCertP->SetCommunityId(theId);
}

void
WON_AuthCertificate1::SetTrustLevel(unsigned short theLevel)
{
	mCertP->SetTrustLevel(theLevel);
}

void
WON_AuthCertificate1::SetPublicKey(const unsigned char* theKeyP, unsigned short theLen)
{
	EGPublicKey aKey(theLen, theKeyP);
	mCertP->SetPublicKey(aKey);
}
