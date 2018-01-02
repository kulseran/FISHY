#include "proto_printer_source.h"

#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/HASH/crc32.h>
#include <CORE/UTIL/stringutil.h>

#include <fstream>
#include <set>
#include <string>

using core::base::BlobSink;
using core::memory::Blob;
using core::types::EnumDef;
using core::types::FieldDef;
using core::types::MessageDef;
using core::types::ProtoDef;
using core::types::RpcFunctionDef;
using core::types::ServiceDef;
using core::types::tEnumList;
using core::types::tFieldList;
using core::types::tServiceList;

/**
 *
 */
void printCppDescriptorGen(
    std::ofstream &ofile,
    const MessageDef &msgDef,
    const std::string &package) {
  const std::string genFunctionName =
      core::util::IdentifierSafe(package + msgDef.m_name);
  ofile << "static ::core::types::ProtoDescriptor InternalGenDescriptor_"
        << genFunctionName << "() {\n";

  core::base::FakeSink sizer;
  sizer << msgDef;

  std::string buffer;
  buffer.resize(sizer.size());
  Blob bufferBlob(buffer);
  BlobSink sink(bufferBlob);
  sink << msgDef;
  ASSERT(sizer.size() > 0);

  ofile << "static const u8 data[" << sizer.size() << "] = {";
  {
    char hex[3];
    snprintf(hex, 3, "%02x", buffer[0]);
    ofile << "'\\x" << hex << "'";
    for (size_t i = 1; i < sizer.size(); ++i) {
      char hex[3];
      snprintf(hex, 3, "%02x", buffer[i]);
      ofile << ", '\\x" << hex << "'";
    }
  }
  ofile << "};\n";
  ofile << "::core::memory::ConstBlob blob(data, ARRAY_LENGTH(data));\n";
  ofile << "::core::base::ConstBlobSink sink(blob);\n";
  ofile << "::core::types::MessageDef defSelf;\n";
  ofile << "sink >> defSelf;\n";
  ofile << "CHECK(!sink.fail());\n";
  ofile << "return ::core::types::ProtoDescriptor(defSelf);\n";
  ofile << "}\n";

  ofile << "static const ::core::types::ProtoDescriptor &GenDescriptor_"
        << genFunctionName << "() {\n";
  ofile << "static ::core::types::ProtoDescriptor descriptor = "
           "InternalGenDescriptor_"
        << genFunctionName << "();\n";
  ofile << "return descriptor;\n";
  ofile << "}\n";

  for (std::vector< MessageDef >::const_iterator message =
           msgDef.m_messages.begin();
       message != msgDef.m_messages.end();
       ++message) {
    printCppDescriptorGen(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 *
 */
void printStaticInitializers(
    std::ofstream &ofile,
    const MessageDef &msgDef,
    const std::string &package) {
  const std::string genFunctionName =
      core::util::IdentifierSafe(package + msgDef.m_name);

  ofile << "class StaticInitDescriptor_" << genFunctionName << " {\n";
  ofile << "public: StaticInitDescriptor_" << genFunctionName << "() {\n";
  ofile << "const ::core::types::ProtoDescriptor &descriptor = "
           "GenDescriptor_"
        << genFunctionName << "();\n";
  ofile << "::core::types::RegisterWithProtoDb(&descriptor);\n";
  ofile << "}\n";
  ofile << "};\n";

  ofile << "static StaticInitDescriptor_" << genFunctionName << " temp_"
        << genFunctionName << ";\n";

  for (std::vector< MessageDef >::const_iterator message =
           msgDef.m_messages.begin();
       message != msgDef.m_messages.end();
       ++message) {
    printStaticInitializers(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 *
 */
static void printCppVirtuals(
    std::ofstream &ofile,
    const MessageDef &msgDef,
    const std::string &package) {

  // Operator ==
  ofile << "bool " << package << msgDef.m_name << "::operator ==(const "
        << package << msgDef.m_name << " &other) const {\n";
  for (tFieldList::const_iterator field = msgDef.m_fields.begin();
       field != msgDef.m_fields.end();
       ++field) {
    if (field->m_repeated) {
      ofile << "if (m_" << field->m_name << " != other.m_" << field->m_name
            << ") {\nreturn false;\n}\n";
    } else if (field->m_type == FieldDef::FIELD_MSG) {
      ofile << "if (has_" << field->m_name << "() != other.has_"
            << field->m_name << "()) {\nreturn false;\n}\n";
      ofile << "if (has_" << field->m_name << "() && (m_" << field->m_name
            << " != other.m_" << field->m_name << ")) {\nreturn false;\n}\n";
    } else {
      ofile << "if (m_" << field->m_name << " != other.m_" << field->m_name
            << ") {\nreturn false;\n}\n";
    }
  }
  ofile << "return true;\n";
  ofile << "}\n\n";

  // Operator !=
  ofile << "bool " << package << msgDef.m_name << "::operator !=(const "
        << package << msgDef.m_name << " &other) const {\n";
  ofile << "return !(*this == other);\n}\n\n";

  // Serializers
  ofile << "size_t " << package << msgDef.m_name << "::byte_size() const {\n";
  ofile << "::core::base::FakeSink sink;\n";

  ofile << "sink << *this;\n";
  ofile << "return sink.size();\n";
  ofile << "}\n\n";

  ofile << "bool " << package << msgDef.m_name
        << "::oserialize(::core::base::iBinarySerializerSink &sink) const {\n";
  ofile << "sink << *this;\n";
  ofile << "return !sink.fail();\n";
  ofile << "}\n";

  ofile << "bool " << package << msgDef.m_name
        << "::iserialize(::core::base::iBinarySerializerSink &sink) {\n";
  ofile << package << msgDef.m_name << "::Builder builder;\n";
  ofile << "sink >> builder;\n";
  ofile << "if (sink.fail()) {\n";
  ofile << "return false;\n";
  ofile << "}\n";
  ofile << "*this = builder.build();\n";
  ofile << "return true;\n";
  ofile << "}\n";

  ofile << "const void *" << package << msgDef.m_name
        << "::getField(const u32 fieldNum) const {\n";
  {
    int count = 0;
    for (tFieldList::const_iterator field = msgDef.m_fields.begin();
         field != msgDef.m_fields.end();
         ++field) {
      if (!field->m_repeated) {
        count++;
      }
    }

    if (count > 0) {
      ofile << "switch (fieldNum) {\n";
      for (tFieldList::const_iterator field = msgDef.m_fields.begin();
           field != msgDef.m_fields.end();
           ++field) {
        if (field->m_repeated) {
          continue;
        }
        ofile << "case " << field->m_fieldNum << ": {\n";
        ofile << "return &m_" << field->m_name << ";\n";
        ofile << "}\n";
      }
      ofile << "}\n";
    }
  }
  ofile << "return false;\n";
  ofile << "}\n";

  ofile << "const void *" << package << msgDef.m_name
        << "::getField(const u32 fieldNum, const size_t index) const {\n";
  {
    int count = 0;
    for (tFieldList::const_iterator field = msgDef.m_fields.begin();
         field != msgDef.m_fields.end();
         ++field) {
      if (field->m_repeated) {
        count++;
      }
    }
    if (count > 0) {
      ofile << "switch (fieldNum) {\n";
      for (tFieldList::const_iterator field = msgDef.m_fields.begin();
           field != msgDef.m_fields.end();
           ++field) {
        if (!field->m_repeated) {
          continue;
        }
        ofile << "case " << field->m_fieldNum << ": {\n";
        ofile << "if (index >= get_" << field->m_name
              << "_size()) {\nreturn nullptr;\n}\n";
        ofile << "return &m_" << field->m_name << "[index];\n";
        ofile << "}\n";
      }
      ofile << "}\n";
    }
  }
  ofile << "return nullptr;\n";
  ofile << "}\n";

  ofile << "const ::core::types::ProtoDescriptor &" << package << msgDef.m_name
        << "::getDescriptor() const {\n";
  ofile << "return "
           "GenDescriptor_"
        << core::util::IdentifierSafe(package) << msgDef.m_name << "();\n";
  ofile << "}\n";

  ofile << "\n";
  for (std::vector< MessageDef >::const_iterator message =
           msgDef.m_messages.begin();
       message != msgDef.m_messages.end();
       ++message) {
    printCppVirtuals(ofile, *message, package + msgDef.m_name + "::");
  }
}

/**
 * Prints the cpp file portion of the proto
 */
bool PrintCpp(
    const ProtoDef &def,
    const std::string &headerName,
    const std::string &fileName) {
  std::ofstream ofile(fileName);
  if (!ofile.is_open()) {
    return false;
  }

  ofile << "#include \"" << headerName << "\"\n";
  ofile << "#include <CORE/UTIL/lexical_cast.h>\n";
  if (!def.m_services.empty()) {
    ofile << "#include <WRAPPERS/NET/packet_handler.h>\n";
  }
  ofile << "\n";

  const std::vector< std::string > package =
      core::util::Splitter().on('.').split(def.m_package);

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printCppDescriptorGen(
        ofile,
        *message,
        core::util::ReplaceStr(def.m_package, ".", "::") + "::");
  }

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printStaticInitializers(
        ofile,
        *message,
        core::util::ReplaceStr(def.m_package, ".", "::") + "::");
  }

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printCppVirtuals(
        ofile,
        *message,
        core::util::ReplaceStr(def.m_package, ".", "::") + "::");
  }

  /*
  for (std::vector< ServiceDef >::const_iterator service =
           def.m_services.begin();
       service != def.m_services.end();
       ++service) {
    printCppServiceHandlers(
        ofile,
        *service,
        core::util::ReplaceStr(def.m_package, ".", "::") + "::");
  }
  */
  return true;
}
