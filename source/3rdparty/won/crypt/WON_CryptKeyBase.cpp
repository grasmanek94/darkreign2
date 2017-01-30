// WON_CryptKeyBase

// Base interface class for keys within the WONCrypt library.  CryptKeyBase is an
// abstract base class, all constructors are protected and there is a pure vitural
// method to be overridden.

// This class simply makes use of the CryptKeyBase class.  It eliminates the STL
// and exceptions from the interface.

// We're in WON Source!
#define _WON_SOURCE_

#include "common/won.h"
#include <iostream>
#include "CryptException.h"
#include "CryptKeyBase.h"
#include "WON_CryptKeyBase.h"

// Private namespace for using and constants
namespace {
	using WONCrypt::CryptKeyBase;
	using WONCrypt::CryptException;
};


// ** CryptReturn Methods **

WON_CryptKeyBase::CryptReturn::CryptReturn(unsigned char* theDataP, unsigned long theLen) :
	mData(theDataP),
	mLen(theLen),
	mOwnIt(1)
{}

WON_CryptKeyBase::CryptReturn::CryptReturn(const WON_CryptKeyBase::CryptReturn& theInst) :
	mData(theInst.mData),
	mLen(theInst.mLen),
	mOwnIt(theInst.mOwnIt)
{
	const_cast<WON_CryptKeyBase::CryptReturn&>(theInst).mOwnIt = 0;
}

WON_CryptKeyBase::CryptReturn::~CryptReturn()
{
	if (mOwnIt != 0) delete mData;
}

WON_CryptKeyBase::CryptReturn&
WON_CryptKeyBase::CryptReturn::operator=(const WON_CryptKeyBase::CryptReturn& theInst)
{
	if (mOwnIt != 0) delete mData;
	mData  = theInst.mData;
	mLen   = theInst.mLen;
	mOwnIt = theInst.mOwnIt;
	const_cast<WON_CryptKeyBase::CryptReturn&>(theInst).mOwnIt = 0;
	return *this;
}

// ** Constructors / Destructor

// Default constructor, allocates nothing (must be done by derived classes)
WON_CryptKeyBase::WON_CryptKeyBase() :
	mLastErrP(NULL),
	mKeyP(NULL)
{}


// Copy Constructor, allocates nothing (must be done by derived classes to prevent
// slicing)
WON_CryptKeyBase::WON_CryptKeyBase(const WON_CryptKeyBase& theKeyR) :
	mLastErrP(NULL),
	mKeyP(NULL)
{}

	
// Destructor
WON_CryptKeyBase::~WON_CryptKeyBase(void)
{
	delete [] mLastErrP;
	delete mKeyP;
}


// ** Protected Methods **

// WON_CryptKeyBase::SetLastError
// Set the last error.  Delete current last error.  If new error not NULL, allocate
// space and copy the text.
void
WON_CryptKeyBase::SetLastError(const char* theErrP)
{
	delete [] mLastErrP;

	if (theErrP)
	{
		mLastErrP = new char [strlen(theErrP)+1];
		strcpy(mLastErrP, theErrP);
	}
	else
		mLastErrP = NULL;
}


// ** Public Methods **

// Assignment operator, does nothing.  Actual assign must be done by the derived
// classes to prevent slicing of CryptKeyBase object.
WON_CryptKeyBase&
WON_CryptKeyBase::operator=(const WON_CryptKeyBase& theKeyR)
{
	return *this;
}


// Equality operator
int
WON_CryptKeyBase::operator==(const WON_CryptKeyBase& theKeyR) const
{
	return ((! mKeyP) ? (mKeyP == theKeyR.mKeyP)
	                  : ((! theKeyR.mKeyP) ? RETURN_FALSE : (*mKeyP == *(theKeyR.mKeyP))));
}


// LessThan operator
int
WON_CryptKeyBase::operator<(const WON_CryptKeyBase& theKeyR) const
{
	return ((! mKeyP) ? (mKeyP != theKeyR.mKeyP)
	                  : ((! theKeyR.mKeyP) ? RETURN_FALSE : (*mKeyP < *(theKeyR.mKeyP))));
}


// WON_CryptKeyBase::GetDefKeyLen
// Class method.  Returns the default key length from CryptKeyBase.
unsigned short
WON_CryptKeyBase::GetDefKeyLen()
{
	return CryptKeyBase::KEYLEN_DEF;
}


// WON_CryptKeyBase::GetKey
// Returns binary rep of the key.  Key should never be NULL, but if it is for
// some reason, this method will allocate a dummy key.
const unsigned char*
WON_CryptKeyBase::GetKey() const
{
	if (! mKeyP) const_cast<WON_CryptKeyBase*>(this)->AllocateKey();
	return mKeyP->GetKey();
}


// WON_CryptKeyBase::GetKeyLen
// Returns length of binary rep of the key.  Key should never be NULL, but if it
// is for some reason, this method will allocate a dummy key.
unsigned short
WON_CryptKeyBase::GetKeyLen() const
{
	if (! mKeyP) const_cast<WON_CryptKeyBase*>(this)->AllocateKey();
	return mKeyP->GetKeyLen();
}
