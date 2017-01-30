#pragma warning (disable : 4786)
#include <list>
#include "ProfileAPI.h"
#include "msg/Profile/SMsgProfileMsgs.h"
#include "common/Event.h"
#include "Authentication/AuthSocket.h"

using namespace WONAPI;
using namespace WONCommon;
using namespace WONMsg;
using namespace std;

typedef list<IPSocket::Address> AddressList;

class ProfileData {
public:
	Error					error;
	string					emailAddress;

	bool					autoDelete;
	long					timeout;
	Event					doneEvent;
	AddressList				profileServers;
	AddressList::iterator	curServer;
	AuthSocket*				profileSocket;

	~ProfileData()
	{
		delete profileSocket;
	}

	void Done()
	{
		if (autoDelete)
			delete this;
		else
			doneEvent.Set();
	}

	void TryNextProfileServer();
};

void ProfileCreateCallback(const AuthSocket::AuthRecvBaseMsgResult& theResultR, ProfileData* profileData)
{
	((AuthSocket*)theResultR.theSocket)->GetSocket()->Close();
	if (theResultR.msg)
	{
		auto_ptr<BaseMessage> autoMsg(theResultR.msg);
		SMsgProfileCreateResponse aCreateResponse(*(SmallMessage*)theResultR.msg);
		profileData->error = aCreateResponse.GetErrorCode();
		switch (profileData->error)
		{
			case StatusCommon_Success:
			case StatusCommon_AlreadyExists:
				profileData->Done(); break;
			default:
				profileData->TryNextProfileServer(); break;
		}
	}
}

void SendCreateProfileRequest(ProfileData* profileData, const char* emailAddress)
{
	SMsgProfileCreateRequest aCreateRequest;
	aCreateRequest.SetField(PROFILE_EMAIL1, StringToWString(emailAddress));
	profileData->profileSocket->SendBaseMsgRequestEx(aCreateRequest, NULL, false, true, 2000, -1, true, true, (void(*)(const AuthSocket::AuthRecvBaseMsgResult&, ProfileData*))ProfileCreateCallback, profileData);
}

void ProfileData::TryNextProfileServer()
{
	if (++curServer == profileServers.end())
		Done();
	else
	{
		profileSocket->Close(0);
		profileSocket->ReleaseSession();
		profileSocket->GetSocket()->SetRemoteAddress(*curServer);
		SendCreateProfileRequest(this, this->emailAddress.c_str());
	}
}

Error WONAPI::CreateProfile(Identity* identity, const IPSocket::Address* profileServers, unsigned int numAddrs,
                    const char* emailAddress, long timeout, bool async, const CompletionContainer<Error>& completion) 
{
	ProfileData* profileData = new ProfileData;
	if (profileData)
	{
		TCPSocket* tcpSocket = new TCPSocket(profileServers[0]);
		if (tcpSocket)
		{
			profileData->profileSocket = new AuthSocket(identity, tcpSocket, true, 2, false, false);
			if (profileData->profileSocket)
			{
				profileData->emailAddress = emailAddress;
				profileData->error = Error_Timeout;
				profileData->timeout = timeout;
				profileData->autoDelete = async;
//				profileData->dirResultCompletion = completion;

				for (unsigned int i = 0; i < numAddrs; i++)
					profileData->profileServers.push_back(profileServers[i]);
				profileData->curServer = profileData->profileServers.begin();

				SendCreateProfileRequest(profileData, emailAddress);

				Error err = Error_Pending;

				if (!async)
				{
					WSSocket::PumpUntil(profileData->doneEvent, timeout);
					err = profileData->error;
					delete profileData;
				}
				return err;
			}
			delete tcpSocket;
		}
		delete profileData;
	}
	completion.Complete(Error_OutOfMemory);
	return Error_OutOfMemory;
}

