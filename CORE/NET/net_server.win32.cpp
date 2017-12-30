#include "net_server.h"

#if defined(PLAT_WIN32)

#  include <CORE/ARCH/timer.h>
#  include <CORE/BASE/checks.h>
#  include <CORE/BASE/logging.h>
#  include <CORE/UTIL/algorithm.h>
#  include <CORE/UTIL/lexical_cast.h>

#  include <WS2tcpip.h>
#  include <WinSock2.h>

#  include <map>

using core::memory::Blob;
using core::memory::ConstBlob;

namespace core {
namespace net {

static const int MAX_PENDING_CONNECTIONS = 10;

/**
 * Implementation of the {@link NetServer} logic for PLAT_WIN32
 */
class NetServer::Impl {
  public:
  /**
   * Container for information about a possibly connected client.
   */
  struct Connection {
    Connection()
        : m_id(INVALID_CONNECTION_ID),
          m_lastModified(0),
          m_socket(INVALID_SOCKET),
          m_clientAddr() {}

    Connection(
        tConnectionId id,
        u64 lastModified,
        SOCKET socket,
        const std::string &hostName,
        sockaddr_storage clientAddr)
        : m_id(id),
          m_lastModified(lastModified),
          m_socket(socket),
          m_hostName(hostName),
          m_clientAddr(clientAddr){};

    bool operator==(const tConnectionId id) { return m_id == id; }

    tConnectionId m_id;
    u64 m_lastModified;
    SOCKET m_socket;
    std::string m_hostName;
    sockaddr_storage m_clientAddr;
  };

  /**
   *
   */
  Impl(eConnectionMode::type mode)
      : m_mode(mode),
        m_socket(INVALID_SOCKET),
        m_pThis(NULL),
        m_connectionIdNext(1) {}

  ~Impl() { ASSERT(m_socket == INVALID_SOCKET); }

  /**
   *
   */
  Status start(
      NetServer *pThis,
      const u32 port,
      const u32 maxConnections,
      const f32 timeoutSec) {

    m_pThis = pThis;

    m_maxConnections = maxConnections;
    m_timeoutSec = timeoutSec;

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;

    switch (m_mode) {
      case eConnectionMode::TCP_ASYNC:
      case eConnectionMode::TCP_BLOCKING:
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        break;

      case eConnectionMode::UDP_ASYNC:
      case eConnectionMode::UDP_BLOCKING:
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        break;
    }

    struct addrinfo *result;
    int ret;

    std::string portStr;
    CHECK(util::lexical_cast< std::string >(port, portStr));
    ret = getaddrinfo(NULL, portStr.c_str(), &hints, &result);
    RET_SM(
        ret == 0,
        Status::GENERIC_ERROR,
        "Error <" << WSAGetLastError() << "> getting address info for server.");

    m_socket =
        socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    RET_SM(
        m_socket != INVALID_SOCKET,
        Status::GENERIC_ERROR,
        "Error creating socket for server.");

    ret = bind(m_socket, result->ai_addr, (int) result->ai_addrlen);
    if (ret != 0) {
      Log(LL::Error) << "Error <" << WSAGetLastError() << "> binding socket.";
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      return Status::GENERIC_ERROR;
    }

    if (port == 0) {
      struct sockaddr_in adr_inet;
      int len_inet = sizeof adr_inet;
      if (getsockname(m_socket, (struct sockaddr *) &adr_inet, &len_inet)
          == 0) {
        m_port = ntohs(adr_inet.sin_port);
      } else {
        Log(LL::Warning) << "Unknown port chosen for server.";
      }
    }

    if (m_mode == eConnectionMode::TCP_ASYNC
        || m_mode == eConnectionMode::TCP_BLOCKING) {
      ret = listen(m_socket, MAX_PENDING_CONNECTIONS);
      if (ret != 0) {
        Log(LL::Error) << "Error <" << WSAGetLastError()
                       << "> listening on socket.";
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return Status::GENERIC_ERROR;
      }
    }

    Log(LL::Info) << "Started Server at port: " << m_port;
    return Status::OK;
  }

