
#pragma warning(disable : 4786)

#include "common/ParameterDataTypes.h"
#include "RawList.h"

////////////////////////////////////////////////////////////////////////////////
// RawList
//
// This class is to holds a list of or a list of lists of elements.
// The lists can be built from a RawBuffer representation, a string representation,
// or a list representation.
// The lists can then be displayed in either of these forms once created.
//
// Binary form of the list
// 
// Type (1 byte) NumberOfElements (1 byte) First Element Second Element... etc
// 
// Each element is of type
// 
// R - Binary		- Length (2 bytes) Data (Length number of bytes)
// B - Bool			- Data (1 byte)
// I - Long			- Data (4 bytes)
// H - LongLong		- Data (8 bytes) ( Not supported currently )
// S - String		- Length (2 bytes) Data (Length number of bytes)
// W - Wide String	- Length (2 bytes) Data (Length number of bytes, characters = length/2)
// L - List			- Type (1 byte) NumberOfElements (1 byte) First Element Second Element... etc
// 
////////////////////////////////////////////////////////////////////////////////

// TO DO ** Put diagnostic messages in this class

using namespace WONCommon;

RawList::RawList(const RawBuffer& theRawBufferR)
{
	// Transtlate from binary format to vector structure
	ParamTypeEnum aType = (ParamTypeEnum)*(unsigned char*)theRawBufferR.data();

	int aResult = RawError;
	switch (aType)
	{
		case ParamType_Binary:
			mListType = RawBinary;
			aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mBinaryList, mListType);
		break;
		case ParamType_Bool:
			mListType = RawBool;
			aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mBoolList, mListType);
		break;
		case ParamType_Long:
			mListType = RawLong;
			aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mLongList, mListType);
		break;
		case ParamType_String:
			mListType = RawString;
			aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mStringList, mListType);
		break;
		case ParamType_WString:
			mListType = RawUnicode;
			aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mUnicodeList, mListType);
		break;
		case ParamType_List:
		{
			// Need to go to first list description to get full type
			ParamTypeEnum aListType = (ParamTypeEnum)theRawBufferR[2];
			if (aListType == ParamType_Long)
			{
				mListType = RawListLong;
				aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mLongListList, mListType);
			}
			else if (aListType == ParamType_String)
			{
				mListType = RawListString;
				aResult = ReadList(theRawBufferR.substr(1,theRawBufferR.size()-1), (void*)&mStringListList, mListType);
			}
			else
			{
				// No other lists supported currently
				mListType = RawUnknown;
			}
		}
		break;
		case ParamType_LongLong: // Not supported currently
		default:
			mListType = RawUnknown;
		break;
	}
	if (aResult == RawError)
	{
		mListType = RawUnknown;
	}
}

RawList::RawList(const wstring& theWStringR, ListTypeEnum theListType)
{
	mListType = theListType;
	// Transtlate from string to vector structure
	int aResult = RawError;
	switch (mListType)
	{
		case RawBinary:
			aResult = ReadList(theWStringR, (void*)&mBinaryList, mListType);
		break;
		case RawBool:
			aResult = ReadList(theWStringR, (void*)&mBoolList, mListType);
		break;
		case RawLong:
			aResult = ReadList(theWStringR, (void*)&mLongList, mListType);
		break;
		case RawString:
			aResult = ReadList(theWStringR, (void*)&mStringList, mListType);
		break;
		case RawUnicode:
			aResult = ReadList(theWStringR, (void*)&mUnicodeList, mListType);
		break;
		case RawListLong:
			aResult = ReadListList(theWStringR, (void*)&mLongListList, mListType);
		break;
		case RawListString:
			aResult = ReadListList(theWStringR, (void*)&mStringListList, mListType);
		break;
		default:
		break;
	}

	if (aResult == RawError)
	{
		mListType = RawUnknown;
	}
}

RawList::RawList(const UnicodeList& theUnicodeListR)
{
	mListType = RawUnicode;
	mUnicodeList = theUnicodeListR;
}

RawList::RawList(const StringList& theStringListR)
{
	mListType = RawString;
	mStringList = theStringListR;
}

RawList::RawList(const LongList& theLongListR)
{
	mListType = RawLong;
	mLongList = theLongListR;
}

