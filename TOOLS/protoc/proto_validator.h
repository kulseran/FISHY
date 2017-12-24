/**
 * Validation of proto defitions.
 */
#ifndef FISHY_PROTO_VALIDATOR_H
#define FISHY_PROTO_VALIDATOR_H

#include <CORE/TYPES/protobuf.h>

/**
 * Verifies that the {@link ProtoDef} is valid, and logs any errors encountered
 *
 * @param def the {@link ProtoDef} to validate.
 * @return true on success, false if there are any errors.
 */
bool verifyDef(const core::types::ProtoDef &def);

#endif
