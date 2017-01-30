#include "won.h"
//#include <crtdbg.h> // for _ASSERT
#include "UTF8String.h"
//#include <winnls.h>

namespace {
	using WONCommon::UTF8String;
};

UTF8String::UTF8String() :
	mLength(-1)
{}

UTF8String::UTF8String(const unsigned char* theUTF8Data, unsigned int theNumBytes) :
	mRawString(theUTF8Data, theNumBytes),
	mLength(-1)
{
	RecalculateLength();
}

UTF8String::UTF8String(const std::string& theStringR)
{
	InitFromASCII(theStringR);
}

UTF8String::UTF8String(const std::wstring& theWideStringR)
{
	InitFromUCS2(theWideStringR);
}

UTF8String::UTF8String(const std::basic_string<unsigned char>& theUnsignedStringR)
{
	InitFromUnsigned(theUnsignedStringR);
}

UTF8String::UTF8String(const UTF8String& aUTF8String) :
	mRawString(aUTF8String.mRawString), 
	mLength(aUTF8String.mLength)
{}

UTF8String::~UTF8String()
{}

UTF8String& UTF8String::operator=(const std::string& theStringR)
{
	InitFromASCII(theStringR);
	return *this;
}

UTF8String& UTF8String::operator=(const std::wstring& theWideStringR)
{
	InitFromUCS2(theWideStringR);
	return *this;
}

UTF8String& UTF8String::operator=(const std::basic_string<unsigned char>& theUnsignedStringR)
{
	InitFromUnsigned(theUnsignedStringR);
	return *this;
}

UTF8String& UTF8String::operator=(const UTF8String& aUTF8String) 
{
	mRawString = aUTF8String.mRawString; 
	mLength = aUTF8String.mLength; 
	return *this; 
}

UTF8String& UTF8String::operator+=(const UTF8String& aString)
{ 
	mRawString += aString.mRawString; 
	mLength += aString.mLength; 
	return *this; 
}

void UTF8String::InitFromASCII(const std::string& theStringR)
{
	// Since all of the incoming characters are 7-bit ASCII, it's
	// essentially already in UTF8.
	mRawString.assign(reinterpret_cast<const unsigned char*>(theStringR.c_str()));
	mLength = theStringR.size();
}

void UTF8String::InitFromUnsigned(const std::basic_string<unsigned char>& theUnsignedStringR)
{
	const unsigned char FirstBitOn   = 0x80; 
	const unsigned char SecondBitOff = 0xBF;
	
	unsigned char ch;
	unsigned char aRawCharacter[3];
	memset(aRawCharacter, 0, sizeof(aRawCharacter));

	for (unsigned int i = 0; i < theUnsignedStringR.size(); i++)
	{
		ch = theUnsignedStringR[i];

		if (ch < 0x80)
			mRawString += ch;
		else
		{
			aRawCharacter[0] = 0xC0 + (ch >> 6);
			aRawCharacter[1] = (ch | FirstBitOn) & SecondBitOff;
			mRawString += aRawCharacter;
		}
	}

	mLength = theUnsignedStringR.size();
}

void UTF8String::InitFromUCS2(const std::wstring& theWideStringR)
{
	// allocate a temporary buffer for the converted value	
	int            aLen = 3 * theWideStringR.size() + 1;
	unsigned char* aBuf = new unsigned char[aLen];

	// do the conversion
	// FIX: wanted to use WC_NO_BEST_FIT_CHARS flag here, but for some reason it
	//      seems that the VC5.0 winnls.h is out of date and doesn't define it.
	if (WideCharToMultiByte(CP_UTF8,    // code page
							0, // performance and mapping flags
							theWideStringR.c_str(), // address of wide-character string
							-1,         // number of characters in string
							reinterpret_cast<char*>(aBuf), // address of buffer for new string
							aLen,       // size of buffer
							NULL,       // address of default for unmappable characters
							NULL) == 0) // address of flag set when default char. used
	{
		//DWORD dwError = GetLastError();
		//_ASSERT(FALSE);
		delete[] aBuf;
		mRawString.erase();
		return;
	}

    RecalculateLength();

	// save the converted value and delete the temporary buffer
	mRawString = aBuf;
	delete[] aBuf;

/*
	          ---------- MY CODE ----------
  
	const wchar CharacterLengthCodes[4] = { 0x00, 0x00, 0xC0, 0xE0 };
	const wchar FirstBitOn   = 0x80; 
	const wchar SecondBitOff = 0xBF;
		
	// FIX: use register?
	unsigned char aRawCharacter[4];
	wchar       ch;
	int           aBytesToWrite = 0;

	for (int i = 0; i < theWideString.size(); i++)
	{
		// get the next wide character
		ch = theWideString[i];

		// determine how many bytes are needed to store the current
		// character in UTF8.
		if (ch < 0x80)
			aBytesToWrite = 1;
		else if (ch < 0x800)
			aBytesToWrite = 2;
		else if (ch < 0x10000)
			aBytesToWrite = 3;
		
		memset(aRawCharacter, 0, sizeof(aRawCharacter));
		unsigned char* pch = aRawCharacter + aBytesToWrite;
		switch (aBytesToWrite) 
		{ // note: code falls through cases!
			case 3:	*--pch = (ch | FirstBitOn) & SecondBitOff; ch >>= 6;
			case 2:	*--pch = (ch | FirstBitOn) & SecondBitOff; ch >>= 6;
			case 1:	*--pch =  ch | CharacterLengthCodes[aBytesToWrite];
		}
		
		mRawString += aRawCharacter;
	}
	
	mLength = theWideString.size();
*/
}