RawList::RawList(const BoolList& theBoolListR)
{
	mListType = RawBool;
	mBoolList = theBoolListR;
}

RawList::RawList(const BinaryList& theBinaryListR)
{
	mListType = RawBinary;
	mBinaryList = theBinaryListR;
}

RawList::RawList(const LongListList& theLongListListR)
{
	mListType = RawListLong;
	mLongListList = theLongListListR;
}

RawList::RawList(const StringListList& theStringListListR)
{
	mListType = RawListString;
	mStringListList = theStringListListR;
}

RawList::~RawList()
{

}

// Return size of the highest level list
long 
RawList::GetListSize()
{
	long aReturn = 0; 
	switch (mListType)
	{
		case RawUnicode:
			aReturn = mUnicodeList.size();
		break;
		case RawString: 
			aReturn = mStringList.size();
		break;
		case RawLong:
			aReturn = mLongList.size();
		break;
		case RawBool:
			aReturn = mBoolList.size();
		break;
		case RawBinary:
			aReturn = mBinaryList.size();
		break;
		case RawListLong:
			aReturn = mLongListList.size();
		break;
		case RawListString:
			aReturn = mStringListList.size();
		break;
		default: // RawUnknown
		break;
	}
	return aReturn;
}

RawBuffer 
RawList::GetRawList()
{
	RawBuffer aReturnBuff;
	bool aSucceed = true;

	switch (mListType)
	{
		case RawUnicode:
			aSucceed = AppendUnicodeList(aReturnBuff, mUnicodeList);
		break;
		case RawString: 
			aSucceed = AppendStringList(aReturnBuff, mStringList);
		break;
		case RawLong:
			aSucceed = AppendLongList(aReturnBuff, mLongList);
		break;
		case RawBool:
			aSucceed = AppendBoolList(aReturnBuff, mBoolList);
		break;
		case RawBinary:
			aSucceed = AppendBinaryList(aReturnBuff, mBinaryList);
		break;
		case RawListLong:
		{
			unsigned long aSize = mLongListList.size();
			if ( aSize <= MAX_ELEMENTS)
			{
				// append type and number of elments
				aReturnBuff.append(1, ParamType_List);
				aReturnBuff.append(1, aSize);
				// append string list
				for (LongListList::const_iterator aItr = mLongListList.begin();aItr != mLongListList.end() && aSucceed; ++aItr)
				{
					aSucceed = AppendLongList(aReturnBuff, *aItr);
				}
			}
			else
			{
				aSucceed = false;
			}
		}
		break;
		case RawListString:
		{
			unsigned long aSize = mStringListList.size() ;
			if ( aSize <= MAX_ELEMENTS)
			{
				// append type and number of elments
				aReturnBuff.append(1, ParamType_List);
				aReturnBuff.append(1, aSize);
				// append string list
				for (StringListList::const_iterator aItr = mStringListList.begin();aItr != mStringListList.end() && aSucceed; ++aItr)
				{
					aSucceed = AppendStringList(aReturnBuff, *aItr);
				}
			}
			else
			{
				aSucceed = false;
			}
		}
		break;
		default: // RawUnknown
			aSucceed = false;
		break;
	}

	if (!aSucceed) // If failed send back an empty list
	{
		aReturnBuff.erase();
	}

	return aReturnBuff;
}

wstring
RawList::GetString()
{
	wstring aReturnString;
	bool aSucceed = true;

	switch (mListType)
	{
		case RawUnicode:
			aSucceed = AppendUnicodeList(aReturnString, mUnicodeList);
		break;
		case RawString: 
			aSucceed = AppendStringList(aReturnString, mStringList);
		break;
		case RawLong:
			aSucceed = AppendLongList(aReturnString, mLongList);
		break;
		case RawBool:
			aSucceed = AppendBoolList(aReturnString, mBoolList);
		break;
		case RawBinary:
			aSucceed = AppendBinaryList(aReturnString, mBinaryList);
		break;
		case RawListLong:
		{
			// append long list	
			bool aComma = false;
			for (LongListList::const_iterator aItr = mLongListList.begin();aItr != mLongListList.end() && aSucceed; ++aItr)
			{
				if (aComma)
				{
					aReturnString.append(L",");
				}
				else
				{
					aComma = true;
				}
				aReturnString.append(L"(");
				aSucceed = AppendLongList(aReturnString, *aItr);
				aReturnString.append(L")");
			}
		}
		break;
		case RawListString:
		{
			// append string list
			bool aComma = false;
			for (StringListList::const_iterator aItr = mStringListList.begin();aItr != mStringListList.end() && aSucceed; ++aItr)
			{
				if (aComma)
				{
					aReturnString.append(L",");
				}
				else
				{
					aComma = true;
				}
				aReturnString.append(L"(");
				aSucceed = AppendStringList(aReturnString, *aItr);
				aReturnString.append(L")");
				
			}
		}
		break;
		default: // RawUnknown
			bool aSucceed = false;
		break;
	}

	if (!aSucceed) // If failed send back an empty list
	{
		aReturnString.erase();
	}

	return aReturnString;
}

