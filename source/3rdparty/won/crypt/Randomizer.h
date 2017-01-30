#ifndef _RANDOMIZER_H
#define _RANDOMIZER_H

#ifndef _MINI_CRYPT

// Randomizer

// Static class that maintains a random number pool for generating keys within then
// crypt library.  Randomizer is use to generate Symmetric keys and Public/Private
// key pairs.

// *NOTE*
// Randomizer is meant to be used internally in the WON crypt lib and exposes some
// classes from crypto++.  Is is NOT recommended that Ramdomizer be used outsize of
// the WON crypt lib.

#include <memory>

// RandomPool and Exception from Crypto++
namespace CryptoPP
{
	class RandomPool;
	class Exception;
};

// In WONCrypt namespace
namespace WONCrypt
{

class Randomizer
{
public:
	// Get a random byte
	static unsigned char GetByte() throw(CryptoPP::Exception);

	// Get a random long
	static unsigned long GetLong() throw(CryptoPP::Exception);

	// Get a random short
	static unsigned short GetShort() throw(CryptoPP::Exception);

	// Get a random array of bytes
	static void GetBlock(unsigned char* theBlock, unsigned long theLen) throw(CryptoPP::Exception);

	// Reset random number generator
	static void Reset() throw(CryptoPP::Exception);

	// Fetch the pool
	static CryptoPP::RandomPool& GetPool();

	// Generate a seed from noise
	static __int64 GenerateSeed(void);

private:
	static std::auto_ptr<CryptoPP::RandomPool> mPoolP;  // MD5 random number generator

	// Do initial pool allocation
	static void AllocatePool();

	// Static class - disbale construction
	Randomizer();
};

};  //namespace WONCrypt

#endif
#endif
