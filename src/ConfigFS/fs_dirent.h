#pragma once

#define MAXNAMLEN 1024
#define DT_REG 8
#define DT_DIR 4
struct dirent {
  char d_name[MAXNAMLEN];
  int d_type;
};


struct DIR_ {};
typedef struct DIR_ DIR;

#ifdef __cplusplus
extern "C" {
#endif
DIR *opendir(const char *name);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
int unlink(const char *pathname);

#ifdef __cplusplus
}
#endif
