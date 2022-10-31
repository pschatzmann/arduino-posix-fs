# Arduino Posix Filesystem

## Overview 
All regular C or C++ projects that use files are based on the [Posix File Operations](https://www.mkompf.com/cplus/posixlist.html). In Microcontrollers things are quite different and you use a SD library which has it's own API to store files on a SD drive. 

If you want to store larger amount of data on a Microcontroller you can also use the Program Memory (PROGMEM). 

The goals of this project is to provide __Posix File API__ support to Microcontrollers in order to 

- deplay and access files in PROGMEM (w/o separate file deployment step)
- access files SD drives (if not supported by default)

so that existing projects can be easily migrated to run on microcontrollers. 

## Documentation

Here is the [link to the class documentation](https://pschatzmann.github.io/arduino-posix-fs/docs/html/annotated.html).

### Implementation & Configuration 

On ESP32 processors this is implmented with the help of the [Virtual filesystem (VFS)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/vfs.html)

On other processors you can activate the implementation by setting the POSIX_C_METHOD_IMPLEMENTATION to 1 in __ConfigFS.h__

### Using PROGMEM Files

You can convert any file into c source code with the help of __xxd__. Don't forget to change the generated defintion by adding a const to the test string. This makes sure that it is stored in PROGMEM.

Then you can register the files with their corresponding name and size: Here is an [example sketch](examples/in-memory-fs/in-memory-fs.ino) that registers some files. You can read the files with the regualr C or C++ APIs: see [this example](examples/in-memory-read/memory-read.ino). 

### Using SD Files

To set up the Posix API you will define a global FileSystemSD object. Then you just need to set up the SD as always: Here is [an example](examples/sd/sd.ino).

### Logging

You can set up the logger by providing the log level and the logging output: 
```
  file_systems::FSLogger.begin(file_systems::FSDebug, Serial); 
```

## Installation in Arduino

You can download the library as zip and call include Library -> zip library. Or you can git clone this project into the Arduino libraries folder e.g. with

```
cd  ~/Documents/Arduino/libraries
git clone pschatzmann/arduino-posix-fs.git
```

I recommend to use git because you can easily update to the latest version just by executing the ```git pull``` command in the project folder.

