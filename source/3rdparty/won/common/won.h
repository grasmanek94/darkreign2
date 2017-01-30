#ifndef _WONDEFS_H
#define _WONDEFS_H

// won.h
// Standard header file for WON modules

// Turn off all of the level 4 warnings that will pop up if we are using the STL.
#pragma warning(disable : 4097) // Typedef-name used as synonym for class name
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4290) // C++ exception specification ignored
#pragma warning(disable : 4514) // Unreferenced inline function removed
#pragma warning(disable : 4710) // Inline function not expanded
#pragma warning(disable : 4786) // Turn off annoying debug trunc message

// Here's an oxymoron, but it keeps windows.h from including everything.
#define WIN32_LEAN_AND_MEAN

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(disable : 4018) // Signed/unsigned mismatch.
#pragma warning(disable : 4100) // Unreferenced formal parameter.
#pragma warning(disable : 4244) // conversion may loose data.
#pragma warning(disable : 4511) // Copy constructor could not be generated.
#pragma warning(disable : 4512) // Assignment operator could not be generated.
#pragma warning(disable : 4530) // Exception handler used w/o unwind semantics.
#pragma warning(disable : 4663) // Language change (w/r/t templates).
#pragma warning(push, 3) // Turn off warnings within the STL.
#endif // WARNING_LVL_4_AND_STL

// Standard headers
#include <string>
#include <time.h>
#include <memory>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#include <sstream>
typedef unsigned short wchar;
#elif defined(_LINUX)
#include "linuxGlue.h"
#elif defined(macintosh) && (macintosh == 1)
#include "macGlue.h"
#else
#error unknown platform
#endif

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(pop) // Restore warnings.
#endif // WARNING_LVL_4_AND_STL

#include "WONDebug.h"

// Pull in common stuff from std namespace
using std::string;
using std::wstring;
using std::auto_ptr;
using std::endl;
using std::ostream;

#include "array_auto_ptr.h"

// Common Definitions and Utility Methods
namespace WONCommon {

	// Raw binary buffer type
	typedef std::basic_string<unsigned char> RawBuffer;

	// Case insensitive string comparison
	int StrcmpNoCase(const string& s1, const string& s2);
	int StrcmpNoCase(const wstring& s1, const wstring& s2);

	// Convert string to a WONDebugLevel
	WONDebug::Debugger::DebugLevel StrToDebugLevel(const string& theStr);

	// ASCII <--> Wide conversion
	wchar_t* AsciiToWide(wchar_t* wBufP, const char* aBufP, size_t nChars);
	char*	 WideToAscii(char* aBufP, const wchar_t* wBufP, size_t nChars);

	wstring StringToWString(const string& theStr);
	wstring StringToWString(const string& theStr, size_t n);
	wstring StringToWString(const string& theStr, size_t pos, size_t n);

	wstring StringToWString(const char* theStr);
	wstring StringToWString(const char* theStr, size_t n);
	wstring StringToWString(const char* theStr, size_t pos, size_t n);

	void    StringToWString(const string& src, wstring& dst);
	void    StringToWString(const string& src, size_t n, wstring& dst);
	void    StringToWString(const string& src, size_t pos, size_t n, wstring& dst);

	void    StringToWString(const char* src, wstring& dst);
	void    StringToWString(const char* src, size_t n, wstring& dst);
	void    StringToWString(const char* src, size_t pos, size_t n, wstring& dst);

	string  WStringToString(const wstring& theStr);
	string  WStringToString(const wstring& theStr, size_t n);
	string  WStringToString(const wstring& theStr, size_t pos, size_t n);

	string  WStringToString(const wchar_t* theStr);
	string  WStringToString(const wchar_t* theStr, size_t n);
	string  WStringToString(const wchar_t* theStr, size_t pos, size_t n);

	void    WStringToString(const wstring& src, string& dst);
	void    WStringToString(const wstring& src, size_t n, string& dst);
	void    WStringToString(const wstring& src, size_t pos, size_t n, string& dst);

	void    WStringToString(const wchar_t* theStr, string& dst);
	void    WStringToString(const wchar_t* theStr, size_t n, string& dst);
	void    WStringToString(const wchar_t* theStr, size_t pos, size_t n, string& dst);

