#include "mbtiles_archive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3/sqlite3.h"
#include "common/strings_routine.h"

static int sqliteCallback(void * runTime, int argc, char ** argv, char ** columnName )
{
  int i;
  char * currentString = "", * oldString = "";
  StringArray * strings = (StringArray *)runTime;
  ++(strings->count);
  char buffer[128];
  for (i = 0; i < argc; ++i) {
    sprintf(buffer, "%s|%s\n", columnName[i], argv[i]);
    oldString = currentString;
    if (strlen(currentString) > 0) { free(currentString); }
    currentString = append_string(oldString, buffer);
  }
  char ** oldStrings = strings->strings;
  strings->strings = malloc(strings->count * sizeof(char *));
  if (strings->count > 1) {
    for(i = 0; i < strings->count; ++i) {
      strings->strings[i] = oldStrings[i];
    }
    free(oldStrings);
  }
  strings->strings[strings->count - 1] = currentString;
  return 0;
}

int queryErrorOut(sqlite3 * db, const char * query)
{
  fprintf(stderr, "Error: SQLITE3 query %s: %s\n", query, sqlite3_errmsg(db));
  return MBA_FAILED;
}

int getID(char * text)
{
  int id = -1;
  StringArray array = split(text, '|');
  if (array.count == 2 && strcmp("id", array.strings[0]) == 0) {
    size_t length = strlen(array.strings[1]);
    array.strings[1][length - 1] = '\0';
    id = atoi(array.strings[1]);
  }
  freeStrings(&array);
  return id;
}

//-------------------------------------------------------------------------------------------------------

DB_DESC openDatabase(const char *name)
{
  sqlite3 *db;
  if(sqlite3_open(name, &db) != SQLITE_OK) {
    fprintf(stderr, "Error: SQLITE3 open: %s\n", sqlite3_errmsg(db));
    return NULL;
  }
  sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
  sqlite3_exec((sqlite3 *)db, "PRAGMA synchronous=0;", NULL, NULL, NULL);
  sqlite3_exec((sqlite3 *)db, "PRAGMA locking_mode=EXCLUSIVE;", NULL, NULL, NULL);
  sqlite3_exec((sqlite3 *)db, "PRAGMA journal_mode=OFF;", NULL, NULL, NULL);
  sqlite3_exec((sqlite3 *)db, "PRAGMA cache_size=16000000;", NULL, NULL, NULL);
  return (DB_DESC)db;
}

uint32_t fillDatabase(DB_DESC db)
{
  uint8_t isCorrect = 1;
  /// @brief Набор запросов на создание таблиц
  const char * createTables [] = {
      "CREATE TABLE POSITION ( z INT NOT NULL, x INT NOT NULL, y INT NOT NULL, id INT);"
    , "CREATE TABLE TILES ( id INTEGER PRIMARY KEY, file BLOB );"
#if !defined(__NO_MD5__)
    , "CREATE TABLE TILES_CHECKSUM ( md5 TEXT, id INT );"
#endif // __NO_MD5__
  };
  uint32_t i = 0;
  for (i = 0; i < sizeof(createTables) / sizeof(const char *); ++i) {
    const char * query = createTables[i];
    if (sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK) {
      fprintf(stderr, "Error: SQLITE3 create table: %s\n", sqlite3_errmsg(db));
      isCorrect = 0;
    }
  }
  if (isCorrect == 0) {
    return MBA_FAILED;
  }
  return MBA_OK;
}

uint32_t freeDatabase(DB_DESC db)
{
  if (sqlite3_close((sqlite3 *)db) != SQLITE_OK) {
    fprintf(stderr, "Error: SQLITE3 close: %s\n", sqlite3_errmsg(db));
    return MBA_FAILED;
  }
  return MBA_OK;
}

