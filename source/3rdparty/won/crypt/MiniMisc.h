#ifndef __SMALLCRYPT_MISC_H__
#define __SMALLCRYPT_MISC_H__

#pragma warning(disable: 4530)

#include <string>

#ifdef _LINUX
#include "linuxGlue.h"
#elif defined(macintosh) && (macintosh == 1)
#include "macGlue.h"
#endif



namespace WONCryptMini
{

typedef std::basic_string<unsigned char> RawBuffer;

}


#endif