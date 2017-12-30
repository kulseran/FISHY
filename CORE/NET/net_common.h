/**
 * Common functions and classes for the TCP connection clients.
 */
#ifndef FISHY_TCP_COMMON_H
#define FISHY_TCP_COMMON_H

#include <CORE/types.h>

#include <string>

namespace core {
namespace net {

/**
 * Called to init the network system.
 * Most OSs require some general setup before sockets can be opened.
 */
void Initialize();

/**
 * Check if net lib is initialized.
 */
bool IsInitialized();

/**
 * Called to shutdown the network system.
 */
void Shutdown();

/**
 * ConnectionId tags a user's connection for sending info back to them.
 */
typedef u32 tConnectionId;
static const tConnectionId INVALID_CONNECTION_ID = (tConnectionId) -1;

/**
 * Basic status structure used by TCP/UDP client/servers.
 */
struct NetStats {
  NetStats() : m_bytesSent(0), m_bytesRecieved(0), m_activeConnections(0) {}

  u64 m_bytesSent;
  u64 m_bytesRecieved;
  u32 m_activeConnections;
};

/**
 * Connection modes
 */
struct eConnectionMode {
  enum type { TCP_ASYNC, TCP_BLOCKING, UDP_ASYNC, UDP_BLOCKING, COUNT };
};

struct ServerDef {
  int m_port;
  int m_maxConnections;
  float m_connectionTimeoutSec;

  ServerDef() : m_port(0), m_maxConnections(0), m_connectionTimeoutSec(60) {}
  ServerDef(
      const int port,
      const int maxConnections,
      const float connectionTimeoutSec)
      : m_port(port),
        m_maxConnections(maxConnections),
        m_connectionTimeoutSec(connectionTimeoutSec) {}
};

struct ClientDef {
  std::string m_dnsName;
  int m_port;
  float m_connectionTimeoutSec;

  ClientDef() : m_port(0), m_connectionTimeoutSec(60) {}
  ClientDef(
      const std::string &dnsName,
      const int port,
      const float connectionTimeoutSec)
      : m_dnsName(dnsName),
        m_port(port),
        m_connectionTimeoutSec(connectionTimeoutSec) {}
};

static const size_t MAX_PACKET_SIZE = 8192;

} // namespace net
} // namespace core

#endif
