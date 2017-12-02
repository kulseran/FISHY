#include "proto_printer.h"

#include <CORE/ARCH/process.h>
#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/HASH/crc32.h>
#include <CORE/UTIL/stringutil.h>

#include <fstream>
#include <string>
#include <set>

using core::util::EnumDef;
using core::util::FieldDef;
using core::util::MessageDef;
using core::util::ProtoDef;
using core::util::RpcFunctionDef;
using core::util::ServiceDef;

static bool printHeader(const ProtoDef &def, const std::string &fileName);
static bool printCpp(const ProtoDef &def, const std::string &headerName, const std::string &fileName);

static void printCloseNamespace(std::ofstream &ofile, const std::vector< std::string > &package);
static void printOpenNamespace(std::ofstream &ofile, const std::vector< std::string > &package);
static void printImports(std::ofstream &ofile, const std::vector< std::string > &imports, bool hasServices);
static void printCppVirtuals(std::ofstream &ofile, const MessageDef &msgDef, const std::string &package);
static void printCppServiceHandlers(std::ofstream &ofile, const ServiceDef &srvDef, const std::string &package);

static void printHeaderMessage(std::ofstream &ofile, const MessageDef &msgDef);
static void printHeaderService(std::ofstream &ofile, const ServiceDef &srvDef);
static void printFieldEnum(std::ofstream &ofile, const MessageDef &msgDef);
static void printBuilder(std::ofstream &ofile, const MessageDef &msgDef);
static void printBuilderStorage(std::ofstream &ofile, const MessageDef &msgDef);
static void printBuilderSetters(std::ofstream &ofile, const MessageDef &msgDef);
static void printBuilderGetters(std::ofstream &ofile, const MessageDef &msgDef);
static void printEnum(std::ofstream &ofile, const EnumDef &enumDef);
static void printInitilizerFromOther(std::ofstream &ofile, const MessageDef &msgDef);
static std::string getBuilderType(const FieldDef::eFieldType type, const std::string &msgName);
static std::string defaultValue(const FieldDef::eFieldType type, const std::string msgType);
static void printOSerializer(std::ofstream &ofile, const std::vector< std::string > &package, const MessageDef &msgDef);
static void printISerializer(std::ofstream &ofile, const std::vector< std::string > &package, const MessageDef &msgDef);

/**
 *
 */
bool print(const ProtoDef &def, const std::string &fileNameRoot) {
  if (!printHeader(def, fileNameRoot + ".pb.h")) {
    return false;
  }
  if (!printCpp(def, fileNameRoot + ".pb.h", fileNameRoot + ".pb.cpp")) {
    return false;
  }
  return true;
}

/**
 *
 */
void styleFile(const std::string &astylePath, const std::string &fileNameRoot) {
  if (astylePath.empty()) {
    return;
  }

  std::vector<std::string> params;
  params.push_back(fileNameRoot + ".pb.h");
  params.push_back("--style=java");
  params.push_back("--add-brackets");
  params.push_back("--S");
  params.push_back("--C");
  params.push_back("--m2");
  params.push_back("--p");
  params.push_back("--H");
  params.push_back("--U");
  params.push_back("--keep-one-line-blocks");
  params.push_back("--convert-tabs");
  params.push_back("--k3");
  params.push_back("--s2");
  params.push_back("--n");
  CHECK(core::process::launch(astylePath, params));
  params[0] = fileNameRoot + "pb.cpp";
  CHECK(core::process::launch(astylePath, params));
}

/**
 * Prints the header file portion of the proto
 */
bool printHeader(const ProtoDef &def, const std::string &fileName) {
  std::ofstream ofile(fileName);
  if (!ofile.is_open()) {
    return false;
  }

  const std::string safeName = core::util::identifierSafe(fileName);
  ofile << "#ifndef FISHY_PROTOC_" << safeName << "_H" << std::endl;
  ofile << "#define FISHY_PROTOC_" << safeName << "_H" << std::endl;

  printImports(ofile, def.m_imports, !def.m_services.empty());
  const std::vector< std::string > package = core::util::Splitter().on('.').split(def.m_package);
  printOpenNamespace(ofile, package);
  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    printHeaderMessage(ofile, *message);
  }
  for (std::vector< ServiceDef >::const_iterator service = def.m_services.begin(); service != def.m_services.end(); ++service) {
    printHeaderService(ofile, *service);
  }
  printCloseNamespace(ofile, package);
  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    printOSerializer(ofile, package, *message);
    printISerializer(ofile, package, *message);
  }
  ofile << "#endif" << std::endl;

  return true;
}

/**
 *
 */
