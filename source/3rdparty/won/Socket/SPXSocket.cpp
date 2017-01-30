#if !defined(macintosh) || (macintosh != 1)

#include "SPXSocket.h"


using namespace WONAPI;


Socket::Type SPXSocket::GetType()
{
	return stream;
}


unsigned long SPXSocket::Write(unsigned long count, const void* buffer)
{
	return Send(count, buffer);
}


unsigned long SPXSocket::Read(unsigned long count, void* buffer)
{
	return Recv(count, buffer);
}


class SPXAcceptCompletion : public Completion<const WSSocket::AcceptResult&>
{
public:
	SPXSocket* acceptSocket;

	CompletionContainer<const WSSocket::AcceptResult&> usersCompletion;

	SPXAcceptCompletion(CompletionContainer<const WSSocket::AcceptResult&> cmpl)
		: Completion<const WSSocket::AcceptResult&>(true), usersCompletion(cmpl)
	{}

	virtual void Complete(const WSSocket::AcceptResult& result);
};


void SPXAcceptCompletion::Complete(const WSSocket::AcceptResult& result)
{
	usersCompletion.Complete(result);
	if (!result.acceptedSocket)
		delete acceptSocket;
	Completion<const WSSocket::AcceptResult&>::Complete(result);
}


SPXSocket* SPXSocket::Accept(SPXSocket* socketToConnect, long timeout, bool async, const CompletionContainer<const AcceptResult&>& completion)
{
	if (socketToConnect)
		return (SPXSocket*)WSSocket::Accept(*socketToConnect, timeout, async, completion);

	SPXAcceptCompletion* acceptCompletion = new SPXAcceptCompletion(completion);
	if (acceptCompletion)
	{
		SPXSocket* acceptSocket = new SPXSocket();
		if (acceptSocket)
		{
			acceptCompletion->acceptSocket = acceptSocket;
			return (SPXSocket*)WSSocket::Accept(*acceptSocket, timeout, async, acceptCompletion);
		}
		delete acceptCompletion;
	}
	completion.Complete(AcceptResult(this, 0));
	return 0;
}


#endif
