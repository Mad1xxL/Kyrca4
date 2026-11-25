// record_file.h — модуль чтения и записи бинарных файлов с записями.

#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

#include "achievement.h"

// Тип файла хранится в заголовке, чтобы не перепутать исходный и второй файл.
enum class FileKind : std::uint32_t {
    Source = 1,    // исходный файл достижений
    Verified = 2   // второй файл: подтверждённые достижения
};

#pragma pack(push, 1)
struct FileHeader {
    char signature[4];           // метка формата "SACT"
    std::uint32_t version;       // версия формата
    std::uint32_t recordSize;    // размер одной записи — для проверки формата
    std::uint32_t kind;          // тип файла (FileKind)
    std::uint32_t nextId;        // следующий свободный регистрационный номер
    std::uint32_t reserved;      // резерв на будущее
};
#pragma pack(pop)

const std::uint32_t FILE_FORMAT_VERSION = 1;

// Открытый файл записей. Файл открывается в конструкторе и закрывается
// в деструкторе, поэтому после выхода из функции он всегда закрыт.
class RecordFile {
public:
    enum class Mode { ReadOnly, ReadWrite };

    // Открывает файл и проверяет заголовок. Бросает FileOpenError,
    // FileReadError или FileFormatError.
    RecordFile(const std::string& path, FileKind expectedKind, Mode mode);

    std::size_t recordCount() const;   // количество записей (вместе с удалёнными)
    const std::string& path() const;

    Achievement read(std::size_t index);                  // прочитать запись по номеру
    void write(std::size_t index, const Achievement& record);  // перезаписать на месте
    void append(const Achievement& record);               // дописать в конец
    std::uint32_t takeNextId();                           // выдать новый рег. номер

private:
    std::streamoff positionOf(std::size_t index) const;
    void readHeader();
    void writeHeader();
    void requireWriteMode() const;

    std::string filePath;
    FileKind fileKind;
    Mode openMode;
    std::fstream stream;
    FileHeader header;
    std::size_t count;
};

// Записывает по пути path файл, содержащий только заголовок.
// Используется для временных файлов.
void writeHeaderOnlyFile(const std::string& path, FileKind kind);

// Заменяет файл targetPath файлом temporaryPath.
void replaceFile(const std::string& temporaryPath, const std::string& targetPath);

bool fileExists(const std::string& path);

// Удаляет файл, если он есть (ошибки игнорируются). Для временных файлов.
void removeFileQuietly(const std::string& path);

// Создаёт каталог data, если его нет.
void ensureDataDirectory();

// Название файла для сообщений пользователю.
std::string fileKindName(FileKind kind);
