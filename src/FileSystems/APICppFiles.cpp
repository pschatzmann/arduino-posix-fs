#include "ConfigFS.h"
#ifdef FS_USE_F_INTERNAL

#include "stdlib.h"

// C++ file operations are mapped to _i methods with the help of defines
FILE *fopen_i(const char *path, const char *mode) {
  int file = open(path, 0);
  if (file < 0)
    return nullptr;
  FILE *fp = (FILE *)malloc(sizeof(FILE));
  fp->_file = file;
  return fp;
}

size_t fread_i(void *buffer, size_t size, size_t count, FILE *stream) {
  return read(stream->_file, buffer, size * count);
}

// Reads a single charac
int fgetc_i(FILE *stream) {
  char c;
  size_t len = fread_i(&c, 1, 1, stream);
  return len == 1 ? c : -1;
}

char *fgets_i(char *s, int n, FILE *f) {
  int count = 0;
  int c = -1;
  for (int j = 0; j < n - 1; j++) {
    c = fgetc_i(f);
    if (c < 0) {
      break;
    }
    count = j;
    s[count] = c;
    if (c == '\n') {
      break;
    }
  }
  s[count + 1] = 0;
  return c != -1 ? s : nullptr;
}

int fclose_i(FILE *fp) { 
  return close(fp->_file); 
}

int fseek_i(FILE *fp, long int offset, int whence) {
  return lseek(fp->_file, offset, whence);
}

#endif
