// EGPrivateKey

// Class that encapsulates an Elgamal Private key used for encryption and signatures.
// Length of each portion of the key in bytes can be defined on creation.

// Note that EGPrivateKey also generates the corresponding EGPublicKey.

// NOTE: Binary representation of EGPrivateKey
// The binary rep EGPrivateKey is stored using the binary rep of the ElGamalSigner class
// from the Crypto++ library.  This representation is a concatentation of all of
// the factors (p, q, g, y) needed to perform the signature verification. Unfortunately,
// the ElGamalDecryptor class in the Crypto++ library uses a subset of this
// representation for its work; it only expects/stores p, g, y.  This means that the
// binary repsentation is not compatible for the Crypto++ private key ElGamal classes.
// There no way to build either class from the other one and no one to access the factors
// in either class.

// In order to solve this problem, the ElGamalDecryptor is built manually from
// the binary representation.  This is a HACK and is subject to problems if the
// underlying implementation in Crypto++ is ever altered.


// This is WONCrypt source
#define _WONCRYPT_SOURCEFILE
#include "EGPrivateKey.h"
#include "common/WONEndian.h"


#ifndef _MINI_CRYPT

#include "common/won.h"
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
#include "EGPublicKey.h"
#include "EGPrivateKey.h"

#ifndef _WONCRYPT_NOEXCEPTIONS
#include "CryptException.h"
#include "common/WONExceptCodes.h"
#endif


// private namespace for using and constants
namespace {
	using namespace CryptoPP;
	using WONCrypt::PublicKey;
	using WONCrypt::PrivateKey;
	using WONCrypt::EGPrivateKey;

	// Constants
	const int BITS_PER_BYTE = 8;
};


// ** Constructors / Destructor

// Default constructor
EGPrivateKey::EGPrivateKey() :
	PrivateKey(),
	mPubKeyP(NULL),
	mCryptP(NULL),
	mSigP(NULL)
{}


// Construct by generating new key of specified length.
	EGPrivateKey::EGPrivateKey(unsigned short theLen) throw(WONCrypt::CryptException) :
	PrivateKey(),
	mPubKeyP(NULL),
	mCryptP(NULL),
	mSigP(NULL)
{
	Create(theLen);
}


// Construct from existing key
EGPrivateKey::EGPrivateKey(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException) :
	PrivateKey(),
	mPubKeyP(NULL),
	mCryptP(NULL),
	mSigP(NULL)
{
	Create(theLen, theKeyP);
}


// Copy Constructor
EGPrivateKey::EGPrivateKey(const EGPrivateKey& theKeyR) :
	PrivateKey(theKeyR),
	mPubKeyP(NULL),
	mCryptP(NULL),
	mSigP(NULL)
{}

	
// Destructor
EGPrivateKey::~EGPrivateKey(void)
{
	ClearLocal();
}


// ** Private Methods **

// EGPrivateKey::ClearLocal()
// Deletes and NULLs allocated members.
void
EGPrivateKey::ClearLocal()
{
	delete mPubKeyP;  mPubKeyP = NULL;
	delete mCryptP;   mCryptP  = NULL;
	delete mSigP;     mSigP    = NULL;
}


// EGPrivateKey::AllocateCrypt
// Allocates the mCryptP member if needed.  Manually decodes binary rep into factors
// and feeds appropriate factor to the ElGamalDecryptor class from Crypto++.
void
EGPrivateKey::AllocateCrypt() const
{
	WTRACE("EGPrivateKey::AllocateCrypt");
	if (! mCryptP)
	{
		WDBG_LL("EGPrivateKey::AllocateCrypt Allocating crypt object.");
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();

		// *HACK*
		// Manually build the ElGamalDecryptor object.  See notes at
		// top of this file.
		Integer p, q, g, y, x;
		BERSequenceDecoder seq(aQueue);
		p.BERDecode(seq);
		q.BERDecode(seq);
		g.BERDecode(seq);
		y.BERDecode(seq);
		x.BERDecode(seq);
		mCryptP = new ElGamalDecryptor(p, g, y, x);
	}
}


