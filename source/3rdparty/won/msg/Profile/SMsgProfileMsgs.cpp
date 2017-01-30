#include "SMsgProfileMsgs.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesProfile.h"

using namespace WONMsg;

const unsigned long PROFILE_MASK_GENDER			= 1<<PROFILE_GENDER; 
const unsigned long PROFILE_MASK_BIRTHDATE		= 1<<PROFILE_BIRTHDATE;
const unsigned long PROFILE_MASK_FIRSTNAME		= 1<<PROFILE_FIRSTNAME;
const unsigned long PROFILE_MASK_LASTNAME		= 1<<PROFILE_LASTNAME;
const unsigned long PROFILE_MASK_MIDDLENAME		= 1<<PROFILE_MIDDLENAME;
const unsigned long PROFILE_MASK_COMPANYNAME	= 1<<PROFILE_COMPANYNAME;
const unsigned long PROFILE_MASK_ADDRESS1		= 1<<PROFILE_ADDRESS1;
const unsigned long PROFILE_MASK_ADDRESS2		= 1<<PROFILE_ADDRESS2;
const unsigned long PROFILE_MASK_CITY			= 1<<PROFILE_CITY;
const unsigned long PROFILE_MASK_POSTALCODE		= 1<<PROFILE_POSTALCODE;
const unsigned long PROFILE_MASK_STATE			= 1<<PROFILE_STATE;
const unsigned long PROFILE_MASK_PHONE1			= 1<<PROFILE_PHONE1;
const unsigned long PROFILE_MASK_PHONE1EXT		= 1<<PROFILE_PHONE1EXT;
const unsigned long PROFILE_MASK_PHONE2			= 1<<PROFILE_PHONE2;
const unsigned long PROFILE_MASK_PHONE2EXT		= 1<<PROFILE_PHONE2EXT;
const unsigned long PROFILE_MASK_EMAIL1			= 1<<PROFILE_EMAIL1;
const unsigned long PROFILE_MASK_EMAIL2			= 1<<PROFILE_EMAIL2;
const unsigned long PROFILE_MASK_CREATIONDATE	= 1<<PROFILE_CREATIONDATE;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileCreateRequest::SMsgProfileCreateRequest() : mFields(PROFILE_MAX)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCreateRequest);
	mFieldMask = 0;
}

SMsgProfileCreateRequest::SMsgProfileCreateRequest(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR), mFields(PROFILE_MAX)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileCreateRequest);
	}
}

SMsgProfileCreateRequest::SMsgProfileCreateRequest(const SMsgProfileCreateRequest& theMsgR) :
	SmallMessage(theMsgR),
	mFieldMask(theMsgR.mFieldMask),
	mFields(theMsgR.mFields)
{
}

SMsgProfileCreateRequest& SMsgProfileCreateRequest::operator=(const SMsgProfileCreateRequest& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mFieldMask = theMsgR.mFieldMask;
	mFields = theMsgR.mFields;

	return *this;
}

TRawMsg* SMsgProfileCreateRequest::Duplicate(void) const
{
	return new SMsgProfileCreateRequest(*this);
}

void SMsgProfileCreateRequest::SetField(int i, const std::wstring& sField)
{
	if ((i < 0) || (i >= PROFILE_MAX))
		return;

	mFieldMask |= 1<<i;
	mFields[i] = sField;
}

bool SMsgProfileCreateRequest::GetField(int i, std::wstring& sField) const
{
	if (((mFieldMask >> i) & 0x1) == 0)
		return (false);
	sField = mFields[i];
	return (true);
}

void* SMsgProfileCreateRequest::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCreateRequest);
	SmallMessage::Pack();

	int aFieldIdx;
	int aFieldCount = 0;

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			aFieldCount++;

	AppendLong(mFieldMask);
	AppendByte(aFieldCount);

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			Append_PW_STRING(mFields[aFieldIdx]);

	return GetDataPtr();
}
 
