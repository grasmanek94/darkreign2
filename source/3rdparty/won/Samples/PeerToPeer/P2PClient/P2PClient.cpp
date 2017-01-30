#ifdef DLLSAMPLE
#include <wondll.h>
#else
#include <Authentication/AuthSocket.h>

namespace {
	using namespace WONAPI;
};
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
	
	// prepare for peer to peer
	PeerAuthClientState aPeerAuthClientState;
	unsigned short aRequestBufLen = 0;
	const void* aRequestBufP = NULL;

	// create Auth1Request
	Error aError = aPeerAuthClientState.Init(gIdentity, &aRequestBufP, &aRequestBufLen);
	printf("PeerAuthClientState::Init: %s.\n", WONErrorToString(aError));
	if (aError != Error_Success) return aError;

	// connect to server
	IPSocket::Address aServerAddress("localhost", SERVER_PORT);
	TCPSocket aServerCommSocket;
	aError = aServerCommSocket.Open(aServerAddress);
	printf("Connecting to server at %s: %s.\n", aServerAddress.GetAddressString().c_str(), WONErrorToString(aError));
	if (aError != Error_Success) return aError;

	// send Auth1Request
	aServerCommSocket.Send(2, &aRequestBufLen);
	aServerCommSocket.Send(aRequestBufLen, aRequestBufP);
	printf("Sent request.\n");

	// receive server response (Auth1Challenge1 or Auth1Complete)
	unsigned short aChallenge1BufLen = 0;
	aServerCommSocket.Recv(2, &aChallenge1BufLen);
	void* aChallenge1BufP = new unsigned char[aChallenge1BufLen];
	aServerCommSocket.Recv(aChallenge1BufLen, aChallenge1BufP);
	printf("Received server->client challenge.\n");

	// process server response:
	//  if Auth1Complete, we're done
	//  if Auth1Challenge1, generate Auth1Challenge2
	unsigned short aChallenge2BufLen = 0;
	const void* aChallenge2BufP = NULL;
	aPeerAuthClientState.RecvChallenge1(aChallenge1BufP, aChallenge1BufLen,
	                                    &aChallenge2BufP, &aChallenge2BufLen);
	printf("PeerAuthClientState::RecvChallenge1: %s.\n", WONErrorToString(aError));
	if (aError != Error_Success) return aError;

	// send Auth1Challenge2
	aServerCommSocket.Send(2, &aChallenge2BufLen);
	aServerCommSocket.Send(aChallenge2BufLen, aChallenge2BufP);
	printf("Sent client->server challenge.\n");

	// receive Auth1Complete
	unsigned short aResultBufLen = 0;
	aServerCommSocket.Recv(2, &aResultBufLen);
	void* aResultBufP = new unsigned char[aResultBufLen];
	aServerCommSocket.Recv(aResultBufLen, aResultBufP);	
	printf("Received result.\n");

	// process Auth1Complete
	aError = aPeerAuthClientState.RecvResult(aResultBufP, aResultBufLen);
	printf("PeerAuthClientState::RecvResult: %s.\n", WONErrorToString(aError));
	return aError;
}