	string  WStringToString(const wstring& theStr);
	string  WStringToString(const wchar_t* theStr);

	void    WStringToString(const wstring& src, string& dst);
	void    WStringToString(const wchar_t* src, string& dst);

	bool ConvertNumberStringToRawBytes(const string& theString, RawBuffer& theRawBytesR, unsigned char theBase = 10);
	bool ConvertWideStringToRawBytes(const wstring& theString, RawBuffer& theRawBytesR);
	bool ConvertStringToRawBytes(const string& theString, RawBuffer& theRawBytesR);

	bool ConvertRawBytesToString(const RawBuffer& theRawBytes, string& theStringR);
	bool ConvertRawBytesToWideString(const RawBuffer& theRawBytes, wstring& theStringR);
	
	inline string TimeToString( time_t theTime, bool useLocalTime=false )
	{
		string aTimeStr;
		if( useLocalTime )
    		aTimeStr = asctime( localtime( &theTime ) );
		else
    		aTimeStr = asctime( gmtime( &theTime ) );
		aTimeStr.erase( aTimeStr.size()-1, 1); // remove trailing newline
		return aTimeStr;
	}

    // convert strings or wstrings in place to upper or lower case.
    inline void CvUpper( std::string &theString )
    { std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >( theString.begin(),theString.end(),theString.begin(), toupper ); }
    inline void CvLower( string &theString )
    { std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >( theString.begin(),theString.end(),theString.begin(), tolower );}
    inline void CvUpper( wstring &theWString )
    { std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >( theWString.begin(),theWString.end(),theWString.begin(), towupper ); }
    inline void CvLower( wstring &theWString )
    { std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >( theWString.begin(),theWString.end(),theWString.begin(), towlower ); }

    // convert strings or wstrings to upper or lower case.
    inline string ToUpper( const string &theString )
    {
        string aString( theString );
        std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >( aString.begin(),aString.end(),aString.begin(), toupper );
        return( aString );
    }
    inline string ToLower( const std::string &theString )
    {
        string aString( theString );
        std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >( aString.begin(),aString.end(),aString.begin(), tolower );
        return( aString );
    }

    inline wstring ToUpper( const std::wstring &theWString )
    {
        wstring aWString( theWString );
        std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >( aWString.begin(),aWString.end(),aWString.begin(), towupper );
        return( aWString );
    }
    inline wstring ToLower( const std::wstring &theWString )
    {
        wstring aWString( theWString );
        std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >( aWString.begin(),aWString.end(),aWString.begin(), towlower );
        return( aWString );
    }

    inline __int64 htotll(__int64 ll)
    {
#ifdef WON_BIG_ENDIAN
	long newValue;
	*((char*)&newValue) = *(((char*)&ll)+7);
	*(((char*)&newValue)+1) = *(((char*)&ll)+6);
	*(((char*)&newValue)+2) = *(((char*)&ll)+5);
	*(((char*)&newValue)+3) = *(((char*)&ll)+4);
	*(((char*)&newValue)+4) = *(((char*)&ll)+3);
	*(((char*)&newValue)+5) = *(((char*)&ll)+2);
	*(((char*)&newValue)+6) = *(((char*)&ll)+1);
	*(((char*)&newValue)+7) = *((char*)&ll);
	return newValue;
#else
	return ll;
#endif /* WON_BIG_ENDIAN */
    }

    inline long htotl(long l)
    {
#ifdef WON_BIG_ENDIAN
	long newValue;
	*((char*)&newValue) = *(((char*)&l)+3);
	*(((char*)&newValue)+1) = *(((char*)&l)+2);
	*(((char*)&newValue)+2) = *(((char*)&l)+1);
	*(((char*)&newValue)+3) = *((char*)&l);
	return newValue;
#else
	return l;
#endif /* WON_BIG_ENDIAN */
    }
    
    inline short htots(short s)
    {
#ifdef WON_BIG_ENDIAN
	short newValue;
	*((char*)&newValue) = *(((char*)&s)+1);
	*(((char*)&newValue)+1) = *(((char*)&s));
	return newValue;
#else
	return s;
#endif /* WON_BIG_ENDIAN */
    }

