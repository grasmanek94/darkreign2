#include "BFSymmetricKey.h"


#ifndef _MINI_CRYPT

// BFSymmetricKey

// Class that encapsulates a Blowfish implementation of a symmectric key.  Length of
// key in bytes can be defined on creation.  Key can be used to encrypt and decrypt
// binary blocks of arbitrary size.


#include "common/won.h"
#include "common/WONEndian.h"
#include "cryptoFiles/blowfish.h"
#include "cryptoFiles/cbc.h"
#include "Randomizer.h"
#include "BFSymmetricKey.h"

#ifndef _WONCRYPT_NOEXCEPTIONS
#include "CryptException.h"
#include "common/WONExceptCodes.h"
#endif

// private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::SymmetricKey;
	using WONCrypt::BFSymmetricKey;

	// constants
	WONCommon::RawBuffer gCryptIV(Blowfish::BLOCKSIZE, 0xff);
};


// ** Constructors / Destructor

// Default constructor
BFSymmetricKey::BFSymmetricKey() :
	SymmetricKey(),
	mEncryptP(NULL),
	mDecryptP(NULL)
{}


// Construct from existing data (theKeyP specified) or generate new key of specified
// length.
	BFSymmetricKey::BFSymmetricKey(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException) :
	SymmetricKey(),
	mEncryptP(NULL),
	mDecryptP(NULL)
{
	if ((theLen > 0) && (theKeyP))
		Create(theLen, theKeyP);
	else
		Create(theLen);
}


// Copy Constructor
BFSymmetricKey::BFSymmetricKey(const BFSymmetricKey& theKeyR) :
	SymmetricKey(theKeyR),
	mEncryptP(NULL),
	mDecryptP(NULL)
{}

	
// Destructor
BFSymmetricKey::~BFSymmetricKey(void)
{
	ClearLocal();
}


// ** Private Methods **

// BFSymmetricKey::ClearLocal()
// Deletes and NULLs allocated members.
void
BFSymmetricKey::ClearLocal()
{
	delete mEncryptP;  mEncryptP = NULL;
	delete mDecryptP;  mDecryptP = NULL;
}



// ** Public Methods **

// Assignment operator
BFSymmetricKey&
BFSymmetricKey::operator=(const BFSymmetricKey& theKey)
{
	if (this != &theKey)  // protect vs a = a
	{
		ClearLocal();
		SymmetricKey::operator=(theKey);
	}
	return *this;
}


// BFSymmetricKey::Create(len)
// Generates a new symmetric key of the specified length in bytes.  (If theLen
// is zero, default length is used.)  Old key (if any) is replaced.
void
BFSymmetricKey::Create(unsigned short theLen) throw(WONCrypt::CryptException)
{
	WTRACE("BFSymmetricKey::Create");
	ClearLocal();

	// Set length and alloc space for new key (length of 0 implies default)
	AllocKeyBuf(theLen == 0 ? KEYLEN_DEF : theLen);
	WDBG_LL("BFSymmetricKey::Create Alloc key, len=" << mKeyLen);

	try
	{
		WDBG_LL("BFSymmetricKey::Create Generating new key.");
		Randomizer::GetBlock(mKey, mKeyLen);
	}
	catch (Exception& anExR)
	{
		WDBG_LL("BFSymmetricKey::Create Caught crypto++ exception: " << anExR.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		throw WONCrypt::CryptException(WONCommon::ExCryptoLib, __LINE__, __FILE__, anExR.what());
#else
		Invalidate();
#endif
	}

	WDBG_LM("BFSymmetricKey::Create key=" << *this);
}


// BFSymmetricKey::Create(len, data)
// Copies existing key.  Clears allocated members.  Old key (if any) is replaced.
void
BFSymmetricKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	ClearLocal();
	SymmetricKey::Create(theLen, theKeyP);
}


// BFSymmetricKey::Encrypt
// Encrypts a block of data of the specified length using Blowfish encryption for this
// key.  Returns allocated block/len of encrypted data.
BFSymmetricKey::CryptReturn
BFSymmetricKey::Encrypt(const void* theMsgP, unsigned long theLen) const
{
	WTRACE("BFSymmetricKey::Encrypt");
	WDBG_LH("BFSymmetricKey::Encrypt Encrypting block, len=" << theLen << "  key=" << *this);

	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		WDBG_LM("BFSymmetricKey::Encrypt No data to encrypt");
		return CryptReturn(NULL,0);
	}

	try
	{
		if (! mEncryptP) mEncryptP = new BlowfishEncryption(mKey, mKeyLen);
		CBCPaddedEncryptor aFilter(*mEncryptP, gCryptIV.data());

		// Add the original length, input block, then close
		WDBG_LL("BFSymmetricKey::Encrypt Adding original length and input block.");
		unsigned long tmpLen = getLittleEndian(theLen);
		aFilter.Put(reinterpret_cast<unsigned char*>(&tmpLen), sizeof(tmpLen));
		aFilter.Put(reinterpret_cast<const unsigned char*>(theMsgP), theLen);
		aFilter.Close();

		// Build return
		WDBG_LL("BFSymmetricKey::Encrypt Building return values.");
		unsigned long anOutLen = aFilter.MaxRetrieveable();
		if (anOutLen == 0)
		{
			WDBG_LM("BFSymmetricKey::Encrypt Encryption has failed.");
			return CryptReturn(NULL,0);
		}

		auto_ptr<unsigned char> anOutP(new unsigned char [anOutLen]);
		aFilter.Get(anOutP.get(), anOutLen);

		WDBG_LM("BFSymmetricKey::Encrypt Encrypted, out block len=" << anOutLen);
		return CryptReturn(anOutP.release(), anOutLen);
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("BFSymmetricKey::Encrypt Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return CryptReturn(NULL,0);
	}
}


