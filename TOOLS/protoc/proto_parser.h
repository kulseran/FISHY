/**
 * Proto Parser
 */
#ifndef FISHY_PROTO_PARSER_H
#define FISHY_PROTO_PARSER_H

#include <CORE/TYPES/protobuf.h>

#include <string>

namespace proto {

/**
 * @param out the parsed definition, if the return is true.
 * @param fileData input file as a string.
 * @return false on error. Errors are logged.
 */
bool parse(core::types::ProtoDef &out, const std::string &fileData);

} // namespace proto

#endif
