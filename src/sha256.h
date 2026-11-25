// sha256.h — хеш-функция SHA-256 (FIPS 180-4), HMAC-SHA256 (RFC 2104)
// и PBKDF2-HMAC-SHA256 (RFC 8018).
// Используется для хеширования пароля через PBKDF2 (модуль auth),
// позже — для хеширования записи перед электронной подписью.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

const std::size_t SHA256_DIGEST_SIZE = 32;  // длина хеша в байтах (256 бит)
const std::size_t SHA256_BLOCK_SIZE = 64;   // длина блока в байтах (512 бит)

using Digest = std::array<std::uint8_t, SHA256_DIGEST_SIZE>;

class Sha256 {
public:
    Sha256();

    // Добавить очередную порцию данных (можно вызывать несколько раз).
    void update(const std::uint8_t* data, std::size_t length);
    void update(const std::string& text);

    // Завершить вычисление и получить хеш.
    Digest finish();

    // Хеш строки за один вызов.
    static Digest hash(const std::string& text);

private:
    void processBlock(const std::uint8_t* block);

    std::uint32_t hashValues[8];               // H0..H7 из стандарта
    std::uint8_t buffer[SHA256_BLOCK_SIZE];    // недозаполненный блок
    std::size_t bufferLength;
    std::size_t totalBytes;                    // сколько байт обработано всего
};

// HMAC-SHA256: имитовставка на основе хеш-функции.
Digest hmacSha256(const std::vector<std::uint8_t>& key, const std::vector<std::uint8_t>& message);

// PBKDF2-HMAC-SHA256: многократное хеширование пароля с солью.
// Возвращает 32 байта (один блок). Большое число итераций замедляет
// подбор пароля перебором.
Digest pbkdf2Sha256(const std::string& password, const std::vector<std::uint8_t>& salt,
                    std::uint32_t iterations);

// Запись байтов в шестнадцатеричном виде, например "ba7816bf...".
std::string toHex(const std::uint8_t* data, std::size_t length);
std::string toHex(const Digest& digest);
