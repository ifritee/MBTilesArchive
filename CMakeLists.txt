cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(MBTilesArchive LANGUAGES C)

set(SOURCE_BIN
  src/main.c
  src/mbtiles_archive.c
  src/tiles_indexes.c
  src/sqlite3/sqlite3.c
  src/md5/md5.c
  src/map/map.c
  src/common/strings_routine.c
)

set(HEADER_BIN
  src/mbtiles_archive.h
  src/tiles_indexes.h
  src/sqlite3/sqlite3.h
  src/md5/md5.h
  src/map/map.h
  src/common/strings_routine.h
)

add_executable(${PROJECT_NAME} ${SOURCE_BIN} ${HEADER_BIN})

target_compile_definitions(${PROJECT_NAME} PRIVATE
  -D_GNU_SOURCE
  -DSQLITE_THREADSAFE=2
  -D__NO_MD5__
  -DSQLITE_OMIT_LOAD_EXTENSION
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
  src/common
  src/sqlite3
  src/md5
  src/map
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  -static
  -static-libgcc
  -lpthread
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -std=c99
)

# Сборка на Elbrus-90 одной строкой
# gcc common/Constants.c common/UTimer.c web_server/FSReader.c web_server/WebServer.c main.c -Icommon -Iweb_server -lpthread -levent -lzip -D_GNU_SOURCE -o NGeoServer
