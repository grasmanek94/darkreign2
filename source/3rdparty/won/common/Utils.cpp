// Utils

// Source module for misc utility methods.  Method prototypes are
// specified in won.h

#include "won.h"
#include <limits.h>
#include "WONEndian.h"


// StrcmpNoCase
// Perform case-insensitive comparisons of 2 strings
// Returns -1 (s1 < s2), 0 (s1 == s2), or 1 (s1 > s2)
int
WONCommon::StrcmpNoCase(const string& s1, const string& s2)
{
	string::const_iterator i1 = s1.begin();
	string::const_iterator i2 = s2.begin();

	while ((i1 != s1.end()) && (i2 != s2.end()))
	{
		if (toupper(*i1) != toupper(*i2))
			return ((toupper(*i1) < toupper(*i2)) ? -1 : 1);

		i1++;  i2++;
	}

	return ((s1.size() == s2.size()) ? 0 : ((s1.size() < s2.size()) ? -1 : 1));
}

// StrcmpNoCase
// Perform case-insensitive comparisons of 2 wstrings
// Returns -1 (s1 < s2), 0 (s1 == s2), or 1 (s1 > s2)
int
WONCommon::StrcmpNoCase(const wstring& s1, const wstring& s2)
{
	wstring::const_iterator i1 = s1.begin();
	wstring::const_iterator i2 = s2.begin();

	while ((i1 != s1.end()) && (i2 != s2.end()))
	{
		if (toupper(*i1) != toupper(*i2))
			return ((toupper(*i1) < toupper(*i2)) ? -1 : 1);

		i1++;  i2++;
	}

	return ((s1.size() == s2.size()) ? 0 : ((s1.size() < s2.size()) ? -1 : 1));
}

// StrToDebugLevel
// Parses a string to a Debug Level constant
WONDebug::Debugger::DebugLevel
WONCommon::StrToDebugLevel(const string& theStr)
{
	if (StrcmpNoCase(theStr,"OFF") == 0)
		return WDBG_OFF;
	else if (StrcmpNoCase(theStr,"AH") == 0)
		return WDBG_APPHIG;
	else if (StrcmpNoCase(theStr,"AM") == 0)
		return WDBG_APPMED;
	else if (StrcmpNoCase(theStr,"AL") == 0)
		return WDBG_APPLOW;
	else if (StrcmpNoCase(theStr,"LH") == 0)
		return WDBG_LIBHIG;
	else if (StrcmpNoCase(theStr,"LM") == 0)
		return WDBG_LIBMED;
	else if (StrcmpNoCase(theStr,"LL") == 0)
		return WDBG_LIBLOW;
	else
		return WDBG_OFF;
}


// AsciiToWide
// Converts an ASCII (char*) buffer to a WIDE (wchar_t*) buffer
// Returns pointer to the WIDE buffer.
// WIDE buffer must be at least twice the size of the ASCII buffer.
wchar_t*
WONCommon::AsciiToWide(wchar_t* wBufP, const char* aBufP, size_t nChars)
{
	if (! wBufP) return NULL;

	*wBufP = '\0';
	if (aBufP)
	{
#ifdef WIN32
		// wBufP better point to a buffer at least twice the size of aBufP
		// or this will blow
		MultiByteToWideChar(CP_ACP, 0, aBufP, nChars, wBufP, nChars);
#else//if defined(_LINUX)
		const char* curSrc = aBufP;
		const char* srcEnd = aBufP + nChars;
		wchar_t* curDst = wBufP;
		while (curSrc != srcEnd)
		{
			*(curDst++) = *(curSrc++);
		}
//#else
//#error unknown platform
#endif
	}

	return wBufP;
}


