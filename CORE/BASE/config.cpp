#include "config.h"

#include <CORE/BASE/checks.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/stringutil.h>

#include <fstream>
#include <map>
#include <vector>

using core::util::Splitter;
using core::util::TrimQuotes;

namespace core {
namespace config {

/**
 * Default available flag.
 * --config_file will be parsed one flag per line to set flags in the system.
 */
Flag< std::string > g_configFile(
    "config_file",
    "File containing one cli flag name space value pair per line.",
    "./settings.txt");

/**
 * Global flag mapping
 */
typedef std::map< std::string, iFlagBase * >::iterator tFlagIter;
static std::map< std::string, iFlagBase * > &GetGlobalFlags() {
  static std::map< std::string, iFlagBase * > g_globalFlags;
  return g_globalFlags;
}

/**
 *
 */
static bool RegisterFlag(iFlagBase *pFlag) {
  if (GetGlobalFlags().find(std::string(pFlag->getName()))
      != GetGlobalFlags().end()) {
    return false;
  }
  GetGlobalFlags().insert(std::make_pair(std::string(pFlag->getName()), pFlag));
  return true;
}

/**
 *
 */
void PrintFlags() {
  for (tFlagIter itr = GetGlobalFlags().begin(); itr != GetGlobalFlags().end();
       ++itr) {
    const iFlagBase *pFlag = itr->second;
    std::cout << pFlag->getName() << ": " << pFlag->getDesc()
              << " (= " << pFlag->toString() << ")" << std::endl;
  }
}

/**
 *
 */
iFlagBase::iFlagBase(const char *name, const char *desc)
    : m_name(name), m_desc(desc), m_set(false) {
  CHECK_M(RegisterFlag(this), "Flag --" << getName() << " already registered!");
}

/**
 *
 */
void iFlagBase::checkSet() const {
  if (!m_set) {
    Log(LL::Error) << "Required flag --" << getName()
                   << " was not set. (usage: " << getDesc() << ")";
    ASSERT(0);
    exit(-Status::BAD_ARGUMENT);
  }
}

/**
 * Parses a config file with lines of the form
 *
 *     flag = value
 *
 * and sets flags as appropriate.
 */
Status ParseConfigFile() {
  Trace();
  if (!g_configFile.wasSet()) {
    Log(LL::Trace) << "No flag config file specified.";
    return Status::ok();
  }
  const std::string &filename = g_configFile.get();
  Log(LL::Trace) << "Begining parse of config file: " << filename;

  std::ifstream ifile(filename.c_str());
  if (!ifile.is_open()) {
    Log(LL::Trace) << "Config file " << filename << " not found.";
    return Status(Status::NOT_FOUND);
  }

  std::string line;
  int lineno = 0;
  while (std::getline(ifile, line)) {
    if (line.empty()) {
      lineno++;
      continue;
    }

    std::vector< std::string > argstr =
        Splitter().on('=').trimWhitespace().split(line, 2);

    RET_SM(
        argstr.size() == 2,
        Status::BAD_INPUT,
        "Error in config file \"" << filename << "\" "
                                  << "on line " << lineno
                                  << " is not a valid flag=value pair");

    tFlagIter itr = GetGlobalFlags().find(argstr[0]);
    RET_SM(
        itr != GetGlobalFlags().end(),
        Status::BAD_INPUT,
        "Unknown flag: " << argstr[0] << " @ line " << lineno);

    iFlagBase *pFlag = itr->second;
    RET_SM(
        pFlag->fromString(TrimQuotes(argstr[1])),
        Status::BAD_INPUT,
        "Invalid flag value for " << argstr[0] << " at line " << lineno
                                  << ". can't parse: " << argstr[1]);

    lineno++;
  }
  return Status::ok();
}

/**
 *
 */
Status ParseFlags(const int argc, const char **argv) {
  Trace();

  for (int i = 0; i < argc; ++i) {
    Log(LL::Info) << "argv[" << i << "] = " << argv[i];
  }

  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if ((arg[0] != '-') || (arg[0] != '\0' && arg[1] != '-')) {
      Log(LL::Error) << "Unknown commandline input #" << i << ": " << arg;
      return Status::BAD_ARGUMENT;
    }

    std::vector< std::string > argstr =
        Splitter().on('=').trimWhitespace().split(std::string(arg + 2), 2);
    RET_SM(!argstr.empty(), Status::BAD_ARGUMENT, "Unknown flag: " << arg);

    tFlagIter itr = GetGlobalFlags().find(argstr[0]);
    RET_SM(
        itr != GetGlobalFlags().end(),
        Status::BAD_ARGUMENT,
        "Unknown flag: " << arg);

    iFlagBase *pFlag = itr->second;

    if (argstr.size() == 2) {
      // Parse case where we have [flag][=][value]
      RET_SM(
          pFlag->fromString(argstr.at(1)),
          Status::BAD_ARGUMENT,
          "Invalid flag value for " << argstr[0] << " can't parse "
                                    << argstr[1]);
    } else if (i + 1 < argc) {
      // Parse case where we have [flag][space][value]
      ++i;
      RET_SM(
          pFlag->fromString(std::string(argv[i])),
          Status::BAD_ARGUMENT,
          "Invalid flag value for " << argstr[0] << " can't parse " << argv[i]);
    } else {
      // Error on case where we have [flag] alone.  Booleans must be specified
      // as [flag][=][true]
      Log(LL::Error) << "Found flag with no value: " << argstr[0];
      return Status::BAD_ARGUMENT;
    }
  }

  return ParseConfigFile();
}

} // namespace config
} // namespace core
