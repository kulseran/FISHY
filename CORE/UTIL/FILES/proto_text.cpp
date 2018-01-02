/**
 * proto_text.cpp
 */
#include "proto_text.h"

#include <CORE/BASE/logging.h>
#include <CORE/BASE/serializer_podtypes.h>
#include <CORE/UTIL/lexical_cast.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/UTIL/tokenizer.h>

#include <memory>

using core::base::ConstBlobSink;
using core::base::iBinarySerializerSink;
using core::memory::Blob;
using core::memory::ConstBlob;
using core::types::EnumDef;
using core::types::FieldDef;
using core::types::FindProtoByName;
using core::types::FindProtoEnumByName;
using core::types::iProtoMessage;
using core::types::ProtoDescriptor;
using core::util::CountLines;
using core::util::Escape;
using core::util::lexical_cast;
using core::util::TrimQuotes;
using core::util::Unescape;
using core::util::parser::RegExPattern;
using core::util::parser::Tokenizer;

namespace core {
namespace util {
namespace files {

/**
 * Recognized token types.
 */
struct eTokens {
  enum type {
    IDENTIFIER,
    WS,
    EOL,
    EQUAL,
    NUMERIC,
    STRING,
    COLON,
    OBRACE,
    CBRACE,
    COMMENT,
    MISC,

    COUNT
  };
};
typedef Tokenizer< eTokens > tTokenizer;

/**
 * Factory for a singleton {@link Tokenizer} with all the appropriate tokens.
 */
class TokenizerFactory {
  public:
  TokenizerFactory() {
    tTokenizer::tTokenizerList tokenRegexs;
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::STRING, RegExPattern("\"[^\"]*\"")));
    tokenRegexs.push_back(tTokenizer::tTokenizer(
        eTokens::NUMERIC, RegExPattern("[-\\+]?\\d+\\.?\\d*")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::IDENTIFIER, RegExPattern("\\a\\w*")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::OBRACE, RegExPattern("{")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::CBRACE, RegExPattern("}")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::EQUAL, RegExPattern("=")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::COLON, RegExPattern(":")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::EOL, RegExPattern("\n")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::WS, RegExPattern("\\s")));
    tokenRegexs.push_back(
        tTokenizer::tTokenizer(eTokens::COMMENT, RegExPattern("//.*\n")));
    tokenRegexs.push_back(tTokenizer::tTokenizer(
        eTokens::MISC, core::util::parser::RegExPattern(".")));
    m_tokenizer = std::shared_ptr< tTokenizer >(new tTokenizer(tokenRegexs));
  }

  const tTokenizer &get() const { return *(m_tokenizer.get()); }

  private:
  std::shared_ptr< tTokenizer > m_tokenizer;
};
static const TokenizerFactory g_tokenizerFactory;

static u32 getFieldTypeId(FieldDef::eFieldType e);
static bool parseMessageToByteString(
    iBinarySerializerSink &buffer,
    const ProtoDescriptor &descriptor,
    std::string::const_iterator &begin,
    const std::string::const_iterator &end,
    const bool errorOnUnknown);

class FieldSkipper {
  public:
  FieldSkipper(
      std::string::const_iterator &begin,
      const std::string::const_iterator &end)
      : m_begin(begin), m_end(end), m_tokenizer(g_tokenizerFactory.get()) {

    m_itr = begin;
    // Skip WS tokens.
    while (m_tokenizer.getNextToken(m_token, m_itr, m_end)) {
      m_itr = m_token.end();
      if (m_token.getId() != eTokens::WS) {
        break;
      }
    }
  }

  bool skipFieldSingle() {
    RET_M(
        m_token.getId() == eTokens::COLON,
        "Expected ':' when looking for singular field.");
    // Fields may start with a ':', and thus take the form:
    //     field: value
    while (m_tokenizer.getNextToken(m_token, m_itr, m_end)) {
      m_itr = m_token.end();
      if (m_token.getId() == eTokens::NUMERIC
          || m_token.getId() == eTokens::STRING) {
        m_begin = m_itr;
        return true;
      } else if (m_token.getId() == eTokens::EOL) {
        Log(LL::Error) << "Expected number or string before newline.";
        return false;
      } else if (m_token.getId() == eTokens::WS) {
        continue;
      } else {
        Log(LL::Error)
            << "Unexpected token while looking for numeric or string: "
            << m_token.getToken();
        return false;
      }
    }
    return false;
  }

