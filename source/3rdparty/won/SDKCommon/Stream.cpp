
#include "Stream.h"


using namespace WONAPI;


Stream::~Stream()
{
}


std::streamsize wonstream::streambuf::xsputn(const char* text, std::streamsize n)
{
	if (!n)
		return 0;

	unsigned long written = socket->Write(n, text);
	
	return (std::streamsize)written;
}


int wonstream::streambuf::overflow(int ch)
{
	if (ch > 0x00FF) // ??
		return 0;
	
	char c = ch;
	unsigned long written = socket->Write(1, &c);
	
	return !written ? EOF : (std::streamsize)written;
}


std::streamsize wonstream::streambuf::xsgetn(char* text, std::streamsize n)
{
	if (!n)
		return 0;

	unsigned long read = socket->Read(n, text);

	return (std::streamsize)read;
}


int wonstream::streambuf::underflow()
{
	if (hasReadAheadChar)
		return readAheadChar;
	
	unsigned long read = socket->Read(1, &readAheadChar);

	if (!read)
		return EOF;

	hasReadAheadChar = true;
	return readAheadChar;
}


int wonstream::streambuf::uflow()
{
	if (hasReadAheadChar)
	{
		hasReadAheadChar = false;
		return readAheadChar;
	}

	char c;
	return !socket->Read(1, &c) ? EOF : c;
}


using std::iostream;

wonstream::wonstream(Stream& stream)
	: sb(stream), iostream(0)
{
	init(&sb);
}
