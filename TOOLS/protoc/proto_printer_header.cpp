#include "proto_printer_header.h"

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

/**
 * Return the type of object for use in the builder classes.
 */
static std::string
getFieldType(const FieldDef::eFieldType type, const std::string &msgName) {
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
      std::make_pair(FieldDef::FIELD_BYTES, "std::string")};
  for (unsigned i = 0; i < ARRAY_LENGTH(s_typeMap); ++i) {
    if (s_typeMap[i].first == type) {
      return s_typeMap[i].second;
    }
  }

  return msgName;
}

/**
 * Return the default value for a given type.
 */
std::string
defaultValue(const FieldDef::eFieldType type, const std::string msgType) {
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
 * Prints the imports needed for protos to work, as well as the imports
 * specified in the proto file itself
 */
static void printImports(
    std::ofstream &ofile,
    const std::vector< std::string > &imports,
    bool hasServices) {
  ofile << "#include <CORE/types.h>\n";
  ofile << "#include <CORE/BASE/checks.h>\n";
  ofile << "#include <CORE/BASE/serializer_podtypes.h>\n";
  ofile << "#include <CORE/MEMORY/blob.h>\n";
  ofile << "\n";

  if (hasServices) {
    ofile << "#include <CORE/UTIL/protobuf_service.h>\n";
  } else {
    ofile << "#include <CORE/TYPES/protobuf.h>\n";
  }

  ofile << "\n";
  for (std::vector< std::string >::const_iterator itr = imports.begin();
       itr != imports.end();
       ++itr) {
    ofile << "#include <" << core::util::ReplaceStr(*itr, ".proto", ".pb.h")
          << ">\n";
  }
  ofile << "\n";
}

/**
 * Opens a proto package definition
 *     package foo.bar;
 * as
 *     namespace foo {
 *     namespace bar {
 */
static void printOpenNamespace(
    std::ofstream &ofile, const std::vector< std::string > &package) {
  for (std::vector< std::string >::const_iterator itr = package.begin();
       itr != package.end();
       ++itr) {
    ofile << "namespace " << *itr << " {\n";
  }
  ofile << "\n";
}

/**
 * Closes a proto package definition
 *     package foo.bar;
 * as
 *     } // namespace bar
 *     } // namespace foo
 */
static void printCloseNamespace(
    std::ofstream &ofile, const std::vector< std::string > &package) {
  for (std::vector< std::string >::const_iterator itr = package.begin();
       itr != package.end();
       ++itr) {
    ofile << "} // namespace " << *itr << "\n";
  }
  ofile << "\n";
}

/**
 * Prints the base function definitions needed for the class.
 */
static void printBase(std::ofstream &ofile, const std::string &name) {
  ofile << "public:\n";

  ofile << "virtual const void *getField(const u32 fieldNum) const override;\n";
  ofile << "virtual const void *getField(const u32 fieldNum, const size_t "
           "index) const override;\n";

  ofile << "virtual size_t byte_size() const;\n";
  ofile
      << "virtual bool oserialize(::core::base::iBinarySerializerSink &) const "
         "override;\n";
  ofile << "virtual bool iserialize(::core::base::iBinarySerializerSink &) "
           "override;\n";
  ofile
      << "virtual const ::core::types::ProtoDescriptor &getDescriptor() const "
         "override;\n";
  ofile << "bool operator ==(const " << name << " &other) const;\n";
  ofile << "bool operator !=(const " << name << " &other) const;\n";
  ofile << "\n";
  ofile << "public: class Builder;\n";
  ofile << "private: friend class Builder;\n";
  ofile << "\n";
}

/**
 * Prints the enumeration containing the field numbers.
 */
static void printFieldEnum(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public:\nstruct eFields {\n";
  ofile << "enum type {\n";
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    ofile << "FIELD_" << itr->m_name << " = " << itr->m_fieldNum << ",\n";
  }
  ofile << "COUNT = " << msgDef.m_fields.size() << "\n";
  ofile << "};\n";
  ofile << "};\n\n";
}

/**
 * Prints an enum described in the protodef
 */
static void printEnum(std::ofstream &ofile, const EnumDef &enumDef) {
  ofile << "public: enum " << enumDef.m_name << " {\n";
  ofile << enumDef.m_name << "_UNKNOWN = 0,\n";
  for (tFieldList::const_iterator itr = enumDef.m_values.begin();
       itr != enumDef.m_values.end();
       ++itr) {
    ofile << itr->m_name << " = " << itr->m_fieldNum << ",\n";
  }
  ofile << enumDef.m_name << "_COUNT = " << enumDef.m_values.size() + 1
        << ",\n";
  ofile << "};\n\n";
}

/**
 * Print the getters and storage for each field.
 */
static void printFields(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public:\n";
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    if (itr->m_repeated) {
      ofile << "const " << getFieldType(itr->m_type, itr->m_msgType) << " &get_"
            << itr->m_name << "(const size_t index) const { CHECK(index < m_"
            << itr->m_name << ".size()); return m_" << itr->m_name
            << "[index]; }\n";
      ofile << "size_t get_" << itr->m_name << "_size() const { return m_"
            << itr->m_name << ".size(); }\n";
      ofile << "typedef std::vector< "
            << getFieldType(itr->m_type, itr->m_msgType) << " > t"
            << itr->m_name << "List;\n";
      ofile << "t" << itr->m_name << "List::const_iterator get_" << itr->m_name
            << "_begin() const { return m_" << itr->m_name << ".begin(); }\n";
      ofile << "t" << itr->m_name << "List::const_iterator get_" << itr->m_name
            << "_end() const { return m_" << itr->m_name << ".end(); }\n";
    } else {
      ofile << "const " << getFieldType(itr->m_type, itr->m_msgType) << " &get_"
            << itr->m_name << "() const { return m_" << itr->m_name << "; }\n";
      if (itr->m_type == FieldDef::FIELD_MSG) {
        ofile << "const bool has_" << itr->m_name << "() const { return m_has_"
              << itr->m_name << "; }\n";
      }
    }
  }
  ofile << "protected:\n";
  for (int i = 0; i < FieldDef::FIELD_COUNT; ++i) {
    for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
         itr != msgDef.m_fields.end();
         ++itr) {
      if (itr->m_type != i) {
        continue;
      }
      if (itr->m_repeated) {
        ofile << "std::vector< " << getFieldType(itr->m_type, itr->m_msgType)
              << " > m_" << itr->m_name << ";\n";
      } else {
        ofile << "" << getFieldType(itr->m_type, itr->m_msgType) << " m_"
              << itr->m_name << ";\n";
      }
    }
  }
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    if (itr->m_type == FieldDef::FIELD_MSG) {
      ofile << "bool m_has_" << itr->m_name << ";\n";
    }
  }
}

