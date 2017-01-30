// RegKey

// Encapsulates access to a Registry Key.  Key must already exist.  Allows
// string and long values to created, read, updated, or deleted.


#include "common/won.h"
#include "common/RegKey.h"

// Private namespace for using, constants, etc
namespace {

	using WONCommon::RegKey;

};


// ** Constructors / Destructor **

// **WARNING** This object is instanciated at LOAD by AppOptions class.
//    Absolutely no debugging in default constructor or destructor!

// Default
RegKey::RegKey(const string& theKey, HKEY theRoot /* = HKEY_LOCAL_MACHINE */, 
			   bool createKey /* = false */, bool nonVolatile /* = true */) :
	mKey(), mOpen(false), mBufP(NULL), mBufLen(0), mLeafName(), mKeyIndex(0), mValueIndex(0) {

	if(theKey.size() > 0){

		if(createKey){

			char aClass;
			DWORD wasCreated;
			DWORD anOptionValue((nonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE);

			mOpen = (RegCreateKeyEx(theRoot, theKey.c_str(), NULL, &aClass, anOptionValue, KEY_ALL_ACCESS, NULL, &mKey, &wasCreated) == ERROR_SUCCESS);

		}
		else{
			mOpen = (RegOpenKeyEx(theRoot, theKey.c_str(), NULL, KEY_ALL_ACCESS, &mKey) == ERROR_SUCCESS);
		}

		if(mOpen){
			mLeafName = theKey;
		}

	}

}


// Destructor
RegKey::~RegKey(void){

	if(mOpen){
		RegCloseKey(mKey);
	}
	delete [] mBufP;

}


// ** Private Methods **

void RegKey::CloseKey(void){

	WTRACE("RegKey::CloseKey");
	if(mOpen){

		WDBG_LL("RegKey::CloseKey Closing current key");
		RegCloseKey(mKey);
		mOpen = false;

	}
}


DWORD RegKey::GetToBuf(const string& theName, unsigned long &theLengthR) const{

	DWORD aType;
	DWORD aLen;

	WTRACE("RegKey::GetToBuf");
	WDBG_LL("RegKey::GetToBuf Fetch into buf for " << theName);

	// First get length of data
	if(RegQueryValueEx(mKey, theName.c_str(), NULL, &aType,
	                   NULL, &aLen) != ERROR_SUCCESS){

		WDBG_LM("RegKey::GetToBuf Fetch FAIL for size of " << theName);
		return REG_NONE;

	}

	theLengthR = aLen;

	// Allocate the buffer
	if(mBufLen < aLen){

		delete [] mBufP;
		mBufLen = aLen;
		mBufP   = new unsigned char [aLen];

	}

	// Fetch the value
	if(RegQueryValueEx(mKey, theName.c_str(), NULL, &aType,
	                   static_cast<unsigned char *>(mBufP), &aLen) == ERROR_SUCCESS){

		WDBG_LM("RegKey::GetToBuf Fetch SUCCESS for " << theName);
		return aType;

	}
	else{

		WDBG_LM("RegKey::GetToBuf Fetch FAIL for data of " << theName);
		return REG_NONE;

	}
}


// ** Public Methods **

bool RegKey::OpenNewKey(const string& theKey, HKEY theRoot /* = HKEY_LOCAL_MACHINE */, 
		    	        bool createKey /* = false */, bool nonVolatile /* = true */){

	WTRACE("RegKey::OpenNewKey");
	CloseKey();

	if(theKey.size() > 0){

		WDBG_LL("RegKey::OpenNewKey Opening key: " << theKey);
		if(createKey){

			char aClass;
			DWORD wasCreated;
			DWORD anOptionValue((nonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE);

			mOpen = (RegCreateKeyEx(theRoot, theKey.c_str(), NULL, &aClass, anOptionValue, KEY_ALL_ACCESS, NULL, &mKey, &wasCreated) == ERROR_SUCCESS);

		}
		else{
			mOpen = (RegOpenKeyEx(theRoot, theKey.c_str(), NULL, KEY_ALL_ACCESS, &mKey) == ERROR_SUCCESS);
		}

		if(mOpen){
			mLeafName = theKey;
		}

		WDBG_LM("RegKey::OpenNewKey Open key: " << theKey << "  State=" << mOpen);

	}
	return mOpen;
}


RegKey::GetResult RegKey::GetValue(const string& theName, string& theValR) const{

	GetResult ret = NotFound;

	WTRACE("RegKey::GetValue(string)");
	theValR.erase();
	if(!IsOpen()){
		return NotFound;
	}

	unsigned long aLength;
	switch (GetToBuf(theName, aLength)){
	case REG_SZ:
		theValR = reinterpret_cast<char *>(mBufP);
		WDBG_LM("RegKey::GetValue(string) " << theName << " = " << theValR);
		ret = Ok;
		break;
	case REG_NONE:
		WDBG_LM("RegKey::GetValue(string) Value not found for " << theName);
		ret = NotFound;
		break;
	default:
		WDBG_LM("RegKey::GetValue(string) Wrong type for " << theName);
		ret = BadType;
		break;
	}

	return ret;
}


RegKey::GetResult RegKey::GetValue(const string& theName, unsigned long& theValR) const{

	GetResult ret = NotFound;

	WTRACE("RegKey::GetValue(unsigned long)");
	theValR = 0;
	if(!IsOpen()){
		return NotFound;
	}

	unsigned long aLength;
	switch (GetToBuf(theName, aLength)){
	case REG_DWORD:
		memcpy(&theValR, mBufP, sizeof(unsigned long));
		WDBG_LM("RegKey::GetValue(unsigned long) " << theName << " = " << theValR);
		ret = Ok;
		break;
	case REG_NONE:
		WDBG_LM("RegKey::GetValue(unsigned long) Value not found for " << theName);
		ret = NotFound;
		break;
	default:
		WDBG_LM("RegKey::GetValue(unsigned long) Wrong type for " << theName);
		ret = BadType;
		break;
	}

	return ret;
}


RegKey::GetResult RegKey::GetValue(const std::string& theName, unsigned char* &theValPR, unsigned long& theLengthR) const{

	GetResult ret = NotFound;

	WTRACE("RegKey::GetValue(binary)");

	if(!IsOpen()){
		return NotFound;
	}

	switch (GetToBuf(theName, theLengthR)){
	case REG_BINARY:
		if(theLengthR > 0){
			theValPR = new unsigned char[theLengthR];
			memcpy(theValPR, mBufP, theLengthR);
			WDBG_LM("RegKey::GetValue(binary) " << theName);
			ret = Ok;
		}
		else{
			WDBG_LM("RegKey::GetValue(binary) No Size for " << theName);
			ret = NoSize;
		}
		break;
	case REG_NONE:
		WDBG_LM("RegKey::GetValue(binary) Value not found for " << theName);
		ret = NotFound;
		break;
	default:
		WDBG_LM("RegKey::GetValue(binary) Wrong type for " << theName);
		ret = BadType;
		break;
	}

	return ret;
}


bool RegKey::SetValue(const string& theName, const string& theValue){

	WTRACE("RegKey::SetValue(string)");
	if(!IsOpen()){
		return false;
	}

	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_SZ,
	                  reinterpret_cast<CONST BYTE*>(theValue.c_str()),
	                  (theValue.size()+1)) == ERROR_SUCCESS){

		WDBG_LM("RegKey::SetValue(string) " << theName << " = " << theValue);
		return true;

	}
	else{

		WDBG_LM("RegKey::SetValue(string) Fail set value for " << theName);
		return false;

	}
}