void SMsgProfileCreateRequest::Unpack(void)
{
	SmallMessage::Unpack();

	mFieldMask = ReadLong();
	ReadByte();
	for (int aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)		
			ReadWString(mFields[aFieldIdx]);
		else
			mFields[aFieldIdx] = L"";		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileCreateResponse::SMsgProfileCreateResponse()
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCreateResponse);
	mErrorCode = StatusSuccessCommonStart;
}

SMsgProfileCreateResponse::SMsgProfileCreateResponse(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileCreateResponse);
	}
}

SMsgProfileCreateResponse::SMsgProfileCreateResponse(const SMsgProfileCreateResponse& theMsgR) :
	SmallMessage(theMsgR),
	mErrorCode(theMsgR.mErrorCode)
{
}

SMsgProfileCreateResponse& SMsgProfileCreateResponse::operator=(const SMsgProfileCreateResponse& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mErrorCode = theMsgR.mErrorCode;

	return *this;
}

TRawMsg* SMsgProfileCreateResponse::Duplicate(void) const
{
	return new SMsgProfileCreateResponse(*this);
}

void SMsgProfileCreateResponse::SetErrorCode(ServerStatus theErrorCode)
{
	mErrorCode = theErrorCode;
}

ServerStatus  SMsgProfileCreateResponse::GetErrorCode() const
{
	return mErrorCode;
}

void* SMsgProfileCreateResponse::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCreateResponse);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));

	return GetDataPtr();
}
 
void SMsgProfileCreateResponse::Unpack(void)
{
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileUpdateRequest::SMsgProfileUpdateRequest() : mFields(PROFILE_MAX)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileUpdateRequest);
	mFieldMask = 0;
}

SMsgProfileUpdateRequest::SMsgProfileUpdateRequest(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR), mFields(PROFILE_MAX)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileUpdateRequest);
	}
}

SMsgProfileUpdateRequest::SMsgProfileUpdateRequest(const SMsgProfileUpdateRequest& theMsgR) :
	SmallMessage(theMsgR),
	mFieldMask(theMsgR.mFieldMask),
	mFields(theMsgR.mFields)
{
}

SMsgProfileUpdateRequest& SMsgProfileUpdateRequest::operator=(const SMsgProfileUpdateRequest& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mFieldMask = theMsgR.mFieldMask;
	mFields = theMsgR.mFields;

	return *this;
}

TRawMsg* SMsgProfileUpdateRequest::Duplicate(void) const
{
	return new SMsgProfileUpdateRequest(*this);
}

void SMsgProfileUpdateRequest::SetField(int i, const std::wstring& sField)
{
	if ((i < 0) || (i >= PROFILE_MAX))
		return;

	mFieldMask |= 1<<i;
	mFields[i] = sField;
}

bool SMsgProfileUpdateRequest::GetField(int i, std::wstring& sField) const
{
	if (((mFieldMask >> i) & 0x1) == 0)
		return (false);
	sField = mFields[i];
	return (true);
}

void* SMsgProfileUpdateRequest::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileUpdateRequest);
	SmallMessage::Pack();

	int aFieldIdx;
	int aFieldCount = 0;

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			aFieldCount++;

	AppendLong(mFieldMask);
	AppendByte(aFieldCount);

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			Append_PW_STRING(mFields[aFieldIdx]);

	return GetDataPtr();
}
 
void SMsgProfileUpdateRequest::Unpack(void)
{
	SmallMessage::Unpack();

	mFieldMask = ReadLong();
	ReadByte();
	for (int aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)		
			ReadWString(mFields[aFieldIdx]);
		else
			mFields[aFieldIdx] = L"";		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileUpdateResponse::SMsgProfileUpdateResponse()
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileUpdateResponse);
	mErrorCode = StatusSuccessCommonStart;
	mFieldMask = 0;
}

SMsgProfileUpdateResponse::SMsgProfileUpdateResponse(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileUpdateResponse);
	}
}

SMsgProfileUpdateResponse::SMsgProfileUpdateResponse(const SMsgProfileUpdateResponse& theMsgR) :
	SmallMessage(theMsgR),
	mErrorCode(theMsgR.mErrorCode),
	mFieldMask(theMsgR.mFieldMask)
{
}

