#include "CryptKeyBase.h"


#ifndef _MINI_CRYPT

// CryptKeyBase

// Base class for keys within the WONCrypt library.

// Also contains source for virtual methods of the SymmetricKey, PublicKey,
// and PrivateKey base classes.  (All these methods could be inline if they
// were not virtual).


#include "common/won.h"
#include <iostream>
#include <limits.h>
#include "cryptoFiles/cryptlib.h"
#include "SymmetricKey.h"
#include "PublicKey.h"
#include "PrivateKey.h"
#include "CryptKeyBase.h"

#ifndef _WONCRYPT_NOEXCEPTIONS
#include "CryptException.h"
#include "common/WONExceptCodes.h"
#endif

// Private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::CryptKeyBase;
	using WONCrypt::SymmetricKey;
	using WONCrypt::PublicKey;
	using WONCrypt::PrivateKey;
};


// ** Constructors / Destructor

// Default constructor, allocates a dummy 1 byte key
CryptKeyBase::CryptKeyBase() :
	mKey(NULL),
	mKeyLen(0)
{
	Invalidate();
}


// Copy Constructor
CryptKeyBase::CryptKeyBase(const CryptKeyBase& theKeyR) :
	mKey(NULL),
	mKeyLen(0)
{
	AllocKeyBuf(theKeyR.mKeyLen);
	memcpy(mKey, theKeyR.mKey, mKeyLen);
}

	
// Destructor
CryptKeyBase::~CryptKeyBase(void)
{
	delete [] mKey;
}


// ** Public Methods **

// Assignment operator
CryptKeyBase&
CryptKeyBase::operator=(const CryptKeyBase& theKeyR)
{
	Create(theKeyR.mKeyLen, theKeyR.mKey);  // protected vs assignment to this
	return *this;
}

// Equality operator
bool
CryptKeyBase::operator==(const CryptKeyBase& theKeyR) const
{ 
	return ((mKeyLen == theKeyR.mKeyLen) && (memcmp(mKey, theKeyR.mKey, mKeyLen) == 0)); 
}



// LessThan operator
bool
CryptKeyBase::operator<(const CryptKeyBase& theKeyR) const
{
	int aTst = memcmp(mKey, theKeyR.mKey, (mKeyLen < theKeyR.mKeyLen ? mKeyLen : theKeyR.mKeyLen));
	return (aTst == 0 ? (mKeyLen < theKeyR.mKeyLen) : (aTst < 0));
}


// CryptKeyBase::IsValid()
// Check validity
bool
CryptKeyBase::IsValid() const
{
	return (! ((mKeyLen == 0) || ((mKeyLen == 1) && (*mKey == 0))));
}


// CryptKeyBase::Create(len, data)
// Builds ket from specified binary buffger and length.  Deletes old key if any.
void
CryptKeyBase::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	WTRACE("CryptKeyBase::Create(key,len)");
	if (mKey == theKeyP) return;  // Protect vs create from this

	// Validate inputs
	if ((! theKeyP) || (theLen == 0))
	{
		WDBG_AH("CryptKeyBase::(key,len) TheKey or len is NULL!");
#ifndef _WONCRYPT_NOEXCEPTIONS
		throw WONCrypt::CryptException(WONCommon::ExSoftwareFail, __LINE__, __FILE__, "NULL key or length in CryptKeyBase::CopyKey!");
#else
		Invalidate();
#endif
	}
	else
	{
		AllocKeyBuf(theLen);
		memcpy(mKey, theKeyP, mKeyLen);
	}
}


// CryptKeyBase::Dump
// Streaming method.  Outputs key to specfied stream.  Outputs key length followed
// by key in hex.
void
CryptKeyBase::Dump(std::ostream& os) const
{
	// Output key length
	os << "(Len=" << mKeyLen << " Key=" << std::hex;

	// Output key as hex
	for (int i=0; i < mKeyLen; i++)
		os << static_cast<unsigned short>(mKey[i]) << ' ';

	os << std::dec << ')';
}


