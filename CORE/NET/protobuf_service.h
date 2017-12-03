/**
 * Definitions for Fishy style protobuffer services.
 * Based loosely off Google protobuffers.
 */
#if 0
#  ifndef FISHY_PROTOBUF_SERVICE_H
#    define FISHY_PROTOBUF_SERVICE_H

#    include "protobuf.h"

#    include <CORE/ARCH/mutex.h>
#    include <CORE/NET/net_client.h>
#    include <CORE/NET/net_server.h>
#    include <CORE/UTIL/CONCURRENT/threadpool.h>
#    include <EXTERN_LIB/srutil/delegate/delegate.hpp>
#    include <WRAPPERS/NET/packet_handler.h>

#    include <atomic>
#    include <condition_variable>
#    include <map>
#    include <thread>

namespace core {
namespace util {

/**
 * Protobuffer RPC service server
 */
class iProtoServiceServer {
  public:
  virtual ~iProtoServiceServer();

  /**
   * Starts the server listener thread.
   */
  bool start(const core::net::ServerDef &);

  /**
   * Stops the server listener thread.
   */
  void stop();

  /**
   * Check if the server thread is running.
   */
  bool valid() const;

  /**
   * Return the underlying network stats.
   */
  core::net::NetStats stats() const;

  protected:
  iProtoServiceServer(const core::util::thread::ThreadPool::PoolOptions &);

  /**
   * The protoc compiler should generate this function.
   * It's purpose is to handle processing of an incoming request.
   */
  virtual bool process(
      core::net::iNetServer &server,
      const core::net::tConnectionId connectionId,
      const wrappers::net::util::Packet &data) = 0;

  private:
  void processInternal();

  std::atomic_bool m_running;

  core::net::NetServer m_server;
  std::thread m_processthread;
  core::util::thread::ThreadPool m_workerPool;
};

/**
 * Protobuffer RPC service client
 */
class iProtoServiceClient {
  public:
  virtual ~iProtoServiceClient();

  /**
   * Starts the server listener thread.
   */
  bool start(const core::net::ServerDef &);

  /**
   * Stops the server listener thread.
   */
  void stop();

  /**
   * Check if the server thread is running.
   */
  bool valid() const;

  /**
   * Return the underlying network stats.
   */
  core::net::NetStats stats() const;

  protected:
  iProtoServiceClient()
      : m_nextIndex(1),
        m_timeoutTicks(0),
        m_client(core::net::eConnectionMode::TCP_ASYNC) {
    m_running = false;
  }

  typedef srutil::delegate< void(
      const u16, const wrappers::net::util::Packet &) >
      tCallback;
  u16 doAsyncRpc(wrappers::net::util::Packet &packet, tCallback *);
  bool doSyncRpc(
      wrappers::net::util::Packet &packetSend,
      wrappers::net::util::Packet &packetRecv);

  template < typename tType >
  class ConvertCallback {
    public:
    ConvertCallback(
        srutil::delegate< void(const u16, const typename tType &) > &cb)
        : m_cb(cb) {}

    void process(const u16 id, const wrappers::net::util::Packet &packet) {
      tType proto;
      if (PacketToProto(packet, proto)) {
        m_cb(id, proto);
      }
      delete this;
    }

    private:
    srutil::delegate< void(const u16, const typename tType &) > m_cb;
  };

  private:
  void processInternal();
  void signalCallback(const u16, const wrappers::net::util::Packet &);

  std::atomic_bool m_running;
  std::atomic_int32_t m_nextIndex;
  u64 m_timeoutTicks;
  core::net::NetClient m_client;
  core::net::ServerDef m_clientDef;
  std::thread m_processthread;
  std::mutex m_mutex;
  std::condition_variable m_cv;
  typedef std::map< u16, wrappers::net::util::Packet > tRecvMap;
  tRecvMap m_recvPackets;
  typedef std::map< u16, tCallback > tCallbackMap;
  tCallbackMap m_callbacks;
};

/**
 * Utility function to parse a proto out of a packet.
 */
bool PacketToProto(
    const wrappers::net::util::Packet &packet, iProtoMessage &proto);

/**
 * Utility function to parse a proto into a packet.
 */
bool ProtoToPacket(
    const iProtoMessage &proto, wrappers::net::util::Packet &packet);

} // namespace util
} // namespace core

#  endif
#endif
