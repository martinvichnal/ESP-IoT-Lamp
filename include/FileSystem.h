#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Arduino.h>
#include "SPIFFS.h"

#ifdef __cplusplus
extern "C"{
#endif  /* __cplusplus */

void initSPIFFS();
String readFile(fs::FS &fs, const char *path);
void writeFile(fs::FS &fs, const char *path, const char *message);

// File paths for SPIFFS
extern char* ssidPath;
extern char* passPath;
extern char* ipPath;
extern char* gatewayPath;

#ifdef __cplusplus
}
#endif  /* __cplusplus */ 

#endif