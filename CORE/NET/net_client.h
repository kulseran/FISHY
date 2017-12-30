/**
 * TCP/UDP Networking Client
 */
#ifndef FISHY_NET_CLIENT_H
#define FISHY_NET_CLIENT_H

#include "net_common.h"

#include <CORE/BASE/status.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/UTIL/noncopyable.h>

namespace core {
namespace net {

class iNetClient;

/**
 * Data handler called from {@link iNetClient#update}
 */
class iClientConnectionHandler {
  public:
  virtual ~iClientConnectionHandler() {}

  /**
   * Called to process any incomming data.
   *
   * @param pClient client to send a response to
   * @param data data to be processed. Caller owns the blob.
   */
  virtual void
  process(iNetClient &client, const core::memory::ConstBlob &data) = 0;

  /**
   * Called after all calls to process before {@link iNetClient#update} returns.
   * Allows the connection handler to clean out temporary data and compact
   * partially completed packets.
   */
  virtual void cleanup() = 0;
};

/**
 * Networking client
 */
class iNetClient : public util::noncopyable {
  public:
  virtual ~iNetClient(){};

  /**
   * Try to connect to a server.
   * Will block, independent of connection mode.
   *
   * @param server server definition
   * @return true if connected successfully
   */
  virtual Status start(const core::net::ClientDef &server) = 0;

  /**
   * Gracefully disconnect from the server.
   */
  virtual void stop() = 0;

  /**
   * Poll the connection for data.
   *
   * @param handler the {@link iClientConnectionHandler} that will recieve data.
   */
  virtual Status update(iClientConnectionHandler &handler) = 0;

  /**
   * @return true if the connection is alive and valid.
   */
  virtual bool valid() const = 0;

  /**
   * Send data to the server.
   *
   * @param msg data to send. Caller owns blob.
   * @return true if all the data was successfully sent
   */
  virtual Status send(const core::memory::ConstBlob &msg) = 0;

  /**
   * @return statistics about the connection
   */
  virtual NetStats stats() const = 0;

  /**
   * @return the connection mode (eg TCP or UDP)
   */
  virtual eConnectionMode::type getConnectionMode() const = 0;
};

/**
 * Networking client
 */
class NetClient : public iNetClient {
  public:
  /**
   * Create a client with {@code mode} type of network protocol.
   */
  NetClient(eConnectionMode::type mode);
  ~NetClient();

  virtual Status start(const core::net::ClientDef &server);
  virtual void stop();
  virtual Status update(iClientConnectionHandler &handler);
  virtual bool valid() const;
  virtual Status send(const memory::ConstBlob &msg);
  virtual NetStats stats() const { return m_stats; }
  virtual eConnectionMode::type getConnectionMode() const;

  protected:
  class Impl;
  Impl *m_pImpl;
  NetStats m_stats;
};

} // namespace net
} // namespace core

#endif
