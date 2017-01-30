#ifndef __WON_STRING_H__
#define __WON_STRING_H__

// Turn off annoying STL generated warnings.
#pragma warning(disable : 4710) // Inline function not expanded
#pragma warning(disable : 4100) // Unreferenced formal parameter.
#pragma warning(disable : 4514) // Unreferenced inline function removed

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(disable : 4275) // Non-dll class base for dll class.
#pragma warning(push, 3) // Diasbale STL warnings
#endif // WARNING_LVL_4_AND_STL

#include <string>

#ifdef WARNING_LVL_4_AND_STL
#pragma warning(pop)     // Restore warnings.
#endif

#ifndef _WONDEFS_H
#include "won.h"
#endif



namespace WONCommon {


// WONString encapsulates either an ASCII or UNICODE string, or both.
//
// WONString could be used as a replacement for std::string, in existing code
// that needs to be ported to support wide strings.  After a global find-and-replace,
// the code should compile and (hopefully) function.  (Assuming we haven't left out
// any member functions).
//
// WONString can also be handy to use as an argument to a function that could
// receive either type of string, rather than defining separate ASCII and UNICODE
// functions.  i.e.
//
//		void foo(const WONString& str);
//
// Because non-explicit (copy) constructors accept either ASCII or UNICODE string
// types, any of those types could be passed directly to that function.  i.e.
//
//		char* asciiStr1;
//		std::string asciiStr2;
//		wchar* unicodeStr1;	// wchar_t -> wchar for linux support
//		std::wstring unicodeStr2;
//		...
//		foo(asciiStr1);
//		foo(asciiStr2);
//		foo(unicodeStr1);
//		foo(unicodeStr2);
//
// The encapsulated string will be automatically translated from ASCII to UNICODE or
// vice versa, only when necessary.
//
// Although accessor members are available to modify the string, they could result in
// unnecessary string conversions.  It would be more efficient to treat a WONString
// as immutable (like a Java string), and always replace its value entirely instead
// of modifying it in place.
//
// Also, it's possible that the ASCII and UNICODE strings may have a different lengths,
// if the ASCII string is used to encapsulate an ANSI multi-byte string.  (???)
// 
class WONString
{
private:
	mutable std::wstring wstr;
	mutable std::string str;
	mutable std::basic_string<wchar> titanwstr;
	mutable char flags;	// bit 0 = converted, bit 1 = initedWide

	enum { converted = 0x01, initedWide = 0x02 };

	// flags by value
	// 0  = not initedWide, not converted
	//  1 = converted , not initedWide
	// 2  = initedWide, not converted
	//  3 = both initedWide and converted

	void MakeWide() const
	{
		if (!flags)							// !initedWide && !converted
		{
			flags = converted;				// !initedWide && converted
			WONCommon::StringToWString(str, wstr);
		}
	}

	void MakeTitanWide() const
	{
		MakeWide();
#ifdef WIN32
		titanwstr.assign(wstr);
#else
		unsigned int aNumChars = wstr.size();
		titanwstr.resize(aNumChars);
		for (int i = 0; i < aNumChars; ++i) 
			titanwstr[i] = htots(wstr[i]);
#endif // WIN32
	}

	void MakeAscii() const
	{
		if (flags == initedWide)			// initedWide && !converted
		{
			flags = converted | initedWide;	// initedWide && converted
			WONCommon::WStringToString(wstr, str);
		}
	}

	bool isConverted() const		{ return (flags & converted) != 0; }

#ifdef _LINUX
	// Compensates for differences between WIN32 and Linux basic_string<>

	mutable std::wstring wstrWithNULL;
	mutable std::basic_string<wchar> titanwstrWithNULL;
	const wchar_t* wstr_c_str() const					{ wstrWithNULL = wstr; wstrWithNULL.append(1, L'\0'); return wstrWithNULL.data(); }
	const wchar* titanwstr_c_str() const					{ titanwstrWithNULL = titanwstr; titanwstrWithNULL.append(1, (wchar)0); return titanwstrWithNULL.data(); }
	int wstr_compare(size_t pos, size_t n, const wchar_t* s) const		{ return wstr.substr(pos, n).compare(s); }
	int wstr_compare(size_t pos, size_t n, const std::wstring& s) const	{ return wstr.substr(pos, n).compare(s); }
	int str_compare(size_t pos, size_t n, const char* s) const			{ return str.substr(pos, n).compare(s); }
	int str_compare(size_t pos, size_t n, const std::string& s) const	{ return str.substr(pos, n).compare(s); }
	int wstr_compare(const wchar_t* s, size_t pos, size_t n) const		{ return wstr.compare(s, pos, n); }
	int wstr_compare(const std::wstring& s, size_t pos, size_t n) const	{ return wstr.compare(s, pos, n); }
	int str_compare(const char* s, size_t pos, size_t n) const			{ return str.compare(s, pos, n); }
	int str_compare(const std::string& s, size_t pos, size_t n) const	{ return str.compare(s, pos, n); }
	int wstr_compare(size_t pos0, size_t n0, const wchar_t* s, size_t pos, size_t n) const		{ return wstr.substr(pos0, n0).compare(s, pos, n); }
	int wstr_compare(size_t pos0, size_t n0, const std::wstring& s, size_t pos, size_t n) const	{ return wstr.substr(pos0, n0).compare(s, pos, n); }
	int str_compare(size_t pos0, size_t n0, const char* s, size_t pos, size_t n) const			{ return str.substr(pos0, n0).compare(s, pos, n); }
	int str_compare(size_t pos0, size_t n0, const std::string& s, size_t pos, size_t n) const	{ return str.substr(pos0, n0).compare(s, pos, n); }

#else