void printCppDescriptorGen(std::ofstream &ofile, const MessageDef &msgDef, const std::string &package) {
  const std::string genFunctionName = core::util::identifierSafe(package + msgDef.m_name);
  ofile << "static core::util::ProtoDescriptor &InternalGenDescriptor_" << genFunctionName << "() {" << std::endl;
  ofile << "core::util::MessageDef defSelf;" << std::endl;
  ofile << "defSelf.m_name = \"" << msgDef.m_name << "\";" << std::endl;
  ofile << "defSelf.m_package = \"::" << package << "\";" << std::endl;
  ofile << "defSelf.m_fields.reserve(" << msgDef.m_fields.size() << ");" << std::endl;
  for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
    ofile << "{" << std::endl;
    ofile << "core::util::FieldDef field;" << std::endl;
    ofile << "field.m_name = \"" << field->m_name << "\";" << std::endl;
    ofile << "field.m_fieldNum = " << field->m_fieldNum << ";" << std::endl;
    ofile << "field.m_msgType = \"" << field->m_msgType << "\";" << std::endl;
    ofile << "field.m_repeated = " << (field->m_repeated ? "true" : "false") << ";" << std::endl;
    ofile << "field.m_type = (core::util::FieldDef::eFieldType) " << ((int) field->m_type) << ";" << std::endl;
    ofile << "defSelf.m_fields.push_back(field);" << std::endl;
    ofile << "}" << std::endl;
  }
  ofile << "defSelf.m_enums.reserve(" << msgDef.m_enums.size() << ");" << std::endl;
  for (std::vector< EnumDef >::const_iterator itr = msgDef.m_enums.begin(); itr != msgDef.m_enums.end(); ++itr) {
    ofile << "{" << std::endl;
    ofile << "core::util::EnumDef enumDef;" << std::endl;
    ofile << "enumDef.m_name = \"" << itr->m_name << "\";" << std::endl;
    ofile << "enumDef.m_values.reserve(" << itr->m_values.size() << ");" << std::endl;
    for (std::vector< FieldDef >::const_iterator field = itr->m_values.begin(); field != itr->m_values.end(); ++field) {
      ofile << "{" << std::endl;
      ofile << "core::util::FieldDef field;" << std::endl;
      ofile << "field.m_name = \"" << field->m_name << "\";" << std::endl;
      ofile << "field.m_fieldNum = " << field->m_fieldNum << ";" << std::endl;
      ofile << "enumDef.m_values.push_back(field);" << std::endl;
      ofile << "}" << std::endl;
    }
    ofile << "defSelf.m_enums.push_back(enumDef);" << std::endl;
    ofile << "}" << std::endl;
  }

  ofile << "static core::util::ProtoDescriptor s_descriptor(defSelf);" << std::endl;
  ofile << "return s_descriptor;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "const core::util::ProtoDescriptor &GenDescriptor_" << genFunctionName << "() {" << std::endl;
  ofile << "return InternalGenDescriptor_" << genFunctionName << "();" << std::endl;
  ofile << "}" << std::endl;
  for (std::vector< MessageDef >::const_iterator message = msgDef.m_messages.begin(); message != msgDef.m_messages.end(); ++message) {
    printCppDescriptorGen(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 *
 */
void printCppVirtuals(std::ofstream &ofile, const MessageDef &msgDef, const std::string &package) {
  ofile << "size_t " << package << msgDef.m_name << "::byte_size() const {" << std::endl;
  ofile << "core::base::FakeSink sink;" << std::endl;
  ofile << "sink << *this;" << std::endl;
  ofile << "return sink.size();" << std::endl;
  ofile << "}" << std::endl;

  ofile << "bool " << package << msgDef.m_name << "::operator ==(const " << package << msgDef.m_name << " &other) const {" << std::endl;
  for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
    if (field->m_repeated) {
      ofile << "if (m_" << field->m_name << " != other.m_" << field->m_name << ") {\nreturn false;\n}" << std::endl;
    } else if (field->m_type == FieldDef::FIELD_MSG) {
      ofile << "if (has_" << field->m_name << "() != other.has_" << field->m_name << "()) {\nreturn false;\n}" << std::endl;
      ofile << "if (has_" << field->m_name << "() && (m_" << field->m_name << " != other.m_" << field->m_name << ")) {\nreturn false;\n}" << std::endl;
    } else {
      ofile << "if (m_" << field->m_name << " != other.m_" << field->m_name << ") {\nreturn false;\n}" << std::endl;
    }
  }
  ofile << "return true;" << std::endl;
  ofile << "}" << std::endl;

  ofile << "bool " << package << msgDef.m_name << "::oserialize(core::base::iBinarySerializerSink &sink) const {" << std::endl;
  ofile << "sink << *this;" << std::endl;
  ofile << "return !sink.fail();" << std::endl;
  ofile << "}" << std::endl;

  ofile << "bool " << package << msgDef.m_name << "::getField(const u32 fieldNum, std::string &value) const {" << std::endl;
  {
    u32 count = 0;
    for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
      if (field->m_repeated) {
        continue;
      }
      count++;
    }
    if (count > 0) {
      ofile << "switch (fieldNum) {" << std::endl;
      for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
        if (field->m_repeated) {
          continue;
        }
        ofile << "case " << field->m_fieldNum << ": {" << std::endl;
        if (field->m_type != FieldDef::FIELD_MSG) {
          ofile << "return core::util::lexical_cast(get_" << field->m_name << "(), value);" << std::endl;
        } else {
          ofile << "return core::util::files::TextFormat::format(value, get_" << field->m_name << "());" << std::endl;
        }
        ofile << "}" << std::endl;
      }
      ofile << "}" << std::endl;
    }
  }
  ofile << "return false;" << std::endl;
  ofile << "}" << std::endl;

  ofile << "bool " << package << msgDef.m_name << "::getField(const u32 fieldNum, const u32 index, std::string &value) const {" << std::endl;
  {
    u32 count = 0;
    for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
      if (!field->m_repeated) {
        continue;
      }
      count++;
    }
    if (count > 0) {
      ofile << "switch (fieldNum) {" << std::endl;
      for (std::vector< FieldDef >::const_iterator field = msgDef.m_fields.begin(); field != msgDef.m_fields.end(); ++field) {
        if (!field->m_repeated) {
          continue;
        }
        ofile << "case " << field->m_fieldNum << ": {" << std::endl;
        ofile << "if (index >= get_" << field->m_name << "_size()) {\nreturn false;\n}" << std::endl;
        if (field->m_type != FieldDef::FIELD_MSG) {
          ofile << "return core::util::lexical_cast(get_" << field->m_name << "(index), value);" << std::endl;
        } else {
          ofile << "return core::util::files::TextFormat::format(value, get_" << field->m_name << "(index));" << std::endl;
        }
        ofile << "}" << std::endl;
      }
      ofile << "}" << std::endl;
    }
  }
  ofile << "return false;" << std::endl;
  ofile << "}" << std::endl;


  ofile << "bool " << package << msgDef.m_name << "::iserialize(core::base::iBinarySerializerSink &sink) {" << std::endl;
  ofile << package << msgDef.m_name << "::Builder builder;" << std::endl;
  ofile << "sink >> builder;" << std::endl;
  ofile << "if (sink.fail()) {" << std::endl;
  ofile << "return false;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "*this = builder.build();" << std::endl;
  ofile << "return true;" << std::endl;
  ofile << "}" << std::endl;

  ofile << "const core::util::ProtoDescriptor &" << package << msgDef.m_name << "::getDescriptor() const {" << std::endl;
  ofile << "const core::util::ProtoDescriptor &descriptor = GenDescriptor_" << core::util::identifierSafe(package) <<  msgDef.m_name << "();" << std::endl;
  ofile << "return descriptor;" << std::endl;
  ofile << "}" << std::endl;
  for (std::vector< MessageDef >::const_iterator message = msgDef.m_messages.begin(); message != msgDef.m_messages.end(); ++message) {
    printCppVirtuals(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 *
 */
void printStaticInitializers(std::ofstream &ofile, const MessageDef &msgDef, const std::string &package) {
  const std::string genFunctionName = core::util::identifierSafe(package + msgDef.m_name);

  std::set< std::string > allIncluded;
  for (std::vector< MessageDef >::const_iterator msg = msgDef.m_messages.begin(); msg != msgDef.m_messages.end(); ++msg) {
    allIncluded.insert(core::util::identifierSafe(package + msgDef.m_name + "::" + msg->m_name));
  }

  ofile << "class StaticInitDescriptor_" << genFunctionName << " {" << std::endl;
  ofile << "public: StaticInitDescriptor_" << genFunctionName << "() {" << std::endl;
  ofile << "core::util::ProtoDescriptor &descriptor = InternalGenDescriptor_" << genFunctionName << "();" << std::endl;
  ofile << "std::vector<const core::util::ProtoDescriptor *> defChildren;" << std::endl;
  for (std::set< std::string >::const_iterator itr = allIncluded.begin(); itr != allIncluded.end(); ++itr) {
    ofile << "defChildren.push_back(&GenDescriptor_" << *itr << "());" << std::endl;
  }
  ofile << "descriptor = core::util::ProtoDescriptor(descriptor.getDef(), defChildren);" << std::endl;
  ofile << "core::util::RegisterWithProtoDb(&descriptor);" << std::endl;
  ofile << "}" << std::endl;
  ofile << "};" << std::endl;

  ofile << "static StaticInitDescriptor_" << genFunctionName << " temp_" << genFunctionName << ";" << std::endl;

  for (std::vector< MessageDef >::const_iterator message = msgDef.m_messages.begin(); message != msgDef.m_messages.end(); ++message) {
    printStaticInitializers(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 * Prints the cpp file portion of the proto
 */
bool printCpp(const ProtoDef &def, const std::string &headerName, const std::string &fileName) {
  std::ofstream ofile(fileName);
  if (!ofile.is_open()) {
    return false;
  }

  ofile << "#include \"" << headerName << "\"" << std::endl;
  ofile << "#include <CORE/UTIL/lexical_cast.h>" << std::endl;
  ofile << "#include <CORE/UTIL/FILES/proto_text.h>" << std::endl;
  if (!def.m_services.empty()) {
    ofile << "#include <WRAPPERS/NET/packet_handler.h>" << std::endl;
  }
  const std::vector< std::string > package = core::util::Splitter().on('.').split(def.m_package);

  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    printCppDescriptorGen(ofile, *message, core::util::replaceStr(def.m_package, ".", "::") + "::");
  }
  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    printStaticInitializers(ofile, *message, core::util::replaceStr(def.m_package, ".", "::") + "::");
  }

  printOpenNamespace(ofile, package);

  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    printCppVirtuals(ofile, *message, core::util::replaceStr(def.m_package, ".", "::") + "::");
  }

  for (std::vector< ServiceDef >::const_iterator service = def.m_services.begin(); service != def.m_services.end(); ++service) {
    printCppServiceHandlers(ofile, *service, core::util::replaceStr(def.m_package, ".", "::") + "::");
  }
  printCloseNamespace(ofile, package);
  return true;
}

/**
 * Opens a proto package definition
 *     package foo.bar;
 * as
 *     namespace foo {
 *     namespace bar {
 */
void printOpenNamespace(std::ofstream &ofile, const std::vector< std::string > &package) {
  for (std::vector< std::string >::const_iterator itr = package.begin(); itr != package.end(); ++itr) {
    ofile << "namespace " << *itr << " {" << std::endl;
  }
  ofile << std::endl;
}

/**
 * Closes a proto package definition
 *     package foo.bar;
 * as
 *     } // namespace bar
 *     } // namespace foo
 */
void printCloseNamespace(std::ofstream &ofile, const std::vector< std::string > &package) {
  for (std::vector< std::string >::const_iterator itr = package.begin(); itr != package.end(); ++itr) {
    ofile << "} // namespace " << *itr << std::endl;
  }
}

/**
 * Prints the imports needed for protos to work, as well as the imports specified in the proto file itself
 */
void printImports(std::ofstream &ofile, const std::vector< std::string > &imports, bool hasServices) {
  ofile << "#include <CORE/types.h>" << std::endl;
  ofile << "#include <CORE/BASE/checks.h>" << std::endl;
  ofile << "#include <CORE/CONTAINERS/bitset.h>" << std::endl;
  ofile << "#include <CORE/MEMORY/blob.h>" << std::endl;
  if (hasServices) {
    ofile << "#include <CORE/UTIL/protobuf_service.h>" << std::endl;
  } else {
    ofile << "#include <CORE/UTIL/protobuf.h>" << std::endl;
  }

  ofile << "#include <CORE/BASE/serializer_podtypes.h>" << std::endl;
  for (std::vector< std::string >::const_iterator itr = imports.begin(); itr != imports.end(); ++itr) {
    ofile << "#include <" << core::util::replaceStr(*itr, ".proto", ".pb.h") << ">" << std::endl;
  }
  ofile << std::endl;
}

/**
 *
 */
void printBase(std::ofstream &ofile, const std::string &name) {
  ofile << "public:" << std::endl;
  ofile << "virtual bool getField(const u32 fieldNum, std::string &value) const;"  << std::endl;
  ofile << "virtual bool getField(const u32 fieldNum, const u32 index, std::string &value) const;"  << std::endl;
  ofile << "virtual size_t byte_size() const;" << std::endl;
  ofile << "virtual bool oserialize(core::base::iBinarySerializerSink &) const;" << std::endl;
  ofile << "virtual bool iserialize(core::base::iBinarySerializerSink &);" << std::endl;
  ofile << "virtual const core::util::ProtoDescriptor &getDescriptor() const;" << std::endl;
  ofile << "bool operator ==(const " << name << " &other) const;" << std::endl;
  ofile << "bool operator !=(const " << name << " &other) const {\nreturn !(*this == other);\n}" << std::endl;
}

/**
 *
 */
void printDefaultInitializer(std::ofstream &ofile, const MessageDef &msgDef) {
  std::vector< std::string > fields;
  fields.reserve(msgDef.m_fields.size());
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    if (itr->m_repeated) {
      continue;
    }

    std::string field = "m_" + itr->m_name + "(" + defaultValue(itr->m_type, itr->m_msgType) + ")";
    fields.push_back(field);
  }

  if (!fields.empty()) {
    ofile << ":";
    ofile << core::util::Joiner().on(",\n").join(fields.begin(), fields.end());
  }
  ofile << " {\n};" << std::endl;
}

/**
 * Prints the header portion of a message.
 */
void printHeaderMessage(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "class " << msgDef.m_name << " : public ::core::util::iProtoMessage {" << std::endl;
  printBase(ofile, msgDef.m_name);
  printFieldEnum(ofile, msgDef);

  for (std::vector< EnumDef >::const_iterator itr = msgDef.m_enums.begin(); itr != msgDef.m_enums.end(); ++itr) {
    printEnum(ofile, *itr);
  }
  for (std::vector< MessageDef >::const_iterator itr = msgDef.m_messages.begin(); itr != msgDef.m_messages.end(); ++itr) {
    printHeaderMessage(ofile, *itr);
  }

  printBuilder(ofile, msgDef);

  ofile << "public:" << std::endl;
  ofile << msgDef.m_name << "()" << std::endl;
  printDefaultInitializer(ofile, msgDef);
// TODO(kulseran): Create rule3 when correct storage is in place
//  ofile << "~" << msgDef.m_name << "();" << std::endl;
//  ofile << msgDef.m_name << "(const " << msgDef.m_name << " &);" << std::endl;
//  ofile << msgDef.m_name << " &operator =(const " << msgDef.m_name << " &);" << std::endl;
  ofile << "private: " << std::endl;
  ofile << "friend class Builder;" << std::endl;
  ofile << msgDef.m_name << " (const Builder &other)" << std::endl;
  printInitilizerFromOther(ofile, msgDef);

  // TODO(kulseran): Replace with more efficient storage mechanism
  printBuilderStorage(ofile, msgDef);
  printBuilderGetters(ofile, msgDef);
  ofile << "};" << std::endl << std::endl;
}

/**
 *
 */
void printHeaderService(std::ofstream &ofile, const ServiceDef &srvDef) {
  ofile << "class " << srvDef.m_name << "Server : public ::core::util::iProtoServiceServer {" << std::endl;
  ofile << "protected:" << std::endl;
  ofile << srvDef.m_name << "Server(const ::core::util::thread::ThreadPool::PoolOptions &poolOptions) : ::core::util::iProtoServiceServer(poolOptions) { }" << std::endl;
  for (std::vector< RpcFunctionDef >::const_iterator itr = srvDef.m_functions.begin(); itr != srvDef.m_functions.end(); ++itr) {
    ofile << "virtual " << itr->m_return << " " << itr->m_name << "(const " << itr->m_param << " &param) = 0;" << std::endl;
  }
  ofile << "protected:\nvirtual bool process(core::net::iNetServer &server, const core::net::tConnectionId connectionId, const wrappers::net::util::Packet &) final;" << std::endl;
  ofile << "};" << std::endl;

  ofile << "class " << srvDef.m_name << "Client : public ::core::util::iProtoServiceClient {" << std::endl;
  ofile << "public:" << std::endl;
  for (std::vector< RpcFunctionDef >::const_iterator itr = srvDef.m_functions.begin(); itr != srvDef.m_functions.end(); ++itr) {
    ofile << itr->m_return << " " << itr->m_name << "(const " << itr->m_param << " &param);" << std::endl;
    ofile << "typedef srutil::delegate<void(const u16, const " << itr->m_return << " &)> t" << itr->m_name << "Callback;" << std::endl;
    ofile << "u16 " << itr->m_name << "Async(const " << itr->m_param << " &param, t" << itr->m_name << "Callback *proc);" << std::endl;
  }
  ofile << "};" << std::endl;
}

/**
 *
 */
void printCppServiceHandlers(std::ofstream &ofile, const ServiceDef &srvDef, const std::string &package) {
  ofile << "bool " << package << srvDef.m_name << "Server::process(core::net::iNetServer &server, const core::net::tConnectionId connectionId, const wrappers::net::util::Packet &packet) {" << std::endl;
  ofile << "switch (packet.getHeader().m_type) {" << std::endl;

  for (std::vector< RpcFunctionDef >::const_iterator itr = srvDef.m_functions.begin(); itr != srvDef.m_functions.end(); ++itr) {
    const u16 sig = static_cast<u16>(core::hash::CRC32(itr->m_name.begin(), itr->m_name.end()));
    ofile << "case " << sig << ": {" << std::endl;
    ofile << itr->m_param << " msgIn;" << std::endl;
    ofile << "if (!core::util::PacketToProto(packet, msgIn)) {\nreturn false;\n}" << std::endl;
    ofile << itr->m_return << " ret = " << itr->m_name << "(msgIn);" << std::endl;
    ofile << "wrappers::net::util::Packet retPacket;" << std::endl;
    ofile << "retPacket.setType(packet.getHeader().m_type);" << std::endl;
    ofile << "retPacket.setIndex(packet.getHeader().m_index);" << std::endl;
    ofile << "if (!core::util::ProtoToPacket(ret, retPacket)) {\nreturn false;\n}" << std::endl;
    ofile << "wrappers::net::util::SendPacket(server, connectionId, retPacket);" << std::endl;
    ofile << "break;" << std::endl;
    ofile << "}" << std::endl;
  }

  ofile << "default:\nreturn false;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "return true;" << std::endl;
  ofile << "}" << std::endl;

  for (std::vector< RpcFunctionDef >::const_iterator itr = srvDef.m_functions.begin(); itr != srvDef.m_functions.end(); ++itr) {
    const u16 sig = static_cast<u16>(core::hash::CRC32(itr->m_name.begin(), itr->m_name.end()));

    ofile << itr->m_return << " " << package << srvDef.m_name << "Client::" << itr->m_name << "(const " << itr->m_param << " &param) {" << std::endl;
    ofile << "wrappers::net::util::Packet packetSend;" << std::endl;
    ofile << "if (!core::util::ProtoToPacket(param, packetSend)) {" << std::endl;
    ofile << "return " << itr->m_return << "();" << std::endl;
    ofile << "}" << std::endl;
    ofile << "packetSend.setType(" << sig << ");" << std::endl;
    ofile << "wrappers::net::util::Packet packetRecv;" << std::endl;
    ofile << "if (doSyncRpc(packetSend, packetRecv)) {" << std::endl;
    ofile << itr->m_return << " ret;" << std::endl;
    ofile << "if (core::util::PacketToProto(packetRecv, ret)) {" << std::endl;
    ofile << "return ret;" << std::endl;
    ofile << "}" << std::endl;
    ofile << "}" << std::endl;
    ofile << "return " << itr->m_return << "();" << std::endl;
    ofile << "}" << std::endl;

    ofile << "u16 " << package << srvDef.m_name << "Client::" << itr->m_name << "Async(const " << itr->m_param << " &param, srutil::delegate<void(const u16, const " << itr->m_return << " &)> *proc) {" << std::endl;
    ofile << "wrappers::net::util::Packet packetSend;" << std::endl;
    ofile << "if (!core::util::ProtoToPacket(param, packetSend)) {" << std::endl;
    ofile << "return 0;" << std::endl;
    ofile << "}" << std::endl;
    ofile << "packetSend.setType(" << sig << ");" << std::endl;
    ofile << "if (proc) {" << std::endl;
    ofile << "ConvertCallback<" << itr->m_return << "> *cb = new ConvertCallback<" << itr->m_return << ">(*proc);" << std::endl;
    ofile << "tCallback callback = tCallback::from_method<ConvertCallback<" << itr->m_return << ">, &ConvertCallback<" << itr->m_return << ">::process>(cb);" << std::endl;
    ofile << "return doAsyncRpc(packetSend, &callback);" << std::endl;
    ofile << "} else {" << std::endl;
    ofile << "return doAsyncRpc(packetSend, nullptr);" << std::endl;
    ofile << "}" << std::endl;
    ofile << "}" << std::endl;
  }
}

/**
 * Prints an enum
 */
void printEnum(std::ofstream &ofile, const EnumDef &enumDef) {
  ofile << "public: enum " << enumDef.m_name << " {" << std::endl;
  ofile << enumDef.m_name << "_UNKNOWN = 0," << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = enumDef.m_values.begin(); itr != enumDef.m_values.end(); ++itr) {
    ofile << itr->m_name << " = " << itr->m_fieldNum << "," << std::endl;
  }
  ofile << enumDef.m_name << "_COUNT = " << enumDef.m_values.size() + 1 << "," << std::endl;
  ofile << "};" << std::endl << std::endl;
}

