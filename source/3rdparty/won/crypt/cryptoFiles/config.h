#ifndef CRYPTOPP_CONFIG_H
#define CRYPTOPP_CONFIG_H

// define this if running on a little-endian CPU

#if defined(macintosh) && (macintosh == 1)
#else
#define IS_LITTLE_ENDIAN
#endif

// define this if your compiler doesn't support namespaces

#ifdef __GNUC__
//#define NO_NAMESPACE
#endif

// switch between different secure memory allocation mechnisms, this is the only
// one available right now

#define SECALLOC_DEFAULT

#define GZIP_OS_CODE 0

// how much memory to use for deflation (compression)
// note 16 bit compilers don't work with BIG_MEM (pointer wraps around segment boundary)

// #define SMALL_MEM
#define MEDIUM_MEM
// #define BIG_MEM

// Try this if your CPU has 256K internal cache or a slow multiply instruction
// and you want a (possibly) faster IDEA implementation using log tables

// #define IDEA_LARGECACHE

// Try this if you have a large cache or your CPU is slow manipulating
// individual bytes.

// #define DIAMOND_USE_PERMTABLE

// Define this if, for the linear congruential RNG, you want to use
// the original constants as specified in S.K. Park and K.W. Miller's
// CACM paper.

// #define LCRNG_ORIGINAL_NUMBERS

// Make sure these typedefs are correct for your platform

typedef unsigned char byte;
typedef unsigned short word16;
#if defined(__alpha) && !defined(_MSC_VER)
typedef unsigned int word32;
#else
typedef unsigned long word32;
#endif

#if defined(__GNUC__) || defined(__MWERKS__)
#define WORD64_AVAILABLE
typedef unsigned long long word64;
#define W64LIT(x) x##LL
#elif defined(_MSC_VER) || defined(__BCPLUSPLUS__)
#define WORD64_AVAILABLE
typedef unsigned __int64 word64;
#define W64LIT(x) x##ui64
#endif

// word should have the same size as your CPU registers
// dword should be twice as big as word

#if (defined(__GNUC__) && !defined(__alpha)) || defined(__MWERKS__)
typedef unsigned long word;
typedef unsigned long long dword;
#elif defined(_MSC_VER) || defined(__BCPLUSPLUS__)
typedef unsigned __int32 word;
typedef unsigned __int64 dword;
#else
typedef unsigned int word;
typedef unsigned long dword;
#endif

const unsigned int WORD_SIZE = sizeof(word);
const unsigned int WORD_BITS = WORD_SIZE * 8;

#define LOW_WORD(x) (word)(x)

#ifdef IS_LITTLE_ENDIAN
#define HIGH_WORD(x) (*(((word *)&(x))+1))
#else
#define HIGH_WORD(x) (*((word *)&(x)))
#endif

// if the above HIGH_WORD macro doesn't work (if you are not sure, compile it
// and run the validation tests), try this:
// #define HIGH_WORD(x) (word)((x)>>WORD_BITS)

#ifdef _MSC_VER
// 4250: dominance
// 4660: explicitly instantiating a class that's already implicitly instantiated
// 4786: identifer was truncated in debug information
#pragma warning(disable: 4250 4660 4786)
#endif

#ifdef NO_NAMESPACE
#define std
#define CryptoPP
#define USING_NAMESPACE(x)
#define NAMESPACE_BEGIN(x)
#define NAMESPACE_END
#define ANONYMOUS_NAMESPACE_BEGIN
#else
#define USING_NAMESPACE(x) using namespace x;
#define NAMESPACE_BEGIN(x) namespace x {
#define ANONYMOUS_NAMESPACE_BEGIN namespace {
#define NAMESPACE_END }
#endif

#ifdef _MSC_VER
// VC60 workaround
#define STDMIN std::_cpp_min
#define STDMAX std::_cpp_max
#else
#define STDMIN std::min
#define STDMAX std::max
#endif

#endif
