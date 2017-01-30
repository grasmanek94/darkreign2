// Auth1Certificate

// WON Authentication Certificate for Auth Family 1.  Encapsulates the following data:
//	Auth Family (From base class)
//	Issue & Expire date/time (From base class)
//	User Info - UserId, CommunityId, TrustLevel
//	Client's Public Key (ElGamal)

// Used as system ticket for various authentication protocols within the WON
// system.


#include "common/won.h"
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "AuthFamilies.h"
#include "Auth1Certificate.h"
#include "common/WONEndian.h"

// Private namespace for using and constants
namespace {
	using WONCommon::RawBuffer;
	using WONAuth::AuthFamilyBuffer;
	using WONAuth::AuthCertificateBase;
	using WONAuth::Auth1Certificate;

	// Constants
	// Min length of a cert.  (Used in UnpackData)
	const unsigned short CERT_MINLEN =
		sizeof(unsigned long) + sizeof(unsigned long) + sizeof(unsigned short) +
		sizeof(unsigned short);
};


// ** Constructors / Destructor

// Default constructor
Auth1Certificate::Auth1Certificate(unsigned long theUserId, unsigned long theCommunityId,
                                   unsigned short theTrustLevel) :
	AuthCertificateBase()
{
	mUserId = theUserId;
	mAccessList.push_back(CommunityAccess(theCommunityId,theTrustLevel));
}


// Construct from raw representation
Auth1Certificate::Auth1Certificate(const unsigned char* theRawP, unsigned short theLen) :
	AuthCertificateBase()
{
	mAccessList.push_back(CommunityAccess(0,0));
	Unpack(theRawP, theLen);
}


// Copy Constructor
Auth1Certificate::Auth1Certificate(const Auth1Certificate& theCertR) :
	AuthCertificateBase(theCertR)
{}

	
// Destructor
Auth1Certificate::~Auth1Certificate(void)
{}


// ** Private Methods **

// Auth1Certificate::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// For PACK, returns base class total + fixed size + length of public key.  For UNPACK,
// returns base class total + fixed size.
RawBuffer::size_type
Auth1Certificate::ComputeBufSize(SizeComputeMode theMode) const
{
	WTRACE("Auth1Certificate::ComputeBufSize");
	RawBuffer::size_type aRet = AuthCertificateBase::ComputeBufSize(theMode) + CERT_MINLEN;
	if (theMode == PACK)
		aRet += mPubKey.GetKeyLen();

	return aRet;
}


// Auth1Certificate::PackData
// Packs member data into raw buffer in base class.  Returns true on success and
// false on failure.  Verifies member data and appends member data to buffer.
bool
Auth1Certificate::PackData()
{
	WTRACE("Auth1Certificate::PackData");
	if (! AuthCertificateBase::PackData()) return false;

	// UserId and CommunityId must not be zero.
	WDBG_LL("Auth1Certificate::PackData Validating...");
	if ((GetUserId() == 0) || (GetCommunityId() == 0))
	{
		WDBG_LH("Auth1Certificate::PackData UserId or CommunityId are 0, pack fails.");
		return false;
	}

	// PubKey must be valid
	if (mPubKey.GetKeyLen() <= 1)
	{
		WDBG_LH("Auth1Certificate::PackData PubKey not valid, pack fails.");
		return false;
	}

	// Append fixed length data
	WDBG_LL("Auth1Certificate::PackData Packing...");
	unsigned long tmpUserId = getLittleEndian(mUserId);
	unsigned long tmpCommunityId = getLittleEndian(GetCommunityId());
	unsigned short tmpTrustLevel = getLittleEndian(GetTrustLevel());
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpUserId),      sizeof(tmpUserId));
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpCommunityId), sizeof(tmpCommunityId));
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpTrustLevel),  sizeof(tmpTrustLevel));

	// Append PubKey length and PubKey
	WDBG_LL("Auth1Certificate::PackData Packing PublicKey.");
	unsigned short aKeyLen = mPubKey.GetKeyLen();
	unsigned short tmpKeyLen = getLittleEndian(aKeyLen);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpKeyLen), sizeof(tmpKeyLen));
	mRawBuf.append(mPubKey.GetKey(), aKeyLen);

	return true;
}


