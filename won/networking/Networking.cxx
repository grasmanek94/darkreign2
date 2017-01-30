#include "Networking.hxx"

void WonNetworkClient::RunNetworking()
{
	ENetPacket* packet = nullptr;
	while (delayed_packets_to_send.try_pop(packet))
	{
		NetworkClient::Send(packet);
	}

	if (NetworkClient::Pull())
	{
		if (event.type != ENET_EVENT_TYPE_NONE)
		{
			received_events_to_process.push(event);
		}
	}
}

void WonNetworkClient::ProcessEvents(MessageReceiver* receiver)
{
	ENetEvent local_event;
	while (received_events_to_process.try_pop(local_event))
	{
		receiver->ProcessEvent(local_event);
	}
}

WonNetworkClient::WonNetworkClient()
{ }
