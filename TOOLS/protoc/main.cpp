#include "proto_parser.h"
#include "proto_printer.h"
#include "proto_validator.h"

#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/types.h>

#include <fstream>
#include <map>
#include <set>

using core::types::EnumDef;
using core::types::FieldDef;
using core::types::MessageDef;
using core::types::ProtoDef;
using core::types::RpcFunctionDef;
using core::types::ServiceDef;
using core::util::ReplaceStr;

core::config::Flag< std::string >
    g_inputFile("infile", "Path to input proto file", "");

static Status openFile(std::string &out, const std::string &fileName);
static std::vector< std::string >::const_iterator
findMatch(const std::vector< std::string > &possible, const std::string ending);
static bool fixupLocalFields(
    MessageDef &def,
    const std::vector< std::string > &possibleMessages,
    const std::vector< std::string > &possibleEnums);
static bool fixupLocalFields(
    ServiceDef &def,
    const std::vector< std::string > &possibleMessages,
    const std::vector< std::string > &possibleEnums);
static bool recursiveCollectDefs(
    std::vector< ProtoDef > &childDefs, std::vector< std::string > imports);
static void recursiveCollectDefines(
    const std::vector< MessageDef > &messages,
    const std::string &root,
    std::vector< std::string > &possibleMessages,
    std::vector< std::string > &possibleEnums);
static bool verifyImportsAndFixFields(ProtoDef &def);

/**
 * Main
 *
 * Required flags:
 *  --infile
 */
int main(int argc, const char **argv) {
  if (!core::config::ParseFlags(argc, argv)) {
    return 1;
  }
  core::logging::RegisterSink(std::shared_ptr< core::logging::iLogSink >(
      new core::logging::LoggingStdioSink(
          core::types::BitSet< LL >() | LL::Error | LL::Warning | LL::Info)));

  g_inputFile.checkSet();

  const std::string &filename = g_inputFile.get();
  const std::string outFileName = ReplaceStr(g_inputFile.get(), ".proto", "");

  std::string str;
  if (!openFile(str, filename)) {
    Log(LL::Error) << "Unable to open file: " << filename;
    return 1;
  }

  ProtoDef def;
  if (!proto::parse(def, str)) {
    Log(LL::Error) << "Unable to parse tokens.";
    return 1;
  }

  if (!verifyImportsAndFixFields(def)) {
    Log(LL::Error) << "Unable to collect imports.";
    return 1;
  }

  if (!verifyDef(def)) {
    Log(LL::Error) << "Error in definition.";
    return 1;
  }

  print(def, outFileName);

  return 0;
}

/**
 *
 */
Status openFile(std::string &out, const std::string &fileName) {
  std::ifstream ifile(fileName);
  if (!ifile.is_open()) {
    return Status::NOT_FOUND;
  }
  std::copy(
      std::istreambuf_iterator< char >(ifile.rdbuf()),
      std::istreambuf_iterator< char >(),
      std::back_inserter(out));
  return Status::OK;
}

/**
 *
 */
std::vector< std::string >::const_iterator findMatch(
    const std::vector< std::string > &possible, const std::string ending) {
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
    return "::" + ReplaceStr(str, ".", "::");
  } else {
    return "::" + str;
  }
}

/**
 * Fixup the message type names to be fully specified paths.
 * Fixup messages that are actually enums to be marked as enums.
 */
bool fixupLocalFields(
    MessageDef &def,
    const std::vector< std::string > &possibleMessages,
    const std::vector< std::string > &possibleEnums) {
  for (std::vector< MessageDef >::iterator message = def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    fixupLocalFields(*message, possibleMessages, possibleEnums);
  }

  for (std::vector< FieldDef >::iterator field = def.m_fields.begin();
       field != def.m_fields.end();
       ++field) {
    if (field->m_type != FieldDef::FIELD_MSG) {
      continue;
    }

    field->m_msgType = fixMessageString(field->m_msgType);

    std::vector< std::string >::const_iterator itr;
    itr = findMatch(possibleMessages, field->m_msgType);
    if (itr == possibleMessages.end()) {
      itr = findMatch(possibleEnums, field->m_msgType);
      if (itr == possibleEnums.end()) {
        Log(LL::Error) << "Unable to locate definition for: "
                       << field->m_msgType;
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
static bool fixupLocalFields(
    ServiceDef &def,
    const std::vector< std::string > &possibleMessages,
    const std::vector< std::string > &possibleEnums) {
  for (std::vector< RpcFunctionDef >::iterator func = def.m_functions.begin();
       func != def.m_functions.end();
       ++func) {
    func->m_return = fixMessageString(func->m_return);
    func->m_param = fixMessageString(func->m_param);

    std::vector< std::string >::const_iterator itr;
    itr = findMatch(possibleMessages, func->m_return);
    if (itr == possibleMessages.end()) {
      itr = findMatch(possibleEnums, func->m_return);
      if (itr == possibleEnums.end()) {
        Log(LL::Error) << "Unable to locate definition for return value: "
                       << func->m_return;
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
        Log(LL::Error) << "Unable to locate definition for parameter value: "
                       << func->m_param;
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
bool recursiveCollectDefs(
    std::vector< ProtoDef > &childDefs, std::vector< std::string > imports) {
  for (std::vector< std::string >::const_iterator itr = imports.begin();
       itr != imports.end();
       ++itr) {
    std::string str;
    if (!openFile(str, *itr)) {
      Log(LL::Error) << "Unable to open import: " << *itr;
      return false;
    }

    ProtoDef def;
    if (!proto::parse(def, str)) {
      Log(LL::Error) << "Unable to parse import: " << *itr;
      return false;
    }
    childDefs.push_back(def);
    recursiveCollectDefs(childDefs, def.m_imports);
  }

  return true;
}

/**
 * Collect all the possible names of messages and enums that we know exist based
 * on the imports.
 */
void recursiveCollectDefines(
    const std::vector< MessageDef > &messages,
    const std::string &root,
    std::vector< std::string > &possibleMessages,
    std::vector< std::string > &possibleEnums) {
  for (std::vector< MessageDef >::const_iterator message = messages.begin();
       message != messages.end();
       ++message) {
    const std::string packageSelf = root + "::" + message->m_name;
    recursiveCollectDefines(
        message->m_messages, packageSelf, possibleMessages, possibleEnums);
    possibleMessages.push_back(packageSelf);
    for (std::vector< EnumDef >::const_iterator itr = message->m_enums.begin();
         itr != message->m_enums.end();
         ++itr) {
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
  for (std::vector< ProtoDef >::const_iterator itr = allDefs.begin();
       itr != allDefs.end();
       ++itr) {
    const std::string packageSelf =
        "::" + ReplaceStr(itr->m_package, ".", "::");
    recursiveCollectDefines(
        itr->m_messages, packageSelf, possibleMessages, possibleEnums);
  }

  for (std::vector< MessageDef >::iterator message = def.m_messages.begin();
       message != def.m_messages.end();
       ++message) {
    if (!fixupLocalFields(*message, possibleMessages, possibleEnums)) {
      return false;
    }
  }

  for (std::vector< ServiceDef >::iterator service = def.m_services.begin();
       service != def.m_services.end();
       ++service) {
    if (!fixupLocalFields(*service, possibleMessages, possibleEnums)) {
      return false;
    }
  }

  return true;
}
