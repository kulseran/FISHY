/**
 * Memory handling routines
 */
#ifndef FISHY_MEMORY_H
#define FISHY_MEMORY_H

#include <CORE/BASE/asserts.h>

#include <stdint.h>

namespace core {
namespace memory {

/**
 * @return true if the input pointer is aligned on a power of 2
 */
bool isPow2(intptr_t in);

/**
 * @return the next power of two above the input value
 */
size_t nextPow2(intptr_t in);

/**
 * @param ptr pointer to align
 * @param alignment byte count of alignment
 * @return aligned pointer
 */
intptr_t alignPtr(intptr_t ptr, size_t alignment);

/**
 * @return aligned pointer, with header indicating original pointer
 * @see alignPtr
 */
intptr_t alignPtrWithHeader(intptr_t ptr, size_t alignment);

/**
 * @return original pointer used with {@link alignPtrWithHeader}
 * @see alignPtrWithHeader
 */
intptr_t getPtrFromAlignedHeader(intptr_t ptr);

/**
 * @param size bytes of requested storage
 * @param alignment bytes count of alignment
 * @return an alligned pointer with atleast {@code size} storage
 * @see free_aligned
 */
void *allocAligned(size_t size, size_t alignment);

/**
 * @param pPtr pointer to free, must have been created with {@link
 * alloc_aligned}
 * @see alloc_aligned
 */
void freeAligned(void *pPtr);

/**
 * Call placement new on a pointer.
 *
 * @return the constructed pointer
 */
template < typename tType >
tType *construct(tType *pObj);

/**
 * Call placement new on a pointer, providing a single parameter
 *
 * @return the constructed pointer
 */
template < typename tType, typename tParamType >
tType *construct(tType *pObj, const tParamType &param);

/**
 * Call placement new on every element in the input array
 *
 * @return the begining of the constructed array
 */
template < typename tType >
tType *constructArray(tType *pObj, const size_t count);

/**
 * Call the typed destructor for a pointer
 *
 * @return the destructed pointer
 */
template < typename tType >
void *destruct(tType *pObj);

/**
 * Call the typed destructor for an input array
 *
 * @return the destructed pointer
 */
template < typename tType >
void *destructArray(tType *pObj, const size_t count);

/**
 * Deleter for use with std::shared_ptr when the input object is allocated with
 * {@link allocAligned}
 */
template < typename tType >
void shared_ptr_aligned_deleter(tType *pObject);

} // namespace memory
} // namespace core

#  include "memory.inl"

#endif
