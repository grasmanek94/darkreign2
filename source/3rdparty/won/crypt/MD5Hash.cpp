#include "MD5Hash.h"

#ifndef _MINI_CRYPT

// MD5Hash

// Class that encapsulates a MD5 hash implementation.  Anum number of bytes may be
// hashed to a constant length value.  Use the Put() methods to add bytes and the
// Hash() method to generate the hash value.  Multiple calls to Put() may be made
// before calling Hash(); use this techinque to queue up bytes for hashing.


#include "common/won.h"
#include "cryptoFiles/md5.h"
#include "MD5Hash.h"

// private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::MD5Hash;
};


// ** Constructors / Destructor

// Default constructor
MD5Hash::MD5Hash() :
	mHashP(NULL),
	mBufP(new unsigned char [HashLength()]),
	mFail(false)
{
	Reset();
}


// Construct and add bytes for hashing.
MD5Hash::MD5Hash(const unsigned char* theDataP, unsigned int theLen) :
	mHashP(NULL),
	mBufP(new unsigned char [HashLength()]),
	mFail(false)
{
	Reset();
	Put(theDataP, theLen);
}


// Destructor
MD5Hash::~MD5Hash(void)
{
	delete mHashP;
	delete mBufP;
}


// ** Public Methods **

// MD5Hash::Put()
// Add an a stream of bytes for hashing.
void
MD5Hash::Put(const unsigned char* theDataP, unsigned int theLen)
{
	WTRACE("MD5Hash::Put");
	try
	{
		WDBG_LL("MD5Hash::Put Putting bytes, len=" << theLen);
		mHashP->Update(theDataP, theLen);
	}
#ifdef _DEBUG
	catch (Exception& anEx)
#else
	catch (Exception&)
#endif
	{
		WDBG_AH("MD5Hash::Put Caught CryptoLib exception: " << anEx.what());
		mFail = true;
	}
}


// MD5Hash::Hash()
// Perform hash of any bytes appened into internal buf and return internal buf
const unsigned char*
MD5Hash::Hash()
{
	WTRACE("MD5Hash::Hash");

	// If all is well, generate the hash
	if (! mFail)
	{
		try
		{
			WDBG_LL("MD5Hash::Hash Hashing...");
			mHashP->Final(mBufP);
		}
#ifdef _DEBUG
		catch (Exception& anEx)
#else
		catch (Exception&)
#endif
		{
			WDBG_AH("MD5Hash::Hash Caught CryptoLib exception: " << anEx.what());
			mFail = true;
		}
	}

	bool aSave = mFail;
	Reset();
	return (aSave ? NULL : mBufP);
}


// MD5Hash::Reset()
// Reset for new hashing.  Clears any bytes already added with Put()
void
MD5Hash::Reset()
{
	WTRACE("MD5Hash::Reset");
	WDBG_LL("MD5Hash::Reset Reseting...");
	delete mHashP;  mHashP = new MD5;
	mFail = false;
}


// MD5Hash::HashLength()
// Retur length of hash value in bytes
unsigned int
MD5Hash::HashLength()
{
	return MD5::DIGESTSIZE;
}

#endif