uint32_t export(DB_DESC db, int8_t * buffer, int32_t length, int32_t z, int32_t x, int32_t y, const char * md5Text)
{
  char query[1024];
  char * errors = 0;
  int id = -1;
  // 1. Ищем в БД md5 сумму и x, y, z
  int countMD5 = 0, countXYZ = 0;
  StringArray strings;
  strings.count = 0;
#if !defined(__NO_MD5__)
  sprintf(query, "SELECT id FROM TILES_CHECKSUM WHERE md5=\"%s\";", md5Text);
  if ( sqlite3_exec((sqlite3 *)db, query, sqliteCallback, &strings, &errors) != SQLITE_OK) {
    freeStrings(&strings);
    return queryErrorOut(db, query);
  }
  countMD5 = strings.count;
  if (countMD5 > 0) { id = getID(strings.strings[0]); }
  freeStrings(&strings);
#endif // __NO_MD5__
  sprintf(query, "SELECT id FROM POSITION WHERE z=%d AND x=%d AND y=%d;", z, x, y);
  if ( sqlite3_exec((sqlite3 *)db, query, sqliteCallback, &strings, &errors) != SQLITE_OK) {
    freeStrings(&strings);
    return queryErrorOut(db, query);
  }
  countXYZ = strings.count;
  if (countXYZ > 0) {
    int readID = getID(strings.strings[0]);
    if (readID >= 0) { id = readID; }
  }
  if (countMD5 > 0) { // 1.1 Если есть сумма
    if (countXYZ > 0) { // 1.1.1 Если совпадают с текущими {xyz}, то ошибка - такой файл уже существует
      freeStrings(&strings);
      return MBA_OK;
    } else  { // 1.1.2 Если не совпадают с текущими {xyz}, то добавляем запись в таблицу POSITION с считанным id
      if (id >= 0) {
        sprintf(query, "INSERT INTO POSITION (z, x, y, id) VALUES (%d, %d, %d, %d);", z, x, y, id);
        if ( sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK) { return queryErrorOut(db, query); }
      }
    }
  } else  { // 1.2 Если такой суммы нет
    if (countXYZ > 0) { // 1.2.1 Если совпадают с текущими {xyz}, то обновляем файл и MD5
      if (id >= 0) {
        sqlite3_stmt * stmt = NULL;
        sprintf(query, "UPDATE TILES SET file = ? WHERE id = id;");
        if (sqlite3_prepare((sqlite3 *)db, query, -1, &stmt, NULL) == SQLITE_OK) {
          if (sqlite3_bind_blob(stmt, 1, buffer, length, SQLITE_STATIC) == SQLITE_OK) {
            if (sqlite3_step(stmt) != SQLITE_DONE) {
              freeStrings(&strings);
              fprintf(stderr, "Error: sqlite3_step %s: %s\n", query, sqlite3_errmsg(db));
              return MBA_FAILED;
            }
            sqlite3_finalize(stmt);
          } else {
            freeStrings(&strings);
            fprintf(stderr, "Error: sqlite3_bind_blob %s: %s\n", query, sqlite3_errmsg(db));
            return MBA_FAILED;
          }
        } else {
          freeStrings(&strings);
          fprintf(stderr, "Error: sqlite3_bind_blob %s: %s\n", query, sqlite3_errmsg(db));
          return MBA_FAILED;
        }
#if !defined(__NO_MD5__)
        sprintf(query, "UPDATE TILES_CHECKSUM SET md5 = \"%s\" WHERE id = id;", md5Text);
        if ( sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK)  { return queryErrorOut(db, query); }
#endif // __NO_MD5__
      }
    } else { // 1.2.2 Если не совпадают с текущими {xyz}, то добавляем файл и параметры во все таблицы
      sqlite3_stmt * stmt = NULL;
      sprintf(query, "INSERT INTO TILES (file) VALUES (?);");
      if (sqlite3_prepare((sqlite3 *)db, query, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_bind_blob(stmt, 1, buffer, length, SQLITE_STATIC) == SQLITE_OK) {
          if (sqlite3_step(stmt) != SQLITE_DONE) {
            freeStrings(&strings);
            fprintf(stderr, "Error: sqlite3_step %s: %s\n", query, sqlite3_errmsg(db));
            return MBA_FAILED;
          }
          sqlite3_finalize(stmt);
        } else {
          freeStrings(&strings);
          fprintf(stderr, "Error: sqlite3_bind_blob %s: %s\n", query, sqlite3_errmsg(db));
          return MBA_FAILED;
        }
      } else {
        freeStrings(&strings);
        fprintf(stderr, "Error: sqlite3_bind_blob %s: %s\n", query, sqlite3_errmsg(db));
        return MBA_FAILED;
      }
      freeStrings(&strings);
      sprintf(query, "SELECT rowid FROM TILES ORDER BY rowid DESC limit 1;");
      if ( sqlite3_exec((sqlite3 *)db, query, sqliteCallback, &strings, &errors) != SQLITE_OK) {
        freeStrings(&strings);
        fprintf(stderr, "Error: SQLITE3 query %s: %s\n", query, sqlite3_errmsg(db));
        return MBA_FAILED;
      }
      if (strings.count != 1) {
        freeStrings(&strings);
        fprintf(stderr, "Error: No current ID");
        return MBA_FAILED;
      }
      id = getID(strings.strings[0]);
      if (id >= 0) {
        sprintf(query, "INSERT INTO POSITION (z, x, y, id) VALUES (%d, %d, %d, %d);", z, x, y, id);
        if ( sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK)  {
          freeStrings(&strings);
          return queryErrorOut(db, query);
        }
#if !defined(__NO_MD5__)
        sprintf(query, "INSERT INTO TILES_CHECKSUM (md5, id) VALUES (\"%s\", %d);", md5Text, id);
        if ( sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK) {
          freeStrings(&strings);
          return queryErrorOut(db, query);
        }
#endif // __NO_MD5__
      }
    }
  }

  return MBA_OK;
}

