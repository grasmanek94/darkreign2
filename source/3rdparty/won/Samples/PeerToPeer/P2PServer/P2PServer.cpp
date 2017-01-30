#ifdef DLLSAMPLE
#include <wondll.h>
#else
#include <Authentication/AuthSocket.h>
using namespace WONAPI;
#endif // DLLSAMPLE
#include "../PeerToPeer.h"

const long gRequestTimeout = -1;
#ifdef DLLSAMPLE
extern HWONAUTH gAuthH;
#else
extern Identity gIdentity;
#endif // DLLSAMPLE

int main(int argc, char** argv)
{
	// use non-default port if provided
	if (argc > 1)
		*(const_cast<unsigned short*>(&SERVER_PORT)) = atoi(argv[1]);
	
	if (!Init())
	{
		printf("Failed to initialize.\n");
		return 1;
	}

	// listen for connections
	TCPSocket aSocket;
	aSocket.Listen(SERVER_PORT);
	printf("Waiting for connections on port %d.\n", SERVER_PORT);

	while (TRUE)
	{
		// accept the next connection
		TCPSocket* aClientCommSocketP = aSocket.Accept();
		printf("Accepted connection from %s.\n", aClientCommSocketP->GetRemoteAddress().GetAddressString().c_str());

		// recieve client's Auth1Request
		unsigned short aRequestBufLen = 0;
		aClientCommSocketP->Recv(2, &aRequestBufLen);
		void* aRequestBufP = new unsigned char[aRequestBufLen];
		aClientCommSocketP->Recv(aRequestBufLen, aRequestBufP);	
		printf("Received request.\n");

		// process Auth1Request and generate response (Auth1Challenge1 or Auth1Complete)
		PeerAuthServerState aPeerAuthServerState;
		unsigned short aChallenge1BufLen = 0;
		const void* aChallenge1BufP = NULL;
		Error aError = aPeerAuthServerState.RecvAuth1Request(gIdentity, aRequestBufP, aRequestBufLen, &aChallenge1BufP, &aChallenge1BufLen);
		printf("PeerAuthServerState::RecvAuth1Request: %s.\n", WONErrorToString(aError));
		if (aError != Error_Success) break;

		// send response to Auth1Request
		aClientCommSocketP->Send(2, &aChallenge1BufLen);
		aClientCommSocketP->Send(aChallenge1BufLen, aChallenge1BufP);
		printf("Sent server->client challenge\n");

		// receive Auth1Challenge2
		unsigned short aChallenge2BufLen = 0;
		aClientCommSocketP->Recv(2, &aChallenge2BufLen);
		void* aChallenge2BufP = new unsigned char[aChallenge2BufLen];
		aClientCommSocketP->Recv(aChallenge2BufLen, aChallenge2BufP);
		printf("Received client->server challenge\n");

		// process Auth1Challenge2 and generate Auth1Complete
		unsigned short aResultBufLen = 0;
		const void* aResultBufP = NULL;
		aError = aPeerAuthServerState.RecvChallenge2(aChallenge2BufP, aChallenge2BufLen, &aResultBufP, &aResultBufLen);
		printf("PeerAuthServerState::RecvChallenge2: %s.\n", WONErrorToString(aError));
		if (aError != Error_Success) break;

		// send Auth1Complete
		aClientCommSocketP->Send(2, &aResultBufLen);
		aClientCommSocketP->Send(aResultBufLen, aResultBufP);
		printf("Sent result.\n");
	}

#ifndef DLLSAMPLE
	StopListeningForPeerAuthentication();
#endif // !DLLSAMPLE

	return 0;
}