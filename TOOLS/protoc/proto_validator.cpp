#include "proto_validator.h"

#include <CORE/BASE/logging.h>

#include <set>

using core::types::EnumDef;
using core::types::FieldDef;
using core::types::MessageDef;
using core::types::ProtoDef;
using core::types::RpcFunctionDef;
using core::types::ServiceDef;

/**
 *
 */
bool verifyService(const ServiceDef &def) {
  std::set< std::string > knownFunctions;
  for (std::vector< RpcFunctionDef >::const_iterator itr =
           def.m_functions.begin();
       itr != def.m_functions.end();
       ++itr) {
    RET_M(
        knownFunctions.find(itr->m_name) == knownFunctions.end(),
        "Service function " << itr->m_name << " is defined more than once.");
    knownFunctions.insert(itr->m_name);
  }
  return true;
}

/**
 *
 */
bool verifyMessage(const MessageDef &def) {

  // Verify:
  // * field numbers are valid
  // * field numbers don't repeat
  // * field names don't repeat
  std::set< s32 > knownFieldNums;
  std::set< std::string > knownFieldNames;
  for (std::vector< FieldDef >::const_iterator itr = def.m_fields.begin();
       itr != def.m_fields.end();
       ++itr) {
    const s32 fieldNum = itr->m_fieldNum;
    const std::string &fieldName = itr->m_name;
    RET_M(fieldNum > 0, "Field numbers must be positive.");
    RET_M(
        knownFieldNums.find(fieldNum) == knownFieldNums.end(),
        "Field number " << fieldNum << " is defined more than once.");
    knownFieldNums.insert(fieldNum);
    RET_M(
        knownFieldNames.find(fieldName) == knownFieldNames.end(),
        "Field " << fieldName << " is defined more than once.");
    knownFieldNames.insert(fieldName);
  }

  // Verify:
  // * Enum numbers are valid
  // * Enum field names don't repeat
  // * Enum names don't repeat
  std::set< std::string > knownEnumNames;
  for (std::vector< EnumDef >::const_iterator message = def.m_enums.begin();
       message != def.m_enums.end();
       ++message) {
    RET_M(
        knownEnumNames.find(message->m_name) == knownEnumNames.end(),
        "Enumeration " << message->m_name << " is defined more than once.");
    knownEnumNames.insert(message->m_name);

    std::set< s32 > knownEnumNums;
    std::set< std::string > knownEnumFieldNames;
    for (std::vector< FieldDef >::const_iterator itr =
             message->m_values.begin();
         itr != message->m_values.end();
         ++itr) {
      const s32 fieldNum = itr->m_fieldNum;
      const std::string fieldName = itr->m_name;
      RET_M(fieldNum != 0, "Enum 0 may not be specified.");
      RET_M(fieldNum >= 0, "Enum values must be positive.");
      RET_M(
          knownEnumNums.find(fieldNum) == knownEnumNums.end(),
          "Enumeration number " << fieldNum << " is defined more than once.");
      knownEnumNums.insert(fieldNum);
      RET_M(
          knownEnumFieldNames.find(fieldName) == knownEnumFieldNames.end(),
          "Enumeration field " << fieldName << " is defined more than once.");
      knownEnumFieldNames.insert(fieldName);
    }
  }

  // Verify child messages.
  std::set< std::string > knownMsgs;
  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    RET_M(
        knownMsgs.find(message->m_name) == knownMsgs.end(),
        "Message " << message->m_name << " is defined more than once.");
    knownMsgs.insert(message->m_name);
    if (!verifyMessage(*message)) {
      return false;
    }
  }
  return true;
}

/**
 *
 */
bool verifyDef(const ProtoDef &def) {
  RET_M(!def.m_package.empty(), "Protodef requires a package.");

  std::set< std::string > knownMsgs;
  // Verify all contained messages.
  // Verify no message repeats.
  for (std::vector< MessageDef >::const_iterator message =
           def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    RET_M(
        knownMsgs.find(message->m_name) == knownMsgs.end(),
        "Message " << message->m_name << " is defined more than once.");
    knownMsgs.insert(message->m_name);
    if (!verifyMessage(*message)) {
      return false;
    }
  }

  // Verify all contained services.
  // Verify no service repeats, or overlap with message names.
  for (std::vector< ServiceDef >::const_iterator service =
           def.m_services.begin();
       service != def.m_services.end();
       ++service) {
    RET_M(
        knownMsgs.find(service->m_name) == knownMsgs.end(),
        "Service " << service->m_name << " is defined more than once.");
    knownMsgs.insert(service->m_name);
    if (!verifyService(*service)) {
      return false;
    }
  }
  return true;
}
