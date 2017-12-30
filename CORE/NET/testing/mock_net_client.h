/**
 * Implementation of {@link iNetClient} which can be used in tests.
 */
#ifndef FISHY_MOCK_NET_CLIENT_H
#define FISHY_MOCK_NET_CLIENT_H

#ifndef TESTING
#  error may only be included in tests
#endif

#include <CORE/BASE/asserts.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/NET/net_client.h>

#include <deque>

namespace testing {
namespace core {
namespace net {

/**
 * Mock {@link iNetClient} that contains several capture and error triggering
 * features for use in test cases.
 */
class MockNetClient : public ::core::net::iNetClient {
  public:
  MockNetClient(::core::net::eConnectionMode::type mode)
      : m_connected(true), m_sendable(true), m_mode(mode) {}

  /**
   * Cause the client to fail subsequent calls to {@link #start}
   */
  void simulateConnectFailure() { m_connected = false; }

  /**
   * @see #simulateConnectFailure
   */
  virtual bool start(const ::core::net::ClientDef &server) {
    return m_connected;
  }

  /**
   *
   */
  virtual void stop() {}

  /**
   * Provide test data for this client to return during the next {@link
   * #update}.
   */
  void simulateRecieve(const std::vector< std::string > &data) {
    std::copy(data.begin(), data.end(), std::back_inserter(m_recieved));
  }

  /**
   * @see #simulateRecieve
   */
  virtual void update(::core::net::iClientConnectionHandler &handler) {
    if (m_recieved.empty()) {
      return;
    }
    const std::string msg = m_recieved.front();
    m_recieved.pop_front();

    handler.process(
        *this, ::core::memory::ConstBlob((u8 *) msg.c_str(), msg.length()));
  }

  /**
   * Cause subsequent calls to {@link #send} to fail.
   */
  void simulateSendFailure() { m_sendable = false; }

  /**
   * Attempt to send data by storing it in a local buffer.
   * @see #getSent
   * @see #simulateSendFailure
   */
  virtual bool send(const ::core::memory::ConstBlob &msg) {
    m_sent.reserve(m_sent.size() + msg.size());
    std::copy(msg.data(), msg.data() + msg.size(), std::back_inserter(m_sent));
    bool wasSent = m_sendable;
    m_sendable = true;
    return wasSent;
  }

  /**
   * @see #send
   */
  const std::string &getSent() const { return m_sent; }

  /**
   *
   */
  virtual bool valid() const { return m_connected && !m_recieved.empty(); }

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
  std::deque< std::string > m_recieved;
  ::core::net::eConnectionMode::type m_mode;
};

} // namespace net
} // namespace core
} // namespace testing

#endif
