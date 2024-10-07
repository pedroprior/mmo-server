#include "network/network.hpp"
#include "network/common.hpp"

enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};

class CustomServer : public Network::ServerInterface<CustomMsgTypes> {
public:
  CustomServer(uint16_t nPort)
      : Network::ServerInterface<CustomMsgTypes>(nPort) {}

protected:
  virtual bool
  OnClientConnect(std::shared_ptr<Network::Connection<CustomMsgTypes>> client) {
    Network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerAccept;

    client->Send(msg);
    return true;
  }

  virtual void
  OnMessage(std::shared_ptr<Network::Connection<CustomMsgTypes>> client,
            Network::Message<CustomMsgTypes> &msg) {
    switch (msg.header.id) {

    case CustomMsgTypes::ServerAccept:
    case CustomMsgTypes::ServerDeny:
    case CustomMsgTypes::ServerPing: {
      std::cout << "[" << client->GetID() << "]: Server Ping\n";

      client->Send(msg);
    }

    break;

    case CustomMsgTypes::MessageAll:

    {
        std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			Network::Message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);
    }

    break;

    case CustomMsgTypes::ServerMessage:
      break;
    }
  };
};


int main() {

    CustomServer server(60000);
    server.Start();

    while(1) {
        server.Update(-1, true);
    }

    return 0;
}