SMsgProfileUpdateResponse& SMsgProfileUpdateResponse::operator=(const SMsgProfileUpdateResponse& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mErrorCode = theMsgR.mErrorCode;
	mFieldMask = theMsgR.mFieldMask;

	return *this;
}

TRawMsg* SMsgProfileUpdateResponse::Duplicate(void) const
{
	return new SMsgProfileUpdateResponse(*this);
}

void SMsgProfileUpdateResponse::SetErrorCode(ServerStatus theErrorCode)
{
	mErrorCode = theErrorCode;
}

ServerStatus SMsgProfileUpdateResponse::GetErrorCode() const
{
	return mErrorCode;
}

void SMsgProfileUpdateResponse::SetField(int i)
{
	mFieldMask |= (1<<i);
}

bool SMsgProfileUpdateResponse::GetField(int i) const
{
	return (mFieldMask & (1<<i)) != 0;
}

void* SMsgProfileUpdateResponse::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileUpdateResponse);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	AppendLong(mFieldMask);

	return GetDataPtr();
}
 
void SMsgProfileUpdateResponse::Unpack(void)
{
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	mFieldMask = ReadLong();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileReadRequest::SMsgProfileReadRequest()
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileReadRequest);
	mFieldMask = 0;
}

SMsgProfileReadRequest::SMsgProfileReadRequest(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileReadRequest);
	}
}

SMsgProfileReadRequest::SMsgProfileReadRequest(const SMsgProfileReadRequest& theMsgR) :
	SmallMessage(theMsgR),
	mFieldMask(theMsgR.mFieldMask)	
{
}

SMsgProfileReadRequest& SMsgProfileReadRequest::operator=(const SMsgProfileReadRequest& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mFieldMask = theMsgR.mFieldMask;	

	return *this;
}

TRawMsg* SMsgProfileReadRequest::Duplicate(void) const
{
	return new SMsgProfileReadRequest(*this);
}

void SMsgProfileReadRequest::SetFieldMask(unsigned long theFieldMask)
{
	mFieldMask = theFieldMask;
}

unsigned long SMsgProfileReadRequest::GetFieldMask() const
{
	return mFieldMask;
}

void SMsgProfileReadRequest::SetField(int i)
{
	mFieldMask |= (1<<i);
}

bool SMsgProfileReadRequest::GetField(int i) const
{
	return (mFieldMask & (1<<i)) != 0;
}

void* SMsgProfileReadRequest::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileReadRequest);
	SmallMessage::Pack();
	
	AppendLong(mFieldMask);

	return GetDataPtr();
}
 
void SMsgProfileReadRequest::Unpack(void)
{
	SmallMessage::Unpack();

	mFieldMask = ReadLong();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileReadResponse::SMsgProfileReadResponse() : mFields(PROFILE_MAX)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileReadResponse);
	mFieldMask = 0;
	mErrorCode = StatusSuccessCommonStart;
	mRequestMask = 0;
}

SMsgProfileReadResponse::SMsgProfileReadResponse(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR), mFields(PROFILE_MAX)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileReadResponse);
	}
}

SMsgProfileReadResponse::SMsgProfileReadResponse(const SMsgProfileReadResponse& theMsgR) :
	SmallMessage(theMsgR),
	mFieldMask(theMsgR.mFieldMask),
	mErrorCode(theMsgR.mErrorCode),
	mRequestMask(theMsgR.mRequestMask),
	mFields(theMsgR.mFields)
{
}

SMsgProfileReadResponse& SMsgProfileReadResponse::operator=(const SMsgProfileReadResponse& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mFieldMask = theMsgR.mFieldMask;
	mErrorCode = theMsgR.mErrorCode;
	mRequestMask = theMsgR.mRequestMask;
	mFields = theMsgR.mFields;

	return *this;
}

TRawMsg* SMsgProfileReadResponse::Duplicate(void) const
{
	return new SMsgProfileReadResponse(*this);
}

