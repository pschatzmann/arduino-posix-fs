#include "FileSystems.h"

file_systems::FileSystemMemory fsm("/mem");
const char *data1 = "12345567890123455678901234556789012345567890";

void setup() {
  Serial.begin(115200);
  file_systems::FSLogger.begin(file_systems::FSInfo, Serial); 
  while(!Serial);

  // setup file
  fsm.add("/mem/test1", data1, strlen(data1)+1);
  assert(fsm.size()==1);

  // read file
  int fd = open("/mem/test1", O_RDONLY);
  if (fd < 0) {
    Serial.println("open failed");
    return;
  }
  char buffer[1024];
  int len = read(fd, buffer, 1024);
  if (len != strlen(data1)+1) {
    Serial.print("fread failed with ");
    Serial.println(len);
    return;
  }
  Serial.println(buffer);

}

void loop() {}