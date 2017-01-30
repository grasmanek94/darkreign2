#ifndef __WON_PROFILE_API_H__
#define __WON_PROFILE_API_H__


#include "Socket/IPSocket.h"
#include "Authentication/Identity.h"

namespace WONAPI {

Error CreateProfile(Identity* identity, const IPSocket::Address* profileServers, unsigned int numSrvrs,
                    const char* emailAddress, long timeout =-1, bool async =false, const CompletionContainer<Error>& completion =DEFAULT_COMPLETION);

template <class privsType>
inline Error CreateProfileEx(Identity* identity, const IPSocket::Address* profileServers, unsigned int numSrvrs,
                             const char* emailAddress, long timeout, bool async, void (*f)(Error, privsType), privsType privs)
	{ return CreateProfile(identity, profileServers, numSrvrs, emailAddress, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true)); }

}; // namespace WONAPI

/*

class Profile
{
protected:
	class ProfileIndex
	{
	public:
		WONCommon::RawBuffer buf;
		bool isValid;

		ProfileIndex()
			:	isValid(false)
		{ }
	};
	
	std::vector<ProfileIndex> elements;

public:
	void Set(unsigned short index, const void* data, unsigned short size)
	{
		while (elements.size() <= index)
			elements.push_back(ProfileIndex());
		ProfileIndex* profileIndex = &(elements[index]);
		profileIndex->buf.assign((unsigned char*)data, size);
		profileIndex->isValid = true;
	}
	const void* Get(unsigned short index, unsigned short* size = 0) const
	{
		unsigned short gotSize = 0;
		void* result = 0;
		if (elements.size() > index)
		{
			const ProfileIndex* profileIndex = &(elements[index]);
			if (profileIndex->isValid)
			{
				gotSize = profileIndex->buf.size();
				result = (void*)(profileIndex->buf.data());
			}
		}
		if (size)
			*size = gotSize;
		return result;
	}
	void Remove(unsigned short index)
	{
		if (elements.size() > index)
			elements[index].isValid = false;
	}


	void SetBool(unsigned short index, bool b)		{ char c = b ? 1 : 0; Set(index, &c, 1); }
	void Set(unsigned short index, bool b)			{ char c = b ? 1 : 0; Set(index, &c, 1); }
	void SetShort(unsigned short index, short s)	{ Set(index, &s, sizeof(short)); }	// byte order dependent
	void Set(unsigned short index, short s)			{ Set(index, &s, sizeof(short)); }	// byte order dependent
	void SetLong(unsigned short index, long l)		{ Set(index, &l, sizeof(long)); }	// byte order dependent
	void Set(unsigned short index, long l)			{ Set(index, &l, sizeof(long)); }	// byte order dependent
	void SetString(unsigned short index, const WONCommon::WONString& str)	{ Set(index, str.GetWideString().data(), str.size() * 2); }
	void Set(unsigned short index, const WONCommon::WONString& str)	{ Set(index, str.GetWideString().data(), str.size() * 2); }


	bool GetBool(unsigned short index) const		{ return *(char*)Get(index) ? true : false; }
	long GetLong(unsigned short index) const 		{ return *(long*)Get(index); }
	short GetShort(unsigned short index) const		{ return *(short*)Get(index); }
	void GetString(unsigned short index, WONCommon::WONString& str) const
	{
		unsigned short size;
		const void* data = Get(index, &size);
		str.assign((const wchar*)data, size / 2);
	}
	void GetString(unsigned short index, wstring& str) const
	{
		unsigned short size;
		const void* data = Get(index, &size);
		str.assign((const wchar*)data, size / 2);
	}
	void GetString(unsigned short index, string& str) const
	{
		WONCommon::WONString tmpStr;
		GetString(index, tmpStr);
		str.assign(tmpStr.GetAsciiString());
	}
	void GetString(unsigned short index, char* str, unsigned short* strBufLength) const
	{
		if (strBufLength)
		{
			unsigned short maxSize = *strBufLength;
			*strBufLength = 0;
			if (str && maxSize > 1)
			{
				WONCommon::WONString tmpStr;
				GetString(index, tmpStr);

				str[maxSize - 1] = 0;
				unsigned short copySize = tmpStr.size() > maxSize-2 ? maxSize-2 : tmpStr.size();
				strncpy(str, tmpStr.GetAsciiCString(), copySize + 1);
				*strBufLength = copySize;
			}
		}
	}
	void GetString(unsigned short index, wchar* str, unsigned short* strBufLength) const
	{
		if (strBufLength)
		{
			unsigned short maxSize = *strBufLength;
			*strBufLength = 0;
			if (str && maxSize > 1)
			{
				WONCommon::WONString tmpStr;
				GetString(index, tmpStr);

				str[maxSize - 1] = 0;
				unsigned short copySize = tmpStr.size() > maxSize-2 ? maxSize-2 : tmpStr.size();
				wcsncpy(str, tmpStr.GetWideCString(), copySize + 1);
				*strBufLength = copySize;
			}
		}
	}

	
	void SetGender(const WONCommon::WONString& gender)			{ SetString(WONMsg::PROFILE_GENDER, gender); }
	void SetBirthdate(time_t birthdate)							{ SetLong(WONMsg::PROFILE_BIRTHDATE, birthdate); }
	void SetFirstName(const WONCommon::WONString& firstName)	{ SetString(WONMsg::PROFILE_FIRSTNAME, firstName); }
	void SetLastName(const WONCommon::WONString& lastName)		{ SetString(WONMsg::PROFILE_LASTNAME, lastName); }
	void SetMiddleName(const WONCommon::WONString& middleName)	{ SetString(WONMsg::PROFILE_MIDDLENAME, middleName); }
	void SetCompanyName(const WONCommon::WONString& companyName){ SetString(WONMsg::PROFILE_COMPANYNAME, companyName); }
	void SetAddress1(const WONCommon::WONString& address1)		{ SetString(WONMsg::PROFILE_ADDRESS1, address1); }
	void SetAddress2(const WONCommon::WONString& address2)		{ SetString(WONMsg::PROFILE_ADDRESS2, address2); }
	void SetCity(const WONCommon::WONString& city)				{ SetString(WONMsg::PROFILE_CITY, city); }
	void SetPostalCode(const WONCommon::WONString& postalCode)	{ SetString(WONMsg::PROFILE_POSTALCODE, postalCode); }
	void SetState(const WONCommon::WONString& state)			{ SetString(WONMsg::PROFILE_STATE, state); }
	void SetPhone1(const WONCommon::WONString& phone1)			{ SetString(WONMsg::PROFILE_PHONE1, phone1); }
	void SetPhone1Ext(const WONCommon::WONString& phone1Ext)	{ SetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext); }
	void SetPhone2(const WONCommon::WONString& phone2)			{ SetString(WONMsg::PROFILE_PHONE2, phone2); }
	void SetPhone2Ext(const WONCommon::WONString& phone2Ext)	{ SetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext); }
	void SetEmail1(const WONCommon::WONString& email1)			{ SetString(WONMsg::PROFILE_EMAIL1, email1); }
	void SetEmail2(const WONCommon::WONString& email2)			{ SetString(WONMsg::PROFILE_EMAIL2, email2); }


	void GetGender(WONCommon::WONString& gender) const			{ GetString(WONMsg::PROFILE_GENDER, gender); }
	void GetGender(wstring& gender) const						{ GetString(WONMsg::PROFILE_GENDER, gender); }
	void GetGender(string& gender) const						{ GetString(WONMsg::PROFILE_GENDER, gender); }
	void GetGender(char* gender, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_GENDER, gender, bufLen); }
	void GetGender(wchar* gender, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_GENDER, gender, bufLen); }
	time_t GetBirthdate() const									{ return GetLong(WONMsg::PROFILE_GENDER); }
	void GetFirstName(WONCommon::WONString& firstName) const	{ GetString(WONMsg::PROFILE_FIRSTNAME, firstName); }
	void GetFirstName(wstring& firstName) const					{ GetString(WONMsg::PROFILE_FIRSTNAME, firstName); }
	void GetFirstName(string& firstName) const					{ GetString(WONMsg::PROFILE_FIRSTNAME, firstName); }
	void GetFirstName(char* firstName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_FIRSTNAME, firstName, bufLen); }
	void GetFirstName(wchar* firstName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_FIRSTNAME, firstName, bufLen); }
	void GetLastName(WONCommon::WONString& lastName) const		{ GetString(WONMsg::PROFILE_LASTNAME, lastName); }
	void GetLastName(wstring& lastName) const					{ GetString(WONMsg::PROFILE_LASTNAME, lastName); }
	void GetLastName(string& lastName) const					{ GetString(WONMsg::PROFILE_LASTNAME, lastName); }
	void GetLastName(char* lastName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_LASTNAME, lastName, bufLen); }
	void GetLastName(wchar* lastName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_LASTNAME, lastName, bufLen); }
	void GetMiddleName(WONCommon::WONString& middleName) const	{ GetString(WONMsg::PROFILE_MIDDLENAME, middleName); }
	void GetMiddleName(wstring& middleName) const				{ GetString(WONMsg::PROFILE_MIDDLENAME, middleName); }
	void GetMiddleName(string& middleName) const				{ GetString(WONMsg::PROFILE_MIDDLENAME, middleName); }
	void GetMiddleName(char* middleName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_MIDDLENAME, middleName, bufLen); }
	void GetMiddleName(wchar* middleName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_MIDDLENAME, middleName, bufLen); }
	void GetCompanyName(WONCommon::WONString& companyName) const{ GetString(WONMsg::PROFILE_COMPANYNAME, companyName); }
	void GetCompanyName(wstring& companyName) const				{ GetString(WONMsg::PROFILE_COMPANYNAME, companyName); }
	void GetCompanyName(string& companyName) const				{ GetString(WONMsg::PROFILE_COMPANYNAME, companyName); }
	void GetCompanyName(char* companyName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_COMPANYNAME, companyName, bufLen); }
	void GetCompanyName(wchar* companyName, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_COMPANYNAME, companyName, bufLen); }
	void GetAddress1(WONCommon::WONString& address1) const		{ GetString(WONMsg::PROFILE_ADDRESS1, address1); }
	void GetAddress1(wstring& address1) const					{ GetString(WONMsg::PROFILE_ADDRESS1, address1); }
	void GetAddress1(string& address1) const					{ GetString(WONMsg::PROFILE_ADDRESS1, address1); }
	void GetAddress1(char* address1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_ADDRESS1, address1, bufLen); }
	void GetAddress1(wchar* address1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_ADDRESS1, address1, bufLen); }
	void GetAddress2(WONCommon::WONString& address2) const		{ GetString(WONMsg::PROFILE_ADDRESS2, address2); }
	void GetAddress2(wstring& address2) const					{ GetString(WONMsg::PROFILE_ADDRESS2, address2); }
	void GetAddress2(string& address2) const					{ GetString(WONMsg::PROFILE_ADDRESS2, address2); }
	void GetAddress2(char* address2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_ADDRESS2, address2, bufLen); }
	void GetAddress2(wchar* address2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_ADDRESS2, address2, bufLen); }
	void GetCity(WONCommon::WONString& city) const				{ GetString(WONMsg::PROFILE_CITY, city); }
	void GetCity(wstring& city) const							{ GetString(WONMsg::PROFILE_CITY, city); }
	void GetCity(string& city) const							{ GetString(WONMsg::PROFILE_CITY, city); }
	void GetCity(char* city, unsigned short* bufLen) const		{ GetString(WONMsg::PROFILE_CITY, city, bufLen); }
	void GetCity(wchar* city, unsigned short* bufLen) const		{ GetString(WONMsg::PROFILE_CITY, city, bufLen); }
	void GetPostalCode(WONCommon::WONString& postalCode) const	{ GetString(WONMsg::PROFILE_POSTALCODE, postalCode); }
	void GetPostalCode(wstring& postalCode) const				{ GetString(WONMsg::PROFILE_POSTALCODE, postalCode); }
	void GetPostalCode(string& postalCode) const				{ GetString(WONMsg::PROFILE_POSTALCODE, postalCode); }
	void GetPostalCode(char* postalCode, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_POSTALCODE, postalCode, bufLen); }
	void GetPostalCode(wchar* postalCode, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_POSTALCODE, postalCode, bufLen); }
	void GetState(WONCommon::WONString& state) const			{ GetString(WONMsg::PROFILE_STATE, state); }
	void GetState(wstring& state) const							{ GetString(WONMsg::PROFILE_STATE, state); }
	void GetState(string& state) const							{ GetString(WONMsg::PROFILE_STATE, state); }
	void GetState(char* state, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_STATE, state, bufLen); }
	void GetState(wchar* state, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_STATE, state, bufLen); }
	void GetPhone1(WONCommon::WONString& phone1) const			{ GetString(WONMsg::PROFILE_PHONE1, phone1); }
	void GetPhone1(wstring& phone1) const						{ GetString(WONMsg::PROFILE_PHONE1, phone1); }
	void GetPhone1(string& phone1) const						{ GetString(WONMsg::PROFILE_PHONE1, phone1); }
	void GetPhone1(char* phone1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE1, phone1, bufLen); }
	void GetPhone1(wchar* phone1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE1, phone1, bufLen); }
	void GetPhone1Ext(WONCommon::WONString& phone1Ext) const	{ GetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext); }
	void GetPhone1Ext(wstring& phone1Ext) const					{ GetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext); }
	void GetPhone1Ext(string& phone1Ext) const					{ GetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext); }
	void GetPhone1Ext(char* phone1Ext, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext, bufLen); }
	void GetPhone1Ext(wchar* phone1Ext, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE1EXT, phone1Ext, bufLen); }
	void GetPhone2(WONCommon::WONString& phone2) const			{ GetString(WONMsg::PROFILE_PHONE2, phone2); }
	void GetPhone2(wstring& phone2) const						{ GetString(WONMsg::PROFILE_PHONE2, phone2); }
	void GetPhone2(string& phone2) const						{ GetString(WONMsg::PROFILE_PHONE2, phone2); }
	void GetPhone2(char* phone2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE2, phone2, bufLen); }
	void GetPhone2(wchar* phone2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE2, phone2, bufLen); }
	void GetPhone2Ext(WONCommon::WONString& phone2Ext) const	{ GetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext); }
	void GetPhone2Ext(wstring& phone2Ext) const					{ GetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext); }
	void GetPhone2Ext(string& phone2Ext) const					{ GetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext); }
	void GetPhone2Ext(char* phone2Ext, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext, bufLen); }
	void GetPhone2Ext(wchar* phone2Ext, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_PHONE2EXT, phone2Ext, bufLen); }
	void GetEmail1(WONCommon::WONString& email1) const			{ GetString(WONMsg::PROFILE_EMAIL1, email1); }
	void GetEmail1(wstring& email1) const						{ GetString(WONMsg::PROFILE_EMAIL1, email1); }
	void GetEmail1(string& email1) const						{ GetString(WONMsg::PROFILE_EMAIL1, email1); }
	void GetEmail1(char* email1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_EMAIL1, email1, bufLen); }
	void GetEmail1(wchar* email1, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_EMAIL1, email1, bufLen); }
	void GetEmail2(WONCommon::WONString& email2) const			{ GetString(WONMsg::PROFILE_EMAIL2, email2); }
	void GetEmail2(wstring& email2) const						{ GetString(WONMsg::PROFILE_EMAIL2, email2); }
	void GetEmail2(string& email2) const						{ GetString(WONMsg::PROFILE_EMAIL2, email2); }
	void GetEmail2(char* email2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_EMAIL2, email2, bufLen); }
	void GetEmail2(wchar* email2, unsigned short* bufLen) const	{ GetString(WONMsg::PROFILE_EMAIL2, email2, bufLen); }

	void RemoveGender()											{ Remove(WONMsg::PROFILE_GENDER);      }
	void RemoveBirthdate()										{ Remove(WONMsg::PROFILE_BIRTHDATE);   }
	void RemoveFirstName()										{ Remove(WONMsg::PROFILE_FIRSTNAME);   }
	void RemoveLastName()										{ Remove(WONMsg::PROFILE_LASTNAME);    }
	void RemoveMiddleName()										{ Remove(WONMsg::PROFILE_MIDDLENAME);  }
	void RemoveCompanyName()									{ Remove(WONMsg::PROFILE_COMPANYNAME); }
	void RemoveAddress1()										{ Remove(WONMsg::PROFILE_ADDRESS1);    }
	void RemoveAddress2()										{ Remove(WONMsg::PROFILE_ADDRESS2);    }
	void RemoveCity()											{ Remove(WONMsg::PROFILE_CITY);        }
	void RemovePostalCode()										{ Remove(WONMsg::PROFILE_POSTALCODE);  }
	void RemoveState()											{ Remove(WONMsg::PROFILE_STATE);       }
	void RemovePhone1()											{ Remove(WONMsg::PROFILE_PHONE1);      }
	void RemovePhone1Ext()										{ Remove(WONMsg::PROFILE_PHONE1EXT);   }
	void RemovePhone2()											{ Remove(WONMsg::PROFILE_PHONE2);      }
	void RemovePhone2Ext()										{ Remove(WONMsg::PROFILE_PHONE2EXT);   }
	void RemoveEmail1()											{ Remove(WONMsg::PROFILE_EMAIL1);      }
	void RemoveEmail2()											{ Remove(WONMsg::PROFILE_EMAIL2);      }
};


// CreateProfile - fail-over list of addresses
Error CreateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
					const Profile& profile, long sourceSeq, bool wantsPromos, 
					long timeout, bool async, const CompletionContainer<Error>& completion);


inline Error CreateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
						   const Profile& profile, long sourceSeq, bool wantsPromos = true, 
						   long timeout = -1, bool async = false)
{ return CreateProfile(ident, profileSrvrs, numSrvrs, profile, sourceSeq, wantsPromos, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error CreateProfileEx(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
							 const Profile& profile, long sourceSeq, bool wantsPromos, long timeout,
							 bool async, void (*f)(Error, privsType), privsType privs)
{ return CreateProfile(ident, profileSrvrs, numSrvrs, profile, sourceSeq, wantsPromo, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}


// CreateProfile - 1 address
inline Error CreateProfile(Identity& ident, const IPSocket::Address& profileSrvr,
						   const Profile& profile, long sourceSeq, bool wantsPromos, 
						   long timeout, bool async, const CompletionContainer<Error>& completion)
{ return CreateProfile(ident, &profileSrvr, 1, profile, sourceSeq, wantsPromos, timeout, async, completion); }


inline Error CreateProfile(Identity& ident, const IPSocket::Address& profileSrvr,
						   const Profile& profile, long sourceSeq, bool wantsPromos = true, 
						   long timeout = -1, bool async = false)
{ return CreateProfile(ident, &profileSrvr, 1, profile, sourceSeq, wantsPromos, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error CreateProfileEx(Identity& ident, const IPSocket::Address& profileSrvr,
							 const Profile& profile, long sourceSeq, bool wantsPromos, long timeout,
							 bool async, void (*f)(Error, privsType), privsType privs)
{ return CreateProfile(ident, &profileSrvr, 1, profile, sourceSeq, wantsPromos, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}



// GetProfile - fail-over list of addresses
Error GetProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
				 Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion);


inline Error GetProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
						Profile& profile, long timeout = -1, bool async = false)
{ return GetProfile(ident, profileSrvrs, numSrvrs, profile, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error GetProfileEx(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
						  const Profile& profile, long timeout, bool async,
						  void (*f)(Error, privsType), privsType privs)
{ return GetProfile(ident, profileSrvrs, numSrvrs, profile, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}


// GetProfile - 1 address
inline Error GetProfile(Identity& ident, const IPSocket::Address& profileSrvr,
						Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion)
{ return GetProfile(ident, &profileSrvr, 1, profile, timeout, async, completion); }


inline Error GetProfile(Identity& ident, const IPSocket::Address& profileSrvr,
						Profile& profile, long timeout = -1, bool async = false)
{ return GetProfile(ident, &profileSrvr, 1, profile, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error GetProfileEx(Identity& ident, const IPSocket::Address& profileSrvr,
						  const Profile& profile, long timeout, bool async,
						  void (*f)(Error, privsType), privsType privs)
{ return GetProfile(ident, &profileSrvr, 1, profile, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}


// UpdateProfile - fail-over list of addresses
Error UpdateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
					const Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion);


inline Error UpdateProfile(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
						   const Profile& profile, long timeout = -1, bool async = false)
{ return UpdateProfile(ident, profileSrvrs, numSrvrs, profile, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error UpdateProfileEx(Identity& ident, const IPSocket::Address* profileSrvrs, unsigned int numSrvrs,
							 const Profile& profile, long timeout, bool async,
							 void (*f)(Error, privsType), privsType privs)
{ return UpdateProfile(ident, &profileSrvr, 1, profile, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}


// UpdateProfile - 1 address
Error UpdateProfile(Identity& ident, const IPSocket::Address& profileSrvr,
					const Profile& profile, long timeout, bool async, const CompletionContainer<Error>& completion)
{ return UpdateProfile(ident, &profileSrvr, 1, profile, timeout, async, completion); }


inline Error UpdateProfile(Identity& ident, const IPSocket::Address& profileSrvr,
						   const Profile& profile, long timeout = -1, bool async = false)
{ return UpdateProfile(ident, &profileSrvr, 1, profile, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error UpdateProfileEx(Identity& ident, const IPSocket::Address& profileSrvr,
							 const Profile& profile, long timeout, bool async,
							 void (*f)(Error, privsType), privsType privs)
{ return UpdateProfile(ident, &profileSrvr, 1, profile, timeout, async, new CompletionWithContents<Error, privsType>(f, privs, true));}
*/

#endif