	const wchar_t* wstr_c_str() const										{ return wstr.c_str(); }
	const wchar* titanwstr_c_str() const					{ return titanwstr.c_str(); }
	int wstr_compare(size_t pos, size_t n, const wchar* s) const		{ return wstr.compare(pos, n, s); }
	int wstr_compare(size_t pos, size_t n, const std::wstring& s) const	{ return wstr.compare(pos, n, s); }
	int str_compare(size_t pos, size_t n, const char* s) const			{ return str.compare(pos, n, s); }
	int str_compare(size_t pos, size_t n, const std::string& s) const	{ return str.compare(pos, n, s); }
	int wstr_compare(const wchar* s, size_t pos, size_t n) const		{ return wstr.compare(0, 0xFFFFFFFF, s, pos, n); }
	int wstr_compare(const std::wstring& s, size_t pos, size_t n) const	{ return wstr.compare(0, 0xFFFFFFFF, s, pos, n); }
	int str_compare(const char* s, size_t pos, size_t n) const			{ return str.compare(0, 0xFFFFFFFF, s, pos, n); }
	int str_compare(const std::string& s, size_t pos, size_t n) const	{ return str.compare(0, 0xFFFFFFFF, s, pos, n); }
	int wstr_compare(size_t pos0, size_t n0, const wchar* s, size_t pos, size_t n) const			{ return wstr.compare(pos0, n0, s, pos, n); }
	int wstr_compare(size_t pos0, size_t n0, const std::wstring& s, size_t pos, size_t n) const	{ return wstr.compare(pos0, n0, s, pos, n); }
	int str_compare(size_t pos0, size_t n0, const char*& s, size_t pos, size_t n) const	{ return str.compare(pos0, n0, s, pos, n); }
	int str_compare(size_t pos0, size_t n0, const std::string& s, size_t pos, size_t n) const	{ return str.compare(pos0, n0, s, pos, n); }

#endif

public:
	WONString()
		:	flags(0)
	{ }

	WONString(const std::string& s)
		:	str(s), flags(0)
	{ }

	WONString(const char* s)
		:	str(s ? s : ""), flags(0)
	{ }

	WONString(const std::wstring& s)
		:	wstr(s), flags(initedWide)
	{ }

	WONString(const wchar_t* s)
		:	wstr(s ? s : L""), flags(initedWide)
	{ }

#ifdef _LINUX
	WONString(const wchar* s)
		:	flags(initedWide)
	{
		unsigned int aNumChar = 0;
		while (TRUE)
		{
			if (s[aNumChar] == 0)
				break;
			else
				++aNumChar;
		}
		InitFromWchar(s, aNumChar);
	}

	void InitFromWchar(const wchar* s, unsigned int n)
	{
		wstr.resize(n); 
		for (int i = 0; i < n; ++i) 
			wstr[i] = ttohs(s[i]);
	}
#endif // _LINUX

	WONString(const WONString& toCopy)
		:	str(toCopy.str), wstr(toCopy.wstr), flags(toCopy.flags)
	{ }

	WONString(const WONString& toCopy, size_t pos, size_t n)
		:	flags(toCopy.flags)
	{
		if (flags != 0)
			wstr.assign(toCopy.wstr, pos, n);
		if (flags != 1)
			str.assign(toCopy.str, pos, n);
		if (empty())
			flags = 0;
	}

	WONString(const std::string& s, size_t pos, size_t n)
		:	str(s, pos, n), flags(0)
	{ }
	
	WONString(const std::wstring& s, size_t pos, size_t n)
		:	wstr(s, pos, n), flags(initedWide)
	{ }
	
	WONString(const char* s, size_t n)
		:	str(s ? s : "", n), flags(0)
	{ }

	WONString(const wchar_t* s, size_t n)
		:	wstr(s ? s : L"", n), flags(initedWide)
	{ }

#ifdef _LINUX
	WONString(const wchar* s, size_t n)
		:	flags(initedWide)
	{ InitFromWchar(s, n); }
#endif // _LINUX

	WONString(size_t n, char c)
		:	str(n, c), flags(0)
	{ }

	WONString(size_t n, wchar_t c)
		:	wstr(n, c), flags(initedWide)
	{ }

	WONString(std::string::const_iterator first, std::string::const_iterator last)
		:	str(first, last), flags(0)
	{ }

	WONString(std::wstring::const_iterator first, std::wstring::const_iterator last)
		:	wstr(first, last), flags(0)
	{ }

	operator const std::string&() const		{ MakeAscii(); return str; }
	operator const std::wstring&() const	{ MakeWide(); return wstr; }
	operator const char*() const			{ MakeAscii(); return str.c_str(); }
	operator const wchar_t*() const			{ MakeWide(); return wstr_c_str(); }
	
	const std::string&		GetAsciiString() const		{ MakeAscii(); return str; }
	const char*			GetAsciiCString() const		{ MakeAscii(); return str.c_str(); }
	const std::wstring&		GetUnicodeString() const	{ MakeWide(); return wstr; }
	const wchar_t*			GetUnicodeCString() const	{ MakeWide(); return wstr_c_str(); }
	const std::wstring&		GetWideString() const		{ MakeWide(); return wstr; }
	const wchar_t*			GetWideCString() const		{ MakeWide(); return wstr_c_str(); }
	const std::basic_string<wchar>& GetTitanUnicodeString() const   { MakeTitanWide(); return titanwstr; } 
	const wchar*			GetTitanUnicodeCString() const  { MakeTitanWide(); return titanwstr_c_str(); }

	bool isWide() const			{ return (flags & initedWide) != 0; }

	size_t length() const		{ return isWide() ? wstr.length() : str.length(); }
	size_t size() const			{ return isWide() ? wstr.size() : str.size(); }
	bool empty() const			{ return isWide() ? wstr.empty() : str.empty(); }

	void erase()
	{
		str.erase();
		wstr.erase();
		flags = 0;
	}