    inline void htotUnicodeString(const wchar_t* theSrcP, wchar* theDestP, unsigned int numChars)
    {
#if defined(WIN32) || (defined(macintosh) && (macintosh == 1))
# ifdef WON_BIG_ENDIAN
	HostToTitanBuf(theSrcP, theDestP, numChars * sizeof(wchar), sizeof(wchar));
# else
	memcpy(theDestP, theSrcP, numChars * sizeof(wchar));
# endif /* WON_BIG_ENDIAN */
#elif _LINUX
	for (int i = 0; i < numChars; ++i)
		theDestP[i] = theSrcP[i];
#endif /* platform switch */
    }

    inline void HostToTitanBuf(const void* theBufP, void* theDestP, unsigned int theBufLen, unsigned int theSectionLen)
    {
#ifdef WON_BIG_ENDIAN
	unsigned int aHalfSectionLen = theSectionLen / 2;
	for (int iBuf = 0, aSecNum = 0; iBuf < theSrcLen; ++aSecNum)
	{
		unsigned int aNextSectionOffset = (aSecNum + 1) * theSectionLen;
		for (int iSec = 0; iSec < aHalfSectionLen; ++iSec, ++iBuf)
		{
			unsigned int aOffset1 = iBuf;
			unsigned int aOffset2 = aNextSectionOffset - iSec - 1;
			*(((unsigned char*)theDestP) + aOffset1) = *(((unsigned char*)theSrcP) + aOffset2);
			*(((unsigned char*)theDestP) + aOffset2) = *(((unsigned char*)theSrcP) + aOffset1);
		}
		iBuf = aNextSectionOffset;
	}
#endif // WON_BIG_ENDIAN
    }

    inline __int64 ttohll(__int64 ll)
    {
#ifdef WON_BIG_ENDIAN
	return htotll(ll);
#else
	return ll;
#endif /* WON_BIG_ENDIAN */
    }

    inline long ttohl(long l)
    {
#ifdef WON_BIG_ENDIAN
	return htotl(l);
#else
	return l;
#endif /* WON_BIG_ENDIAN */
    }

    inline short ttohs(short s)
    {
#ifdef WON_BIG_ENDIAN
	return htots(s);
#else
	return s;
#endif /* WON_BIG_ENDIAN */
    }

    inline void ttohUnicodeString(const wchar* theSrcP, wchar_t* theDestP, unsigned int numChars)
    {
#if defined(WIN32) || (defined(macintosh) && (macintosh == 1))
# ifdef WON_BIG_ENDIAN
	TitanToHostBuf(theSrcP, theDestP, numChars * sizeof(wchar), sizeof(wchar));
# else
	memcpy(theDestP, theSrcP, numChars * sizeof(wchar));
# endif /* WON_BIG_ENDIAN */
#elif _LINUX
	for (int i = 0; i < numChars; ++i)
		theDestP[i] = theSrcP[i];
#endif /* platform switch */
    }

    inline void TitanToHostBuf(const void* theSrcP, void* theDestP, unsigned int theSrcLen, unsigned int theSectionLen)
    { HostToTitanBuf(theSrcP, theDestP, theSrcLen, theSectionLen); }
};

// Output operators
inline ostream&
operator<<(ostream& os, const wstring& aStr)
{ os << WONCommon::WStringToString(aStr);  return os; }

inline ostream&
operator<<(ostream& os, const wchar_t* aBuf)
{ os << wstring(aBuf);  return os; }

#ifdef WIN32

inline ostream&
operator<<(ostream& os, __int64 theValue)
{
	char s[21];
	
	// *NOTE*
	// Uncomment the following if block for VC5

	// There's a bug in VC5's _i64toa.  It's not able to convert negative numbers
	// correctly.  Get around this by outputting our own negative sign and then
	// the "positive version" of the number.
	//if (theValue < 0)
		//os << "-" << _i64toa(-theValue, s, 16);
	//else
		os << _i64toa(theValue, s, 16);
	return os;
}

#endif

#endif
