#include "FileSystems.h"

file_systems::FileSystemMemory fsm("/mem");

const char *data1 = "12345567890123455678901234556789012345567890";

void setup() {
  Serial.begin(115200);
  file_systems::FSLogger.begin(file_systems::FSInfo, Serial); 

  // setup files
  fsm.add("/mem/test1", data1, strlen(data1));
  fsm.add("/mem/test2", data1, strlen(data1));
  fsm.add("/mem/dir1/test3", data1, strlen(data1));
  fsm.add("/mem/dir1/test4", data1, strlen(data1));
  fsm.add("/mem/test5", data1, strlen(data1));

  // read directory
  DIR *dir;
  struct dirent *entry;

  if ((dir = opendir("/mem")) == NULL)
    Serial.println("opendir() error");
  else {
    Serial.println("contents of root:");
    while ((entry = readdir(dir)) != NULL)
      Serial.println(entry->d_name);
    closedir(dir);
  }
}

void loop() {}