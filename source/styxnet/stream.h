///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Stream Header
//


#ifndef __STREAM_H
#define __STREAM_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "std.h"

#pragma warning (push, 3)
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <strstream>
#pragma warning (pop)

using std::istream;
using std::ostream;
using std::fstream;
using std::iostream;

using std::strstream;
using std::istrstream;
using std::ostrstream;

using std::stringstream;
using std::istringstream;
using std::ostringstream;

using std::ends;
using std::endl;
using std::flush;
using std::ios;
using std::dec;
using std::hex;
using std::setw;
using std::setfill;
using std::setiosflags;
using std::setprecision;

#define HEX(n, w) setfill('0') << setw(w) << std::hex << setiosflags(ios::right | ios::uppercase) << n << 'h'

#endif