/**
 * Outputs the builder class
 */
void printBuilder(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public: class Builder {" << std::endl;
  ofile << "public:" << std::endl;
  ofile << "Builder()" << std::endl;
  printDefaultInitializer(ofile, msgDef);
  ofile << "Builder(const " << msgDef.m_name << "&other)" << std::endl;
  printInitilizerFromOther(ofile, msgDef);

  printBuilderSetters(ofile, msgDef);
  printBuilderGetters(ofile, msgDef);
  ofile << msgDef.m_name << " build() const { return " << msgDef.m_name << "(*this); }" << std::endl;
  printBuilderStorage(ofile, msgDef);
  ofile << "friend class " << msgDef.m_name << ";" << std::endl;
  ofile << "};" << std::endl << std::endl;
}

void printBuilderSetters(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public:" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    if (itr->m_repeated) {
      ofile << "Builder &add_" << itr->m_name << "(const " << getBuilderType(itr->m_type, itr->m_msgType) << " &value) { m_" << itr->m_name << ".push_back(value); return *this; }" << std::endl;
      ofile << "Builder &set_" << itr->m_name << "(const " << getBuilderType(itr->m_type, itr->m_msgType) << " &value, const u32 index) { CHECK(index < m_" << itr->m_name << ".size()); m_" << itr->m_name << "[index] = value; return *this; }" << std::endl;
      ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name << ".clear(); return *this; }" << std::endl;
    } else {
      ofile << "Builder &set_" << itr->m_name << "(const " << getBuilderType(itr->m_type, itr->m_msgType) << " &value) { m_" << itr->m_name << " = value; ";
      if (itr->m_type == FieldDef::FIELD_MSG) {
        ofile << "m_has_" << itr->m_name << " = true; return *this; }" << std::endl;
        ofile << "Builder &clear_" << itr->m_name << "() { m_has_" << itr->m_name << " = false; return *this; }" << std::endl;
      } else if (itr->m_repeated) {
        ofile << "return *this; }" << std::endl;
        ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name << ".clear(); return *this; }" << std::endl;
      } else {
        ofile << "return *this; }" << std::endl;
        ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name << " = " << defaultValue(itr->m_type, itr->m_msgType) << "; return *this; }" << std::endl;
      }
    }
  }
}

