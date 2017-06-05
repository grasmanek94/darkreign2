#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <set>
#include <sstream>
#include <exception>
#include <algorithm>
#include <iterator>

#include <Networking/Networking.hxx>

class Server : public MessageReceiver
{
private:
	WonNetworkServer connection;

	// Handlers:
	void Handle(ENetPeer* peer, const std::shared_ptr<EventConnect>& data) override
	{
		//connection.Send(peer, GameSetup(id));
		std::cout << "Peer connected: " << peer->address.host << ":" << peer->address.port << " with ID: " << reinterpret_cast<size_t>(peer->data) << std::endl;
	}

	void Handle(ENetPeer* peer, const std::shared_ptr<EventDisconnect>& data) override
	{
		//connection.Broadcast(player_quit, peer);

		std::cout << "Peer disconnected: " << peer->address.host << ":" << peer->address.port << " with ID: " << reinterpret_cast<size_t>(peer->data) << std::endl;
	}

	void Handle(ENetPeer* peer, const std::shared_ptr<ChatMessage>& message) override
	{
		//Player* player = Link::GetPlayer(peer);

		//message->SetSender(player->id);

		std::wcout << "[" << message->GetSender() << "]: " << message->GetContents() << std::endl;

		connection.Broadcast(message);
	}

public:
	Server(const std::string bind_address, uint16_t port, size_t max_players)
	{
		int init_code = connection.GetInitCode();

		if (init_code)
		{
			// TODO custom exception class
			throw std::exception(
#ifdef _WIN32
			("Cannot initialize ENET, error code: " + std::to_string(init_code)).c_str()
#endif
			);
		}

		connection.SetHost(bind_address, port);

		if (!connection.Create(max_players) || !connection.Good())
		{
#ifdef _WIN32
			// TODO custom exception class
			throw std::exception("ENET host member creation failed");
#endif
		}

		//peers.resize(max_players);
		//for (size_t i = 0; i < max_players; ++i)
		//{
		//	peers[i].GetPlayer()->id = i;
		//}
		//ready
	}

	void Tick()
	{
		if (connection.Pull())
		{
			ProcessEvent(connection.Event());
		}
	}

	~Server()
	{

	}
};

std::unique_ptr<Server> server;
int main()
{
	server = std::make_unique<Server>("0.0.0.0", 5544, 32);
	while (true)
	{
		server->Tick();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}
