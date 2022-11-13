#include "FileSystems.h"

file_systems::FileSystemMemory fsm("/mem");

const char *data1 = "12345567890123455678901234556789012345567890";

void setup() {
  Serial.begin(115200);
  file_systems::FSLogger.begin(file_systems::FSWarning, Serial); 
  while(!Serial);

  // setup files
  fsm.add("/mem/test1", data1, strlen(data1)+1);
  fsm.add("/mem/test2", data1, strlen(data1)+1);
  fsm.add("/mem/dir1/test3", data1, strlen(data1)+1);
  fsm.add("/mem/dir1/test4", data1, strlen(data1)+1);
  fsm.add("/mem/test5", data1, strlen(data1)+1);

  // read directory
  DIR *dir;
  char dir_path[80] = {0};
  struct dirent *entry;

  if ((dir = opendir("/mem")) == NULL)
    Serial.println("opendir() error");
  else {
    Serial.println("contents of root:");
    while ((entry = readdir(dir)) != NULL){
      Serial.print(entry->d_name);
      Serial.print(": ");
      // Read file
      strcpy(dir_path,"/mem");
      strcat(dir_path,"/");
      strcat(dir_path,entry->d_name);
      Serial.print(dir_path);
      Serial.print(": ");
      FILE *fp = fopen(dir_path, "rb");
      char buffer[1024];
      int len = fread(buffer, 1, 1024, fp);
      Serial.println(buffer);
      fclose(fp);
    }
    closedir(dir);
  }
}

void loop() {}