void printBuilderGetters(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public:" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    if (itr->m_repeated) {
      ofile << "const " << getBuilderType(itr->m_type, itr->m_msgType) << " &get_" << itr->m_name << "(const u32 index) const { CHECK(index < m_" << itr->m_name << ".size()); return m_" << itr->m_name << "[index]; }" << std::endl;
      ofile << "u32 get_" << itr->m_name << "_size() const { return m_" << itr->m_name << ".size(); }" << std::endl;
      ofile << "typedef std::vector< " << getBuilderType(itr->m_type, itr->m_msgType) << " > t" << itr->m_name << "List;" << std::endl;
      ofile << "t" << itr->m_name << "List::const_iterator get_" << itr->m_name << "_begin() const { return m_" << itr->m_name << ".begin(); }" << std::endl;
      ofile << "t" << itr->m_name << "List::const_iterator get_" << itr->m_name << "_end() const { return m_" << itr->m_name << ".end(); }" << std::endl;
    } else {
      ofile << "const " << getBuilderType(itr->m_type, itr->m_msgType) << " &get_" << itr->m_name << "() const { return m_" << itr->m_name << "; }" << std::endl;
      if (itr->m_type == FieldDef::FIELD_MSG) {
        ofile << "const bool has_" << itr->m_name << "() const { return m_has_" << itr->m_name << "; }" << std::endl;
      }
    }
  }
}