// EGPrivateKey::AllocateSig
// Allocates the mSigP member if needed.  Feeds binary rep of key into the
// ElGamalSigner class from Crypto++.
void
EGPrivateKey::AllocateSig() const
{
	WTRACE("EGPrivateKey::AllocateSig");
	if (! mSigP)
	{
		WDBG_LL("EGPrivateKey::AllocateSig Allocating crypt object.");
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();
		mSigP = new ElGamalSigner(aQueue);
	}
}


// EGPrivateKey::DecryptBlock
// Decrypts a block of specified length into the specified queue.  This method is
// called by the DecryptData() method for each block.  Note that theLen specified a max
// size for the decrypted block and is only use to allocate a buffer.  Also note that
// mCryptP must be valid before this method is called.
void
EGPrivateKey::DecryptBlock(BufferedTransformation& aQueue, const unsigned char* theBlockP,
                           unsigned long theLen) const
{
	WTRACE("EGPrivateKey::DecryptBlock");
	WDBG_LL("EGPrivateKey::DecryptBlock, len=" << theLen);
	auto_ptr<unsigned char> aBuf(new unsigned char[theLen]);
	unsigned int aLen = mCryptP->Decrypt(theBlockP, aBuf.get());
	aQueue.Put(aBuf.get(), aLen);
}


// EGPrivateKey::DecryptData
// Decrypts a block of specified length into the specified	queue.  Determines the
// number of individual blocks to be decrypted from the first 2 bytes of theMsgP.
// Decrypts each block into the queue in turn by calling DecryptBlock().  Note that
// mCryptP must be valid before this method is called.  If block to be decrypted is
// invalid, returns false.
bool
EGPrivateKey::DecryptData(BufferedTransformation& aQueue, const unsigned char* theMsgP,
                          unsigned long theLen) const
{
	WTRACE("EGPrivateKey::DecryptData");
	WDBG_LL("EGPrivateKey::DecryptData, len=" << theLen);

	// Determine number of blocks and sanity check
	unsigned long aBlockLen = mCryptP->CipherTextLength();
	unsigned long aNumBlock = *(reinterpret_cast<const unsigned long*>(theMsgP));
	makeLittleEndian(aNumBlock);
	theLen  -= sizeof(aNumBlock);
	if (theLen != (aNumBlock * aBlockLen))
	{
		WDBG_LL("EGPrivateKey::DecryptData Bad block length, expecting " << aNumBlock * aBlockLen << " given " << theLen);
		return false;
	}

	// Decrypt the data, one block at a time
	theMsgP += sizeof(aNumBlock);
	while (theLen > 0)
	{
		DecryptBlock(aQueue, theMsgP, aBlockLen);
		theMsgP += aBlockLen;
		theLen  -= aBlockLen;
	}

	// Close the queue
	aQueue.Close();
	return true;
}


// ** Public Methods **

// Assignment operator
EGPrivateKey&
EGPrivateKey::operator=(const EGPrivateKey& theKeyR)
{
	if (this != &theKeyR)  // protect vs a = a
	{
		ClearLocal();
		PrivateKey::operator=(theKeyR);
	}
	return *this;
}


// EGPrivateKey::GetPublicKey
// Generates EGPublicKey for this private key if needed.  Returns the EGPublicKey.
const PublicKey&
EGPrivateKey::GetPublicKey() const throw(WONCrypt::CryptException)
{
	if (! mPubKeyP) mPubKeyP = new EGPublicKey(*this);
	return *mPubKeyP;
}


