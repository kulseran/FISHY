/**
 * Network server
 */
#ifndef FISHY_NET_SERVER_H
#define FISHY_NET_SERVER_H

#include "net_common.h"

#include <CORE/BASE/status.h>
#include <CORE/MEMORY/blob.h>
#include <CORE/UTIL/noncopyable.h>

namespace core {
namespace net {

class iNetServer;

/**
 * Data handler called from {@link iNetServer#update}
 */
class iServerConnectionHandler {
  public:
  virtual ~iServerConnectionHandler() {}

  /**
   * Called to process incomming data from a single client.
   *
   * @param server server to send responses with.
   * @param connectionId identifier for the connection who sent the data.
   * @param data data to be processed. Caller owns the blob.
   */
  virtual bool process(
      iNetServer &server,
      const tConnectionId connectionId,
      const core::memory::ConstBlob &data) = 0;

  /**
   * Called when a new client connects
   *
   * @param connectionId the identifier for the new client
   */
  virtual void open(const tConnectionId connectionId) = 0;

  /**
   * Called when an existing client disconnects
   *
   * @param connectionId the identifier for the old client
   */
  virtual void close(const tConnectionId connectionId) = 0;

  /**
   * Called after all calls to process before {@link iNetServer#update} returns.
   * Allows the connection handler to clean out temporary data and compact
   * partially completed packets.
   */
  virtual void cleanup() = 0;
};

/**
 * Networking server interface
 */
class iNetServer : util::noncopyable {
  public:
  virtual ~iNetServer(){};

  /**
   * Try to start a server.
   */
  virtual Status start(const core::net::ServerDef &def) = 0;

  /**
   * Gracefully disconnect all clients and shut down.
   */
  virtual void stop() = 0;

  /**
   * Poll for client connection data.
   *
   * @param handler the {@link iServerConnectionHandler} that will recieve data.
   */
  virtual Status update(iServerConnectionHandler &handler) = 0;

  /**
   * @return true if the server is alive and valid
   */
  virtual bool valid() const = 0;

  /**
   * Send data to a specific client
   *
   * @param connectionId active connection to send data to
   * @param msg data to send. Caller owns blob.
   */
  virtual Status send(
      const tConnectionId connectionId, const core::memory::ConstBlob &msg) = 0;

  /**
   * @return statistics about the connection
   */
  virtual NetStats stats() const = 0;

  /**
   * @return the mode of this server (eg. TCP vs UDP)
   */
  virtual eConnectionMode::type getConnectionMode() const = 0;
};

/**
 * Networking server
 */
class NetServer : public iNetServer {
  public:
  /**
   * Create a server with {@code mode} type of network protocol.
   */
  NetServer(eConnectionMode::type mode);
  ~NetServer();

  virtual Status start(const core::net::ServerDef &def);
  virtual void stop();
  virtual Status update(iServerConnectionHandler &handler);
  virtual bool valid() const;
  virtual Status
  send(const tConnectionId connectionId, const core::memory::ConstBlob &msg);
  virtual eConnectionMode::type getConnectionMode() const;

  virtual NetStats stats() const { return m_stats; }

  private:
  class Impl;
  Impl *m_pImpl;
  NetStats m_stats;
};

} // namespace net
} // namespace core

#endif
