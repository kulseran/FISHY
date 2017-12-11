/**
 * Generates a 32bit crc checksum for a given buffer
 */
#ifndef FISHY_CRC_H
#define FISHY_CRC_H

#include <CORE/types.h>

namespace core {
namespace hash {

/**
 * Create a CRC.
 * @param buffer bytes to hash
 * @param size of buffer
 */
Signature CRC32(const void *buffer, const u64 sz);

/**
 * Create a CRC. But treat all letters as capitals (A-Z)
 *
 * @param buffer bytes to hash
 * @param size of buffer
 */
Signature CiCRC32(const void *buffer, const u64 sz);

/**
 * Init a signature for use with appending {@link CRC32} or {@link CiCRC32}.
 * CRC must be finalized with {@link FinalizeCRC} to be valid.
 */
void StartCRC(Signature &sig);

/**
 * Append a single byte to a given CRC.
 * CRC should have been initalized with {@link StartCRC}
 */
Signature CRC32(const Signature start, const u8 byte);

/**
 * Append a single byte to a given CRC. Treating a letter as capital (A-Z)
 * CRC should have been initalized with {@link StartCRC}
 */
Signature CiCRC32(const Signature start, const u8 byte);

/**
 * Finalize a CRC sig created with {@link StartCRC}
 * and byte appending with {@link CRC32} or {@link CiCRC32}
 */
void FinalizeCRC(Signature &sig);

/**
 * Iterator wrapper for creating a CRC.
 */
template < typename tForwardIterator >
inline Signature
CRC32(const tForwardIterator &begin, const tForwardIterator &end) {
  Signature sig;
  StartCRC(sig);
  for (tForwardIterator itr = begin; itr != end; ++itr) {
    sig = CRC32(sig, static_cast< u8 >(*itr));
  }
  FinalizeCRC(sig);
  return sig;
}

/**
 * Iterator wrapper for creating a CiCRC
 */
template < typename tForwardIterator >
inline Signature
CiCRC32(const tForwardIterator &begin, const tForwardIterator &end) {
  Signature sig;
  StartCRC(sig);
  for (tForwardIterator itr = begin; itr != end; ++itr) {
    sig = CiCRC32(sig, *itr);
  }
  FinalizeCRC(sig);
  return sig;
}

} // namespace hash
} // namespace core

#endif
