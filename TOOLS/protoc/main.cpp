#include "proto_parser.h"
#include "proto_printer.h"

#include <CORE/types.h>
#include <CORE/BASE/logging.h>
#include <CORE/BASE/config.h>
#include <CORE/UTIL/stringutil.h>

#include <fstream>
#include <map>
#include <set>

using core::util::EnumDef;
using core::util::FieldDef;
using core::util::MessageDef;
using core::util::ProtoDef;
using core::util::RpcFunctionDef;
using core::util::ServiceDef;

core::config::Flag< std::string > g_astylePath("astyle_path", "Path to the AStyle.exe binary for formatting", "");
core::config::Flag< std::string > g_inputFile("infile", "Path to input proto file", "");

static bool openFile(std::string &out, const std::string &fileName);
static std::vector< std::string >::const_iterator findMatch(const std::vector< std::string > &possible, const std::string ending);
static bool fixupLocalFields(MessageDef &def, const std::vector< std::string > &possibleMessages, const std::vector< std::string > &possibleEnums);
static bool fixupLocalFields(ServiceDef &def, const std::vector< std::string > &possibleMessages, const std::vector< std::string > &possibleEnums);
static bool recursiveCollectDefs(std::vector< ProtoDef > &childDefs, std::vector< std::string > imports);
static void recursiveCollectDefines(const std::vector< MessageDef > &messages, const std::string &root, std::vector< std::string > &possibleMessages, std::vector< std::string > &possibleEnums);
static bool verifyImportsAndFixFields(ProtoDef &def);
static bool verifyDef(const ProtoDef &def);

/**
 * Main
 *
 * Required flags:
 *  --infile
 */
int main(int argc, const char **argv) {
  core::config::ParseFlags(argc, argv);

  g_inputFile.checkSet();

  const std::string &filename = g_inputFile.get();
  const std::string outFileName = core::util::replaceStr(g_inputFile.get(), ".proto", "");

  std::string str;
  if (!openFile(str, filename)) {
    Log(LL::Error) << "Unable to open file: " << filename << std::endl;
    return 1;
  }

  ProtoDef def;
  if (!proto::parse(def, str)) {
    Log(LL::Error) << "Unable to parse tokens" << std::endl;
    return 1;
  }

  if (!verifyImportsAndFixFields(def)) {
    Log(LL::Error) << "Unable to collect imports" << std::endl;
    return 1;
  }

  if (!verifyDef(def)) {
    Log(LL::Error) << "Error in definition" << std::endl;
  }

  print(def, outFileName);
  styleFile(g_astylePath.get(), outFileName);

  return 0;
}

/**
 *
 */
bool openFile(std::string &out, const std::string &fileName) {
  std::ifstream ifile(fileName);
  if (!ifile.is_open()) {
    return false;
  }
  std::copy(std::istreambuf_iterator<char>(ifile.rdbuf()), std::istreambuf_iterator<char>(), std::back_inserter(out));
  return true;
}

/**
 *
 */
std::vector< std::string >::const_iterator findMatch(const std::vector< std::string > &possible, const std::string ending) {
  std::vector< std::string >::const_iterator itr;
  for (itr = possible.begin(); itr != possible.end(); ++itr) {
    if (itr->length() < ending.length()) {
      continue;
    }

    std::string::size_type findPos = itr->find(ending);
    if (findPos == itr->npos) {
      continue;
    }
    if (itr->length() - findPos == ending.length()) {
      break;
    }
  }
  return itr;
}

/**
 *
 */
static std::string fixMessageString(const std::string &str) {
  if (str.find_first_of(".") != str.npos) {
    return "::" + core::util::replaceStr(str, ".", "::");
  } else {
    return "::" + str;
  }
}

/**
 * Fixup the message type names to be fully specified paths.
 * Fixup messages that are actually enums to be marked as enums.
 */
bool fixupLocalFields(MessageDef &def, const std::vector< std::string > &possibleMessages, const std::vector< std::string > &possibleEnums) {
  for (std::vector< MessageDef >::iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    fixupLocalFields(*message, possibleMessages, possibleEnums);
  }

  for (std::vector< FieldDef >::iterator field = def.m_fields.begin(); field != def.m_fields.end(); ++field) {
    if (field->m_type != FieldDef::FIELD_MSG) {
      continue;
    }

    field->m_msgType = fixMessageString(field->m_msgType);

    std::vector< std::string >::const_iterator itr;
    itr = findMatch(possibleMessages, field->m_msgType);
    if (itr == possibleMessages.end()) {
      itr = findMatch(possibleEnums, field->m_msgType);
      if (itr == possibleEnums.end()) {
        Log(LL::Error) << "Unable to locate definition for: " << field->m_msgType << std::endl;
        return false;
      }
      field->m_msgType = *itr;
      field->m_type = FieldDef::FIELD_ENUM;
    } else {
      field->m_msgType = *itr;
    }
  }
  return true;
}

/**
 * Fixup the message type names to be fully specified paths.
 * Fixup messages that are actually enums to be marked as enums.
 */