/**
 * Print the constructor and destructors for the class.
 */
static void printCtor(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public: " << msgDef.m_name << "()\n";

  std::vector< std::string > fields;
  fields.reserve(msgDef.m_fields.size());
  for (int i = 0; i < FieldDef::FIELD_COUNT; ++i) {
    for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
         itr != msgDef.m_fields.end();
         ++itr) {
      if (itr->m_type != i) {
        continue;
      }
      if (itr->m_repeated || itr->m_type == FieldDef::FIELD_STRING
          || itr->m_type == FieldDef::FIELD_BYTES) {
        continue;
      }

      std::string field = "m_" + itr->m_name + "("
                          + defaultValue(itr->m_type, itr->m_msgType) + ")";
      fields.push_back(field);
    }
  }

  if (!fields.empty()) {
    ofile << ":";
    ofile << core::util::Joiner().on(",\n").join(fields.begin(), fields.end());
  }
  ofile << " {\n}\n";

  ofile << "virtual ~" << msgDef.m_name << "() { }\n";

  ofile << "\n";
}

/**
 * Prints the header portion of a message.
 */
static void printMessage(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "class " << msgDef.m_name
        << " : public ::core::types::iProtoMessage {\n";
  printFieldEnum(ofile, msgDef);

  for (std::vector< EnumDef >::const_iterator itr = msgDef.m_enums.begin();
       itr != msgDef.m_enums.end();
       ++itr) {
    printEnum(ofile, *itr);
  }
  for (std::vector< MessageDef >::const_iterator itr =
           msgDef.m_messages.begin();
       itr != msgDef.m_messages.end();
       ++itr) {
    printMessage(ofile, *itr);
  }

  printFields(ofile, msgDef);

  printBase(ofile, msgDef.m_name);
  printCtor(ofile, msgDef);
  ofile << "};\n\n";
}

