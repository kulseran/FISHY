#ifndef FISHY_MEMORY_INL
#define FISHY_MEMORY_INL

#include <CORE/types.h>

namespace core {
namespace memory {

/**
 *
 */
inline bool isPow2(intptr_t in) {
  return ((in & (in - 1)) == 0);
}

/**
 *
 */
inline size_t nextPow2(intptr_t in) {
  if (in == 0) {
    return 1;
  }
  in--;
  for (size_t i = 1; i < sizeof(size_t) * 8; i <<= 1) {
    in = in | in >> i;
  }
  return in + 1;
}

/**
 *
 */
template < typename tType >
inline tType *construct(tType *pObj) {
  new (pObj) tType;
  return pObj;
}

/**
 *
 */
template < typename tType, typename tParamType >
inline tType *construct(tType *pObj, const tParamType &param) {
  new (pObj) tType(param);
  return pObj;
}

/**
 *
 */
template < typename tType >
inline tType *constructArray(tType *pObj, const size_t count) {
  for (size_t i = 0; i < count; ++i) {
    construct(pObj + i);
  }
  return pObj;
}

/**
 *
 */
template < typename tType >
inline void *destruct(tType *pObj) {
  pObj->~tType();
  return pObj;
}

/**
 *
 */
template < typename tType >
inline void *destructArray(tType *pObj, const size_t count) {
  for (size_t i = 0; i < count; ++i) {
    destruct(pObj + i);
  }
  return pObj;
}

/**
 *
 */
inline intptr_t alignPtr(intptr_t ptr, size_t alignment) {
  ASSERT(isPow2(alignment));
  const size_t alignMask = alignment - 1;
  return ((ptr & alignMask) ? ((ptr + alignMask) & ~alignMask) : (ptr));
}

/**
 *
 */
inline intptr_t alignPtrWithHeader(intptr_t ptr, size_t alignment) {
  u8 *u8ptr = (u8 *) ptr;

  u8 *rVal = (u8 *) alignPtr((intptr_t)(u8ptr + sizeof(intptr_t)), alignment);
  ASSERT((((intptr_t) rVal) & (alignment - 1)) == 0);

  intptr_t *tag = (intptr_t *) (rVal - sizeof(intptr_t));
  *tag = ptr;

  return (intptr_t) rVal;
}

/**
 *
 */
inline intptr_t getPtrFromAlignedHeader(intptr_t ptr) {
  u8 *u8ptr = (u8 *) ptr;
  intptr_t *tag = (intptr_t *) (u8ptr - sizeof(intptr_t));
  return (*tag);
}

/**
 *
 */
inline void *allocAligned(size_t size, size_t alignment) {
  const size_t actualSz = size + sizeof(intptr_t) + alignment * 2;
  return (void *) alignPtrWithHeader((intptr_t) new u8[actualSz], alignment);
}

/**
 *
 */
inline void freeAligned(void *pPtr) {
  if (pPtr == nullptr) {
    return;
  }

  u8 *actualPtr = (u8 *) getPtrFromAlignedHeader((intptr_t) pPtr);
  delete[] actualPtr;
}

/**
 * Deleter for use with std::shared_ptr when the input object is allocated with
 * {@link allocAligned}
 */
template < typename tType >
void shared_ptr_aligned_deleter(tType *pObject) {
  freeAligned(destruct(pObject));
}

} // namespace memory
} // namespace core

#endif
