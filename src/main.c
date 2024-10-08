#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <argp.h>
#include <limits.h>

#include "md5.h"
#include "mbtiles_archive.h"
#include "tiles_indexes.h"
#include "common/strings_routine.h"

#define MBTA_MAJOR 0
#define MBTA_MINOR 1
#define PATH_LENGTH 2048
#define INDEXFILE_LENGTH 256
#define STRINGBUFFER_LENGTH 1024

int main(int argc, char ** argv)
{
  int32_t fileLength = 0, readLength = 0;
  int8_t * buffer = NULL;
  unsigned char checksum[16];
  char checksumString[16 * 2 + 1] = "ebc56fb93494dbe272833caf75fcb142";
  char stringBuffer[STRINGBUFFER_LENGTH];
  char pathToTiles[PATH_LENGTH] = "?";
  char indexFileName[INDEXFILE_LENGTH] = "mbt.index";
  char dbFileName[INDEXFILE_LENGTH] = "tiles.db";
  int zMin, zMax, xMin, xMax;
  int8_t isCreateIndexOnly = 0; ///< @brief Флаг создания только индексов

  //----- 01. Считываем параметры командной строки: ------------------------------------
  /// -p[--path] PATH - Путь к папке с тайлами
  /// -i[--index] INDEX - Файл с индексами тайлов
  /// -d[--database] DATABASE - Имя БД
  /// -ci[--create_index] {z_min, z_max, x_min, x_max} - Создать индексный файл выбранными параметрами (* - до последнего элемента)
  /// -h[--help] - справка
  /// -v[--version] - версия
  const char * help = "-p[--path] PATH - Path to tiles directory\n"
                      "-i[--index] INDEX - File to save indexes\n"
                      "-d[--database] DATABASE - Database file name\n"
                      "-ci[--create_index] {Zmin,Zmax,Xmin,Xmax} Only create index file by Z coord min,max, X coord min,max -. * - MAX [0,9,0,*] - all X to 9 z"
                      "-h[--help] - this help\n"
                      "-v[--version] - version\n"
      ;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--path") == 0 ) {
      // Если количества данных не хватает
      if(argc == i + 1) { errx(EXIT_FAILURE, "No data for tiles path {-p or --path} %s\n", help);}
      if (strlen(argv[i + 1]) >= PATH_LENGTH) { errx(EXIT_FAILURE, "Name of config file is biggest\n"); }
      strncpy(pathToTiles, argv[++i], PATH_LENGTH);
    }
    else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--index") == 0 ) {
      // Если количества данных не хватает
      if(argc == i + 1) { errx(EXIT_FAILURE, "No data for index name {-i or --index} %s\n", help);}
      if (strlen(argv[i + 1]) >= INDEXFILE_LENGTH) {errx(EXIT_FAILURE, "Name of index file is biggest\n");}
      strncpy(indexFileName, argv[++i], INDEXFILE_LENGTH);
    }
    else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--database") == 0 ) {
      // Если количества данных не хватает
      if(argc == i + 1) { errx(EXIT_FAILURE, "No data for database file name {-d or --database} %s\n", help);}
      if (strlen(argv[i + 1]) >= INDEXFILE_LENGTH) {errx(EXIT_FAILURE, "Name of database file is biggest\n");}
      strncpy(dbFileName, argv[++i], INDEXFILE_LENGTH);
    } else if (strcmp(argv[i], "-ci") == 0 || strcmp(argv[i], "--create_index") == 0) {
      // Если количества данных не хватает
      if(argc == i + 1) { errx(EXIT_FAILURE, "No data for create index file {-ci or --create_index} %s\n", help);}
      StringArray array = split(argv[i + 1], ',');
      if (array.count != 4) { errx(EXIT_FAILURE, "No current data for create index file {-ci or --create_index} %s\n", help); }
      zMin = atoi(array.strings[0]);
      zMax = atoi(array.strings[1]);
      xMin = atoi(array.strings[2]);
      xMax = atoi(array.strings[3]);
      if (strcmp(array.strings[1], "*") == 0) { zMax = 19; }
      if (strcmp(array.strings[3], "*") == 0) { xMax = INT32_MAX; }
      freeStrings(&array);
      isCreateIndexOnly = 1;
    }
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ) {
      printf("%s\n", help);
      return EXIT_SUCCESS;
    }
    else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0 ) {
      printf("version: %d.%d\n", MBTA_MAJOR, MBTA_MINOR);
      return EXIT_SUCCESS;
    }
  }
  if (pathToTiles[0] == '?') {
    errx(EXIT_FAILURE, "No path to tiles! Read help:\n%s\n", help);
  }

  //----- 02. Работа основная ------------------------------------
  if (isCreateIndexOnly == 1) {
    create_indexes(pathToTiles, indexFileName, zMin, zMax, xMin, xMax);
    return 0;
  }
  DB_DESC db = openDatabase(dbFileName);
  if (db) {
    fillDatabase(db);

    FILE * fileIndex = fopen(indexFileName, "r");
    uint32_t z, x, y, zRef, xRef, yRef, count = 0;
    uint64_t id, idRef;
    StringArray strings;
    strings.count = 0;
    if (fileIndex) {
      transaction(db);
      while(fgets(stringBuffer, STRINGBUFFER_LENGTH, fileIndex) != NULL) {
        strings = split(stringBuffer, '|');
        if (strings.count == 7) {
          id = atoll(strings.strings[0]);
          z = atoi(strings.strings[1]);
          x = atoi(strings.strings[2]);
          y = atoi(strings.strings[3]);
          fileLength = atoi(strings.strings[4]);
          int index = strlen(strings.strings[6]);
          strings.strings[6][index -1] = '\0';
          if (strings.strings[6][0] == '+') { // Это ссылка
            idRef = atoll(&strings.strings[6][1]);
            zRef = (uint32_t)((idRef >> 50) & 0xFF);
            xRef = (uint32_t)((idRef >> 30) & 0xFFFFF);
            yRef = (uint32_t)((idRef >> 10) & 0xFFFFF);
            if (export_ref(db, z, x, y, zRef, xRef, yRef) != MBA_OK) {
              errx(EXIT_FAILURE, "Error: export file crashed!\n"); // exit внутри
            }
          } else { // это основное данное
            FILE * fileTile = fopen(strings.strings[6], "rb");
            if (fileTile) {
              buffer = (int8_t *)malloc(fileLength + 1);
              readLength = fread(buffer, 1, fileLength, fileTile);
              if (readLength <= 0) {
                free(buffer);
                fclose(fileTile);
                err(EXIT_FAILURE, "Error: read tile file: %d\n", ferror(fileTile)); // exit внутри
              }
              buffer[fileLength] = '\0';
              if (readLength != fileLength) {
                free(buffer);
                fclose(fileTile);
                err(EXIT_FAILURE, "Error: read length not equal tile length: %d\n", ferror(fileTile)); // exit внутри
              }
              if (export(db, buffer, fileLength, z, x, y, checksumString) != MBA_OK) {
                free(buffer);
                fclose(fileTile);
                err(EXIT_FAILURE, "Error: export file crashed!\n"); // exit внутри
              }
              free(buffer);
              fclose(fileTile);
            }
          }
          if (count += fileLength > 16000000) {
            commit(db);
            transaction(db);
            count = 0;
          }
        }
        freeStrings(&strings);
      }
      commit(db);
      fclose(fileIndex);
    }
    freeDatabase(db);
  }
  return 0;
}