bool RegKey::SetValue(const string& theName, unsigned long theValue){

	WTRACE("RegKey::SetValue(unsigned long)");
	if(!IsOpen()){
		return false;
	}

	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_DWORD,
	                  reinterpret_cast<CONST BYTE*>(&theValue),
	                  sizeof(theValue)) == ERROR_SUCCESS){

		WDBG_LM("RegKey::SetValue(unsigned long) " << theName << " = " << theValue);

		return true;

	}
	else{

		WDBG_LM("RegKey::SetValue(unsigned long) Fail set value for " << theName);
		return false;

	}
}


bool RegKey::SetValue(const std::string& theName, const unsigned char* theValueP, unsigned long theLength){

	WTRACE("RegKey::SetValue(binary)");
	if(!IsOpen()){
		return false;
	}

	if(RegSetValueEx(mKey, theName.c_str(), NULL, REG_BINARY,
	                 theValueP,
	                 theLength) == ERROR_SUCCESS){

		WDBG_LM("RegKey::SetValue(binary) " << theName);

		return true;

	}
	else{

		WDBG_LM("RegKey::SetValue(binary) Fail set value for " << theName);
		return false;

	}
}


bool RegKey::DeleteValue(const string& theName){

	WTRACE("RegKey::DeleteValue");
	if(!IsOpen()){
		return false;
	}

	return (RegDeleteValue(mKey, theName.c_str()) == ERROR_SUCCESS);

}


