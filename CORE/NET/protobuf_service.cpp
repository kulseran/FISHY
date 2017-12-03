#if 0
#  include "protobuf_service.h"

#  include <CORE/ARCH/intrinsics.h>
#  include <CORE/ARCH/timer.h>

using core::net::ServerDef;
using core::util::thread::ThreadPool;
using wrappers::net::util::ServerPacketHandler;
using wrappers::net::util::ClientPacketHandler;

namespace core {
namespace util {

/**
 * Helper class for delegating requests to the worker threads.
 */
class ProcessRunner : public core::util::Runnable {
  public:
    typedef srutil::delegate<bool(core::net::iNetServer &server,
                                  const core::net::tConnectionId connectionId,
                                  const wrappers::net::util::Packet &data)> tProc;

    ProcessRunner(
      core::net::iNetServer &server,
      core::net::tConnectionId connectionId,
      const wrappers::net::util::Packet &data,
      tProc proc)
      : m_server(server),
        m_connectionId(connectionId),
        m_data(data),
        m_proc(proc) {
    }

    virtual void run() override {
      m_proc(m_server, m_connectionId, m_data);
    }

  private:
    tProc m_proc;
    core::net::iNetServer &m_server;
    core::net::tConnectionId m_connectionId;
    const wrappers::net::util::Packet &m_data;
};

/**
 *
 */
iProtoServiceServer::iProtoServiceServer(const ThreadPool::PoolOptions &poolOptions)
  : m_server(core::net::eConnectionMode::TCP_BLOCKING),
    m_workerPool(poolOptions) {
}
/**
 *
 */
iProtoServiceServer::~iProtoServiceServer() {
  if (m_running) {
    stop();
  }
}

/**
 *
 */
bool iProtoServiceServer::start(const ServerDef &def) {
  if (!m_server.start(def)) {
    return false;
  }
  m_running = true;

  m_workerPool.start();
  m_processthread = std::thread(srutil::delegate<void(void)>::from_method<iProtoServiceServer, &iProtoServiceServer::processInternal>(this));
  return true;
}

/**
 *
 */
void iProtoServiceServer::stop() {
  m_running = false;
  m_server.stop();
  m_processthread.join();
  m_workerPool.terminate();
}

/**
 *
 */
bool iProtoServiceServer::valid() const {
  return m_running && m_server.valid();
}

/**
 *
 */
core::net::NetStats iProtoServiceServer::stats() const {
  return m_server.stats();
}

/**
 *
 */
void iProtoServiceServer::processInternal() {
  wrappers::net::util::ServerPacketHandler packetHandler;
  while (m_running) {
    m_server.update(packetHandler);

    const ServerPacketHandler::tPacketList packets = packetHandler.getPackets();
    std::vector<ProcessRunner> runners;
    runners.reserve(packets.size());
    for (ServerPacketHandler::tPacketList::const_iterator packet = packets.begin(); packet != packets.end(); ++packet) {
      runners.push_back(ProcessRunner(m_server, packet->first, packet->second, ProcessRunner::tProc::from_method<iProtoServiceServer, &iProtoServiceServer::process>(this)));
    }
    for (u32 idx = 0; idx < packets.size(); ++idx) {
      m_workerPool.execute(&runners[idx]);
    }
    m_workerPool.wait();

    std::this_thread::yield();
  }
}

/**
 *
 */
iProtoServiceClient::~iProtoServiceClient() {
  if (m_running) {
    stop();
  }
}

/**
 *
 */
bool iProtoServiceClient::start(const core::net::ServerDef &def) {
  if (!m_client.start(def)) {
    return false;
  }
  m_running = true;
  m_clientDef = def;

  m_processthread = std::thread(srutil::delegate<void(void)>::from_method<iProtoServiceClient, &iProtoServiceClient::processInternal>(this));
  m_timeoutTicks = core::timer::TimeToTicks(static_cast<f64>(def.get_connection_timeout_sec()));
  return true;
}

/**
 *
 */
void iProtoServiceClient::stop() {
  m_running = false;
  if (m_client.valid()) {
    m_client.stop();
  }
  m_processthread.join();
}

/**
 *
 */
bool iProtoServiceClient::valid() const {
  return m_running && m_client.valid();
}

/**
 *
 */
core::net::NetStats iProtoServiceClient::stats() const {
  return m_client.stats();
}

/**
 *
 */
void iProtoServiceClient::processInternal() {
  ClientPacketHandler packetHandler;

  u64 timeoutTicks = 0;
  while (m_running) {
    if (!m_client.valid()) {
      u32 RECONNECT_DELAY_SEC = 5;
      std::this_thread::sleep_for(std::chrono::seconds(RECONNECT_DELAY_SEC));
      if (!m_client.start(m_clientDef)) {
        Log(LL::Error) << "Failed to reconnect to server. Retrying in " << RECONNECT_DELAY_SEC << "s." << std::endl;
      } else {
        Log(LL::Warning) << "Reconnected to server." << std::endl;
      }
      continue;
    }
    m_client.update(packetHandler);

    const ClientPacketHandler::tPacketList packets = packetHandler.getPackets();

    m_mutex.lock();
    for (ClientPacketHandler::tPacketList::const_iterator packet = packets.begin(); packet != packets.end(); ++packet) {
      tCallbackMap::iterator callback = m_callbacks.find(packet->getHeader().m_index);
      if (callback == m_callbacks.end()) {
        continue;
      }
      callback->second(packet->getHeader().m_index, *packet);
      m_callbacks.erase(callback);
    }
    m_mutex.unlock();

    const u64 now = core::timer::GetTicks();
    if (now > timeoutTicks) {
      timeoutTicks = now + m_timeoutTicks;
      m_cv.notify_all();
    }
    std::this_thread::yield();
  }
  m_running = false;
}

/**
 *
 */
void iProtoServiceClient::signalCallback(const u16 index, const wrappers::net::util::Packet &packet) {
  m_recvPackets[index] = packet;
  m_cv.notify_all();
}

/**
 *
 */
u16 iProtoServiceClient::doAsyncRpc(wrappers::net::util::Packet &packet, tCallback *pCallback) {
  u16 index = m_nextIndex++;

  if (pCallback) {
    LOCK_MUTEX(m_mutex);
    m_callbacks[index] = *pCallback;
  }
  packet.setIndex(index);
  wrappers::net::util::SendPacket(m_client, packet);
  return index;
}

/**
 *
 */
bool iProtoServiceClient::doSyncRpc(wrappers::net::util::Packet &packetSend, wrappers::net::util::Packet &packetRecv) {
  tCallback callback = tCallback::from_method<iProtoServiceClient, &iProtoServiceClient::signalCallback>(this);
  std::unique_lock<std::mutex> lock(m_mutex);
  const u16 index = doAsyncRpc(packetSend, &callback);

  const u64 timeoutTicks = core::timer::GetTicks() + m_timeoutTicks;
  const long timeoutMillis = static_cast<long>(core::timeunit::Seconds.toMillis(core::timer::TicksToTime(m_timeoutTicks)));
  while (core::timer::GetTicks() < timeoutTicks) {
    m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMillis));

    tRecvMap::iterator itr = m_recvPackets.find(index);
    if (itr == m_recvPackets.end()) {
      continue;
    }

    packetRecv = itr->second;
    m_recvPackets.erase(itr);
    return true;
  }

  return false;
}

/**
 *
 */
bool PacketToProto(const wrappers::net::util::Packet &packet, iProtoMessage &proto) {
  core::base::ConstBlobSink sink(packet.getPayload());
  return proto.iserialize(sink);
}

/**
 *
 */
bool ProtoToPacket(const iProtoMessage &proto, wrappers::net::util::Packet &packet) {
  const size_t output_size = proto.byte_size();
  if (output_size > wrappers::net::util::PACKET_MAX_SIZE_TCP) {
    return false;
  }
  core::base::BlobSink sink(packet.getMutablePayload(output_size));
  return proto.oserialize(sink);
}

} // namespace util
} // namespace core
#endif
