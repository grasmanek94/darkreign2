#include "common/won.h"
#include <iostream>
#include <time.h>
#include "crypt/EGPublicKey.h"
#include "crypt/EGPrivateKey.h"
#include "Auth1PublicKeyBlock.h"
#include "WON_AuthCertificate1.h"
#include "WON_AuthPublicKeyBlock1.h"
#include "WON_AuthFactory.h"

using std::cout;
using WONCrypt::EGPrivateKey;
using WONCrypt::EGPublicKey;

int
main(int argc, const char** argv)
{
	WDBG_INIT(WDBG_LIBLOW, "AuthTest", NULL, "E:\\Logs");
	EGPrivateKey aKey1(12);
	EGPrivateKey aKey2(12);
	EGPrivateKey aKey3(12);

	{
		cout << endl << "Testing AuthCertificateV1..." << endl;
		WON_AuthCertificate1* aCert = WON_AuthFactory::NewAuthCertificate1();
		aCert->SetLifespan(time(NULL), 3600);
		aCert->SetUserId(10);
		aCert->SetCommunityId(23);
		aCert->SetTrustLevel(99);
		aCert->SetPublicKey(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen());
//		cout << aCert << endl;
		cout << "Cert Pack returns: " << aCert->Pack(aKey1.GetKey(), aKey1.GetKeyLen()) << endl;
//		cout << aCert << endl;

		WON_AuthCertificate1* aRead = WON_AuthFactory::NewAuthCertificate1(aCert->GetRaw(), aCert->GetRawLen());
		cout << "Read IsValid returns: " << aRead->IsValid() << endl;
		cout << "Read Verify returns: " << aRead->Verify(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen()) << endl;
//		cout << aRead << endl;
		time_t aTime = aRead->GetIssueTime();
		cout << "aRead.Issue = " << ctime(&aTime);
		aTime = aRead->GetExpireTime();
		cout << "aRead.Expire = " << ctime(&aTime);
		cout << "aRead.UserId = " << aRead->GetUserId() << endl;
		cout << "aRead.CommunityId = " << aRead->GetCommunityId() << endl;
		cout << "aRead.TrustLevel = " << aRead->GetTrustLevel() << endl;
		cout << "aRead.PubKeyLen = " << aRead->GetPubKeyLen() << endl;

		cout << endl << endl;
		time_t now = time(NULL);

		WON_AuthCertificate1* aCert1 = WON_AuthFactory::NewAuthCertificate1();
		aCert1->SetLifespan(now, 3600);
		aCert1->SetUserId(1);
		aCert1->SetCommunityId(1);
		aCert1->SetTrustLevel(1);
		aCert1->SetPublicKey(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen());

		WON_AuthCertificate1* aCert2 = WON_AuthFactory::NewAuthCertificate1();
		aCert2->SetLifespan(now+10, 3600);
		aCert2->SetUserId(23);
		aCert2->SetCommunityId(23);
		aCert2->SetTrustLevel(23);
		aCert2->SetPublicKey(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen());

		WON_AuthCertificate1* aCert3 = WON_AuthFactory::NewAuthCertificate1();
		aCert3->SetLifespan(now+20, 3600);
		aCert3->SetUserId(99);
		aCert3->SetCommunityId(99);
		aCert3->SetTrustLevel(99);
		aCert3->SetPublicKey(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen());

		cout << "aCert1 < aCert2 = " << (*aCert1 < *aCert2) << endl;
		cout << "aCert3 > aCert1 = " << (*aCert3 > *aCert1) << endl;
		cout << "aCert == aRead  = " << (*aCert == *aRead) << endl;

		WON_AuthFactory::DeleteAuthCertificate1(aCert);
		WON_AuthFactory::DeleteAuthCertificate1(aRead);
		WON_AuthFactory::DeleteAuthCertificate1(aCert1);
		WON_AuthFactory::DeleteAuthCertificate1(aCert2);
		WON_AuthFactory::DeleteAuthCertificate1(aCert3);
	}

	{
		time_t now = time(NULL);

		WON_AuthCertificate1* aCert = WON_AuthFactory::NewAuthCertificate1();
		aCert->SetLifespan(time(NULL), 3600);
		aCert->SetUserId(10);
		aCert->SetCommunityId(23);
		aCert->SetTrustLevel(99);
		aCert->SetPublicKey(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen());
		
		WONAuth::Auth1PublicKeyBlock aBlock(1);
		aBlock.SetLifespan(now, 3600);
		aBlock.KeyList().push_back(dynamic_cast<const EGPublicKey&>(aKey1.GetPublicKey()));
		aBlock.KeyList().push_back(dynamic_cast<const EGPublicKey&>(aKey2.GetPublicKey()));
		aBlock.KeyList().push_back(dynamic_cast<const EGPublicKey&>(aKey3.GetPublicKey()));
		cout << aBlock << endl;
		cout << "Block Pack returns: " << aBlock.Pack(aKey1) << endl;
		cout << aBlock << endl;

		WON_AuthPublicKeyBlock1* aRead = WON_AuthFactory::NewAuthPublicKeyBlock1(aBlock.GetRaw(), aBlock.GetRawLen());
		cout << "Read IsValid returns: " << aRead->IsValid() << endl;
		cout << "Read Verify returns: " << aRead->Verify(aKey1.GetPublicKey().GetKey(), aKey1.GetPublicKey().GetKeyLen()) << endl;
//		cout << aRead << endl;
		time_t aTime = aRead->GetIssueTime();
		cout << "aRead.Issue = " << ctime(&aTime);
		aTime = aRead->GetExpireTime();
		cout << "aRead.Expire = " << ctime(&aTime);
		cout << "aRead.BlockId = " << aRead->GetBlockId() << endl;
		cout << "aRead.NumKeys = " << aRead->GetNumKeys() << endl;
		aRead->GetFirstKey();
		for (int i=1; aRead->GetNextKey().mKeyP; i++) ;
		cout << "aRead.KeyCount = " << i << endl;

		cout << "Verify Cert: " << aRead->VerifyCertificate(aCert) << endl;
		/*
		AuthPublicKeyBlock1 aBlock1(1);
		aBlock1.SetLifespan(now, 3600);
		AuthPublicKeyBlock1 aBlock2(1);
		aBlock2.SetLifespan(now, 3600);
		AuthPublicKeyBlock1 aBlock3(1);
		aBlock3.SetLifespan(now, 3600);

		EGPublicKey aPubKey1(aKey1);
		EGPublicKey aPubKey2(aKey2);
		EGPublicKey aPubKey3(aKey3);
		cout << "key1 < key2 = " << (aPubKey1 < aPubKey2) << endl;
		cout << "key2 < key3 = " << (aPubKey2 < aPubKey3) << endl;
		cout << "key1 < key3 = " << (aPubKey1 < aPubKey3) << endl;

		aBlock1.KeyList().push_back(aPubKey1);
		aBlock1.KeyList().push_back(aPubKey2);
		aBlock2.KeyList().push_back(aPubKey1);
		aBlock2.KeyList().push_back(aPubKey3);
		aBlock3.KeyList().push_back(aPubKey1);
		cout << "Block1.Compare(Block2) = " << aBlock1.Compare(aBlock2) << endl;
		cout << "Block2.Compare(Block3) = " << aBlock2.Compare(aBlock3) << endl;
		cout << "Block1.Compare(Block3) = " << aBlock1.Compare(aBlock3) << endl;
		*/
	}

	return 0;
}