bool RegKey::OpenSubKey(const std::string& theSubKey, bool createSubKey /* = false */, 
			        	bool nonVolatile /* = true */){


	WTRACE("RegKey::OpenSubKey");
	bool subkeyOpen(false);
	if(theSubKey.size() > 0 && mOpen){

		WDBG_LL("RegKey::OpenSubKey Opening SubKey: " << theSubKey);
		HKEY aSubKey;
		
		if(createSubKey){

			char aClass;
			DWORD wasCreated;
			DWORD anOptionValue((nonVolatile) ? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE);

			subkeyOpen = (RegCreateKeyEx(mKey, theSubKey.c_str(), NULL, &aClass, anOptionValue, KEY_ALL_ACCESS, NULL, &aSubKey, &wasCreated) == ERROR_SUCCESS);

		}
		else{
			subkeyOpen = (RegOpenKeyEx(mKey, theSubKey.c_str(), NULL, KEY_ALL_ACCESS, &aSubKey) == ERROR_SUCCESS);
		}

		if(subkeyOpen){
			CloseKey();
			mLeafName = theSubKey;
			mKey = aSubKey;
			mOpen = subkeyOpen;
		}

		WDBG_LM("RegKey::OpenSubKey Open key: " << theSubKey << "  State=" << mOpen);

	}

	return subkeyOpen;

}


bool RegKey::GetSubKey(const std::string& theSubKeyName, RegKey& theSubKey, 
					   bool createSubKey /* = false */, 
					   bool nonVolatile /* = true */) const {


	WTRACE("RegKey::GetSubKey");
	if(theSubKeyName.size() > 0 && mOpen){

		WDBG_LL("RegKey::GetSubKey Getting SubKey: " << theSubKeyName);
		return theSubKey.OpenNewKey(theSubKeyName, mKey, createSubKey, nonVolatile);

	}

	return false;

}


bool RegKey::DeleteSubKey(const std::string& theSubKey) const {

	if(theSubKey.size() > 0 && mOpen){

		return (RegDeleteKey(mKey, theSubKey.c_str()) == ERROR_SUCCESS);

	}

	return false;

}


bool RegKey::DeleteSubKey(RegKey& theSubKey) const {

	if(mOpen){

		if(theSubKey.mOpen){
			theSubKey.CloseKey();
		}

		return (RegDeleteKey(mKey, theSubKey.GetLeafName().c_str()) == ERROR_SUCCESS);

	}

	return false;

}


RegKey::GetResult RegKey::GetFirstSubKey(RegKey& theSubKey) const {

	mKeyIndex = 0;
	return ((GetNextSubKey(theSubKey) == NoMore) ? NotFound : Ok);

}


RegKey::GetResult RegKey::GetNextSubKey(RegKey& theSubKey) const {

	FILETIME aFileTime;
	char aNameBuffer[256];
	DWORD aNameLength(sizeof(aNameBuffer));
	GetResult ret(NoMore);

	if(RegEnumKeyEx(mKey, mKeyIndex, aNameBuffer, &aNameLength, 0, NULL, NULL, &aFileTime) != ERROR_NO_MORE_ITEMS){

		mKeyIndex++;
		if(theSubKey.OpenNewKey(aNameBuffer, mKey)){
			ret = Ok;
		}

	}

	return ret;

}


RegKey::GetResult RegKey::GetFirstValueName(std::string& theName, DataType& theType) const {

	mValueIndex = 0;
	return ((GetNextValueName(theName, theType) == NoMore) ? NotFound : Ok);

}


RegKey::GetResult RegKey::GetNextValueName(std::string& theName, DataType& theType) const {

	char aNameBuffer[256];
	DWORD aNameLength(sizeof(aNameBuffer)), aDataType;
	GetResult ret(NoMore);

	if(RegEnumValue(mKey, mValueIndex, aNameBuffer, &aNameLength, 0, &aDataType, NULL, NULL) != ERROR_NO_MORE_ITEMS){

		switch(aDataType){
		case REG_DWORD:
			theType=Long;
			break;
		case REG_SZ:
			theType=String;
			break;
		case REG_BINARY:
			theType=Binary;
			break;
		default:
			theType=Other;
			break;
		}
		mValueIndex++;
		theName = aNameBuffer;
		ret = Ok;

	}

	return ret;

}

