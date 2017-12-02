#include <APP_SHARED/app_main.h>
#include <APP_SHARED/fileutil.h>
#include <CORE/BASE/config.h>
#include <CORE/BASE/logging.h>
#include <CORE/UTIL/stringutil.h>
#include <CORE/UTIL/FILES/recordio.h>
#include <CORE/VFS/vfs.h>

#include <iostream>
#include <memory>

//----------------------------------------------------------------------------
// P R O T O T Y P E S
//----------------------------------------------------------------------------
FISHY_APPLICATION(ProtoTool);

struct eMode {
  enum type {
    UNKNOWN,
    PRINT,
    CONVERT_TO_BIN,
    CONVERT_TO_TEXT,
    CONVERT_TO_COLUMNS,
    COUNT
  };

  static const char *enumNames[COUNT];
};
const char *eMode::enumNames[eMode::COUNT] = {
  "UNKNOWN",
  "PRINT",
  "CONVERT_TO_BIN",
  "CONVERT_TO_TEXT",
  "CONVERT_TO_COLUMNS"
};

struct eInputType {
  enum type {
    UNKNOWN,
    TEXT_PROTO,
    BIN_PROTO,
    RECORD_PROTO,
    COUNT
  };

  static const char *enumNames[COUNT];
};
const char *eInputType::enumNames[eInputType::COUNT] = {
  "UNKNOWN",
  "TEXT_PROTO",
  "BIN_PROTO",
  "RECORD_PROTO",
};

//----------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//----------------------------------------------------------------------------
core::config::Flag<std::string> g_inputFile("infile", "Input filename", "");
core::config::Flag<std::string> g_outputFile("outfile", "Output filename", "");
core::config::Flag<std::string> g_protoType("proto", "Fully qualified proto name", "");
core::config::Flag<core::config::FlagEnum<eMode>>  g_mode("mode", "processing mode", core::config::FlagEnum<eMode>(eMode::UNKNOWN));
core::config::Flag<core::config::FlagEnum<eInputType>>  g_inputType("input_type", "Input file type", core::config::FlagEnum<eInputType>(eInputType::UNKNOWN));

class iResultProcessor {
  public:
    virtual int processResult(const core::util::DynamicProto &result) = 0;
};

class StdOutProcessor : public iResultProcessor {
  public:
    StdOutProcessor() : m_recordNum(0) { }
    int processResult(const core::util::DynamicProto &result) override {
      std::string str;
      core::util::files::TextFormat::format(str, result);
      std::cout << "\nRecord " << m_recordNum++ << "\n" << str << std::endl;
      return eExitCode::OK;
    }
  private:
    u32 m_recordNum;
};

class TextOutProcessor : public iResultProcessor {
  public:
    int processResult(const core::util::DynamicProto &result) override {
      if (!appshared::printProtoToFile(vfs::Path(g_outputFile.get()), result)) {
        Log(LL::Error) << "Unable to open output file: " << g_outputFile.get() << std::endl;
        return eExitCode::EXCEPTION;
      }
      return eExitCode::OK;
    }
};

class BinOutProcessor : public iResultProcessor {
  public:
    int processResult(const core::util::DynamicProto &result) override {
      if (!appshared::serializeProtoToFile(vfs::Path(g_outputFile.get()), result)) {
        Log(LL::Error) << "Unable to open output file: " << g_outputFile.get() << std::endl;
        return eExitCode::EXCEPTION;
      }
      return eExitCode::OK;
    }
};

class RecordOutProcessor : public iResultProcessor {
  public:
    int processResult(const core::util::DynamicProto &result) override {
      CHECK_NOT_IMPLEMENTED();
      return eExitCode::OK;
    }
};


/**
 *
 */
void ProtoTool::printHelp() {
  std::cout << "Usage:" << std::endl;
  std::cout << "    prototool --infile <i> --outfile <o> --proto <p> --mode <m> --input_type <t>" << std::endl;
}