  bool skipFieldGroup() {
    RET_M(
        m_token.getId() == eTokens::OBRACE,
        "Expected '{' when looking for nested message.");
    // Fields may start with a '{', and thus take the form:
    //     field { ... }
    int depth = 0;
    while (m_tokenizer.getNextToken(m_token, m_itr, m_end)) {
      m_itr = m_token.end();
      if (m_token.getId() == eTokens::CBRACE) {
        if (depth == 0) {
          m_begin = m_itr;
          return true;
        } else if (depth > 0) {
          --depth;
        } else {
          CHECK_UNREACHABLE();
          return false;
        }
      } else if (m_token.getId() == eTokens::OBRACE) {
        depth++;
      }
    }
    return false;
  }

  bool skipField() {
    if (m_token.getId() == eTokens::OBRACE) {
      return skipFieldGroup();
    } else if (m_token.getId() == eTokens::COLON) {
      return skipFieldSingle();
    } else {
      Log(LL::Error) << "Attempted to skip field, but encountered unexpected: "
                     << m_token.getToken();
      return false;
    }
  }

  private:
  std::string::const_iterator &m_begin;
  const std::string::const_iterator &m_end;
  const tTokenizer &m_tokenizer;
  tTokenizer::Token m_token;
  std::string::const_iterator m_itr;
};

/**
 *
 */
bool ConvertField(std::string &ret, const void *pField, const FieldDef &field) {
  switch (field.m_type) {
    case FieldDef::FIELD_UINT64:
    case FieldDef::FIELD_FIXED64:
    case FieldDef::FIELD_INT64: {
      const u64 *pValue = reinterpret_cast< const u64 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_SINT64:
    case FieldDef::FIELD_SFIXED64: {
      const s64 *pValue = reinterpret_cast< const s64 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_DOUBLE: {
      const f64 *pValue = reinterpret_cast< const f64 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_FLOAT: {
      const f32 *pValue = reinterpret_cast< const f32 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_INT32:
    case FieldDef::FIELD_UINT32:
    case FieldDef::FIELD_FIXED32:
    case FieldDef::FIELD_SINT32: {
      const u32 *pValue = reinterpret_cast< const u32 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_SFIXED32: {
      const s32 *pValue = reinterpret_cast< const s32 * >(pField);
      return lexical_cast(*pValue, ret);
    }
    case FieldDef::FIELD_STRING: {
      ret = *reinterpret_cast< const std::string * >(pField);
      return true;
    }
    case FieldDef::FIELD_BYTES: {
      ret = *reinterpret_cast< const std::string * >(pField);
      return true;
    }
    case FieldDef::FIELD_ENUM: {
      const EnumDef *pEnumDef = FindProtoEnumByName(field.m_msgType);
      if (pEnumDef == nullptr) {
        return false;
      }
      const s32 *pValue = reinterpret_cast< const s32 * >(pField);
      for (std::vector< FieldDef >::const_iterator itr =
               pEnumDef->m_values.begin();
           itr != pEnumDef->m_values.end();
           ++itr) {
        if (itr->m_fieldNum == *pValue) {
          ret = itr->m_name;
          return true;
        }
      }
      return false;
    }
    case FieldDef::FIELD_MSG: {
      const iProtoMessage *pMsg =
          reinterpret_cast< const iProtoMessage * >(pField);
      TextFormat::format(ret, *pMsg);
      return true;
    }
    case FieldDef::FIELD_BOOL: {
      const bool *pValue = reinterpret_cast< const bool * >(pField);
      return lexical_cast(*pValue, ret);
    }
  }
  return false;
}

/**
 *
 */
bool GetFieldAsString(
    std::string &ret,
    const iProtoMessage &msg,
    const FieldDef &field,
    const size_t index) {
  const void *pField = msg.getField(field.m_fieldNum, index);
  if (pField == nullptr) {
    return false;
  }
  return ConvertField(ret, pField, field);
}

/**
 *
 */
bool GetFieldAsString(
    std::string &ret, const iProtoMessage &msg, const FieldDef &field) {
  const void *pField = msg.getField(field.m_fieldNum);
  return ConvertField(ret, pField, field);
}

/**
 *
 */
void TextFormat::format(std::string &retVal, const iProtoMessage &msg) {
  const ProtoDescriptor &descriptor = msg.getDescriptor();
  for (std::vector< FieldDef >::const_iterator field =
           descriptor.getDef().m_fields.begin();
       field != descriptor.getDef().m_fields.end();
       ++field) {
    if (field->m_repeated) {
      std::string val;
      size_t index = 0;
      while (GetFieldAsString(val, msg, *field, index)) {
        if (field->m_type == FieldDef::FIELD_MSG) {
          retVal += field->m_name + " {\n" + val + "}\n";
        } else if (
            field->m_type == FieldDef::FIELD_STRING
            || field->m_type == FieldDef::FIELD_BYTES) {
          retVal += field->m_name + ": \"" + Escape(val) + "\"\n";
        } else {
          retVal += field->m_name + ": " + val + "\n";
        }
        index++;
      }
    } else {
      std::string val;
      if (!GetFieldAsString(val, msg, *field)) {
        continue;
      }
      if (field->m_type == FieldDef::FIELD_MSG) {
        retVal += field->m_name + " {\n" + val + "}\n";
      } else if (
          field->m_type == FieldDef::FIELD_STRING
          || field->m_type == FieldDef::FIELD_BYTES) {
        retVal += field->m_name + ": \"" + Escape(val) + "\"\n";
      } else {
        retVal += field->m_name + ": " + val + "\n";
      }
    }
  }
}

/**
 *
 */
Status TextFormat::parse(
    iProtoMessage &msg, const std::string &textProto, bool errorOnUnknown) {
  const ProtoDescriptor &descriptor = msg.getDescriptor();
  std::string::const_iterator begin = textProto.begin();

  std::string byteBuffer;
  byteBuffer.resize(textProto.size(), '\0');
  core::base::BlobSink tempSink(
      Blob(reinterpret_cast< u8 * >(&byteBuffer[0]), byteBuffer.size()));
  if (!parseMessageToByteString(
          tempSink, descriptor, begin, textProto.end(), errorOnUnknown)) {
    Log(LL::Error) << "Parse failed near line: "
                   << CountLines(textProto.begin(), begin);
    return Status::BAD_INPUT;
  }
  ConstBlobSink sink(ConstBlob(
      reinterpret_cast< const u8 * >(byteBuffer.data()), tempSink.size()));
  return Status(msg.iserialize(sink));
}

/**
 * @return the Protobuffer type identifier for a given field type.
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

/**
 * Parses a text message into a binary message.
 * @param buffer the sink to write the binary message to
 * @param descriptor the proto descriptor to use when parsing fields
 * @param begin the buffer being, will be updated on success return
 * @param end the buffer end
 * @return true on success
 */
bool parseMessageToByteString(
    iBinarySerializerSink &buffer,
    const ProtoDescriptor &descriptor,
    std::string::const_iterator &begin,
    const std::string::const_iterator &end,
    const bool errorOnUnknown) {
  const tTokenizer &tokenizer = g_tokenizerFactory.get();
  tTokenizer::Token token;

  // used to track if this parse is over a {} pair.
  bool openedMessage = false;

  // scan all tokens
  while (tokenizer.getNextToken(token, begin, end)) {
    begin = token.end();

    // Skip whitespace
    if (token.getId() == eTokens::WS || token.getId() == eTokens::COMMENT
        || token.getId() == eTokens::EOL) {
      continue;
    }

    if (token.getId() == eTokens::OBRACE && !openedMessage) {
      openedMessage = true;
      continue;
    }
    if (token.getId() == eTokens::CBRACE && openedMessage) {
      openedMessage = false;
      break;
    }

    RET_M(
        token.getId() == eTokens::IDENTIFIER,
        "Expected identifier, found: '" << token.getToken() << "'");

    // Lookup field and parse.
    const FieldDef *fieldDef = descriptor.findFieldByName(token.getToken());
    if (fieldDef == nullptr) {
      if (errorOnUnknown) {
        Log(LL::Error) << "Unknown field encountered: '" << token.getToken()
                       << "'";
        return false;
      }
      Log(LL::Warning) << "Unknown field encountered: '" << token.getToken()
                       << "'";
      if (FieldSkipper(begin, end).skipField()) {
        continue;
      } else {
        break;
      }
    }

    // Write field tag
    const u32 tag =
        (fieldDef->m_fieldNum << 3) | getFieldTypeId(fieldDef->m_type);
    buffer << VarUInt(tag);

    if (fieldDef->m_type == FieldDef::FIELD_MSG) {
      // Handle a message field
      const ProtoDescriptor *msgDef = FindProtoByName(fieldDef->m_msgType);
      RET_M(
          msgDef != nullptr,
          "Unknown message encountered: '" << token.getToken() << "'");

      // Determine where the message field ends to limit the parsing range
      std::string::const_iterator oldBegin = begin;
      if (!FieldSkipper(begin, end).skipFieldGroup()) {
        break;
      }

      // Parse once to determine the size
      std::string::const_iterator tempBegin = oldBegin;
      core::base::FakeSink tempBuffer;
      if (!parseMessageToByteString(
              tempBuffer, *msgDef, tempBegin, begin, errorOnUnknown)) {
        return false;
      }

      // Write the size tag
      buffer << VarUInt(tempBuffer.size());

      // Parse final output into buffer
      tempBegin = oldBegin;
      parseMessageToByteString(
          buffer, *msgDef, oldBegin, begin, errorOnUnknown);
    } else {

      // Skip WS
      while (tokenizer.getNextToken(token, begin, end)) {
        begin = token.end();
        if (token.getId() != eTokens::WS) {
          break;
        }
      }

      // Non-message fields must start with a ':'
      RET_M(
          token.getId() == eTokens::COLON,
          "Expected ':', found '" << token.getToken() << "'");

      // Skip WS
      while (tokenizer.getNextToken(token, begin, end)) {
        begin = token.end();
        if (token.getId() != eTokens::WS) {
          break;
        }
      }

      // Confirm that the following token is the correct type for the given
      // field.
      switch (fieldDef->m_type) {
        case FieldDef::eFieldType::FIELD_DOUBLE:
        case FieldDef::eFieldType::FIELD_FLOAT:
        case FieldDef::eFieldType::FIELD_INT32:
        case FieldDef::eFieldType::FIELD_INT64:
        case FieldDef::eFieldType::FIELD_UINT32:
        case FieldDef::eFieldType::FIELD_UINT64:
        case FieldDef::eFieldType::FIELD_SINT32:
        case FieldDef::eFieldType::FIELD_SINT64:
        case FieldDef::eFieldType::FIELD_FIXED32:
        case FieldDef::eFieldType::FIELD_FIXED64:
        case FieldDef::eFieldType::FIELD_SFIXED32:
        case FieldDef::eFieldType::FIELD_SFIXED64:
          RET_M(
              token.getId() == eTokens::NUMERIC,
              "Expected numeric value for field: " << fieldDef->m_name);
          break;
        case FieldDef::eFieldType::FIELD_STRING:
        case FieldDef::eFieldType::FIELD_BYTES:
          RET_M(
              token.getId() == eTokens::STRING,
              "Expected string value for field: " << fieldDef->m_name);
          break;
        case FieldDef::eFieldType::FIELD_ENUM:
          RET_M(
              (token.getId() == eTokens::NUMERIC
               || token.getId() == eTokens::IDENTIFIER),
              "Expected numeric or identifer value for field: "
                  << fieldDef->m_name);
          break;
        case FieldDef::eFieldType::FIELD_BOOL:
          RET_M(
              token.getId() == eTokens::IDENTIFIER,
              "Expected identifer value for field: " << fieldDef->m_name);
          break;
        case FieldDef::eFieldType::FIELD_MSG:
          CHECK_UNREACHABLE();
          break;
      }

      // Parse the field text into a field value
      switch (fieldDef->m_type) {
        case FieldDef::eFieldType::FIELD_DOUBLE: {
          double val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as double numeric value.");
          buffer << val;
        } break;
        case FieldDef::eFieldType::FIELD_FLOAT: {
          float val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as float numeric value.");
          buffer << val;
        } break;
        case FieldDef::eFieldType::FIELD_FIXED32: {
          u32 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << val;
        } break;
        case FieldDef::eFieldType::FIELD_FIXED64: {
          u64 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << val;
        } break;
        case FieldDef::eFieldType::FIELD_SFIXED32: {
          s32 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << val;
        } break;

        case FieldDef::eFieldType::FIELD_SFIXED64: {
          s64 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << val;
        } break;
        case FieldDef::eFieldType::FIELD_INT32:
        case FieldDef::eFieldType::FIELD_INT64:
        case FieldDef::eFieldType::FIELD_SINT32:
        case FieldDef::eFieldType::FIELD_SINT64: {
          s64 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << VarInt(val);
        } break;
        case FieldDef::eFieldType::FIELD_UINT32:
        case FieldDef::eFieldType::FIELD_UINT64: {
          u64 val;
          RET_M(
              lexical_cast(token.getToken(), val),
              "Could not parse " << token.getToken()
                                 << " as integral numeric value.");
          buffer << VarUInt(val);
        } break;

        case FieldDef::eFieldType::FIELD_STRING:
        case FieldDef::eFieldType::FIELD_BYTES: {
          const std::string val = Unescape(TrimQuotes(token.getToken()));
          buffer << VarUInt(val.size());
          buffer.write(ConstBlob(
              reinterpret_cast< const u8 * >(val.data()), val.size()));
        } break;

        case FieldDef::eFieldType::FIELD_ENUM: {
          if (token.getId() == eTokens::IDENTIFIER) {
            const EnumDef *pEnumDef = FindProtoEnumByName(fieldDef->m_msgType);
            RET_M(
                pEnumDef != nullptr,
                "Could not find enum: " << fieldDef->m_msgType);
            const std::string value = token.getToken();
            bool found = false;
            for (std::vector< FieldDef >::const_iterator itr =
                     pEnumDef->m_values.begin();
                 itr != pEnumDef->m_values.end();
                 ++itr) {
              if (itr->m_name == value) {
                buffer << VarUInt((u64) itr->m_fieldNum);
                found = true;
                break;
              }
            }
            if (found == false) {
              Log(LL::Error) << "Value isn't defined for enum: " << value;
              return false;
            }
          } else {
            u64 val;
            RET_M(
                lexical_cast(token.getToken(), val),
                "Could not parse " << token.getToken()
                                   << " as integral numeric value.");
            buffer << VarUInt(val);
          }
          break;
        }

        case FieldDef::eFieldType::FIELD_BOOL: {
          const std::string val = token.getToken();
          if (val == "true") {
            buffer << VarUInt(1ull);
          } else if (val == "false") {
            buffer << VarUInt(0ull);
          } else {
            Log(LL::Error) << "Expected 'true' or 'false' for boolean field: "
                           << fieldDef->m_name;
            return false;
          }
        } break;
        case FieldDef::eFieldType::FIELD_MSG:
          CHECK_UNREACHABLE();
          break;
      }
    }
  }
  if (begin != end || openedMessage) {
    return false;
  }
  return true;
}

} // namespace files
} // namespace util
} // namespace core
