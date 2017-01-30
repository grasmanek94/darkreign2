// SMsgDirG2ModifyService.h

// DirectoryServer modify service message.  Modifies attributes of a service,
// including its name or net address (key fields).


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2ModifyService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateExtendBase;
	using WONMsg::SMsgDirG2ModifyService;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2ModifyService::SMsgDirG2ModifyService(bool isExtended) :
	SMsgDirG2UpdateExtendBase(KT_SERVICE, isExtended, isExtended),
	mEntityFlags(0),
	mNewName(),
	mNewNetAddress(),
	mNewDisplayName(),
	mNewLifespan(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(IsExtended() ? WONMsg::DirG2ModifyServiceEx : WONMsg::DirG2ModifyService);
}


// SmallMessage ctor
SMsgDirG2ModifyService::SMsgDirG2ModifyService(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(0),
	mNewName(),
	mNewNetAddress(),
	mNewDisplayName(),
	mNewLifespan(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2ModifyService::SMsgDirG2ModifyService(const SMsgDirG2ModifyService& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(theMsgR.mEntityFlags),
	mNewName(theMsgR.mNewName),
	mNewNetAddress(theMsgR.mNewNetAddress),
	mNewDisplayName(theMsgR.mNewDisplayName),
	mNewLifespan(theMsgR.mNewLifespan)
{}


// Destructor
SMsgDirG2ModifyService::~SMsgDirG2ModifyService(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2ModifyService&
SMsgDirG2ModifyService::operator=(const SMsgDirG2ModifyService& theMsgR)
{
	SMsgDirG2UpdateExtendBase::operator=(theMsgR);
	mEntityFlags    = theMsgR.mEntityFlags;
	mNewName        = theMsgR.mNewName;
	mNewNetAddress  = theMsgR.mNewNetAddress;
	mNewDisplayName = theMsgR.mNewDisplayName;
	mNewLifespan    = theMsgR.mNewLifespan;
	return *this;
}


// SMsgDirG2ModifyService::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2ModifyService::Pack(void)
{
	WTRACE("SMsgDirG2ModifyService::Pack");
	SetKeyType(KT_SERVICE);
	SetServiceType(WONMsg::SmallDirServerG2);
	if (IsExtended())
	{
		SetMessageType(WONMsg::DirG2ModifyServiceEx);
		SetExtended(true, true);
	}
	else
		SetMessageType(WONMsg::DirG2ModifyService);

	SMsgDirG2UpdateExtendBase::Pack();
	AppendByte(mEntityFlags);
	PackKey(*this);
	Append_PW_STRING(mNewName);

	unsigned char aLen = mNewNetAddress.size();
	AppendByte(aLen);
	if (aLen > 0)
		AppendBytes(aLen, mNewNetAddress.data());

	Append_PW_STRING(mNewDisplayName);
	AppendLong(mNewLifespan);
	PackExtended();
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2ModifyService::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2ModifyService::Unpack(void)
{
	WTRACE("SMsgDirG2ModifyService::Unpack");
	SetKeyType(KT_SERVICE);
	SetExtended((GetMessageType() != WONMsg::DirG2ModifyService), (GetMessageType() == WONMsg::DirG2ModifyServiceEx));
	SMsgDirG2UpdateExtendBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2ModifyService) &&
	     (GetMessageType() != WONMsg::DirG2ModifyServiceEx) &&
		 (GetMessageType() != WONMsg::DirG2ModifyServiceExObsolete)))
	{
		WDBG_AH("SMsgDirG2ModifyService::Unpack Not a DirG2ModifyService(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2ModifyService(Ex) message.");
	}

	mEntityFlags = ReadByte();
	UnpackKey(*this);
	ReadWString(mNewName);

	unsigned char aLen = ReadByte();
	WDBG_LL("SMsgDirG2ModifyService::Unpack Read New Addr len=" << aLen);
	if (aLen > 0)
		mNewNetAddress.assign(reinterpret_cast<const unsigned char*>(ReadBytes(aLen)), aLen);

	ReadWString(mNewDisplayName);
	mNewLifespan = ReadLong();
	UnpackExtended();
	UnpackPeerData();
}
