// errors.h — классы исключений программы.
// Все ожидаемые ошибки наследуются от AppError. Меню перехватывает их
// и сообщает, какая операция не выполнена и почему
#pragma once

#include <stdexcept>
#include <string>

class AppError : public std::runtime_error {
public:
    explicit AppError(const std::string& message) : std::runtime_error(message) {}
};

// Файл не найден или нет доступа.
class FileOpenError : public AppError {
public:
    explicit FileOpenError(const std::string& message) : AppError(message) {}
};

// Ошибка при чтении файла.
class FileReadError : public AppError {
public:
    explicit FileReadError(const std::string& message) : AppError(message) {}
};

// Ошибка при записи в файл.
class FileWriteError : public AppError {
public:
    explicit FileWriteError(const std::string& message) : AppError(message) {}
};

// Файл не соответствует формату (чужой или повреждённый).
class FileFormatError : public AppError {
public:
    explicit FileFormatError(const std::string& message) : AppError(message) {}
};

// Пользователь отменил операцию (ввёл 0).
class OperationCancelled : public AppError {
public:
    OperationCancelled() : AppError("Операция отменена пользователем") {}
};

// Поток ввода закрыт (Ctrl+D / Ctrl+Z).
class InputClosed : public AppError {
public:
    InputClosed() : AppError("Поток ввода закрыт") {}
};

// Ошибка учётных данных. Текст не раскрывает хеш и служебные данные.
class AuthError : public AppError {
public:
    explicit AuthError(const std::string& message) : AppError(message) {}
};