static bool fixupLocalFields(ServiceDef &def, const std::vector< std::string > &possibleMessages, const std::vector< std::string > &possibleEnums) {
  for (std::vector< RpcFunctionDef >::iterator func = def.m_functions.begin(); func != def.m_functions.end(); ++func) {
    func->m_return = fixMessageString(func->m_return);
    func->m_param = fixMessageString(func->m_param);

    std::vector< std::string >::const_iterator itr;
    itr = findMatch(possibleMessages, func->m_return);
    if (itr == possibleMessages.end()) {
      itr = findMatch(possibleEnums, func->m_return);
      if (itr == possibleEnums.end()) {
        Log(LL::Error) << "Unable to locate definition for return value: " << func->m_return << std::endl;
        return false;
      }
      func->m_return = *itr;
    } else {
      func->m_return = *itr;
    }

    itr = findMatch(possibleMessages, func->m_param);
    if (itr == possibleMessages.end()) {
      itr = findMatch(possibleEnums, func->m_param);
      if (itr == possibleEnums.end()) {
        Log(LL::Error) << "Unable to locate definition for parameter value: " << func->m_param << std::endl;
        return false;
      }
      func->m_param = *itr;
    } else {
      func->m_param = *itr;
    }
  }
  return true;
}

/**
 * Open all the imports and attempt to parse them for definitions
 */
bool recursiveCollectDefs(std::vector< ProtoDef > &childDefs, std::vector< std::string > imports) {
  for (std::vector< std::string >::const_iterator itr = imports.begin(); itr != imports.end(); ++itr) {
    std::string str;
    if (!openFile(str, *itr)) {
      Log(LL::Error) << "Unable to open import: " << *itr << std::endl;
      return false;
    }

    ProtoDef def;
    if (!proto::parse(def, str)) {
      Log(LL::Error) << "Unable to parse import: " << *itr << std::endl;
      return false;
    }
    childDefs.push_back(def);
    recursiveCollectDefs(childDefs, def.m_imports);
  }

  return true;
}

/**
 * Collect all the possible names of messages and enums that we know exist based on the imports.
 */
void recursiveCollectDefines(const std::vector< MessageDef > &messages, const std::string &root, std::vector< std::string > &possibleMessages, std::vector< std::string > &possibleEnums) {
  for (std::vector< MessageDef >::const_iterator message = messages.begin(); message != messages.end(); ++message) {
    const std::string packageSelf = root + "::" + message->m_name;
    recursiveCollectDefines(message->m_messages, packageSelf, possibleMessages, possibleEnums);
    possibleMessages.push_back(packageSelf);
    for (std::vector< EnumDef >::const_iterator itr = message->m_enums.begin(); itr != message->m_enums.end(); ++itr) {
      possibleEnums.push_back(packageSelf + "::" + itr->m_name);
    }
  }
}

/**
 * Verify that the imports represent all the correct data
 */
bool verifyImportsAndFixFields(ProtoDef &def) {
  std::vector< ProtoDef > allDefs;
  allDefs.push_back(def);
  if (!recursiveCollectDefs(allDefs, def.m_imports)) {
    return false;
  }

  std::vector< std::string > possibleMessages;
  std::vector< std::string > possibleEnums;
  for (std::vector< ProtoDef >::const_iterator itr = allDefs.begin(); itr != allDefs.end(); ++itr) {
    const std::string packageSelf = "::" + core::util::replaceStr(itr->m_package, ".", "::");
    recursiveCollectDefines(itr->m_messages, packageSelf, possibleMessages, possibleEnums);
  }

  for (std::vector< MessageDef >::iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    if (!fixupLocalFields(*message, possibleMessages, possibleEnums)) {
      return false;
    }
  }

  for (std::vector< ServiceDef >::iterator service = def.m_services.begin(); service != def.m_services.end(); ++service) {
    if (!fixupLocalFields(*service, possibleMessages, possibleEnums)) {
      return false;
    }
  }

  return true;
}

/**
 *
 */
bool verifyMsg(const MessageDef &def) {
  std::set< s32 > knownFields;
  for (std::vector< FieldDef >::const_iterator itr = def.m_fields.begin(); itr != def.m_fields.end(); ++itr) {
    const s32 fieldNum = itr->m_fieldNum;
    RET_M(fieldNum > 0, "Field numbers must be positive.");
    RET_M(knownFields.find(fieldNum) == knownFields.end(), "Field number " << fieldNum << " is repeated.");
    knownFields.insert(fieldNum);
  }

  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    if (!verifyMsg(*message)) {
      return false;
    }
  }

  for (std::vector< EnumDef >::const_iterator message = def.m_enums.begin(); message != def.m_enums.end(); ++message) {
    std::set< s32 > knownFields;
    for (std::vector< FieldDef >::const_iterator itr = message->m_values.begin(); itr != message->m_values.end(); ++itr) {
      const s32 fieldNum = itr->m_fieldNum;
      RET_M(knownFields.find(fieldNum) == knownFields.end(), "Enumeration number " << fieldNum << " is repeated.");
      RET_M(fieldNum != 0, "Enum 0 may not be specified.");
      RET_M(fieldNum >= 0, "Enum values must be positive.");
      knownFields.insert(fieldNum);
    }
  }
  return true;
}

/**
 * Verify def
 */
bool verifyDef(const ProtoDef &def) {
  for (std::vector< MessageDef >::const_iterator message = def.m_messages.begin(); message != def.m_messages.end(); ++message) {
    if (!verifyMsg(*message)) {
      return false;
    }
  }
  return true;
}