/**
 * Print the builder constructors and desctructors for the builder.
 */
static void printBuilderCtor(
    std::ofstream &ofile,
    const MessageDef &msgDef,
    const std::string &package) {
  ofile << "Builder() {}\n";
  ofile << "Builder(const " << msgDef.m_name << " &other) {\n";
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    ofile << "m_" + itr->m_name + " = other.m_" + itr->m_name + ";\n";
  }
  ofile << "}\n";
  ofile << "\n";
}

/**
 * Print setters for the builder.
 */
void printBuilderSetters(std::ofstream &ofile, const MessageDef &msgDef) {
  ofile << "public:\n";
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    if (itr->m_repeated) {
      ofile << "Builder &add_" << itr->m_name << "(const "
            << getFieldType(itr->m_type, itr->m_msgType) << " &value) { m_"
            << itr->m_name << ".push_back(value); return *this; }\n";
      ofile << "Builder &set_" << itr->m_name << "(const "
            << getFieldType(itr->m_type, itr->m_msgType)
            << " &value, const size_t index) { CHECK(index < m_" << itr->m_name
            << ".size()); m_" << itr->m_name
            << "[index] = value; return *this; }\n";
      ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name
            << ".clear(); return *this; }\n";
    } else {
      ofile << "Builder &set_" << itr->m_name << "(const "
            << getFieldType(itr->m_type, itr->m_msgType) << " &value) { m_"
            << itr->m_name << " = value; ";
      if (itr->m_type == FieldDef::FIELD_MSG) {
        ofile << "m_has_" << itr->m_name << " = true; return *this; }\n";
        ofile << "Builder &clear_" << itr->m_name << "() { m_has_"
              << itr->m_name << " = false; return *this; }\n";
      } else if (itr->m_repeated) {
        ofile << "return *this; }\n";
        ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name
              << ".clear(); return *this; }\n";
      } else {
        ofile << "return *this; }\n";
        ofile << "Builder &clear_" << itr->m_name << "() { m_" << itr->m_name
              << " = " << defaultValue(itr->m_type, itr->m_msgType)
              << "; return *this; }\n";
      }
    }
  }
}

/**
 * Prints the header portion of a message.
 */
static void printMessageBuilder(
    std::ofstream &ofile,
    const MessageDef &msgDef,
    const std::string &package) {

  ofile << "class " << package << "::Builder : public " << package << " {\n";

  ofile << "public:\n";
  printBuilderCtor(ofile, msgDef, package);

  ofile << msgDef.m_name << " build() const { return *this; }\n\n";
  printBuilderSetters(ofile, msgDef);

  ofile << "};\n\n";

  for (std::vector< MessageDef >::const_iterator message =
           msgDef.m_messages.begin();
       message != msgDef.m_messages.end();
       ++message) {
    printMessageBuilder(ofile, *message, package + "::" + message->m_name);
  }
}

/**
 *
 */