void printFieldEnum(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public: struct eFields {" << std::endl;
  ofile << "enum type {" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    ofile << "FIELD_" << itr->m_name << " = " << itr->m_fieldNum << "," << std::endl;
  }
  ofile << "COUNT" << std::endl;
  ofile << "};" << std::endl;
  ofile << "};" << std::endl;
  ofile << std::endl;
}

/**
 * Return the type of object for use in the builder classes.
 */
std::string getBuilderType(const FieldDef::eFieldType type, const std::string &msgName) {
  static const std::pair< FieldDef::eFieldType, const char * > s_typeMap[15] = {
    std::make_pair(FieldDef::FIELD_DOUBLE, "double"),
    std::make_pair(FieldDef::FIELD_FLOAT, "float"),
    std::make_pair(FieldDef::FIELD_INT32, "s32"),
    std::make_pair(FieldDef::FIELD_INT64, "s64"),
    std::make_pair(FieldDef::FIELD_UINT32, "u32"),
    std::make_pair(FieldDef::FIELD_UINT64, "u64"),
    std::make_pair(FieldDef::FIELD_SINT32, "s32"),
    std::make_pair(FieldDef::FIELD_SINT64, "s64"),
    std::make_pair(FieldDef::FIELD_FIXED32, "u32"),
    std::make_pair(FieldDef::FIELD_FIXED64, "u64"),
    std::make_pair(FieldDef::FIELD_SFIXED32, "s32"),
    std::make_pair(FieldDef::FIELD_SFIXED64, "s64"),
    std::make_pair(FieldDef::FIELD_BOOL, "bool"),
    std::make_pair(FieldDef::FIELD_STRING, "std::string"),
    std::make_pair(FieldDef::FIELD_BYTES, "std::string")
  };
  for (unsigned i = 0; i < ARRAY_LENGTH(s_typeMap); ++i) {
    if (s_typeMap[i].first == type) {
      return s_typeMap[i].second;
    }
  }

  return msgName;
}

