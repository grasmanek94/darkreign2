#include "common/won.h"
#include <time.h>
#ifdef WIN32
#include <crtdbg.h> // for _ASSERT
#include <sstream>
#endif
#include "db/DBTypes.h"
#include "common/OutputOperators.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesEvent.h"
#include "SMsgEventRecordEvent.h"

// Private namespace for using, types, and constants
namespace {
	using WONMsg::SmallMessage;
	using WONMsg::SMsgEventRecordEvent;
	using WONDatabase::DBBlob;
	using WONDatabase::DBNumber8;
	using WONDatabase::DBNumber10_2;
	using WONDatabase::DBSeqIdentifier;
	using WONDatabase::DBSeqSmall;
	using WONDatabase::DBSeqTiny;
	using WONDatabase::DBVarCharWide;
};

SMsgEventRecordEvent::SMsgEventRecordEvent(void) : 
    SmallMessage(),
	mActivityType(0),
	mActivityDateTime(0),
	mHasRelatedServer(false),
	mHasRelatedClient(false),
	mHasRelatedUser(false),
	mHasDateTime(false),
    mServerType(0),
	mUserAuthenticationMethod(0),
    mUserId(0)
{
	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventRecordEvent);
}

SMsgEventRecordEvent::~SMsgEventRecordEvent(void)
{}

SMsgEventRecordEvent::SMsgEventRecordEvent(const SMsgEventRecordEvent& theMsgR) :
    SmallMessage(theMsgR),
	mActivityType(theMsgR.mActivityType),
	mActivityDateTime(theMsgR.mActivityDateTime),
	mHasRelatedServer(theMsgR.mHasRelatedServer),
	mHasRelatedClient(theMsgR.mHasRelatedClient),
	mHasRelatedUser(theMsgR.mHasRelatedUser),
	mHasDateTime(theMsgR.mHasDateTime),
	mDetailList(theMsgR.mDetailList),
	mAttachmentList(theMsgR.mAttachmentList),
    mClientName(theMsgR.mClientName),
    mClientNetAddress(theMsgR.mClientNetAddress),
    mServerType(theMsgR.mServerType),
    mServerLogicalName(theMsgR.mServerLogicalName),
    mServerNetAddress(theMsgR.mServerNetAddress),
	mUserAuthenticationMethod(theMsgR.mUserAuthenticationMethod),
    mUserId(theMsgR.mUserId),
    mUserName(theMsgR.mUserName)
{}

SMsgEventRecordEvent::SMsgEventRecordEvent(const SmallMessage& theMsgR, bool doUnpack) : 
    SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
}

SMsgEventRecordEvent& SMsgEventRecordEvent::operator =(const SMsgEventRecordEvent& theMsgR)
{
	if(this == &theMsgR){
		return *this;
	}

    // set fields from theMsgR
	mActivityType = theMsgR.mActivityType;
	mActivityDateTime = theMsgR.mActivityDateTime;
	mHasRelatedServer = theMsgR.mHasRelatedServer;
	mHasRelatedClient = theMsgR.mHasRelatedClient;
	mHasRelatedUser = theMsgR.mHasRelatedUser;
	mHasDateTime = theMsgR.mHasDateTime;
	mDetailList = theMsgR.mDetailList;
	mAttachmentList = theMsgR.mAttachmentList;
    mClientName = theMsgR.mClientName;
    mClientNetAddress = theMsgR.mClientNetAddress;
    mServerType = theMsgR.mServerType;
    mServerLogicalName = theMsgR.mServerLogicalName;
    mServerNetAddress = theMsgR.mServerNetAddress;
	mUserAuthenticationMethod = theMsgR.mUserAuthenticationMethod;
    mUserId = theMsgR.mUserId;
    mUserName = theMsgR.mUserName;

	return *this;
}

void SMsgEventRecordEvent::AddAttachment(const DBVarCharWide& theDescriptionR, DBSeqTiny theContentType, const DBBlob& theBodyR)
{
	Attachment anAttachment;
	anAttachment.mSize = theBodyR.size();
	anAttachment.mDescription = theDescriptionR;
	anAttachment.mContentType = theContentType;
	anAttachment.mBody = theBodyR;
	AddAttachment(anAttachment);
}

void* SMsgEventRecordEvent::Pack(void)
{
	WTRACE("SMsgEventRecordEvent::Pack");

	SetServiceType(WONMsg::SmallEventServer);
	SetMessageType(WONMsg::EventRecordEvent);
	SmallMessage::Pack();

	WDBG_LL("SMsgEventRecordEvent::Pack Appending message data");

	SimplePack();

	return GetDataPtr();
}

