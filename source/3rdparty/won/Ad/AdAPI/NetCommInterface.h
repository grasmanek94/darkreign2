#ifndef _NETCOMMINTERFACE_H
#define _NETCOMMINTERFACE_H

#ifdef ADAPI_EXPORTS
#define ADAPI_API __declspec(dllexport)
#else
#define ADAPI_API __declspec(dllimport)
#endif

namespace WONAPI
{

// Base class of the Network communications layer that needs to be filled in by clients
class ADAPI_API NetCommInterface
{
public:
	typedef void (__stdcall *NetCommCallback)(bool theOperationResult, void *theNetCommDataP);

	NetCommInterface(){};
	virtual ~NetCommInterface(){};
	
	virtual bool SendHTTPRequest(char* theRequest, char* theIpAddress, NetCommCallback theNetCommCallbackP, void* theCallbackDataP) = 0;
	virtual bool HTTPGet(const char* theServer, const char* theServerPath, const char* theDestPath, NetCommCallback theNetCommCallbackP, void* theCallbackDataP) = 0;

};

}  // WONAPI

#endif // _NETCOMMINTERFACE_H