// BFSymmetricKey::Decrypt
// Decrypts a block of data of the specified length using Blowfish decryption for this
// key.  Returns allocated block/len of decrypted data.
BFSymmetricKey::CryptReturn
BFSymmetricKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen) const
{
	WTRACE("BFSymmetricKey::Decrypt");
	WDBG_LH("BFSymmetricKey::Decrypt Decrypting block, len=" << theLen << "  key=" << *this);

	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		WDBG_LM("BFSymmetricKey::Decrypt No data to decrypt");
		return CryptReturn(NULL,0);
	}

	try
	{
		if (! mDecryptP) mDecryptP = new BlowfishDecryption(mKey, mKeyLen);
		CBCPaddedDecryptor aFilter(*mDecryptP, gCryptIV.data());
		unsigned long      anOutLen = 0;

		// Add the input block, then close
		WDBG_LL("BFSymmetricKey::Decrypt Adding input block.");
		aFilter.Put(theMsgP, theLen);
		aFilter.Close();
		if (aFilter.MaxRetrieveable() < sizeof(anOutLen))
		{
			WDBG_LM("BFSymmetricKey::Decrypt Decryption has failed.");
			return CryptReturn(NULL,0);
		}

		// Extract length
		if (aFilter.Get(reinterpret_cast<unsigned char*>(&anOutLen), sizeof(anOutLen)) != sizeof(anOutLen))
		{
			WDBG_LM("BFSymmetricKey::Decrypt fail to read orignal block len.");
			return CryptReturn(NULL,0);
		}
		makeLittleEndian(anOutLen);

		// Verify Length
		if (anOutLen > aFilter.MaxRetrieveable())
		{
			WDBG_LM("BFSymmetricKey::Decrypt original block length too long.");
			return CryptReturn(NULL,0);
		}

		// Extract data
		auto_ptr<unsigned char> anOutP(new unsigned char [anOutLen]);
		if (aFilter.Get(anOutP.get(), anOutLen) != anOutLen)
		{
			WDBG_LM("BFSymmetricKey::Decrypt fail to read orignal block.");
			return CryptReturn(NULL,0);
		}

		WDBG_LM("BFSymmetricKey::Decrypt Decrypted, out block len=" << anOutLen);
		return CryptReturn(anOutP.release(), anOutLen);
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("BFSymmetricKey::Decrypt Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return CryptReturn(NULL,0);
	}
}

#else

// BFSymmetricKey

// Class that encapsulates a Blowfish implementation of a symmectric key.  Length of
#include "BFSymmetricKey.h"
#include "Random.h"

namespace 
{
	using namespace WONCryptMini;
	using WONCrypt::SymmetricKey;
	using WONCrypt::BFSymmetricKey;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey::BFSymmetricKey() :
	SymmetricKey()
{}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey::BFSymmetricKey(unsigned short theLen, const unsigned char* theKeyP) :
	SymmetricKey()
{
	if ((theLen > 0) && (theKeyP))
		Create(theLen, theKeyP);
	else
		Create(theLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey::BFSymmetricKey(const BFSymmetricKey& theKeyR) 
{
	Create(theKeyR.GetKeyLen(),theKeyR.GetKey());
}

	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey::~BFSymmetricKey(void)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey&
BFSymmetricKey::operator=(const BFSymmetricKey& theKey)
{
	if (this != &theKey)  // protect vs a = a
	{
		Create(theKey.GetKeyLen(),theKey.GetKey());
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void BFSymmetricKey::Create(unsigned short theLen) 
{
	AllocKeyBuf(theLen == 0 ? KEYLEN_DEF : theLen);

	Random aRandom;
	aRandom.nextBytes(mKey,mKeyLen);
	mBlowfish.setKey(mKey,mKeyLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void BFSymmetricKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	SymmetricKey::Create(theLen, theKeyP);
	mBlowfish.setKey(mKey,mKeyLen);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BFSymmetricKey::CryptReturn BFSymmetricKey::Encrypt(const void* theMsgP, unsigned long theLen) const
{
	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		return CryptReturn(NULL,0);
	}


	RawBuffer anEncrypt;
	if(!mBlowfish.encrypt(theMsgP,theLen,anEncrypt))
		return CryptReturn(NULL,0);

	unsigned char *anOutP = new unsigned char[anEncrypt.length()];
	memcpy(anOutP,anEncrypt.data(),anEncrypt.length());

	return CryptReturn(anOutP,anEncrypt.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



BFSymmetricKey::CryptReturn BFSymmetricKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen) const
{
	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		return CryptReturn(NULL,0);
	}


	RawBuffer aDecrypt;
	if(!mBlowfish.decrypt(theMsgP,theLen,aDecrypt))
		return CryptReturn(NULL,0);

	unsigned char *anOutP = new unsigned char[aDecrypt.length()];
	memcpy(anOutP,aDecrypt.data(),aDecrypt.length());

	return CryptReturn(anOutP,aDecrypt.length());
}

#endif