static u32 getFieldTypeId(FieldDef::eFieldType e) {
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

/**
 *
 */
static void printOSerializer(
    std::ofstream &ofile,
    const std::vector< std::string > &package,
    const MessageDef &msgDef) {
  for (std::vector< MessageDef >::const_iterator itr =
           msgDef.m_messages.begin();
       itr != msgDef.m_messages.end();
       ++itr) {
    std::vector< std::string > subPackage = package;
    subPackage.push_back(msgDef.m_name);
    printOSerializer(ofile, subPackage, *itr);
  }
  ofile << "OSERIALIZE("
        << core::util::Joiner().on("::").join(package.begin(), package.end())
        << "::" << msgDef.m_name << ") {\n";
  for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
       itr != msgDef.m_fields.end();
       ++itr) {
    const u32 tag = (itr->m_fieldNum << 3) | getFieldTypeId(itr->m_type);
    if (itr->m_repeated) {
      ofile << "for (size_t i = 0; i < obj.get_" << itr->m_name
            << "_size(); ++i) {\n";
      ofile << "buff << VarUInt(" << tag << "ull);\n";
    } else if (itr->m_type == FieldDef::FIELD_MSG) {
      ofile << "if (obj.has_" << itr->m_name << "()) {\n";
      ofile << "buff << VarUInt(" << tag << "ull);\n";
    } else {
      ofile << "if (obj.get_" << itr->m_name
            << "() != " << defaultValue(itr->m_type, itr->m_msgType) << ") {\n";
      ofile << "buff << VarUInt(" << tag << "ull);\n";
    }
    const std::string index = itr->m_repeated ? "(i)" : "()";
    switch (itr->m_type) {
      case FieldDef::FIELD_FLOAT:
      case FieldDef::FIELD_FIXED32:
      case FieldDef::FIELD_SFIXED32:
      case FieldDef::FIELD_FIXED64:
      case FieldDef::FIELD_SFIXED64:
      case FieldDef::FIELD_DOUBLE:
        ofile << "buff << obj.get_" << itr->m_name << index << ";\n";
        break;
      case FieldDef::FIELD_INT32:
      case FieldDef::FIELD_INT64:
      case FieldDef::FIELD_SINT32:
      case FieldDef::FIELD_SINT64:
        ofile << "buff << VarInt(obj.get_" << itr->m_name << index << ");\n";
        break;
      case FieldDef::FIELD_BOOL:
        ofile << "buff << VarInt(obj.get_" << itr->m_name << index
              << " ? 1 : 0);\n";
        break;
      case FieldDef::FIELD_UINT32:
      case FieldDef::FIELD_UINT64:
        ofile << "buff << VarUInt(obj.get_" << itr->m_name << index << ");\n";
        break;
      case FieldDef::FIELD_ENUM:
        ofile << "buff << VarUInt((u64) obj.get_" << itr->m_name << index
              << ");\n";
        break;
      case FieldDef::FIELD_STRING:
      case FieldDef::FIELD_BYTES:
        ofile << "buff << VarUInt(obj.get_" << itr->m_name << index
              << ".size());\n";
        ofile << "buff.write(::core::memory::ConstBlob((const u8 *)obj.get_"
              << itr->m_name << index << ".data(), obj.get_" << itr->m_name
              << index << ".size()));\n";
        break;
      case FieldDef::FIELD_MSG:
        ofile << "::core::base::FakeSink sink;\n";
        ofile << "sink << obj.get_" << itr->m_name << index << ";\n";
        ofile << "buff << VarUInt(sink.size());\n";
        ofile << "buff << obj.get_" << itr->m_name << index << ";\n";
        break;
      default:
        CHECK(false);
    }

    ofile << "}\n";
  }
  ofile << "return buff;\n";
  ofile << "}\n";
}

/**
 *
 */