WONError WONProfileCreate(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numAddrs, 
                          WON_CONST char* emailAddress, long timeout)
{
	Error err = Error_InvalidParams;
	if (numAddrs && profileSrvrs)
	{
		IPSocket::Address* profileServAddrs = new IPSocket::Address[numAddrs];
		if (!profileServAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoProfileServAddrs(profileServAddrs);

			for (int i = 0; i < numAddrs; i++)
				profileServAddrs[i].Set(profileSrvrs[i].ipAddress, profileSrvrs[i].portNum);

			err = WONAPI::CreateProfile((Identity*)hAuth, profileServAddrs, numAddrs, emailAddress, timeout, false);
		}
	}
	return err;
}

static void TranslateWONCompletionWithError(Error err, HWONCOMPLETION hCompletion)
{
	WONComplete(hCompletion, (void*)err);
}

void WONProfileCreateAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numAddrs,
                           WON_CONST char* emailAddress, long timeout, HWONCOMPLETION hCompletion)
{
	Error err = Error_InvalidParams;
	if (numAddrs && profileSrvrs)
	{
		IPSocket::Address* profileServAddrs = new IPSocket::Address[numAddrs];
		if (!profileServAddrs)
			err = Error_OutOfMemory;
		else
		{
			array_auto_ptr<IPSocket::Address> autoProfileServAddrs(profileServAddrs);

			for (int i = 0; i < numAddrs; i++)
				profileServAddrs[i].Set(profileSrvrs[i].ipAddress, profileSrvrs[i].portNum);

			err = WONAPI::CreateProfileEx((Identity*)hAuth, profileServAddrs, numAddrs, 
                                emailAddress, timeout, true, TranslateWONCompletionWithError, hCompletion);
			return;
		}
	}
	WONComplete(hCompletion, (void*)err);
}

