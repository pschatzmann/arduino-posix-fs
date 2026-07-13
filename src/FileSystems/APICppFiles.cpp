#include "ConfigFS.h"
#ifdef FS_USE_F_INTERNAL

#include "stdlib.h"

// Minimal file handle used behind the FILE* returned by fopen_i(): FILE
// itself is an opaque, implementation-defined type on the host libc (its
// real layout must not be assumed), so we never touch its fields -- we
// allocate our own struct and only ever hand it back through the fopen_i()
// family, cast to FILE* purely to satisfy the fopen()-compatible signature.
struct fs_internal_file_t {
  int fd;
};

// C++ file operations are mapped to _i methods with the help of defines
FILE *fopen_i(const char *path, const char *mode) {
  int file = open(path, 0);
  if (file < 0)
    return nullptr;
  fs_internal_file_t *fp = (fs_internal_file_t *)malloc(sizeof(fs_internal_file_t));
  fp->fd = file;
  return (FILE *)fp;
}

size_t fread_i(void *buffer, size_t size, size_t count, FILE *stream) {
  return read(((fs_internal_file_t *)stream)->fd, buffer, size * count);
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
  int result = close(((fs_internal_file_t *)fp)->fd);
  free(fp);
  return result;
}

int fseek_i(FILE *fp, long int offset, int whence) {
  return lseek(((fs_internal_file_t *)fp)->fd, offset, whence);
}

#endif
