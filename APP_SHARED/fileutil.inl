#ifndef FISHY_FILEUTIL_INL
#define FISHY_FILEUTIL_INL

#include <CORE/BASE/serializer.h>
#include <CORE/BASE/serializer_streamsink.h>
#include <CORE/UTIL/FILES/proto_text.h>
#include <CORE/VFS/vfs_file.h>

namespace appshared {

/**
 *
 */
template < typename tType >
inline Status parseProtoFromFile(const vfs::Path &path, tType &proto) {
  std::string content;
  Status readStatus = parseFileToString(path, content);
  if (!readStatus) {
    return readStatus;
  }
  return Status(core::util::files::TextFormat::parse(proto, content));
}

/**
 *
 */
template < typename tType >
inline Status printProtoToFile(const vfs::Path &path, const tType &proto) {
  vfs::ofstream ofile(path);
  if (!ofile.is_open()) {
    return Status::NOT_FOUND;
  }

  std::string content;
  if (!core::util::files::TextFormat::format(content, proto)) {
    return Status::GENERIC_ERROR;
  }
  ofile.write(content.data(), content.size());
  return Status(!ofile.fail());
}

/**
 *
 */
template < typename tType >
inline Status serializeProtoFromFile(const vfs::Path &path, tType &proto) {
  vfs::ifstream ifile(path);
  if (!ifile.is_open()) {
    return Status::NOT_FOUND;
  }
  core::base::InStreamSink isink(ifile);
  return Status(proto.iserialize(isink));
}

/**
 *
 */
template < typename tType >
inline Status serializeProtoToFile(const vfs::Path &path, const tType &proto) {
  vfs::ofstream ofile(path);
  if (!ofile.is_open()) {
    return Status::NOT_FOUND;
  }
  core::base::OutStreamSink osink(ofile);
  return Status(proto.oserialize(osink));
}

/**
 *
 */
inline Status parseFileToString(const vfs::Path &path, std::string &content) {
  vfs::ifstream ifile(path);
  if (!ifile.is_open()) {
    return Status::NOT_FOUND;
  }
  const size_t fileLen = static_cast< size_t >(ifile.getFileLen());
  content.resize(fileLen, 0);
  ifile.read(&content[0], fileLen);
  return Status(ifile.gcount() == fileLen);
}

} // namespace appshared

#endif
