// TMsgCommQueryOptions.cpp

// Common query option message classes.  Supports option requests and replys to
// WON servers.


#include "common/won.h"
#include "msg/TMessage.h"
#include "msg/BadMsgException.h"
#include "msg/TServiceTypes.h"
#include "msg/ServerStatus.h"
#include "TMsgTypesComm.h"
#include "TMsgCommQueryOptions.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::TMessage;
	using WONMsg::TMsgCommQueryOptions;
	using WONMsg::TMsgCommQueryOptReply;
};


// ** TMsgCommQueryOptions **

// ** Constructors / Destructor

// Default ctor
TMsgCommQueryOptions::TMsgCommQueryOptions(void) :
	TMessage(),
	mOptionList()
{
	WTRACE("TMsgCommQueryOptions::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommQueryOptions);
}


// TMessage ctor
TMsgCommQueryOptions::TMsgCommQueryOptions(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mOptionList()
{
	WTRACE("TMsgCommQueryOptions::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommQueryOptions::TMsgCommQueryOptions(const TMsgCommQueryOptions& theMsgR) :
	TMessage(theMsgR),
	mOptionList(theMsgR.mOptionList)
{
	WTRACE("TMsgCommQueryOptions::ctor(copy)");
}


// Destructor
TMsgCommQueryOptions::~TMsgCommQueryOptions(void)
{
	WTRACE("TMsgCommQueryOptions::destruct");
}


// ** Public Methods **

// Assignment operator
TMsgCommQueryOptions&
TMsgCommQueryOptions::operator=(const TMsgCommQueryOptions& theMsgR)
{
	WTRACE("TMsgCommQueryOptions::operator=");
	TMessage::operator=(theMsgR);
	mOptionList = theMsgR.mOptionList;
	return *this;
}


// TMsgCommQueryOptions::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommQueryOptions::Pack(void)
{
	WTRACE("TMsgCommQueryOptions::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommQueryOptions);
	TMessage::Pack();

	WDBG_LL("TMsgCommQueryOptions::Pack Appending " << mOptionList.size() << "entries.");
	AppendShort(mOptionList.size());
	OptionList::iterator anItr = mOptionList.begin();
	for (; anItr != mOptionList.end(); anItr++)
		Append_PA_STRING(*anItr);

	return GetDataPtr();
}


// TMsgCommQueryOptions::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommQueryOptions::Unpack(void)
{
	WTRACE("TMsgCommQueryOptions::Unpack");
	mOptionList.clear();
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommQueryOptions))
	{
		WDBG_AH("TMsgCommRehupOptions::Unpack Not a CommQueryOptions message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommQueryOptions message.");
	}

	// Get num options
	unsigned short aCt = ReadShort();

	WDBG_LL("TMsgCommQueryOptions::Unpack Reading " << aCt << "entries.");
	for (int i=0; i < aCt; i++)
	{
		string aBuf;
		ReadString(aBuf);
		mOptionList.push_back(aBuf);
	}
}


// ** TMsgCommQueryOptReply **

// ** Constructors / Destructor

// Default ctor
TMsgCommQueryOptReply::TMsgCommQueryOptReply(void) :
	TMessage(),
	mStatus(WONMsg::StatusCommon_Success),
	mOptionMap()
{
	WTRACE("TMsgCommQueryOptReply::ctor(def)");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommQueryOptionsReply);
}


// TMessage ctor
TMsgCommQueryOptReply::TMsgCommQueryOptReply(const TMessage& theMsgR) :
	TMessage(theMsgR),
	mStatus(WONMsg::StatusCommon_Success),
	mOptionMap()
{
	WTRACE("TMsgCommQueryOptReply::ctor(TMessage)");
	Unpack();
}


// Copy ctor
TMsgCommQueryOptReply::TMsgCommQueryOptReply(const TMsgCommQueryOptReply& theMsgR) :
	TMessage(theMsgR),
	mStatus(theMsgR.mStatus),
	mOptionMap(theMsgR.mOptionMap)
{
	WTRACE("TMsgCommQueryOptReply::ctor(copy)");
}


// Destructor
TMsgCommQueryOptReply::~TMsgCommQueryOptReply(void)
{
	WTRACE("TMsgCommQueryOptReply::destruct");
}


// ** Public Methods **

// Assignment operator
TMsgCommQueryOptReply&
TMsgCommQueryOptReply::operator=(const TMsgCommQueryOptReply& theMsgR)
{
	WTRACE("TMsgCommPing::operator=");
	TMessage::operator=(theMsgR);
	mStatus = theMsgR.mStatus;
	mOptionMap = theMsgR.mOptionMap;
	return *this;
}


// TMsgCommQueryOptReply::Pack
// Virtual method from TMessage.  Packs data into message buffer and
// sets the new message length.
void*
TMsgCommQueryOptReply::Pack(void)
{
	WTRACE("TMsgCommQueryOptReply::Pack");
	SetServiceType(WONMsg::CommonService);
	SetMessageType(WONMsg::CommQueryOptionsReply);
	TMessage::Pack();

	WDBG_LL("TMsgCommQueryOptReply::Pack Appending " << mOptionMap.size() << " entries");
	AppendShort(mOptionMap.size());

	OptionMap::iterator anItr = mOptionMap.begin();
	for (; anItr != mOptionMap.end(); anItr++)
	{
		Append_PA_STRING(anItr->first);
		AppendByte(anItr->second.first);
		Append_PW_STRING(anItr->second.second);
	}

	AppendShort(static_cast<short>(mStatus));

	return GetDataPtr();
}


// TMsgCommPingReply::unpack
// Virtual method from TMessage.  Extracts data from message buffer.
void
TMsgCommQueryOptReply::Unpack(void)
{
	WTRACE("TMsgCommQueryOptReply::Unpack");
	mOptionMap.clear();
	TMessage::Unpack();

	if ((GetServiceType() != WONMsg::CommonService) ||
	    (GetMessageType() != WONMsg::CommQueryOptionsReply))
	{
		WDBG_AH("TMsgCommQueryOptReply::Unpack Not a CommQueryOptionsReply message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a CommQueryOptionsReply message.");
	}

	unsigned short aCt = ReadShort();
	WDBG_LL("TMsgCommQueryOptReply::Unpack reading " << aCt << " entries");

	for (int i=0; i < aCt; i++)
	{
		OptionDef aDef;
		string    anOpt;
		ReadString(anOpt);
		aDef.first  = static_cast<OptionStatus>(ReadByte());
		ReadWString(aDef.second);
		mOptionMap[anOpt] = aDef;
	}

	if (BytesLeftToRead() >= 2)
		mStatus = static_cast<WONMsg::ServerStatus>(static_cast<short>(ReadShort()));
}