void SMsgProfileReadResponse:: SetRequestFieldMask(unsigned long theMask)
{
	mRequestMask = theMask;
}

void SMsgProfileReadResponse::SetRequestField(int i)
{
	mRequestMask |= (1<<i);
}

bool SMsgProfileReadResponse::GetRequestField(int i) const
{
	return (mRequestMask & (1<<i)) != 0;
}

void SMsgProfileReadResponse::SetErrorCode(ServerStatus theErrorCode)
{
	mErrorCode = theErrorCode;
}

ServerStatus SMsgProfileReadResponse::GetErrorCode() const
{
	return mErrorCode;
}

void SMsgProfileReadResponse::SetField(int i, const std::wstring& sField)
{
	if ((i < 0) || (i >= PROFILE_MAX))
		return;

	mFieldMask |= 1<<i;
	mFields[i] = sField;
}

bool SMsgProfileReadResponse::GetField(int i, std::wstring& sField) const
{
	if (((mFieldMask >> i) & 0x1) == 0)
		return (false);
	sField = mFields[i];
	return (true);
}

void* SMsgProfileReadResponse::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileReadResponse);
	SmallMessage::Pack();

	int aFieldIdx;
	int aFieldCount = 0;

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			aFieldCount++;

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	AppendLong(mRequestMask);
	AppendLong(mFieldMask);
	AppendByte(aFieldCount);

	for (aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			Append_PW_STRING(mFields[aFieldIdx]);

	return GetDataPtr();
}
 
void SMsgProfileReadResponse::Unpack(void)
{
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	mRequestMask = ReadLong();
	mFieldMask = ReadLong();
	ReadByte();
	for (int aFieldIdx = 0; aFieldIdx < PROFILE_MAX; aFieldIdx++)
		if ((mFieldMask & (1<<aFieldIdx)) != 0)
			ReadWString(mFields[aFieldIdx]);
		else
			mFields[aFieldIdx] = L"";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileCheckFieldsRequest::SMsgProfileCheckFieldsRequest()
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCheckFieldsRequest);
	mFieldMask = 0;
}

SMsgProfileCheckFieldsRequest::SMsgProfileCheckFieldsRequest(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileCheckFieldsRequest);
	}
}

SMsgProfileCheckFieldsRequest::SMsgProfileCheckFieldsRequest(const SMsgProfileCheckFieldsRequest& theMsgR) :
	SmallMessage(theMsgR),
	mFieldMask(theMsgR.mFieldMask)
{
}

SMsgProfileCheckFieldsRequest& SMsgProfileCheckFieldsRequest::operator=(const SMsgProfileCheckFieldsRequest& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mFieldMask = theMsgR.mFieldMask;

	return *this;
}

TRawMsg* SMsgProfileCheckFieldsRequest::Duplicate(void) const
{
	return new SMsgProfileCheckFieldsRequest(*this);
}

void SMsgProfileCheckFieldsRequest::SetFieldMask(unsigned long theFieldMask)
{
	mFieldMask = theFieldMask;
}

unsigned long SMsgProfileCheckFieldsRequest::GetFieldMask() const 
{
	return mFieldMask;
}

void SMsgProfileCheckFieldsRequest::SetField(int i)
{
	mFieldMask |= (1<<i);
}

bool SMsgProfileCheckFieldsRequest::GetField(int i) const
{
	return (mFieldMask & (1<<i)) != 0;
}

void* SMsgProfileCheckFieldsRequest::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCheckFieldsRequest);
	SmallMessage::Pack();
	
	AppendLong(mFieldMask);

	return GetDataPtr();
}
 
void SMsgProfileCheckFieldsRequest::Unpack(void)
{
	SmallMessage::Unpack();

	mFieldMask = ReadLong();	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SMsgProfileCheckFieldsResponse::SMsgProfileCheckFieldsResponse()
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCheckFieldsResponse);	
	mRequestMask = 0;
	mFieldMask = 0;
	mErrorCode = StatusSuccessCommonStart;	
}

