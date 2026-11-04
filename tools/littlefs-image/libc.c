// The few C library functions littlefs needs, for a freestanding WebAssembly
// build with no libc. Built with -fno-builtin, so the compiler does not turn
// these loops back into calls to themselves.

#include <string.h>

void* memcpy(void* dest, const void* src, size_t n) {
  unsigned char* d = dest;
  const unsigned char* s = src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

void* memset(void* dest, int c, size_t n) {
  unsigned char* d = dest;
  while (n--) {
    *d++ = (unsigned char)c;
  }
  return dest;
}

int memcmp(const void* a, const void* b, size_t n) {
  const unsigned char* x = a;
  const unsigned char* y = b;
  for (; n; n--, x++, y++) {
    if (*x != *y) {
      return *x - *y;
    }
  }
  return 0;
}

size_t strlen(const char* s) {
  size_t n = 0;
  while (s[n]) {
    n++;
  }
  return n;
}

char* strcpy(char* dest, const char* src) {
  char* d = dest;
  while ((*d++ = *src++)) {
  }
  return dest;
}

char* strchr(const char* s, int c) {
  for (;; s++) {
    if (*s == (char)c) {
      return (char*)s;
    }
    if (!*s) {
      return 0;
    }
  }
}

size_t strspn(const char* s, const char* accept) {
  size_t n = 0;
  while (s[n] && strchr(accept, s[n])) {
    n++;
  }
  return n;
}

size_t strcspn(const char* s, const char* reject) {
  size_t n = 0;
  while (s[n] && !strchr(reject, s[n])) {
    n++;
  }
  return n;
}
