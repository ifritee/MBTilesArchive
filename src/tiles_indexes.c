#include "tiles_indexes.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "map/map.h"
#include "common/strings_routine.h"

void create_indexes(const char * path, const char * indexesFileName, int z_min, int z_max, int x_min, int x_max)
{
  uint32_t z, x, y, maxCount, fileLength;
  FILE * fileTile;
  unsigned char checksum[16];
  char checksumString[16 * 2 + 1];
  char fileName[1024];
  uint64_t id = 0;
  map_uint64_t tilesMap;
  map_init(&tilesMap);
  FILE * fileIndex = fopen(indexesFileName, "wb");
  if (fileIndex) {
    for(z = z_min; z <= z_max; ++z) { // Пробегаем по z
      maxCount = (1 << z); // Возведем 2 в степень z
      int xMaxCount = maxCount - 1;
      if (xMaxCount > x_max) {
        xMaxCount = x_max;
      }
      for (x = x_min; x <= xMaxCount; ++x) {
        for (y = 0; y < maxCount; ++y) {
          ++id;
          sprintf(fileName, "%s/%u/%u/%u.png", path, z, x, y);
          fileTile = fopen(fileName, "rb");
          if (fileTile) {
            fseek(fileTile, 0, SEEK_END);
            fileLength = ftell(fileTile);
            rewind(fileTile);
            md5File(fileTile, checksum);
            for (int i = 0; i < 16; ++i) {
              sprintf(&checksumString[i*2], "%02x", checksum[i]);
            }
            checksumString[16 * 2] = '\0';
            fclose(fileTile);
            uint64_t * contains = map_get(&tilesMap, checksumString);
            if (!contains) {
              uint64_t data = 0;
              data = (data | ((uint64_t)z << 50));
              data = (data | ((uint64_t)x << 30));
              data = (data | ((uint64_t)y << 10));
              map_set(&tilesMap, checksumString, data);
            } else {
#ifdef __x86_64__
              sprintf(fileName, "+%lu", *contains);
#else // 32-bits
              sprintf(fileName, "+%llu", *contains);
#endif
            }
#ifdef __x86_64__
            fprintf(fileIndex, "%lu|%u|%u|%u|%u|%s|%s\n", id, z, x, y, fileLength, checksumString, fileName);
#else // 32-bits
            fprintf(fileIndex, "%llu|%u|%u|%u|%u|%s|%s\n", id, z, x, y, fileLength, checksumString, fileName);
#endif
          }
        }
      }
    }
    fclose(fileIndex);
  }
  map_deinit(&tilesMap);
}
