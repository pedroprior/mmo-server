#include "network/common.hpp"
#include "network/network.hpp"

enum class CustomMsgTypes : uint32_t {
  ServerAccept,
  ServerDeny,
  ServerPing,
  MessageAll,
  ServerMessage,
};

class CustomClient : public Network::ClientInterface<CustomMsgTypes> {
public:
  void PingServer() {
    Network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerPing;

    std::chrono::system_clock::time_point timeNow =
        std::chrono::system_clock::now();

    msg << timeNow;
    Send(msg);
  }

  void MessageAll() {
    Network::Message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::MessageAll;
    Send(msg);
  }
};

int main() {

  CustomClient client;

  client.Connect("127.0.0.1", 60000);

  bool bQuit = false;

  while (!bQuit) {

    if (client.IsConnected()) {
      if (!client.Incoming().empty()) {
        auto msg = client.Incoming().pop_front().msg;

        switch (msg.header.id) {

        case CustomMsgTypes::ServerAccept:

        {
          std::cout << "Server Accepted Connection\n";
        }

        break;

        case CustomMsgTypes::ServerPing:

        {
          std::chrono::system_clock::time_point timeNow =
              std::chrono::system_clock::now();
          std::chrono::system_clock::time_point timeThen;
          msg >> timeThen;
          std::cout << "Ping: "
                    << std::chrono::duration<double>(timeNow - timeThen).count()
                    << "\n";
        } break;

        case CustomMsgTypes::ServerMessage:

        {
          uint32_t clientID;
          msg >> clientID;
          std::cout << "Hello from [" << clientID << "]\n";
        } break;

        case CustomMsgTypes::MessageAll:
        case CustomMsgTypes::ServerDeny:
          break;
        }
      } else {
        std::cout << "Server Down\n";
        bQuit = true;
      }
    }
  }

  return 0;
}