SMsgProfileCheckFieldsResponse::SMsgProfileCheckFieldsResponse(const SmallMessage& theMsgR, bool doUnpack) :
	SmallMessage(theMsgR)
{
	if (doUnpack)
		Unpack();
	else
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::ProfileCheckFieldsResponse);
	}
}

SMsgProfileCheckFieldsResponse::SMsgProfileCheckFieldsResponse(const SMsgProfileCheckFieldsResponse& theMsgR) : 
	SmallMessage(theMsgR),
	mRequestMask(theMsgR.mRequestMask),
	mFieldMask(theMsgR.mFieldMask),
	mErrorCode(theMsgR.mErrorCode)
{
}

SMsgProfileCheckFieldsResponse& SMsgProfileCheckFieldsResponse::operator=(const SMsgProfileCheckFieldsResponse& theMsgR)
{
	SmallMessage::operator=(theMsgR);

	mRequestMask = theMsgR.mRequestMask;
	mFieldMask = theMsgR.mFieldMask;
	mErrorCode = theMsgR.mErrorCode;

	return *this;
}

TRawMsg* SMsgProfileCheckFieldsResponse::Duplicate(void) const
{
	return new SMsgProfileCheckFieldsResponse(*this);
}

void SMsgProfileCheckFieldsResponse:: SetRequestFieldMask(unsigned long theMask)
{
	mRequestMask = theMask;
}

void SMsgProfileCheckFieldsResponse::SetRequestField(int i)
{
	mRequestMask |= (1<<i);
}

bool SMsgProfileCheckFieldsResponse::GetRequestField(int i) const
{
	return (mRequestMask & (1<<i)) != 0;
}

void SMsgProfileCheckFieldsResponse::SetErrorCode(ServerStatus theErrorCode)
{
	mErrorCode = theErrorCode;
}

ServerStatus SMsgProfileCheckFieldsResponse::GetErrorCode() const
{
	return mErrorCode;
}

void SMsgProfileCheckFieldsResponse::SetField(int i)
{
	mFieldMask |= (1<<i);
}

bool SMsgProfileCheckFieldsResponse::GetField(int i) const
{
	return (mFieldMask & (1<<i)) != 0;
}

void* SMsgProfileCheckFieldsResponse::Pack(void)
{
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::ProfileCheckFieldsResponse);
	SmallMessage::Pack();		

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	AppendLong(mRequestMask);
	AppendLong(mFieldMask);

	return GetDataPtr();
}
 
void SMsgProfileCheckFieldsResponse::Unpack(void)
{
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	mRequestMask = ReadLong();
	mFieldMask = ReadLong();
}

// ============
// Profile2 Msg

//////////////
// Profile2Get

void* SMsgProfile2Get::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2Get);
	SmallMessage::Pack();

	AppendShort(mFields.size());
	std::set<int>::const_iterator it = mFields.begin(), end = mFields.end();
	for (; it != end; ++it) AppendLong(*it);
	return GetDataPtr();
}

void SMsgProfile2Get::Unpack() {
	SmallMessage::Unpack();
	unsigned short i = ReadShort();
	while (i--) mFields.insert(ReadLong());
}

TRawMsg* SMsgProfile2Get::Duplicate() const { 
	return new SMsgProfile2Get(*this); 
}

///////////////////
// Profile2GetReply

void SMsgProfile2GetReply::PackProfile2GetReplyInfo(const Profile2GetReplyInfo& info) {
	AppendRawString(info.mBuffer);
	AppendShort(static_cast<short>(info.mErrorCode));
	Append_PA_STRING(info.mErrorString);
}

void SMsgProfile2GetReply::UnpackProfile2GetReplyInfo(Profile2GetReplyInfo& info) {
	ReadRawString(info.mBuffer);
	info.mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(info.mErrorString);
}

void* SMsgProfile2GetReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mFields.size());
	std::multimap<int, Profile2GetReplyInfo>::const_iterator it = mFields.begin(), end = mFields.end();
	for (; it != end; ++it) { AppendLong(it->first); PackProfile2GetReplyInfo(it->second); }
	return GetDataPtr();
}