unsigned char CharacterWidth[256] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
							         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
							         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
							         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
							         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
							         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                     2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
							         2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
                                     3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
							         4,4,4,4,4,4,4,4,5,5,5,5,6,6,6,6};

UTF8String::operator std::string() const
{
	const SingleByteReplacementCharacter = 0x00;
	
	std::string aNarrowString;
	for (unsigned int i = 0; i < mRawString.size(); )
	{
		char ch = mRawString[i++];
		if (ch >= 0x80)
		{
			i += CharacterWidth[ch] - 1;
			ch = SingleByteReplacementCharacter;
		}

		aNarrowString += ch;
	}

	return aNarrowString;
}

UTF8String::operator std::basic_string<unsigned char>() const
{
	std::basic_string<unsigned char> aUnsignedString;
	for (unsigned int i = 0; i < mRawString.size(); )
	{
		unsigned char ch = mRawString[i++];
		
		if (ch & 0x80)
			ch = (ch & 0x03) + (mRawString[i++] & 0x3F);
		
		aUnsignedString += ch;
	}

	return aUnsignedString;
}

UTF8String::operator std::wstring() const
{
	// allocate a temporary buffer for the converted value
	int      aLen = mRawString.size() + 1;
	wchar* aBuf = new wchar[aLen];

	// do the conversion
	if (MultiByteToWideChar(CP_UTF8,    // code page
                            0,          // character-type options
                            reinterpret_cast<const char*>(mRawString.c_str()), // address of string to map
                            -1,         // number of bytes in string
                            aBuf,       // address of wide-character buffer
                            aLen) == 0) // size of buffer)
	{
		//DWORD aError = GetLastError();
		//_ASSERT(FALSE);
		delete[] aBuf;
		return std::wstring();
	}
	
	// setup the return value and delete the temporary buffer
	std::wstring aWideString(aBuf);
	delete[] aBuf;
	
	return aWideString;

/* 
	               --------- MY CODE ---------

	const wchar UCS2ReplacementCharacter = 0x0000;
	const wchar UTF8Residue[4] = { 0, 0, 0x3080, 0x1080 };
	std::wstring aWideString;
	
	for (int i = 0; i < mRawString.size(); )
	{
		wchar ch = 0;
		unsigned char aBytesInCharacter = CharacterWidth[mRawString[i]];
		
		switch(theBytesInCharacter) 
		{	// note: code falls through cases!
			case 3:	ch += mRawString[i++]; ch <<= 6;
			case 2:	ch += mRawString[i++]; ch <<= 6;
			case 1:	ch += mRawString[i++]; break;
			default:
				ch = UCS2ReplacementCharacter; i += aBytesInCharacter;
		}

		ch -= UTF8Residue[aBytesInCharacter];
		
		theWideString += ch;
	}

	return aWideString;
*/
}

const unsigned char* UTF8String::c_str() const
{
	return mRawString.c_str();
}

int UTF8String::size() const
{
	return mRawString.size();
}

int UTF8String::length() const
{
	return mLength;
}

void UTF8String::RecalculateLength()
{
	mLength = 0;
	for (unsigned int i = 0; i < mRawString.size(); )
	{
		mLength++;
		i += CharacterWidth[mRawString[i]];
	}
}

UTF8String WONCommon::operator+(const UTF8String& left, const UTF8String& right) {
	UTF8String result = left;
	result += right;
	return result;
}