  /**
   *
   */
  Status acceptConnections(iServerConnectionHandler &handler) {
    ASSERT(valid());

    timeval selectTime = {0, 0};
    switch (m_mode) {
      case eConnectionMode::TCP_BLOCKING: {
        static const timeval waitTimer = {1, 0};
        selectTime = waitTimer;
        break;
      }

      case eConnectionMode::UDP_ASYNC:
      case eConnectionMode::UDP_BLOCKING:
        return Status::OK;
    }

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(m_socket, &fd);

    int ret = select(1, &fd, NULL, NULL, &selectTime);
    if (ret == 1) {
      SOCKET clientSocket = accept(m_socket, NULL, NULL);
      if (clientSocket == INVALID_SOCKET) {
        const int errorcode = WSAGetLastError();
        if (errorcode == WSAEWOULDBLOCK) {
          return Status::OK;
        } else {
          return Status::GENERIC_ERROR;
        }
      }
      if (!insertConnection(clientSocket, handler)) {
        closesocket(clientSocket);
      }
    } else if (ret == SOCKET_ERROR) {
      return Status::GENERIC_ERROR;
    }
    return Status::OK;
  }

  /**
   *
   */
  Status readConnections(iServerConnectionHandler &handler) {
    timeval selectTime = {0, 0};
    switch (m_mode) {
      case eConnectionMode::TCP_BLOCKING:
      case eConnectionMode::UDP_BLOCKING: {
        static const timeval waitTimer = {1, 0};
        selectTime = waitTimer;
        break;
      }
    }

    std::vector< SOCKET > sockets;
    switch (m_mode) {
      case eConnectionMode::TCP_BLOCKING:
      case eConnectionMode::TCP_ASYNC:
        sockets = getAllConnectedSockets();
        break;
      case eConnectionMode::UDP_BLOCKING:
      case eConnectionMode::UDP_ASYNC:
        sockets.push_back(m_socket);
        break;
    }

    for (std::vector< SOCKET >::iterator itr = sockets.begin();
         itr != sockets.end();
         ++itr) {
      fd_set fd;
      FD_ZERO(&fd);
      FD_SET(*itr, &fd);

      int ret = select(1, &fd, NULL, NULL, &selectTime);
      if (ret == 1) {
        u8 buffer[MAX_PACKET_SIZE] = {0};
        Blob data(buffer, MAX_PACKET_SIZE);

        sockaddr_storage addr;
        int addrlen = sizeof(addr);
        int ret = ::recvfrom(
            *itr,
            (char *) data.data(),
            MAX_PACKET_SIZE,
            0,
            (sockaddr *) &addr,
            &addrlen);

        Connection *con;
        if (!getConnectionForHost(&con, *itr, addr, handler)) {
          continue;
        }

        if (ret > 0) {
          data.trimSize(ret);
          m_pThis->m_stats.m_bytesRecieved += ret;
          if (!handler.process(*m_pThis, con->m_id, data)) {
            closeConnection(*con, &handler);
          }
          con->m_lastModified = timer::GetTicks();
        } else if (ret == 0) {
          Log(LL::Info) << "Connection closed: " << con->m_id;
          closeConnection(*con, &handler);
        } else {
          Log(LL::Info) << "Connection error on connection: " << con->m_id;
          closeConnection(*con, &handler);
        }
      }
    }

    for (tConnectionMap::iterator it = m_connections.begin();
         it != m_connections.end();) {
      if (timer::TicksToTime(timer::GetTicks() - it->second.m_lastModified)
          > m_timeoutSec) {
        Log(LL::Info) << "Connection timeout: " << it->second.m_id;
        closeConnection(it->second, &handler);
      }
      if (isConnectionClosed(*it)) {
        m_connectedHosts.erase(it->second.m_hostName);
        it = m_connections.erase(it);
      } else {
        ++it;
      }
    }

    m_pThis->m_stats.m_activeConnections =
        static_cast< u32 >(m_connections.size());
    handler.cleanup();

    return Status::OK;
  }