void SMsgEventRecordEvent::SimplePack(void)
{
	// header-type stuff
	unsigned char aContentFlags = 0;
	if (mHasDateTime)      aContentFlags |= 0x01;
	if (mHasRelatedServer) aContentFlags |= 0x02;
	if (mHasRelatedClient) aContentFlags |= 0x04;
	if (mHasRelatedUser)   aContentFlags |= 0x08;
	AppendByte(aContentFlags);

	AppendByte(mDetailList.size());
	AppendByte(mAttachmentList.size());
	
	// general info
	AppendSeqSmall(mActivityType);

	// date-time
	if (mHasDateTime)
		AppendBytes(sizeof(mActivityDateTime), (void*)&mActivityDateTime);
	
	// server info
	if (mHasRelatedServer)
	{
		AppendSeqSmall(mServerType);
		Append_PW_STRING(mServerLogicalName);
		Append_PA_STRING(mServerNetAddress);
	}

    // client info
	if (mHasRelatedClient)
	{
		Append_PW_STRING(mClientName);
		Append_PA_STRING(mClientNetAddress);
	}

	// user info
	if (mHasRelatedUser)
	{
		AppendSeqSmall(mUserAuthenticationMethod);
		AppendSeqIdentifier(mUserId);
		Append_PW_STRING(mUserName);
	}

	// details
	DetailList::iterator aDetailItr = mDetailList.begin();
	for ( ; aDetailItr != mDetailList.end(); aDetailItr++)
	{
		AppendSeqSmall(aDetailItr->mDetailType);
		AppendByte(aDetailItr->mDataType);
		switch (aDetailItr->mDataType)
		{
			case NUMBER:
				AppendLong(aDetailItr->mNumber); break;
			case STRING:
				Append_PW_STRING(aDetailItr->mString); break;
			case DATE:
			default:
				_ASSERT(false);
		}
	}

	// attachments
	AttachmentList::iterator aAttachmentItr = mAttachmentList.begin();
	for ( ; aAttachmentItr != mAttachmentList.end(); aAttachmentItr++)
	{
		Append_PW_STRING(aAttachmentItr->mDescription);
		AppendSeqTiny(aAttachmentItr->mContentType);
		AppendNumber8(aAttachmentItr->mSize);
		AppendBytes(aAttachmentItr->mSize, aAttachmentItr->mBody.c_str());
	}
}

void SMsgEventRecordEvent::Unpack(void)
{
	WTRACE("SMsgEventRecordEvent::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallEventServer) ||
	    (GetMessageType() != WONMsg::EventRecordEvent))
	{
		WDBG_AH("SMsgEventRecordEvent::Unpack Not a EventRecordEvent message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a EventRecordEvent message.");
	}

	WDBG_LL("SMsgEventRecordEvent::Unpack Reading message data");

	SimpleUnpack();
}

void SMsgEventRecordEvent::SimpleUnpack(void)
{
	// header-type stuff
	unsigned char aContentFlags = ReadByte();
	mHasDateTime      = (aContentFlags & 0x01) != 0;
	mHasRelatedServer = (aContentFlags & 0x02) != 0;
	mHasRelatedClient = (aContentFlags & 0x04) != 0;
	mHasRelatedUser   = (aContentFlags & 0x08) != 0;

	unsigned char aNumDetailList     = ReadByte();
	unsigned char aNumAttachmentList = ReadByte();
	
	// general info
	mActivityType = ReadSeqSmall();

	// date-time
	if (mHasDateTime)
		mActivityDateTime = *(time_t*)ReadBytes(sizeof(mActivityDateTime));

	// server info
	if (mHasRelatedServer)
	{
		mServerType  = ReadSeqSmall();
		mServerLogicalName = Read_PW_STRING();
		mServerNetAddress  = Read_PA_STRING();
	}

	// client info
    if (mHasRelatedClient)
	{
		mClientName       = Read_PW_STRING();
		mClientNetAddress = Read_PA_STRING();
	}

	// user info
    if (mHasRelatedUser)
	{
		mUserAuthenticationMethod = ReadSeqSmall();
		mUserId                   = ReadSeqIdentifier();
		mUserName                 = Read_PW_STRING();
	}

	// details
	for (int iDetail = 0; iDetail < aNumDetailList; iDetail++)
	{
		Detail aDetail;
		aDetail.mDetailType = ReadSeqSmall();
		switch (ReadByte())
		{
			case 0: // number
				aDetail.mDataType = NUMBER;
				aDetail.mNumber = ReadLong(); break;
			case 1: // Unicode string
				aDetail.mDataType = STRING;
				 ReadWString(aDetail.mString); break;
			case 2: // date
				aDetail.mDataType = DATE;
				// fall through for now
			default:
				std::stringstream aExceptStr;
				aExceptStr << "DataType: " << (int)aDetail.mDataType;
				throw WONMsg::BadMsgException(WONCommon::ExEventBadDataType, __LINE__, __FILE__, aExceptStr.str().c_str());
		}
		mDetailList.push_back(aDetail);
	}

	// attachments
	for (int iAttachment = 0; iAttachment < aNumAttachmentList; iAttachment++)
	{
		Attachment anAttachment;
		anAttachment.mDescription = Read_PW_STRING();
		anAttachment.mContentType = ReadSeqTiny();
		anAttachment.mSize        = ReadNumber8();
		anAttachment.mBody.assign((unsigned char*)ReadBytes(anAttachment.mSize));
		mAttachmentList.push_back(anAttachment);
	}
}