uint32_t export_ref(DB_DESC db, int32_t z, int32_t x, int32_t y, int32_t zRef, int32_t xRef, int32_t yRef)
{
  char query[1024];
  char * errors = 0;
  int id = -1;
  StringArray strings;
  strings.count = 0;
  sprintf(query, "SELECT id FROM POSITION WHERE z=%d AND x=%d AND y=%d;", zRef, xRef, yRef);
  if ( sqlite3_exec((sqlite3 *)db, query, sqliteCallback, &strings, &errors) != SQLITE_OK) {
    freeStrings(&strings);
    return queryErrorOut(db, query);
  }
  if (strings.count == 0) {
    fprintf(stderr, "Error: No current reference ID");
    freeStrings(&strings);
    return MBA_FAILED;
  }
  id = getID(strings.strings[0]);
  if (id >= 0) {
    sprintf(query, "INSERT INTO POSITION (z, x, y, id) VALUES (%d, %d, %d, %d);", z, x, y, id);
    if ( sqlite3_exec((sqlite3 *)db, query, NULL, NULL, NULL) != SQLITE_OK)  {
      freeStrings(&strings);
      return queryErrorOut(db, query);
    }
  }
  freeStrings(&strings);
  return MBA_OK;
}

uint32_t import(DB_DESC db, char ** buffer, int * length, int z, int x, int y)
{
  StringArray strings;
  strings.count = 0;
  char query[1024];
  char * errors = 0;
  sprintf(query, "SELECT id FROM POSITION WHERE z=%d AND x=%d AND y=%d;", z, x, y);
  if ( sqlite3_exec((sqlite3 *)db, query, sqliteCallback, &strings, &errors) != SQLITE_OK) { return queryErrorOut(db, query); }
  if (strings.count > 0) {
    int id = getID(strings.strings[0]);
    if (id >= 0) { // Если ID существует
      sqlite3_stmt *stmt;
      sprintf(query, "SELECT file FROM TILES WHERE id=%d;", id);
      if (sqlite3_prepare ((sqlite3 *)db, query, -1, &stmt, 0) == SQLITE_OK) { // Если БД готова к считыванию
        int ctotal = sqlite3_column_count (stmt);  // сtotal хранит количество столбцов в БД
        while(1) {
          int res = sqlite3_step(stmt);
          if ( res == SQLITE_ROW ) { // Если удачно считана новая строка БД
            (*length) = sqlite3_column_bytes (stmt, 0);
            char * temp = (char *) sqlite3_column_blob (stmt, 0);
            *buffer = (char *)malloc(*length);
            memcpy(*buffer, temp, *length);
          } else if (res == SQLITE_DONE || res == SQLITE_ERROR) {   // Если считана вся БД или произошла ошибка,
            sqlite3_finalize(stmt);
            break;  // то остановить считывание данных из БД
          }
        }
      }
    }
  }
  return MBA_OK;
}

uint32_t transaction(DB_DESC db)
{
  if (sqlite3_exec((sqlite3 *)db, "BEGIN TRANSACTION;", NULL, NULL, NULL) != SQLITE_OK) {
    return queryErrorOut(db, "BEGIN TRANSACTION");
  }
  return MBA_OK;
}

uint32_t commit(DB_DESC db)
{
  if (sqlite3_exec((sqlite3 *)db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK) {
    return queryErrorOut(db, "COMMIT");
  }
  return MBA_OK;
}