  /**
   *
   */
  Status send(const tConnectionId connectionId, const ConstBlob &data) {
    tConnectionMap::iterator itr = m_connections.find(connectionId);
    if (itr == m_connections.end() || itr->second.m_socket == INVALID_SOCKET) {
      return Status::BAD_ARGUMENT;
    }

    char *pStart = (char *) data.data();
    char *pEnd = pStart + data.size();
    while (pStart != pEnd) {
      const int sz = static_cast< int >(std::distance(pStart, pEnd));
      const int ret = ::sendto(
          itr->second.m_socket,
          pStart,
          sz,
          0, // flags
          (const sockaddr *) &(itr->second.m_clientAddr),
          sizeof(itr->second.m_clientAddr));
      if (ret == SOCKET_ERROR) {
        return Status::GENERIC_ERROR;
      }
      pStart += ret;
      m_pThis->m_stats.m_bytesSent += ret;
    }
    return Status::OK;
  }

  /**
   *
   */
  void stop() {
    for (tConnectionMap::iterator itr = m_connections.begin();
         itr != m_connections.end();
         ++itr) {
      closeConnection(itr->second, NULL);
    }
    m_connections.clear();
    m_connectedHosts.clear();
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
  }

  /**
   *
   */
  bool valid() const { return (m_socket != INVALID_SOCKET); }

  /**
   *
   */
  eConnectionMode::type getConnectionMode() const { return m_mode; }

  private:
  eConnectionMode::type m_mode;
  SOCKET m_socket;
  NetServer *m_pThis;

  typedef std::map< tConnectionId, Connection > tConnectionMap;
  typedef std::map< std::string, tConnectionId > tConnectionHostMap;
  typedef std::map< SOCKET, tConnectionId > tConnectionSocketMap;
  tConnectionMap m_connections;
  tConnectionHostMap m_connectedHosts;
  tConnectionSocketMap m_connectedSockets;
  u32 m_maxConnections;
  u32 m_port;

  f32 m_timeoutSec;

  tConnectionId m_connectionIdNext;

  void closeConnection(Connection &c, iServerConnectionHandler *pHandler) {
    if (c.m_socket == INVALID_SOCKET) {
      return;
    }

    Log(LL::Info) << "Closing connection: " << c.m_id << ":" << c.m_hostName;
    if (m_mode == eConnectionMode::TCP_ASYNC
        || m_mode == eConnectionMode::TCP_BLOCKING) {
      closesocket(c.m_socket);
      c.m_socket = INVALID_SOCKET;
    }

    if (pHandler) {
      pHandler->close(c.m_id);
    }
  }

  bool
  insertConnection(SOCKET &clientSocket, iServerConnectionHandler &handler) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getpeername(clientSocket, (struct sockaddr *) &addr, &addr_len)
        != SOCKET_ERROR) {
      return insertConnection(clientSocket, addr, handler);
    }
    return false;
  }

  bool insertConnection(
      SOCKET &clientSocket,
      const sockaddr_storage &host,
      iServerConnectionHandler &handler) {
    if (m_connections.size() >= m_maxConnections) {
      Log(LL::Warning) << "Max connections (" << m_maxConnections
                       << ") was reached. Dropping connection attempt.";
      return false;
    }

    std::string addrString = "<unknown>";
    if (!getHostName(addrString, host)) {
      return false;
    }

    Connection con = Connection(
        m_connectionIdNext++,
        core::timer::GetTicks(),
        clientSocket,
        addrString,
        host);
    m_connections[con.m_id] = con;
    m_connectedHosts[con.m_hostName] = con.m_id;
    m_connectedSockets[clientSocket] = con.m_id;
    handler.open(con.m_id);
    Log(LL::Info) << "Accepted connection " << con.m_id << " from "
                  << con.m_hostName;
    return true;
  }

