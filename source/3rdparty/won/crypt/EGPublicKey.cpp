

// EGPublicKey

// Class that encapsulates an ElGamal Public key used for encryption and signatures.
// ElGamal Public keys cannot be created by this class.  Use the ElGamal private key
// class to generate the corresponding public key.

// NOTE: Binary representation of EGPublicKey
// The binary rep EGPublicKey is stored using the binary rep of the ElGamalVerifier
// class from the Crypto++ library.  This representation is a concatentation of all of
// the factors (p, q, g, y) needed to perform the signature verification. Unfortunately,
// the ElGamalEncryptor class in the Crypto++ library uses a subset of this
// representation for its work; it only expects/stores p, g, y.  This means that the
// binary repsentation is not compatible for the Crypto++ public key ElGamal classes.
// There no way to build either class from the other one and no one to access the factors
// in either class.

// In order to solve this problem, the ElGamalEncryptor is built manually from
// the binary representation.  This is a HACK and is subject to problems if the
// underlying implementation in Crypto++ is ever altered.


// This is WONCrypt source
#define _WONCRYPT_SOURCEFILE
#include "EGPublicKey.h"


#ifndef _MINI_CRYPT

#include "common/won.h"
#include "common/WONEndian.h"
#include <iostream>
#include "cryptoFiles/cryptlib.h"
#include "cryptoFiles/integer.h"
#include "cryptoFiles/randpool.h"
#include "cryptoFiles/md5.h"
#include "cryptoFiles/nr.h"
#include "cryptoFiles/elgamal.h"
#include "cryptoFiles/queue.h"
#include "cryptoFiles/asn.h"
#include "Randomizer.h"
#include "EGPrivateKey.h"
#include "EGPublicKey.h"

#ifndef _WONCRYPT_NOEXCEPTIONS
#include "CryptException.h"
#include "common/WONExceptCodes.h"
#endif

// Private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::PublicKey;
	using WONCrypt::EGPublicKey;
	using WONCrypt::EGPrivateKey;
};


// ** Constructors / Destructor

// Default constructor
EGPublicKey::EGPublicKey() :
	PublicKey(),
	mCryptP(NULL),
	mSigP(NULL)
{}


// Construct from existing key
	EGPublicKey::EGPublicKey(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException) :
	PublicKey(),
	mCryptP(NULL),
	mSigP(NULL)
{
	Create(theLen, theKeyP);
}


// Construct from EGPrivateKey
EGPublicKey::EGPublicKey(const EGPrivateKey& theKeyR) throw(WONCrypt::CryptException) :
	PublicKey(),
	mCryptP(NULL),
	mSigP(NULL)
{
	CopyFromPrivateKey(theKeyR);
}


// Copy Constructor
EGPublicKey::EGPublicKey(const EGPublicKey& theKeyR) :
	PublicKey(theKeyR),
	mCryptP(NULL),
	mSigP(NULL)
{}

	
// Destructor
EGPublicKey::~EGPublicKey(void)
{
	ClearLocal();
}


// ** Private Methods **

// EGPublicKey::ClearLocal()
// Deletes and NULLs allocated members.
void
EGPublicKey::ClearLocal()
{
	delete mCryptP;  mCryptP = NULL;
	delete mSigP;    mSigP   = NULL;
}


