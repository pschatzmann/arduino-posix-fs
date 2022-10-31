#include "FileSystems.h"

#define PIN_SD_CARD_CS 13  
#define PIN_SD_CARD_MISO 2
#define PIN_SD_CARD_MOSI 15
#define PIN_SD_CARD_CLK  14

file_systems::FileSystemSD mysd("/sd", SD);

void setup() {
// setup logger
  Serial.begin(115200);
  file_systems::FSLogger.begin(file_systems::FSDebug, Serial); 

// setup SD
  //SPI.begin(PIN_SD_CARD_CLK, PIN_SD_CARD_MISO, PIN_SD_CARD_MOSI, PIN_SD_CARD_CS);
  SPI.begin();
  // "sd" is default mount point
  while(!SD.begin(PIN_SD_CARD_CS)){
    delay(500);
  }

// read directory
  DIR *dir;
  struct dirent *entry;

  if ((dir = opendir("/sd")) == NULL)
    Serial.println("opendir() error");
  else {
    Serial.println("contents of root:");
    while ((entry = readdir(dir)) != NULL)
      Serial.println(entry->d_name);
    closedir(dir);
  }
}

void loop(){}