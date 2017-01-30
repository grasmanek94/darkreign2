////////////////////////////////////////////////////////////////////////////////////

#ifndef __SMSGPROFILEMSGS_H__
#define __SMSGPROFILEMSGS_H__

#ifndef H_TMessage
#include "msg/TMessage.h"
#endif // H_TMessage

#ifndef _ServerStatus_H_
#include "msg/ServerStatus.h"
#endif // _ServerStatus_H_

#ifndef _SServiceTypes_H_
#include "msg/SServiceTypes.h"
#endif // _SServiceTypes_H_

#ifndef __SMSGTYPESPROFILE_H__
#include "SMsgTypesProfile.h"
#endif // __SMSGTYPESPROFILE_H__

#ifndef _ASSERT_INCLUDED_
#include <assert.h>
#define _ASSERT_INCLUDED_
#endif // _ASSERT_INCLUDED_

#ifndef _STRING_INCLUDED_
#include <string>
#define _STRING_INCLUDED_
#endif // _STRING_INCLUDED_

#ifndef _VECTOR_INCLUDED_
#include <vector>
#define _VECTOR_INCLUDED_
#endif // _VECTOR_INCLUDED_

#ifndef _MAP_INCLUDED_
#include <map>
#define _MAP_INCLUDED_
#endif // _MAP_INCLUDED_

#ifndef _SET_INCLUDED_
#include <set>
#define _SET_INCLUDED_
#endif // _SET_INCLUDED_

#ifndef __WON_STRING_H__
#include "common/WONString.h"
#endif // __WON_STRING_H__

#ifndef __WON_ENDIAN_H__
#include "common/WONEndian.h"
#endif // __WON_ENDIAN_H__

namespace WONMsg {

enum PROFILE_FIELDS
{
	 PROFILE_GENDER = 0
	,PROFILE_BIRTHDATE
	,PROFILE_FIRSTNAME
	,PROFILE_LASTNAME
	,PROFILE_MIDDLENAME
	,PROFILE_COMPANYNAME
	,PROFILE_ADDRESS1
	,PROFILE_ADDRESS2
	,PROFILE_CITY
	,PROFILE_POSTALCODE
	,PROFILE_STATE
	,PROFILE_PHONE1
	,PROFILE_PHONE1EXT
	,PROFILE_PHONE2
	,PROFILE_PHONE2EXT
	,PROFILE_EMAIL1
	,PROFILE_EMAIL2
	,PROFILE_CREATIONDATE
	,PROFILE_MODIFIEDDATE
	,PROFILE_MAX
	// added for 2nd generation messages; should not affect 1st generation
	,PROFILE_AGERANGE = PROFILE_MAX
	,PROFILE_COUNTRY
	,PROFILE_SOURCE
	,PROFILE_NEWSLETTERSUBS
	,PROFILE_MAX2
};

////////////////////////////////////////////////////////////////
// birthday_t datatype (time_t only works for dates in the range
// midnight, January 1, 1970 -- January 18, 19:14:07, 2038)
// Instead I chose the following format (not the most compact...): 
//		byte 0: month; byte 1: day; bytes 2-3: year
//
// IsValid() checks for (but does not enforce) silly figures (days > 31, feb. 30 etc.)
// The profile server takes care of additional "logical" constraints 
// (e.g., age can't be below 13 or some other magic number, etc.)
//
struct birthday_t {
	birthday_t() : date(0) {}

	void SetMonth(unsigned char m) { Set(m, MONTH); }	// valid values: 1 to 12
	void SetDay(unsigned char d) { Set(d, DAY); }		// valid values: 1 to 31
	void SetYear(unsigned short y) { Set(y, YEAR); }	// valid values: 1881 to current (Jeanne Calment died recently)

	int GetMonth() { static const unsigned char m = MONTH; return Get(m); }
	int GetDay() { static const unsigned char d = DAY; return Get(d); }
	int GetYear() { static const unsigned short y = YEAR; return Get(y); }

	bool IsValid() { 
		int month = GetMonth(), day = GetDay(), year = GetYear();
		time_t aTime = time(NULL);
		struct tm* current_tm = localtime(&aTime);

		return (year > 1880) && month && day && (month < 13) && 
			   ((year < current_tm->tm_year + 1900) || 
				((year == current_tm->tm_year + 1900) && 
				 ((month < current_tm->tm_mon + 1) || 
				  ((month == current_tm->tm_mon + 1) && (current_tm->tm_mday <= day))))) &&
			   ((((month != 2) && (day/31 <= month%2)) || 
				((month == 2) && ( (day < 29) || ( (day == 29) && (!(year / 4))))))); // Y2K compliant;
				// But: ignores leap years every new century and 400-multiple exceptions for now (until 2100)
	}
	unsigned long Get() { return date; }
	void Clear() { date = 0; }

private:
	enum {MONTH, DAY, YEAR};
	template <class T> void Set(T& t, int offset) { memcpy((char*)&date + offset, &t, sizeof(t)); }
	template <class T> int Get(const T t) { int result = 0; return *(int*)memcpy(&result, (char*)&date + t, sizeof(t)); }
	unsigned long date;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileCreateRequest : public SmallMessage
{
public:
	SMsgProfileCreateRequest();
	
	explicit SMsgProfileCreateRequest(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileCreateRequest(const SMsgProfileCreateRequest& theMsgR);	

	SMsgProfileCreateRequest& operator=(const SMsgProfileCreateRequest& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// i is zero-based index
	void SetField(int i, const std::wstring&);

	// returns true if field set by field mask
	bool GetField(int i, std::wstring& sField) const;	

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:

	unsigned long mFieldMask;
	std::vector<std::wstring> mFields;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileCreateResponse : public SmallMessage
{
public:
	SMsgProfileCreateResponse();

	explicit SMsgProfileCreateResponse(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileCreateResponse(const SMsgProfileCreateResponse& theMsgR);	

	SMsgProfileCreateResponse& operator=(const SMsgProfileCreateResponse& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetErrorCode(ServerStatus);
	ServerStatus  GetErrorCode() const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileUpdateRequest : public SmallMessage
{
public:
	SMsgProfileUpdateRequest();

	explicit SMsgProfileUpdateRequest(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileUpdateRequest(const SMsgProfileUpdateRequest& theMsgR);	

	SMsgProfileUpdateRequest& operator=(const SMsgProfileUpdateRequest& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// i is zero-based index
	void SetField(int i, const std::wstring&);

	// returns true if field set by field mask
	bool GetField(int i, std::wstring&) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:

	unsigned long mFieldMask;
	std::vector<std::wstring> mFields;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileUpdateResponse : public SmallMessage
{
public:
	SMsgProfileUpdateResponse();

	explicit SMsgProfileUpdateResponse(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileUpdateResponse(const SMsgProfileUpdateResponse& theMsgR);	

	SMsgProfileUpdateResponse& operator=(const SMsgProfileUpdateResponse& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetErrorCode(ServerStatus);
	ServerStatus  GetErrorCode() const;

	// Bits are true (1) if field was updated, false (0) if field update failed
	// or update not attempted
	void SetField(int i);
	bool GetField(int i) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	unsigned long mFieldMask; // bits: 1 if field updated, 0 if failed/not attempted
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileReadRequest : public SmallMessage
{
public:
	SMsgProfileReadRequest();

	explicit SMsgProfileReadRequest(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileReadRequest(const SMsgProfileReadRequest& theMsgR);	

	SMsgProfileReadRequest& operator=(const SMsgProfileReadRequest& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetFieldMask(unsigned long theFieldMask);
	unsigned long GetFieldMask() const;

	// Set which fields we want to read
	void SetField(int i);
	bool GetField(int i) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:

	unsigned long mFieldMask;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileReadResponse : public SmallMessage
{
public:
	SMsgProfileReadResponse();

	explicit SMsgProfileReadResponse(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileReadResponse(const SMsgProfileReadResponse& theMsgR);	

	SMsgProfileReadResponse& operator=(const SMsgProfileReadResponse& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// Find out which fields were requested
	void SetRequestFieldMask(unsigned long theMask);
	void SetRequestField(int i);
	bool GetRequestField(int i) const;

	// General error
	void SetErrorCode(ServerStatus theErrorCode);
	ServerStatus GetErrorCode() const;

	// i is zero-based index
	void SetField(int i, const std::wstring&);

	// returns true if field was read without error
	bool GetField(int i, std::wstring&) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	unsigned long mRequestMask;
	unsigned long mFieldMask; // bits: 1 if field read, 0 if failed/not attempted
	std::vector<std::wstring> mFields;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileCheckFieldsRequest : public SmallMessage
{
public:
	SMsgProfileCheckFieldsRequest();

	explicit SMsgProfileCheckFieldsRequest(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileCheckFieldsRequest(const SMsgProfileCheckFieldsRequest& theMsgR);	

	SMsgProfileCheckFieldsRequest& operator=(const SMsgProfileCheckFieldsRequest& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetFieldMask(unsigned long theFieldMask);
	unsigned long GetFieldMask() const;

	// Set which fields we want to check
	void SetField(int i);
	bool GetField(int i) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:

	unsigned long mFieldMask;
};

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfileCheckFieldsResponse : public SmallMessage
{
public:
	SMsgProfileCheckFieldsResponse();

	explicit SMsgProfileCheckFieldsResponse(const SmallMessage& theMsgR, bool doUnpack = true);
	SMsgProfileCheckFieldsResponse(const SMsgProfileCheckFieldsResponse& theMsgR);	

	SMsgProfileCheckFieldsResponse& operator=(const SMsgProfileCheckFieldsResponse& theMsgR);

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	// Find out which fields were requested
	void SetRequestFieldMask(unsigned long theMask);
	void SetRequestField(int i);
	bool GetRequestField(int i) const;

	// General error
	void SetErrorCode(ServerStatus theErrorCode);
	ServerStatus GetErrorCode() const;

	// Bits are true (1) if field exists and valid, false (0) if field doesn't
	// exist, not valid, or not checked
	void SetField(int i);
	bool GetField(int i) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	unsigned long mRequestMask;
	unsigned long mFieldMask; // bits: 1 if field exists, 0 if invalid/not attempted
};


// ===
// Get

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfile2Get : public SmallMessage
{
public:
	SMsgProfile2Get(bool doSetAllFields = false)
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2Get);
		if (doSetAllFields) SetAllFields();
	}

	explicit SMsgProfile2Get(const SmallMessage& theMsgR, bool doUnpack = true) 
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2Get);
		}
	}

	SMsgProfile2Get(const SMsgProfile2Get& theMsgR)
		:	mFields(theMsgR.mFields), SmallMessage(theMsgR)
	{ }

	SMsgProfile2Get& operator=(const SMsgProfile2Get& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mFields = theMsgR.mFields;
		return *this;
	}

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetField(int i, bool setTo = true)
	{
		if (setTo)
			mFields.insert(i);
		else
			mFields.erase(i);
	}
	void ClearField(int i)	{ mFields.erase(i); }
	void RemoveField(int i)	{ mFields.erase(i); }

	bool GetField(int i) const	{ return (mFields.find(i) != mFields.end()); }

	const std::set<int>& GetFields() const
	{ return mFields; }


	void SetGender(bool b = true)		{ SetField(PROFILE_GENDER, b); }
	void SetBirthdate(bool b = true)	{ SetField(PROFILE_BIRTHDATE, b); }
	void SetFirstName(bool b = true)	{ SetField(PROFILE_FIRSTNAME, b); }
	void SetLastName(bool b = true)		{ SetField(PROFILE_LASTNAME, b); }
	void SetMiddleName(bool b = true)	{ SetField(PROFILE_MIDDLENAME, b); }
	void SetCompanyName(bool b = true)	{ SetField(PROFILE_COMPANYNAME, b); }
	void SetAddress1(bool b = true)		{ SetField(PROFILE_ADDRESS1, b); }
	void SetAddress2(bool b = true)		{ SetField(PROFILE_ADDRESS2, b); }
	void SetCity(bool b = true)			{ SetField(PROFILE_CITY, b); }
	void SetPostalCode(bool b = true)	{ SetField(PROFILE_POSTALCODE, b); }
	void SetState(bool b = true)		{ SetField(PROFILE_STATE, b); }
	void SetPhone1(bool b = true)		{ SetField(PROFILE_PHONE1, b); }
	void SetPhone1Ext(bool b = true)	{ SetField(PROFILE_PHONE1EXT, b); }
	void SetPhone2(bool b = true)		{ SetField(PROFILE_PHONE2, b); }
	void SetPhone2Ext(bool b = true)	{ SetField(PROFILE_PHONE2EXT, b); }
	void SetEmail1(bool b = true)		{ SetField(PROFILE_EMAIL1, b); }
	void SetEmail2(bool b = true)		{ SetField(PROFILE_EMAIL2, b); }
	void SetCreationDate(bool b = true)	{ SetField(PROFILE_CREATIONDATE, b); }
	void SetModifiedDate(bool b = true)	{ SetField(PROFILE_MODIFIEDDATE, b); }
	void SetAgeRange(bool b = true)		{ SetField(PROFILE_AGERANGE, b); }
	void SetCountry(bool b = true)		{ SetField(PROFILE_COUNTRY, b); }
	void SetSource(bool b = true)		{ SetField(PROFILE_SOURCE, b); }
	void SetNewsletterSubs(bool b = true){ SetField(PROFILE_NEWSLETTERSUBS, b); }
	void SetAllFields(bool b = true)	{ for (int i = PROFILE_GENDER; i < PROFILE_MAX2; ++i) SetField(i, b); }

	void ClearGender()			{ mFields.erase(PROFILE_GENDER); }
	void ClearBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void ClearFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void ClearLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void ClearMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void ClearCompanyName()		{ mFields.erase(PROFILE_COMPANYNAME); }
	void ClearAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void ClearAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void ClearCity()			{ mFields.erase(PROFILE_CITY); }
	void ClearPostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void ClearState()			{ mFields.erase(PROFILE_STATE); }
	void ClearPhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void ClearPhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void ClearPhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void ClearPhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void ClearEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void ClearEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void ClearCreationDate()	{ mFields.erase(PROFILE_CREATIONDATE); }
	void ClearModifiedDate()	{ mFields.erase(PROFILE_MODIFIEDDATE); }
	void ClearAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void ClearCountry()			{ mFields.erase(PROFILE_COUNTRY); }
	void ClearSource()			{ mFields.erase(PROFILE_SOURCE); }
	void ClearNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }
	void ClearAllFields()		{ for (int i = PROFILE_GENDER; i < PROFILE_MAX2; ++i) mFields.erase(i); }

	void RemoveGender()			{ mFields.erase(PROFILE_GENDER); }
	void RemoveBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void RemoveFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void RemoveLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void RemoveMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void RemoveCompanyName()	{ mFields.erase(PROFILE_COMPANYNAME); }
	void RemoveAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void RemoveAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void RemoveCity()			{ mFields.erase(PROFILE_CITY); }
	void RemovePostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void RemoveState()			{ mFields.erase(PROFILE_STATE); }
	void RemovePhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void RemovePhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void RemovePhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void RemovePhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void RemoveEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void RemoveEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void RemoveCreationDate()	{ mFields.erase(PROFILE_CREATIONDATE); }
	void RemoveModifiedDate()	{ mFields.erase(PROFILE_MODIFIEDDATE); }
	void RemoveAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void RemoveCountry()		{ mFields.erase(PROFILE_COUNTRY); }
	void RemoveSource()			{ mFields.erase(PROFILE_SOURCE); }
	void RemoveNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }
	void RemoveAllFields()		{ for (int i = PROFILE_GENDER; i < PROFILE_MAX2; ++i) mFields.erase(i); }

	bool GetGender()			{ return GetField(PROFILE_GENDER); }
	bool GetBirthdate()			{ return GetField(PROFILE_BIRTHDATE); }
	bool GetFirstName()			{ return GetField(PROFILE_FIRSTNAME); }
	bool GetLastName()			{ return GetField(PROFILE_LASTNAME); }
	bool GetMiddleName()		{ return GetField(PROFILE_MIDDLENAME); }
	bool GetCompanyName()		{ return GetField(PROFILE_COMPANYNAME); }
	bool GetAddress1()			{ return GetField(PROFILE_ADDRESS1); }
	bool GetAddress2()			{ return GetField(PROFILE_ADDRESS2); }
	bool GetCity()				{ return GetField(PROFILE_CITY); }
	bool GetPostalCode()		{ return GetField(PROFILE_POSTALCODE); }
	bool GetState()				{ return GetField(PROFILE_STATE); }
	bool GetPhone1()			{ return GetField(PROFILE_PHONE1); }
	bool GetPhone1Ext()			{ return GetField(PROFILE_PHONE1EXT); }
	bool GetPhone2()			{ return GetField(PROFILE_PHONE2); }
	bool GetPhone2Ext()			{ return GetField(PROFILE_PHONE2EXT); }
	bool GetEmail1()			{ return GetField(PROFILE_EMAIL1); }
	bool GetEmail2()			{ return GetField(PROFILE_EMAIL2); }
	bool GetCreationDate()		{ return GetField(PROFILE_CREATIONDATE); }
	bool GetModifiedDate()		{ return GetField(PROFILE_MODIFIEDDATE); }
	bool GetAgeRange()			{ return GetField(PROFILE_AGERANGE); }
	bool GetCountry()			{ return GetField(PROFILE_COUNTRY); }
	bool GetSource()			{ return GetField(PROFILE_SOURCE); }
	bool GetNewsletterSubs()	{ return GetField(PROFILE_NEWSLETTERSUBS); }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	std::set<int> mFields;
};

////////////////////////////////////////////////////////////////////////////////////


struct Profile2GetReplyInfo
{
	WONCommon::RawBuffer	mBuffer;
	ServerStatus			mErrorCode;
	std::string				mErrorString;

	Profile2GetReplyInfo(const WONCommon::RawBuffer& buf, ServerStatus err,
		const std::string& errString = std::string())
		:	mBuffer(buf), mErrorCode(err), mErrorString(errString)
	{ }

	Profile2GetReplyInfo(const WONCommon::WONString& str, ServerStatus err,
		const std::string& errString = std::string())
		:	mBuffer((unsigned char*)(str.GetUnicodeString().data()), str.GetUnicodeString().size() * 2),
		mErrorCode(err), mErrorString(errString)
	{ }

	Profile2GetReplyInfo(unsigned long l, ServerStatus err, const std::string& errString = std::string())
		:	mErrorCode(err), mErrorString(errString)
	{ SetLong(l); }

	Profile2GetReplyInfo(unsigned short s, ServerStatus err, const std::string& errString = std::string())
		:	mErrorCode(err), mErrorString(errString)
	{ SetShort(s); }

	Profile2GetReplyInfo(unsigned char c, ServerStatus err, const std::string& errString = std::string())
		:	mErrorCode(err), mErrorString(errString)
	{ SetChar(c); }
	
	Profile2GetReplyInfo(bool b, ServerStatus err, const std::string& errString = std::string())
		:	mErrorCode(err), mErrorString(errString)
	{ SetBool(b); }

	Profile2GetReplyInfo()
	{}

	void SetString(const WONCommon::WONString& str)
	{
		const std::wstring& tmpStr = str.GetUnicodeString();
		mBuffer.assign((unsigned char*)(tmpStr.data()), tmpStr.size() * 2);
	}

	void SetLong(unsigned long l)
	{
		unsigned long tmp = getLittleEndian(l);
		mBuffer.assign((unsigned char*)&tmp, sizeof(unsigned long));
	}
	void SetShort(unsigned short s)
	{
		unsigned short tmp = getLittleEndian(s);
		mBuffer.assign((unsigned char*)&tmp, sizeof(unsigned short));
	}
	void SetChar(unsigned char c)
	{
		mBuffer.assign(&c, sizeof(unsigned char));
	}
	void SetBool(bool b)
	{
		SetChar(b ? (char)1 : (char)0);
	}

	void GetString(WONCommon::WONString& str) const
	{ str.assign((const wchar*)(mBuffer.data()), mBuffer.size() / 2); }

	WONCommon::WONString GetString(void) const
	{ return WONCommon::WONString((const wchar*)(mBuffer.data()), mBuffer.size() / 2); }

	unsigned long GetLong() const
	{ return getLittleEndian(*(unsigned long*)(mBuffer.data())); }

	unsigned short GetShort() const
	{ return getLittleEndian(*(unsigned short*)(mBuffer.data())); }

	unsigned char GetChar() const
	{ return *(unsigned char*)(mBuffer.data()); }

	bool GetBool() const
	{ return GetChar() ? true : false; }
};


class SMsgProfile2GetReply : public SmallMessage
{
public:
	SMsgProfile2GetReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetReply);
	}

	explicit SMsgProfile2GetReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetReply);
		}
	}

	SMsgProfile2GetReply(const SMsgProfile2GetReply& theMsgR)
		: APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString), 
			mFields(theMsgR.mFields), 
			SmallMessage(theMsgR)
	{ }

	SMsgProfile2GetReply& operator=(const SMsgProfile2GetReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		mFields = theMsgR.mFields;
		return *this;
	}
	

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetField(int i, const Profile2GetReplyInfo& info)	{ 
		mFields.insert(std::multimap<int, Profile2GetReplyInfo>::value_type(i, info)); }
	void SetField(int i, const WONCommon::RawBuffer& buf, ServerStatus err, 
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(buf, err, errString)); }

	void SetStringField(int i, const WONCommon::WONString& str, ServerStatus err = StatusCommon_Success,
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(str, err, errString)); }

	void SetLongField(int i, unsigned long l, ServerStatus err = StatusCommon_Success,
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(l, err, errString)); }

	void SetShortField(int i, unsigned short s, ServerStatus err = StatusCommon_Success,
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(s, err, errString)); }

	void SetCharField(int i, unsigned char c, ServerStatus err = StatusCommon_Success,
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(c, err, errString)); }

	void SetBoolField(int i, bool b, ServerStatus err = StatusCommon_Success,
		const std::string& errString = std::string())
	{ SetField(i, Profile2GetReplyInfo(b, err, errString)); }


	// overloaded GetField()

	///////////////////////////////////////////////
	// returning first element of each sub-sequence

	const Profile2GetReplyInfo* GetField(int i) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}
	bool GetField(int i, Profile2GetReplyInfo& sField) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}
	const WONCommon::RawBuffer* GetFieldValue(int i) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second.mBuffer);
	}
	bool GetFieldValue(int i, WONCommon::RawBuffer& buf) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		buf = (*itor).second.mBuffer;
		return true;
	}
	const std::string* GetFieldError(int i) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second.mErrorString);
	}
	bool GetFieldError(int i, std::string& errString) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		errString = (*itor).second.mErrorString;
		return true;
	}
	bool GetFieldStatus(int i, ServerStatus& status) const
	{
		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		status = (*itor).second.mErrorCode;
		return true;
	}

	///////////////////////////////////////////////////////////////////
	// for newsletters: retrieving each element withing newsletter list

	int GetSize(int i) const { return mFields.count(i); }

	const Profile2GetReplyInfo* GetField(int i, int j) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return 0;

		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}
	bool GetField(int i, int j, Profile2GetReplyInfo& sField) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return false;

		std::multimap<int, Profile2GetReplyInfo>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}

	// per-type

	bool GetStringField(int i, WONCommon::WONString& str)
	{
		const Profile2GetReplyInfo* info = GetField(i);
		if (info)
		{
			info->GetString(str);
			return true;
		}
		return false;
	}

	bool GetLongField(int i, unsigned long& l)
	{
		const Profile2GetReplyInfo* info = GetField(i);
		if (info)
		{
			l = info->GetLong();
			return true;
		}
		return false;
	}

	bool GetShortField(int i, unsigned short& s)
	{
		const Profile2GetReplyInfo* info = GetField(i);
		if (info)
		{
			s = info->GetShort();
			return true;
		}
		return false;
	}

	bool GetCharField(int i, unsigned char& c)
	{
		const Profile2GetReplyInfo* info = GetField(i);
		if (info)
		{
			c = info->GetChar();
			return true;
		}
		return false;
	}

	bool GetBoolField(int i, bool& b)
	{
		const Profile2GetReplyInfo* info = GetField(i);
		if (info)
		{
			b = info->GetBool();
			return true;
		}
		return false;
	}


	void ClearField(int i)	{ mFields.erase(i); }
	void RemoveField(int i)	{ mFields.erase(i); }

	const std::multimap<int, Profile2GetReplyInfo>& GetFields() const
	{ return mFields; }


	void SetGender(bool b)									{ SetBoolField(PROFILE_GENDER, b); }
	void SetBirthdate(const WONCommon::WONString& str)		{ SetStringField(PROFILE_BIRTHDATE, str); }
	void SetFirstName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_FIRSTNAME, str); }
	void SetLastName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_LASTNAME, str); }
	void SetMiddleName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_MIDDLENAME, str); }
	void SetCompanyName(const WONCommon::WONString& str)	{ SetStringField(PROFILE_COMPANYNAME, str); }
	void SetAddress1(const WONCommon::WONString& str)		{ SetStringField(PROFILE_ADDRESS1, str); }
	void SetAddress2(const WONCommon::WONString& str)		{ SetStringField(PROFILE_ADDRESS2, str); }
	void SetCity(const WONCommon::WONString& str)			{ SetStringField(PROFILE_CITY, str); }
	void SetPostalCode(const WONCommon::WONString& str)		{ SetStringField(PROFILE_POSTALCODE, str); }
	void SetState(const WONCommon::WONString& str)			{ SetStringField(PROFILE_STATE, str); }
	void SetPhone1(const WONCommon::WONString& str)			{ SetStringField(PROFILE_PHONE1, str); }
	void SetPhone1Ext(const WONCommon::WONString& str)		{ SetStringField(PROFILE_PHONE1EXT, str); }
	void SetPhone2(const WONCommon::WONString& str)			{ SetStringField(PROFILE_PHONE2, str); }
	void SetPhone2Ext(const WONCommon::WONString& str)		{ SetStringField(PROFILE_PHONE2EXT, str); }
	void SetEmail1(const WONCommon::WONString& str)			{ SetStringField(PROFILE_EMAIL1, str); }
	void SetEmail2(const WONCommon::WONString& str)			{ SetStringField(PROFILE_EMAIL2, str); }
	void SetCreationDate(const WONCommon::WONString& str)	{ SetStringField(PROFILE_CREATIONDATE, str); }
	void SetModifiedDate(const WONCommon::WONString& str)	{ SetStringField(PROFILE_MODIFIEDDATE, str); }
	void SetAgeRange(const WONCommon::WONString& str)		{ SetStringField(PROFILE_AGERANGE, str); }
	void SetCountry(const WONCommon::WONString& str)		{ SetStringField(PROFILE_COUNTRY, str); }
	void SetSource(const WONCommon::WONString& str)			{ SetStringField(PROFILE_SOURCE, str); }
	void SetNewsletterSubs(const WONCommon::WONString& str)	{ SetStringField(PROFILE_NEWSLETTERSUBS, str); }

	bool GetGender(bool &b)								{ return GetBoolField(PROFILE_GENDER, b); }
	bool GetBirthdate(WONCommon::WONString& str)		{ return GetStringField(PROFILE_BIRTHDATE, str); }
	bool GetFirstName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_FIRSTNAME, str); }
	bool GetLastName(WONCommon::WONString& str)			{ return GetStringField(PROFILE_LASTNAME, str); }
	bool GetMiddleName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_MIDDLENAME, str); }
	bool GetCompanyName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_COMPANYNAME, str); }
	bool GetAddress1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_ADDRESS1, str); }
	bool GetAddress2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_ADDRESS2, str); }
	bool GetCity(WONCommon::WONString& str)				{ return GetStringField(PROFILE_CITY, str); }
	bool GetPostalCode(WONCommon::WONString& str)		{ return GetStringField(PROFILE_POSTALCODE, str); }
	bool GetState(WONCommon::WONString& str)			{ return GetStringField(PROFILE_STATE, str); }
	bool GetPhone1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_PHONE1, str); }
	bool GetPhone1Ext(WONCommon::WONString& str)		{ return GetStringField(PROFILE_PHONE1EXT, str); }
	bool GetPhone2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_PHONE2, str); }
	bool GetPhone2Ext(WONCommon::WONString& str)		{ return GetStringField(PROFILE_PHONE2EXT, str); }
	bool GetEmail1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_EMAIL1, str); }
	bool GetEmail2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_EMAIL2, str); }
	bool GetCreationDate(WONCommon::WONString& str)		{ return GetStringField(PROFILE_CREATIONDATE, str); }
	bool GetModifiedDate(WONCommon::WONString& str)		{ return GetStringField(PROFILE_MODIFIEDDATE, str); }
	bool GetAgeRange(WONCommon::WONString& str)			{ return GetStringField(PROFILE_AGERANGE, str); }
	bool GetCountry(WONCommon::WONString& str)			{ return GetStringField(PROFILE_COUNTRY, str); }
	bool GetSource(WONCommon::WONString& str)			{ return GetStringField(PROFILE_SOURCE, str); }

	//////////////
	// newsletters
	int GetNewsletterSubsCount()						{ return GetSize(PROFILE_NEWSLETTERSUBS); }
	bool GetNewsletterSub(WONCommon::WONString& str, int j = 0)	{ 
		const Profile2GetReplyInfo* p = GetField(PROFILE_NEWSLETTERSUBS, j);
		return p ? (p->GetString(str), true) : false;
	}

	////////////////
	// for debugging
	void ShowAllFields() {
		bool b, b1;
		WONCommon::WONString str;
		b = GetGender(b1);
		for (int i = PROFILE_BIRTHDATE; i < PROFILE_NEWSLETTERSUBS; ++i) 
			b = GetStringField(i, str);
		int count = GetNewsletterSubsCount();
		for (i = 0; i < count; ++i) 
			GetNewsletterSub(str, i);
		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

	void ClearGender()			{ mFields.erase(PROFILE_GENDER); }
	void ClearBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void ClearFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void ClearLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void ClearMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void ClearCompanyName()		{ mFields.erase(PROFILE_COMPANYNAME); }
	void ClearAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void ClearAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void ClearCity()			{ mFields.erase(PROFILE_CITY); }
	void ClearPostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void ClearState()			{ mFields.erase(PROFILE_STATE); }
	void ClearPhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void ClearPhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void ClearPhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void ClearPhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void ClearEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void ClearEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void ClearCreationDate()	{ mFields.erase(PROFILE_CREATIONDATE); }
	void ClearModifiedDate()	{ mFields.erase(PROFILE_MODIFIEDDATE); }
	void ClearAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void ClearCountry()			{ mFields.erase(PROFILE_COUNTRY); }
	void ClearSource()			{ mFields.erase(PROFILE_SOURCE); }
	void ClearNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	void RemoveGender()			{ mFields.erase(PROFILE_GENDER); }
	void RemoveBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void RemoveFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void RemoveLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void RemoveMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void RemoveCompanyName()	{ mFields.erase(PROFILE_COMPANYNAME); }
	void RemoveAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void RemoveAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void RemoveCity()			{ mFields.erase(PROFILE_CITY); }
	void RemovePostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void RemoveState()			{ mFields.erase(PROFILE_STATE); }
	void RemovePhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void RemovePhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void RemovePhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void RemovePhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void RemoveEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void RemoveEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void RemoveCreationDate()	{ mFields.erase(PROFILE_CREATIONDATE); }
	void RemoveModifiedDate()	{ mFields.erase(PROFILE_MODIFIEDDATE); }
	void RemoveAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void RemoveCountry()		{ mFields.erase(PROFILE_COUNTRY); }
	void RemoveSource()			{ mFields.erase(PROFILE_SOURCE); }
	void RemoveNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	void SetErrorCode(ServerStatus status)			{ mErrorCode = status; }
	ServerStatus  GetErrorCode() const				{ return mErrorCode; }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	std::multimap<int, Profile2GetReplyInfo> mFields; // multimap used to return multiple newsletter settings

private:
	void PackProfile2GetReplyInfo(const Profile2GetReplyInfo&);
	void UnpackProfile2GetReplyInfo(Profile2GetReplyInfo&);
};


// ===
// Set

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfile2Set : public SmallMessage
{
public:
	SMsgProfile2Set()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2Set);
	}

	explicit SMsgProfile2Set(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2Set);
		}
	}

	SMsgProfile2Set(const SMsgProfile2Set& theMsgR)
		:	mFields(theMsgR.mFields), SmallMessage(theMsgR)
	{ }

	SMsgProfile2Set& operator=(const SMsgProfile2Set& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mFields = theMsgR.mFields;
		return *this;
	}
	
	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	///////////
	// SetField

	void SetField(int i, const WONCommon::RawBuffer& buf)	{ 
		mFields.insert(std::multimap<int, WONCommon::RawBuffer>::value_type(i, buf)); }
	void SetField(int i, const Profile2GetReplyInfo& info) { 
		mFields.insert(std::multimap<int, WONCommon::RawBuffer>::value_type(i, info.mBuffer)); } // for server testing

	void SetStringField(int i, const WONCommon::WONString& str)
	{
		const std::wstring& tmpStr = str.GetUnicodeString();
		SetField(i, WONCommon::RawBuffer((unsigned char*)(tmpStr.data()), tmpStr.size() * 2));
	}

	void SetLongField(int i, unsigned long l)
	{
		unsigned long tmp = getLittleEndian(l);
		SetField(i, WONCommon::RawBuffer((unsigned char*)(&tmp), sizeof(unsigned long)));
	}

	void SetShortField(int i, unsigned short s)
	{
		unsigned short tmp = getLittleEndian(s);
		SetField(i, WONCommon::RawBuffer((unsigned char*)(&tmp), sizeof(unsigned short)));
	}

	void SetCharField(int i, unsigned char c)
	{ SetField(i, WONCommon::RawBuffer(&c, sizeof(unsigned char))); }

	void SetBoolField(int i, bool b)
	{ SetCharField(i, b ? (char)1 : (char)0); }

	///////////
	// GetField

	const WONCommon::RawBuffer* GetField(int i) const
	{
		std::multimap<int, WONCommon::RawBuffer>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}
	bool GetField(int i, WONCommon::RawBuffer& sField) const
	{
		std::multimap<int, WONCommon::RawBuffer>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}
	
	///////////////////////////////////////////////////////////////////
	// for newsletters: retrieving each element withing newsletter list

	int GetSize(int i) const { return mFields.count(i); }

	const WONCommon::RawBuffer* GetField(int i, int j) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return 0;

		std::multimap<int, WONCommon::RawBuffer>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}
	bool GetField(int i, int j, WONCommon::RawBuffer& sField) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return false;

		std::multimap<int, WONCommon::RawBuffer>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}

	//per-type

	bool GetStringField(int i, WONCommon::WONString& str) const
	{
		const WONCommon::RawBuffer* buf = GetField(i);
		if (buf)
		{
			str.assign((const wchar*)(buf->data()), buf->size() / 2);
			return true;
		}
		return false;
	}

	bool GetLongField(int i, unsigned long& l) const
	{
		const WONCommon::RawBuffer* buf = GetField(i);
		if (buf)
		{
			l = getLittleEndian(*(unsigned long*)(buf->data()));
			return true;
		}
		return false;
	}

	bool GetShortField(int i, unsigned short& s)
	{
		const WONCommon::RawBuffer* buf = GetField(i);
		if (buf)
		{
			s = getLittleEndian(*(unsigned short*)(buf->data()));
			return true;
		}
		return false;
	}

	bool GetCharField(int i, unsigned char& c)
	{
		const WONCommon::RawBuffer* buf = GetField(i);
		if (buf)
		{
			c = *(unsigned char*)(buf->data());
			return true;
		}
		return false;
	}

	bool GetBoolField(int i, bool& b) const
	{
		const WONCommon::RawBuffer* buf = GetField(i);
		if (buf)
		{
			b = *(unsigned char*)(buf->data()) ? true : false;
			return true;
		}
		return false;
	}


	void ClearField(int i)	{ mFields.erase(i); }
	void RemoveField(int i)	{ mFields.erase(i); }

	const std::multimap<int, WONCommon::RawBuffer>& GetFields() const
	{ return mFields; }

	void SetGender(bool b)									{ SetBoolField(PROFILE_GENDER, b); }
	void SetBirthdate(const WONCommon::WONString& str)		{ SetStringField(PROFILE_BIRTHDATE, str); }
	void SetFirstName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_FIRSTNAME, str); }
	void SetLastName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_LASTNAME, str); }
	void SetMiddleName(const WONCommon::WONString& str)		{ SetStringField(PROFILE_MIDDLENAME, str); }
	void SetCompanyName(const WONCommon::WONString& str)	{ SetStringField(PROFILE_COMPANYNAME, str); }
	void SetAddress1(const WONCommon::WONString& str)		{ SetStringField(PROFILE_ADDRESS1, str); }
	void SetAddress2(const WONCommon::WONString& str)		{ SetStringField(PROFILE_ADDRESS2, str); }
	void SetCity(const WONCommon::WONString& str)			{ SetStringField(PROFILE_CITY, str); }
	void SetPostalCode(const WONCommon::WONString& str)		{ SetStringField(PROFILE_POSTALCODE, str); }
	void SetState(const WONCommon::WONString& str)			{ SetStringField(PROFILE_STATE, str); }
	void SetPhone1(const WONCommon::WONString& str)			{ SetStringField(PROFILE_PHONE1, str); }
	void SetPhone1Ext(const WONCommon::WONString& str)		{ SetStringField(PROFILE_PHONE1EXT, str); }
	void SetPhone2(const WONCommon::WONString& str)			{ SetStringField(PROFILE_PHONE2, str); }
	void SetPhone2Ext(const WONCommon::WONString& str)		{ SetStringField(PROFILE_PHONE2EXT, str); }
	void SetEmail1(const WONCommon::WONString& str)			{ SetStringField(PROFILE_EMAIL1, str); }
	void SetEmail2(const WONCommon::WONString& str)			{ SetStringField(PROFILE_EMAIL2, str); }
	void SetAgeRange(const WONCommon::WONString& str)		{ SetStringField(PROFILE_AGERANGE, str); }
	void SetCountry(const WONCommon::WONString& str)		{ SetStringField(PROFILE_COUNTRY, str); }
	void SetSource(const WONCommon::WONString& str)			{ SetStringField(PROFILE_SOURCE, str); }
	void SetNewsletterSubs(const WONCommon::WONString& str)	{ SetStringField(PROFILE_NEWSLETTERSUBS, str); }

	bool GetGender(bool &b)								{ return GetBoolField(PROFILE_GENDER, b); }
	bool GetBirthdate(WONCommon::WONString& str)		{ return GetStringField(PROFILE_BIRTHDATE, str); }
	bool GetFirstName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_FIRSTNAME, str); }
	bool GetLastName(WONCommon::WONString& str)			{ return GetStringField(PROFILE_LASTNAME, str); }
	bool GetMiddleName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_MIDDLENAME, str); }
	bool GetCompanyName(WONCommon::WONString& str)		{ return GetStringField(PROFILE_COMPANYNAME, str); }
	bool GetAddress1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_ADDRESS1, str); }
	bool GetAddress2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_ADDRESS2, str); }
	bool GetCity(WONCommon::WONString& str)				{ return GetStringField(PROFILE_CITY, str); }
	bool GetPostalCode(WONCommon::WONString& str)		{ return GetStringField(PROFILE_POSTALCODE, str); }
	bool GetState(WONCommon::WONString& str)			{ return GetStringField(PROFILE_STATE, str); }
	bool GetPhone1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_PHONE1, str); }
	bool GetPhone1Ext(WONCommon::WONString& str)		{ return GetStringField(PROFILE_PHONE1EXT, str); }
	bool GetPhone2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_PHONE2, str); }
	bool GetPhone2Ext(WONCommon::WONString& str)		{ return GetStringField(PROFILE_PHONE2EXT, str); }
	bool GetEmail1(WONCommon::WONString& str)			{ return GetStringField(PROFILE_EMAIL1, str); }
	bool GetEmail2(WONCommon::WONString& str)			{ return GetStringField(PROFILE_EMAIL2, str); }
	bool GetAgeRange(WONCommon::WONString& str)			{ return GetStringField(PROFILE_AGERANGE, str); }
	bool GetCountry(WONCommon::WONString& str)			{ return GetStringField(PROFILE_COUNTRY, str); }
	bool GetSource(WONCommon::WONString& str)			{ return GetStringField(PROFILE_SOURCE, str); }

	//////////////
	// newsletters
	int GetNewsletterSubsCount() const						{ return GetSize(PROFILE_NEWSLETTERSUBS); }
	bool GetNewsletterSub(WONCommon::WONString& str, int j = 0)	 const { 
		const WONCommon::RawBuffer* p = GetField(PROFILE_NEWSLETTERSUBS, j);
		return p ? (str.assign((const wchar*)(p->data()), p->size() / 2), true) : false;
	}

	void ClearGender()			{ mFields.erase(PROFILE_GENDER); }
	void ClearBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void ClearFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void ClearLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void ClearMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void ClearCompanyName()		{ mFields.erase(PROFILE_COMPANYNAME); }
	void ClearAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void ClearAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void ClearCity()			{ mFields.erase(PROFILE_CITY); }
	void ClearPostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void ClearState()			{ mFields.erase(PROFILE_STATE); }
	void ClearPhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void ClearPhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void ClearPhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void ClearPhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void ClearEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void ClearEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void ClearAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void ClearCountry()			{ mFields.erase(PROFILE_COUNTRY); }
	void ClearSource()			{ mFields.erase(PROFILE_SOURCE); }
	void ClearNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	void RemoveGender()			{ mFields.erase(PROFILE_GENDER); }
	void RemoveBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void RemoveFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void RemoveLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void RemoveMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void RemoveCompanyName()	{ mFields.erase(PROFILE_COMPANYNAME); }
	void RemoveAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void RemoveAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void RemoveCity()			{ mFields.erase(PROFILE_CITY); }
	void RemovePostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void RemoveState()			{ mFields.erase(PROFILE_STATE); }
	void RemovePhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void RemovePhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void RemovePhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void RemovePhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void RemoveEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void RemoveEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void RemoveAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void RemoveCountry()		{ mFields.erase(PROFILE_COUNTRY); }
	void RemoveSource()			{ mFields.erase(PROFILE_SOURCE); }
	void RemoveNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	std::multimap<int, WONCommon::RawBuffer> mFields; // multimap used to input multiple newsletter settings
};



////////////////////////////////////////////////////////////////////////////////////


struct Profile2SetReplyInfo
{
	ServerStatus			mErrorCode;
	std::string				mErrorString;

	Profile2SetReplyInfo(ServerStatus err = StatusCommon_Success, const std::string& errString = std::string())
		:	mErrorCode(err), mErrorString(errString)
	{ }
};


class SMsgProfile2SetReply : public SmallMessage
{
public:
	SMsgProfile2SetReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2SetReply);
	}

	explicit SMsgProfile2SetReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2SetReply);
		}
	}

	SMsgProfile2SetReply(const SMsgProfile2SetReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			mFields(theMsgR.mFields), SmallMessage(theMsgR)
	{ }

	SMsgProfile2SetReply& operator=(const SMsgProfile2SetReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		mFields = theMsgR.mFields;
		return *this;
	}

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	///////////
	// SetField

	void SetField(int i, const Profile2SetReplyInfo& info)
	{ mFields.insert(std::multimap<int, Profile2SetReplyInfo>::value_type(i, info)); }

	void SetField(int i, ServerStatus err, const std::string& errString = std::string())
	{ SetField(i, Profile2SetReplyInfo(err, errString)); }


	///////////
	// GetField

	const Profile2SetReplyInfo* GetField(int i) const
	{
		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}

	bool GetField(int i, Profile2SetReplyInfo& sField) const
	{
		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}
	const std::string* GetFieldError(int i) const
	{
		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second.mErrorString);
	}
	bool GetFieldError(int i, std::string& errString) const
	{
		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		errString = (*itor).second.mErrorString;
		return true;
	}

	bool GetFieldStatus(int i, ServerStatus& status) const
	{
		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.find(i);
		if (itor == mFields.end())
			return false;
		status = (*itor).second.mErrorCode;
		return true;
	}

	///////////////////////////////////////////////////////////////////
	// for newsletters: retrieving each element withing newsletter list

	int GetSize(int i) const { return mFields.count(i); }

	const Profile2SetReplyInfo* GetField(int i, int j) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return 0;

		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return 0;
		return &((*itor).second);
	}
	bool GetField(int i, int j, Profile2SetReplyInfo& sField) const
	{
		assert((j >= 0) && (j <= GetSize(i)));
		if (!((j >= 0) && (j <= GetSize(i)))) return false;

		std::multimap<int, Profile2SetReplyInfo>::const_iterator itor = mFields.lower_bound(i);
		for (; j > 0; --j, ++itor);
		if (itor == mFields.end())
			return false;
		sField = (*itor).second;
		return true;
	}

	//

	void ClearField(int i)	{ mFields.erase(i); }
	void RemoveField(int i)	{ mFields.erase(i); }

	const std::multimap<int, Profile2SetReplyInfo>& GetFields() const { return mFields; }

	const Profile2SetReplyInfo* GetGender()		{ return GetField(PROFILE_GENDER); }
	const Profile2SetReplyInfo* GetBirthdate()	{ return GetField(PROFILE_BIRTHDATE); }
	const Profile2SetReplyInfo* GetFirstName()	{ return GetField(PROFILE_FIRSTNAME); }
	const Profile2SetReplyInfo* GetLastName()	{ return GetField(PROFILE_LASTNAME); }
	const Profile2SetReplyInfo* GetMiddleName()	{ return GetField(PROFILE_MIDDLENAME); }
	const Profile2SetReplyInfo* GetCompanyName(){ return GetField(PROFILE_COMPANYNAME); }
	const Profile2SetReplyInfo* GetAddress1()	{ return GetField(PROFILE_ADDRESS1); }
	const Profile2SetReplyInfo* GetAddress2()	{ return GetField(PROFILE_ADDRESS2); }
	const Profile2SetReplyInfo* GetCity()		{ return GetField(PROFILE_CITY); }
	const Profile2SetReplyInfo* GetPostalCode()	{ return GetField(PROFILE_POSTALCODE); }
	const Profile2SetReplyInfo* GetState()		{ return GetField(PROFILE_STATE); }
	const Profile2SetReplyInfo* GetPhone1()		{ return GetField(PROFILE_PHONE1); }
	const Profile2SetReplyInfo* GetPhone1Ext()	{ return GetField(PROFILE_PHONE1EXT); }
	const Profile2SetReplyInfo* GetPhone2()		{ return GetField(PROFILE_PHONE2); }
	const Profile2SetReplyInfo* GetPhone2Ext()	{ return GetField(PROFILE_PHONE2EXT); }
	const Profile2SetReplyInfo* GetEmail1()		{ return GetField(PROFILE_EMAIL1); }
	const Profile2SetReplyInfo* GetEmail2()		{ return GetField(PROFILE_EMAIL2); }
	const Profile2SetReplyInfo* GetAgeRange()	{ return GetField(PROFILE_AGERANGE); }
	const Profile2SetReplyInfo* GetCountry()	{ return GetField(PROFILE_COUNTRY); }
	const Profile2SetReplyInfo* GetSource()		{ return GetField(PROFILE_SOURCE); }

	bool GetGender(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_GENDER, sField); }
	bool GetBirthdate(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_BIRTHDATE, sField); }
	bool GetFirstName(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_FIRSTNAME, sField); }
	bool GetLastName(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_LASTNAME, sField); }
	bool GetMiddleName(Profile2SetReplyInfo& sField)	{ return GetField(PROFILE_MIDDLENAME, sField); }
	bool GetCompanyName(Profile2SetReplyInfo& sField)	{ return GetField(PROFILE_COMPANYNAME, sField); }
	bool GetAddress1(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_ADDRESS1, sField); }
	bool GetAddress2(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_ADDRESS2, sField); }
	bool GetCity(Profile2SetReplyInfo& sField)			{ return GetField(PROFILE_CITY, sField); }
	bool GetPostalCode(Profile2SetReplyInfo& sField)	{ return GetField(PROFILE_POSTALCODE, sField); }
	bool GetState(Profile2SetReplyInfo& sField)			{ return GetField(PROFILE_STATE, sField); }
	bool GetPhone1(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_PHONE1, sField); }
	bool GetPhone1Ext(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_PHONE1EXT, sField); }
	bool GetPhone2(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_PHONE2, sField); }
	bool GetPhone2Ext(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_PHONE2EXT, sField); }
	bool GetEmail1(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_EMAIL1, sField); }
	bool GetEmail2(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_EMAIL2, sField); }
	bool GetAgeRange(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_AGERANGE, sField); }
	bool GetCountry(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_COUNTRY, sField); }
	bool GetSource(Profile2SetReplyInfo& sField)		{ return GetField(PROFILE_SOURCE, sField); }

	// newsletters
	int GetNewsletterSubsCount()						{ return GetSize(PROFILE_NEWSLETTERSUBS); }
	const Profile2SetReplyInfo* GetNewsletterSubs(int j = 0) 
														{ return GetField(PROFILE_NEWSLETTERSUBS, j); }
	bool GetNewsletterSubs(Profile2SetReplyInfo& sField, int j = 0) 
														{ return GetField(PROFILE_NEWSLETTERSUBS, j, sField); }

	////////////////
	// for debugging
	void ShowAllFields() {
		bool b;
		Profile2SetReplyInfo info;
		for (int i = PROFILE_GENDER; i < PROFILE_NEWSLETTERSUBS; ++i) 
			b = GetField(i, info);

		int count = GetNewsletterSubsCount();
		for (i = 0; i < count; ++i) 
			b = GetNewsletterSubs(info, i);

		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

	const std::string* GetGenderError()			{ return GetFieldError(PROFILE_GENDER); }
	const std::string* GetBirthdateError()		{ return GetFieldError(PROFILE_BIRTHDATE); }
	const std::string* GetFirstNameError()		{ return GetFieldError(PROFILE_FIRSTNAME); }
	const std::string* GetLastNameError()		{ return GetFieldError(PROFILE_LASTNAME); }
	const std::string* GetMiddleNameError()		{ return GetFieldError(PROFILE_MIDDLENAME); }
	const std::string* GetCompanyNameError()	{ return GetFieldError(PROFILE_COMPANYNAME); }
	const std::string* GetAddress1Error()		{ return GetFieldError(PROFILE_ADDRESS1); }
	const std::string* GetAddress2Error()		{ return GetFieldError(PROFILE_ADDRESS2); }
	const std::string* GetCityError()			{ return GetFieldError(PROFILE_CITY); }
	const std::string* GetPostalCodeError()		{ return GetFieldError(PROFILE_POSTALCODE); }
	const std::string* GetStateError()			{ return GetFieldError(PROFILE_STATE); }
	const std::string* GetPhone1Error()			{ return GetFieldError(PROFILE_PHONE1); }
	const std::string* GetPhone1ExtError()		{ return GetFieldError(PROFILE_PHONE1EXT); }
	const std::string* GetPhone2Error()			{ return GetFieldError(PROFILE_PHONE2); }
	const std::string* GetPhone2ExtError()		{ return GetFieldError(PROFILE_PHONE2EXT); }
	const std::string* GetEmail1Error()			{ return GetFieldError(PROFILE_EMAIL1); }
	const std::string* GetEmail2Error()			{ return GetFieldError(PROFILE_EMAIL2); }
	const std::string* GetAgeRangeError()		{ return GetFieldError(PROFILE_AGERANGE); }
	const std::string* GetCountryError()		{ return GetFieldError(PROFILE_COUNTRY); }
	const std::string* GetSourceError()			{ return GetFieldError(PROFILE_SOURCE); }

	bool GetGenderError(std::string& str)		{ return GetFieldError(PROFILE_GENDER, str); }
	bool GetBirthdateError(std::string& str)	{ return GetFieldError(PROFILE_BIRTHDATE, str); }
	bool GetFirstNameError(std::string& str)	{ return GetFieldError(PROFILE_FIRSTNAME, str); }
	bool GetLastNameError(std::string& str)		{ return GetFieldError(PROFILE_LASTNAME, str); }
	bool GetMiddleNameError(std::string& str)	{ return GetFieldError(PROFILE_MIDDLENAME, str); }
	bool GetCompanyNameError(std::string& str)	{ return GetFieldError(PROFILE_COMPANYNAME, str); }
	bool GetAddress1Error(std::string& str)		{ return GetFieldError(PROFILE_ADDRESS1, str); }
	bool GetAddress2Error(std::string& str)		{ return GetFieldError(PROFILE_ADDRESS2, str); }
	bool GetCityError(std::string& str)			{ return GetFieldError(PROFILE_CITY, str); }
	bool GetPostalCodeError(std::string& str)	{ return GetFieldError(PROFILE_POSTALCODE, str); }
	bool GetStateError(std::string& str)		{ return GetFieldError(PROFILE_STATE, str); }
	bool GetPhone1Error(std::string& str)		{ return GetFieldError(PROFILE_PHONE1, str); }
	bool GetPhone1ExtError(std::string& str)	{ return GetFieldError(PROFILE_PHONE1EXT, str); }
	bool GetPhone2Error(std::string& str)		{ return GetFieldError(PROFILE_PHONE2, str); }
	bool GetPhone2ExtError(std::string& str)	{ return GetFieldError(PROFILE_PHONE2EXT, str); }
	bool GetEmail1Error(std::string& str)		{ return GetFieldError(PROFILE_EMAIL1, str); }
	bool GetEmail2Error(std::string& str)		{ return GetFieldError(PROFILE_EMAIL2, str); }
	bool GetAgeRangeError(std::string& str)		{ return GetFieldError(PROFILE_AGERANGE, str); }
	bool GetCountryError(std::string& str)		{ return GetFieldError(PROFILE_COUNTRY, str); }
	bool GetSourceError(std::string& str)		{ return GetFieldError(PROFILE_SOURCE, str); }

	bool GetGenderStatus(ServerStatus& status)		{ return GetFieldStatus(PROFILE_GENDER, status); }
	bool GetBirthdateStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_BIRTHDATE, status); }
	bool GetFirstNameStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_FIRSTNAME, status); }
	bool GetLastNameStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_LASTNAME, status); }
	bool GetMiddleNameStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_MIDDLENAME, status); }
	bool GetCompanyNameStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_COMPANYNAME, status); }
	bool GetAddress1Status(ServerStatus& status)	{ return GetFieldStatus(PROFILE_ADDRESS1, status); }
	bool GetAddress2Status(ServerStatus& status)	{ return GetFieldStatus(PROFILE_ADDRESS2, status); }
	bool GetCityStatus(ServerStatus& status)		{ return GetFieldStatus(PROFILE_CITY, status); }
	bool GetPostalCodeStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_POSTALCODE, status); }
	bool GetStateStatus(ServerStatus& status)		{ return GetFieldStatus(PROFILE_STATE, status); }
	bool GetPhone1Status(ServerStatus& status)		{ return GetFieldStatus(PROFILE_PHONE1, status); }
	bool GetPhone1ExtStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_PHONE1EXT, status); }
	bool GetPhone2Status(ServerStatus& status)		{ return GetFieldStatus(PROFILE_PHONE2, status); }
	bool GetPhone2ExtStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_PHONE2EXT, status); }
	bool GetEmail1Status(ServerStatus& status)		{ return GetFieldStatus(PROFILE_EMAIL1, status); }
	bool GetEmail2Status(ServerStatus& status)		{ return GetFieldStatus(PROFILE_EMAIL2, status); }
	bool GetAgeRangeStatus(ServerStatus& status)	{ return GetFieldStatus(PROFILE_AGERANGE, status); }
	bool GetCountryStatus(ServerStatus& status)		{ return GetFieldStatus(PROFILE_COUNTRY, status); }
	bool GetSourceStatus(ServerStatus& status)		{ return GetFieldStatus(PROFILE_SOURCE, status); }


	void ClearGender()			{ mFields.erase(PROFILE_GENDER); }
	void ClearBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void ClearFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void ClearLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void ClearMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void ClearCompanyName()		{ mFields.erase(PROFILE_COMPANYNAME); }
	void ClearAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void ClearAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void ClearCity()			{ mFields.erase(PROFILE_CITY); }
	void ClearPostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void ClearState()			{ mFields.erase(PROFILE_STATE); }
	void ClearPhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void ClearPhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void ClearPhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void ClearPhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void ClearEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void ClearEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void ClearAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void ClearCountry()			{ mFields.erase(PROFILE_COUNTRY); }
	void ClearSource()			{ mFields.erase(PROFILE_SOURCE); }
	void ClearNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	void RemoveGender()			{ mFields.erase(PROFILE_GENDER); }
	void RemoveBirthdate()		{ mFields.erase(PROFILE_BIRTHDATE); }
	void RemoveFirstName()		{ mFields.erase(PROFILE_FIRSTNAME); }
	void RemoveLastName()		{ mFields.erase(PROFILE_LASTNAME); }
	void RemoveMiddleName()		{ mFields.erase(PROFILE_MIDDLENAME); }
	void RemoveCompanyName()	{ mFields.erase(PROFILE_COMPANYNAME); }
	void RemoveAddress1()		{ mFields.erase(PROFILE_ADDRESS1); }
	void RemoveAddress2()		{ mFields.erase(PROFILE_ADDRESS2); }
	void RemoveCity()			{ mFields.erase(PROFILE_CITY); }
	void RemovePostalCode()		{ mFields.erase(PROFILE_POSTALCODE); }
	void RemoveState()			{ mFields.erase(PROFILE_STATE); }
	void RemovePhone1()			{ mFields.erase(PROFILE_PHONE1); }
	void RemovePhone1Ext()		{ mFields.erase(PROFILE_PHONE1EXT); }
	void RemovePhone2()			{ mFields.erase(PROFILE_PHONE2); }
	void RemovePhone2Ext()		{ mFields.erase(PROFILE_PHONE2EXT); }
	void RemoveEmail1()			{ mFields.erase(PROFILE_EMAIL1); }
	void RemoveEmail2()			{ mFields.erase(PROFILE_EMAIL2); }
	void RemoveAgeRange()		{ mFields.erase(PROFILE_AGERANGE); }
	void RemoveCountry()		{ mFields.erase(PROFILE_COUNTRY); }
	void RemoveSource()			{ mFields.erase(PROFILE_SOURCE); }
	void RemoveNewsletterSubs()	{ mFields.erase(PROFILE_NEWSLETTERSUBS); }

	void SetErrorCode(ServerStatus status)	{ mErrorCode = status; }
	ServerStatus  GetErrorCode() const		{ return mErrorCode; }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	std::multimap<int, Profile2SetReplyInfo> mFields; // multimap used for multiple newsletter settings

private:
	void PackProfile2SetReplyInfo(const Profile2SetReplyInfo&);
	void UnpackProfile2SetReplyInfo(Profile2SetReplyInfo&);
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// ======================
// More specific messages

////////////////////////////////////////////////////////////////////////////////////

class SMsgProfile2GetCommunities : public SmallMessage
{
public:
	SMsgProfile2GetCommunities(bool b = true)	:	justMine(b)
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetCommunities);
	}

	explicit SMsgProfile2GetCommunities(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetCommunities);
		}
	}

	SMsgProfile2GetCommunities(const SMsgProfile2GetCommunities& theMsgR)
		:	justMine(theMsgR.justMine), SmallMessage(theMsgR)
	{ }

	SMsgProfile2GetCommunities& operator=(const SMsgProfile2GetCommunities& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		justMine = theMsgR.justMine;
		return *this;
	}

	
	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetAll()		{ justMine = false; }
	void SetMine()		{ justMine = true;  }

	bool GetAll()	const	{ return !justMine; }
	bool GetMine()	const	{ return justMine;  }

	virtual void* Pack(void);
    virtual void  Unpack(void);

private:
	bool justMine;		// false = all communities, true = just this user's
};



////////////////////////////////////////////////////////////////////////////////////

class SMsgProfile2GetCommunitiesReply : public SmallMessage
{
public:
	SMsgProfile2GetCommunitiesReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetCommunitiesReply);
	}

	explicit SMsgProfile2GetCommunitiesReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetCommunitiesReply);
		}
	}

	SMsgProfile2GetCommunitiesReply(const SMsgProfile2GetCommunitiesReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			SmallMessage(theMsgR), mCommunities(theMsgR.mCommunities)
	{ }

	SMsgProfile2GetCommunitiesReply& operator=(const SMsgProfile2GetCommunitiesReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		mCommunities = theMsgR.mCommunities;
		return *this;
	}

	void SetCommunity(long CommunitySeq, const std::wstring& CommunityName) { 
		mCommunities[CommunitySeq] = CommunityName; 
	}

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;
	
	const std::map<long, std::wstring>& GetCommunities() const
	{ return mCommunities; }

	std::map<long, std::wstring>& GetCommunities()
	{ return mCommunities; }

	void SetErrorCode(ServerStatus err)					{ mErrorCode = err; }
	ServerStatus GetErrorCode() const					{ return mErrorCode;   }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	////////////////
	// for debugging
	void ShowAllFields() {
		std::map<long, std::wstring>::const_iterator it = GetCommunities().begin(), end = GetCommunities().end();
		for (; it != end; ++it) { 
			long seq = it->first; 
			std::wstring str = it->second; 
		}
		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

private:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	std::map<long, std::wstring> mCommunities;
};

////////////////////////////////////////////////////////////////////////////////////


class SMsgProfile2GetNewsletters : public SmallMessage
{
public:
	SMsgProfile2GetNewsletters(bool b = true)	:	justMine(b)
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetNewsletters);
	}

	explicit SMsgProfile2GetNewsletters(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetNewsletters);
		}
	}

	SMsgProfile2GetNewsletters(const SMsgProfile2GetNewsletters& theMsgR)
		:	justMine(theMsgR.justMine), SmallMessage(theMsgR)
	{ }

	SMsgProfile2GetNewsletters& operator=(const SMsgProfile2GetNewsletters& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		justMine = theMsgR.justMine;
		return *this;
	}

	
	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetAll()		{ justMine = false; }
	void SetMine()		{ justMine = true;  }

	bool GetAll() const		{ return !justMine; }
	bool GetMine()	const	{ return justMine;  }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	bool justMine;		// false = all Newsletters, true = just this user's
};


////////////////////////////////////////////////////////////////////////////////////


struct Profile2NewsletterInfo
{
	//std::wstring mName;	// not necessary, key in map
	std::wstring mDesc;
	std::set<std::wstring> mCommunitySet; // a newsletter may belong to 0 or more communities

	Profile2NewsletterInfo(const std::wstring& desc, const std::wstring& community) : mDesc(desc)
		{ AddCommunity(community); }

	// Not all newletters have an associated community
	Profile2NewsletterInfo(const std::wstring& desc) : mDesc(desc) {}

	Profile2NewsletterInfo() {}

	void AddCommunity(const std::wstring& community) { mCommunitySet.insert(community); }
};


class SMsgProfile2GetNewslettersReply : public SmallMessage
{
public:
	SMsgProfile2GetNewslettersReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2SetNewslettersReply);
	}

	explicit SMsgProfile2GetNewslettersReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2SetNewslettersReply);
		}
	}

	SMsgProfile2GetNewslettersReply(const SMsgProfile2GetNewslettersReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			mNewsletters(theMsgR.mNewsletters), SmallMessage(theMsgR)
	{ }

	SMsgProfile2GetNewslettersReply& operator=(const SMsgProfile2GetNewslettersReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		mNewsletters = theMsgR.mNewsletters;
		return *this;
	}

	bool GetField(const std::wstring& newsletter, Profile2NewsletterInfo& info) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		info = itor->second;
		return true;
	}

	const Profile2NewsletterInfo* GetNewsletterInfo(const std::wstring& newsletter) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return 0;
		return &((*itor).second);
	}

	bool GetNewsletterInfo(const std::wstring& newsletter, Profile2NewsletterInfo& info) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		info = ((*itor).second);
		return true;
	}

	const std::wstring* GetNewsletterDesc(const std::wstring& newsletter) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return 0;
		return &((*itor).second.mDesc);
	}

	bool GetNewsletterDesc(const std::wstring& newsletter, std::wstring& desc) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		desc = ((*itor).second.mDesc);
		return true;
	}

	const std::set<std::wstring>* GetNewsletterCommunity(const std::wstring& newsletter) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return 0;
		return &((*itor).second.mCommunitySet);
	}

	bool GetNewsletterCommunity(const std::wstring& newsletter, std::set<std::wstring>& rCommunitySet) const
	{
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		rCommunitySet = ((*itor).second.mCommunitySet);
		return true;
	}

	const std::map<std::wstring, Profile2NewsletterInfo>& GetNewsletters() const
	{ return mNewsletters; }

	void SetNewsletter(const std::wstring& newsletter, const Profile2NewsletterInfo& info)
	{ mNewsletters[newsletter] = info; }

	void SetNewsletter(const std::wstring& newsletter, const std::wstring& desc, const std::wstring& community)
	{ SetNewsletter(newsletter, Profile2NewsletterInfo(desc, community)); }

	void SetNewsletter(const std::wstring& newsletter, const std::wstring& desc)
	{ SetNewsletter(newsletter, Profile2NewsletterInfo(desc)); }

	void SetErrorCode(ServerStatus status)	{ mErrorCode = status; }
	ServerStatus  GetErrorCode() const		{ return mErrorCode; }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	////////////////
	// for debugging
	void ShowAllFields() {	
		int count = mNewsletters.size();
		std::map<std::wstring, Profile2NewsletterInfo>::const_iterator it = mNewsletters.begin(), end = mNewsletters.end();
		for (; it != end; ++it) {
			std::wstring str = it->first;
			Profile2NewsletterInfo info = it->second;
			std::wstring str2 = info.mDesc;
			int count2 = info.mCommunitySet.size();
			std::set<std::wstring>::const_iterator it2 = info.mCommunitySet.begin(), end2 = info.mCommunitySet.end();
			for (; it2 != end2; ++it2) 
				std::wstring str3 = *it2;
		}
		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	std::map<std::wstring, Profile2NewsletterInfo> mNewsletters;

private:
	void PackProfile2NewsletterInfo(const Profile2NewsletterInfo&);
	void UnpackProfile2NewsletterInfo(Profile2NewsletterInfo&);
};


////////////////////////////////////////////////////////////////////////////////////


class SMsgProfile2GetCommunityNewsletters : public SmallMessage
{
public:
	SMsgProfile2GetCommunityNewsletters(const std::wstring& community, bool b = true)	:	
	  justMine(b), mCommunity(community)
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetCommunityNewsletters);
	}

	explicit SMsgProfile2GetCommunityNewsletters(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetCommunityNewsletters);
		}
	}

	SMsgProfile2GetCommunityNewsletters(const SMsgProfile2GetCommunityNewsletters& theMsgR)
		:	justMine(theMsgR.justMine), mCommunity(theMsgR.mCommunity), SmallMessage(theMsgR)
	{ }

	SMsgProfile2GetCommunityNewsletters& operator=(const SMsgProfile2GetCommunityNewsletters& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		justMine = theMsgR.justMine;
		mCommunity = theMsgR.mCommunity;
		return *this;
	}

	
	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	void SetAll()				{ justMine = false; }
	void SetMine()				{ justMine = true;  }

	bool GetAll() const				{ return !justMine; }
	bool GetMine() const			{ return justMine; }

	void SetCommunity(const std::wstring& community)	{ mCommunity = community;  }
	const std::wstring& GetCommunity() const					{ return mCommunity; }

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	bool justMine;		// false = all Newsletters, true = just this user's
	std::wstring mCommunity;
};


////////////////////////////////////////////////////////////////////////////////////


class SMsgProfile2GetCommunityNewslettersReply : public SmallMessage
{
public:
	SMsgProfile2GetCommunityNewslettersReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2GetCommunityNewslettersReply);
	}

	explicit SMsgProfile2GetCommunityNewslettersReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2GetCommunityNewslettersReply);
		}
	}

	SMsgProfile2GetCommunityNewslettersReply(const SMsgProfile2GetCommunityNewslettersReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			SmallMessage(theMsgR), mNewsletters(theMsgR.mNewsletters)
	{ }

	SMsgProfile2GetCommunityNewslettersReply& operator=(const SMsgProfile2GetCommunityNewslettersReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		mNewsletters = theMsgR.mNewsletters;
		return *this;
	}

	const std::wstring* GetNewsletterDesc(const std::wstring& newsletter) const
	{
		std::map<std::wstring, std::wstring>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return 0;
		return &((*itor).second);
	}

	bool GetNewsletterDesc(const std::wstring& newsletter, std::wstring& desc) const
	{
		std::map<std::wstring, std::wstring>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		desc = ((*itor).second);
		return true;
	}

	const std::map<std::wstring, std::wstring>& GetNewsletters() const
	{ return mNewsletters; }

	void SetNewsletter(const std::wstring& newsletter, const std::wstring& desc)
	{ mNewsletters[newsletter] = desc; }

	void SetErrorCode(ServerStatus status)	{ mErrorCode = status; }
	ServerStatus  GetErrorCode() const		{ return mErrorCode; }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	////////////////
	// for debugging
	void ShowAllFields() {	
		int count = mNewsletters.size();
		std::map<std::wstring, std::wstring>::const_iterator it = mNewsletters.begin(), end = mNewsletters.end();
		for (; it != end; ++it) {
			std::wstring str1 = it->first;
			std::wstring str2 = it->second;
		}
		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	// a community may have 0 or more newsletters, hence the map
	std::map<std::wstring, std::wstring> mNewsletters; // newsletter names and descriptions
};


////////////////////////////////////////////////////////////////////////////////////


class SMsgProfile2SetNewsletters : public SmallMessage
{
public:
	SMsgProfile2SetNewsletters()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2SetNewsletters);
	}

	explicit SMsgProfile2SetNewsletters(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2SetNewsletters);
		}
	}

	SMsgProfile2SetNewsletters(const SMsgProfile2SetNewsletters& theMsgR)
		:	SmallMessage(theMsgR), mNewsletters(theMsgR.mNewsletters)
	{ }

	SMsgProfile2SetNewsletters& operator=(const SMsgProfile2SetNewsletters& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mNewsletters = theMsgR.mNewsletters;
		return *this;
	}

	const bool* GetNewsletter(const std::wstring& newsletter) const
	{
		std::map<std::wstring, bool>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return 0;
		return &((*itor).second);
	}
	bool GetNewsletter(const std::wstring& newsletter, bool& subscribe) const
	{
		std::map<std::wstring, bool>::const_iterator itor = mNewsletters.find(newsletter);
		if (itor == mNewsletters.end())
			return false;
		subscribe = (*itor).second;
		return true;
	}

	void SetNewsletter(const std::wstring& newsletter, bool b = true)	{ mNewsletters[newsletter] = b; }

	// second param is dummy (but needed for server testing)
	void SetField(const std::wstring& newsletter, const Profile2NewsletterInfo&) { mNewsletters[newsletter] = true; }

	void erase(const std::wstring& newsletter)
	{
		std::map<std::wstring, bool>::iterator itor = mNewsletters.find(newsletter);
		if (itor != mNewsletters.end())
			mNewsletters.erase(itor);
	}
	void clear()								{ mNewsletters.clear(); }

	const std::map<std::wstring, bool>& GetNewsletters() const
	{ return mNewsletters; }
	
	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

protected:
	std::map<std::wstring, bool> mNewsletters;
};


////////////////////////////////////////////////////////////////////////////////////


class SMsgProfile2SetNewslettersReply : public SmallMessage
{
public:
	SMsgProfile2SetNewslettersReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2SetNewslettersReply);
	}

	explicit SMsgProfile2SetNewslettersReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2SetNewslettersReply);
		}
	}

	SMsgProfile2SetNewslettersReply(const SMsgProfile2SetNewslettersReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			SmallMessage(theMsgR), mResults(theMsgR.mResults)
	{ }
	
	SMsgProfile2SetNewslettersReply& operator=(const SMsgProfile2SetNewslettersReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		mResults = theMsgR.mResults;
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		return *this;
	}


	void SetErrorCode(ServerStatus err)					{ mErrorCode = err; }
	ServerStatus GetErrorCode() const					{ return mErrorCode;   }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetFieldErrorString(const std::wstring& newsletter, const std::string& aString)
		{ mResults[newsletter] = aString;  }
	bool GetFieldErrorString(const std::wstring& newsletter, std::string& aString) const
	{
		std::map<std::wstring, std::string>::const_iterator itor = mResults.find(newsletter);
		if (itor != mResults.end()) {
			aString = (*itor).second;
			return true;
		}
		return false;
	}

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	void clear()			{ mResults.clear(); }
	void clear(const std::wstring& newsletter)
	{
		std::map<std::wstring, std::string>::iterator itor = mResults.find(newsletter);
		if (itor != mResults.end())
			mResults.erase(itor);
	}

	// Virtual Duplicate from BaseMessage
	virtual TRawMsg* Duplicate(void) const;

	virtual void* Pack(void);
    virtual void  Unpack(void);

	////////////////
	// for debugging
	void ShowAllFields() {	
		int count = mResults.size();
		std::map<std::wstring, std::string>::const_iterator it = mResults.begin(), end = mResults.end();
		for (; it != end; ++it) {
			std::wstring str1 = it->first;
			std::string str2 = it->second;
		}
		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

protected:
	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;
	std::map<std::wstring, std::string> mResults; // the string is an error string (if applicable)
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class SMsgProfile2DownloadCommunityTree : public SmallMessage
{
public:
	SMsgProfile2DownloadCommunityTree()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2DownloadCommunityTree);
	}

	explicit SMsgProfile2DownloadCommunityTree(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
			{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2DownloadCommunityTree);
			}
	}

	SMsgProfile2DownloadCommunityTree(const SMsgProfile2DownloadCommunityTree& theMsgR) 
		: SmallMessage(theMsgR) {}
	SMsgProfile2DownloadCommunityTree& operator=(const SMsgProfile2DownloadCommunityTree& theMsgR)
	{ SmallMessage::operator=(theMsgR); return *this; }

	virtual TRawMsg* Duplicate() const {
		return new SMsgProfile2DownloadCommunityTree(*this); 
	}

	virtual void* Pack() {
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2DownloadCommunityTree);
		SmallMessage::Pack();
		return GetDataPtr();
	}
    virtual void  Unpack() { SmallMessage::Unpack(); }
};

////////////////////////////////////////////////////////////////////////////////////

//////////////////////
struct CommunityData {
	long CommunitySeq;
	long TrustLevelId;
	std::wstring Name;
	std::wstring Descr;
	long JoinByInvitationOnly;
	long ParentCommunitySeq;
	std::vector<long> ChildrenSeq;
};

class SMsgProfile2DownloadCommunityTreeReply : public SmallMessage
{
public:
	SMsgProfile2DownloadCommunityTreeReply()
	{
		SetServiceType(WONMsg::SmallProfileServer);
		SetMessageType(WONMsg::Profile2DownloadCommunityTreeReply);
	}

	explicit SMsgProfile2DownloadCommunityTreeReply(const SmallMessage& theMsgR, bool doUnpack = true)
		: SmallMessage(theMsgR)
	{
		if (doUnpack)
			Unpack();
		else
		{
			SetServiceType(WONMsg::SmallProfileServer);
			SetMessageType(WONMsg::Profile2DownloadCommunityTreeReply);
		}
	}

	SMsgProfile2DownloadCommunityTreeReply(const SMsgProfile2DownloadCommunityTreeReply& theMsgR)
		:	APIErrorCode(theMsgR.APIErrorCode), mErrorCode(theMsgR.mErrorCode), mErrorString(theMsgR.mErrorString),
			SmallMessage(theMsgR), data(theMsgR.data), databyname(theMsgR.databyname), rootcommunities(theMsgR.rootcommunities)
	{}

	SMsgProfile2DownloadCommunityTreeReply& operator=(const SMsgProfile2DownloadCommunityTreeReply& theMsgR)
	{
		SmallMessage::operator=(theMsgR);
		APIErrorCode = theMsgR.APIErrorCode;
		mErrorCode = theMsgR.mErrorCode;
		mErrorString = theMsgR.mErrorString;
		data = theMsgR.data;
		databyname = theMsgR.databyname;
		rootcommunities = theMsgR.rootcommunities;
		return *this;
	}

	virtual TRawMsg* Duplicate(void) const {
		return new SMsgProfile2DownloadCommunityTreeReply(*this); 
	}
	virtual void* Pack(void);
    virtual void  Unpack(void);

	void SetErrorCode(ServerStatus err)				{ mErrorCode = err; }
	ServerStatus GetErrorCode() const				{ return mErrorCode;   }

	void SetAPIErrorCode(long err)					{ APIErrorCode = err; }
	long GetAPIErrorCode() const					{ return APIErrorCode; }

	void SetErrorString(const std::string& rs)		{ mErrorString = rs; }
	const std::string& GetErrorString() const		{ return mErrorString; }

	void AddData(const CommunityData& info) {
		data.insert(std::map<long, CommunityData>::value_type(info.CommunitySeq, info));
		databyname.insert(std::map<std::wstring, long>::value_type(info.Name, info.CommunitySeq));
	}
	void AddRoot(long seq) { rootcommunities.push_back(seq); }

	const std::map<long, CommunityData>& GetData()	const				{ return data; }
	const std::map<std::wstring, long>& GetDataByName() const	{ return databyname; }
	const std::vector<long>& GetRoots() const								{ return rootcommunities; }

	////////////////
	// for debugging
	void ShowAllFields() {
		int i = data.size();
		int j = databyname.size();
		int k = rootcommunities.size();

		std::map<long, CommunityData>::const_iterator it = data.begin(), end = data.end();
		for (; it != end; ++it) {
			long i1 = it->first;
			CommunityData info = it->second;
		}

		std::map<std::wstring, long>::const_iterator it2 = databyname.begin(), end2 = databyname.end();
		for (; it2 != end2; ++it2) {
			std::wstring str = it2->first;
			long i2 = it2->second;
		}

		std::vector<long>::const_iterator it3 = rootcommunities.begin(), end3 = rootcommunities.end();
		for (; it3 != end3; ++it3) long i3 = *it3;

		short errorcode = GetErrorCode();
		std::string	 errorstring = GetErrorString();
	}

private:
	void PackCommunityData(const CommunityData&);
	void UnpackCommunityData(std::map<long, CommunityData>::iterator&);

	long APIErrorCode;
	ServerStatus mErrorCode;
	std::string	 mErrorString;

	std::map<long, CommunityData>			data;
	std::map<std::wstring, long>			databyname;
	std::vector<long>						rootcommunities;
};

};

#endif //__SMSGPROFILEMSGS_H__
