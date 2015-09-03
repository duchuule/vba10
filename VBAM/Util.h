#ifndef UTIL_H
#define UTIL_H

#include "System.h"

enum IMAGE_TYPE {
  IMAGE_UNKNOWN = -1,
  IMAGE_GBA     = 0,
  IMAGE_GB      = 1
};

// save game
typedef struct {
  void *address;
  int size;
} variable_desc;

bool utilWritePNGFile(const char *, int, int, u8 *);
bool utilWriteBMPFile(const char *, int, int, u8 *);
void utilApplyIPS(const char *ips, u8 **rom, int *size);
bool utilIsGBAImage(const char *);
bool utilIsGBImage(const char *);
bool utilIsGzipFile(const char *);
void utilStripDoubleExtension(const char *, char *);

void utilPutDword(u8 *, u32);
void utilPutWord(u8 *, u16);
void utilWriteData(gzFile, variable_desc *);
void utilReadData(gzFile, variable_desc *);
void utilReadDataSkip(gzFile, variable_desc *);
int utilReadInt(gzFile);
void utilWriteInt(gzFile, int);
gzFile utilGzOpen(const char *file, const char *mode);
gzFile utilMemGzOpen(char *memory, int available, const char *mode);
int utilGzWrite(gzFile file, const voidp buffer, unsigned int len);
int utilGzRead(gzFile file, voidp buffer, unsigned int len);
int utilGzClose(gzFile file);
z_off_t utilGzSeek(gzFile file, z_off_t offset, int whence);
long utilGzMemTell(gzFile file);
void utilGBAFindSave(const int);
void utilUpdateSystemColorMaps(bool lcd = false);
bool utilFileExists( const char *filename );

enum RTCSTATE { IDLE, COMMAND, DATA, READDATA };

typedef struct {
  u8 byte0;
  u8 byte1;
  u8 byte2;
  u8 command;
  int dataLen;
  int bits;
  RTCSTATE state;
  u8 data[12];
  // reserved variables for future
  u8 reserved[12];
  bool reserved2;
  u32 reserved3;
} RTCCLOCKDATA;

#endif // UTIL_H

