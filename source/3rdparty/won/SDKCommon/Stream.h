/********************************************************************************

  WON::Stream
	
	Stream is an interface (abstract base class with no data members) for classes
	which support reading and writing via the insertion (<<) and extraction (>>)
	operators.  Any object that can be inserted into, or extracted from, a Stream
	object, can be inserted into, or extracted from, a derived object.


  WON::wonstream

	Standard iostream -derived class for formatted text stream support.
	
	2/1/99	-	Colen Garoutte-Carson	- Initial functionality

********************************************************************************/

#ifndef __WON_STREAM_H__
#define __WON_STREAM_H__


#include <iostream>
#include <string>
#include <string.h>
#include "common/won.h"
#ifdef _LINUX
#include "common/linuxGlue.h"
#elif defined(macintosh) && (macintosh == 1)
#include "common/macGlue.h"
#endif

#include "common/WONEndian.h"


namespace WONAPI {


class Stream
{
public:
	virtual ~Stream();
	
	virtual unsigned long Write(unsigned long count, const void* buffer) = 0;
	virtual unsigned long Read(unsigned long count, void* buffer) = 0;

	Stream& operator>>(long& l)				{ Read(sizeof(long), &l); makeLittleEndian(l); return *this;			}
	Stream& operator>>(unsigned long& l)	{ Read(sizeof(unsigned long), &l); makeLittleEndian(l); return *this;	}
	Stream& operator>>(short& s)			{ Read(sizeof(short), &s); makeLittleEndian(s); return *this;			}
	Stream& operator>>(unsigned short& s)	{ Read(sizeof(unsigned short), &s); makeLittleEndian(s); return *this;	}
	Stream& operator>>(char& c)				{ Read(sizeof(char), &c); return *this;				}
	Stream& operator>>(unsigned char& c)	{ Read(sizeof(unsigned char), &c); return *this;	}
	Stream& operator>>(signed char& c)		{ Read(sizeof(signed char), &c); return *this;		}
	Stream& operator>>(bool& b)				{ Read(sizeof(bool), &b); return *this;				}

	Stream& operator<<(long l)				{ makeLittleEndian(l); Write(sizeof(long), &l); return *this;			}
	Stream& operator<<(unsigned long l)		{ makeLittleEndian(l); Write(sizeof(unsigned long), &l); return *this;	}
	Stream& operator<<(short s)				{ makeLittleEndian(s); Write(sizeof(short), &s); return *this;			}
	Stream& operator<<(unsigned short s)	{ makeLittleEndian(s); Write(sizeof(unsigned short), &s); return *this;	}
	Stream& operator<<(bool b)				{ Write(sizeof(bool), &b); return *this;			}
	Stream& operator<<(char& c)				{ Write(sizeof(char), &c); return *this;			}
	Stream& operator<<(unsigned char& c)	{ Write(sizeof(unsigned char), &c); return *this;	}
	Stream& operator<<(signed char& c)		{ Write(sizeof(signed char), &c); return *this;		}
	Stream& operator<<(const char* s)		{ Write(strlen(s), s); return *this;				}
	Stream& operator<<(const std::string& s){ Write(s.size(), s.data()); return *this;			}
	Stream& operator<<(const std::wstring& s)
	{
#ifdef _LINUX
		std::wstring::const_iterator itor = s.begin();
		while (itor != s.end())
		{
			wchar wc = *itor;
			Write(sizeof(wchar), &wc);
			itor++;
		}
#else
		if (isLittleEndian())
			Write(s.size()*2, s.data());
		else
		{
			std::wstring::const_iterator itor = s.begin();
			while (itor != s.end())
			{
				wchar wc = *itor;
				wc = SwitchEndian(wc);
				Write(sizeof(wchar), &wc);
				itor++;
			}
		}
#endif // _LINUX
		return *this;
	}
};


class wonstream : public std::iostream
{
private:
	class streambuf : public std::streambuf
	{
	private:
		Stream* socket;
		char readAheadChar;
		bool hasReadAheadChar;
	public:
		streambuf(Stream& stream) : hasReadAheadChar(false) {}

		virtual std::streamsize xsputn(const char* text, std::streamsize n);
		virtual int overflow(int ch);
		virtual std::streamsize xsgetn(char* text, std::streamsize n);
		virtual int underflow();
		virtual int uflow();
	};

	streambuf sb;
public:
	explicit wonstream(Stream& stream);
};


};

#endif