// EGPrivateKey::Create(len)
// Generates a new ElGamal public/private key pair using the specified length in bytes
// for the factors.  (If theLen is zero, default length is used.)  Old key (if any) is
// replaced.  Uses the ElGamalSigner class from Crypto++ to generate the key.
void
EGPrivateKey::Create(unsigned short theLen) throw(WONCrypt::CryptException)
{
	// Validate length and delete old PubKey (if any)
	WTRACE("EGPrivateKey::Create(theLen)");
	if (theLen == 0) theLen = KEYLEN_DEF;
	ClearLocal();

	try
	{
		WDBG_LL("EGPrivateKey::Create(theLen) Generating new key, len=" << theLen);
		ElGamalSigner aKey(Randomizer::GetPool(), theLen * BITS_PER_BYTE);
		ByteQueue     aQueue;
		aKey.DEREncode(aQueue);
		aQueue.Close();

		AllocKeyBuf(aQueue.MaxRetrieveable());
		aQueue.Get(mKey, mKeyLen);
	}
	catch (Exception& anEx)
	{
		WDBG_AH("EGPrivateKey::Create(theLen) Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		throw WONCrypt::CryptException(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#else
		Invalidate();
#endif
	}

	WDBG_LM("EGPrivateKey::Create(theLen) Created. Key=" << *this);
}


// EGPrivateKey::Create(len, data)
// Copies existing key.  Clears allocated members.  Old key (if any) is replaced.
void
EGPrivateKey::Create(unsigned short theLen, const unsigned char* theKeyP) throw(WONCrypt::CryptException)
{
	WTRACE("EGPrivateKey::Create(theLen, theKey)");
	ClearLocal();
	PrivateKey::Create(theLen, theKeyP);
	WDBG_LM("EGPrivateKey::Create(theLen, theKey) Created. Key=" << *this);
}


// EGPrivateKey::Decrypt
// Decrypts a block of data of the specified length using ElGamal decryption for this
// key.  Returns allocated block/len of encrypted data.
EGPrivateKey::CryptReturn
EGPrivateKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen) const
{
	WTRACE("EGPrivateKey::Decrypt");

	// Sanity check - must have a buffer and size must be at least sizeof a ushort
	// so number of blocks to decrypt can be read.
	if ((! theMsgP) || (theLen <= sizeof(unsigned long)))
	{
		WDBG_LM("EGPrivateKey::Decrypt No data to decrypt");
		return CryptReturn(NULL,0);
	}

	try
	{
		// Allocate crypt object if needed
		AllocateCrypt();

		// Encrypt the data
		ByteQueue aQueue;
		if (! DecryptData(aQueue, theMsgP, theLen))
		{
			WDBG_LM("EGPrivateKey::Decrypt Decrypt failed.");
			return CryptReturn(NULL, 0);
		}

		// Build return
		WDBG_LL("EGPrivateKey::Decrypt Building return values.");
		unsigned long anOutLen = aQueue.MaxRetrieveable();
		if (anOutLen == 0)
		{
			WDBG_LM("EGPrivateKey::Decrypt Decrypt failed, no data.");
			return CryptReturn(NULL, 0);
		}

		auto_ptr<unsigned char> anOutP(new unsigned char [anOutLen]);
		aQueue.Get(anOutP.get(), anOutLen);

		WDBG_LM("EGPrivateKey::Decrypt Decrypted, out block len=" << anOutLen);
		return CryptReturn(anOutP.release(), anOutLen);
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("EGPrivateKey::Decrypt Caught CryptoLib exception: " << anEx.what());
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return CryptReturn(NULL,0);
	}
}


// EGPrivateKey::Sign
// Generates a signature for block of data of the specified length using ElGamal
// signature gneration for this key.  Returns allocated block/len of signature.
EGPrivateKey::CryptReturn
EGPrivateKey::Sign(const void* theMsgP, unsigned long theLen) const
{
	WTRACE("EGPrivateKey::Sign");
	WDBG_LL("EGPublicKey::Sign, input len=" << theLen);
	CryptReturn aRet(NULL, 0);

	// Sanity check
	if ((! theMsgP) || (theLen == 0))
	{
		WDBG_LM("EGPrivateKey::Sign No data to sign.");
		return aRet;
	}

	try
	{
		// Allocate sig object if needed
		AllocateSig();

		// Build return
		aRet.second = mSigP->SignatureLength();
		aRet.first = new unsigned char [aRet.second];

		// Generate the sig into the return
		auto_ptr<HashModule> aHashP(mSigP->NewMessageAccumulator());
		aHashP->Update(reinterpret_cast<const unsigned char*>(theMsgP), theLen);
		mSigP->Sign(Randomizer::GetPool(), aHashP.release(), aRet.first);
		return aRet;
	}
	catch (Exception& anEx)
	{
		// Little trick here, construct a CryptException to auto log the failure!
		WDBG_AH("EGPrivateKey::Sign Caught CryptoLib exception: " << anEx.what());
		delete aRet.first;
#ifndef _WONCRYPT_NOEXCEPTIONS
		WONCrypt::CryptException aLog(WONCommon::ExCryptoLib, __LINE__, __FILE__, anEx.what());
#endif
		return CryptReturn(NULL,0);
	}
}


