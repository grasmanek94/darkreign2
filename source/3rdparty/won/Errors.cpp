#include "wondll.h"
#include "Errors.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace WONAPI;


WON_CONST char* WONErrorToString(long errorVal)
{
	switch (errorVal)
	{
	case Error_Success:
		return "Success";
	case Error_OutOfMemory:
		return "Out of memory";
	case Error_GeneralFailure:
		return "General failure";
	case Error_InvalidState:
		return "Invalid state";
	case Error_NetFailed:
		return "Net failed";
	case Error_NetResourceUnavailable:
		return "Net resource unavailable";
	case Error_BadAddress:
		return "Bad address";
	case Error_AddressInUse:
		return "Address in use";
	case Error_HostUnreachable:
		return "Host unreachable";
	case Error_Timeout:
		return "Timeout";
	case Error_Pending:
		return "Pending";
	case Error_ConnectionClosed:
		return "Connection closed";
	case Error_InvalidMessage:
		return "Invalid message";
	case Error_Winsock2Required:
		return "Winsock 2 required";
	case Error_InvalidParams:
		return "Invalid Parameters";
	case Error_InvalidVerifierKey:
		return "Invalid verifier key";
	case Error_GetPubKeysFailed:
		return "Failed to get public key block";
	case Error_InvalidPubKeys:
		return "Invalid public key block";
	case Error_FailedToGenerateKey:
		return "Failed to generate key";
	case Error_InvalidAuthLoginReply:
		return "Invalid login reply";
	case Error_InvalidAuthLogin:
		return "Invalid login";
	case Error_BadNewPassword:
		return "Bad new password";
	case Error_InvalidCDKey:
		return "Invalid CD key";
	case Error_ExpiredKey:
		return "Expired key";
	case Error_VerifyFailed:
		return "Verify failed";
	case Error_LockedOut:
		return "Locked out";
	case Error_KeyInUse:
		return "Key in use";
	case Error_CRCFailed:
		return "CRC failed";
	case Error_UserExists:
		return "User exists";
	case Error_UserNotFound:
		return "User not found";
	case Error_BadPassword:
		return "Bad password";
	case Error_DirtyWord:
		return "Dirty word";
	case Error_BadCommunity:
		return "Bad Community";
	case Error_NotInCommunity:
		return "Not in Community";
	case Error_InvalidSessionKey:
		return "Invalid session key";
	case Error_InvalidCertificate:
		return "Invalid certificate";
	case Error_InvalidRemoteCertificate:
		return "Invalid remote certificate";
	case Error_InvalidPrivateKey:
		return "Invalid private key";
	case Error_EncryptFailed:
		return "Encrypt failed";
	case Error_InvalidSecret:
		return "Invalid secret exchange";
	case Error_Failure:
		return "Failure";
	case Error_InvalidParameters:
		return "Invalid parameters sent to server";
	case Error_Unimplemented:
		return "Feature not implemented";
	case Error_Aborted:
		return "Operation aborted";
	case Error_UnableToOpenFile:
		return "Unable to open file";
	case Error_UnableToWriteToFile:
		return "Unable to write to file";
	case Error_NotSupported:			
		return "Not supported";
	default:
		{
			char tmp[50];
			sprintf(tmp, "%ld", errorVal);
			static std::string s;
			s = std::string(tmp) + " <Error is undefined or is a server status>";
			return (WON_CONST char*)s.c_str();
		}
	};
}
