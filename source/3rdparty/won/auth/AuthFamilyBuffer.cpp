// AuthFamilyBuffer

// Abstract base class for a binary buffer for Auth Family data.  Buffer is of
// arbitrary length with the following header:
//		Auth Family ID   (ushort - 2 bytes)
//		Issue Date/Time  (time_t - 4 bytes)
//		Expire Date/Time (time_t - 4 bytes)
// The ID identifies the family of the buffer and the Issue/Expire date/time
// identify the lifespan of the buffer.

// AuthFamilyBuffer supports copy, assignment, a full range of logical operators,
// and the stream output operator.  Dervied classes will get the logical and stream
// output operators for free if they override the Compare() and Dump() methods.

// AuthFamilyBuffer has serveral pure virtual methods that must be overridden by
// derived classes.


#include "common/won.h"
#include "crypt/PrivateKey.h"
#include "crypt/PublicKey.h"
#include "AuthFamilyBuffer.h"
#include "common/WONEndian.h"

// Private namespace for using and constants
namespace {
	using WONAuth::AuthFamilyBuffer;
	using WONCrypt::PrivateKey;
	using WONCrypt::PublicKey;

	// Min length of buffer.  (Used in Unpack)
	const unsigned short BUF_MINLEN = sizeof(unsigned short) + sizeof(time_t) + sizeof(time_t);
};


// ** Constructors / Destructor

// Default constructor
AuthFamilyBuffer::AuthFamilyBuffer() :
	mRawBuf(),
	mIssueTime(0),
	mExpireTime(0),
	mDataLen(0)
{}


// Copy Constructor
AuthFamilyBuffer::AuthFamilyBuffer(const AuthFamilyBuffer& theBufR) :
	mRawBuf(theBufR.mRawBuf),
	mIssueTime(theBufR.mIssueTime),
	mExpireTime(theBufR.mExpireTime),
	mDataLen(theBufR.mDataLen)
{}

	
// Destructor
AuthFamilyBuffer::~AuthFamilyBuffer(void)
{}


// ** Protected Methods **

// AuthFamilyBuffer::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// AuthFamilyBuffer has fixed data; required size is always fixed.
WONCommon::RawBuffer::size_type
AuthFamilyBuffer::ComputeBufSize(SizeComputeMode) const
{
	return BUF_MINLEN;
}


// AuthFamilyBuffer::PackData
// This is a place holder so derived classes can call the base class method.
bool
AuthFamilyBuffer::PackData()
{
	return true;
}


// AuthFamilyBuffer::UnpackData
// This is a place holder so derived classes can call the base class method.
bool
AuthFamilyBuffer::UnpackData()
{
	return true;
}


// AuthFamilyBuffer::Sign
// Generates and appends signature to the raw buffer using specified Private Key.
// Returns true on success and false on failure.  Buffer must be valid.  This
// method catches any exceptions thrown by the crypt lib.  A caught exception will
// imply failure.
bool
AuthFamilyBuffer::Sign(const PrivateKey& theKeyR)
{
	WTRACE("AuthFamilyBuffer::Sign");
	WDBG_LM("AuthFamilyBuffer::Sign Raw buf size=" << mRawBuf.size());
	if (! IsValid()) 
	{
		WDBG_LH("AuthFamilyBuffer::Sign Not valid, sig fails!");
		return false;
	}

	// Set the data length and generate signature
	mDataLen = mRawBuf.size();
	PrivateKey::CryptReturn aSig(theKeyR.Sign(mRawBuf.data(), mDataLen));

	// Append sig if sig is valid
	if (aSig.first)
	{
		WDBG_LL("AuthFamilyBuffer::Sign Appending sig.");
		mRawBuf.append(aSig.first, aSig.second);
		delete aSig.first;  aSig.first = NULL;
	}

	WDBG_LM("AuthFamilyBuffer::Sign Sig result=" << (mRawBuf.size() > mDataLen));
	return (mRawBuf.size() > mDataLen);
}


// ** Public Methods **

// Assignment operator
AuthFamilyBuffer&
AuthFamilyBuffer::operator=(const AuthFamilyBuffer& theBufR)
{
	if (this != &theBufR)  // protect vs a = a
	{
		mRawBuf     = theBufR.mRawBuf;
		mIssueTime  = theBufR.mIssueTime;
		mExpireTime = theBufR.mExpireTime;
		mDataLen    = theBufR.mDataLen;
	}
	return *this;
}


// AuthFamilyBuffer::Compare
// Compares two AuthFamilyBuffer objects.  Compares family, expireTime, then issueTime.
// Note: since the times are ulongs, we can't safely use a difference for the compare.
int
AuthFamilyBuffer::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("AuthFamilyBuffer::Compare");
	int aRet = static_cast<int>(GetFamily()) - static_cast<int>(theBufR.GetFamily());
	if (aRet != 0) return aRet;

	aRet = mExpireTime - theBufR.mExpireTime;
	if (aRet != 0) return aRet;

	return mIssueTime - theBufR.mIssueTime;
}


