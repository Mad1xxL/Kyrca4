// sha256.cpp — реализация SHA-256, HMAC-SHA256 и PBKDF2-HMAC-SHA256.
// Обозначения функций (Ch, Maj, Σ0, Σ1, σ0, σ1) взяты из FIPS 180-4.
#include "sha256.h"

#include <cstring>

// 64 константы раундов: первые 32 бита дробных частей кубических корней
// первых 64 простых чисел.
static const std::uint32_t ROUND_CONSTANTS[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
    0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
    0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
    0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
    0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
    0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
    0xc67178f2};

// Начальные значения H0..H7: дробные части квадратных корней первых 8 простых.
static const std::uint32_t INITIAL_HASH_VALUES[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

// Циклический сдвиг 32-битного слова вправо на shift бит.
static std::uint32_t rotateRight(std::uint32_t value, int shift) {
    return (value >> shift) | (value << (32 - shift));
}

// Ch: для каждого бита selector выбирает бит из ifOne (если 1) или ifZero (если 0).
static std::uint32_t choose(std::uint32_t selector, std::uint32_t ifOne, std::uint32_t ifZero) {
    return (selector & ifOne) ^ (~selector & ifZero);
}

// Maj: каждый бит результата равен большинству из трёх соответствующих битов.
static std::uint32_t majority(std::uint32_t first, std::uint32_t second, std::uint32_t third) {
    return (first & second) ^ (first & third) ^ (second & third);
}

static std::uint32_t bigSigma0(std::uint32_t value) {
    return rotateRight(value, 2) ^ rotateRight(value, 13) ^ rotateRight(value, 22);
}

static std::uint32_t bigSigma1(std::uint32_t value) {
    return rotateRight(value, 6) ^ rotateRight(value, 11) ^ rotateRight(value, 25);
}

static std::uint32_t smallSigma0(std::uint32_t value) {
    return rotateRight(value, 7) ^ rotateRight(value, 18) ^ (value >> 3);
}

static std::uint32_t smallSigma1(std::uint32_t value) {
    return rotateRight(value, 17) ^ rotateRight(value, 19) ^ (value >> 10);
}

// Класс Sha256

Sha256::Sha256() : bufferLength(0), totalBytes(0) {
    for (int index = 0; index < 8; ++index) {
        hashValues[index] = INITIAL_HASH_VALUES[index];
    }
    std::memset(buffer, 0, sizeof(buffer));
}

void Sha256::processBlock(const std::uint8_t* block) {
    // 1. Расписание сообщения: 16 слов из блока + 48 вычисленных слов.
    std::uint32_t schedule[64];
    for (int wordIndex = 0; wordIndex < 16; ++wordIndex) {
        schedule[wordIndex] = (static_cast<std::uint32_t>(block[wordIndex * 4]) << 24) |
                              (static_cast<std::uint32_t>(block[wordIndex * 4 + 1]) << 16) |
                              (static_cast<std::uint32_t>(block[wordIndex * 4 + 2]) << 8) |
                              (static_cast<std::uint32_t>(block[wordIndex * 4 + 3]));
    }
    for (int wordIndex = 16; wordIndex < 64; ++wordIndex) {
        schedule[wordIndex] = smallSigma1(schedule[wordIndex - 2]) + schedule[wordIndex - 7] +
                              smallSigma0(schedule[wordIndex - 15]) + schedule[wordIndex - 16];
    }

    // 2. Рабочие переменные. В стандарте они обозначены буквами a..h.
    std::uint32_t varA = hashValues[0];
    std::uint32_t varB = hashValues[1];
    std::uint32_t varC = hashValues[2];
    std::uint32_t varD = hashValues[3];
    std::uint32_t varE = hashValues[4];
    std::uint32_t varF = hashValues[5];
    std::uint32_t varG = hashValues[6];
    std::uint32_t varH = hashValues[7];

    // 3. 64 раунда сжатия.
    for (int round = 0; round < 64; ++round) {
        const std::uint32_t sum1 = varH + bigSigma1(varE) + choose(varE, varF, varG) +
                                   ROUND_CONSTANTS[round] + schedule[round];
        const std::uint32_t sum2 = bigSigma0(varA) + majority(varA, varB, varC);
        varH = varG;
        varG = varF;
        varF = varE;
        varE = varD + sum1;
        varD = varC;
        varC = varB;
        varB = varA;
        varA = sum1 + sum2;
    }

    // 4. Прибавляем результат к промежуточному хешу.
    hashValues[0] += varA;
    hashValues[1] += varB;
    hashValues[2] += varC;
    hashValues[3] += varD;
    hashValues[4] += varE;
    hashValues[5] += varF;
    hashValues[6] += varG;
    hashValues[7] += varH;
}

void Sha256::update(const std::uint8_t* data, std::size_t length) {
    for (std::size_t position = 0; position < length; ++position) {
        buffer[bufferLength] = data[position];
        ++bufferLength;
        if (bufferLength == SHA256_BLOCK_SIZE) {
            processBlock(buffer);
            bufferLength = 0;
        }
    }
    totalBytes += length;
}

void Sha256::update(const std::string& text) {
    update(reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
}

Digest Sha256::finish() {
    // Длина сообщения в битах записывается в конец как 64-битное число
    // (старшая и младшая 32-битные половины).
    const std::uint32_t bitLengthHigh = static_cast<std::uint32_t>(totalBytes >> 29);
    const std::uint32_t bitLengthLow = static_cast<std::uint32_t>(totalBytes << 3);

    // Дополнение: байт 0x80, затем нули, пока до конца блока не останется 8 байт.
    const std::uint8_t paddingStart = 0x80;
    const std::uint8_t zeroByte = 0x00;
    update(&paddingStart, 1);
    while (bufferLength != SHA256_BLOCK_SIZE - 8) {
        update(&zeroByte, 1);
    }

    std::uint8_t lengthBytes[8];
    for (int index = 0; index < 4; ++index) {
        lengthBytes[index] = static_cast<std::uint8_t>(bitLengthHigh >> (24 - 8 * index));
        lengthBytes[index + 4] = static_cast<std::uint8_t>(bitLengthLow >> (24 - 8 * index));
    }
    update(lengthBytes, 8);  // блок заполнен и обработан внутри update

    Digest digest{};
    for (int wordIndex = 0; wordIndex < 8; ++wordIndex) {
        for (int byteIndex = 0; byteIndex < 4; ++byteIndex) {
            digest[wordIndex * 4 + byteIndex] =
                static_cast<std::uint8_t>(hashValues[wordIndex] >> (24 - 8 * byteIndex));
        }
    }
    return digest;
}

Digest Sha256::hash(const std::string& text) {
    Sha256 hasher;
    hasher.update(text);
    return hasher.finish();
}

// HMAC и PBKDF2

Digest hmacSha256(const std::vector<std::uint8_t>& key, const std::vector<std::uint8_t>& message) {
    // Ключ длиннее блока сначала хешируется, короче — дополняется нулями.
    std::uint8_t blockKey[SHA256_BLOCK_SIZE] = {0};
    if (key.size() > SHA256_BLOCK_SIZE) {
        Sha256 keyHasher;
        keyHasher.update(key.data(), key.size());
        const Digest keyDigest = keyHasher.finish();
        std::memcpy(blockKey, keyDigest.data(), keyDigest.size());
    } else if (!key.empty()) {
        std::memcpy(blockKey, key.data(), key.size());
    }

    std::uint8_t innerPad[SHA256_BLOCK_SIZE];
    std::uint8_t outerPad[SHA256_BLOCK_SIZE];
    for (std::size_t index = 0; index < SHA256_BLOCK_SIZE; ++index) {
        innerPad[index] = static_cast<std::uint8_t>(blockKey[index] ^ 0x36);
        outerPad[index] = static_cast<std::uint8_t>(blockKey[index] ^ 0x5c);
    }

    // HMAC = H( (K xor opad) || H( (K xor ipad) || message ) )
    Sha256 innerHasher;
    innerHasher.update(innerPad, SHA256_BLOCK_SIZE);
    if (!message.empty()) {
        innerHasher.update(message.data(), message.size());
    }
    const Digest innerDigest = innerHasher.finish();

    Sha256 outerHasher;
    outerHasher.update(outerPad, SHA256_BLOCK_SIZE);
    outerHasher.update(innerDigest.data(), innerDigest.size());
    return outerHasher.finish();
}

Digest pbkdf2Sha256(const std::string& password, const std::vector<std::uint8_t>& salt,
                    std::uint32_t iterations) {
    const std::vector<std::uint8_t> passwordBytes(password.begin(), password.end());

    // U1 = HMAC(пароль, соль || номер блока 1 в виде 4 байт)
    std::vector<std::uint8_t> firstMessage = salt;
    firstMessage.push_back(0);
    firstMessage.push_back(0);
    firstMessage.push_back(0);
    firstMessage.push_back(1);
    Digest currentBlock = hmacSha256(passwordBytes, firstMessage);
    Digest result = currentBlock;

    // U2..Uc: каждый следующий — HMAC от предыдущего; результат — XOR всех U.
    for (std::uint32_t iteration = 2; iteration <= iterations; ++iteration) {
        const std::vector<std::uint8_t> previousBlock(currentBlock.begin(), currentBlock.end());
        currentBlock = hmacSha256(passwordBytes, previousBlock);
        for (std::size_t index = 0; index < result.size(); ++index) {
            result[index] = static_cast<std::uint8_t>(result[index] ^ currentBlock[index]);
        }
    }
    return result;
}

std::string toHex(const std::uint8_t* data, std::size_t length) {
    const char* hexDigits = "0123456789abcdef";
    std::string text;
    for (std::size_t index = 0; index < length; ++index) {
        text.push_back(hexDigits[data[index] >> 4]);
        text.push_back(hexDigits[data[index] & 0x0F]);
    }
    return text;
}

std::string toHex(const Digest& digest) {
    return toHex(digest.data(), digest.size());
}