////////////////////////////////////////////////////////////////////////////////
// Private Util functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Append to RawBuffer routines
bool
RawList::AppendUnicode(RawBuffer& theRawBufferR, const wstring& theInWStringR)
{
	bool aReturn = false;

	unsigned short aSize = theInWStringR.size()*2;
	if ( aSize <= MAX_LENGTH)
	{
		// append 2 byte length
		theRawBufferR.append((unsigned char *)&aSize, LENGTH_BYTES);

		// append string
		theRawBufferR.append((unsigned char*)theInWStringR.data(), aSize);
		aReturn = true;
	}
	return aReturn;
}

bool
RawList::AppendString(RawBuffer& theRawBufferR, const string& theInStringR)
{
	bool aReturn = false;

	unsigned short aSize = theInStringR.size();
	if (aSize<= MAX_LENGTH)
	{
		// append 2 byte length
		theRawBufferR.append((unsigned char *)&aSize, LENGTH_BYTES);

		// append string
		theRawBufferR.append((unsigned char*)theInStringR.data(), aSize);
		aReturn = true;
	}

	return aReturn;
}

bool
RawList::AppendLong(RawBuffer& theRawBufferR, const long theInLong)
{
	// append long
	theRawBufferR.append((unsigned char*)&theInLong, 4);

	return true;
}

bool
RawList::AppendBool(RawBuffer& theRawBufferR, const long theInBool)
{
	// append bool
	theRawBufferR.append((unsigned char*)&theInBool, 1);

	return true;
}

bool
RawList::AppendBinary(RawBuffer& theRawBufferR, const RawBuffer& theInRawBufferR)
{
	bool aReturn = false;

	unsigned short aSize = theInRawBufferR.size();
	if ( aSize <= MAX_LENGTH)
	{
		// append 2 byte length
		theRawBufferR.append((unsigned char *)&aSize, LENGTH_BYTES);

		// append RawBuffer
		theRawBufferR.append(theInRawBufferR);
		aReturn = true;
	}

	return aReturn;
}

bool
RawList::AppendUnicodeList(RawBuffer& theRawBufferR, const UnicodeList& theInUnicodeListR)
{
	bool aReturn = true;

	unsigned int aSize = theInUnicodeListR.size();
	if (aSize <= MAX_ELEMENTS)
	{
		// append type and number of elments
		theRawBufferR.append(1, (unsigned char)ParamType_WString);
		theRawBufferR.append(1, aSize);
		// append string list
		for (UnicodeList::const_iterator aItr = theInUnicodeListR.begin();aItr != theInUnicodeListR.end() && aReturn; ++aItr)
		{
			aReturn = AppendUnicode(theRawBufferR, *aItr);
		}
	}
	else
	{
		aReturn = false;
	}

	return aReturn;
}

bool
RawList::AppendStringList(RawBuffer& theRawBufferR, const StringList& theInStringListR)
{
	bool aReturn = true;

	unsigned int aSize = theInStringListR.size();
	if (aSize <= MAX_ELEMENTS)
	{
		// append type and number of elments
		theRawBufferR.append(1, (unsigned char)ParamType_String);
		theRawBufferR.append(1, aSize);
		// append string list
		for (StringList::const_iterator aItr = theInStringListR.begin();aItr != theInStringListR.end() && aReturn; ++aItr)
		{
			aReturn = AppendString(theRawBufferR, *aItr);
		}
	}
	else
	{
		aReturn = false;
	}

	return aReturn;
}

