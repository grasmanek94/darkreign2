#ifndef UTF8STRING_H
#define UTF8STRING_H

#include <string>

namespace WONCommon {

typedef unsigned short UCS2;

class UTF8String {
private:
	std::basic_string<unsigned char> mRawString;
	int                              mLength;
public:
	// construction/destruction
	UTF8String();                                 // empty
	UTF8String(const unsigned char* theUTF8Data, unsigned int theNumBytes); // from a raw UTF8 string
	UTF8String(const std::string& theStringR);            // from a narrow (ASCII) string
	UTF8String(const std::wstring& theWideStringR);       // from a wide (UCS2) string
	UTF8String(const std::basic_string<unsigned char>& theUnsignedStringR); // from a raw binary (unsigned) string
	UTF8String(const UTF8String& aUTF8String); // cpy ctor
	~UTF8String();

	// operators
	UTF8String& operator=(const std::string& theStringR);
	UTF8String& operator=(const std::wstring& theWideStringR);
	UTF8String& operator=(const std::basic_string<unsigned char>& theUnsignedStringR);
	UTF8String& operator=(const UTF8String& aUTF8String); // assignement op

	// concatenation
	UTF8String& operator+=(const UTF8String& aString);

	// conversions
	operator std::string() const;  // convert to narrow (ASCII) string
	operator std::wstring() const; // convert to wide (UCS2) string
	operator std::basic_string<unsigned char>() const; // convert to raw binary (unsigned) string

	// operations
	const unsigned char* c_str() const;
	int size() const;   // number of bytes in the string
	int length() const; // number of characters in the string
private:
	void InitFromASCII(const std::string& theStringR);
	void InitFromUnsigned(const std::basic_string<unsigned char>& theUnsignedStringR);
	void InitFromUCS2(const std::wstring& theWideStringR);
	void RecalculateLength();
};

// Binary ops
UTF8String operator+(const UTF8String& left, const UTF8String& right);

};

#endif // UTF8STRING_H