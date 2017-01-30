// WON_AuthFamilyBuffer

// Base interface class for Auth buffers within the WONAuth library.  AuthFamilyBuffer
// is an abstract base class, all constructors are protected.

// This class simply makes use of the AuthFamilyBuffer class.  It eliminates the STL
// and VC5 enhancements from the interface.

// *IMPORTANT**
// Implementation relies on the fact that mBufP will NEVER be NULL after construction.

// *NOTE*
// This header is compatible with VC4.  Ergo, no using bool, string, etc.


// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "crypt/EGPublicKey.h"
#include "crypt/EGPrivateKey.h"
#include "AuthFamilyBuffer.h"
#include "WON_AuthFamilyBuffer.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::EGPublicKey;
	using WONCrypt::EGPrivateKey;
	using WONAuth::AuthFamilyBuffer;
};


// ** Constructors / Destructor

// Default constructor, allocates nothing (must be done by derived classes)
WON_AuthFamilyBuffer::WON_AuthFamilyBuffer() :
	mBufP(NULL)
{}


// Copy Constructor, allocates nothing (must be done by derived classes to prevent
// slicing)
WON_AuthFamilyBuffer::WON_AuthFamilyBuffer(const WON_AuthFamilyBuffer& theBufR) :
	mBufP(NULL)
{}

	
// Destructor
WON_AuthFamilyBuffer::~WON_AuthFamilyBuffer(void)
{
	delete mBufP;
}


// ** Public Methods **

// Assignment operator, does nothing.  Actual assign must be done by the derived
// classes to prevent slicing.
WON_AuthFamilyBuffer&
WON_AuthFamilyBuffer::operator=(const WON_AuthFamilyBuffer& theBufR)
{
	return *this;
}


// Check validity
int
WON_AuthFamilyBuffer::IsValid() const
{
	return (mBufP->IsValid() ? RETURN_TRUE : RETURN_FALSE);
}


// Compare method
int
WON_AuthFamilyBuffer::Compare(const WON_AuthFamilyBuffer& theBufR) const
{
	return mBufP->Compare(*theBufR.mBufP);
}


// WON_AuthFamilyBuffer::Pack
// Pack into raw form and sign with specified raw private key.
int
WON_AuthFamilyBuffer::Pack(const unsigned char* thePrivKeyP, unsigned short theLen)
{
	EGPrivateKey aKey(theLen, thePrivKeyP);
	return (mBufP->Pack(aKey) ? RETURN_TRUE : RETURN_FALSE);
}


// WON_AuthFamilyBuffer::Unpack
// Unpack data from raw form into buffer.
int
WON_AuthFamilyBuffer::Unpack(const unsigned char* theRawP, unsigned short theLen)
{
	return (mBufP->Unpack(theRawP, theLen) ? RETURN_TRUE : RETURN_FALSE);
}


// WON_AuthFamilyBuffer::Verify
// Veriy signature using specified raw public key.
int
WON_AuthFamilyBuffer::Verify(const unsigned char* thePubKeyP, unsigned short theLen) const
{
	EGPublicKey aKey(theLen, thePubKeyP);
	return (mBufP->Verify(aKey) ? RETURN_TRUE : RETURN_FALSE);
}


// ** Accessor Methods **
// All methods simply pass through to real buffer object

unsigned short
WON_AuthFamilyBuffer::GetFamily() const
{
	return mBufP->GetFamily();
}

time_t
WON_AuthFamilyBuffer::GetIssueTime() const
{
	return mBufP->GetIssueTime();
}

time_t
WON_AuthFamilyBuffer::GetExpireTime() const
{
	return mBufP->GetExpireTime();
}

unsigned long
WON_AuthFamilyBuffer::GetLifespan() const
{
	return mBufP->GetLifespan();
}

void
WON_AuthFamilyBuffer::SetLifespan(time_t theIssueTime, unsigned long theLifespan)
{
	mBufP->SetLifespan(theIssueTime, theLifespan);
}

void
WON_AuthFamilyBuffer::SetIssueTime(time_t theTime)
{
	mBufP->SetIssueTime(theTime);
}

void
WON_AuthFamilyBuffer::SetExpireTime(time_t theTime)
{
	mBufP->SetExpireTime(theTime);
}

const unsigned char*
WON_AuthFamilyBuffer::GetRaw() const
{
	return mBufP->GetRaw();
}

unsigned short
WON_AuthFamilyBuffer::GetRawLen() const
{
	return mBufP->GetRawLen();
}

const unsigned char*
WON_AuthFamilyBuffer::GetData() const
{
	return mBufP->GetData();
}

unsigned short
WON_AuthFamilyBuffer::GetDataLen() const
{
	return mBufP->GetDataLen();
}

const unsigned char*
WON_AuthFamilyBuffer::GetSignature() const
{
	return mBufP->GetSignature();
}

unsigned short
WON_AuthFamilyBuffer::GetSignatureLen() const
{
	return mBufP->GetSignatureLen();
}