  std::vector< SOCKET > getAllConnectedSockets() {
    std::vector< SOCKET > rVal;
    for (tConnectionMap::const_iterator itr = m_connections.begin();
         itr != m_connections.end();
         ++itr) {
      rVal.push_back(itr->second.m_socket);
    }
    return std::move(rVal);
  }

  bool getConnectionForHost(
      Connection **rVal,
      SOCKET &clientSocket,
      const sockaddr_storage &host,
      iServerConnectionHandler &handler) {
    *rVal = NULL;
    std::string hostName;
    if (getHostName(hostName, host)) {
      tConnectionHostMap::const_iterator itr = m_connectedHosts.find(hostName);
      if (itr != m_connectedHosts.end()) {
        *rVal = &m_connections[itr->second];
      } else if (
          m_mode == eConnectionMode::UDP_ASYNC
          || m_mode == eConnectionMode::UDP_BLOCKING) {
        if (insertConnection(clientSocket, host, handler)) {
          *rVal = &m_connections[m_connectionIdNext - 1];
        }
      }
    } else {
      tConnectionSocketMap::const_iterator itr =
          m_connectedSockets.find(clientSocket);
      if (itr != m_connectedSockets.end()) {
        *rVal = &m_connections[itr->second];
      }
    }
    return *rVal != NULL;
  }

  bool getHostName(std::string &out, const sockaddr_storage &host) {
    char hostName[INET6_ADDRSTRLEN];
    memset(hostName, 0, INET6_ADDRSTRLEN);
    DWORD len = INET6_ADDRSTRLEN;
    if (WSAAddressToString(
            (sockaddr *) &host, sizeof(host), NULL, hostName, &len)
        != 0) {
      Log(LL::Trace) << "Could not resolve host name.";
      return false;
    }
    out = hostName;
    return true;
  }

  static bool
  isConnectionClosed(const std::pair< tConnectionId, Connection > &itr) {
    return (itr.second.m_socket == INVALID_SOCKET);
  }
};

/**
 * Delegate to implementation
 */
NetServer::NetServer(eConnectionMode::type mode)
    : m_pImpl(new NetServer::Impl(mode)) {
}

/**
 *
 */
NetServer::~NetServer() {
  delete m_pImpl;
}

/**
 * Delegate to implementation
 */
Status NetServer::start(const core::net::ServerDef &def) {
  Log(LL::Trace) << "Starting server at localhost:" << def.m_port << " with "
                 << def.m_maxConnections << " connections.";

  CHECK_M(
      def.m_port == 0 || def.m_port == 80 || def.m_port > 1024,
      "Bad port."); // common reserved ports
  CHECK_M(
      def.m_maxConnections > 0,
      "Bad server connection count."); // require positive, non-zero
  CHECK_M(
      def.m_connectionTimeoutSec >= 0,
      "Bad server timeout"); // require positive

  CHECK_M(core::net::IsInitialized(), "net::Initialize() must be called");
  return m_pImpl->start(
      this, def.m_port, def.m_maxConnections, def.m_connectionTimeoutSec);
}

/**
 * Delegate to implementation
 */
void NetServer::stop() {
  if (!valid()) {
    return;
  }
  m_pImpl->stop();
}

/**
 * Delegate to implementation
 */
Status NetServer::update(iServerConnectionHandler &handler) {
  if (!valid()) {
    return Status::BAD_STATE;
  }

  Status ret = m_pImpl->acceptConnections(handler);
  if (!ret) {
    return ret;
  }
  return m_pImpl->readConnections(handler);
}

/**
 * Delegate to implementation
 */
bool NetServer::valid() const {
  return m_pImpl->valid();
}

/**
 * Delegate to implementation
 */
Status NetServer::send(const tConnectionId connectionId, const ConstBlob &msg) {
  if (!valid()) {
    return Status::BAD_STATE;
  }
  ASSERT(msg.size() < std::numeric_limits< int >::max());

  return m_pImpl->send(connectionId, msg);
}

/**
 *
 */
eConnectionMode::type NetServer::getConnectionMode() const {
  return m_pImpl->getConnectionMode();
}

} // namespace net
} // namespace core

#endif