// ** Virtual Methods for SymmetricKey **

// SymmetricKey::Create
// Place holder method.  Just calls the base class.
void
SymmetricKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	CryptKeyBase::Create(theLen, theKeyP);
}


// ** Virtual Methods for PublicKey **

// PublicKey::Create(len, data)
// Place holder method.  Just calls the base class.
void
PublicKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	CryptKeyBase::Create(theLen, theKeyP);
}


// PublicKey::Create(len)
// PublicKeys cannot be genrated by default; that takes place in the corresponding
// PrivateKey.  Throws an exception.
void
PublicKey::Create(unsigned short) throw(WONCrypt::CryptException)
{
#ifndef _WONCRYPT_NOEXCEPTIONS
	throw WONCrypt::CryptException(WONCommon::ExSoftwareFail, __LINE__, __FILE__, "Create(len) called on PublicKey.");
#else
	Invalidate();
#endif
}


// ** Virtual Methods for PrivateKey **

// PrivateKey::Create
// Place holder method.  Just calls the base class.
void
PrivateKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	CryptKeyBase::Create(theLen, theKeyP);
}

#else

#pragma warning(disable: 4530)

#include <iostream>
#include <limits.h>
#include "CryptKeyBase.h"
#include "Symmetrickey.h"
#include "PublicKey.h"
#include "PrivateKey.h"


// Private namespace for using and constants
namespace {
	using WONCrypt::CryptKeyBase;
	using WONCrypt::SymmetricKey;
	using WONCrypt::PublicKey;
	using WONCrypt::PrivateKey;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CryptKeyBase::CryptKeyBase() :
	mKey(NULL),
	mKeyLen(0)
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CryptKeyBase::CryptKeyBase(const CryptKeyBase& theKeyR) :
	mKey(NULL),
	mKeyLen(0)
{
	AllocKeyBuf(theKeyR.mKeyLen);
	memcpy(mKey, theKeyR.mKey, mKeyLen);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CryptKeyBase::~CryptKeyBase(void)
{
	delete [] mKey;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CryptKeyBase& CryptKeyBase::operator=(const CryptKeyBase& theKeyR)
{
	Create(theKeyR.mKeyLen, theKeyR.mKey);  // protected vs assignment to this
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool CryptKeyBase::operator==(const CryptKeyBase& theKeyR) const
{ 
	return ((mKeyLen == theKeyR.mKeyLen) && (memcmp(mKey, theKeyR.mKey, mKeyLen) == 0)); 
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool CryptKeyBase::operator<(const CryptKeyBase& theKeyR) const
{
	int aTst = memcmp(mKey, theKeyR.mKey, (mKeyLen < theKeyR.mKeyLen ? mKeyLen : theKeyR.mKeyLen));
	return (aTst == 0 ? (mKeyLen < theKeyR.mKeyLen) : (aTst < 0));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool CryptKeyBase::IsValid() const
{
	return (! ((mKeyLen == 0) || ((mKeyLen == 1) && (*mKey == 0))));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void CryptKeyBase::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	if (mKey == theKeyP) return;  // Protect vs create from this

	// Validate inputs
	if ((! theKeyP) || (theLen == 0))
	{
		Invalidate();
	}
	else
	{
		AllocKeyBuf(theLen);
		memcpy(mKey, theKeyP, mKeyLen);
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void CryptKeyBase::Dump(std::ostream& os) const
{

//	os << "(Len=" << mKeyLen;
/*	
	// Output key length
	os << "(Len=" << mKeyLen << " Key=" << std::hex;

	// Output key as hex
	for (int i=0; i < mKeyLen; i++)
		os << static_cast<unsigned short>(mKey[i]) << ' ';

	os << std::dec << ')';*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SymmetricKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	CryptKeyBase::Create(theLen, theKeyP);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PublicKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	CryptKeyBase::Create(theLen, theKeyP);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PublicKey::Create(unsigned short theLen) 
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PrivateKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	CryptKeyBase::Create(theLen, theKeyP);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif
