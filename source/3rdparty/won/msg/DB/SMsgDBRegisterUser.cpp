

#include "common/won.h"
#include "msg/BadMsgException.h"
#include "msg/SServiceTypes.h"
#include "SMsgTypesDB.h"
#include "SMsgDBRegisterUser.h"
#include "Crypt/BFSymmetricKey.h"

// Private namespace for using, types, and constants

using namespace WONMsg;

const unsigned char SMsgDBRegisterUser::mKeyData[16] = {0x36, 0xfd, 0x43, 0x3d, 0xf0, 0x16, 0x32, 0x04, 0x21, 0xc0, 0x7e, 0xdb, 0x66, 0x27, 0x9f, 0x9a};

SMsgDBRegisterUser::SMsgDBRegisterUser(void) : SmallMessage()
{
	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBRegisterUser);
}


SMsgDBRegisterUser::~SMsgDBRegisterUser(void)
{
}


SMsgDBRegisterUser::SMsgDBRegisterUser(const SMsgDBRegisterUser& theMsgR) : SmallMessage(theMsgR),
	mUserName(theMsgR.mUserName), mPassword(theMsgR.mPassword),
	mFirstName(theMsgR.mFirstName), mMiddleName(theMsgR.mMiddleName), mLastName(theMsgR.mLastName),
	mBirthDate(theMsgR.mBirthDate), mAgeRange(theMsgR.mAgeRange), mGender(theMsgR.mGender),
	mAddress1(theMsgR.mAddress1), mAddress2(theMsgR.mAddress2), mCity(theMsgR.mCity),
	mState(theMsgR.mState), mZip(theMsgR.mZip), mCountry(theMsgR.mCountry),
	mPhone1(theMsgR.mPhone1), mPhone2(theMsgR.mPhone2), mEMail(theMsgR.mEMail),
	mWONNewsletterSubscribed(theMsgR.mWONNewsletterSubscribed), mSubmissionDate(theMsgR.mSubmissionDate)
{
}


SMsgDBRegisterUser::SMsgDBRegisterUser(const SmallMessage& theMsgR) : SmallMessage(theMsgR)
{
	Unpack();
}


SMsgDBRegisterUser& SMsgDBRegisterUser::operator =(const SMsgDBRegisterUser& theMsgR)
{

	if(this == &theMsgR)
		return *this;

	SmallMessage::operator=(theMsgR);

	mUserName = theMsgR.mUserName;
	mPassword = theMsgR.mPassword;

	mFirstName = theMsgR.mFirstName;
	mMiddleName = theMsgR.mMiddleName;
	mLastName = theMsgR.mLastName;

	mBirthDate = theMsgR.mBirthDate;
	mAgeRange = theMsgR.mAgeRange;
	mGender = theMsgR.mGender;
	
	mAddress1 = theMsgR.mAddress1;
	mAddress2 = theMsgR.mAddress2;
	mCity = theMsgR.mCity;
	mState = theMsgR.mState;
	mZip = theMsgR.mZip;
	mCountry = theMsgR.mCountry;

	mPhone1 = theMsgR.mPhone1;
	mPhone2 = theMsgR.mPhone2;
	
	mEMail = theMsgR.mEMail;
	mWONNewsletterSubscribed = theMsgR.mWONNewsletterSubscribed;

	mSubmissionDate = theMsgR.mSubmissionDate;

	return *this;

}


void* SMsgDBRegisterUser::Pack(void)
{
	WTRACE("SMsgDBRegisterUser::Pack");

	SetServiceType(WONMsg::SmallCommonService);
	SetMessageType(WONMsg::DBRegisterUser);
	SmallMessage::Pack();

	WDBG_LL("SMsgDBRegisterUser::Pack Appending message data");

	Append_PA_STRING(mUserName);

	WONCrypt::BFSymmetricKey aBFKey(16, mKeyData);
	WONCrypt::CryptKeyBase::CryptReturn aPassword = aBFKey.Encrypt(mPassword.data(), mPassword.length());
	AppendShort(aPassword.second);
	AppendBytes(aPassword.second, aPassword.first);
	delete [] aPassword.first;

	Append_PA_STRING(mFirstName);
	Append_PA_STRING(mMiddleName);
	Append_PA_STRING(mLastName);

	Append_PA_STRING(mBirthDate);
	Append_PA_STRING(mAgeRange);
	Append_PA_STRING(mGender);

	Append_PA_STRING(mAddress1);
	Append_PA_STRING(mAddress2);
	Append_PA_STRING(mCity);
	Append_PA_STRING(mState);
	Append_PA_STRING(mZip);
	Append_PA_STRING(mCountry);

	Append_PA_STRING(mPhone1);
	Append_PA_STRING(mPhone2);

	Append_PA_STRING(mEMail);
	Append_PA_STRING(mWONNewsletterSubscribed);

	Append_PA_STRING(mSubmissionDate);

	return GetDataPtr();
}


void SMsgDBRegisterUser::Unpack(void)
{
	WTRACE("SMsgDBRegisterUser::Unpack");
	SmallMessage::Unpack();

	if ((GetServiceType() != WONMsg::SmallCommonService) ||
	    (GetMessageType() != WONMsg::DBRegisterUser))
	{
		WDBG_AH("SMsgDBRegisterUser::Unpack Not a DBRegisterUser message!");
		throw WONMsg::BadMsgException(*this, __LINE__, __FILE__,
		                              "Not a DBRegisterUser message.");
	}

	WDBG_LL("SMsgDBRegisterUser::Unpack Reading message data");

	ReadString(mUserName);

	unsigned short aLength = ReadShort();
	WONCommon::RawBuffer aPasswordBuffer;
	aPasswordBuffer.assign((unsigned char*)ReadBytes(aLength), aLength);

	WONCrypt::BFSymmetricKey aBFKey(16, mKeyData);
	WONCrypt::CryptKeyBase::CryptReturn aPassword = aBFKey.Decrypt(aPasswordBuffer.data(), aPasswordBuffer.length());
	mPassword.assign((char*)aPassword.first, aPassword.second);
	delete [] aPassword.first;

	ReadString(mFirstName);
	ReadString(mMiddleName);
	ReadString(mLastName);

	ReadString(mBirthDate);
	ReadString(mAgeRange);
	ReadString(mGender);

	ReadString(mAddress1);
	ReadString(mAddress2);
	ReadString(mCity);
	ReadString(mState);
	ReadString(mZip);
	ReadString(mCountry);

	ReadString(mPhone1);
	ReadString(mPhone2);

	ReadString(mEMail);
	ReadString(mWONNewsletterSubscribed);

	ReadString(mSubmissionDate);
}
