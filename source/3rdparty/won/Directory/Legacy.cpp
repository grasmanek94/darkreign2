#include "DirectoryAPI.h"
#include "common/CriticalSection.h"


struct WONIPAddr
{
	unsigned short port;
	in_addr ip;
};


using namespace WONAPI;
using namespace WONMsg;
using namespace WONCommon;


static DirEntityList dirListResult;
static DirEntityList::iterator curDirListEntity;
CriticalSection crit;


extern "C" {


int WONDirInitDirServerQuery(const char** addrs, unsigned short numAddrs,
							 const char* dir, unsigned long timeout)	// in seconds, sug:5
{
	AutoCrit autoCrit(crit);
	int err;
	int i  = 0;
	if (numAddrs)
	{
		do
		{
			err = WONAPI::GetDirectoryContents(0, addrs[i], dir, &dirListResult, 0, 0, 5000);
			if (err == Error_Success)
				break;
		} while (++i < numAddrs);
	}

	curDirListEntity = dirListResult.begin();

	return 0;
}


int WONDirGetNextAddr(WONIPAddr* ipAddr)
{
	AutoCrit autoCrit(crit);
	if (curDirListEntity != dirListResult.end())
	{
		const unsigned char* addrData = (*curDirListEntity).mNetAddress.data();
		ipAddr->port = (*(addrData+1)) + (short)((*(addrData) << 8));
		ipAddr->ip = *(in_addr*)((short*)addrData + 1);

		curDirListEntity++;
	}
	else
	{
#ifdef WIN32
		ipAddr->ip.S_un.S_addr = 0;
#else//if defined(_LINUX)
		ipAddr->ip.s_addr = 0;
#endif
		ipAddr->port = 0;
	}

	return 0;
}


void WONDirAbortDirServerQuery()
{
	AutoCrit autoCrit(crit);
	curDirListEntity = dirListResult.end();
}


struct AddRenewData
{
	IPSocket::Address thisAddr;
	unsigned short timeout;
	std::string dir;
	IPSocket::Address* addrList;
	unsigned short numAddrs;
};


static void AddCompletion(const DirResult& result, AddRenewData* addRenewData)
{
	AutoCrit autoCrit(crit);
	delete[] addRenewData->addrList;
	delete addRenewData;
}


static void RenewCompletion(const DirResult& result, AddRenewData* addRenewData)
{
	AutoCrit autoCrit(crit);
	if (result.error == Error_Success)
	{
		delete[] addRenewData->addrList;
		delete addRenewData;
	}
	else
		WONAPI::AddServiceEx((Identity*)0, addRenewData->addrList, addRenewData->numAddrs, 0,
			addRenewData->dir, "", "", addRenewData->thisAddr,
			(unsigned long)addRenewData->timeout + 60, false, true,
			WONCommon::DataObjectTypeSet(), 10000, true, AddCompletion, addRenewData);
}


void WONDirAddOrRenewService(const char** dirAddrs, unsigned short numAddrs,
							 unsigned short gamePort, int timeout, const char* dir)
{
	if (!numAddrs)
		return;

	AutoCrit autoCrit(crit);
	AddRenewData* addRenewData = new AddRenewData;
	if (!addRenewData)
		return;

	addRenewData->addrList = new IPSocket::Address[numAddrs];
	if (!addRenewData->addrList)
	{
		delete addRenewData;
		return;
	}

	for (int i=0; i<numAddrs; i++)
		addRenewData->addrList[i].Set(dirAddrs[i]);
	addRenewData->numAddrs = numAddrs;
	addRenewData->timeout = timeout;
	addRenewData->thisAddr.InitFromLocalIP(gamePort);
	addRenewData->dir = dir;//"/DrBrain/";

	RenewServiceEx(0, addRenewData->addrList, numAddrs, 0, addRenewData->dir, "", addRenewData->thisAddr,
		addRenewData->timeout + 60, 10000, true, RenewCompletion, addRenewData);
}


};

