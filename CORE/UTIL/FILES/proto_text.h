/**
 * proto_text.h
 *
 * Fishy protobuffer text formatting
 */
#ifndef FISHY_PROTO_TEXT_H
#define FISHY_PROTO_TEXT_H

#include <CORE/BASE/status.h>
#include <CORE/TYPES/protobuf.h>

namespace core {
namespace util {
namespace files {

/**
 * Protobuffer parsing and formating to Text
 */
class TextFormat {
  public:
  /**
   * Pretty prints a proto to a string.
   *
   * @param out the output string on success
   * @param msg the input message to print
   */
  static void format(std::string &out, const ::core::types::iProtoMessage &msg);

  /**
   * Read a pretty printed string in as a proto message.
   *
   * @param out the output message on success
   * @param protoText the text to parse
   */
  static Status parse(
      ::core::types::iProtoMessage &out,
      const std::string &protoText,
      bool errorOnUnknown = false);
};

} // namespace files
} // namespace util
} // namespace core

#endif