/**
 *
 */
void ProtoTool::init(const char *arg0) {
  (void) arg0;
}

/**
 *
 */
int ProtoTool::main() {
  vfs::Mount("./", "./", std::ios::in | std::ios::out | std::ios::binary);
  g_mode.checkSet();
  g_protoType.checkSet();
  g_inputFile.checkSet();

  const core::util::ProtoDescriptor *pDescriptor = core::util::FindProtoByName(g_protoType.get());
  if (pDescriptor == nullptr) {
    Log(LL::Error) << "Unknown proto type: " << g_protoType.get() << std::endl;
    const std::vector< std::string > names = core::util::ListAllProtoNames();
    Log(LL::Info) << "Available types are: \n" << core::util::Joiner().on("\n").join(names.begin(), names.end()) << std::endl;
    return eExitCode::INVALID_FLAGS;
  }

  std::unique_ptr<iResultProcessor> pProcessor;
  switch (g_mode.get().m_value) {
    case eMode::PRINT:
      pProcessor = std::unique_ptr<iResultProcessor>(new StdOutProcessor());
      break;
    case eMode::CONVERT_TO_TEXT:
      CHECK_M(g_inputType.get().m_value != eInputType::RECORD_PROTO, "Can't convert records to textproto file.");
      pProcessor = std::unique_ptr<iResultProcessor>(new TextOutProcessor());
      break;
    case eMode::CONVERT_TO_BIN:
      switch (g_inputType.get().m_value) {
        case eInputType::RECORD_PROTO:
          pProcessor = std::unique_ptr<iResultProcessor>(new RecordOutProcessor());
          break;
        default:
          pProcessor = std::unique_ptr<iResultProcessor>(new BinOutProcessor());
          break;
      }
      break;
    default:
      CHECK_NOT_IMPLEMENTED();
  }

  switch (g_inputType.get().m_value) {
    case eInputType::TEXT_PROTO: {
      core::util::DynamicProto buffer(*pDescriptor);
      if (!appshared::parseProtoFromFile(vfs::Path(g_inputFile.get()), buffer)) {
        Log(LL::Error) << "Unable to open input." << std::endl;
        return eExitCode::BAD_FILE;
      }
      pProcessor->processResult(buffer);
      break;
    }
    case eInputType::BIN_PROTO: {
      core::util::DynamicProto buffer(*pDescriptor);
      if (!appshared::serializeProtoFromFile(vfs::Path(g_inputFile.get()), buffer)) {
        Log(LL::Error) << "Unable to open input." << std::endl;
        return eExitCode::BAD_FILE;
      }
      pProcessor->processResult(buffer);
      break;
    }
    case eInputType::RECORD_PROTO: {
      vfs::ifstream ifile(vfs::Path(g_inputFile.get()));
      if (!ifile.is_open()) {
        Log(LL::Error) << "Unable to open input." << std::endl;
        return eExitCode::BAD_FILE;
      }
      core::util::files::InRecordIOStream recordStream(ifile);

      u32 recordNum = 0;
      size_t recordSz = 0;
      std::string buffer;
      while (recordStream.sizeNextRecord(recordSz)) {
        buffer.reserve(std::max(buffer.size(), recordSz));
        core::memory::Blob bufferBlob(buffer);
        if (!recordStream.readNextRecord(bufferBlob)) {
          Log(LL::Warning) << "Unreadable record at: " << recordNum << std::endl;
          break;
        }
        core::base::ConstBlobSink sink(bufferBlob);
        core::util::DynamicProto buffer(*pDescriptor);
        if (!buffer.iserialize(sink)) {
          Log(LL::Warning) << "Unparseable record at: " << recordNum << std::endl;
        } else {
          pProcessor->processResult(buffer);
        }
        recordNum++;
      }
      break;
    }
    default:
      CHECK_NOT_IMPLEMENTED();
  }

  return eExitCode::OK;
}
