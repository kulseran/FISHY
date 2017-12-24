/**
 * protobuf.h
 *
 * Definitions for Fishy style protobuffers.
 * Based loosely off Google protobuffers.
 */
#ifndef FISHY_PROTOBUF_H
#define FISHY_PROTOBUF_H

#include <CORE/types.h>

#include <map>
#include <string>
#include <vector>

namespace core {
namespace base {
class iBinarySerializerSink;
}
} // namespace core

namespace core {
namespace types {

/**
 * Defines a field within a message.
 */
struct FieldDef {
  enum eFieldType {
    FIELD_DOUBLE,
    FIELD_INT64,
    FIELD_UINT64,
    FIELD_SINT64,
    FIELD_FIXED64,
    FIELD_SFIXED64,
    FIELD_FLOAT,
    FIELD_INT32,
    FIELD_UINT32,
    FIELD_SINT32,
    FIELD_FIXED32,
    FIELD_SFIXED32,
    FIELD_STRING,
    FIELD_BYTES,
    FIELD_ENUM,
    FIELD_MSG,
    FIELD_BOOL,
    FIELD_COUNT
  };

  std::string m_name;
  std::string m_msgType;
  eFieldType m_type;
  int m_fieldNum;
  bool m_repeated;
};
typedef std::vector< FieldDef > tFieldList;
/**
 * Defines an enumeration
 */
struct EnumDef {
  std::string m_name;
  tFieldList m_values;
};
typedef std::vector< EnumDef > tEnumList;

/**
 * Defines a message
 */
struct MessageDef {
  std::string m_name;
  std::string m_package;
  std::vector< MessageDef > m_messages;
  tFieldList m_fields;
  tEnumList m_enums;
};
typedef std::vector< MessageDef > tMessageList;

/**
 * Defines a RPC callable function
 */
struct RpcFunctionDef {
  std::string m_name;
  std::string m_return;
  std::string m_param;
};
typedef std::vector< RpcFunctionDef > tRpcFunctionList;

/**
 * Defines a RPC service
 */
struct ServiceDef {
  std::string m_name;
  std::string m_package;
  tRpcFunctionList m_functions;
};
typedef std::vector< ServiceDef > tServiceList;

/**
 * Main definiton output of a file, may contain multiple top level messages
 */
struct ProtoDef {
  std::string m_package;
  std::vector< std::string > m_imports;
  tMessageList m_messages;
  tServiceList m_services;
};

/**
 * Protobuffer descriptor
 */
class ProtoDescriptor {
  public:
  ProtoDescriptor(const MessageDef &self);
  ProtoDescriptor(
      const MessageDef &self,
      const std::vector< const ProtoDescriptor * > &childen);

  /**
   * Find a child field by name
   *
   * @param name the field name to find
   * @return the located field or nullptr
   */
  const FieldDef *findFieldByName(const std::string &name) const;

  /**
   * Find a child field by number
   *
   * @param num the field number to find
   * @return the located field or nullptr
   */
  const FieldDef *findFieldByNum(const u32 num) const;

  /**
   * Find a child message by name
   *
   * @param name message name to find
   * @return the child {@link ProtoDescriptor} or nullptr
   */
  const ProtoDescriptor *findMessageByName(const std::string &name) const;

  /**
   * Find a child Enum by name
   *
   * @param name message name to find
   * @return the child {@link ProtoDescriptor} or nullptr
   */
  const EnumDef *findEnumByName(const std::string &name) const;

  /**
   * @return the {@link MessageDef} for this proto.
   */
  const MessageDef &getDef() const;

  private:
  MessageDef m_self;
  std::vector< const ProtoDescriptor * > m_children;
};

/**
 * Register a protodescriptor in the global db.
 * Normally done as an internal call, an no need to do this manually.
 */
void RegisterWithProtoDb(const ProtoDescriptor *const pDescriptor);

/**
 * Lookup any globally registered protobuffer
 *
 * @param name the fully qualified proto name
 */
const ProtoDescriptor *const FindProtoByName(const std::string &name);

/**
 * Lookup any globally registered protobuffer enumeration
 *
 * @param name the fully qualified proto enum name
 */
const EnumDef *const FindProtoEnumByName(const std::string &name);

/**
 * Lists all globally registered protobuffers.
 */
std::vector< std::string > ListAllProtoNames();

/**
 * Protobuffer interface defining basic refelection and serialization functions.
 */
class iProtoMessage {
  public:
  virtual ~iProtoMessage() {}

  virtual const ProtoDescriptor &getDescriptor() const = 0;

  size_t byte_size() const;
  bool oserialize(core::base::iBinarySerializerSink &) const;
  bool iserialize(core::base::iBinarySerializerSink &);

  protected:
  iProtoMessage() {}
};

/**
 * A RTTI implementation of iProtoMessage
 */
class DynamicProto : public iProtoMessage {
  public:
  DynamicProto(const ProtoDescriptor &descriptor);
  const ProtoDescriptor &getDescriptor() const override;

  private:
  typedef std::map< int, std::vector< std::string > > tFieldMap;
  const ProtoDescriptor &m_descriptor;
  tFieldMap m_fields;
  std::vector< u8 > m_buffer;
};

} // namespace types
} // namespace core

#endif
