/**
 * Proto printer to generate code
 */
#ifndef FISHY_PROTO_PRINTER_H
#define FISHY_PROTO_PRINTER_H

#include <CORE/TYPES/protobuf.h>

/**
 * Print a {@link ProtoDef} to the given output file root.
 * This outputs a pair of files:
 *    ${fileNameRoot}.pb.h
 *    ${fileNameRoot}.ph.cpp
 */
bool print(const core::types::ProtoDef &def, const std::string &fileNameRoot);

#endif
