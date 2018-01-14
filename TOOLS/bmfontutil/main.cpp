#include <APP_SHARED/app_main.h>
#include <APP_SHARED/fileutil.h>
#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
#include <CORE/VFS/vfs.h>

#include "font_parser.h"

#include <iostream>
#include <string>

using bmfont::FontDef;
using core::util::lexical_cast;
using core::util::files::XmlNode;

FISHY_APPLICATION(BmFontUtil);

core::config::Flag< std::string > g_inputFile("infile", "Input filename", "");
core::config::Flag< std::string >
    g_outputFile("outfile", "Output filename", "");
core::config::Flag< bool >
    g_exportAsDistanceFont("export_as_distance_font", false);

/**
 *
 */
void BmFontUtil::printHelp() {
  std::cout << "Usage:" << std::endl;
  std::cout << "    bmfontutil --infile <i> --outfile <o>" << std::endl;
}

/**
 *
 */
void BmFontUtil::init(const char *arg0) {
  (void) arg0;
}

/**
 *
 */
int BmFontUtil::main() {
  g_inputFile.checkSet();
  g_outputFile.checkSet();

  vfs::Mount("./", "./", std::ios::in | std::ios::out | std::ios::binary);

  const vfs::Path filename(g_inputFile.get());
  const vfs::Path rootPath(filename.dir());

  std::string content;
  if (!appshared::parseFileToString(filename, content)) {
    Log(LL::Error) << "Unable to open input font set: " << g_inputFile.get();
    return eExitCode::BAD_FILE;
  }

  XmlNode doc;
  if (!doc.parse(content)) {
    Log(LL::Error) << "Unable to parse XML document.";
    return eExitCode::BAD_DATA;
  }

  size_t nodeIndex;
  if (!doc.getChild("font", nodeIndex)) {
    Log(LL::Error)
        << "XML is not a valid font file. Missing top level 'font' node.";
    return eExitCode::BAD_DATA;
  }

  FontDef def;
  if (!ParseDocument(
          def,
          rootPath,
          doc.getChild(nodeIndex),
          g_exportAsDistanceFont.get())) {
    return eExitCode::BAD_DATA;
  }

  if (!appshared::serializeProtoToFile(vfs::Path(g_outputFile.get()), def)) {
    Log(LL::Error) << "Unable to write output.";
    return eExitCode::EXCEPTION;
  }
  return eExitCode::OK;
}
