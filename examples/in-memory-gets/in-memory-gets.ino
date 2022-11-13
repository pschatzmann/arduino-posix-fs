#include "FileSystems.h"

file_systems::FileSystemMemory fsm("/mem");
const char *data1 = "12345567890\n12345567890\n12345567890\n12345567890";

void setup() {
  Serial.begin(115200);
  file_systems::FSLogger.begin(file_systems::FSWarning, Serial);
  while(!Serial);

  // setup file
  fsm.add("/mem/test1", data1, strlen(data1) + 1);
  assert(fsm.size() == 1);

  int bufferLength = 255;
  char buffer[bufferLength]; /* not ISO 90 compatible */
  FILE *fp = fopen("/mem/test1", "r");

  while (fgets(buffer, bufferLength, fp)) {
    Serial.print(buffer);
  }

  fclose(fp);
}

void loop() {}