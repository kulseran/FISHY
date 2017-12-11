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
    FIELD_FLOAT,
    FIELD_INT32,
    FIELD_INT64,
    FIELD_UINT32,
    FIELD_UINT64,
    FIELD_SINT32,
    FIELD_SINT64,
    FIELD_FIXED32,
    FIELD_FIXED64,
    FIELD_SFIXED32,
    FIELD_SFIXED64,
    FIELD_BOOL,
    FIELD_STRING,
    FIELD_BYTES,
    FIELD_ENUM,
    FIELD_MSG
  };

  std::string m_name;
  eFieldType m_type;
  std::string m_msgType;
  int m_fieldNum;
  bool m_repeated;
};

/**
 * Defines an enumeration
 */
struct EnumDef {
  std::string m_name;
  std::vector< FieldDef > m_values;
};

/**
 * Defines a message
 */
struct MessageDef {
  std::string m_name;
  std::string m_package;
  std::vector< MessageDef > m_messages;
  std::vector< FieldDef > m_fields;
  std::vector< EnumDef > m_enums;
};

/**
 * Defines a RPC callable function
 */
struct RpcFunctionDef {
  std::string m_name;
  std::string m_return;
  std::string m_param;
};

/**
 * Defines a RPC service
 */
struct ServiceDef {
  std::string m_name;
  std::string m_package;
  std::vector< RpcFunctionDef > m_functions;
};

/**
 * Main definiton output of a file, may contain multiple top level messages
 */
struct ProtoDef {
  std::string m_package;
  std::vector< std::string > m_imports;
  std::vector< MessageDef > m_messages;
  std::vector< ServiceDef > m_services;
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

  virtual bool getField(const u32 fieldNum, std::string &value) const = 0;
  virtual bool
  getField(const u32 fieldNum, const u32 index, std::string &value) const = 0;

  virtual const ProtoDescriptor &getDescriptor() const = 0;
  virtual size_t byte_size() const = 0;
  virtual bool oserialize(core::base::iBinarySerializerSink &) const = 0;
  virtual bool iserialize(core::base::iBinarySerializerSink &) = 0;

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
  bool getField(const u32 fieldNum, std::string &value) const override;
  bool getField(
      const u32 fieldNum, const u32 index, std::string &value) const override;
  size_t byte_size() const override;
  bool oserialize(core::base::iBinarySerializerSink &sink) const override;
  bool iserialize(core::base::iBinarySerializerSink &fileSink) override;

  private:
  typedef std::map< int, std::vector< std::string > > tFieldMap;
  const ProtoDescriptor &m_descriptor;
  tFieldMap m_fields;
  std::vector< u8 > m_buffer;
};

} // namespace types
} // namespace core

#endif