/**
 *
 */
void printBuilderStorage(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "private:" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    if (itr->m_repeated) {
      ofile << "std::vector< " << getBuilderType(itr->m_type, itr->m_msgType) << " > m_" << itr->m_name << ";" << std::endl;
    } else {
      ofile << getBuilderType(itr->m_type, itr->m_msgType) << " m_" << itr->m_name << ";" << std::endl;
    }
  }
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    if (itr->m_type == FieldDef::FIELD_MSG) {
      ofile << "bool m_has_" << itr->m_name << ";" << std::endl;
    }
  }
}

/**
 *
 */
void printInitilizerFromOther(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "{" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    ofile << "m_" << itr->m_name << " = other.m_" << itr->m_name << ";" << std::endl;
    if (itr->m_type == FieldDef::FIELD_MSG) {
      ofile << "m_has_" << itr->m_name << " = other.m_has_" << itr->m_name << ";" << std::endl;
    }
  }
  ofile << "}" << std::endl;
}

/**
 *
 */
u32 getFieldTypeId(FieldDef::eFieldType e) {
  switch (e) {
    case FieldDef::FIELD_FLOAT:
    case FieldDef::FIELD_FIXED32:
    case FieldDef::FIELD_SFIXED32:
      return 5;
    case FieldDef::FIELD_FIXED64:
    case FieldDef::FIELD_SFIXED64:
    case FieldDef::FIELD_DOUBLE:
      return 1;
    case FieldDef::FIELD_INT32:
    case FieldDef::FIELD_INT64:
    case FieldDef::FIELD_UINT32:
    case FieldDef::FIELD_UINT64:
    case FieldDef::FIELD_SINT32:
    case FieldDef::FIELD_SINT64:
    case FieldDef::FIELD_ENUM:
    case FieldDef::FIELD_BOOL:
      return 0;
    case FieldDef::FIELD_STRING:
    case FieldDef::FIELD_BYTES:
    case FieldDef::FIELD_MSG:
      return 2;
  }
  CHECK(false);
  return -1;
}

