#include "protobuf.h"

#include <CORE/BASE/serializer_podtypes.h>
#include <CORE/UTIL/FILES/proto_text.h>
#include <CORE/UTIL/lexical_cast.h>

namespace core {
namespace types {

typedef std::vector< const ProtoDescriptor * > tDescriptorList;

/**
 *
 */
tDescriptorList &GetGlobalDb() {
  static std::vector< const ProtoDescriptor * > g_globalDb;
  return g_globalDb;
}

/**
 *
 */
bool DebugNoDoubleRegister(const ProtoDescriptor *pDescriptor) {
  for (tDescriptorList::const_iterator itr = GetGlobalDb().begin();
       itr != GetGlobalDb().end();
       ++itr) {
    if (pDescriptor == *itr) {
      return false;
    }
    if ((pDescriptor->getDef().m_name == (*itr)->getDef().m_name
         && pDescriptor->getDef().m_package == (*itr)->getDef().m_package)) {
      CHECK_UNREACHABLE();
    }
  }

  return true;
}

/**
 *
 */
void RegisterWithProtoDb(const ProtoDescriptor *pDescriptor) {
  ASSERT(DebugNoDoubleRegister(pDescriptor));
  GetGlobalDb().push_back(pDescriptor);
}

/**
 *
 */
std::vector< std::string > ListAllProtoNames() {
  std::vector< std::string > rVal;
  for (tDescriptorList::const_iterator itr = GetGlobalDb().begin();
       itr != GetGlobalDb().end();
       ++itr) {
    const ProtoDescriptor *pDescriptor = *itr;
    rVal.push_back(
        pDescriptor->getDef().m_package + pDescriptor->getDef().m_name);
  }
  return rVal;
}

/**
 *
 */
const ProtoDescriptor *FindProtoByName(const std::string &name) {
  for (tDescriptorList::const_iterator itr = GetGlobalDb().begin();
       itr != GetGlobalDb().end();
       ++itr) {
    const ProtoDescriptor *pDescriptor = *itr;
    if ((pDescriptor->getDef().m_package + "::" + pDescriptor->getDef().m_name)
        == name) {
      return pDescriptor;
    }
  }
  return nullptr;
}

/**
 *
 */
const EnumDef *FindProtoEnumByName(const std::string &name) {
  std::string::size_type findpos = name.find_last_of(":");
  if (findpos == name.npos) {
    return nullptr;
  }
  const std::string containerName = name.substr(0, findpos - 1);
  const std::string enumName = name.substr(findpos + 1);
  const ProtoDescriptor *pDescriptor = FindProtoByName(containerName);
  if (pDescriptor == nullptr) {
    return nullptr;
  }
  return pDescriptor->findEnumByName(enumName);
}

/**
 *
 */
std::vector< const ProtoDescriptor * > g_emptyChildList;
ProtoDescriptor::ProtoDescriptor(const MessageDef &self)
    : m_self(self), m_children(g_emptyChildList) {
}
ProtoDescriptor::ProtoDescriptor(
    const MessageDef &self,
    const std::vector< const ProtoDescriptor * > &childen)
    : m_self(self), m_children(childen) {
}

/**
 *
 */
const FieldDef *
ProtoDescriptor::findFieldByName(const std::string &name) const {
  for (std::vector< FieldDef >::const_iterator field = m_self.m_fields.begin();
       field != m_self.m_fields.end();
       ++field) {
    if (field->m_name == name) {
      return &(*field);
    }
  }
  return nullptr;
}

/**
 *
 */
const FieldDef *ProtoDescriptor::findFieldByNum(const u32 num) const {
  for (std::vector< FieldDef >::const_iterator field = m_self.m_fields.begin();
       field != m_self.m_fields.end();
       ++field) {
    if (field->m_fieldNum == num) {
      return &(*field);
    }
  }
  return nullptr;
}

/**
 *
 */
const ProtoDescriptor *
ProtoDescriptor::findMessageByName(const std::string &name) const {
  for (std::vector< const ProtoDescriptor * >::const_iterator itr =
           m_children.begin();
       itr != m_children.end();
       ++itr) {
    if (((*itr)->m_self.m_package + (*itr)->m_self.m_name) == name) {
      return *itr;
    }
  }
  return nullptr;
}

/**
 *
 */
const EnumDef *ProtoDescriptor::findEnumByName(const std::string &name) const {
  for (std::vector< EnumDef >::const_iterator field = m_self.m_enums.begin();
       field != m_self.m_enums.end();
       ++field) {
    if (field->m_name == name) {
      return &(*field);
    }
  }
  return nullptr;
}

/**
 *
 */
const MessageDef &ProtoDescriptor::getDef() const {
  return m_self;
}

/**
 *
 *
DynamicProto::DynamicProto(const core::util::ProtoDescriptor &descriptor)
    : m_descriptor(descriptor) {
}

/**
 *
 *
const core::util::ProtoDescriptor &DynamicProto::getDescriptor() const {
  return m_descriptor;
}

/**
 *
 *
bool DynamicProto::getField(const u32 fieldNum, std::string &value) const {
  tFieldMap::const_iterator itr = m_fields.find(fieldNum);
  if (itr == m_fields.end()) {
    return false;
  } else {
    value = itr->second.front();
    return true;
  }
}

/**
 *
 *
bool DynamicProto::getField(
    const u32 fieldNum, const u32 index, std::string &value) const {
  tFieldMap::const_iterator itr = m_fields.find(fieldNum);
  if (itr == m_fields.end()) {
    return false;
  } else if (index < itr->second.size()) {
    value = itr->second.at(index);
    return true;
  }
  return false;
}

/**
 *
 *
size_t DynamicProto::byte_size() const {
  return m_buffer.size();
}

/**
 *
 *
bool DynamicProto::oserialize(core::base::iBinarySerializerSink &sink) const {
  return m_buffer.size()
         == sink.write(core::memory::ConstBlob(&m_buffer[0], m_buffer.size()));
}

typedef std::map< int, std::vector< std::string > > tFieldMap;
/**
 *
 *
template < typename tType >
static bool CastFromSink(
    core::base::iBinarySerializerSink &sink,
    const u32 fieldNum,
    tFieldMap &out) {
  tType temp;
  sink >> temp;
  std::string tempStr;
  if (!core::util::lexical_cast(temp, tempStr)) {
    return false;
  }
  out[fieldNum].push_back(tempStr);
  return true;
}

/**
 *
 *
template <>
static bool CastFromSink< VarInt >(
    core::base::iBinarySerializerSink &sink,
    const u32 fieldNum,
    tFieldMap &out) {
  VarInt temp;
  sink >> temp;
  std::string tempStr;
  if (!core::util::lexical_cast(temp.get(), tempStr)) {
    return false;
  }
  out[fieldNum].push_back(tempStr);
  return true;
}

/**
 *
 *
template <>
static bool CastFromSink< VarUInt >(
    core::base::iBinarySerializerSink &sink,
    const u32 fieldNum,
    tFieldMap &out) {
  VarUInt temp;
  sink >> temp;
  std::string tempStr;
  if (!core::util::lexical_cast(temp.get(), tempStr)) {
    return false;
  }
  out[fieldNum].push_back(tempStr);
  return true;
}

/**
 *
 *
bool DynamicProto::iserialize(core::base::iBinarySerializerSink &fileSink) {
  m_buffer.resize(fileSink.avail());
  fileSink.read(core::memory::Blob(&m_buffer[0], m_buffer.size()));
  core::base::ConstBlobSink sink(
      core::memory::ConstBlob(&m_buffer[0], m_buffer.size()));

  while (sink.avail() && !sink.fail()) {
    VarUInt tag;
    sink >> tag;
    const u32 fieldNum = ((u32) tag.get()) >> 3;
    switch (m_descriptor.findFieldByNum(fieldNum)->m_type) {
      case FieldDef::FIELD_FLOAT:
        RET_M(
            CastFromSink< float >(sink, fieldNum, m_fields),
            "cast error to float");
        break;
      case FieldDef::FIELD_FIXED32:
        RET_M(
            CastFromSink< u32 >(sink, fieldNum, m_fields), "cast error to u32");
        break;
      case FieldDef::FIELD_SFIXED32:
        RET_M(
            CastFromSink< s32 >(sink, fieldNum, m_fields), "cast error to s32");
        break;
      case FieldDef::FIELD_FIXED64:
        RET_M(
            CastFromSink< u64 >(sink, fieldNum, m_fields), "cast error to u64");
        break;
      case FieldDef::FIELD_SFIXED64:
        RET_M(
            CastFromSink< s64 >(sink, fieldNum, m_fields), "cast error to s64");
        break;
      case FieldDef::FIELD_DOUBLE:
        RET_M(
            CastFromSink< double >(sink, fieldNum, m_fields),
            "cast error to double");
        break;
      case FieldDef::FIELD_INT32:
        RET_M(
            CastFromSink< VarInt >(sink, fieldNum, m_fields),
            "cast error to VarInt");
        break;
      case FieldDef::FIELD_INT64:
        RET_M(
            CastFromSink< VarInt >(sink, fieldNum, m_fields),
            "cast error to VarInt");
        break;
      case FieldDef::FIELD_UINT32:
        RET_M(
            CastFromSink< VarUInt >(sink, fieldNum, m_fields),
            "cast error to VarUInt");
        break;
      case FieldDef::FIELD_UINT64:
        RET_M(
            CastFromSink< VarUInt >(sink, fieldNum, m_fields),
            "cast error to VarUInt");
        break;
      case FieldDef::FIELD_SINT32:
        RET_M(
            CastFromSink< VarInt >(sink, fieldNum, m_fields),
            "cast error to VarInt");
        break;
      case FieldDef::FIELD_SINT64:
        RET_M(
            CastFromSink< VarInt >(sink, fieldNum, m_fields),
            "cast error to VarInt");
        break;
      case FieldDef::FIELD_ENUM:
        RET_M(
            CastFromSink< VarUInt >(sink, fieldNum, m_fields),
            "cast error to VarUInt");
        break;
      case FieldDef::FIELD_BOOL:
        RET_M(
            CastFromSink< VarUInt >(sink, fieldNum, m_fields),
            "cast error to VarUInt");
        break;

      case FieldDef::FIELD_STRING:
      case FieldDef::FIELD_BYTES: {
        VarUInt len;
        sink >> len;
        std::string msg;
        msg.resize((u32) len.get(), 0);
        sink.read(core::memory::Blob((u8 *) &msg[0], (u32) len.get()));
        m_fields[fieldNum].push_back(msg);
        break;
      }
      case FieldDef::FIELD_MSG: {
        VarUInt len;
        sink >> len;
        std::vector< u8 > buffer;
        buffer.resize((u32) len.get());
        sink.read(core::memory::Blob(&buffer[0], (u32) len.get()));
        core::base::ConstBlobSink subSink(
            core::memory::ConstBlob(&buffer[0], buffer.size()));
        const ProtoDescriptor *pDescriptor =
            FindProtoByName(m_descriptor.findFieldByNum(fieldNum)->m_msgType);
        RET_M(
            pDescriptor != nullptr,
            "unable to find submessage: "
                << m_descriptor.findFieldByNum(fieldNum)->m_msgType);
        DynamicProto subMessage(*pDescriptor);
        RET_M(
            subMessage.iserialize(subSink), "parse error on internal message");
        std::string msg;
        RET_M(
            core::util::files::TextFormat::format(msg, subMessage),
            "unable to format internal message");
        m_fields[fieldNum].push_back(msg);
        break;
      }
    }
  }
  return true;
}
*/

} // namespace types
} // namespace core