void SMsgEventRecordEvent::Dump(std::ostream& os) const
{
	os << "  ActivityType: "      << mActivityType                      << endl;

	
	os << "  HasRelatedServer: "  << (mHasRelatedServer ? "yes" : "no") << endl  
	   << "  HasRelatedClient: "  << (mHasRelatedClient ? "yes" : "no") << endl  
	   << "  HasRelatedUser: "    << (mHasRelatedUser ? "yes" : "no")   << endl
	   << "  HasDateTime: "       << (mHasDateTime ? "yes" : "no")      << endl
	   << "  NumDetailList: "     << mDetailList.size()                 << endl 
	   << "  NumAttachmentList: " << mAttachmentList.size()             << endl;

	os << "  ActivityDateTime: ";
	if (mHasDateTime)
	{
		char aDateTimeString[20];
		tm* aTMP = gmtime(&mActivityDateTime);
		if (aTMP)
		{
			strftime(aDateTimeString, 20, "%m/%d/%Y %H:%M:%S", aTMP);
			os << aDateTimeString;
		}
		else
			os << "Invalid value";
	}
	else
		os << "DEFAULT";
	os << endl;
	if (mHasRelatedServer)
	{
		os << "  ServerType: "        << mServerType                        << endl 
	       << "  ServerLogicalName: " << mServerLogicalName                 << endl 
	       << "  ServerNetAddress: "  << mServerNetAddress                  << endl;
	}
	if (mHasRelatedClient)
	{
	    os << "  ClientName: "        << mClientName                        << endl 
	       << "  ClientNetAddress: "  << mClientNetAddress                  << endl;
	}
	if (mHasRelatedUser)
	{
	    os << "  UserAuthMethod: "    << mUserAuthenticationMethod          << endl 
	       << "  UserId: "            << (unsigned long) mUserId            << endl 
	       << "  UserName: "          << mUserName                          << endl;
	}

	DetailList::const_iterator aDetailItr = mDetailList.begin();
	for (int iDetail = 1; aDetailItr != mDetailList.end(); iDetail++, aDetailItr++)
	{
		os << "Detail #" << iDetail << ":"
		   << "\n\tDetail Type: " << aDetailItr->mDetailType
		   << "\n\tData Type: ";
		switch (aDetailItr->mDataType)
		{
			case NUMBER:
				os << "NUMBER" << "\n\tValue: " << aDetailItr->mNumber; break;
			case STRING:
				os << "STRING" << "\n\tValue: " << aDetailItr->mString; break;
			case DATE:
				os << "DATE" << "\n\tValue: " << aDetailItr->mDate; break;
			default:
				_ASSERT(false);
		}
		os << endl;
	}

	AttachmentList::const_iterator aAttachmentItr = mAttachmentList.end();
	for (int iAttachment = 1; aAttachmentItr != mAttachmentList.end(); iAttachment++, aAttachmentItr++)
	{
		os << "Attachment #" << iAttachment << ":"
		   << "\n\tSize: " << aAttachmentItr->mSize
		   << "\n\tDescription: " << aAttachmentItr->mDescription
		   << "\n\tContentType: " << aAttachmentItr->mContentType
		   << "\n\tBody: " << aAttachmentItr->mBody << endl;
	}
}