#pragma once

#include <stdint.h>

typedef void * DB_DESC; ///< @brief Дескриптор работы с БД

#define MBA_OK 0
#define MBA_FAILED 1
#define MBA_WARNING 2

/**
 * @brief openDatabase Открывает базу данных для дальнейшей работы с ней
 * @param [name] Имя файла БД
 * @return NULL - Ошибка, > 0 указатель на БД */
DB_DESC openDatabase(const char * name);
/**
 * @brief fillDatabase Заполняет новую БД
 * @param [db] Указатель на БД
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t fillDatabase(DB_DESC db);
/**
 * @brief freeDatabase Освобождает БД
 * @param [db] Дескриптор БД
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t freeDatabase(DB_DESC db);
/**
 * @brief export Экспортирует файл в БД
 * @param [db] БД SQLite
 * @param [buffer] Буфер с считанным файлом тайла
 * @param [length] Размер буфера данных
 * @param [z] Уровень тайла
 * @param [x] X координата тайла
 * @param [y] y координата тайла
 * @param [md5Text] текст суммы md5
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t export(DB_DESC db, int8_t * buffer, int32_t length, int32_t z, int32_t x, int32_t y, const char * md5Text);
/**
 * @brief export_ref Записывает ссылку на другой файл
 * @param [db] БД SQLite
 * @param [z] Уровень тайла
 * @param [x] X координата тайла
 * @param [y] y координата тайла
 * @param [zRef] Ссылка на Уровень тайла предыдущей записи
 * @param [xRef] Ссылка на X координату тайла предыдущей записи
 * @param [yRef] Ссылка на Y координату тайла предыдущей записи
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t export_ref(DB_DESC db, int32_t z, int32_t x, int32_t y, int32_t zRef, int32_t xRef, int32_t yRef);
/**
 * @brief import Импортирует считанные из БД файлы в буфер
 * @param [db] БД SQLite
 * @param [buffer] Буфер для картинки с тайлом
 * @param [length] Размер файла
 * @param [z] Уровень тайла
 * @param [x] X координата тайла
 * @param [y] y координата тайла
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t import(DB_DESC db, char ** buffer, int * length, int32_t z, int32_t x, int32_t y);
/**
 * @brief transaction Начало транзакции БД
 * @param [db] БД SQLite
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t transaction(DB_DESC db);
/**
 * @brief commit Конец транзакции БД
 * @param [db] БД SQLite
 * @return MBA_OK - Все хорошо, MBA_FAILED - Все плохо */
uint32_t commit(DB_DESC db);
