#pragma once

/**
 * @brief create_indexes Создает файл с индексами
 * @param [path] Путь к папке с тайлами (z/x/y.png)
 * @param [indexesFileName] Имя файла с индексами
 * @param [z_min] Уровень с которого начинается индексация
 * @param [z_max] Уровень до которого идет индексация
 * @param [x_min] Уровень до которого идет индексация
 * @param [x_max] Уровень до которого идет индексация */
void create_indexes(const char * path, const char * indexesFileName, int z_min, int z_max, int x_min, int x_max);