void SMsgProfile2GetReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	while (i--) 
		UnpackProfile2GetReplyInfo(mFields.insert(std::multimap<int, Profile2GetReplyInfo>::value_type(ReadLong(), Profile2GetReplyInfo()))->second);
}

TRawMsg* SMsgProfile2GetReply::Duplicate() const { 
	return new SMsgProfile2GetReply(*this); 
}

//////////////
// Profile2Set

void* SMsgProfile2Set::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2Set);
	SmallMessage::Pack();

	AppendShort(mFields.size());
	std::multimap<int, WONCommon::RawBuffer>::const_iterator it = mFields.begin(), end = mFields.end();
	for (; it != end; ++it) { 
		AppendLong(it->first); 
		AppendRawString(it->second); 
	}
	return GetDataPtr();
}

void SMsgProfile2Set::Unpack() {
	SmallMessage::Unpack();

	unsigned short i = ReadShort();
	while (i--) 
		ReadRawString(mFields.insert(std::multimap<int, WONCommon::RawBuffer>::value_type(ReadLong(), WONCommon::RawBuffer()))->second);
}

TRawMsg* SMsgProfile2Set::Duplicate() const { 
	return new SMsgProfile2Set(*this); 
}

///////////////////
// Profile2SetReply

void SMsgProfile2SetReply::PackProfile2SetReplyInfo(const Profile2SetReplyInfo& info) {
	AppendShort(static_cast<short>(info.mErrorCode));
	Append_PA_STRING(info.mErrorString);
}

void SMsgProfile2SetReply::UnpackProfile2SetReplyInfo(Profile2SetReplyInfo& info) {
	info.mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(info.mErrorString);
}

void* SMsgProfile2SetReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2SetReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mFields.size());
	std::multimap<int, Profile2SetReplyInfo>::const_iterator it = mFields.begin(), end = mFields.end();
	for (; it != end; ++it) { AppendLong(it->first); PackProfile2SetReplyInfo(it->second); }
	return GetDataPtr();
}

void SMsgProfile2SetReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	while (i--) 
		UnpackProfile2SetReplyInfo(mFields.insert(std::multimap<int, Profile2SetReplyInfo>::value_type(ReadLong(), Profile2SetReplyInfo()))->second);
}

TRawMsg* SMsgProfile2SetReply::Duplicate() const { 
	return new SMsgProfile2SetReply(*this); 
}

/////////////////////////
// Profile2GetCommunities

void* SMsgProfile2GetCommunities::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetCommunities);
	SmallMessage::Pack();
	AppendByte(justMine);
	return GetDataPtr();
}

void SMsgProfile2GetCommunities::Unpack() {
	SmallMessage::Unpack();
	justMine = ReadBool();
}

TRawMsg* SMsgProfile2GetCommunities::Duplicate() const { 
	return new SMsgProfile2GetCommunities(*this); 
}

//////////////////////////////
// Profile2GetCommunitiesReply

void* SMsgProfile2GetCommunitiesReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetCommunitiesReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mCommunities.size());
	std::map<long, std::wstring>::const_iterator it = mCommunities.begin(), end = mCommunities.end();
	for (; it != end; ++it) { AppendLong(it->first); Append_PW_STRING(it->second); }	
	return GetDataPtr();
}

void SMsgProfile2GetCommunitiesReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	while (i--) 
		ReadWString(mCommunities.insert(std::map<long, std::wstring>::value_type(ReadLong(), std::wstring())).first->second);
}

TRawMsg* SMsgProfile2GetCommunitiesReply::Duplicate() const { 
	return new SMsgProfile2GetCommunitiesReply(*this); 
}

/////////////////////////
// Profile2GetNewsletters

void* SMsgProfile2GetNewsletters::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetNewsletters);
	SmallMessage::Pack();
	AppendByte(justMine);
	return GetDataPtr();
}

void SMsgProfile2GetNewsletters::Unpack() {
	SmallMessage::Unpack();
	justMine = ReadBool();
}

