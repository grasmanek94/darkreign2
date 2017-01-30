// SMsgDirG2KeyedBase

// BAse class that handles packing and unpacking entity key fields.  Provides
// KeyType and key fields.  KeyType controls which key fields are active as follows:

//		KeyType			Appliable Key Fields
//		--------------	---------------------
//		KT_DIRECTORY	Path
//		KT_SERVICE		Path, Name, NetAddress

// Provides PackKey() and UnpackKey() methods to pack/unpack the key.  These methods
// pack/unpack appropriate key fields based upon the KeyType.  These methods should
// be called be derived class Pack/Unpack at the appropriate point to pack/unpack
// key fields if needed.

// Note that this base class IS NOT derived from SmallMessage.  It is meant to be
// multiply inherited by classes already dervied fromn SmallMessage to add the keyed
// atributes to a message.


#include "common/won.h"
#include "msg/TMessage.h"
#include "SMsgDirG2KeyedBase.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::BaseMessage;
	using WONMsg::SMsgDirG2KeyedBase;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2KeyedBase::SMsgDirG2KeyedBase(KeyType theType) :
	mKeyType(theType),
	mPath(),
	mName(),
	mNetAddress()
{}



// Copy ctor
SMsgDirG2KeyedBase::SMsgDirG2KeyedBase(const SMsgDirG2KeyedBase& theMsgR) :
	mKeyType(theMsgR.mKeyType),
	mPath(theMsgR.mPath),
	mName(theMsgR.mName),
	mNetAddress(theMsgR.mNetAddress)
{}


// Destructor
SMsgDirG2KeyedBase::~SMsgDirG2KeyedBase(void)
{}


// ** Protected Methods **

void
SMsgDirG2KeyedBase::PackKey(BaseMessage& theMsgR)
{
	WTRACE("SMsgDirG2KeyedBase::PackKey");
	WDBG_LL("SMsgDirG2KeyedBase::PackKey KeyType=" << mKeyType);

	if (mKeyType == KT_SERVICE)
	{
		WDBG_LL("SMsgDirG2KeyedBase::PackKey Path=" << mPath << " Name=" << mName << " NetAddrSize=" << mNetAddress.size());
		theMsgR.Append_PW_STRING(mPath);
		theMsgR.Append_PW_STRING(mName);
		unsigned char aLen = mNetAddress.size();
		theMsgR.AppendByte(aLen);
		if (aLen > 0)
			theMsgR.AppendBytes(aLen, mNetAddress.data());
	}
	else // KT_DIRECTORY
	{
		WDBG_LL("SMsgDirG2KeyedBase::PackKey Path=" << mPath);
		theMsgR.Append_PW_STRING(mPath);
	}
}


void
SMsgDirG2KeyedBase::UnpackKey(BaseMessage& theMsgR)
{
	WTRACE("SMsgDirG2KeyedBase::UnpackKey");
	WDBG_LL("SMsgDirG2KeyedBase::UnpackKey KeyType=" << mKeyType);

	if (mKeyType == KT_SERVICE)
	{
		theMsgR.ReadWString(mPath);
		theMsgR.ReadWString(mName);
		unsigned char aLen = theMsgR.ReadByte();
		WDBG_LL("SMsgDirG2KeyedBase::UnpackKey Read path=" << mPath << " name=" << mName << " NetAddrSize=" << aLen);
		if (aLen > 0)
			mNetAddress.assign(reinterpret_cast<const unsigned char*>(theMsgR.ReadBytes(aLen)), aLen);
	}
	else // KT_DIRECTORY
	{
		theMsgR.ReadWString(mPath);
		WDBG_LL("SMsgDirG2KeyedBase::UnpackKey Read path=" << mPath);
	}

	// Strip trailing L'/' from path if needed
	wstring::size_type aLastPos = mPath.size();
	if ((aLastPos > 1) && (mPath.at(--aLastPos) == L'/'))
		mPath.erase(aLastPos); 
}


// ** Public Methods **

// Assignment operator
SMsgDirG2KeyedBase&
SMsgDirG2KeyedBase::operator=(const SMsgDirG2KeyedBase& theDataR)
{
	mKeyType    = theDataR.mKeyType;
	mPath       = theDataR.mPath;
	mName       = theDataR.mName;
	mNetAddress = theDataR.mNetAddress;
	return *this;
}