/*

Error WONAPI::CreateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
							const Profile& profile, long sourceSeq, bool wantsPromos, 
							long timeout, bool async, const CompletionContainer<Error>& completion)
{
	completion.Complete(Error_Unimplemented);
	return Error_Unimplemented;
}


Error WONAPI::GetProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
						 Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion)
{
	completion.Complete(Error_Unimplemented);
	return Error_Unimplemented;
}


Error WONAPI::UpdateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
							const Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion)
{
	completion.Complete(Error_Unimplemented);
	return Error_Unimplemented;
}


#include "wondll.h"


HWONPROFILE WONProfileCreate()
{
	return (HWONPROFILE)new Profile;
}


void WONProfileCloseHandle(HWONPROFILE hProfile)
{
	delete (Profile*)hProfile;
}


void WONProfileSet(HWONPROFILE hProfile, unsigned short index, WON_CONST void* data, unsigned short size)
{
	Profile* profile = (Profile*)hProfile;
	profile->Set(index, data, size);
}


WON_CONST void* WONProfileGet(HWONPROFILE hProfile, unsigned short index, unsigned short* size)
{
	Profile* profile = (Profile*)hProfile;
	return (WON_CONST void*)profile->Get(index, size);
}


void WONProfileRemove(HWONPROFILE hProfile, unsigned short index)
{
	Profile* profile = (Profile*)hProfile;
	profile->Remove(index);
}


void WONProfileSetBool(HWONPROFILE hProfile, unsigned short index, BOOL b)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetBool(index, b ? true : false);
}


void WONProfileSetShort(HWONPROFILE hProfile, unsigned short index, short s)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetShort(index, s);
}


void WONProfileSetLong(HWONPROFILE hProfile, unsigned short index, long l)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetLong(index, l);
}


void WONProfileSetStringA(HWONPROFILE hProfile, unsigned short index, WON_CONST char* str)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetString(index, str);
}


void WONProfileSetStringW(HWONPROFILE hProfile, unsigned short index, WON_CONST wchar_t* str)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetString(index, str);
}


BOOL WONProfileGetBool(HWONPROFILE hProfile, unsigned short index)
{
	Profile* profile = (Profile*)hProfile;
	return profile->GetBool(index) ? TRUE : FALSE;
}


long WONProfileGetLong(HWONPROFILE hProfile, unsigned short index)
{
	Profile* profile = (Profile*)hProfile;
	return profile->GetLong(index);
}


short WONProfileGetShort(HWONPROFILE hProfile, unsigned short index)
{
	Profile* profile = (Profile*)hProfile;
	return profile->GetShort(index);
}


void WONProfileGetStringA(HWONPROFILE hProfile, unsigned short index, char* str, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetString(index, str, strBufLength);
}


void WONProfileGetStringW(HWONPROFILE hProfile, unsigned short index, wchar_t* str, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetString(index, str, strBufLength);
}


void WONProfileSetGenderA(HWONPROFILE hProfile, WON_CONST char* gender)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetGender(gender);
}


void WONProfileSetGenderW(HWONPROFILE hProfile, WON_CONST wchar_t* gender)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetGender(gender);
}


void WONProfileSetBirthdate(HWONPROFILE hProfile, time_t birthdate)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetBirthdate(birthdate);
}


void WONProfileSetFirstNameA(HWONPROFILE hProfile, WON_CONST char* firstName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetFirstName(firstName);
}


void WONProfileSetFirstNameW(HWONPROFILE hProfile, WON_CONST wchar_t* firstName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetFirstName(firstName);
}


void WONProfileSetLastNameA(HWONPROFILE hProfile, WON_CONST char* lastName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetLastName(lastName);
}


void WONProfileSetLastNameW(HWONPROFILE hProfile, WON_CONST wchar_t* lastName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetLastName(lastName);
}


void WONProfileSetMiddleNameA(HWONPROFILE hProfile, WON_CONST char* middleName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetMiddleName(middleName);
}


void WONProfileSetMiddleNameW(HWONPROFILE hProfile, WON_CONST wchar_t* middleName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetMiddleName(middleName);
}


void WONProfileSetCompanyNameA(HWONPROFILE hProfile, WON_CONST char* companyName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetCompanyName(companyName);
}


void WONProfileSetCompanyNameW(HWONPROFILE hProfile, WON_CONST wchar_t* companyName)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetCompanyName(companyName);
}


void WONProfileSetAddress1A(HWONPROFILE hProfile, WON_CONST char* address1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetAddress1(address1);
}


void WONProfileSetAddress1W(HWONPROFILE hProfile, WON_CONST wchar_t* address1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetAddress1(address1);
}


void WONProfileSetAddress2A(HWONPROFILE hProfile, WON_CONST char* address2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetAddress2(address2);
}


void WONProfileSetAddress2W(HWONPROFILE hProfile, WON_CONST wchar_t* address2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetAddress2(address2);
}


void WONProfileSetCityA(HWONPROFILE hProfile, WON_CONST char* city)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetCity(city);
}


void WONProfileSetCityW(HWONPROFILE hProfile, WON_CONST wchar_t* city)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetCity(city);
}


void WONProfileSetPostalCodeA(HWONPROFILE hProfile, WON_CONST char* postalCode)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPostalCode(postalCode);
}


void WONProfileSetPostalCodeW(HWONPROFILE hProfile, WON_CONST wchar_t* postalCode)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPostalCode(postalCode);
}


void WONProfileSetStateA(HWONPROFILE hProfile, WON_CONST char* state)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetState(state);
}


void WONProfileSetStateW(HWONPROFILE hProfile, WON_CONST wchar_t* state)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetState(state);
}


void WONProfileSetPhone1A(HWONPROFILE hProfile, WON_CONST char* phone1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone1(phone1);
}


void WONProfileSetPhone1W(HWONPROFILE hProfile, WON_CONST wchar_t* phone1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone1(phone1);
}


void WONProfileSetPhone1ExtA(HWONPROFILE hProfile, WON_CONST char* phone1Ext)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone1Ext(phone1Ext);
}


void WONProfileSetPhone1ExtW(HWONPROFILE hProfile, WON_CONST wchar_t* phone1Ext)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone1Ext(phone1Ext);
}


void WONProfileSetPhone2A(HWONPROFILE hProfile, WON_CONST char* phone2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone2(phone2);
}


void WONProfileSetPhone2W(HWONPROFILE hProfile, WON_CONST wchar_t* phone2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone2(phone2);
}


void WONProfileSetPhone2ExtA(HWONPROFILE hProfile, WON_CONST char* phone2Ext)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone2Ext(phone2Ext);
}


void WONProfileSetPhone2ExtW(HWONPROFILE hProfile, WON_CONST wchar_t* phone2Ext)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetPhone2Ext(phone2Ext);
}


void WONProfileSetEmail1A(HWONPROFILE hProfile, WON_CONST char* email1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetEmail1(email1);
}


void WONProfileSetEmail1W(HWONPROFILE hProfile, WON_CONST wchar_t* email1)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetEmail1(email1);
}


void WONProfileSetEmail2A(HWONPROFILE hProfile, WON_CONST char* email2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetEmail2(email2);
}


void WONProfileSetEmail2W(HWONPROFILE hProfile, WON_CONST wchar_t* email2)
{
	Profile* profile = (Profile*)hProfile;
	profile->SetEmail2(email2);
}


void WONProfileGetGenderA(HWONPROFILE hProfile, char* gender, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetGender(gender, strBufLength);
}


void WONProfileGetGenderW(HWONPROFILE hProfile, wchar_t* gender, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetGender(gender, strBufLength);
}


time_t WONProfileGetBirthdate(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	return profile->GetBirthdate();
}


void WONProfileGetFirstNameA(HWONPROFILE hProfile, char* firstName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetFirstName(firstName, strBufLength);
}


void WONProfileGetFirstNameW(HWONPROFILE hProfile, wchar_t* firstName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetFirstName(firstName, strBufLength);
}


void WONProfileGetLastNameA(HWONPROFILE hProfile, char* lastName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetLastName(lastName, strBufLength);
}


void WONProfileGetLastNameW(HWONPROFILE hProfile, wchar_t* lastName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetLastName(lastName, strBufLength);
}


void WONProfileGetMiddleNameA(HWONPROFILE hProfile, char* middleName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetMiddleName(middleName, strBufLength);
}


void WONProfileGetMiddleNameW(HWONPROFILE hProfile, wchar_t* middleName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetMiddleName(middleName, strBufLength);
}


void WONProfileGetCompanyNameA(HWONPROFILE hProfile, char* companyName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetCompanyName(companyName, strBufLength);
}


void WONProfileGetCompanyNameW(HWONPROFILE hProfile, wchar_t* companyName, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetCompanyName(companyName, strBufLength);
}


void WONProfileGetAddress1A(HWONPROFILE hProfile, char* address1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetAddress1(address1, strBufLength);
}


void WONProfileGetAddress1W(HWONPROFILE hProfile, wchar_t* address1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetAddress1(address1, strBufLength);
}


void WONProfileGetAddress2A(HWONPROFILE hProfile, char* address2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetAddress2(address2, strBufLength);
}


void WONProfileGetAddress2W(HWONPROFILE hProfile, wchar_t* address2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetAddress2(address2, strBufLength);
}


void WONProfileGetCityA(HWONPROFILE hProfile, char* city, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetCity(city, strBufLength);
}


void WONProfileGetCityW(HWONPROFILE hProfile, wchar_t* city, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetCity(city, strBufLength);
}


void WONProfileGetPostalCodeA(HWONPROFILE hProfile, char* postalCode, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPostalCode(postalCode, strBufLength);
}


void WONProfileGetPostalCodeW(HWONPROFILE hProfile, wchar_t* postalCode, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPostalCode(postalCode, strBufLength);
}


void WONProfileGetStateA(HWONPROFILE hProfile, char* state, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetState(state, strBufLength);
}


void WONProfileGetStateW(HWONPROFILE hProfile, wchar_t* state, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetState(state, strBufLength);
}


void WONProfileGetPhone1A(HWONPROFILE hProfile, char* phone1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone1(phone1, strBufLength);
}


void WONProfileGetPhone1W(HWONPROFILE hProfile, wchar_t* phone1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone1(phone1, strBufLength);
}


void WONProfileGetPhone1ExtA(HWONPROFILE hProfile, char* phone1Ext, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone1Ext(phone1Ext, strBufLength);
}


void WONProfileGetPhone1ExtW(HWONPROFILE hProfile, wchar_t* phone1Ext, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone1Ext(phone1Ext, strBufLength);
}


void WONProfileGetPhone2A(HWONPROFILE hProfile, char* phone2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone2(phone2, strBufLength);
}


void WONProfileGetPhone2W(HWONPROFILE hProfile, wchar_t* phone2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone2(phone2, strBufLength);
}


void WONProfileGetPhone2ExtA(HWONPROFILE hProfile, char* phone2Ext, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone2Ext(phone2Ext, strBufLength);
}


void WONProfileGetPhone2ExtW(HWONPROFILE hProfile, wchar_t* phone2Ext, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetPhone2Ext(phone2Ext, strBufLength);
}


void WONProfileGetEmail1A(HWONPROFILE hProfile, char* email1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetEmail1(email1, strBufLength);
}


void WONProfileGetEmail1W(HWONPROFILE hProfile, wchar_t* email1, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetEmail1(email1, strBufLength);
}


void WONProfileGetEmail2A(HWONPROFILE hProfile, char* email2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetEmail2(email2, strBufLength);
}


void WONProfileGetEmail2W(HWONPROFILE hProfile, wchar_t* email2, unsigned short* strBufLength)
{
	Profile* profile = (Profile*)hProfile;
	profile->GetEmail2(email2, strBufLength);
}


void WONProfileRemoveGender(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveGender();
}


void WONProfileRemoveBirthdate(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveBirthdate();
}


void WONProfileRemoveFirstName(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveFirstName();
}


void WONProfileRemoveLastName(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveLastName();
}


void WONProfileRemoveMiddleName(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveMiddleName();
}


void WONProfileRemoveCompanyName(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveCompanyName();
}


void WONProfileRemoveAddress1(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveAddress1();
}


void WONProfileRemoveAddress2(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveAddress2();
}


void WONProfileRemoveCity(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveCity();
}


void WONProfileRemovePostalCode(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemovePostalCode();
}


void WONProfileRemoveState(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveState();
}


void WONProfileRemovePhone1(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemovePhone1();
}


void WONProfileRemovePhone1Ext(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemovePhone1Ext();
}


void WONProfileRemovePhone2(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemovePhone2();
}


void WONProfileRemovePhone2Ext(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemovePhone2Ext();
}


void WONProfileRemoveEmail1(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveEmail1();
}


void WONProfileRemoveEmail2(HWONPROFILE hProfile)
{
	Profile* profile = (Profile*)hProfile;
	profile->RemoveEmail2();
}




WONError WONProfileCreateAccount(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
								 HWONPROFILE hProfile, long sourceSeq, BOOL wantsPromos, long timeout)
{
	Profile* profile = (Profile*)hProfile;
	return Error_Unimplemented;
}



void WONProfileCreateAccountAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
								  HWONPROFILE hProfile, long sourceSeq, BOOL wantsPromos, long timeout,
								  HWONCOMPLETION hCompletion)
{
	Profile* profile = (Profile*)hProfile;
	WONComplete(hCompletion, (void*)Error_Unimplemented);
}




WONError WONProfileGetAccount(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
							  HWONPROFILE hProfile, long timeout)
{
	Profile* profile = (Profile*)hProfile;
	return Error_Unimplemented;
}



void WONProfileGetAccountAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
							   HWONPROFILE hProfile, long timeout, HWONCOMPLETION hCompletion)
{
	Profile* profile = (Profile*)hProfile;
	WONComplete(hCompletion, (void*)Error_Unimplemented);
}




WONError WONProfileUpdateAccount(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
								 HWONPROFILE hProfile, long timeout)
{
	Profile* profile = (Profile*)hProfile;
	return Error_Unimplemented;
}



void WONProfileUpdateAccountAsync(HWONAUTH hAuth, WON_CONST WONIPAddress* profileSrvrs, unsigned int numSrvrs,
								  HWONPROFILE hProfile, long timeout, HWONCOMPLETION hCompletion)
{
	Profile* profile = (Profile*)hProfile;
	WONComplete(hCompletion, (void*)Error_Unimplemented);
}


*/

