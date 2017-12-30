/**
 * mock_net_server.hpp
 *
 * Implementation of {@link iNetServer} which can be used in tests.
 */
#ifndef FISHY_MOCK_NET_SERVER_HPP
#define FISHY_MOCK_NET_SERVER_HPP

#ifndef TESTING
#error may only be included in tests
#endif

#include <CORE/BASE/asserts.h>
#include <CORE/NET/net_server.h>

#include <deque>
#include <map>

namespace testing {
namespace core {
namespace net {

/**
 * Mock {@link iNetServer} that contains several capture and error triggering
 * features for use in test cases.
 */
class MockNetServer : public ::core::net::iNetServer {
  public:
    MockNetServer(::core::net::eConnectionMode::type mode)
      : m_connected(true),
        m_sendable(true),
        m_mode(mode) {
    }

    /**
     * Provide test connections for this server to respond to.
     * @see update
     */
    void simulateConnect(::core::net::tConnectionId connectionId) {
      m_incomingConnections.push_back(connectionId);
    }

    /**
     * Cause the client to fail subsequent calls to {@link #start}
     */
    void simulateConnectFailure() {
      m_connected = false;
    }

    /**
     * @see #simulateConnectFailure
     */
    virtual bool start(const ::core::net::ServerDef &server) {
      return m_connected;
    }

    /**
     *
     */
    virtual void stop() {
    }

    /**
     * Provide test data for this server to return during the next {@link #update}.
     * @see simulateConnect
     */
    void simulateRecieve(const ::core::net::tConnectionId connectionId, const std::vector<std::string> &data) {      
      std::copy(data.begin(), data.end(), std::back_inserter(m_recieved[connectionId]));
    }

    /**
     * @see #simulateRecieve
     */
    virtual void update(::core::net::iServerConnectionHandler &handler) {
      for (std::vector<::core::net::tConnectionId>::const_iterator itr = m_incomingConnections.begin(); itr != m_incomingConnections.end(); ++itr) {
        handler.open(*itr);
      }

      if (m_recieved.empty()) {
        return;
      }
      for (std::map<::core::net::tConnectionId, std::deque<std::string>>::iterator itr = m_recieved.begin(); itr != m_recieved.end(); ++itr) {
        const std::string msg = itr->second.front();
        itr->second.pop_front();

        handler.process(*this, itr->first, ::core::memory::ConstBlob((u8*) msg.c_str(), msg.length()));
      }
    }

    /**
     * Cause subsequent calls to {@link #send} to fail.
     */
    void simulateSendFailure() {
      m_sendable = false;
    }

    /**
     * Attempt to send data by storing it in a local buffer.
     * @see #getSent
     * @see #simulateSendFailure
     */
    virtual bool send(const ::core::net::tConnectionId connectionId, const ::core::memory::ConstBlob &msg) {
      m_sent.reserve(m_sent.size() + msg.size());
      std::copy(msg.data(), msg.data() + msg.size(), std::back_inserter(m_sent));
      bool wasSent = m_sendable;
      m_sendable = true;
      return wasSent;
    }

    /**
     * @see #send
     */
    const std::string &getSent() const {
      return m_sent;
    }

    /**
     *
     */
    virtual bool valid() const {
      return m_connected && !m_recieved.empty();
    }
    
    /**
     *
     */
    virtual ::core::net::NetStats stats() const { 
      return ::core::net::NetStats();
    }

    /**
     *
     */
    virtual ::core::net::eConnectionMode::type getConnectionMode() const {
      return m_mode;
    }

  private:
    bool m_connected;
    bool m_sendable;
    std::string m_sent;
    std::map<::core::net::tConnectionId, std::deque<std::string>> m_recieved;
    std::vector<::core::net::tConnectionId> m_incomingConnections;
    ::core::net::eConnectionMode::type m_mode;
};

} // namespace net
} // namespace core
} // namespace testing

#endif