// auth.cpp — первичная настройка и проверка учётных данных.
#include "auth.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "app_config.h"
#include "console_io.h"
#include "errors.h"
#include "record_file.h"
#include "sha256.h"

namespace auth {

static const char AUTH_SIGNATURE[4] = {'A', 'U', 'T', 'H'};
static const std::uint32_t AUTH_FORMAT_VERSION = 1;

bool credentialsExist() {
    return fileExists(cfg::AUTH_FILE);
}

std::string checkLogin(const std::string& login) {
    if (login.size() < 3 || login.size() >= LOGIN_SIZE) {
        return "логин должен содержать от 3 до " + std::to_string(LOGIN_SIZE - 1) + " символов";
    }
    for (const char symbol : login) {
        const unsigned char code = static_cast<unsigned char>(symbol);
        if (code >= 128 || (!std::isalnum(code) && symbol != '_')) {
            return "допускаются только латинские буквы, цифры и «_»";
        }
    }
    return "";
}

std::string checkPassword(const std::string& password) {
    if (password.size() < PASSWORD_MIN_LENGTH) {
        return "пароль должен содержать не менее " + std::to_string(PASSWORD_MIN_LENGTH) +
               " символов";
    }
    bool hasLetter = false;
    bool hasDigit = false;
    for (const char symbol : password) {
        const unsigned char code = static_cast<unsigned char>(symbol);
        if (std::isdigit(code)) {
            hasDigit = true;
        } else if (std::isalpha(code) || code >= 128) {
            hasLetter = true;
        }
    }
    if (!hasLetter || !hasDigit) {
        return "пароль должен содержать хотя бы одну букву и одну цифру";
    }
    return "";
}

// Случайная соль из аппаратного генератора случайных чисел.
static std::vector<std::uint8_t> generateSalt() {
    std::random_device randomDevice;
    std::uniform_int_distribution<int> byteDistribution(0, 255);
    std::vector<std::uint8_t> salt(SALT_SIZE);
    for (std::size_t index = 0; index < SALT_SIZE; ++index) {
        salt[index] = static_cast<std::uint8_t>(byteDistribution(randomDevice));
    }
    return salt;
}

// Сравнение хешей за одинаковое время, независимо от того, в каком байте
// первое различие. Так по времени ответа нельзя угадать часть хеша.
static bool hashesEqual(const std::uint8_t* first, const std::uint8_t* second, std::size_t length) {
    std::uint8_t difference = 0;
    for (std::size_t index = 0; index < length; ++index) {
        difference = static_cast<std::uint8_t>(difference | (first[index] ^ second[index]));
    }
    return difference == 0;
}

void saveCredentials(const std::string& login, const std::string& password) {
    CredentialRecord credentials{};
    std::memcpy(credentials.signature, AUTH_SIGNATURE, sizeof(AUTH_SIGNATURE));
    credentials.version = AUTH_FORMAT_VERSION;
    const std::size_t loginLength = login.size() < LOGIN_SIZE ? login.size() : LOGIN_SIZE - 1;
    std::memcpy(credentials.login, login.data(), loginLength);  // остальное — нули
    credentials.iterations = PASSWORD_HASH_ITERATIONS;

    const std::vector<std::uint8_t> salt = generateSalt();
    std::memcpy(credentials.salt, salt.data(), SALT_SIZE);
    const Digest passwordHash = pbkdf2Sha256(password, salt, credentials.iterations);
    std::memcpy(credentials.passwordHash, passwordHash.data(), PASSWORD_HASH_SIZE);

    // Пишем во временный файл и только потом заменяем основной.
    const std::string temporaryPath = cfg::AUTH_FILE + ".tmp";
    {
        std::ofstream output(temporaryPath, std::ios::binary | std::ios::trunc);
        if (!output.is_open()) {
            throw FileWriteError("не удалось создать файл учётных данных");
        }
        output.write(reinterpret_cast<const char*>(&credentials), sizeof(credentials));
        output.flush();
        if (!output) {
            throw FileWriteError("ошибка записи файла учётных данных");
        }
    }
    replaceFile(temporaryPath, cfg::AUTH_FILE);
}

static CredentialRecord loadCredentials() {
    std::ifstream input(cfg::AUTH_FILE, std::ios::binary);
    if (!input.is_open()) {
        throw AuthError("не удалось открыть файл учётных данных");
    }
    CredentialRecord credentials{};
    input.read(reinterpret_cast<char*>(&credentials), sizeof(credentials));
    // Файл должен быть ровно одного размера с структурой.
    const bool readWholeRecord = static_cast<bool>(input);
    input.peek();
    const bool hasExtraBytes = !input.eof();
    if (!readWholeRecord || hasExtraBytes ||
        std::memcmp(credentials.signature, AUTH_SIGNATURE, sizeof(AUTH_SIGNATURE)) != 0 ||
        credentials.version != AUTH_FORMAT_VERSION || credentials.iterations == 0 ||
        std::memchr(credentials.login, '\0', LOGIN_SIZE) == nullptr) {
        // Подробности (какое поле не совпало) намеренно не сообщаются.
        throw AuthError("файл учётных данных повреждён. Восстановите его из резервной копии");
    }
    return credentials;
}

bool checkCredentials(const std::string& login, const std::string& password) {
    const CredentialRecord credentials = loadCredentials();
    const std::vector<std::uint8_t> salt(credentials.salt, credentials.salt + SALT_SIZE);
    const Digest enteredHash = pbkdf2Sha256(password, salt, credentials.iterations);

    const bool loginMatches = (login == std::string(credentials.login));
    const bool passwordMatches =
        hashesEqual(enteredHash.data(), credentials.passwordHash, PASSWORD_HASH_SIZE);
    return loginMatches && passwordMatches;
}

void runFirstSetup() {
    std::cout << "\nУчётные данные не настроены. Выполняется первичная настройка.\n"
              << "Логин: 3–31 символ (латинские буквы, цифры, «_»).\n"
              << "Пароль: не менее " << PASSWORD_MIN_LENGTH
              << " символов, хотя бы одна буква и одна цифра.\n";

    std::string login;
    while (true) {
        login = con::readLine("Придумайте логин: ");
        const std::string error = checkLogin(login);
        if (error.empty()) {
            break;
        }
        std::cout << "Ошибка: " << error << ".\n";
    }

    std::string password;
    while (true) {
        password = con::readPassword("Придумайте пароль: ");
        const std::string error = checkPassword(password);
        if (!error.empty()) {
            std::cout << "Ошибка: " << error << ".\n";
            continue;
        }
        const std::string repeatedPassword = con::readPassword("Повторите пароль: ");
        if (repeatedPassword != password) {
            std::cout << "Ошибка: пароли не совпадают. Повторите ввод.\n";
            continue;
        }
        break;
    }

    saveCredentials(login, password);
    std::cout << "Учётные данные сохранены. Теперь выполните вход.\n";
}

bool runLogin() {
    std::cout << "\n--- Вход в систему ---\n";
    for (int attempt = 1; attempt <= cfg::MAX_LOGIN_ATTEMPTS; ++attempt) {
        const std::string login = con::readLine("Логин: ");
        const std::string password = con::readPassword("Пароль: ");
        if (checkCredentials(login, password)) {
            std::cout << "Вход выполнен. Добро пожаловать, " << login << "!\n";
            return true;
        }
        const int attemptsLeft = cfg::MAX_LOGIN_ATTEMPTS - attempt;
        std::cout << "Неверный логин или пароль.";
        if (attemptsLeft > 0) {
            std::cout << " Осталось попыток: " << attemptsLeft << ".";
        }
        std::cout << "\n";
    }
    std::cout << "Превышено число попыток входа. Доступ к записям закрыт.\n";
    return false;
}

}