TRawMsg* SMsgProfile2GetNewsletters::Duplicate() const { 
	return new SMsgProfile2GetNewsletters(*this); 
}

//////////////////////////////
// Profile2GetNewslettersReply

void SMsgProfile2GetNewslettersReply::PackProfile2NewsletterInfo(const Profile2NewsletterInfo& info) {
	Append_PW_STRING(info.mDesc);
	AppendShort(info.mCommunitySet.size());
	std::set<std::wstring>::const_iterator it = info.mCommunitySet.begin(), end = info.mCommunitySet.end();
	for (; it != end; ++it) Append_PW_STRING(*it);
}

void SMsgProfile2GetNewslettersReply::UnpackProfile2NewsletterInfo(Profile2NewsletterInfo& info) {
	ReadWString(info.mDesc);
	unsigned short i = ReadShort();
	std::wstring w;
	while (i--) { ReadWString(w); info.mCommunitySet.insert(w); }
}

void* SMsgProfile2GetNewslettersReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetNewslettersReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mNewsletters.size());
	std::map<std::wstring, Profile2NewsletterInfo>::const_iterator it = mNewsletters.begin(), end = mNewsletters.end();
	for (; it != end; ++it) { 
		Append_PW_STRING(it->first); 
		PackProfile2NewsletterInfo(it->second);
	}	
	return GetDataPtr();
}

void SMsgProfile2GetNewslettersReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	std::wstring w;
	while (i--) {
		ReadWString(w);
		UnpackProfile2NewsletterInfo(mNewsletters.insert(std::map<std::wstring, Profile2NewsletterInfo>::value_type(w, Profile2NewsletterInfo())).first->second);
	}
}

TRawMsg* SMsgProfile2GetNewslettersReply::Duplicate() const { 
	return new SMsgProfile2GetNewslettersReply(*this); 
}

///////////////////////////////////
// Profile2GetCommunityNewsletters

void* SMsgProfile2GetCommunityNewsletters::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetCommunityNewsletters);
	SmallMessage::Pack();
	AppendByte(justMine);
	Append_PW_STRING(mCommunity);
	return GetDataPtr();
}

void SMsgProfile2GetCommunityNewsletters::Unpack() {
	SmallMessage::Unpack();
	justMine = ReadBool();
	ReadWString(mCommunity);
}

TRawMsg* SMsgProfile2GetCommunityNewsletters::Duplicate() const { 
	return new SMsgProfile2GetCommunityNewsletters(*this); 
}

///////////////////////////////////////
// Profile2GetCommunityNewslettersReply

void* SMsgProfile2GetCommunityNewslettersReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2GetCommunityNewslettersReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mNewsletters.size());
	std::map<std::wstring, std::wstring>::const_iterator it = mNewsletters.begin(), end = mNewsletters.end();
	for (; it != end; ++it) { 
		Append_PW_STRING(it->first); 
		Append_PW_STRING(it->second);
	}	
	return GetDataPtr();
}

void SMsgProfile2GetCommunityNewslettersReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	std::wstring w;
	while (i--) {
		ReadWString(w);
		ReadWString(mNewsletters.insert(std::map<std::wstring, std::wstring>::value_type(w, std::wstring())).first->second);
	}
}

TRawMsg* SMsgProfile2GetCommunityNewslettersReply::Duplicate() const { 
	return new SMsgProfile2GetCommunityNewslettersReply(*this); 
}

/////////////////////////
// Profile2SetNewsletters

void* SMsgProfile2SetNewsletters::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2SetNewsletters);
	SmallMessage::Pack();

	AppendShort(mNewsletters.size());
	std::map<std::wstring, bool>::const_iterator it = mNewsletters.begin(), end = mNewsletters.end();
	for (; it != end; ++it) { 
		Append_PW_STRING(it->first); 
		AppendByte(it->second);
	}
	return GetDataPtr();
}

