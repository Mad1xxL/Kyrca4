// auth.h — модуль проверки учётных данных.
// Пароль в открытом виде нигде не хранится. В файле data/auth.dat лежат:
// логин, случайная соль, число итераций и хеш PBKDF2-HMAC-SHA256(пароль, соль).
// При входе хеш вычисляется заново и сравнивается с сохранённым.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

const std::size_t LOGIN_SIZE = 32;
const std::size_t SALT_SIZE = 16;
const std::size_t PASSWORD_HASH_SIZE = 32;
const std::uint32_t PASSWORD_HASH_ITERATIONS = 20000;
const std::size_t PASSWORD_MIN_LENGTH = 8;

#pragma pack(push, 1)
struct CredentialRecord {
    char signature[4];                          // метка формата "AUTH"
    std::uint32_t version;                      // версия формата
    char login[LOGIN_SIZE];                     // логин (не секретен)
    std::uint8_t salt[SALT_SIZE];               // случайная соль
    std::uint32_t iterations;                   // число итераций PBKDF2
    std::uint8_t passwordHash[PASSWORD_HASH_SIZE];  // хеш пароля
};
#pragma pack(pop)

namespace auth {

bool credentialsExist();

// Проверки правил для логина и пароля. Пустая строка — значение верно.
std::string checkLogin(const std::string& login);
std::string checkPassword(const std::string& password);

// Сохраняет учётные данные: генерирует соль, вычисляет хеш, пишет файл.
void saveCredentials(const std::string& login, const std::string& password);

// true — логин и пароль верны. Бросает AuthError, если файл повреждён.
bool checkCredentials(const std::string& login, const std::string& password);

// Диалог первичной настройки при первом запуске.
void runFirstSetup();

// Диалог входа: не более cfg::MAX_LOGIN_ATTEMPTS попыток.
// Возвращает true при успешном входе.
bool runLogin();

}