bool
RawList::AppendLongList(RawBuffer& theRawBufferR, const LongList& theInLongListR)
{
	bool aReturn = true;

	unsigned int aSize = theInLongListR.size();
	if (aSize <= MAX_ELEMENTS)
	{
		// append type and number of elments
		theRawBufferR.append(1, (unsigned char)ParamType_Long);
		theRawBufferR.append(1, aSize);
		// append long list
		for (LongList::const_iterator aItr = theInLongListR.begin();aItr != theInLongListR.end() && aReturn; ++aItr)
		{
			aReturn = AppendLong(theRawBufferR, *aItr);
		}
	}
	else
	{
		aReturn = false;
	}

	return aReturn;
}

bool
RawList::AppendBoolList(RawBuffer& theRawBufferR, const BoolList& theInBoolListR)
{
	bool aReturn = true;

	unsigned int aSize = theInBoolListR.size();
	if ( aSize <= MAX_ELEMENTS)
	{
		// append type and number of elments
		theRawBufferR.append(1, (unsigned char)ParamType_Bool);
		theRawBufferR.append(1, aSize);
		// append long list
		for (BoolList::const_iterator aItr = theInBoolListR.begin();aItr != theInBoolListR.end() && aReturn; ++aItr)
		{
			aReturn = AppendBool(theRawBufferR, *aItr);
		}
	}
	else
	{
		aReturn = false;
	}

	return aReturn;
}

bool
RawList::AppendBinaryList(RawBuffer& theRawBufferR, const BinaryList& theInBinaryListR)
{
	bool aReturn = true;

	unsigned int aSize = theInBinaryListR.size();
	if ( aSize <= MAX_ELEMENTS)
	{
		// append type and number of elments
		theRawBufferR.append(1, (unsigned char)ParamType_Binary);
		theRawBufferR.append(1, aSize);
		// append Binary list
		for (BinaryList::const_iterator aItr = theInBinaryListR.begin();aItr != theInBinaryListR.end() && aReturn; ++aItr)
		{
			aReturn = AppendBinary(theRawBufferR, *aItr);
		}
	}
	else
	{
		aReturn = false;
	}

	return aReturn;
}



////////////////////////////////////////////////////////////////////////////////
// Append to String routines
bool
RawList::AppendUnicode(wstring& theWStringR, const wstring& theInWStringR)
{
	// append string
	theWStringR.append(theInWStringR);

	return true;
}

bool
RawList::AppendString(wstring& theWStringR, const string& theInStringR)
{
	// append string
	theWStringR.append(StringToWString(theInStringR));

	return true;
}

bool
RawList::AppendLong(wstring& theWStringR, const long theInLong)
{
	wchar_t buffer[15];

	_ltow(theInLong,buffer,10);
	// append long
	theWStringR.append(buffer);

	return true;
}

bool
RawList::AppendBool(wstring& theWStringR, const bool theInBool)
{
	// append bool
	theWStringR.append(theInBool ? L"true" : L"false");

	return true;
}

bool
RawList::AppendBinary(wstring& theWStringR, const RawBuffer& theInRawBufferR)
{
	wchar_t buffer[3];
	// append binary
	for (RawBuffer::const_iterator aItr = theInRawBufferR.begin(); aItr != theInRawBufferR.end();++aItr)
	{
		int temp = *aItr;
		_itow(temp,buffer,16);
		theWStringR.append(buffer);
	}

	return true;
}


bool
RawList::AppendUnicodeList(wstring& theWStringR, const UnicodeList& theInUnicodeListR)
{
	bool aReturn = true;

	// append Unicode list
	bool aComma = false;
	for (UnicodeList::const_iterator aItr = theInUnicodeListR.begin();aItr != theInUnicodeListR.end() && aReturn; ++aItr)
	{
		if (aComma)
		{
			theWStringR.append(L",");
		}
		else
		{
			aComma = true;
		}
		aReturn = AppendUnicode(theWStringR, *aItr);
	}

	return aReturn;
}

bool
RawList::AppendStringList(wstring& theWStringR, const StringList& theInStringListR)
{
	bool aReturn = true;

	// append string list
	bool aComma = false;
	for (StringList::const_iterator aItr = theInStringListR.begin();aItr != theInStringListR.end() && aReturn; ++aItr)
	{
		if (aComma)
		{
			theWStringR.append(L",");
		}
		else
		{
			aComma = true;
		}
		aReturn = AppendString(theWStringR, *aItr);
	}

	return aReturn;
}