static void printISerializer(
    std::ofstream &ofile,
    const std::vector< std::string > &package,
    const MessageDef &msgDef) {
  for (std::vector< MessageDef >::const_iterator itr =
           msgDef.m_messages.begin();
       itr != msgDef.m_messages.end();
       ++itr) {
    std::vector< std::string > subPackage = package;
    subPackage.push_back(msgDef.m_name);
    printISerializer(ofile, subPackage, *itr);
  }
  ofile << "ISERIALIZE("
        << core::util::Joiner().on("::").join(package.begin(), package.end())
        << "::" << msgDef.m_name << "::Builder) {\n";
  ofile << "while(buff.avail() && !buff.fail()) {\n";
  ofile << "VarUInt tag;\n";
  ofile << "buff >> tag;\n";
  ofile << "const u32 fieldNum = ((u32) tag.get()) >> 3;\n";
  ofile << "const u32 fieldType = ((u32) tag.get()) & 0x7;\n";

  if (!msgDef.m_fields.empty()) {
    ofile << "switch (fieldNum) {\n";
    for (tFieldList::const_iterator itr = msgDef.m_fields.begin();
         itr != msgDef.m_fields.end();
         ++itr) {
      const u32 tag = (itr->m_fieldNum << 3) | getFieldTypeId(itr->m_type);
      ofile << "case " << itr->m_fieldNum << ": {\n";

      const std::string method = itr->m_repeated ? "add_" : "set_";

      switch (itr->m_type) {
        case FieldDef::FIELD_FLOAT:
          ofile << "float tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_FIXED32:
          ofile << "u32 tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_SFIXED32:
          ofile << "s32 tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_FIXED64:
          ofile << "u64 tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_SFIXED64:
          ofile << "s64 tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_DOUBLE:
          ofile << "double tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          break;
        case FieldDef::FIELD_INT32:
        case FieldDef::FIELD_SINT32:
          ofile << "VarInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "((s32) tmp.get());\n";
          break;
        case FieldDef::FIELD_INT64:
        case FieldDef::FIELD_SINT64:
          ofile << "VarInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp.get());\n";
          break;
        case FieldDef::FIELD_BOOL:
          ofile << "VarInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp.get() != 0);\n";
          break;
        case FieldDef::FIELD_UINT32:
          ofile << "VarUInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "((u32) tmp.get());\n";
          break;
        case FieldDef::FIELD_UINT64:
          ofile << "VarUInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp.get());\n";
          break;
        case FieldDef::FIELD_ENUM:
          ofile << "VarUInt tmp;\n";
          ofile << "buff >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "((" << itr->m_msgType
                << ") tmp.get());\n";
          break;

        case FieldDef::FIELD_STRING:
        case FieldDef::FIELD_BYTES:
          ofile << "VarUInt tmpSz;\n";
          ofile << "buff >> tmpSz;\n";
          ofile << "if (!buff.fail()) {\n";
          ofile << "if (tmpSz.get() > (1 << 31)) {\n";
          ofile << "buff.set_fail();\n";
          ofile << "} else {\n";
          ofile << "char *tmpStr = new char[(size_t) tmpSz.get()];\n";
          ofile << "::core::memory::Blob blob((u8 *) tmpStr, (size_t) "
                   "tmpSz.get());\n";
          ofile << "buff.read(blob);\n";
          ofile << "std::string tmp(tmpStr, tmpStr + tmpSz.get());\n";
          ofile << "obj." << method << itr->m_name << "(tmp);\n";
          ofile << "}\n";
          ofile << "}\n";
          break;
        case FieldDef::FIELD_MSG:
          ofile << "VarUInt tmpSz;\n";
          ofile << "buff >> tmpSz;\n";
          ofile << "if (!buff.fail()) {\n";
          ofile
              << "::core::base::RangeSink sink(buff, (size_t) tmpSz.get());\n";
          ofile << itr->m_msgType << "::Builder tmp;\n";
          ofile << "sink >> tmp;\n";
          ofile << "obj." << method << itr->m_name << "(tmp.build());\n";
          ofile << "}\n";
          break;
        default:
          CHECK(false);
      }

      ofile << "continue;\n";
      ofile << "}\n";
    }
    ofile << "}\n";
  }

  ofile << "switch (fieldType) {\n";
  ofile << "case 0: {\n";
  ofile << "VarInt temp;\n";
  ofile << "buff >> temp;\n";
  ofile << "break;\n";
  ofile << "}\n";
  ofile << "case 1: {\n";
  ofile << "buff.seek(sizeof(u64));\n";
  ofile << "break;\n";
  ofile << "}\n";
  ofile << "case 2: {\n";
  ofile << "VarInt temp;\n";
  ofile << "buff >> temp;\n";
  ofile << "buff.seek((size_t) temp.get());\n";
  ofile << "break;\n";
  ofile << "}\n";
  ofile << "case 5: {\n";
  ofile << "buff.seek(sizeof(u32));\n";
  ofile << "break;\n";
  ofile << "}\n";
  ofile << "}\n";

  ofile << "}\n";

  ofile << "return buff;\n";
  ofile << "}\n";
}

/**
 * Prints the header file portion of the proto
 */
bool PrintHeader(const ProtoDef &def, const std::string &fileName) {
  std::ofstream ofile(fileName);
  if (!ofile.is_open()) {
    return false;
  }

  const std::string safeName = core::util::IdentifierSafe(fileName);
  ofile << "#ifndef FISHY_PROTOC_" << safeName << "_H\n";
  ofile << "#define FISHY_PROTOC_" << safeName << "_H\n";

  printImports(ofile, def.m_imports, !def.m_services.empty());
  const std::vector< std::string > package =
      core::util::Splitter().on('.').split(def.m_package);
  printOpenNamespace(ofile, package);

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printMessage(ofile, *message);
  }

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printMessageBuilder(ofile, *message, message->m_name);
  }

  /*
  for (std::vector< ServiceDef >::const_iterator service =
           def.m_services.begin();
       service != def.m_services.end();
       ++service) {
    printHeaderService(ofile, *service);
  }
  */
  printCloseNamespace(ofile, package);

  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    printOSerializer(ofile, package, *message);
    printISerializer(ofile, package, *message);
  }

  ofile << "#endif\n";

  ofile.flush();
  return !ofile.fail();
}
