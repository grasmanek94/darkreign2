
#ifndef __WON_MEMSTREAM_H__
#define __WON_MEMSTREAM_H__


#include "Stream.h"


namespace WONAPI {


class MemStream : public Stream
{
private:
	unsigned char* buffer;
	size_t sizeLeft;
	size_t curPos;
	bool owns;

public:
	MemStream(void* buf, size_t size)
		:	buffer((unsigned char*)buf), sizeLeft(size), owns(false), curPos(0)
	{ }

	explicit MemStream(size_t size)
		:	buffer(new unsigned char[size]), sizeLeft(size), owns(true), curPos(0)
	{ }

	virtual ~MemStream();

	virtual unsigned long Write(unsigned long count, const void* buf);
	virtual unsigned long Read(unsigned long count, void* buf);

	unsigned char* GetBufferAtPos()	{ return buffer + curPos; }
	size_t GetBufferSizeLeft()		{ return sizeLeft; }

	using Stream::operator<<;
	using Stream::operator>>;
};


};


#endif
