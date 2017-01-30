#include "MemStream.h"


using namespace WONAPI;


MemStream::~MemStream()
{
	if (buffer && owns)
		delete buffer;
}


unsigned long MemStream::Write(unsigned long count, const void* buf)
{
	if (buffer && count)
	{
		size_t copySize = count > sizeLeft ? sizeLeft : count;
		if (buf)
			memcpy(buffer + curPos, buf, copySize);
		sizeLeft -= copySize;
		curPos += copySize;
		return copySize;
	}
	return 0;
}


unsigned long MemStream::Read(unsigned long count, void* buf)
{
	if (buffer && count)
	{
		size_t copySize = count > sizeLeft ? sizeLeft : count;
		if (buf)
			memcpy(buf, buffer + curPos, copySize);
		sizeLeft -= copySize;
		curPos += copySize;
		return copySize;
	}
	return 0;
}