std::string defaultValue(const FieldDef::eFieldType type, const std::string msgType) {
  switch (type) {
    case FieldDef::FIELD_FLOAT:
      return "0.0f";
    case FieldDef::FIELD_DOUBLE:
      return "0.0";
    case FieldDef::FIELD_FIXED32:
      return "0ul";
    case FieldDef::FIELD_SFIXED32:
      return "0l";
    case FieldDef::FIELD_FIXED64:
      return "0ull";
    case FieldDef::FIELD_SFIXED64:
      return "0ll";
    case FieldDef::FIELD_INT32:
      return "0l";
    case FieldDef::FIELD_INT64:
      return "0ll";
    case FieldDef::FIELD_UINT32:
      return "0ul";
    case FieldDef::FIELD_UINT64:
      return "0ull";
    case FieldDef::FIELD_SINT32:
      return "0l";
    case FieldDef::FIELD_SINT64:
      return "0ll";
    case FieldDef::FIELD_ENUM:
      return "(" + msgType + ") 0u";
    case FieldDef::FIELD_BOOL:
      return "false";
    case FieldDef::FIELD_STRING:
      return "\"\"";
    case FieldDef::FIELD_BYTES:
      return "\"\"";
  }
  return "";
}

/**
 *
 */
void printOSerializer(std::ofstream &ofile, const std::vector< std::string > &package, const MessageDef &msgDef) {
  for (std::vector< MessageDef >::const_iterator itr = msgDef.m_messages.begin(); itr != msgDef.m_messages.end(); ++itr) {
    std::vector< std::string > subPackage = package;
    subPackage.push_back(msgDef.m_name);
    printOSerializer(ofile, subPackage, *itr);
  }
  ofile << "OSERIALIZE(" << core::util::Joiner().on("::").join(package.begin(), package.end()) << "::" << msgDef.m_name << ") {" << std::endl;
  for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
    const u32 tag = (itr->m_fieldNum << 3) | getFieldTypeId(itr->m_type);
    if (itr->m_repeated) {
      ofile << "for (u32 i = 0; i < obj.get_" << itr->m_name << "_size(); ++i) {" << std::endl;
      ofile << "buff << VarUInt(" << tag << "ull);" << std::endl;
    } else if (itr->m_type == FieldDef::FIELD_MSG) {
      ofile << "if (obj.has_" << itr->m_name << "()) {" << std::endl;
      ofile << "buff << VarUInt(" << tag << "ull);" << std::endl;
    } else {
      ofile << "if (obj.get_" << itr->m_name << "() != " << defaultValue(itr->m_type, itr->m_msgType) << ") {" << std::endl;
      ofile << "buff << VarUInt(" << tag << "ull);" << std::endl;
    }
    const std::string index = itr->m_repeated ? "(i)" : "()";
    switch (itr->m_type) {
      case FieldDef::FIELD_FLOAT:
      case FieldDef::FIELD_FIXED32:
      case FieldDef::FIELD_SFIXED32:
      case FieldDef::FIELD_FIXED64:
      case FieldDef::FIELD_SFIXED64:
      case FieldDef::FIELD_DOUBLE:
        ofile << "buff << obj.get_" << itr->m_name << index << ";" << std::endl;
        break;
      case FieldDef::FIELD_INT32:
      case FieldDef::FIELD_INT64:
      case FieldDef::FIELD_SINT32:
      case FieldDef::FIELD_SINT64:
        ofile << "buff << VarInt(obj.get_" << itr->m_name << index << ");" << std::endl;
        break;
      case FieldDef::FIELD_BOOL:
        ofile << "buff << VarInt(obj.get_" << itr->m_name << index << " ? 1 : 0);" << std::endl;
        break;
      case FieldDef::FIELD_UINT32:
      case FieldDef::FIELD_UINT64:
        ofile << "buff << VarUInt(obj.get_" << itr->m_name << index << ");" << std::endl;
        break;
      case FieldDef::FIELD_ENUM:
        ofile << "buff << VarUInt((u64) obj.get_" << itr->m_name << index << ");" << std::endl;
        break;
      case FieldDef::FIELD_STRING:
      case FieldDef::FIELD_BYTES:
        ofile << "buff << VarUInt(obj.get_" << itr->m_name << index << ".size());" << std::endl;
        ofile << "buff.write(::core::memory::ConstBlob((const u8 *)obj.get_" << itr->m_name << index << ".data(), obj.get_" << itr->m_name << index << ".size()));" << std::endl;
        break;
      case FieldDef::FIELD_MSG:
        ofile << "::core::base::FakeSink sink;" << std::endl;
        ofile << "sink << obj.get_" << itr->m_name << index << ";" << std::endl;
        ofile << "buff << VarUInt(sink.size());" << std::endl;
        ofile << "buff << obj.get_" << itr->m_name << index << ";" << std::endl;
        break;
      default:
        CHECK(false);
    }

    ofile << "}" << std::endl;
  }
  ofile << "return buff;" << std::endl;
  ofile << "}" << std::endl;
}