bool
RawList::AppendLongList(wstring& theWStringR, const LongList& theInLongListR)
{
	bool aReturn = true;

	// append long list
	bool aComma = false;
	for (LongList::const_iterator aItr = theInLongListR.begin();aItr != theInLongListR.end() && aReturn; ++aItr)
	{
		if (aComma)
		{
			theWStringR.append(L",");
		}
		else
		{
			aComma = true;
		}
		aReturn = AppendLong(theWStringR, *aItr);
	}

	return aReturn;
}

bool
RawList::AppendBoolList(wstring& theWStringR, const BoolList& theInBoolListR)
{
	bool aReturn = true;

	// append Bool list
	bool aComma = false;
	for (BoolList::const_iterator aItr = theInBoolListR.begin();aItr != theInBoolListR.end() && aReturn; ++aItr)
	{
		if (aComma)
		{
			theWStringR.append(L",");
		}
		else
		{
			aComma = true;
		}
		aReturn = AppendBool(theWStringR, *aItr);
	}

	return aReturn;
}

bool
RawList::AppendBinaryList(wstring& theWStringR, const BinaryList& theInBinaryListR)
{
	bool aReturn = true;

	// append Binary list
	bool aComma = false;
	for (BinaryList::const_iterator aItr = theInBinaryListR.begin();aItr != theInBinaryListR.end() && aReturn; ++aItr)
	{
		if (aComma)
		{
			theWStringR.append(L",");
		}
		else
		{
			aComma = true;
		}
		aReturn = AppendBinary(theWStringR, *aItr);
	}

	return aReturn;
}


////////////////////////////////////////////////////////////////////////////////
// Read from RawBuffer routines

// Returns the number of bytes read RawError if failed
int
RawList::ReadUnicode(const RawBuffer& theRawBufferR, wstring& theOutWStringR)
{
	// Read 2 length bytes at front 
	int aLength = *(unsigned short*)theRawBufferR.data();

	// Read WString
	theOutWStringR.erase();
	wchar_t* aCurCharP = (wchar_t*)theRawBufferR.c_str()+1;
	for (int i=0; i<aLength; i+=2, aCurCharP++)
	{
		theOutWStringR.append(aCurCharP, 1);
	}
	return aLength+2; // add length bytes
}

int
RawList::ReadString(const RawBuffer& theRawBufferR, string& theOutStringR)
{
	// Read 2 length bytes at front 
	int aLength = *(unsigned short*)theRawBufferR.data();

	// Read String
	theOutStringR.erase();
	char* aCurCharP = (char *)theRawBufferR.c_str()+2;
	for (int i=0; i<aLength; ++i, aCurCharP++)
	{
		theOutStringR.append(aCurCharP, 1);
	}
	return aLength+2; // add length bytes
}

int
RawList::ReadLong(const RawBuffer& theRawBufferR, long& theOutLongR)
{
	// Read long
	theOutLongR = *(long*)theRawBufferR.data();

	return 4;
}

int
RawList::ReadBool(const RawBuffer& theRawBufferR, bool& theOutBool)
{
	// Read bool
	theOutBool = *(bool*)theRawBufferR.data();

	return 1;
}

int
RawList::ReadBinary(const RawBuffer& theRawBufferR, RawBuffer& theOutRawBufferR)
{
	// Read 2 length bytes at front 
	int aLength = *(unsigned short*)theRawBufferR.data();

	// Read Binary
	theOutRawBufferR = theRawBufferR.substr(2, aLength);

	return aLength+2;
}

