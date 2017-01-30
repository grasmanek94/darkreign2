#ifndef __WON_ENDIAN_H__
#define __WON_ENDIAN_H__


#include "won.h"


inline bool isLittleEndian(void)
{
	short s = 1;				// = 0x0001 big-endian,    0x0100 little-endian
	return (*((char*)&s)) == 1;	//     ^^----------or--------^^
}	// Should optimize into a constant


inline bool isBigEndian(void)
{
	short s = 1;				// = 0x0001 big-endian,    0x0100 little-endian
	return (*((char*)&s)) != 1;	//     ^^----------or--------^^
}	// Should optimize into a constant



template <class T>
inline T SwitchEndian(const T& t)
{
	T newT;
	for (int curByte = 0; curByte < sizeof(T); ++curByte)
		*((char*)&newT + curByte) = *(((char*)&t) + ((sizeof(T) - 1) - curByte));
	return newT;
}

template <class T>
inline T getLittleEndian(const T& t)
{
	if (!isLittleEndian())
		return SwitchEndian(t);
	return t;
}

template <class T>
inline T getBigEndian(const T& t)
{
	if (isLittleEndian())
		return SwitchEndian(t);
	return t;
}

template <class T>
inline void makeLittleEndian(T& t)
{
	if (!isLittleEndian())
		t = SwitchEndian(t);
}

template <class T>
inline void makeBigEndian(T& t)
{
	if (isLittleEndian())
		t = SwitchEndian(t);
}


inline void makeLittleEndianWString(std::wstring& s)
{
	if (!isLittleEndian())
	{
		size_t count = s.size();
		for (size_t i = 0; i < count; i++)
			s[i] = SwitchEndian(s[i]);
	}
}

inline void makeBigEndianWString(std::wstring& s)
{
	if (isLittleEndian())
	{
		size_t count = s.size();
		for (size_t i = 0; i < count; i++)
			s[i] = SwitchEndian(s[i]);
	}
}


#endif
