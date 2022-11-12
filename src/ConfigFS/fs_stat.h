#pragma once

#ifndef S_IFREG
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFMT 00170000

struct stat {
  size_t st_size = 0;
  int st_mode = 0;
  int st_ino = 0;
};

#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)

#endif
