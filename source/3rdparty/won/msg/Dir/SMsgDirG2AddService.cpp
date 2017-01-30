// SMsgDirG2AddService.h

// DirectoryServer add service message.  Adds a new service at specified path.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2AddService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2UpdateExtendBase;
	using WONMsg::SMsgDirG2AddService;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2AddService::SMsgDirG2AddService(bool isExtended) :
	SMsgDirG2UpdateExtendBase(KT_SERVICE, isExtended,isExtended),
	mEntityFlags(0),
	mDisplayName(),
	mLifespan(0)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(IsExtended() ? WONMsg::DirG2AddServiceEx : WONMsg::DirG2AddService);
}


// SmallMessage ctor
SMsgDirG2AddService::SMsgDirG2AddService(const SmallMessage& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(0),
	mDisplayName(),
	mLifespan(0)
{
	Unpack();
}


// Copy ctor
SMsgDirG2AddService::SMsgDirG2AddService(const SMsgDirG2AddService& theMsgR) :
	SMsgDirG2UpdateExtendBase(theMsgR),
	mEntityFlags(theMsgR.mEntityFlags),
	mDisplayName(theMsgR.mDisplayName),
	mLifespan(theMsgR.mLifespan)
{}


// Destructor
SMsgDirG2AddService::~SMsgDirG2AddService(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2AddService&
SMsgDirG2AddService::operator=(const SMsgDirG2AddService& theMsgR)
{
	SMsgDirG2UpdateExtendBase::operator=(theMsgR);
	mEntityFlags = theMsgR.mEntityFlags;
	mDisplayName = theMsgR.mDisplayName;
	mLifespan    = theMsgR.mLifespan;
	return *this;
}


// SMsgDirG2AddService::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2AddService::Pack(void)
{
	WTRACE("SMsgDirG2AddService::Pack");
	SetKeyType(KT_SERVICE);
	SetServiceType(WONMsg::SmallDirServerG2);
	if (IsExtended())
	{
		SetMessageType(WONMsg::DirG2AddServiceEx);
		SetExtended(true, true);
	}
	else
		SetMessageType(WONMsg::DirG2AddService);

	SMsgDirG2UpdateExtendBase::Pack();
	AppendByte(mEntityFlags);
	PackKey(*this);
	Append_PW_STRING(mDisplayName);
	AppendLong(mLifespan);
	PackExtended();
	PackPeerData();

	return GetDataPtr();
}


// SMsgDirG2AddService::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2AddService::Unpack(void)
{
	WTRACE("SMsgDirG2AddService::Unpack");
	SetKeyType(KT_SERVICE);
	SetExtended((GetMessageType() != WONMsg::DirG2AddService), (GetMessageType() == WONMsg::DirG2AddServiceEx));
	SMsgDirG2UpdateExtendBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2AddService) &&
	     (GetMessageType() != WONMsg::DirG2AddServiceEx) &&
		 (GetMessageType() != WONMsg::DirG2AddServiceExObsolete)))
	{
		WDBG_AH("SMsgDirG2AddService::Unpack Not a DirG2AddService(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2AddService(Ex) message.");
	}

	mEntityFlags = ReadByte();
	UnpackKey(*this);
	ReadWString(mDisplayName);
	mLifespan = ReadLong();
	UnpackExtended();
	UnpackPeerData();
}
