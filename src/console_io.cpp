// console_io.cpp — реализация ввода-вывода консоли.
#include "console_io.h"

#include <cctype>
#include <cstdio>
#include <iostream>

#include "errors.h"

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace con {

void init() {
#ifdef _WIN32
    // Строки собраны в кодировке windows-1251 (см. Makefile).
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif
}

std::string trim(const std::string& text) {
    const std::string spaces = " \t\r\n";
    const std::size_t firstPosition = text.find_first_not_of(spaces);
    if (firstPosition == std::string::npos) {
        return "";
    }
    const std::size_t lastPosition = text.find_last_not_of(spaces);
    return text.substr(firstPosition, lastPosition - firstPosition + 1);
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw InputClosed();
    }
    return trim(line);
}

// Строка → целое число. Только цифры (и минус в начале), не больше 9 цифр.
static bool convertToInt(const std::string& text, int& result) {
    std::size_t position = 0;
    bool isNegative = false;
    if (!text.empty() && text[0] == '-') {
        isNegative = true;
        position = 1;
    }
    const std::size_t digitCount = text.size() - position;
    if (digitCount == 0 || digitCount > 9) {
        return false;
    }
    int value = 0;
    for (; position < text.size(); ++position) {
        const unsigned char symbol = static_cast<unsigned char>(text[position]);
        if (!std::isdigit(symbol)) {
            return false;
        }
        value = value * 10 + (symbol - '0');
    }
    result = isNegative ? -value : value;
    return true;
}

int readInt(const std::string& prompt, int minValue, int maxValue,
            const std::string& errorMessage) {
    while (true) {
        const std::string text = readLine(prompt);
        int value = 0;
        if (convertToInt(text, value) && value >= minValue && value <= maxValue) {
            return value;
        }
        if (!errorMessage.empty()) {
            std::cout << errorMessage << "\n";
        } else {
            std::cout << "Ошибка: ожидается целое число от " << minValue << " до "
                      << maxValue << ". Повторите ввод.\n";
        }
    }
}

std::string readPassword(const std::string& prompt) {
#ifdef _WIN32
    if (!_isatty(_fileno(stdin))) {
        return readLine(prompt);  // ввод из файла, а не с клавиатуры
    }
    std::cout << prompt;
    std::string password;
    while (true) {
        const int keyCode = _getch();
        if (keyCode == '\r' || keyCode == '\n') {
            break;
        }
        if (keyCode == 0 || keyCode == 0xE0) {  // стрелки, F1... — пропускаем
            _getch();
            continue;
        }
        if (keyCode == '\b') {  // Backspace стирает последний символ
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
            continue;
        }
        if (keyCode == 3) {  // Ctrl+C
            throw InputClosed();
        }
        password.push_back(static_cast<char>(keyCode));
        std::cout << '*';
    }
    std::cout << "\n";
    return password;
#else
    if (!isatty(STDIN_FILENO)) {
        return readLine(prompt);  // ввод из файла, а не с клавиатуры
    }
    // Отключаем эхо терминала на время ввода пароля.
    termios oldSettings{};
    tcgetattr(STDIN_FILENO, &oldSettings);
    termios hiddenSettings = oldSettings;
    hiddenSettings.c_lflag &= ~static_cast<tcflag_t>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &hiddenSettings);

    std::cout << prompt;
    std::cout.flush();
    std::string password;
    const bool isRead = static_cast<bool>(std::getline(std::cin, password));

    tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);  // эхо возвращаем всегда
    std::cout << "\n";
    if (!isRead) {
        throw InputClosed();
    }
    return password;
#endif
}

bool confirm(const std::string& question) {
    while (true) {
        const std::string answer = readLine(question + " (д/н): ");
        if (answer == "д" || answer == "Д" || answer == "да") {
            return true;
        }
        if (answer == "н" || answer == "Н" || answer == "нет" || answer == "0") {
            return false;
        }
        std::cout << "Ошибка: введите «д» (да) или «н» (нет).\n";
    }
}

bool isCancel(const std::string& text) {
    return text == "0";
}

std::size_t textLength(const std::string& text) {
#ifdef _WIN32
    return text.size();  // windows-1251: один байт на символ
#else
    // UTF-8 (macOS, Linux): русская буква занимает 2 байта, второй байт
    // имеет вид 10xxxxxx — такие байты не считаем.
    std::size_t symbolCount = 0;
    for (const char symbol : text) {
        const bool isContinuationByte = (static_cast<unsigned char>(symbol) & 0xC0) == 0x80;
        if (!isContinuationByte) {
            ++symbolCount;
        }
    }
    return symbolCount;
#endif
}

void printRule(std::size_t width, char symbol) {
    std::cout << std::string(width, symbol) << "\n";
}

}
