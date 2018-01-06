#include "proto_printer.h"
#include "proto_printer_header.h"
#include "proto_printer_source.h"

#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/HASH/crc32.h>
#include <CORE/UTIL/stringutil.h>

#include <fstream>
#include <set>
#include <string>

using core::types::EnumDef;
using core::types::FieldDef;
using core::types::MessageDef;
using core::types::ProtoDef;
using core::types::RpcFunctionDef;
using core::types::ServiceDef;
using core::types::tEnumList;
using core::types::tFieldList;
using core::types::tServiceList;

static void printImports(
    std::ofstream &ofile,
    const std::vector< std::string > &imports,
    bool hasServices);
static void printCppVirtuals(
    std::ofstream &ofile, const MessageDef &msgDef, const std::string &package);

/**
 *
 */
bool print(const ProtoDef &def, const std::string &fileNameRoot) {
  if (!PrintHeader(def, fileNameRoot + ".h")) {
    return false;
  }
  if (!PrintCpp(def, fileNameRoot + ".h", fileNameRoot + ".cpp")) {
    return false;
  }
  return true;
}

/**
 *
 */
void styleFile(const std::string &astylePath, const std::string &fileNameRoot) {
}

/**
 *
 */

/**
 *
 *
void printHeaderService(std::ofstream &ofile, const ServiceDef &srvDef) {
  ofile << "class " << srvDef.m_name
        << "Server : public ::types::util::iProtoServiceServer {\n";
  ofile << "protected:\n";
  ofile << srvDef.m_name
        << "Server(const ::core::util::thread::ThreadPool::PoolOptions "
           "&poolOptions) : ::core::util::iProtoServiceServer(poolOptions) { }"
        << std::endl;
  for (std::vector< RpcFunctionDef >::const_iterator itr =
           srvDef.m_functions.begin();
       itr != srvDef.m_functions.end();
       ++itr) {
    ofile << "virtual " << itr->m_return << " " << itr->m_name << "(const "
          << itr->m_param << " &param) = 0;\n";
  }
  ofile << "protected:\nvirtual bool process(core::net::iNetServer &server, "
           "const core::net::tConnectionId connectionId, const "
           "wrappers::net::util::Packet &) final;"
        << std::endl;
  ofile << "};\n";

  ofile << "class " << srvDef.m_name
        << "Client : public ::core::types::iProtoServiceClient {\n";
  ofile << "public:\n";
  for (std::vector< RpcFunctionDef >::const_iterator itr =
           srvDef.m_functions.begin();
       itr != srvDef.m_functions.end();
       ++itr) {
    ofile << itr->m_return << " " << itr->m_name << "(const " << itr->m_param
          << " &param);\n";
    ofile << "typedef srutil::delegate<void(const u16, const " << itr->m_return
          << " &)> t" << itr->m_name << "Callback;\n";
    ofile << "u16 " << itr->m_name << "Async(const " << itr->m_param
          << " &param, t" << itr->m_name << "Callback *proc);\n";
  }
  ofile << "};\n";
}

/**
 *
 *
void printCppServiceHandlers(
    std::ofstream &ofile,
    const ServiceDef &srvDef,
    const std::string &package) {
  ofile << "bool " << package << srvDef.m_name
        << "Server::process(core::net::iNetServer &server, const "
           "core::net::tConnectionId connectionId, const "
           "wrappers::net::util::Packet &packet) {"
        << std::endl;
  ofile << "switch (packet.getHeader().m_type) {\n";

  for (std::vector< RpcFunctionDef >::const_iterator itr =
           srvDef.m_functions.begin();
       itr != srvDef.m_functions.end();
       ++itr) {
    const u16 sig = static_cast< u16 >(
        core::hash::CRC32(itr->m_name.begin(), itr->m_name.end()));
    ofile << "case " << sig << ": {\n";
    ofile << itr->m_param << " msgIn;\n";
    ofile
        << "if (!core::util::PacketToProto(packet, msgIn)) {\nreturn false;\n}"
        << std::endl;
    ofile << itr->m_return << " ret = " << itr->m_name << "(msgIn);"
          << std::endl;
    ofile << "wrappers::net::util::Packet retPacket;\n";
    ofile << "retPacket.setType(packet.getHeader().m_type);\n";
    ofile << "retPacket.setIndex(packet.getHeader().m_index);\n";
    ofile
        << "if (!core::util::ProtoToPacket(ret, retPacket)) {\nreturn false;\n}"
        << std::endl;
    ofile << "wrappers::net::util::SendPacket(server, connectionId, retPacket);"
          << std::endl;
    ofile << "break;\n";
    ofile << "}\n";
  }

  ofile << "default:\nreturn false;\n";
  ofile << "}\n";
  ofile << "return true;\n";
  ofile << "}\n";

  for (std::vector< RpcFunctionDef >::const_iterator itr =
           srvDef.m_functions.begin();
       itr != srvDef.m_functions.end();
       ++itr) {
    const u16 sig = static_cast< u16 >(
        core::hash::CRC32(itr->m_name.begin(), itr->m_name.end()));

    ofile << itr->m_return << " " << package << srvDef.m_name
          << "Client::" << itr->m_name << "(const " << itr->m_param
          << " &param) {\n";
    ofile << "wrappers::net::util::Packet packetSend;\n";
    ofile << "if (!core::util::ProtoToPacket(param, packetSend)) {"
          << std::endl;
    ofile << "return " << itr->m_return << "();\n";
    ofile << "}\n";
    ofile << "packetSend.setType(" << sig << ");\n";
    ofile << "wrappers::net::util::Packet packetRecv;\n";
    ofile << "if (doSyncRpc(packetSend, packetRecv)) {\n";
    ofile << itr->m_return << " ret;\n";
    ofile << "if (core::util::PacketToProto(packetRecv, ret)) {\n";
    ofile << "return ret;\n";
    ofile << "}\n";
    ofile << "}\n";
    ofile << "return " << itr->m_return << "();\n";
    ofile << "}\n";

    ofile << "u16 " << package << srvDef.m_name << "Client::" << itr->m_name
          << "Async(const " << itr->m_param
          << " &param, srutil::delegate<void(const u16, const " << itr->m_return
          << " &)> *proc) {\n";
    ofile << "wrappers::net::util::Packet packetSend;\n";
    ofile << "if (!core::util::ProtoToPacket(param, packetSend)) {"
          << std::endl;
    ofile << "return 0;\n";
    ofile << "}\n";
    ofile << "packetSend.setType(" << sig << ");\n";
    ofile << "if (proc) {\n";
    ofile << "ConvertCallback<" << itr->m_return
          << "> *cb = new ConvertCallback<" << itr->m_return << ">(*proc);"
          << std::endl;
    ofile << "tCallback callback = tCallback::from_method<ConvertCallback<"
          << itr->m_return << ">, &ConvertCallback<" << itr->m_return
          << ">::process>(cb);\n";
    ofile << "return doAsyncRpc(packetSend, &callback);\n";
    ofile << "} else {\n";
    ofile << "return doAsyncRpc(packetSend, nullptr);\n";
    ofile << "}\n";
    ofile << "}\n";
  }
}
*/
