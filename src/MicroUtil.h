#ifndef __MICROUTIL_H__
#define __MICROUTIL_H__

#define Q(x) #x
#define QUOTE(x) Q(x)

extern constexpr unsigned int shash(const char* str, int h = 0) {
  return !str[h] ? 5381 : (shash(str, h + 1) * 33) ^ str[h];
}

#endif // __MICROUTIL_H__
