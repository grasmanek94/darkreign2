#include "common/won.h"
#include <iostream>
#include <set>

#include "Randomizer.h"
#include "BFSymmetricKey.h"
#include "EGPublicKey.h"
#include "EGPrivateKey.h"

using std::cout;
using namespace CryptoPP;
using namespace WONCrypt;

class LessCryptKey
{
public:
	bool operator()(const CryptKeyBase* k1, const CryptKeyBase* k2) const
		{ return (*k1 < *k2 ? true : false); }
};

typedef std::set<CryptKeyBase*, LessCryptKey> KeySet;


int
main(int argc, const char** argv)
{
	//WDBG_INIT(WDBG_LIBLOW, "CryptTest", NULL, "E:\\Logs");

	{
		cout << endl << "Testing Random Number Generator..." << endl;
		for (int i=1; i <= 100; i++)
		{
			unsigned short aShort = Randomizer::GetShort();
			cout << std::hex << aShort << std::dec << "  ";
			if ((i % 5) == 0) cout << endl;
		}
		cout << endl;
	}

	{
		cout << endl << "Testing Key Classes..." << endl;
		KeySet aKeySet;
		aKeySet.insert(new BFSymmetricKey(CryptKeyBase::KEYLEN_DEF));
		aKeySet.insert(new BFSymmetricKey(4));
		BFSymmetricKey aKey(12);
		aKeySet.insert(new BFSymmetricKey(aKey.GetKeyLen(), aKey.GetKey()));

		aKeySet.insert(new EGPrivateKey(CryptKeyBase::KEYLEN_DEF));
		aKeySet.insert(new EGPrivateKey(4));
		EGPrivateKey aPvKey(12);
		EGPrivateKey aPvKey1(0);
		aKeySet.insert(new EGPrivateKey(aPvKey.GetKeyLen(), aPvKey.GetKey()));
		aKeySet.insert(new EGPrivateKey(aPvKey1));

		const EGPublicKey& aPubKey = dynamic_cast<const EGPublicKey&>(aPvKey1.GetPublicKey());
		aKeySet.insert(new EGPublicKey(aPvKey));
		aKeySet.insert(new EGPublicKey(aPubKey));

		// Output keys
		KeySet::iterator anItr(aKeySet.begin());
		for (int i=1; anItr != aKeySet.end(); i++)
		{
			cout << "Key " << i << ":  " << *(*anItr) << endl;
			delete *anItr;
			anItr = aKeySet.erase(anItr);
		}
		cout << endl;
	}

	{
		cout << endl << "Testing SymmetricCrypt..." << endl;
		BFSymmetricKey* myKey  = new BFSymmetricKey(8);
		BFSymmetricKey* badKey = new BFSymmetricKey(12);
		char* iStr = "Mike has no life!";
		BFSymmetricKey::CryptReturn encrypt(myKey->Encrypt(reinterpret_cast<unsigned char*>(iStr), strlen(iStr)+1));
		BFSymmetricKey::CryptReturn decrypt(myKey->Decrypt(encrypt.first, encrypt.second));
		cout << "Input:  " << iStr << endl;
		cout << "Output: " << reinterpret_cast<const char*>(decrypt.first ? decrypt.first : (const unsigned char*)"NULL") << endl;
		delete encrypt.first;  delete decrypt.first;
		delete myKey;
		delete badKey;
	}

	{
		cout << endl << "Testing ElGamal Crypt..." << endl;
		EGPrivateKey* myPrivKey = new EGPrivateKey(8);
		EGPrivateKey* badKey = new EGPrivateKey(12);
		EGPublicKey*  myPubKey  = new EGPublicKey(*myPrivKey);
		char* iStr = "Mike has no life!";
		EGPublicKey::CryptReturn  encrypt(myPubKey->Encrypt(reinterpret_cast<unsigned char*>(iStr), strlen(iStr)+1));
		EGPrivateKey::CryptReturn decrypt(myPrivKey->Decrypt(encrypt.first, encrypt.second));
		cout << "Input:  " << iStr << endl;
		cout << "Output: " << reinterpret_cast<const char*>(decrypt.first ? decrypt.first : (const unsigned char*)"NULL") << endl;
		delete encrypt.first;  delete decrypt.first;
		delete myPrivKey;
		delete myPubKey;
		delete badKey;
	}

	{
		cout << endl << "Testing ElGamal Sigs..." << endl;
		EGPrivateKey* myPrivKey = new EGPrivateKey(8);
		EGPrivateKey* badKey = new EGPrivateKey(12);
		EGPublicKey*  myPubKey  = new EGPublicKey(*myPrivKey);
		char* iStr = "Mike has no life!";
		EGPrivateKey::CryptReturn sig(myPrivKey->Sign(reinterpret_cast<unsigned char*>(iStr), strlen(iStr)+1));
		int tstSig = myPubKey->Verify(sig.first, sig.second, reinterpret_cast<unsigned char*>(iStr), strlen(iStr)+1);
		cout << "Msg:    " << iStr << "  sigLen=" << sig.second << endl;
		cout << "Verify: " << (tstSig ? "TRUE" : "FALSE") << endl;
		delete sig.first;
		delete myPrivKey;
		delete myPubKey;
	}

	return 0;
}