// StringToWString
// Converts a string to a wstring.  Returns the wstring.
wstring
WONCommon::StringToWString(const string& theStr)
{
	size_t aCopySize = theStr.size();
	if (!aCopySize)
		return wstring();
		
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


wstring
WONCommon::StringToWString(const string& theStr, size_t n)
{
	size_t aStrSize = theStr.size();
	if (!aStrSize || !n)
		return wstring();
	
	size_t aCopySize = (n > aStrSize) ? aStrSize : n;
	
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


wstring
WONCommon::StringToWString(const string& theStr, size_t pos, size_t n)
{
	size_t aStrSize = theStr.size();
	if (aStrSize <= pos || !n)
		return wstring();
	
	size_t remaining = aStrSize - pos;
	size_t aCopySize = (n > remaining) ? remaining : n;
	
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr.data() + pos, aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


wstring
WONCommon::StringToWString(const char* theStr)
{
	if (!theStr || !*theStr)
		return wstring();

	size_t aCopySize = strlen(theStr);
		
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr, aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


// Just in case a char* specified is not NULL terminated
static size_t strnlen(const char* s, size_t n)
{
	size_t result = 0;
	const char* cur = s;
	while (result != n)
	{
		if (!*cur)
			break;
		++cur;
		++result;
	}
	return result;
}


wstring
WONCommon::StringToWString(const char* theStr, size_t n)
{
	if (!theStr || !*theStr || !n)
		return wstring();

	size_t aCopySize = strnlen(theStr, n);
	
	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr, aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


wstring
WONCommon::StringToWString(const char* theStr, size_t pos, size_t n)
{
	if (!theStr || !*theStr || !n)
		return wstring();

	size_t aStrSize = strnlen(theStr, pos + n);
	if (aStrSize <= pos)
		return wstring();

	size_t aCopySize = aStrSize - pos;

	wchar_t* aBuf = new wchar_t[aCopySize + 1];
	AsciiToWide(aBuf, theStr + pos, aCopySize);
	aBuf[aCopySize] = 0;
	wstring  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


void WONCommon::StringToWString(const string& src, wstring& dst)
{
	size_t aStrSize = src.size();
	if (!aStrSize)
		dst.erase();
	else
	{
		wchar_t* aBuf = new wchar_t[aStrSize + 1];
		AsciiToWide(aBuf, src.data(), aStrSize);
		aBuf[aStrSize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::StringToWString(const string& src, size_t n, wstring& dst)
{
	size_t aStrSize = src.size();
	if (!aStrSize || !n)
		dst.erase();
	else
	{
		size_t aCopySize = (n > aStrSize) ? aStrSize : n;
		
		wchar_t* aBuf = new wchar_t[aCopySize + 1];
		AsciiToWide(aBuf, src.data(), aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::StringToWString(const string& src, size_t pos, size_t n, wstring& dst)
{
	size_t aStrSize = src.size();
	if (aStrSize <= pos || !n)
		dst.erase();
	else
	{
		size_t remaining = aStrSize - pos;
		size_t aCopySize = (n > remaining) ? remaining : n;
		
		wchar_t* aBuf = new wchar_t[aCopySize + 1];
		AsciiToWide(aBuf, src.data() + pos, aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::StringToWString(const char* src, wstring& dst)
{
	if (!src || !*src)
		dst.erase();
	else
	{
		size_t aStrSize = strlen(src);

		wchar_t* aBuf = new wchar_t[aStrSize + 1];
		AsciiToWide(aBuf, src, aStrSize);
		aBuf[aStrSize] = 0;
		dst = aBuf;

		delete [] aBuf;
	}
}


void WONCommon::StringToWString(const char* src, size_t n, wstring& dst)
{
	if (!src || !*src || !n)
		dst.erase();
	else
	{
		size_t aCopySize = strnlen(src, n);
		
		wchar_t* aBuf = new wchar_t[aCopySize + 1];
		AsciiToWide(aBuf, src, aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::StringToWString(const char* src, size_t pos, size_t n, wstring& dst)
{
	if (!src || !*src || !n)
		dst.erase();
	else
	{
		size_t aStrSize = strnlen(src, pos + n);
		if (aStrSize <= pos)
			dst.erase();
		else
		{
			size_t aCopySize = aStrSize - pos;
			
			wchar_t* aBuf = new wchar_t[aCopySize + 1];
			AsciiToWide(aBuf, src + pos, aCopySize);
			aBuf[aCopySize] = 0;
			dst = aBuf;

			delete[] aBuf;
		}
	}
}


// WideToAscii
// Converts a WIDE (wchar_t*) buffer to an ASCII (char*) buffer
// Returns pointer to the ASCII buffer.
// ASCII buffer must be at least the size of the ASCII buffer.
char*
WONCommon::WideToAscii(char* aBufP, const wchar_t* wBufP, size_t nChars)
{
	if (! aBufP) return NULL;

	*aBufP = '\0';
	if (wBufP)
	{
#ifdef WIN32
		// aBufP better point to a buffer at least the size of wBufP
		// or this will blow
		WideCharToMultiByte(CP_ACP, 0, wBufP, nChars, aBufP, nChars, NULL, NULL);
#else//if defined(_LINUX)
		const wchar_t* curSrc = wBufP;
		const wchar_t* srcEnd = wBufP + nChars;
		char* curDst = aBufP;
		while (curSrc != srcEnd)
		{
			*(curDst++) = *(curSrc++);
		}
//#else
//#error unknown platform
#endif
	}

	return aBufP;
}


string
WONCommon::WStringToString(const wstring& theStr)
{
	size_t aCopySize = theStr.size();
	if (!aCopySize)
		return string();
		
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


string
WONCommon::WStringToString(const wstring& theStr, size_t n)
{
	size_t aStrSize = theStr.size();
	if (!aStrSize || !n)
		return string();
	
	size_t aCopySize = (n > aStrSize) ? aStrSize : n;
	
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr.data(), aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


string
WONCommon::WStringToString(const wstring& theStr, size_t pos, size_t n)
{
	size_t aStrSize = theStr.size();
	if (aStrSize <= pos || !n)
		return string();
	
	size_t remaining = aStrSize - pos;
	size_t aCopySize = (n > remaining) ? remaining : n;
	
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr.data() + pos, aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


string
WONCommon::WStringToString(const wchar_t* theStr)
{
	if (!theStr || !*theStr)
		return string();

	size_t aCopySize = wcslen(theStr);
		
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr, aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


// Just in case a char* specified is not NULL terminated
static size_t wcsnlen(const wchar_t* s, size_t n)
{
	size_t result = 0;
	const wchar_t* cur = s;
	while (result != n)
	{
		if (!*cur)
			break;
		++cur;
		++result;
	}
	return result;
}


string
WONCommon::WStringToString(const wchar_t* theStr, size_t n)
{
	if (!theStr || !*theStr || !n)
		return string();

	size_t aCopySize = wcsnlen(theStr, n);
	
	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr, aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


string
WONCommon::WStringToString(const wchar_t* theStr, size_t pos, size_t n)
{
	if (!theStr || !*theStr || !n)
		return string();

	size_t aStrSize = wcsnlen(theStr, pos + n);
	if (aStrSize <= pos)
		return string();

	size_t aCopySize = aStrSize - pos;

	char* aBuf = new char[aCopySize + 1];
	WideToAscii(aBuf, theStr + pos, aCopySize);
	aBuf[aCopySize] = 0;
	string  aRet(aBuf);

	delete[] aBuf;
	return aRet;
}


void WONCommon::WStringToString(const wstring& src, string& dst)
{
	size_t aStrSize = src.size();
	if (!aStrSize)
		dst.erase();
	else
	{
		char* aBuf = new char[aStrSize + 1];
		WideToAscii(aBuf, src.data(), aStrSize);
		aBuf[aStrSize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::WStringToString(const wstring& src, size_t n, string& dst)
{
	size_t aStrSize = src.size();
	if (!aStrSize || !n)
		dst.erase();
	else
	{
		size_t aCopySize = (n > aStrSize) ? aStrSize : n;
		
		char* aBuf = new char[aCopySize + 1];
		WideToAscii(aBuf, src.data(), aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::WStringToString(const wstring& src, size_t pos, size_t n, string& dst)
{
	size_t aStrSize = src.size();
	if (aStrSize <= pos || !n)
		dst.erase();
	else
	{
		size_t remaining = aStrSize - pos;
		size_t aCopySize = (n > remaining) ? remaining : n;
		
		char* aBuf = new char[aCopySize + 1];
		WideToAscii(aBuf, src.data() + pos, aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::WStringToString(const wchar_t* src, string& dst)
{
	if (!src || !*src)
		dst.erase();
	else
	{
		size_t aStrSize = wcslen(src);

		char* aBuf = new char[aStrSize + 1];
		WideToAscii(aBuf, src, aStrSize);
		aBuf[aStrSize] = 0;
		dst = aBuf;

		delete [] aBuf;
	}
}


void WONCommon::WStringToString(const wchar_t* src, size_t n, string& dst)
{
	if (!src || !*src || !n)
		dst.erase();
	else
	{
		size_t aCopySize = wcsnlen(src, n);
		
		char* aBuf = new char[aCopySize + 1];
		WideToAscii(aBuf, src, aCopySize);
		aBuf[aCopySize] = 0;
		dst = aBuf;

		delete[] aBuf;
	}
}


void WONCommon::WStringToString(const wchar_t* src, size_t pos, size_t n, string& dst)
{
	if (!src || !*src || !n)
		dst.erase();
	else
	{
		size_t aStrSize = wcsnlen(src, pos + n);
		if (aStrSize <= pos)
			dst.erase();
		else
		{
			size_t aCopySize = aStrSize - pos;
			
			char* aBuf = new char[aCopySize + 1];
			WideToAscii(aBuf, src + pos, aCopySize);
			aBuf[aCopySize] = 0;
			dst = aBuf;

			delete[] aBuf;
		}
	}
}

bool WONCommon::ConvertNumberStringToRawBytes(const string& theString, WONCommon::RawBuffer& theRawBytesR, unsigned char theBase /* = 10 */)
{
	
	unsigned long aValue(0l);
	theRawBytesR.erase();

	char* pStart = const_cast<char*>(theString.c_str());
	char* pEnd = NULL;
	while(*pStart != '\0')
	{
		aValue = strtoul(pStart, &pEnd, theBase);
		if(aValue > UCHAR_MAX)
			return false;

		theRawBytesR += static_cast<unsigned char>(aValue);
		pStart = pEnd;
		while(isspace(*pStart)) pStart++;
	}

	return true;

}

bool WONCommon::ConvertWideStringToRawBytes(const wstring& theString, WONCommon::RawBuffer& theRawBytesR)
{
	wstring tmpS(theString);
	makeLittleEndianWString(tmpS);
	theRawBytesR.assign(reinterpret_cast<const unsigned char*>(tmpS.data()), tmpS.length()*2);
	return true;
}

bool WONCommon::ConvertStringToRawBytes(const string& theString, WONCommon::RawBuffer& theRawBytesR)
{
	theRawBytesR.assign(reinterpret_cast<const unsigned char*>(theString.data()), theString.length());
	return true;
}

bool WONCommon::ConvertRawBytesToString(const WONCommon::RawBuffer& theRawBytes, string& theStringR)
{
	theStringR.assign(reinterpret_cast<const char*>(theRawBytes.data()), theRawBytes.length());
	return true;
}

bool WONCommon::ConvertRawBytesToWideString(const WONCommon::RawBuffer& theRawBytes, wstring& theStringR)
{
	if(theRawBytes.length() % 2)
		return false;

	theStringR.assign(reinterpret_cast<const wchar_t*>(theRawBytes.data()), theRawBytes.length()/2);
	makeLittleEndianWString(theStringR);
	return true;
}

