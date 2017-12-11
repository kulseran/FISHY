/**
 * Proto printer to generate code
 */
#ifndef FISHY_PROTO_PRINTER_H
#define FISHY_PROTO_PRINTER_H

#include "proto_parser.h"

/**
 * Print a {@link ProtoDef} to the given output file root.
 * This outputs a pair of files:
 *    ${fileNameRoot}.pb.h
 *    ${fileNameRoot}.ph.cpp
 */
bool print(const core::types::ProtoDef &def, const std::string &fileNameRoot);

/**
 * Styles the files output by {@link print} for proper indentation.
 */
void styleFile(const std::string &astylePath, const std::string &fileNameRoot);

#endif