// EGPublicKey::Dump
// Streaming method.  Outputs key to specfied stream.  Outputs key length followed
// by each factor of the key.
void
EGPrivateKey::Dump(std::ostream& os) const
{
	// Output KeyLen followed by P, G, Y and X
	try
	{
		ByteQueue aQueue;
		aQueue.Put(mKey, mKeyLen);
		aQueue.Close();

		Integer            p, q, g, y, x;
		BERSequenceDecoder aDecoder(aQueue);
		p.BERDecode(aDecoder);
		q.BERDecode(aDecoder);
		g.BERDecode(aDecoder);
		y.BERDecode(aDecoder);
		x.BERDecode(aDecoder);

		os << "(Len=" << mKeyLen << " P=" << p << " Q=" << q << " G=" << g
		   << " Y=" << y << " X=" << x << ')';
	}
	catch (Exception& anEx)
	{
		WDBG_AH("EGPrivateKey::Dump Caught CryptoLib exception: " << anEx.what());
		os << "(EGPrivateKey Exception: " << anEx.what() << ')';
	}
}


#else

#pragma warning(disable: 4530)

#include <iostream>
#include "EGPublicKey.h"
#include "EGPrivateKey.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace 
{
	using namespace WONCryptMini;
	using WONCrypt::PublicKey;
	using WONCrypt::PrivateKey;
	using WONCrypt::EGPrivateKey;

	// Constants
	const int BITS_PER_BYTE = 8;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::EGPrivateKey() :
	PrivateKey()
{}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::EGPrivateKey(unsigned short theLen)  :
	PrivateKey()
{
	Create(theLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::EGPrivateKey(unsigned short theLen, const unsigned char* theKeyP) :
	PrivateKey()
{
	Create(theLen, theKeyP);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::EGPrivateKey(const EGPrivateKey& theKeyR) 
{
	Create(theKeyR.GetKeyLen(),theKeyR.GetKey());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::~EGPrivateKey(void)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey& EGPrivateKey::operator=(const EGPrivateKey& theKeyR)
{
	if (this != &theKeyR)  // protect vs a = a
	{
		Create(theKeyR.GetKeyLen(),theKeyR.GetKey());	
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const PublicKey& EGPrivateKey::GetPublicKey() const 
{
	RawBuffer aBuf;
	mElGamal.GetRawPublicKey(aBuf);
	mPubKey.Create(aBuf.length(),aBuf.data());
	return mPubKey;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void EGPrivateKey::Create(unsigned short theLen) 
{
	Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void EGPrivateKey::Create(unsigned short theLen, const unsigned char* theKeyP) 
{
	PrivateKey::Create(theLen, theKeyP);
	mElGamal.SetPrivateKey(mKey,mKeyLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::CryptReturn EGPrivateKey::Decrypt(const unsigned char* theMsgP, unsigned long theLen) const
{
	if ((! theMsgP) || (theLen <= sizeof(unsigned long)))
	{
		return CryptReturn(NULL,0);
	}


	RawBuffer aDecrypt;
	if(!mElGamal.decrypt(theMsgP,theLen,aDecrypt))
		return CryptReturn(NULL,0);

	unsigned char *anOutP = new unsigned char[aDecrypt.length()];
	memcpy(anOutP,aDecrypt.data(),aDecrypt.length());

	return CryptReturn(anOutP,aDecrypt.length());
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

EGPrivateKey::CryptReturn EGPrivateKey::Sign(const void* theMsgP, unsigned long theLen) const
{
	if ((! theMsgP) || (theLen == 0))
	{
		return CryptReturn(NULL,0);
	}


	RawBuffer aSignature;
	if(!mElGamal.sign(theMsgP,theLen,aSignature))
		return CryptReturn(NULL,0);

	unsigned char *anOutP = new unsigned char[aSignature.length()];
	memcpy(anOutP,aSignature.data(),aSignature.length());

	return CryptReturn(anOutP,aSignature.length());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void EGPrivateKey::Dump(std::ostream& os) const
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#endif
