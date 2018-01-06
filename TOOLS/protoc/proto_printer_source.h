/**
 * Print functions for the Souce part of a generated proto message.
 */
#ifndef FISHY_PROTO_PRINTER_SOURCE_H
#define FISHY_PROTO_PRINTER_SOURCE_H

#include <CORE/TYPES/protobuf.h>

#include <string>

bool PrintCpp(
    const core::types::ProtoDef &def,
    const std::string &headerName,
    const std::string &fileName);

#endif