// EGPublicKey::CopyFromPrivateKey
// Copies public key from a private key.  Existing key (if any) is replaced.  This
// method takes the binary rep of the private key and drops the private portion
// to generate the public key.
void
EGPublicKey::CopyFromPrivateKey(const EGPrivateKey& theKeyR)
{
	ClearLocal();

	try
	{
		ByteQueue aPrivQueue;
		aPrivQueue.Put(theKeyR.GetKey(), theKeyR.GetKeyLen());
		aPrivQueue.Close();

		ElGamalVerifier aKey(aPrivQueue);
		ByteQueue       aPubQueue;
		aKey.DEREncode(aPubQueue);
		aPubQueue.Close();

		AllocKeyBuf(aPubQueue.MaxRetrieveable());
		aPubQueue.Get(mKey, mKeyLen);
	}
	catch (Exception& anEx)
	{
		WDBG_AH("EGPublicKey::CopyFromPrivateKey Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		throw WONCrypt::CryptException(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#else
		Invalidate();
#endif
	}
}


// EGPublicKey::AllocateCrypt
// Allocates the mCryptP member if needed.  Manually decodes binary rep into factors
// and feeds appropriate factor to the ElGamalEncryptor class from Crypto++.
void
EGPublicKey::AllocateCrypt() const
{
	WTRACE("EGPublicKey::AllocateCrypt");
	if (! mCryptP)
	{
		WDBG_LL("EGPublicKey::AllocateCrypt Allocating crypt object.");
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();

		// *HACK*
		// Manually build the ElGamalEncryptor object.  See notes at
		// top of this file.
		Integer p, q, g, y;
		BERSequenceDecoder seq(aQueue);
		p.BERDecode(seq);
		q.BERDecode(seq);
		g.BERDecode(seq);
		y.BERDecode(seq);
		mCryptP = new ElGamalEncryptor(p, g, y);
	}
}


// EGPublicKey::AllocateSig
// Allocates the mSigP member if needed.  Feeds binary rep of key into the
// ElGamalVerifier class from Crypto++.
void
EGPublicKey::AllocateSig() const
{
	WTRACE("EGPublicKey::AllocateSig");
	if (! mSigP)
	{
		WDBG_LL("EGPublicKey::AllocateSig Allocating crypt object.");
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();
		mSigP = new ElGamalVerifier(aQueue);
	}
}


// EGPublicKey::EncryptBlock
// Encrypts a block of specified length into the specified queue.  This method is
// called by the EncryptData() method for each block.  Note that theLen MUST NOT exceed
// the max block size of the mCryptP object (EncryptData enforces this).  Also note that
// mCryptP must be valid before this method is called.
void
EGPublicKey::EncryptBlock(BufferedTransformation& aQueue, const unsigned char* theBlockP,
                         unsigned long theLen) const
{
	WTRACE("EGPublicKey::EncryptBlock");
	WDBG_LL("EGPublicKey::EncryptBlock, len=" << theLen << ", cipherlen=" << mCryptP->CipherTextLength());
	auto_ptr<unsigned char> aBuf(new unsigned char[mCryptP->CipherTextLength()]);
	mCryptP->Encrypt(Randomizer::GetPool(), theBlockP, theLen, aBuf.get());
	aQueue.Put(aBuf.get(), mCryptP->CipherTextLength());
}


// EGPublicKey::EncryptData
// Encrypts a block of specified length into the specified	queue.  Determines the
// number of individual blocks to be encrypted.  Adds block count to queue.  Then
// calls EncryptBlock() to encrypt each block into the queue.  Note that mCryptP
// must be valid before this method is called.
void
EGPublicKey::EncryptData(BufferedTransformation& aQueue, const unsigned char* theMsgP,
                         unsigned long theLen) const
{
	WTRACE("EGPublicKey::EncryptData");
	WDBG_LL("EGPublicKey::EncryptData, len=" << theLen);

	// Determine block length and number of blocks
	unsigned long aBlockLen = mCryptP->MaxPlainTextLength();
	unsigned long aNumBlock = theLen / aBlockLen;
	if ((theLen % aBlockLen) != 0) aNumBlock++;
	WDBG_LL("EGPublicKey::EncryptBlock NumBlocks=" << aNumBlock << ", BlockLen=" << aBlockLen);

	// A num blocks to output
	unsigned long tmpNumBlock = getLittleEndian(aNumBlock);
	aQueue.Put(reinterpret_cast<unsigned char*>(&tmpNumBlock), sizeof(tmpNumBlock));

	// Encrypt the data, one block at a time
	while (theLen > aBlockLen)
	{
		EncryptBlock(aQueue, theMsgP, aBlockLen);
		theMsgP += aBlockLen;
		theLen  -= aBlockLen;
	}

	// Encrypt the last block and close the queue
	EncryptBlock(aQueue, theMsgP, theLen);
	aQueue.Close();
}


// ** Public Methods **

// Assignment Operator
EGPublicKey&
EGPublicKey::operator=(const EGPublicKey& theKeyR)
{
	if (this != &theKeyR)  // protect vs a = a
	{
		ClearLocal();
		PublicKey::operator=(theKeyR);
	}
	return *this;
}


// Assignment from EGPrivateKey
EGPublicKey&
EGPublicKey::operator=(const EGPrivateKey& theKeyR)
{
	CopyFromPrivateKey(theKeyR);
	return *this;
}


// EGPublicKey::Create(len, data)
// Copies existing key.  Clears allocated members.  Old key (if any) is replaced.
void
EGPublicKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	WTRACE("EGPublicKey::Create(theLen, theKey)");
	ClearLocal();
	PublicKey::Create(theLen, theKeyP);
	WDBG_LM("EGPublicKey::Create(theLen, theKey) Created. Key=" << *this);
}


// EGPublicKey::Encrypt
// Encrypts a block of data of the specified length using ElGamal encryption for this
// key.  Returns allocated block/len of encrypted data.
EGPublicKey::CryptReturn
EGPublicKey::Encrypt(const void* theMsgP, unsigned long theLen) const
{
	WTRACE("EGPublicKey::Encrypt");

	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		WDBG_LM("EGPublicKey::Encrypt No data to encrypt");
		return CryptReturn(NULL,0);
	}

	try
	{
		// Allocate crypt object if needed
		AllocateCrypt();

		// Encrypt the data
		ByteQueue aQueue;
		EncryptData(aQueue, reinterpret_cast<const unsigned char*>(theMsgP), theLen);

		// Build return
		WDBG_LL("EGPublicKey::Encrypt Building return values.");
		unsigned long anOutLen = aQueue.MaxRetrieveable();
		if (anOutLen == 0)
		{
			WDBG_LM("EGPublicKey::Encrypt Encrypt failed, no data.");
			return CryptReturn(NULL, 0);
		}

		auto_ptr<unsigned char> anOutP(new unsigned char [anOutLen]);
		aQueue.Get(anOutP.get(), anOutLen);

		WDBG_LM("EGPublicKey::Encrypt Encrypted, out block len=" << anOutLen);
		return CryptReturn(anOutP.release(), anOutLen);
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("EGPublicKey::Encrypt Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return CryptReturn(NULL,0);
	}
}