void SMsgProfile2SetNewsletters::Unpack() {
	SmallMessage::Unpack();

	unsigned short i = ReadShort();
	std::wstring w;
	while (i--) {
		ReadWString(w);
		mNewsletters.insert(std::map<std::wstring, bool>::value_type(w, ReadBool()));
	}
}

TRawMsg* SMsgProfile2SetNewsletters::Duplicate() const { 
	return new SMsgProfile2SetNewsletters(*this); 
}

//////////////////////////////
// Profile2SetNewslettersReply

void* SMsgProfile2SetNewslettersReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2SetNewslettersReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(mResults.size());
	std::map<std::wstring, std::string>::const_iterator it = mResults.begin(), end = mResults.end();
	for (; it != end; ++it) { 
		Append_PW_STRING(it->first); 
		Append_PA_STRING(it->second);
	}

	return GetDataPtr();
}

void SMsgProfile2SetNewslettersReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	std::wstring w;
	while (i--) {
		ReadWString(w);
		ReadString(mResults.insert(std::map<std::wstring, std::string>::value_type(w, std::string())).first->second);
	}
}

TRawMsg* SMsgProfile2SetNewslettersReply::Duplicate() const { 
	return new SMsgProfile2SetNewslettersReply(*this); 
}

/////////////////////////////////////
// Profile2DownloadCommunityTreeReply

void SMsgProfile2DownloadCommunityTreeReply::PackCommunityData(const CommunityData& info) {
	AppendLong(info.CommunitySeq);
	AppendLong(info.TrustLevelId);
	Append_PW_STRING(info.Name);
	Append_PW_STRING(info.Descr);
	AppendLong(info.JoinByInvitationOnly);
	AppendLong(info.ParentCommunitySeq);

	AppendShort(info.ChildrenSeq.size());
	std::vector<long>::const_iterator it = info.ChildrenSeq.begin(), end = info.ChildrenSeq.end();
	for (; it != end; ++it) AppendLong(*it);
}

void SMsgProfile2DownloadCommunityTreeReply::UnpackCommunityData(std::map<long, CommunityData>::iterator& itInfo) {
	itInfo->second.CommunitySeq = itInfo->first;
	itInfo->second.TrustLevelId = ReadLong();
	ReadWString(itInfo->second.Name);
	ReadWString(itInfo->second.Descr);
	itInfo->second.JoinByInvitationOnly = ReadLong();
	itInfo->second.ParentCommunitySeq = ReadLong();

	unsigned short n = ReadShort();
	for (int i = 0; i < n; ++i) itInfo->second.ChildrenSeq.push_back(ReadLong());

	databyname.insert(std::map<std::wstring, long>::value_type(itInfo->second.Name, itInfo->second.CommunitySeq));
}

void* SMsgProfile2DownloadCommunityTreeReply::Pack() { 
	SetServiceType(WONMsg::SmallProfileServer);
	SetMessageType(WONMsg::Profile2DownloadCommunityTreeReply);
	SmallMessage::Pack();

	AppendLong(APIErrorCode);
	AppendShort(static_cast<short>(mErrorCode));
	Append_PA_STRING(mErrorString);

	AppendShort(data.size());
	std::map<long, CommunityData>::const_iterator it = data.begin(), end = data.end();
	for (; it != end; ++it) PackCommunityData(it->second);

	AppendShort(rootcommunities.size());
	std::vector<long>::const_iterator it2 = rootcommunities.begin(), end2 = rootcommunities.end();
	for (; it2 != end2; ++it2) AppendLong(*it2);

	return GetDataPtr();
}

void SMsgProfile2DownloadCommunityTreeReply::Unpack() {
	SmallMessage::Unpack();

	APIErrorCode = ReadLong();
	mErrorCode = static_cast<ServerStatus>((short)ReadShort());
	ReadString(mErrorString);

	unsigned short i = ReadShort();
	while (i--)
		UnpackCommunityData(data.insert(std::map<long, CommunityData>::value_type(ReadLong(), CommunityData())).first);
	
	i = ReadShort();
	for (int j = 0; j < i; ++j) rootcommunities.push_back(ReadLong());
}
