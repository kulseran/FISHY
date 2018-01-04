#include "net_client.h"

#include <CORE/ARCH/timer.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/lexical_cast.h>

#if defined(PLAT_WIN32)
#  include <WS2tcpip.h>
#  include <WinSock2.h>
#elif defined(PLAT_LINUX)
#  include <cstdlib>
#  include <netdb.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <unistd.h>
#else
#  error "networking not supported on this platform"
#endif
#include <algorithm>

using core::memory::Blob;
using core::memory::ConstBlob;

#if defined(PLAT_WIN32)
#  define MAX_HOST_NAMELEN (INET6_ADDRSTRLEN)

/**
 * Cross platform wrap error handling.
 */
static int NetGetLastError() {
  return WSAGetLastError();
}

#elif defined(PLAT_LINUX)
typedef int SOCKET;

#  define MAX_HOST_NAMELEN (NI_MAXHOST)
#  define INVALID_SOCKET (-1)
#  define SOCKET_ERROR (-1)

/**
 * Cross platform wrap error handling.
 */
static int NetGetLastError() {
  return errno;
}

/**
 * Cross platform wrap closing a socket.
 */
static void closesocket(SCOKET &fd) {
  close(fd);
}

#else
#  error "networking not supported on this platform"
#endif

namespace core {
namespace net {

/**
 * Implementation of the {@link NetClient} logic for PLAT_WIN32
 */
class NetClient::Impl {
  public:
  /**
   *
   */
  Impl(const eConnectionMode::type mode)
      : m_mode(mode), m_pThis(nullptr), m_socket(INVALID_SOCKET) {}

  ~Impl() { ASSERT(m_socket == INVALID_SOCKET); }

  /**
   *
   */
  Status
  start(NetClient *pThis, const std::string &remoteHost, const u32 port) {
    m_pThis = pThis;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
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
    ret = getaddrinfo(remoteHost.c_str(), portStr.c_str(), &hints, &result);
    RET_SM(
        ret == 0,
        Status::BAD_ARGUMENT,
        "Error <" << NetGetLastError() << "> getting address info for client.");

    m_socket =
        socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    RET_SM(
        m_socket != INVALID_SOCKET,
        Status::GENERIC_ERROR,
        "Failed to create socket for client.");

    ret = connect(
        m_socket, result->ai_addr, static_cast< int >(result->ai_addrlen));
    if (ret == SOCKET_ERROR) {
      Log(LL::Error) << "Failed to connect to server at: " << remoteHost << ":"
                     << port;
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      return Status::GENERIC_ERROR;
    }

    std::string addrString = "<unknown>";
    char host[MAX_HOST_NAMELEN];
    memset(host, 0, MAX_HOST_NAMELEN);
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (getpeername(m_socket, (struct sockaddr *) &addr, &addr_len)
        != SOCKET_ERROR) {
#if defined(PLAT_WIN32)
      DWORD len = MAX_HOST_NAMELEN;
      if (WSAAddressToString(
              (struct sockaddr *) &addr, addr_len, nullptr, host, &len)
          == 0) {
        addrString = host;
      }
#elif defined(PLAT_LINUX)
      if (getnameinfo(
              (struct sockaddr *) &addr,
              addr_len,
              host,
              MAX_HOST_NAMELEN,
              nullptr,
              0,
              NI_NUMERICSERV)
          == 0) {
        addrString = host;
      }
#else
#  error "networking not supported on this platform"
#endif
    }

    m_pThis->m_stats.m_activeConnections = 1;
    Log(LL::Info) << "Connected to server: " << addrString;
    return Status::OK;
  }

  /**
   *
   */
  Status readConnections(iClientConnectionHandler &handler) {
    timeval selectTime = {0, 0};

    switch (m_mode) {
      case eConnectionMode::TCP_BLOCKING:
      case eConnectionMode::UDP_BLOCKING: {
        static const timeval waitTimer = {1, 0};
        selectTime = waitTimer;
        break;
      }
    }

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(m_socket, &fd);

    int ret = select(1, &fd, nullptr, nullptr, &selectTime);
    if (ret == 1) {
      u8 buffer[MAX_PACKET_SIZE] = {0};
      Blob data(buffer, MAX_PACKET_SIZE);
      int ret = ::recv(m_socket, (char *) data.data(), MAX_PACKET_SIZE, 0);
      if (ret > 0) {
        m_pThis->m_stats.m_bytesRecieved += ret;
        data.trimSize(ret);
        handler.process(*m_pThis, data);
        handler.cleanup();
      } else if (ret == 0) {
        Log(LL::Info) << "Connection to server closed.";
        stop();
      } else {
        Log(LL::Info) << "Connection error talking to server.";
        stop();
      }
    } else if (ret == 0) {
      return Status::TIMEOUT;
    } else if (ret == SOCKET_ERROR) {
      return Status::GENERIC_ERROR;
    }
    return Status::OK;
  }

  /**
   *
   */
  Status send(const ConstBlob &data) {
    char *pStart = (char *) data.data();
    char *pEnd = pStart + data.size();
    while (pStart != pEnd) {
      const int sz = static_cast< int >(std::distance(pStart, pEnd));
      const int ret = ::send(m_socket, pStart, sz, 0);
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
    Log(LL::Info) << "Closing connection to server.";
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
  NetClient *m_pThis;
  SOCKET m_socket;
};

/**
 * Delegate to implementation
 */
NetClient::NetClient(const eConnectionMode::type mode)
    : m_pImpl(new NetClient::Impl(mode)) {
}

NetClient::~NetClient() {
  delete m_pImpl;
}

/**
 * Delegate to implementation
 */
bool NetClient::valid() const {
  return m_pImpl->valid();
}

/**
 * Delegate to implementation
 */
Status NetClient::start(const ClientDef &def) {
  Log(LL::Trace) << "Connecting to server: " << def.m_dnsName << ":"
                 << def.m_port;

  CHECK_M(!def.m_dnsName.empty(), "ServerDef missing required field: dns_name");
  CHECK_M(def.m_port == 0, "ServerDef missing required field: port");

  CHECK_M(core::net::IsInitialized(), "net::Initialize() must be called");
  return m_pImpl->start(this, def.m_dnsName, def.m_port);
}

/**
 * Delegate to implementation
 */
void NetClient::stop() {
  if (!valid()) {
    return;
  }

  m_pImpl->stop();
}

/**
 * Delegate to implementation
 */
Status NetClient::update(iClientConnectionHandler &handler) {
  if (!valid()) {
    return Status::BAD_STATE;
  }
  return m_pImpl->readConnections(handler);
}

/**
 * Delegate to implementation
 */
Status NetClient::send(const ConstBlob &data) {
  if (!valid()) {
    return Status::BAD_STATE;
  }
  ASSERT(data.size() < std::numeric_limits< int >::max());
  return m_pImpl->send(data);
}

/**
 *
 */
eConnectionMode::type NetClient::getConnectionMode() const {
  return m_pImpl->getConnectionMode();
}

} // namespace net
} // namespace core
