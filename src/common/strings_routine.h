#pragma once

typedef struct { int count; char ** strings; } StringArray;

/**
 * @brief split Разделение строки по ключу
 * @param [str] Исходная строка
 * @param [sep] Ключ
 * @return указатель на массив строк (!!! Не забудь освободить память !!!) */
StringArray split(const char *str, char sep);

/**
 * @brief freeStrings Очистка массива строк
 * @param [data] массив строк */
void freeStrings(StringArray * data);

/**
 * @brief append_string Добавляет к старой строке новую
 * @param [oldString] Старая строка
 * @param [newString] Новая строка
 * @return указатель на новую строку */
char * append_string(const char * oldString, char * newString);
