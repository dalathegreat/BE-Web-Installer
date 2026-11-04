// Minimal <string.h> for the freestanding WebAssembly build; see libc.c.
#ifndef LFS_IMAGE_STRING_H
#define LFS_IMAGE_STRING_H
#include <stddef.h>
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* dest, int c, size_t n);
int memcmp(const void* a, const void* b, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strchr(const char* s, int c);
size_t strspn(const char* s, const char* accept);
size_t strcspn(const char* s, const char* reject);
#endif
