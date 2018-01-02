/**
 * Print functions for the Header part of a generated proto message.
 */
#ifndef FISHY_PROTO_PRINTER_HEADER_H
#define FISHY_PROTO_PRINTER_HEADER_H

#include <CORE/TYPES/protobuf.h>

#include <string>

bool PrintHeader(const core::types::ProtoDef &def, const std::string &fileName);

#endif
