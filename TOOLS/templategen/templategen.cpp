#include "templategen.h"

#include <fstream>
#include <iostream>
#include <map>

namespace templategen {

#define INC()                                              \
  do {                                                     \
    itr++;                                                 \
    if (itr == in.end()) {                                 \
      std::cerr << "Unexpected end of file." << std::endl; \
      return false;                                        \
    }                                                      \
  } while (0)

#define FIND_END(j)                                        \
  do {                                                     \
    j++;                                                   \
    if (j == in.end()) {                                   \
      std::cerr << "Unexpected end of file." << std::endl; \
      return false;                                        \
    }                                                      \
  } while (*j != '$')

typedef std::map< std::string, std::string > tVarMap;

static bool format(const std::string &in, std::string &out, tVarMap &varMap) {

  std::string::const_iterator itr = in.begin();

  while (itr != in.end()) {
    if (*itr != '$') {
      out.push_back(*itr);
      itr++;
      continue;
    }
    INC();
    if (*itr == '$') {
      out.push_back(*itr);
      itr++;
      continue;
    }
    if (*itr == 'i') {
      INC();
      if (*itr != ' ') {
        std::cerr << "expected space after $i" << std::endl;
        return false;
      }
      INC();
      std::string::const_iterator filenameStart = itr;
      FIND_END(itr);
      std::string::const_iterator filenameEnd = itr;

      const std::string fileName(filenameStart, filenameEnd);
      std::ifstream ifile(fileName.c_str(), std::ios::in | std::ios::binary);
      if (!ifile.is_open()) {
        std::cerr << "Could not open import: " << fileName << std::endl;
        return false;
      }
      ifile.seekg(0, std::ios::end);
      const size_t fileLen = static_cast< size_t >(ifile.tellg());
      ifile.seekg(0, std::ios::beg);
      std::string content;
      content.resize(fileLen, 0);
      ifile.read(&content[0], fileLen);
      if (ifile.gcount() != fileLen) {
        std::cerr << "Unable to read import: " << fileName << std::endl;
        return false;
      }
      std::string contentOut;
      if (!format(content, contentOut, varMap)) {
        std::cerr << "Unable to grok import: " << fileName << std::endl;
        return false;
      }
      out.append(contentOut);
    } else if (*itr == 'v') {
      INC();
      if (*itr != ' ') {
        std::cerr << "expected space after $v" << std::endl;
        return false;
      }
      INC();
      const std::string::const_iterator varNameStart = itr;
      FIND_END(itr);
      const std::string::const_iterator varNameEnd = itr;
      INC();
      const std::string::const_iterator valueStart = itr;
      FIND_END(itr);
      const std::string::const_iterator valueEnd = itr;
      const std::string name(varNameStart, varNameEnd);
      const std::string value(valueStart, valueEnd);
      varMap[name] = value;
    } else if (*itr == '%') {
      INC();
      const std::string::const_iterator varNameStart = itr;
      FIND_END(itr);
      const std::string::const_iterator varNameEnd = itr;

      const std::string varname(varNameStart, varNameEnd);
      const tVarMap::const_iterator value = varMap.find(varname);
      if (value == varMap.end()) {
        std::cerr << "Var " << varname << " is not set within script."
                  << std::endl;
        return false;
      }
      out.append(value->second);
    } else if (*itr == '?') {
      INC();
      const std::string::const_iterator varNameStart = itr;
      FIND_END(itr);
      const std::string::const_iterator varNameEnd = itr;

      const std::string varname(varNameStart, varNameEnd);
      const tVarMap::const_iterator value = varMap.find(varname);
      if (value != varMap.end()) {
        out.append(value->second);
      }
    } else {
      std::cerr << "Unknown $ operator" << std::endl;
      return false;
    }

    if (itr != in.end()) {
      itr++;
    }
  }

  return true;
}

bool format(const std::string &in, std::string &out) {
  out.reserve(in.length() * 2);
  tVarMap varMap;
  return format(in, out, varMap);
}

} // namespace templategen
