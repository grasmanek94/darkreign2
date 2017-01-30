// SMsgDirG2FindService

// DirectoryServer find service message.  Locates one or more services based on
// key fields.  Fields of service(s) included in reply are configurable.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDir.h"
#include "SMsgDirG2QueryFindBase.h"
#include "SMsgDirG2FindService.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgDirG2QueryFindBase;
	using WONMsg::SMsgDirG2FindService;
};


// ** Constructors / Destructor **

// Default ctor
SMsgDirG2FindService::SMsgDirG2FindService(bool isExtended) :
	SMsgDirG2QueryFindBase(KT_SERVICE, isExtended)
{
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mExtended ? WONMsg::DirG2FindServiceEx : WONMsg::DirG2FindService);
}


// SmallMessage ctor
SMsgDirG2FindService::SMsgDirG2FindService(const SmallMessage& theMsgR) :
	SMsgDirG2QueryFindBase(theMsgR)
{
	Unpack();
}


// Copy ctor
SMsgDirG2FindService::SMsgDirG2FindService(const SMsgDirG2FindService& theMsgR) :
	SMsgDirG2QueryFindBase(theMsgR)
{}


// Destructor
SMsgDirG2FindService::~SMsgDirG2FindService(void)
{}


// ** Public Methods **

// Assignment operator
SMsgDirG2FindService&
SMsgDirG2FindService::operator=(const SMsgDirG2FindService& theMsgR)
{
	SMsgDirG2QueryFindBase::operator=(theMsgR);
	return *this;
}


// SMsgDirG2FindService::Pack
// Virtual method from SmallMessage.  Packs data into message buffer.
void*
SMsgDirG2FindService::Pack(void)
{
	WTRACE("SMsgDirG2FindService::Pack");
	SetKeyType(KT_SERVICE);
	SetServiceType(WONMsg::SmallDirServerG2);
	SetMessageType(mExtended ? WONMsg::DirG2FindServiceEx : WONMsg::DirG2FindService);

	SMsgDirG2QueryFindBase::Pack();
	AppendShort(mEntitiesPerReply);

	PackKey(*this);
	Append_PW_STRING(mDisplayName);
	PackSearchObjects();
	PackExtended();
	return GetDataPtr();
}


// SMsgDirG2FindService::Unpack
// Virtual method from SmallMessage.  Extracts data from message buffer.
void
SMsgDirG2FindService::Unpack(void)
{
	WTRACE("SMsgDirG2FindService::Unpack");
	SetKeyType(KT_SERVICE);
	mExtended = (GetMessageType() == WONMsg::DirG2FindServiceEx);
	SMsgDirG2QueryFindBase::Unpack();

	if ((GetServiceType() != WONMsg::SmallDirServerG2) ||
	    ((GetMessageType() != WONMsg::DirG2FindService) &&
	     (GetMessageType() != WONMsg::DirG2FindServiceEx)))
	{
		WDBG_AH("SMsgDirG2FindService::Unpack Not a DirG2FindService(Ex) message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DirG2FindService(Ex) message.");
	}

	mEntitiesPerReply = ReadShort();
	UnpackKey(*this);
	ReadWString(mDisplayName);
	UnpackSearchObjects();
	UnpackExtended();
}
