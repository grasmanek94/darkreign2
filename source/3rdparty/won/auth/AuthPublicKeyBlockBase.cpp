// AuthPublicKeyBlockBase

// Abstract base class for WON Auth Server Public Key Blocks.  Defines common
// functionality for AuthServer Public Key Blocks.  Note that AuthPublicKeyBlockBase
// does not override the pure virtual GetFamily() method of its base class.  This
// must be overridden in derived classes.


#include "common/won.h"
#include "AuthPublicKeyBlockBase.h"
#include "common/WONEndian.h"

// Private namespace for using and constants
namespace {
	using WONAuth::AuthFamilyBuffer;
	using WONAuth::AuthPublicKeyBlockBase;

	// Constants
	// Min length of a block base.  (Used in UnpackData)
	const unsigned short BLOCKBASE_MINLEN = sizeof(unsigned short);
};


// ** Constructors / Destructor

// Default constructor
AuthPublicKeyBlockBase::AuthPublicKeyBlockBase(unsigned short theBlockId) :
	AuthFamilyBuffer(),
	mBlockId(theBlockId)
{}


// Copy Constructor
AuthPublicKeyBlockBase::AuthPublicKeyBlockBase(const AuthPublicKeyBlockBase& theBlockR) :
	AuthFamilyBuffer(theBlockR),
	mBlockId(theBlockR.mBlockId)
{}

	
// Destructor
AuthPublicKeyBlockBase::~AuthPublicKeyBlockBase(void)
{}


// ** Protected Methods **

// AuthPublicKeyBlockBase::ComputeBufSize
// Determines number of bytes required in buffer for PACK and UNPACK operations.
// AuthPublicKeyBlockBase has fixed data; required size is fixed + base class size.
WONCommon::RawBuffer::size_type
AuthPublicKeyBlockBase::ComputeBufSize(SizeComputeMode theMode) const
{
	WTRACE("AuthPublicKeyBlockBase::ComputeBufSize");
	return (AuthFamilyBuffer::ComputeBufSize(theMode) + BLOCKBASE_MINLEN);
}


// AuthPublicKeyBlockBase::PackData
// Packs member data into raw buffer in base class.  Returns true on success and
// false on failure.  Appends member data to buffer.
bool
AuthPublicKeyBlockBase::PackData()
{
	WTRACE("AuthPublicKeyBlockBase::PackData");

	// Append block id
	WDBG_LL("AuthPublicKeyBlockBase::PackData Pack BlockId.");
	unsigned short tmpBlockId = getLittleEndian(mBlockId);
	mRawBuf.append(reinterpret_cast<unsigned char*>(&tmpBlockId), sizeof(tmpBlockId));
	return true;
}


// AuthPublicKeyBlockBase::UnpackData
// Unpacks member data from raw buffer in base class.  Returns true on success and
// false on failure.  Reads member data, updates mDatalen.
bool
AuthPublicKeyBlockBase::UnpackData()
{
	WTRACE("AuthPublicKeyBlockBase::UnpackData");

	// Read BlockId
	WDBG_LL("AuthPublicKeyBlockBase::UnpackData Unpack BlockId.");
	mBlockId = *(reinterpret_cast<const unsigned short*>(mRawBuf.data() + mDataLen));
	makeLittleEndian(mBlockId);
	mDataLen += BLOCKBASE_MINLEN;
	return true;
}


// ** Public Methods **

// Assignment operator
AuthPublicKeyBlockBase&
AuthPublicKeyBlockBase::operator=(const AuthPublicKeyBlockBase& theBlockR)
{
	if (this != &theBlockR)  // protect vs a = a
	{
		AuthFamilyBuffer::operator=(theBlockR);
		mBlockId = theBlockR.mBlockId;
	}
	return *this;
}


// Compare method

// AuthPublicKeyBlockBase::Compare
// Compares two AuthPublicKeyBlockBase objects.  Performs base class compare.  If equal,
// compares the block ids.
int
AuthPublicKeyBlockBase::Compare(const AuthFamilyBuffer& theBufR) const
{
	WTRACE("AuthPublicKeyBlockBase::Compare");
	int aRet = AuthFamilyBuffer::Compare(theBufR);
	if (aRet != 0) return aRet;

	// If not of same type, have to PUNT
	const AuthPublicKeyBlockBase* aCmpP = dynamic_cast<const AuthPublicKeyBlockBase*>(&theBufR);
	if (! aCmpP) return aRet;

	// Compare block IDs
	return (static_cast<int>(mBlockId) - static_cast<int>(aCmpP->mBlockId));
}


// AuthPublicKeyBlockBase::Dump
// Streaming method.  Outputs block info.  Outputs lengths, family, and blockId.
void
AuthPublicKeyBlockBase::Dump(std::ostream& os) const
{
	// Output buf and data length
	os << "(RawLen=" << mRawBuf.size() << " DataLen=" << mDataLen
	   << "  Family=" << GetFamily() << " BlockId=" << mBlockId << ')';
}