	WONString& erase(size_t pos, size_t n = (size_t)-1)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			str.erase(pos, n);
			break;
		//case 2:	// !initedWide && converted
		//case 3:	// initedWide && converted
		default:
			str.erase(pos, n);
		case 1:		// initedWide && !converted
			wstr.erase(pos, n);
		}
		if (empty())
			flags = 0;
		return *this;
	}
	
	WONString& assign(const WONString& toCopy)
	{
		str.assign(toCopy.str);
		wstr.assign(toCopy.wstr);
		flags = toCopy.flags;
		return *this;
	}

	WONString& assign(const WONString& toCopy, size_t n)
	{
		flags = toCopy.flags;
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			wstr.erase();
			str.assign(toCopy.str, 0, n);
			break;
		case 1:		// initedWide && !converted
			str.erase();
			wstr.assign(toCopy.wstr, 0, n);
			break;
		default:
		//case 2:		// !initedWide && converted
		//case 3:		// initedWide && converted
			str.assign(toCopy.str, 0, n);
			wstr.assign(toCopy.wstr, 0, n);
		}
		if (empty())
			flags = 0;
		return *this;
	}


	WONString& assign(const WONString& toCopy, size_t pos, size_t n)
	{
		flags = toCopy.flags;
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			wstr.erase();
			str.assign(toCopy.str, pos, n);
			break;
		case 1:		// initedWide && !converted
			str.erase();
			wstr.assign(toCopy.wstr, pos, n);
			break;
		default:
		//case 2:		// !initedWide && converted
		//case 3:		// initedWide && converted
			str.assign(toCopy.str, pos, n);
			wstr.assign(toCopy.wstr, pos, n);
		}
		if (empty())
			flags = 0;
		return *this;
	}

	WONString& assign(const std::string& toCopy, size_t n)
	{
		flags = 0;				// !initedWide && !converted
		wstr.erase();
		if (n)
			str.assign(toCopy, 0, n);
		else
			str.erase();
		return *this;
	}

	WONString& assign(const std::string& toCopy, size_t pos, size_t n)	
	{
		flags = 0;				// !initedWide && !converted
		wstr.erase();
		if (n)
			str.assign(toCopy, pos, n);
		else
			str.erase();
		return *this;
	}
	
	WONString& assign(const char* toCopy, size_t n)	
	{
		flags = 0;				// !initedWide && !converted
		wstr.erase();
		if (toCopy && n)
			str.assign(toCopy, 0, n);
		else
			str.erase();
		return *this;
	}
	
	WONString& assign(const char* toCopy, size_t pos, size_t n)	
	{
		flags = 0;				// !initedWide && !converted
		wstr.erase();
		if (toCopy && n)
			str.assign(toCopy, pos, n);
		else
			str.erase();
		return *this;
	}

	WONString& assign(const std::wstring& toCopy, size_t n)
	{
		flags = 0;
		str.erase();
		if (n)
		{
			wstr.assign(toCopy, 0, n);
			if (!wstr.empty())
				flags = (char)initedWide;
		}
		else
			wstr.erase();
		return *this;
	}

	WONString& assign(const std::wstring& toCopy, size_t pos, size_t n)	
	{
		flags = 0;
		str.erase();
		if (n)
		{
			wstr.assign(toCopy, pos, n);
			if (!wstr.empty())
				flags = (char)initedWide;
		}
		else
			wstr.erase();
		return *this;
	}
	
	WONString& assign(const wchar_t* toCopy, size_t n)	
	{
		flags = 0;
		str.erase();
		if (toCopy && n)
		{
			wstr.assign(toCopy, 0, n);
			if (!wstr.empty())
				flags = (char)initedWide;
		}
		else
			wstr.erase();
		return *this;
	}

	WONString& assign(const wchar_t* toCopy, size_t pos, size_t n)	
	{
		flags = 0;
		str.erase();
		if (toCopy && n)
		{
			wstr.assign(toCopy, pos, n);
			if (!wstr.empty())
				flags = (char)initedWide;
		}
		else
			wstr.erase();
		return *this;
	}

	WONString& assign(size_t n, char c)
	{
		flags = 0;
		wstr.erase();
		str.assign(n, c);
		return *this;
	}

	WONString& assign(size_t n, wchar c)
	{
		str.erase();
		wstr.assign(n, c);
		flags = wstr.empty() ? (char)0 : (char)initedWide;
		return *this;
	}

	WONString& assign(std::string::const_iterator first, std::string::const_iterator last)
	{
		flags = 0;
		wstr.erase();
		str.assign(first, last);
		return *this;
	}

	WONString& assign(std::wstring::const_iterator first, std::wstring::const_iterator last)
	{
		str.erase();
		wstr.assign(first, last);
		flags = wstr.empty() ? (char)0 : (char)initedWide;
		return *this;
	}

	WONString& operator=(const WONString& toCopy)		{ return assign(toCopy); }
	WONString& operator=(const char* toCopy)			{ return assign(toCopy); }
	WONString& operator=(const wchar_t* toCopy)			{ return assign(toCopy); }
	WONString& operator=(const std::string& toCopy)		{ return assign(toCopy); }
	WONString& operator=(const std::wstring& toCopy)	{ return assign(toCopy); }

	WONString& append(const WONString& toAdd)
	{
		if (toAdd.isWide())
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:		// !initedWide && converted
				flags = initedWide | converted;
			//case 3:		// initedWide && converted
			default:
				toAdd.MakeAscii();
				str.append(toAdd.str);
			case 1:		// initedWide && !converted
				wstr.append(toAdd.wstr);
			}
		}
		else
		{
			switch (flags)
			{
			case 1:		// initedWide && !converted
				WStringToString(wstr, str);
			//case 2:		// !initedWide && converted
			//case 3:		// initedWide && converted
			default:
				toAdd.MakeWide();
				wstr.append(toAdd.wstr);
			case 0:		// !initedWide && !converted
				str.append(toAdd.str);
			}
		}
		return *this;
	}

	WONString& append(const WONString& toAdd, size_t n)
	{
		if (n)
		{
			if (toAdd.isWide())
			{
				switch (flags)
				{
				case 0:		// !initedWide && !converted
					StringToWString(str, wstr);
				case 2:		// !initedWide && converted
					flags = initedWide | converted;
				//case 3:		// initedWide && converted
				default:
					toAdd.MakeAscii();
					str.append(toAdd.str, 0, n);
				case 1:		// initedWide && !converted
					wstr.append(toAdd.wstr, 0, n);
				}
			}
			else
			{
				switch (flags)
				{
				case 1:		// initedWide && !converted
					WStringToString(wstr, str);
				//case 2:		// !initedWide && converted
				//case 3:		// initedWide && converted
				default:
					toAdd.MakeWide();
					wstr.append(toAdd.wstr, 0, n);
				case 0:		// !initedWide && !converted
					str.append(toAdd.str, 0, n);
				}
			}
		}
		return *this;
	}

	WONString& append(const WONString& toAdd, size_t pos, size_t n)
	{
		if (n)
		{
			if (toAdd.isWide())
			{
				switch (flags)
				{
				case 0:		// !initedWide && !converted
					StringToWString(str, wstr);
				case 2:		// !initedWide && converted
					flags = initedWide | converted;
				//case 3:		// initedWide && converted
				default:
					toAdd.MakeAscii();
					str.append(toAdd.str, pos, n);
				case 1:		// initedWide && !converted
					wstr.append(toAdd.wstr, pos, n);
				}
			}
			else
			{
				switch (flags)
				{
				case 1:		// initedWide && !converted
					WStringToString(wstr, str);
				//case 2:		// !initedWide && converted
				//case 3:		// initedWide && converted
				default:
					toAdd.MakeWide();
					wstr.append(toAdd.wstr, pos, n);
				case 0:		// !initedWide && !converted
					str.append(toAdd.str, pos, n);
				}
			}
		}
		return *this;
	}


	WONString& append(const std::string& s, size_t n)
	{
		if (n)
		{
			if (flags != initedWide)	// !(initedWide && !converted) === !initedWide || converted
			{
				str.append(s, 0, n);
				if (flags == 0)			// !initedWide && !converted
					return *this;
			}
			wstring tmp;
			StringToWString(s, n, tmp);
			wstr.append(tmp);
		}
		return *this;
	}

	WONString& append(const std::string& s, size_t pos, size_t n)
	{
		if (n)
		{
			if (flags != initedWide)	// !(initedWide && !converted) === !initedWide || converted
			{
				str.append(s, pos, n);
				if (flags == 0)			// !initedWide && !converted
					return *this;
			}
			wstring tmp;
			StringToWString(s, pos, n, tmp);
			wstr.append(tmp);
		}
		return *this;
	}

	WONString& append(const char* s, size_t n)	
	{
		if (s && *s && n)
		{
			if (flags != initedWide)	// !(initedWide && !converted) === !initedWide || converted
			{
				str.append(s, 0, n);
				if (flags == 0)			// !initedWide && !converted
					return *this;
			}
			wstring tmp;
			StringToWString(s, n, tmp);
			wstr.append(tmp);
		}
		return *this;
	}

	WONString& append(const char* s, size_t pos, size_t n)	
	{
		if (s && *s && n)
		{
			if (flags != initedWide)	// !(initedWide && !converted) === !initedWide || converted
			{
				str.append(s, pos, n);
				if (flags == 0)			// !initedWide && !converted
					return *this;
			}
			wstring tmp;
			StringToWString(s, pos, n, tmp);
			wstr.append(tmp);
		}
		return *this;
	}

	WONString& append(const std::wstring& s, size_t n)	
	{
		if (n)
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:		// !initedWide && converted
				flags = initedWide | converted;
			//case 3:		// initedWide && converted
			default:
				{
					string tmp;
					WStringToString(s, 0, n);
					str.append(tmp);
				}
			case 1:		// initedWide && !converted
				wstr.append(s, 0, n);
			}
		}
		return *this;
	}

	WONString& append(const std::wstring& s, size_t pos, size_t n)	
	{
		if (n)
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:		// !initedWide && converted
				flags = initedWide | converted;
			//case 3:		// initedWide && converted
			default:
				{
					string tmp;
					WStringToString(s, pos, n);
					str.append(tmp);
				}
			case 1:		// initedWide && !converted
				wstr.append(s, pos, n);
			}
		}
		return *this;
	}

	WONString& append(const wchar_t* s, size_t n)	
	{
		if (s && *s && n)
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:		// !initedWide && converted
				flags = initedWide | converted;
			//case 3:		// initedWide && converted
			default:
				{
					string tmp;
					WStringToString(s, 0, n);
					str.append(tmp);
				}
			case 1:		// initedWide && !converted
				wstr.append(s, 0, n);
			}
		}
		return *this;
	}

	WONString& append(const wchar_t* s, size_t pos, size_t n)	
	{
		if (s && *s && n)
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:		// !initedWide && converted
				flags = initedWide | converted;
			//case 3:		// initedWide && converted
			default:
				{
					string tmp;
					WStringToString(s, pos, n);
					str.append(tmp);
				}
			case 1:		// initedWide && !converted
				wstr.append(s, pos, n);
			}
		}
		return *this;
	}

	WONString& append(size_t n, wchar c)
	{
		if (n)
		{
			switch (flags)
			{
			case 0:		// !initedWide && !converted
				StringToWString(str, wstr);

			case 2:		// !initedWide && converted
				flags = initedWide | converted;
				// Convert and add to ascii string
				{
					wstring src(n, c);
					string tmp;
					WStringToString(src, tmp);
					str.append(tmp);
				}
			//case 1:		//  initedWide && !converted
			//case 3:		//  initedWide && converted
			default:
				wstr.append(n, c);
			}
		}
		return *this;
	}

	WONString& append(size_t n, char c)
	{
		if (n)
		{
			if (flags != 0)		// !initedWide && !converted
			{
				wstr.append(n, (wchar)c);
				if (flags == 1)	// if (flags != (initedWide | !converted))
					return *this;
			}
			str.append(n, c);
		}
		return *this;
	}


	WONString& append(std::wstring::const_iterator first, std::wstring::const_iterator last)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:		//  initedWide && converted
		default:
			{
				wstring toAdd(first, last);
				string tmp;
				WStringToString(toAdd, tmp);
				str.append(tmp);
			}
		case 1:		//  initedWide && !converted
			wstr.append(first, last);
		}
		return *this;
	}

	WONString& append(std::string::const_iterator first, std::string::const_iterator last)
	{
		if (flags != 0)
		{
			string toAdd(first, last);
			wstring tmp;
			StringToWString(toAdd, tmp);
			wstr.append(tmp);
			if (flags == 1)
				return *this;
		}
		str.append(first, last);
		return *this;
	}

	WONString& operator+=(const WONString& toAdd)	{ return append(toAdd); }
	WONString& operator+=(const std::string& toAdd)	{ return append(toAdd); }
	WONString& operator+=(const std::wstring& toAdd){ return append(toAdd); }
	WONString& operator+=(const char* toAdd)		{ return append(toAdd); }
	WONString& operator+=(const wchar_t* toAdd)		{ return append(toAdd); }

	WONString operator+(const WONString& toAdd) const	{ return (WONString(*this) += toAdd); }
	WONString operator+(const std::string& toAdd) const	{ return (WONString(*this) += toAdd); }
	WONString operator+(const std::wstring& toAdd) const{ return (WONString(*this) += toAdd); }
	WONString operator+(const char* toAdd) const		{ return (WONString(*this) += toAdd); }
	WONString operator+(const wchar_t* toAdd) const		{ return (WONString(*this) += toAdd); }

	friend WONString operator+(const std::string& addTo, const WONString& toAdd)
	{
		return (WONString(addTo) += toAdd);
	}

	friend WONString operator+(const std::wstring& addTo, const WONString& toAdd)
	{
		return (WONString(addTo) += toAdd);
	}

	friend WONString operator+(const char* addTo, const WONString& toAdd)
	{
		return (WONString(addTo) += toAdd);
	}

	friend WONString operator+(const wchar_t* addTo, const WONString& toAdd)
	{
		return (WONString(addTo) += toAdd);
	}

	int compare(const WONString& compareTo) const
	{
		if (isWide())
			compareTo.MakeWide();
		else
		{
			if (!compareTo.isWide())
				return str.compare(compareTo.str);
			MakeWide();
		}
		return wstr.compare(compareTo.wstr);
	}

	int compare(const WONString& compareTo, size_t pos, size_t n = (size_t)-1) const
	{
		if (isWide())
			compareTo.MakeWide();
		else
		{
			if (!compareTo.isWide())
				return str_compare(compareTo.str, pos, n);
			MakeWide();
		}
		return wstr_compare(compareTo.wstr, pos, n);
	}

	int compare(size_t pos0, size_t n0, const WONString& compareTo)
	{
		if (isWide())
			compareTo.MakeWide();
		else
		{
			if (!compareTo.isWide())
				return str_compare(pos0, n0, compareTo.str);
			MakeWide();
		}
		return wstr_compare(pos0, n0, compareTo.wstr);
	}

	int compare(size_t pos0, size_t n0, const WONString& compareTo, size_t pos, size_t n)
	{
		if (isWide())
			compareTo.MakeWide();
		else
		{
			if (!compareTo.isWide())
				return str_compare(pos0, n0, compareTo.str, pos, n);
			MakeWide();
		}
		return wstr_compare(pos0, n0, compareTo.wstr, pos, n);
	}

	int compare(const std::string& compareTo) const
	{
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, tmp);
			return wstr.compare(tmp);
		}
		return str.compare(compareTo);
	}

	int compare(size_t pos0, size_t n0, const std::string& compareTo)
	{
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, tmp);
			return wstr_compare(pos0, n0, tmp);
		}
		return str_compare(pos0, n0, compareTo);
	}

	int compare(size_t pos0, size_t n0, const std::string& compareTo, size_t pos, size_t n)
	{
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, pos, n, tmp);
			return wstr_compare(pos0, n0, tmp);
		}
		return str_compare(pos0, n0, compareTo);
	}

	int compare(const char* compareTo) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, tmp);
			return wstr.compare(tmp);
		}
		return str.compare(compareTo);
	}

	int compare(const char* compareTo, size_t pos, size_t n = (size_t)-1) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, tmp);
			return wstr_compare(tmp, pos, n);
		}
		return str_compare(compareTo, pos, n);
	}

	int compare(size_t pos0, size_t n0, const char* compareTo) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, tmp);
			return wstr_compare(pos0, n0, tmp);
		}
		return str_compare(pos0, n0, compareTo);
	}

	int compare(size_t pos0, size_t n0, const char* compareTo, size_t pos, size_t n) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		if (isWide())
		{
			wstring tmp;
			StringToWString(compareTo, pos, n, tmp);
			return wstr_compare(pos0, n0, tmp);
		}
		return str_compare(pos0, n0, compareTo);
	}
	
	int compare(const std::wstring& compareTo) const
	{
		MakeWide();
		return wstr.compare(compareTo);
	}

	int compare(size_t pos0, size_t n0, const std::wstring& compareTo)
	{
		MakeWide();
		return wstr_compare(pos0, n0, compareTo);
	}

	int compare(size_t pos0, size_t n0, const std::wstring& compareTo, size_t pos, size_t n)
	{
		MakeWide();
		return wstr_compare(pos0, n0, compareTo, pos, n);
	}

	int compare(const wchar_t* compareTo) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		MakeWide();
		return wstr.compare(compareTo);
	}

	int compare(const wchar_t* compareTo, size_t pos, size_t n = (size_t)-1) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		MakeWide();
		return wstr_compare(compareTo, pos, n);
	}

	int compare(size_t pos0, size_t n0, const wchar_t* compareTo) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		MakeWide();
		return wstr_compare(pos0, n0, compareTo);
	}

	int compare(size_t pos0, size_t n0, const wchar_t* compareTo, size_t pos, size_t n) const
	{
		if (!compareTo)
			return empty() ? 0 : 1;
		MakeWide();
		return wstr_compare(pos0, n0, compareTo, pos, n);
	}

	
	bool operator==(const WONString& compareTo) const		{ return compare(compareTo) == 0; }
	bool operator==(const std::string& compareTo) const		{ return compare(compareTo) == 0; }
	bool operator==(const std::wstring& compareTo) const	{ return compare(compareTo) == 0; }
	bool operator==(const char* compareTo) const			{ return compare(compareTo) == 0; }
	bool operator==(const wchar_t* compareTo) const			{ return compare(compareTo) == 0; }

	bool operator!=(const WONString& compareTo) const		{ return compare(compareTo) != 0; }
	bool operator!=(const std::string& compareTo) const		{ return compare(compareTo) != 0; }
	bool operator!=(const std::wstring& compareTo) const	{ return compare(compareTo) != 0; }
	bool operator!=(const char* compareTo) const			{ return compare(compareTo) != 0; }
	bool operator!=(const wchar_t* compareTo) const			{ return compare(compareTo) != 0; }

	bool operator>(const WONString& compareTo) const		{ return compare(compareTo) > 0; }
	bool operator>(const std::string& compareTo) const		{ return compare(compareTo) > 0; }
	bool operator>(const std::wstring& compareTo) const		{ return compare(compareTo) > 0; }
	bool operator>(const char* compareTo) const				{ return compare(compareTo) > 0; }
	bool operator>(const wchar_t* compareTo) const			{ return compare(compareTo) > 0; }
	
	bool operator>=(const WONString& compareTo) const		{ return compare(compareTo) >= 0; }
	bool operator>=(const std::string& compareTo) const		{ return compare(compareTo) >= 0; }
	bool operator>=(const std::wstring& compareTo) const	{ return compare(compareTo) >= 0; }
	bool operator>=(const char* compareTo) const			{ return compare(compareTo) >= 0; }
	bool operator>=(const wchar_t* compareTo) const			{ return compare(compareTo) >= 0; }

	bool operator<(const WONString& compareTo) const		{ return compare(compareTo) < 0; }
	bool operator<(const std::string& compareTo) const		{ return compare(compareTo) < 0; }
	bool operator<(const std::wstring& compareTo) const		{ return compare(compareTo) < 0; }
	bool operator<(const char* compareTo) const				{ return compare(compareTo) < 0; }
	bool operator<(const wchar_t* compareTo) const			{ return compare(compareTo) < 0; }
	
	bool operator<=(const WONString& compareTo) const		{ return compare(compareTo) <= 0; }
	bool operator<=(const std::string& compareTo) const		{ return compare(compareTo) <= 0; }
	bool operator<=(const std::wstring& compareTo) const	{ return compare(compareTo) <= 0; }
	bool operator<=(const char* compareTo) const			{ return compare(compareTo) <= 0; }
	bool operator<=(const wchar_t* compareTo) const			{ return compare(compareTo) <= 0; }

	void swap(WONString& swapWith)
	{
		str.swap(swapWith.str);
		wstr.swap(swapWith.wstr);
		char savedFlags = flags;
		flags = swapWith.flags;
		swapWith.flags = savedFlags;
	}

	void swap(std::string& swapWith)
	{
		switch (flags)
		{
		case 1:		//  initedWide && !converted
			WStringToString(wstr, str);
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			flags = 0;
			wstr.erase();
		case 0:		// !initedWide && !converted
			str.swap(swapWith);
		}
	}

	void swap(std::wstring& swapWith)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.erase();
			flags = initedWide;
		case 1:		//  initedWide && !converted
			wstr.swap(swapWith);
		}
	}

	wchar operator[](int i)
	{
		if (isWide())
			return wstr[i];
		return (wchar)(str[i]);
	}

	WONString& insert(size_t p0, const WONString& toInsert)
	{
		if (toInsert.isWide())
		{
			switch (flags)
			{
			case 0:	// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:	// !initedWide && converted
				flags = initedWide | converted;			
			default:
				break;
			}
		}

		switch (flags)
		{
		//case 0:		// !initedWide && !converted
		//case 2:		// !initedWide && converted
		//case 3:		//  initedWide && converted
		default:
			toInsert.MakeAscii();
			str.insert(p0, toInsert.str);
		case 1:		//  initedWide && !converted
			toInsert.MakeWide();
			wstr.insert(p0, toInsert.wstr);
		}
		return *this;
	}

	WONString& insert(size_t p0, const std::string& toInsert)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.insert(p0, toInsert);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			wstring tmp;
			StringToWString(toInsert, tmp);
			wstr.insert(p0, tmp);
		}
		return *this;
	}

	WONString& insert(size_t p0, const std::wstring& toInsert)
	{
		switch (flags)
		{
		case 0:	// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:	// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(toInsert, tmp);
				str.insert(p0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.insert(p0, toInsert);
		}
		return *this;
	}

	WONString& insert(size_t p0, const char* toInsert)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.insert(p0, toInsert);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			wstring tmp;
			StringToWString(toInsert, tmp);
			wstr.insert(p0, tmp);
		}
		return *this;
	}

	WONString& insert(size_t p0, const wchar_t* toInsert)
	{
		switch (flags)
		{
		case 0:	// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:	// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(toInsert, tmp);
				str.insert(p0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.insert(p0, toInsert);
		}
		return *this;
	}

	WONString& insert(size_t p0, const char* toInsert, size_t n)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.insert(p0, toInsert, n);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			wstring tmp;
			StringToWString(toInsert, n, tmp);
			wstr.insert(p0, tmp);
		}
		return *this;
	}

	WONString& insert(size_t p0, const wchar_t* toInsert, size_t n)
	{
		switch (flags)
		{
		case 0:	// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:	// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(toInsert, n, tmp);
				str.insert(p0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.insert(p0, toInsert, n);
		}
		return *this;
	}

	WONString& insert(size_t p0, const WONString& toInsert, size_t pos, size_t n)
	{
		if (toInsert.isWide())
		{
			switch (flags)
			{
			case 0:	// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:	// !initedWide && converted
				flags = initedWide | converted;			
			default:
				break;
			}
		}

		switch (flags)
		{
		//case 0:		// !initedWide && !converted
		//case 2:		// !initedWide && converted
		//case 3:		//  initedWide && converted
		default:
			toInsert.MakeAscii();
			str.insert(p0, toInsert.str, pos, n);
		case 1:		//  initedWide && !converted
			toInsert.MakeWide();
			wstr.insert(p0, toInsert.wstr, pos, n);
		}
		return *this;
	}

	WONString& insert(size_t p0, const std::string& toInsert, size_t pos, size_t n)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.insert(p0, toInsert, pos, n);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			wstring tmp;
			StringToWString(toInsert, pos, n, tmp);
			wstr.insert(p0, tmp);
		}
		return *this;
	}

	WONString& insert(size_t p0, const std::wstring& toInsert, size_t pos, size_t n)
	{
		switch (flags)
		{
		case 0:	// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:	// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(toInsert, pos, n, tmp);
				str.insert(p0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.insert(p0, toInsert, pos, n);
		}
		return *this;
	}

	WONString& insert(size_t p0, size_t n, char c)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.insert(p0, n, c);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			string src(n, c);
			wstring tmp;
			StringToWString(src, tmp);
			wstr.insert(p0, tmp);
		}
		return *this;
	}

	WONString& insert(size_t p0, size_t n, wchar_t c)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				wstring src(n, c);
				string tmp;
				WStringToString(src, tmp);
				str.insert(p0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.insert(p0, n, c);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const WONString& replaceWith)
	{
		if (replaceWith.isWide())
		{
			switch (flags)
			{
			case 0:	// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:	// !initedWide && converted
				flags = initedWide | converted;			
			default:
				break;
			}
		}

		switch (flags)
		{
		//case 0:		// !initedWide && !converted
		//case 2:		// !initedWide && converted
		//case 3:		//  initedWide && converted
		default:
			replaceWith.MakeAscii();
			str.replace(p0, n0, replaceWith.str);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			replaceWith.MakeWide();
			wstr.replace(p0, n0, replaceWith.wstr);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const std::string& replaceWith)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.replace(p0, n0, replaceWith);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			wstring tmp;
			StringToWString(replaceWith, tmp);
			wstr.replace(p0, n0, tmp);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const std::wstring& replaceWith)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(replaceWith, tmp);
				str.replace(p0, n0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.replace(p0, n0, replaceWith);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const char* replaceWith)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.replace(p0, n0, replaceWith);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			wstring tmp;
			StringToWString(replaceWith, tmp);
			wstr.replace(p0, n0, tmp);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const wchar_t* replaceWith)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(replaceWith, tmp);
				str.replace(p0, n0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.replace(p0, n0, replaceWith);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const WONString& replaceWith, size_t pos, size_t n)
	{
		if (replaceWith.isWide())
		{
			switch (flags)
			{
			case 0:	// !initedWide && !converted
				StringToWString(str, wstr);
			case 2:	// !initedWide && converted
				flags = initedWide | converted;			
			default:
				break;
			}
		}

		switch (flags)
		{
		//case 0:		// !initedWide && !converted
		//case 2:		// !initedWide && converted
		//case 3:		//  initedWide && converted
		default:
			replaceWith.MakeAscii();
			str.replace(p0, n0, replaceWith.str, pos, n);
			if (!flags)
				break;
		case 1:		//  initedWide && !converted
			replaceWith.MakeWide();
			wstr.replace(p0, n0, replaceWith.wstr, pos, n);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const std::string& replaceWith, size_t pos, size_t n)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide &&  converted
		//case 3:	//  initedWide &&  converted
		default:
			str.replace(p0, n0, replaceWith, pos, n);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			string src(replaceWith, pos, n);
			wstring tmp;
			StringToWString(src, tmp);
			wstr.replace(p0, n0, tmp);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const std::wstring& replaceWith, size_t pos, size_t n)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				wstring src(replaceWith, pos, n);
				string tmp;
				WStringToString(src, tmp);
				str.replace(p0, n0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.replace(p0, n0, replaceWith, pos, n);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const char* replaceWith, size_t n)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.replace(p0, n0, replaceWith, n);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			wstring tmp;
			StringToWString(replaceWith, n, tmp);
			wstr.replace(p0, n0, tmp);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, const wchar_t* replaceWith, size_t n)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				string tmp;
				WStringToString(replaceWith, n, tmp);
				str.replace(p0, n0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.replace(p0, n0, replaceWith, n);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, size_t n, char c)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			str.replace(p0, n0, n, c);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			string src(n, c);
			wstring tmp;
			StringToWString(src, tmp);
			wstr.replace(p0, n0, tmp);
		}
		return *this;
	}

	WONString& replace(size_t p0, size_t n0, size_t n, wchar c)
	{
		switch (flags)
		{
		case 0:		// !initedWide && !converted
			StringToWString(str, wstr);
		case 2:		// !initedWide && converted
			flags = initedWide | converted;
		//case 3:	//  initedWide && converted
		default:
			{
				wstring src(n, c);
				string tmp;
				WStringToString(src, tmp);
				str.replace(p0, n0, tmp);
			}
		case 1:	//  initedWide && !converted
			wstr.replace(p0, n0, n, c);
		}
		return *this;
	}

	size_t copy(char* s, size_t n, size_t pos = 0) const
	{
		MakeAscii();
		return str.copy(s, n, pos);
	}

	size_t copy(wchar_t* s, size_t n, size_t pos = 0) const
	{
		MakeWide();
		return wstr.copy(s, n, pos);
	}


	size_t find(const WONString& toFind, size_t pos = 0) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.find(toFind.str, pos);
			MakeWide();
		}
		return wstr.find(toFind.wstr, pos);
	}
	
	size_t find(const std::string& toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find(tmp, pos);
	}

	size_t find(const std::wstring& toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find(tmp, pos);
	}

	size_t find(const char* toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find(tmp, pos);
	}

	size_t find(const wchar_t* toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find(tmp, pos);
	}

	size_t find(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.find(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.find(tmp, pos);
	}

	size_t find(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.find(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.find(tmp, pos);
	}

	size_t find(char toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find(toFind, pos);
		return wstr.find((wchar)toFind, pos);
	}

	size_t find(wchar_t toFind, size_t pos = 0) const
	{
		MakeWide();
		return wstr.find(toFind, pos);
	}

	size_t rfind(const WONString& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.rfind(toFind.str, pos);
			MakeWide();
		}
		return wstr.rfind(toFind.wstr, pos);
	}

	size_t rfind(const std::string& toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.rfind(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.rfind(tmp, pos);
	}

	
	size_t rfind(const std::wstring& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.rfind(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.rfind(tmp, pos);
	}

	size_t rfind(const char* toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.rfind(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.rfind(tmp, pos);
	}

	size_t rfind(const wchar_t* toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.rfind(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.rfind(tmp, pos);
	}

	size_t rfind(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.rfind(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.rfind(tmp, pos);
	}

	size_t rfind(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.rfind(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.rfind(tmp, pos);
	}

	size_t rfind(char toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.rfind(toFind, pos);
		return wstr.rfind((wchar)toFind, pos);
	}

	size_t rfind(wchar toFind, size_t pos = (size_t)-1) const
	{
		MakeWide();
		return wstr.rfind(toFind, pos);
	}

	size_t find_first_of(const WONString& toFind, size_t pos = 0) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.find_first_of(toFind.str, pos);
			MakeWide();
		}
		return wstr.find_first_of(toFind.wstr, pos);
	}

	size_t find_first_of(const std::string& toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_first_of(tmp, pos);
	}

	size_t find_first_of(const std::wstring& toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find_first_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_first_of(tmp, pos);
	}

	size_t find_first_of(const char* toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_first_of(tmp, pos);
	}

	size_t find_first_of(const wchar_t* toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find_first_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_first_of(tmp, pos);
	}

	size_t find_first_of(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.find_first_of(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.find_first_of(tmp, pos);
	}

	size_t find_first_of(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.find_first_of(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.find_first_of(tmp, pos);
	}

	size_t find_first_of(char toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_of(toFind, pos);
		return wstr.find_first_of((wchar)toFind, pos);
	}

	size_t find_first_of(wchar_t toFind, size_t pos = 0) const
	{
		MakeWide();
		return wstr.find_first_of(toFind, pos);
	}

	size_t find_last_of(const WONString& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.find_last_of(toFind.str, pos);
			MakeWide();
		}
		return wstr.find_last_of(toFind.wstr, pos);
	}

	size_t find_last_of(const std::string& toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_last_of(tmp, pos);
	}

	
	size_t find_last_of(const std::wstring& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.find_last_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_last_of(tmp, pos);
	}

	size_t find_last_of(const char* toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_last_of(tmp, pos);
	}

	size_t find_last_of(const wchar_t* toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.find_last_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_last_of(tmp, pos);
	}

	size_t find_last_of(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.find_last_of(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.find_last_of(tmp, pos);
	}

	size_t find_last_of(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.find_last_of(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.find_last_of(tmp, pos);
	}

	size_t find_last_of(char toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_of(toFind, pos);
		return wstr.find_last_of((wchar)toFind, pos);
	}

	size_t find_last_of(wchar toFind, size_t pos = (size_t)-1) const
	{
		MakeWide();
		return wstr.find_last_of(toFind, pos);
	}

	size_t find_first_not_of(const WONString& toFind, size_t pos = 0) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.find_first_not_of(toFind.str, pos);
			MakeWide();
		}
		return wstr.find_first_not_of(toFind.wstr, pos);
	}

	size_t find_first_not_of(const std::string& toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_not_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(const std::wstring& toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find_first_not_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(const char* toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_not_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(const wchar_t* toFind, size_t pos = 0) const
	{
		if (isWide())
			return wstr.find_first_not_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.find_first_not_of(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.find_first_not_of(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.find_first_not_of(tmp, pos);
	}

	size_t find_first_not_of(char toFind, size_t pos = 0) const
	{
		if (!isWide())
			return str.find_first_not_of(toFind, pos);
		return wstr.find_first_not_of((wchar)toFind, pos);
	}

	size_t find_first_not_of(wchar_t toFind, size_t pos = 0) const
	{
		MakeWide();
		return wstr.find_first_not_of(toFind, pos);
	}

	size_t find_last_not_of(const WONString& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			toFind.MakeWide();
		else
		{
			if (!toFind.isWide())
				return str.find_last_not_of(toFind.str, pos);
			MakeWide();
		}
		return wstr.find_last_not_of(toFind.wstr, pos);
	}

	size_t find_last_not_of(const std::string& toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_not_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_last_not_of(tmp, pos);
	}

	
	size_t find_last_not_of(const std::wstring& toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.find_last_not_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_last_not_of(tmp, pos);
	}

	size_t find_last_not_of(const char* toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_not_of(toFind, pos);
		wstring tmp;
		StringToWString(toFind, tmp);
		return wstr.find_last_not_of(tmp, pos);
	}

	size_t find_last_not_of(const wchar_t* toFind, size_t pos = (size_t)-1) const
	{
		if (isWide())
			return wstr.find_last_not_of(toFind, pos);
		string tmp;
		WStringToString(toFind, tmp);
		return str.find_last_not_of(tmp, pos);
	}

	size_t find_last_not_of(const char* toFind, size_t pos, size_t n) const
	{
		if (!isWide())
			return str.find_last_not_of(toFind, pos, n);
		wstring tmp;
		StringToWString(toFind, n, tmp);
		return wstr.find_last_not_of(tmp, pos);
	}

	size_t find_last_not_of(const wchar_t* toFind, size_t pos, size_t n) const
	{
		if (isWide())
			return wstr.find_last_not_of(toFind, pos, n);
		string tmp;
		WStringToString(toFind, n, tmp);
		return str.find_last_not_of(tmp, pos);
	}

	size_t find_last_not_of(char toFind, size_t pos = (size_t)-1) const
	{
		if (!isWide())
			return str.find_last_not_of(toFind, pos);
		return wstr.find_last_not_of((wchar)toFind, pos);
	}

	size_t find_last_not_of(wchar_t toFind, size_t pos = (size_t)-1) const
	{
		MakeWide();
		return wstr.find_last_not_of(toFind, pos);
	}

	WONString substr(size_t pos = 0, size_t n = (size_t)-1) const
	{
		return WONString(*this, pos, n);
	}

	const char GetChar(size_t idx) const
	{
		MakeAscii();
		return str[idx];
	}

	const wchar_t GetWChar(size_t idx) const
	{
		MakeWide();
		return wstr[idx];
	}

	const WONString& ToUpper(void)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >(str.begin(), str.end(), str.begin(), toupper);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >(wstr.begin(), wstr.end(), wstr.begin(), towupper);
		}
		return *this;
	}

	const WONString& ToLower(void)
	{
		switch (flags)
		{
		//case 0:	// !initedWide && !converted
		//case 2:	// !initedWide && converted
		//case 3:	//  initedWide && converted
		default:
			std::transform<std::string::iterator, std::string::iterator, int (__cdecl *)(int) >(str.begin(), str.end(), str.begin(), tolower);
			if (!flags)
				break;
		case 1:	//  initedWide && !converted
			std::transform<std::wstring::iterator, std::wstring::iterator, wint_t (__cdecl *)(wint_t) >(wstr.begin(), wstr.end(), wstr.begin(), towlower);
		}
		return *this;
	}

	bool ReplaceSubString(const WONString& sKey, const WONString& sVal, bool bAllMatches = false)
	{
		bool bFound = false;
		size_t keyLength = sKey.length();
		size_t nPos = find(sKey);
		while (nPos != (size_t)-1)
		{
			replace(nPos, keyLength, sVal);
			bFound = true;
			nPos = find(sKey);

			if (! bAllMatches)
				break;
		}
		return bFound;
	}

};


// Doesn't actually use WONString (wouldn't make sense).
// Here as a alternative to stringstream, not supported by Linux
class wonstringstream : public std::iostream
{
private:
	class wonstringstreambuf : public std::streambuf
	{
	private:
		string s;
		unsigned int readPos;
		char readAheadChar;
		bool hasReadAheadChar;
	public:
		wonstringstreambuf() : hasReadAheadChar(false), readPos(0) {}

		virtual std::streamsize xsputn(const char* text, std::streamsize n);
		virtual int overflow(int ch);
		virtual std::streamsize xsgetn(char* text, std::streamsize n);
		virtual int underflow();
		virtual int uflow();
		const string& str() const	{ return s; }
		void str(const string& s0)	{ s.assign(s0); readPos = 0; hasReadAheadChar = false; }
		void clear()				{ s.erase(); readPos = 0; hasReadAheadChar = false; }
		size_t size() const			{ return s.size(); }
	};

	wonstringstreambuf sb;
public:
	explicit wonstringstream();

	void clear()				{ sb.clear(); }
	const string& str() const	{ return sb.str(); }
	void str(const string& s0)	{ sb.str(s0); }
	size_t size() const			{ return sb.size(); }
};


};

// Output Operator
inline std::ostream&
operator<<(std::ostream& os, const WONCommon::WONString& theStr)
{
	if (theStr.isWide())
		return (os << theStr.GetUnicodeString());
	return (os << theStr.GetAsciiString());
}
/*
#ifdef _LINUX
inline std::ostream&
operator<<(std::ostream& os, const std::basic_string<wchar>& theStr)
{
	const unsigned int aNumChars = theStr.size();
	for (int i = 0; i < aNumChars; ++i)
		os << theStr[i];
	return os;
}
#endif // _LINUX
*/
#endif
