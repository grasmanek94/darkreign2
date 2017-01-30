
#include "TCPSocket.h"


using namespace WONAPI;


Socket::Type TCPSocket::GetType()
{
	return stream;
}


unsigned long TCPSocket::Write(unsigned long count, const void* buffer)
{
	return Send(count, buffer);
}


unsigned long TCPSocket::Read(unsigned long count, void* buffer)
{
	return Recv(count, buffer);
}


class TCPAcceptCompletion : public Completion<const WSSocket::AcceptResult&>
{
public:
	TCPSocket* acceptSocket;

	CompletionContainer<const WSSocket::AcceptResult&> usersCompletion;

	TCPAcceptCompletion(CompletionContainer<const WSSocket::AcceptResult&> cmpl)
		: Completion<const WSSocket::AcceptResult&>(true), usersCompletion(cmpl)
	{}

	virtual void Complete(const WSSocket::AcceptResult& result);
};


void TCPAcceptCompletion::Complete(const WSSocket::AcceptResult& result)
{
	usersCompletion.Complete(result);
	if (!result.acceptedSocket)
		delete acceptSocket;
	Completion<const WSSocket::AcceptResult&>::Complete(result);
}


TCPSocket* TCPSocket::Accept(TCPSocket* socketToConnect, long timeout, bool async, const CompletionContainer<const AcceptResult&>& completion)
{
	if (socketToConnect)
		return (TCPSocket*)WSSocket::Accept(*socketToConnect, timeout, async, completion);

	TCPAcceptCompletion* acceptCompletion = new TCPAcceptCompletion(completion);
	if (acceptCompletion)
	{
		TCPSocket* acceptSocket = new TCPSocket();
		if (acceptSocket)
		{
			acceptCompletion->acceptSocket = acceptSocket;
			return (TCPSocket*)WSSocket::Accept(*acceptSocket, timeout, async, acceptCompletion);
		}
		delete acceptCompletion;
	}
	completion.Complete(AcceptResult(this, 0));
	return 0;
}