// EGPublicKey::Verify
// Verifies a signature for a message using ElGamal signature verification.  Returns
// true if sig verifies, false if not.
bool
EGPublicKey::Verify(const unsigned char* theSigP, unsigned long theSigLen,
	                const void* theMsgP, unsigned long theMsgLen) const
{
	WTRACE("EGPublicKey::Verify");
	WDBG_LL("EGPublicKey::Verify, sig len=" << theSigLen << ", msg len=" << theMsgLen);

	// Sanity check
	if ((! theMsgP) || (theMsgLen == 0))
	{
		WDBG_LM("EGPublicKey::Verify No data to verify.");
		return false;
	}

	try
	{
		// Allocate sig object if needed
		AllocateSig();

		// Does given sig look ok?
		if ((! theSigP) || (theSigLen != mSigP->SignatureLength()))
		{
			WDBG_LM("EGPublicKey::Verify Sig NULL or length mismatch (sigLen=" << theSigLen << ", Expected len=" << mSigP->SignatureLength());
			return false;
		}

		// Return signature verification
		auto_ptr<HashModule> aHashP(mSigP->NewMessageAccumulator());
		aHashP->Update(reinterpret_cast<const unsigned char*>(theMsgP), theMsgLen);
		bool aRet = mSigP->Verify(aHashP.release(), theSigP);
		WDBG_LM("EGPublicKey::Verify Sig Verify=" << aRet);
		return aRet;
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("EGPublicKey::Verify Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return false;
	}
}


// EGPublicKey::Dump
// Streaming method.  Outputs key to specfied stream.  Outputs key length followed
// by each factor of the key.
void
EGPublicKey::Dump(std::ostream& os) const
{
	// Output KeyLen followed by P, G, and Y
	try
	{
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();

		Integer            p, q, g, y;
		BERSequenceDecoder aDecoder(aQueue);
		p.BERDecode(aDecoder);
		q.BERDecode(aDecoder);
		g.BERDecode(aDecoder);
		y.BERDecode(aDecoder);
		os << "(Len=" << mKeyLen << " P=" << p << " Q=" << q << " G=" << g
		   << " Y=" << y << ')';
	}
	catch (Exception& anEx)
	{
		WDBG_AH("EGPublicKey::Dump Caught CryptoLib exception: " << anEx.what());
		os << "(EGPublicKey Exception: " << anEx.what() << ')';
	}
}

#else

#pragma warning(disable: 4530)

#include <iostream>
#include "EGPrivateKey.h"
#include "EGPublicKey.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace {
	using namespace WONCryptMini;
	using WONCrypt::PublicKey;
	using WONCrypt::EGPublicKey;
	using WONCrypt::EGPrivateKey;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::EGPublicKey() :
	PublicKey()
{}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::EGPublicKey(unsigned short theLen, const unsigned char* theKeyP) :
	PublicKey()
{
	Create(theLen, theKeyP);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::EGPublicKey(const EGPublicKey& theKeyR) 
{
	Create(theKeyR.GetKeyLen(),theKeyR.GetKey());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::EGPublicKey(const EGPrivateKey& theKeyR) 
{
	this->operator=((const EGPublicKey&)theKeyR.GetPublicKey());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::~EGPublicKey(void)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey& EGPublicKey::operator=(const EGPublicKey& theKeyR)
{

	if (this != &theKeyR)  // protect vs a = a
	{
		Create(theKeyR.GetKeyLen(),theKeyR.GetKey());
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey& EGPublicKey::operator=(const EGPrivateKey& theKey)
{
	this->operator=((const EGPublicKey&)theKey.GetPublicKey());
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void EGPublicKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	PublicKey::Create(theLen, theKeyP);
	mElGamal.SetPublicKey(mKey,mKeyLen);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPublicKey::CryptReturn EGPublicKey::Encrypt(const void* theMsgP, unsigned long theLen) const
{
	if ((! theMsgP) || (theLen == 0))
	{
		return CryptReturn(NULL,0);
	}


	RawBuffer anEncrypt;
	if(!mElGamal.encrypt(theMsgP,theLen,anEncrypt))
		return CryptReturn(NULL,0);

	unsigned char *anOutP = new unsigned char[anEncrypt.length()];
	memcpy(anOutP,anEncrypt.data(),anEncrypt.length());

	return CryptReturn(anOutP,anEncrypt.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool EGPublicKey::Verify(const unsigned char* theSigP, unsigned long theSigLen,
	                const void* theMsgP, unsigned long theMsgLen) const
{
	if ((! theMsgP) || (theMsgLen == 0))
	{
		return false;
	}

	return 
		mElGamal.verify(theMsgP,theMsgLen,theSigP,theSigLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void EGPublicKey::Dump(std::ostream& os) const
{
	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


#endif