/**
 *
 */
void printISerializer(std::ofstream &ofile, const std::vector< std::string > &package, const MessageDef &msgDef) {
  for (std::vector< MessageDef >::const_iterator itr = msgDef.m_messages.begin(); itr != msgDef.m_messages.end(); ++itr) {
    std::vector< std::string > subPackage = package;
    subPackage.push_back(msgDef.m_name);
    printISerializer(ofile, subPackage, *itr);
  }
  ofile << "ISERIALIZE(" << core::util::Joiner().on("::").join(package.begin(), package.end()) << "::" << msgDef.m_name << "::Builder) {" << std::endl;
  ofile << "while(buff.avail() && !buff.fail()) {" << std::endl;
  ofile << "VarUInt tag;" << std::endl;
  ofile << "buff >> tag;" << std::endl;
  ofile << "const u32 fieldNum = ((u32) tag.get()) >> 3;" << std::endl;
  ofile << "const u32 fieldType = ((u32) tag.get()) & 0x7;" << std::endl;

  if (!msgDef.m_fields.empty()) {
    ofile << "switch (fieldNum) {" << std::endl;
    for (std::vector< FieldDef >::const_iterator itr = msgDef.m_fields.begin(); itr != msgDef.m_fields.end(); ++itr) {
      const u32 tag = (itr->m_fieldNum << 3) | getFieldTypeId(itr->m_type);
      ofile << "case " << itr->m_fieldNum << ": {" << std::endl;

      const std::string method = itr->m_repeated ? "add_" : "set_";

      switch (itr->m_type) {
        case FieldDef::FIELD_FLOAT:
          ofile << "float tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_FIXED32:
          ofile << "u32 tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_SFIXED32:
          ofile << "s32 tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_FIXED64:
          ofile << "u64 tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_SFIXED64:
          ofile << "s64 tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_DOUBLE:
          ofile << "double tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          break;
        case FieldDef::FIELD_INT32:
        case FieldDef::FIELD_SINT32:
          ofile << "VarInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "((s32) tmp.get());" << std::endl;
          break;
        case FieldDef::FIELD_INT64:
        case FieldDef::FIELD_SINT64:
          ofile << "VarInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp.get());" << std::endl;
          break;
        case FieldDef::FIELD_BOOL:
          ofile << "VarInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp.get() != 0);" << std::endl;
          break;
        case FieldDef::FIELD_UINT32:
          ofile << "VarUInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "((u32) tmp.get());" << std::endl;
          break;
        case FieldDef::FIELD_UINT64:
          ofile << "VarUInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp.get());" << std::endl;
          break;
        case FieldDef::FIELD_ENUM:
          ofile << "VarUInt tmp;" << std::endl;
          ofile << "buff >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "((" << itr->m_msgType << ") tmp.get());" << std::endl;
          break;

        case FieldDef::FIELD_STRING:
        case FieldDef::FIELD_BYTES:
          ofile << "VarUInt tmpSz;" << std::endl;
          ofile << "buff >> tmpSz;" << std::endl;
          ofile << "if (!buff.fail()) {" << std::endl;
          ofile << "if (tmpSz.get() > (1 << 31)) {" << std::endl;
          ofile << "buff.set_fail();" << std::endl;
          ofile << "} else {" << std::endl;
          ofile << "char *tmpStr = new char[(size_t) tmpSz.get()];" << std::endl;
          ofile << "::core::memory::Blob blob((u8 *) tmpStr, (size_t) tmpSz.get());" << std::endl;
          ofile << "buff.read(blob);" << std::endl;
          ofile << "std::string tmp(tmpStr, tmpStr + tmpSz.get());" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp);" << std::endl;
          ofile << "}" << std::endl;
          ofile << "}" << std::endl;
          break;
        case FieldDef::FIELD_MSG:
          ofile << "VarUInt tmpSz;" << std::endl;
          ofile << "buff >> tmpSz;" << std::endl;
          ofile << "if (!buff.fail()) {" << std::endl;
          ofile << "::core::base::RangeSink sink(buff, (size_t) tmpSz.get());" << std::endl;
          ofile << itr->m_msgType << "::Builder tmp;" << std::endl;
          ofile << "sink >> tmp;" << std::endl;
          ofile << "obj." << method << itr->m_name << "(tmp.build());" << std::endl;
          ofile << "}" << std::endl;
          break;
        default:
          CHECK(false);
      }

      ofile << "continue;" << std::endl;
      ofile << "}" << std::endl;
    }
    ofile << "}" << std::endl;
  }

  ofile << "switch (fieldType) {" << std::endl;
  ofile << "case 0: {" << std::endl;
  ofile << "VarInt temp;" << std::endl;
  ofile << "buff >> temp;" << std::endl;
  ofile << "break;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "case 1: {" << std::endl;
  ofile << "buff.seek(sizeof(u64));" << std::endl;
  ofile << "break;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "case 2: {" << std::endl;
  ofile << "VarInt temp;" << std::endl;
  ofile << "buff >> temp;" << std::endl;
  ofile << "buff.seek((size_t) temp.get());" << std::endl;
  ofile << "break;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "case 5: {" << std::endl;
  ofile << "buff.seek(sizeof(u32));" << std::endl;
  ofile << "break;" << std::endl;
  ofile << "}" << std::endl;
  ofile << "}" << std::endl;

  ofile << "}" << std::endl;

  ofile << "return buff;" << std::endl;
  ofile << "}" << std::endl;
}
