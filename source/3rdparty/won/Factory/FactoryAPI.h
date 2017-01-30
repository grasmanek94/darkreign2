#ifndef __WON_FACTORY_API_H__
#define __WON_FACTORY_API_H__


#include "Socket/IPSocket.h"
#include <list>
#include "Errors.h"
#include "SDKCommon/Completion.h"
#include "common/WONString.h"
#include "Authentication/Identity.h"


namespace WONAPI {


struct StartTitanServerResult
{
	Error error;
	IPSocket::Address factory;
	std::list<unsigned short> processPorts;

	StartTitanServerResult(Error err, const IPSocket::Address& addr)
		:	error(err),
			factory(addr)
	{ }
};


struct StopTitanServerResult
{
	Error error;
	IPSocket::Address factory;
	unsigned short processPort;

	StopTitanServerResult(Error err, const IPSocket::Address& addr, unsigned short port)
		:	error(err),
			factory(addr),
			processPort(port)
	{ }
};


// StartTitanServer


// -- With fail-over factory address list, directory server fields

Error StartTitanServer(Identity* identity, const IPSocket::Address* factories,
					   unsigned int numFactories, IPSocket::Address* startedOnFactory,
					   const string& configurationName, const IPSocket::Address& directoryServer,
					   const WONCommon::WONString& dirPath, const WONCommon::WONString& displayName,
					   bool abortOnDirRegisterFailure, unsigned short* startedOnPorts,
					   unsigned short* numStartedPorts, const WONCommon::WONString& commandLineFragment,
					   bool replaceCommandLine,
					   unsigned char numPortsRequested, unsigned char numSpecificPorts,
					   unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs ,
					   unsigned short numAuthorizedIPs, long timeout, bool async,
					   const CompletionContainer<const StartTitanServerResult&>& completion);


inline Error StartTitanServer(Identity* identity, const IPSocket::Address* factories,
					   unsigned int numFactories, IPSocket::Address* startedOnFactory,
					   const string& configurationName, const IPSocket::Address& directoryServer,
					   const WONCommon::WONString& dirPath, const WONCommon::WONString& displayName,
					   bool abortOnDirRegisterFailure = true, unsigned short* startedOnPorts = 0,
					   unsigned short* numStartedPorts = 0,
					   const WONCommon::WONString& commandLineFragment = WONCommon::WONString(""),
					   bool replaceCommandLine = false,
					   unsigned char numPortsRequested = 1, unsigned char numSpecificPorts = 0,
					   unsigned short* specificPortsArray = 0, const IPSocket::Address* authorizedIPs = 0,
					   unsigned short numAuthorizedIPs = 0, long timeout = -1, bool async = false)
{ return StartTitanServer(identity, factories, numFactories, startedOnFactory, configurationName, directoryServer, dirPath, displayName, abortOnDirRegisterFailure, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error StartTitanServerEx(Identity* identity, const IPSocket::Address* factories, unsigned int numFactories, IPSocket::Address* startedOnFactory, const string& configurationName, const IPSocket::Address& directoryServer, const WONCommon::WONString& dirPath, const WONCommon::WONString& displayName, bool abortOnDirRegisterFailure,  unsigned short* startedOnPorts, unsigned short* numStartedPorts, const WONCommon::WONString& commandLineFragment, bool replaceCommandLine, unsigned char numPortsRequested, unsigned char numSpecificPorts, unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs, unsigned short numAuthorizedIPs, long timeout, bool async, void (*f)(const StartTitanServerResult&, privsType), privsType t)
{ return StartTitanServer(identity, factories, numFactories, startedOnFactory, configurationName, directoryServer, dirPath, displayName, abortOnDirRegisterFailure, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, new CompletionWithContents<const StartTitanServerResult&, privsType>(t, f, true)); }


// -- With fail-over factory address list, no directory server fields

inline Error StartTitanServer(Identity* identity, const IPSocket::Address* factories,
							  unsigned int numFactories, IPSocket::Address* startedOnFactory,
							  const string& configurationName, unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							  const WONCommon::WONString& commandLineFragment, bool replaceCommandLine = false,
							  unsigned char numPortsRequested = 1, unsigned char numSpecificPorts = 0,
							  unsigned short* specificPortsArray = 0, const IPSocket::Address* authorizedIPs = 0,
							  unsigned short numAuthorizedIPs = 0, long timeout = -1, bool async = false,
							  const CompletionContainer<const StartTitanServerResult&>& completion = DEFAULT_COMPLETION )
{ return StartTitanServer(identity, factories, numFactories, startedOnFactory, configurationName, IPSocket::Address(), WONCommon::WONString(), WONCommon::WONString(), false, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, completion); }


template <class privsType>
inline Error StartTitanServerEx(Identity* identity, const IPSocket::Address* factories, unsigned int numFactories, IPSocket::Address* startedOnFactory, const string& configurationName, unsigned short* startedOnPorts, unsigned short* numStartedPorts, const WONCommon::WONString& commandLineFragment, bool replaceCommandLine, unsigned char numPortsRequested, unsigned char numSpecificPorts, unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs, unsigned short numAuthorizedIPs, long timeout, bool async, void (*f)(const StartTitanServerResult&, privsType), privsType t)
{ return StartTitanServer(identity, factories, numFactories, startedOnFactory, configurationName, IPSocket::Address(), WONCommon::WONString(), WONCommon::WONString(), false, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, new CompletionWithContents<const StartTitanServerResult&, privsType>(t, f, true)); }



// -- With only 1 factory server address, no directory server fields

inline Error StartTitanServer(Identity* identity, const IPSocket::Address& factory,
							  const string& configurationName,  unsigned short* startedOnPorts, unsigned short* numStartedPorts,
							  const WONCommon::WONString& commandLineFragment, bool replaceCommandLine = false,
							  unsigned char numPortsRequested = 1, unsigned char numSpecificPorts = 0,
							  unsigned short* specificPortsArray = 0, const IPSocket::Address* authorizedIPs = 0,
							  unsigned short numAuthorizedIPs = 0, long timeout = -1, bool async = false,
							  const CompletionContainer<const StartTitanServerResult&>& completion = DEFAULT_COMPLETION )
{ return StartTitanServer(identity, &factory, 1, 0, configurationName, IPSocket::Address(), WONCommon::WONString(), WONCommon::WONString(), false, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, completion); }


template <class privsType>
inline Error StartTitanServerEx(Identity* identity, const IPSocket::Address& factory, const string& configurationName, unsigned short* startedOnPorts, unsigned short* numStartedPorts, const WONCommon::WONString& commandLineFragment, bool replaceCommandLine, unsigned char numPortsRequested, unsigned char numSpecificPorts, unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs, unsigned short numAuthorizedIPs, long timeout, bool async, void (*f)(const StartTitanServerResult&, privsType), privsType t)
{ return StartTitanServer(identity, &factory, 1, 0, configurationName, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, new CompletionWithContents<const StartTitanServerResult&, privsType>(t, f, true)); }



// -- With only 1 factory server address, directory server fields

inline Error StartTitanServer(Identity* identity, const IPSocket::Address& factory,
							  const string& configurationName, const IPSocket::Address& directoryServer,
							  const WONCommon::WONString& dirPath, const WONCommon::WONString& displayName,
							  bool abortOnDirRegisterFailure = true,  unsigned short* startedOnPorts = 0,
							  unsigned short* numStartedPorts = 0, const WONCommon::WONString& commandLineFragment = WONCommon::WONString(""),
							  bool replaceCommandLine = false,
							  unsigned char numPortsRequested = 1, unsigned char numSpecificPorts = 0,
							  unsigned short* specificPortsArray = 0, const IPSocket::Address* authorizedIPs = 0,
							  unsigned short numAuthorizedIPs = 0, long timeout = -1, bool async = false,
							  const CompletionContainer<const StartTitanServerResult&>& completion = DEFAULT_COMPLETION )
{ return StartTitanServer(identity, &factory, 1, 0, configurationName, directoryServer, dirPath, displayName, abortOnDirRegisterFailure, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, completion); }


template <class privsType>
inline Error StartTitanServerEx(Identity* identity, const IPSocket::Address& factory, const string& configurationName, const IPSocket::Address& directoryServer, const WONCommon::WONString& dirPath, const WONCommon::WONString& displayName, bool abortOnDirRegisterFailure, unsigned short* startedOnPorts, unsigned short* numStartedPorts, const WONCommon::WONString& commandLineFragment, bool replaceCommandLine, unsigned char numPortsRequested, unsigned char numSpecificPorts, unsigned short* specificPortsArray, const IPSocket::Address* authorizedIPs, unsigned short numAuthorizedIPs, long timeout, bool async, void (*f)(const StartTitanServerResult&, privsType), privsType t)
{ return StartTitanServer(identity, &factory, 1, 0, configurationName, directoryServer, dirPath, displayName, abortOnDirRegisterFailure, startedOnPorts, numStartedPorts, commandLineFragment, replaceCommandLine, numPortsRequested, numSpecificPorts, specificPortsArray, authorizedIPs, numAuthorizedIPs, timeout, async, new CompletionWithContents<const StartTitanServerResult&, privsType>(t, f, true)); }


// StopTitanServer

Error StopTitanServer(Identity* identity, const IPSocket::Address& factory, const string& configurationName,
					  unsigned short processPort, long timeout, bool async,
					  const CompletionContainer<const StopTitanServerResult&>& completion );

inline Error StopTitanServer(Identity* identity, const IPSocket::Address& factory, const string& configurationName,
					  unsigned short processPort, long timeout = -1, bool async = false)
{ return StopTitanServer(identity, factory, configurationName, processPort, timeout, async, DEFAULT_COMPLETION); }


template <class privsType>
inline Error StopTitanServerEx(Identity* identity, const IPSocket::Address& factory, const string& configurationName, unsigned short processPort, long timeout, bool async, void (*f)(const StopTitanServerResult&, privsType), privsType t)
{ return StopTitanServer(identity, factory, configurationName, processPort, timeout, async, new CompletionWithContents<const StopTitanServerResult&, privsType>(t, f, true)); }


};


#endif
