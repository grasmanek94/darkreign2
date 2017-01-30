#include "WONString.h"


using namespace WONCommon;
using namespace std;


std::streamsize wonstringstream::wonstringstreambuf::xsputn(const char* text, std::streamsize n)
{
	if (n)
		s.append(text, n);
	return n;
}


int wonstringstream::wonstringstreambuf::overflow(int ch)
{
	if (ch > 0x00FF) // ??
		return 0;
	
	s.append(1, (char)ch);
	
	return 1;
}


std::streamsize wonstringstream::wonstringstreambuf::xsgetn(char* text, std::streamsize n)
{
	std::streamsize readSize = 0;
	if (n)
	{
		std::streamsize remaining = s.size() - readPos;
		readSize = (n > remaining) ? remaining : n;
		if (readSize)
		{
			memcpy(text, s.data() + readPos, readSize);
			readPos += readSize;
		}
	}
	return readSize;
}


int wonstringstream::wonstringstreambuf::underflow()
{
	if (hasReadAheadChar)
		return readAheadChar;
	
	if (s.size() == readPos)
		return EOF;

	readAheadChar = *(s.data() + readPos);
	readPos++;
	hasReadAheadChar = true;

	return readAheadChar;
}


int wonstringstream::wonstringstreambuf::uflow()
{
	if (hasReadAheadChar)
	{
		hasReadAheadChar = false;
		return readAheadChar;
	}

	if (s.size() == readPos)
		return EOF;

	return *(s.data() + readPos++);
}


wonstringstream::wonstringstream()
	: sb(), iostream(0)
{
	init(&sb);
}
