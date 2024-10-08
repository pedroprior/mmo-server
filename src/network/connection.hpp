#pragma once

#include "common.hpp"
#include "message.hpp"
#include "tsqueue.hpp"

namespace Network {

template <typename T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
public:
  enum class Owner { Server, Client };

public:
  Connection(Owner parent, asio::io_context &asioContext,
             asio::ip::tcp::socket socket,
             ThreadSafeQueue<OwnedMessage<T>> &qIn)
      : m_asioContext(asioContext), m_socket(std::move(socket)),
        m_qMessagesIn(qIn) {
    m_nOwnerType = parent;
  }

  virtual ~Connection() {}

  uint32_t GetID() const { return id; }

public:
  void ConnectToClient(uint32_t uid = 0) {
    if (m_nOwnerType == Owner::Server) {
      if (m_socket.is_open()) {
        id = uid;
        ReadHeader();
      }
    }
  }

  void ConnectToServer(const asio::ip::tcp::resolver::results_type &endpoints) {

    if (m_nOwnerType == Owner::Client) {
      asio::async_connect(
          m_socket, endpoints,
          [this](std::error_code ec, asio::ip::tcp::endpoint endpoint) {
            if (!ec) {
              ReadHeader();
            }
          });
    }
  }

  void Disconnect() {
    if (IsConnected())
      asio::post(m_asioContext, [this]() { m_socket.close(); });
  }

  bool IsConnected() const { return m_socket.is_open(); }

  void StartListening() {}

public:
  void Send(const Message<T> &msg) {
    asio::post(m_asioContext, [this, msg]() {
      bool bWritingMessage = !m_qMessagesOut.empty();
      m_qMessagesOut.push_back(msg);
      if (!bWritingMessage) {
        WriteHeader();
      }
    });
  }

private:
  void WriteHeader() {

    asio::async_write(
        m_socket,
        asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
        [this](std::error_code ec, std::size_t length) {
          if (!ec) {
            if (m_qMessagesOut.front().body.size() > 0) {

              WriteBody();
            } else {
              m_qMessagesOut.pop_front();

              if (!m_qMessagesOut.empty()) {
                WriteHeader();
              }
            }
          } else {

            std::cout << "[" << id << "] Write Header Fail.\n";
            m_socket.close();
          }
        });
  }

  void WriteBody() {
    asio::async_write(m_socket,
                      asio::buffer(m_qMessagesOut.front().body.data(),
                                   m_qMessagesOut.front().body.size()),
                      [this](std::error_code ec, std::size_t length) {
                        if (!ec) {

                          m_qMessagesOut.pop_front();

                          if (!m_qMessagesOut.empty()) {
                            WriteHeader();
                          }
                        } else {

                          std::cout << "[" << id << "] Write Body Fail.\n";
                          m_socket.close();
                        }
                      });
  }

  void ReadHeader() {

    asio::async_read(
        m_socket,
        asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
        [this](std::error_code ec, std::size_t length) {
          if (!ec) {

            if (m_msgTemporaryIn.header.size > 0) {

              m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
              ReadBody();
            } else {

              AddToIncomingMessageQueue();
            }
          } else {

            std::cout << "[" << id << "] Read Header Fail.\n";
            m_socket.close();
          }
        });
  }

  void ReadBody() {
    asio::async_read(m_socket,
                     asio::buffer(m_msgTemporaryIn.body.data(),
                                  m_msgTemporaryIn.body.size()),
                     [this](std::error_code ec, std::size_t length) {
                       if (!ec) {

                         AddToIncomingMessageQueue();
                       } else {

                         std::cout << "[" << id << "] Read Body Fail.\n";
                         m_socket.close();
                       }
                     });
  }

  void AddToIncomingMessageQueue() {

    if (m_nOwnerType == Owner::Server)
      m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
    else
      m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});

    ReadHeader();
  }

protected:
  asio::ip::tcp::socket m_socket;
  asio::io_context &m_asioContext;

  ThreadSafeQueue<Message<T>> m_qMessagesOut;
  ThreadSafeQueue<OwnedMessage<T>> &m_qMessagesIn;

  Message<T> m_msgTemporaryIn;

  Owner m_nOwnerType = Owner::Server;

  uint32_t id = 0;
};

}; // namespace Network