// Auth1Certificate::UnpackData
// Unpacks member data from raw buffer in base class.  Returns true on success and
// false on failure.  Verifies raw data length, reads memeber data, sets mDatalen.
bool
Auth1Certificate::UnpackData()
{
	WTRACE("Auth1Certificate::UnpackData");
	if (! AuthCertificateBase::UnpackData()) return false;

	// Get data pointer (skip header data)
	WDBG_LL("Auth1Certificate::UnpackData Unpack fixed fields.");
	const unsigned char* aDataP = mRawBuf.data() + mDataLen;

	// Read UserId
	mUserId = *(reinterpret_cast<const unsigned long*>(aDataP));
	makeLittleEndian(mUserId);
	aDataP += sizeof(mUserId);

	// Read CommunityId
	unsigned long aCommunityId = *(reinterpret_cast<const unsigned long*>(aDataP));
	SetCommunityId(aCommunityId);
	makeLittleEndian(aCommunityId);
	aDataP += sizeof(aCommunityId);

	// Read TrustLeve
	unsigned short aTrustLevel = *(reinterpret_cast<const unsigned short*>(aDataP));
	SetTrustLevel(aTrustLevel);
	makeLittleEndian(aTrustLevel);
	aDataP += sizeof(aTrustLevel);

	// Read length of Pubkey and set mDataLen.  Verify there's enough data left
	// to read PubKey
	unsigned short aKeyLen = *(reinterpret_cast<const unsigned short*>(aDataP));
	makeLittleEndian(aKeyLen);
	aDataP   += sizeof(aKeyLen);
	mDataLen += CERT_MINLEN + aKeyLen;
	if (mRawBuf.size() < mDataLen)
	{
		WDBG_LH("Auth1Certificate::UnpackData Raw buf len to short for PubKey read (" << mRawBuf.size() << " < " << (CERT_MINLEN + aKeyLen) << ')');
		return false;
	}

	// Read PubKey
	WDBG_LL("Auth1Certificate::UnpackData Unpack PubKey and set dataLen.");
	mPubKey.Create(aKeyLen, aDataP);
	return true;
}


// ** Public Methods **

// Assignment operator
Auth1Certificate&
Auth1Certificate::operator=(const Auth1Certificate& theCertR)
{
	if (this != &theCertR)  // protect vs a = a
	{
		AuthCertificateBase::operator=(theCertR);
	}
	return *this;
}


// Compare method

// Auth1Certificate::Compare
// Compares two Auth1Certificate objects.  Performs base class compare.  If equal,
// compares (in this order) userId, communityId, and PublicKey.  Note that TrustLevel
// is ignored for purposes of compare.
int
Auth1Certificate::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("Auth1Certificate::Compare");
	int aRet = AuthCertificateBase::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const Auth1Certificate* aCmpP = dynamic_cast<const Auth1Certificate*>(&theBufR);
	if (! aCmpP) return aRet;

	// Compare userid and communityid
	aRet = LenientCompare(*aCmpP);
	if (aRet != 0) return aRet;

	// Compare publicKey
	return (mPubKey <  aCmpP->mPubKey ? -1 : (mPubKey == aCmpP->mPubKey ? 0 : 1));
}

// Auth1Certificate::LenientCompare
// Compares the userid and community id of two Auth1Certificate objects.  
// Does NOT do base class compare.  Does NOT compare pub keys.  Does NOT compare trustlevels
int
Auth1Certificate::LenientCompare(const Auth1Certificate& theCertR) const
{
	// Compare userId
	int aRet = ((mUserId < theCertR.mUserId) ? -1 : (int)(mUserId - theCertR.mUserId));
	if (aRet != 0) return aRet;

	// Compare communityId
	aRet = ((GetCommunityId() < theCertR.GetCommunityId()) ? -1 : (int)(GetCommunityId() - theCertR.GetCommunityId()));
	return aRet;
}

// Auth1Certificate::GetFamily
// Returns certificate family.  Uses AuthCertv1 which is defined in base class header.
unsigned short
Auth1Certificate::GetFamily() const
{
	return WONAuth::AuthFamily1;  // From AuthFamilies.h
}


// Auth1Certificate::Dump
// Streaming method.  Outputs certificate info.  Outputs lengths and intenral memebers.
void
Auth1Certificate::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(AuthCert1 RawLen=" << mRawBuf.size() << " DataLen=" << mDataLen << endl;
	os << "  Family=" << GetFamily() << endl;
	os << "  IssueTime=" << ctime(&mIssueTime);
	os << "  ExpireTime=" << ctime(&mExpireTime);
	os << "  UserId=" << mUserId << " CommunityId=" << GetCommunityId() << " TrustLevel=" << GetTrustLevel() << endl;
	os << "  PublicKey=" << mPubKey << ')';
}