// AuthFamilyBuffer::Pack
// Generates raw form of the buffer.  Any existing raw form is discared.
// Returns true on success and false on failure.  Appends the family and lifespan
// data, calls PackData(), and Sign() to generate raw form.
bool
AuthFamilyBuffer::Pack(const PrivateKey& theKeyR)
{
	WTRACE("AuthFamilyBuffer::Pack");
	WDBG_LM("AuthFamilyBuffer::Pack Key=" << theKeyR);
	Invalidate();

	// ExpireTime must be greater than IssueTime
	if (mExpireTime <= mIssueTime)
	{
		WDBG_LH("AuthFamilyBuffer::Pack ExpireTime <= IssueTime, pack fails.");
		return false;
	}

	// Preallocate buffer for performance
	mRawBuf.reserve(ComputeBufSize(PACK));

	// Add the family and lifespan
	unsigned short aFamily = GetFamily();
	unsigned short tmpFamily = getLittleEndian(aFamily);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpFamily),     sizeof(tmpFamily));
	time_t tmpIssueTime = getLittleEndian(mIssueTime);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpIssueTime),  sizeof(tmpIssueTime));
	time_t tmpExpireTime = getLittleEndian(mExpireTime);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpExpireTime), sizeof(tmpExpireTime));

	// Let derived class pack data
	mDataLen = BUF_MINLEN;
	if (! PackData())
	{
		WDBG_LH("AuthFamilyBuffer::Pack PackData() failed!");
		Invalidate();
	}

	// Generate/append signature
	else if (! Sign(theKeyR))
	{
		WDBG_LH("AuthFamilyBuffer::Pack Sign() failed!");
		Invalidate();
	}

	return IsValid();
}


// AuthFamilyBuffer::Unpack
// Builds buffer from specified raw buffer.  Any existing raw form is discared.
// Returns true on success and false on failure.  Verifies first two byes of buffer
// are expected family.  Extracts lifespan data.  Calls calls UnpackData() and
// validates mDataLen.
bool
AuthFamilyBuffer::Unpack(const unsigned char* theRawP, unsigned short theLen)
{
	WTRACE("AuthFamilyBuffer::Unpack");
	WDBG_LM("AuthFamilyBuffer::Unpack raw len=" << theLen);
	Invalidate();

	// Verify params
	if ((! theRawP) || (theLen < ComputeBufSize(UNPACK)))
	{
		WDBG_LH("AuthFamilyBuffer::Unpack NULL buffer or raw buf len less than min len (" << theLen << " <= " << BUF_MINLEN << ')');
		return false;
	}

	// Verify family
	unsigned short aFamily = *(reinterpret_cast<const unsigned short*>(theRawP));
	makeLittleEndian(aFamily);
	if (aFamily != GetFamily())
	{
		WDBG_LH("AuthFamilyBuffer::Unpack Family invalid!, expected=" << GetFamily() << " raw=" << aFamily);
		return false;
	}

	// Add raw data to buffer and extract lifespan info
	mRawBuf.append(theRawP, theLen);
	theRawP += sizeof(unsigned short);
	mIssueTime  = *(reinterpret_cast<const time_t*>(theRawP));
	theRawP += sizeof(time_t);
	mExpireTime = *(reinterpret_cast<const time_t*>(theRawP));

	// Let derived class unpack its data
	mDataLen = BUF_MINLEN;
	if (! UnpackData())
	{
		WDBG_LH("AuthFamilyBuffer::Unpack UnpackData() failed!");
		Invalidate();
	}

	// Make sure dataLen makes sense
	else if ((mDataLen < BUF_MINLEN) || (mDataLen >= mRawBuf.size()))
	{
		WDBG_LH("AuthFamilyBuffer::Unpack Invalid DataLen=" << mDataLen << ", RawSize=" << mRawBuf.size());
		Invalidate();
	}

	return IsValid();
}


// AuthFamilyBuffer::Verify
// Verifies signature block of the raw buffer using specified Public Key.  Returns
// true on success and false on failure.  Buffer must be valid.  This method
// catches any exceptions thrown by the crypt lib.  A caught exception will imply
// failure.
bool
AuthFamilyBuffer::Verify(const PublicKey& theKeyR) const
{
	WTRACE("AuthFamilyBuffer::Verify");
	WDBG_LM("AuthFamilyBuffer::Verify Raw buf size=" << mRawBuf.size());
	if (! IsValid()) 
	{
		WDBG_LH("AuthFamilyBuffer::Verify Not valid, verify fails!");
		return false;
	}

	// Verify the sig with specified key
	WDBG_LL("AuthFamilyBuffer::Verify Verifying data.");
	const unsigned char* aSigP   = mRawBuf.data() + mDataLen;
	unsigned short       aSigLen = mRawBuf.size()  - mDataLen;
	return theKeyR.Verify(aSigP, aSigLen, mRawBuf.data(), mDataLen);
}


// AuthFamilyBuffer::Dump
// Streaming method.  Outputs buffer info.  Outputs family and lengths.
void
AuthFamilyBuffer::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(Family=" << GetFamily() << " RawLen=" << mRawBuf.size()
	   << " DataLen=" << mDataLen << ')';
}
