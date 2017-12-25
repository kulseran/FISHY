#include <APP_SHARED/app_main.h>
#include <APP_SHARED/fileutil.h>

#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
#include <CORE/HASH/crc32.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/VFS/vfs.h>
#include <CORE/VFS/vfs_file.h>
#include <CORE/VFS/vfs_util.h>
#include <CORE/types.h>

#include <algorithm>
#include <iostream>

FISHY_APPLICATION(Bin2H)

using core::config::Flag;
using core::util::IdentifierSafe;

static const char *hex(unsigned char ch);
static const char hexCh(unsigned char ch);
static Status ProcessFile();

Flag< std::string > g_inputFileName("infile", "input file name", "");
Flag< std::string > g_outputFileName("outfile", "output file name", "");
Flag< std::string >
    g_outDataName("dataname", "variable name to export data as", "");
Flag< bool > g_allowOverwrite(
    "allow_overwrite", "allow overwriting existing files", false);

/**
 *
 */
void Bin2H::init(const char *arg0) {
  (void) arg0;
}

/**
 *
 */
void Bin2H::printHelp() {
  std::cout << "Process a file inot an array in an include header."
            << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "    bin2h <input> <output> <dataname>" << std::endl;
}

/**
 *
 */
int Bin2H::main() {
  const vfs::tMountId writeableMount = vfs::Mount(
      vfs::Path("./"),
      vfs::Path("./"),
      std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  CHECK_M(
      writeableMount != vfs::INVALID_MOUNT_ID,
      "Could not mount drive for writing.");

  g_inputFileName.checkSet();
  g_outputFileName.checkSet();
  g_outDataName.checkSet();
  CHECK_M(
      g_inputFileName.get() != g_outputFileName.get(),
      "Please specify different files for --infile and --outfile");
  CHECK_M(
      vfs::Path(g_outputFileName.get()).extension() == "h",
      "--outfile should be of type .h");
  CHECK_M(
      IdentifierSafe(g_outDataName.get()) == g_outDataName.get(),
      "Please specify a valid identifier name as --dataname");

  const int ret = ProcessFile() ? 0 : -1;
  vfs::Unmount(writeableMount);
  return ret;
}

/**
 *
 */
Status ProcessFile() {
  std::string inputContent;
  Status ret = appshared::parseFileToString(
      vfs::Path(g_inputFileName.get()), inputContent);
  RET_SM(
      ret,
      ret,
      "Unable to read input file " << g_inputFileName.get() << " error "
                                   << ret.getStatus() << ":"
                                   << ret.getMessage());

  std::string datanameCaps(g_outDataName.get());
  std::transform(
      datanameCaps.begin(),
      datanameCaps.end(),
      datanameCaps.begin(),
      static_cast< int (*)(int) >(toupper));

  RET_SM(
      g_allowOverwrite.get()
          || !vfs::util::Stat(vfs::Path(g_outputFileName.get())).m_exists,
      Status::BAD_ARGUMENT,
      "Unable to overwrite existing file. Use --allow_overwrite if this is "
      "intended.");

  vfs::ofstream ofile(vfs::Path(g_outputFileName.get()));
  RET_SM(
      ofile.is_open(),
      Status::NOT_FOUND,
      "Unable to open output file " << g_outputFileName.get());

  ofile << "/**\n * Header generated by bin2h from source:\n *     "
        << g_inputFileName.get() << "\n */\n";
  ofile << "#ifndef BIN2H_GENERATED_" << datanameCaps << "_H\n";
  ofile << "#define BIN2H_GENERATED_" << datanameCaps << "_H\n";
  ofile << "\nstatic const unsigned char g_" << g_outDataName.get()
        << "_data[] = {";

  u32 size = 0;
  for (std::string::const_iterator itr = inputContent.begin();
       itr != inputContent.end();
       ++itr) {
    if (size != 0) {
      ofile << ", ";
    }
    if (size % 16 == 0) {
      ofile << "\n  ";
    }
    ofile << "'\\x" << hex(*itr) << "'";
    ++size;
  }

  ofile << "\n};\n";
  ofile << "\nstatic const size_t g_" << g_outDataName.get()
        << "_sz = " << inputContent.size() << ";\n";
  ofile << "\nstatic const unsigned int g_" << g_outDataName.get()
        << "_signature = "
        << core::hash::CRC32(inputContent.begin(), inputContent.end()) << ";\n";
  ofile << "\n#endif\n";

  return Status::OK;
}

/**
 *
 */
const char hexCh(unsigned char ch) {
  ASSERT(ch >= 0 && ch <= 15);
  if (ch < 10) {
    return '0' + ch;
  } else {
    return 'a' + (ch - 10);
  }
}

/**
 *
 */
const char *hex(unsigned char ch) {
  static char buffer[3];
  unsigned char high = ch / 16;
  unsigned char low = ch % 16;
  buffer[0] = hexCh(high);
  buffer[1] = hexCh(low);
  buffer[2] = 0;
  return buffer;
}