// Returns number of bytes read or error
/*
int
RawList::ReadUnicodeList(const RawBuffer& theRawBufferR, UnicodeList& theOutUnicodeListR)
{
	int aReturn = 0;
	int aNumElements = *(unsigned char*)theRawBufferR.data();

	// Read long list
	int aBeginPos = 1; // Skip first length byte
	int aEndPos = theRawBufferR.size()-1;
	if (aBeginPos < aEndPos)
	{
		for (int i=0; i< aNumElements && aBeginPos < aEndPos; ++i)
		{
			wstring temp;
			int aNumRead = ReadUnicode(theRawBufferR.substr(aBeginPos, aEndPos), temp);
			if (aNumRead > RawError)
			{
				aBeginPos += aNumRead;
				theOutUnicodeListR.push_back(temp);
			}
			else // Error
			{
				aReturn = RawError;
			}
		}
	}
	else // error
	{
		aReturn = RawError;
	}

	// We need to know how many bytes have been consumed
	if (aReturn != RawError)
	{
		aReturn = aBeginPos;
	}

	return aReturn;
}

int
RawList::ReadStringList(const RawBuffer& theRawBufferR, StringList& theOutStringListR)
{
	int aReturn = 0;
	int aNumElements = *(unsigned char*)theRawBufferR.data();

	// Read long list
	int aBeginPos = 1; // Skip first length byte
	int aEndPos = theRawBufferR.size()-1;
	if (aBeginPos < aEndPos)
	{
		for (int i=0; i< aNumElements && aBeginPos < aEndPos; ++i)
		{
			string temp;
			int aNumRead = ReadString(theRawBufferR.substr(aBeginPos, aEndPos), temp);
			if (aNumRead > RawError)
			{
				aBeginPos += aNumRead;
				theOutStringListR.push_back(temp);
			}
			else // Error
			{
				aReturn = RawError;
			}
		}
	}
	else // error
	{
		aReturn = RawError;
	}

	// We need to know how many bytes have been consumed
	if (aReturn != RawError)
	{
		aReturn = aBeginPos;
	}

	return aReturn;
}

int
RawList::ReadLongList(const RawBuffer& theRawBufferR, LongList& theOutLongListR)
{
	int aReturn = 0;
	int aNumElements = *(unsigned char*)theRawBufferR.data();

	// Read long list
	int aBeginPos = 1; // Skip first length byte
	int aEndPos = theRawBufferR.size()-1;
	if (aBeginPos < aEndPos)
	{
		for (int i=0; i< aNumElements && aBeginPos < aEndPos; ++i)
		{
			long temp = 0;
			aBeginPos += ReadLong(theRawBufferR.substr(aBeginPos, aEndPos), temp);
			theOutLongListR.push_back(temp);
		}
	}
	else // error
	{
		aReturn = RawError;
	}

	// We need to know how many bytes have been consumed
	if (aReturn != RawError)
	{
		aReturn = aBeginPos;
	}

	return aReturn;
}
*/
// Returns number of bytes read or error
int
RawList::ReadList(const RawBuffer& theRawBufferR, void* theListP, ListTypeEnum theListType)
{
	int aReturn = 0;
	int aNumElements = *(unsigned char*)theRawBufferR.data();

	// Read long list
	int aBeginPos = 1; // Skip first length byte
	int aCurrLength = theRawBufferR.size()-1;
	if (aCurrLength > 0)
	{
		for (int i=0; i< aNumElements && aCurrLength > 0; ++i)
		{
			switch (theListType)
			{
				case RawUnicode:
				{
					wstring temp;
					int aNumRead = ReadUnicode(theRawBufferR.substr(aBeginPos, aCurrLength), temp);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((UnicodeList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				case RawString:
				{
					string temp;
					int aNumRead = ReadString(theRawBufferR.substr(aBeginPos, aCurrLength), temp);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((StringList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				case RawLong:
				{
					long temp;
					int aNumRead = ReadLong(theRawBufferR.substr(aBeginPos, aCurrLength), temp);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((LongList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				case RawBool:
				{
					bool temp;
					int aNumRead = ReadBool(theRawBufferR.substr(aBeginPos, aCurrLength), temp);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((BoolList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}	
				break;
				case RawBinary:
				{
					RawBuffer temp;
					int aNumRead = ReadBinary(theRawBufferR.substr(aBeginPos, aCurrLength), temp);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((BinaryList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				case RawListLong:
				{
					LongList temp;
					// Read sublist
					int aNumRead = ReadList(theRawBufferR.substr(++aBeginPos, --aCurrLength), (void*)&temp, RawLong);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((LongListList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				case RawListString:
				{
					StringList temp;
					// Read sublist
					int aNumRead = ReadList(theRawBufferR.substr(++aBeginPos, --aCurrLength), (void*)&temp, RawString);
					if (aNumRead > RawError)
					{
						aBeginPos += aNumRead;
						aCurrLength -= aNumRead;
						((StringListList*)theListP)->push_back(temp);
					}
					else // Error
					{
						aReturn = RawError;
					}
				}
				break;
				default:
					aReturn = RawError;
				break;
			}
		}
	}
	else // error
	{
		aReturn = RawError;
	}

	// We need to know how many bytes have been consumed
	if (aReturn != RawError)
	{
		aReturn = aBeginPos;
	}

	return aReturn;
}

////////////////////////////////////////////////////////////////////////////////
// Read from String routines

// Returns the number of characters read RawError if failed
int
RawList::ReadUnicode(const wstring& theWStringR, wstring& theOutWStringR)
{
	int aLength = theWStringR.size();

	theOutWStringR = theWStringR;

	return aLength;
}

int
RawList::ReadString(const wstring& theWStringR, string& theOutStringR)
{
	int aLength = theWStringR.size();

	// Read string
	theOutStringR = WStringToString(theWStringR);

	return aLength;
}

int
RawList::ReadLong(const wstring& theWStringR, long& theOutLongR)
{
	int aLength = theWStringR.size();

	// Read long
	theOutLongR = _wtol(theWStringR.c_str());

	return aLength;
}

int
RawList::ReadBool(const wstring& theWStringR, bool& theOutBoolR)
{
	int aLength = theWStringR.size();

	// Read bool
	if (!StrcmpNoCase(theWStringR, L"true") || !StrcmpNoCase(theWStringR, L"on") || !StrcmpNoCase(theWStringR, L"1"))
	{
		theOutBoolR = true;
	}
	else
	{
		if ( !StrcmpNoCase(theWStringR, L"false") || !StrcmpNoCase(theWStringR, L"off") || !StrcmpNoCase(theWStringR, L"0") )
		{
			theOutBoolR = false;
		}
		else // Error
		{
			aLength = RawError;
		}
	}

	return aLength;
}

int
RawList::ReadBinary(const wstring& theWStringR, RawBuffer& theOutRawBufferR)
{
	int aLength = theWStringR.size();

	theOutRawBufferR.erase();

	int aNumCharToRead;

	// if we have an odd number of characters insert a 0 at front 
	// Ex. FFF => 0FFF
	bool aMakeEvenLength = false;
	if (aLength%2) // Odd length
	{
		aNumCharToRead = aLength+1;
		aMakeEvenLength = true;
	}
	else
	{
		aNumCharToRead = aLength;
	}

	// Read binary
	// Convert 2 characters to 1 byte at a time
	long aValue = 0;
	wchar_t* aCurrStringP = (wchar_t*)(theWStringR.c_str());
	wchar_t aCurrConvert[3];
	wchar_t* aEndP = NULL;
	memset(aCurrConvert,0,sizeof(wchar_t)*3);
	for (int i = 0; i < aNumCharToRead; i+=2)
	{
		if (aMakeEvenLength)
		{
			aCurrConvert[0] = '0';
			aMakeEvenLength = false;
		}
		else
		{
			aCurrConvert[0] = *(aCurrStringP++);
		}
		aCurrConvert[1] = *(aCurrStringP++);
		aValue = wcstoul(aCurrConvert, &aEndP, 16);
		if(aValue <= UCHAR_MAX)
		{
			theOutRawBufferR += (unsigned char)(aValue);	
		}
		else // error
		{
			aLength = RawError;
		}
	}

	return aLength;
}

// List in the form element,element,...
// Input sting always contains this string and only this string
int
RawList::ReadList(const wstring& theWStringR, void* theListP, ListTypeEnum theListType)
{
	int aLength = theWStringR.size();

	// Read list
	int aBeginPos = 0;
	int aNumRead = 0;

	int aEndPos = theWStringR.find(L",");
	int aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
	while (aCurrLength > 0 && aNumRead != RawError)
	{
		switch (theListType)
		{
			case RawUnicode:
			{
				wstring temp;
				aNumRead = ReadUnicode(theWStringR.substr(aBeginPos, aCurrLength), temp);
				((UnicodeList*)theListP)->push_back(temp);
				aBeginPos += aCurrLength+1;
				aEndPos = theWStringR.find(L",", aBeginPos);
				aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
			}
			break;
			case RawString:
			{
				string temp;
				aNumRead = ReadString(theWStringR.substr(aBeginPos, aCurrLength), temp);
				((StringList*)theListP)->push_back(temp);
				aBeginPos += aCurrLength+1;
				aEndPos = theWStringR.find(L",", aBeginPos);
				aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
			}
			break;
			case RawLong:
			{
				long temp;
				aNumRead = ReadLong(theWStringR.substr(aBeginPos, aCurrLength), temp);
				((LongList*)theListP)->push_back(temp);
				aBeginPos += aCurrLength+1;
				aEndPos = theWStringR.find(L",", aBeginPos);
				aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
			}
			break;
			case RawBool:
			{
				bool temp;
				aNumRead = ReadBool(theWStringR.substr(aBeginPos, aCurrLength), temp);
				((BoolList*)theListP)->push_back(temp);
				aBeginPos += aCurrLength+1;
				aEndPos = theWStringR.find(L",", aBeginPos);
				aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
			}
			break;
			case RawBinary:
			{
				RawBuffer temp;
				aNumRead = ReadBinary(theWStringR.substr(aBeginPos, aCurrLength), temp);
				((BinaryList*)theListP)->push_back(temp);
				aBeginPos += aCurrLength+1;
				aEndPos = theWStringR.find(L",", aBeginPos);
				aCurrLength = aEndPos != -1 ? aEndPos-aBeginPos : aLength-aBeginPos;
			}
			break;
			default:
				aLength = RawError;
			break;
		}
	}

	if (aNumRead == RawError) // error
	{
		aLength = RawError;
	}

	return aLength;
}

// Read lists of lists in the form (elem,elem),(elem,elem,elem),(elem)...
// Return number of characters read
int 
RawList::ReadListList(const wstring& theWStringR, void* theListListP, ListTypeEnum theListListType)
{
	int aLength = theWStringR.size();
	int aBeginPos = theWStringR.find(L"(")+1;
	int aEndPos = theWStringR.find(L")", aBeginPos)-1;

	if (aBeginPos > 0 )
	{
		if (aEndPos > 0 )
		{
			int aNumRead = 0;
			bool aDone = false;
			int aCurrLength = aEndPos-aBeginPos+1;
			while (aCurrLength > 0 && !aDone && aNumRead != RawError)
			{
				if (theListListType == RawListLong)
				{
					LongList temp;
					aNumRead = ReadList(theWStringR.substr(aBeginPos, aCurrLength), (void*)&temp, RawLong);
					((LongListList*)theListListP)->push_back(temp);
				}
				else if (theListListType == RawListString)
				{
					StringList temp;
					aNumRead = ReadList(theWStringR.substr(aBeginPos, aCurrLength), (void*)&temp, RawString);
					((StringListList*)theListListP)->push_back(temp);
				} 
				else // Error
				{
					aNumRead = RawError;
				}
				if (aNumRead != RawError)
				{
					aBeginPos = theWStringR.find(L"(", aBeginPos)+1;
					if (aBeginPos > 0)
					{
						aEndPos = theWStringR.find(L")", aBeginPos)-1;
						if (aEndPos > 0)
						{
							aCurrLength = aEndPos-aBeginPos+1;	
						}
						else // No end parenthesis
						{
							aNumRead = RawError;	
						}
						
					}
					else
					{
						aDone = true;
					}
				}
			}
			if (aNumRead == RawError)
			{
				aLength = RawError;
			}
		}
		else // error
		{
			aLength = RawError;
		}
	}
	else // No first ( treat like a normal list
	{
		int aNumRead = 0;
		if (theListListType == RawListLong)
		{
			LongList temp;
			aNumRead = ReadList(theWStringR, (void*)&temp, RawLong);
			((LongListList*)theListListP)->push_back(temp);
		}
		else if (theListListType == RawListString)
		{
			StringList temp;
			aNumRead = ReadList(theWStringR, (void*)&temp, RawString);
			((StringListList*)theListListP)->push_back(temp);
		} 
		else // Error
		{
			aNumRead = RawError;
		}
		if (aNumRead == RawError)
		{
			aLength = RawError;
		}
	}
	return aLength;
}
