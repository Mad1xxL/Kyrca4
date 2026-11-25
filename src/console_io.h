// console_io.h — ввод с клавиатуры с проверкой и вспомогательный вывод.
#pragma once

#include <cstddef>
#include <string>

namespace con {

// Настройка консоли. В Windows включается кодовая страница 1251,
// в macOS и Linux ничего делать не нужно (там UTF-8).
void init();

// Читает строку целиком и убирает пробелы по краям.
std::string readLine(const std::string& prompt);

// Читает целое число из диапазона [minValue, maxValue], при ошибке повторяет запрос.
int readInt(const std::string& prompt, int minValue, int maxValue,
            const std::string& errorMessage = "");

// Читает пароль, не показывая символы на экране.
std::string readPassword(const std::string& prompt);

// Вопрос «д/н». true — да.
bool confirm(const std::string& question);

// true, если пользователь ввёл «0» (отмена).
bool isCancel(const std::string& text);

// Длина строки в символах (в UTF-8 русская буква занимает 2 байта).
std::size_t textLength(const std::string& text);

void printRule(std::size_t width = 100, char symbol = '-');

std::string trim(const std::string& text);

}  
