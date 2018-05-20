#include <fstream>
#include <iostream>
#include <string>

#include "templategen.h"

int main(int argv, char **argc) {
  if (argv != 3) {
    std::cerr << "Requires 2 arguments, input and output file." << std::endl;
    return -1;
  }
  if (strcmp(argc[1], argc[2]) == 0) {
    std::cerr << "Input and output files cannot be the same." << std::endl;
    return -1;
  }
  if (std::ifstream(argc[2], std::ios::in).is_open()) {
    std::cerr << "Cannot overwrite existing output file: " << argc[2]
              << std::endl;
    return -1;
  }

  std::ifstream ifile(argc[1], std::ios::in | std::ios::binary);
  if (!ifile.is_open()) {
    std::cerr << "Cannot find input file: " << argc[1] << std::endl;
    return -1;
  }
  ifile.seekg(0, std::ios::end);
  const size_t fileLen = static_cast< size_t >(ifile.tellg());
  ifile.seekg(0, std::ios::beg);
  std::string content;
  content.resize(fileLen, 0);
  ifile.read(&content[0], fileLen);
  if (ifile.gcount() != fileLen) {
    std::cerr << "Unable to read input file. Got " << ifile.gcount() << " of "
              << fileLen << " bytes." << std::endl;
    return -1;
  }
  ifile.close();

  std::string contentOut;
  if (!templategen::format(content, contentOut)) {
    std::cerr << "Unable to grok file content: " << argc[1] << std::endl;
    return -1;
  }

  std::ofstream ofile(argc[2], std::ios::out | std::ios::binary);
  if (!ofile.is_open()) {
    std::cerr << "Unable to output output file: " << argc[2] << std::endl;
    return -1;
  }
  ofile.write(&contentOut[0], contentOut.length());

  return 0;
}
