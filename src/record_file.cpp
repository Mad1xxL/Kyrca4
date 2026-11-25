// record_file.cpp — чтение и запись бинарных файлов с записями.
#include "record_file.h"

#include <cstring>
#include <filesystem>

#include "app_config.h"
#include "errors.h"

namespace fs = std::filesystem;

static const char FORMAT_SIGNATURE[4] = {'S', 'A', 'C', 'T'};

// Вспомогательные функции 

bool fileExists(const std::string& path) {
    std::error_code errorCode;
    return fs::exists(path, errorCode);
}

void removeFileQuietly(const std::string& path) {
    std::error_code errorCode;
    fs::remove(path, errorCode);
}

void ensureDataDirectory() {
    std::error_code errorCode;
    fs::create_directories(cfg::DATA_DIR, errorCode);
    if (errorCode) {
        throw FileWriteError("не удалось создать каталог «" + cfg::DATA_DIR + "»");
    }
}

std::string fileKindName(FileKind kind) {
    return kind == FileKind::Source ? "исходный файл достижений"
                                    : "файл подтверждённых достижений";
}

static FileHeader makeHeader(FileKind kind) {
    FileHeader header{};
    std::memcpy(header.signature, FORMAT_SIGNATURE, sizeof(FORMAT_SIGNATURE));
    header.version = FILE_FORMAT_VERSION;
    header.recordSize = sizeof(Achievement);
    header.kind = static_cast<std::uint32_t>(kind);
    header.nextId = 1;
    header.reserved = 0;
    return header;
}

void writeHeaderOnlyFile(const std::string& path, FileKind kind) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        throw FileWriteError("не удалось создать файл «" + path + "»");
    }
    const FileHeader header = makeHeader(kind);
    output.write(reinterpret_cast<const char*>(&header), sizeof(header));
    output.flush();
    if (!output) {
        throw FileWriteError("ошибка записи в файл «" + path + "»");
    }
}

void replaceFile(const std::string& temporaryPath, const std::string& targetPath) {
    std::error_code errorCode;
    fs::rename(temporaryPath, targetPath, errorCode);
    if (!errorCode) {
        return;
    }
    // Некоторые компиляторы под Windows не заменяют существующий файл при
    // rename. Тогда удаляем старый и повторяем переименование.
    std::error_code removeError;
    fs::remove(targetPath, removeError);
    fs::rename(temporaryPath, targetPath, errorCode);
    if (errorCode) {
        throw FileWriteError("не удалось заменить файл «" + targetPath +
                             "». Новые данные сохранены во временном файле «" +
                             temporaryPath + "»");
    }
}

// Класс RecordFile

RecordFile::RecordFile(const std::string& path, FileKind expectedKind, Mode mode)
    : filePath(path), fileKind(expectedKind), openMode(mode), header{}, count(0) {
    if (!fileExists(path)) {
        throw FileOpenError("файл «" + path + "» не найден");
    }
    std::ios::openmode flags = std::ios::binary | std::ios::in;
    if (mode == Mode::ReadWrite) {
        flags |= std::ios::out;  // in | out — запись без очистки файла
    }
    stream.open(path, flags);
    if (!stream.is_open()) {
        throw FileOpenError("нет доступа к файлу «" + path +
                            "» (файл занят другой программой или недостаточно прав)");
    }
    readHeader();
}

void RecordFile::readHeader() {
    std::error_code errorCode;
    const std::uintmax_t fileSize = fs::file_size(filePath, errorCode);
    if (errorCode) {
        throw FileReadError("не удалось определить размер файла «" + filePath + "»");
    }
    if (fileSize < sizeof(FileHeader)) {
        throw FileFormatError("файл «" + filePath + "» повреждён: отсутствует заголовок");
    }

    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!stream) {
        throw FileReadError("не удалось прочитать заголовок файла «" + filePath + "»");
    }

    if (std::memcmp(header.signature, FORMAT_SIGNATURE, sizeof(FORMAT_SIGNATURE)) != 0) {
        throw FileFormatError("файл «" + filePath + "» не является файлом программы");
    }
    if (header.version != FILE_FORMAT_VERSION) {
        throw FileFormatError("файл «" + filePath + "» имеет неподдерживаемую версию формата");
    }
    if (header.recordSize != sizeof(Achievement)) {
        throw FileFormatError("в файле «" + filePath + "» другой размер записи");
    }
    if (header.kind != static_cast<std::uint32_t>(fileKind)) {
        throw FileFormatError("файл «" + filePath + "» не является: " + fileKindName(fileKind));
    }

    const std::uintmax_t dataSize = fileSize - sizeof(FileHeader);
    if (dataSize % sizeof(Achievement) != 0) {
        throw FileFormatError("файл «" + filePath +
                              "» повреждён: размер не кратен размеру записи");
    }
    count = static_cast<std::size_t>(dataSize / sizeof(Achievement));
}

void RecordFile::writeHeader() {
    requireWriteMode();
    stream.clear();
    stream.seekp(0);
    stream.write(reinterpret_cast<const char*>(&header), sizeof(header));
    stream.flush();
    if (!stream) {
        throw FileWriteError("не удалось обновить заголовок файла «" + filePath + "»");
    }
}

void RecordFile::requireWriteMode() const {
    if (openMode != Mode::ReadWrite) {
        throw FileWriteError("файл «" + filePath + "» открыт только для чтения");
    }
}

std::streamoff RecordFile::positionOf(std::size_t index) const {
    return static_cast<std::streamoff>(sizeof(FileHeader) + index * sizeof(Achievement));
}

std::size_t RecordFile::recordCount() const {
    return count;
}

const std::string& RecordFile::path() const {
    return filePath;
}

Achievement RecordFile::read(std::size_t index) {
    if (index >= count) {
        throw FileReadError("в файле «" + filePath + "» нет записи с позицией " +
                            std::to_string(index + 1));
    }
    Achievement record = makeEmptyAchievement();
    stream.clear();
    stream.seekg(positionOf(index));
    stream.read(reinterpret_cast<char*>(&record), sizeof(record));
    if (!stream) {
        throw FileReadError("не удалось прочитать запись в позиции " +
                            std::to_string(index + 1) + " из файла «" + filePath + "»");
    }
    if (!isRecordIntact(record)) {
        throw FileFormatError("запись в позиции " + std::to_string(index + 1) +
                              " файла «" + filePath + "» повреждена");
    }
    return record;
}

void RecordFile::write(std::size_t index, const Achievement& record) {
    requireWriteMode();
    if (index >= count) {
        throw FileWriteError("в файле «" + filePath + "» нет записи с позицией " +
                             std::to_string(index + 1));
    }
    stream.clear();
    stream.seekp(positionOf(index));
    stream.write(reinterpret_cast<const char*>(&record), sizeof(record));
    stream.flush();  // сразу на диск: после сообщения об успехе данные уже сохранены
    if (!stream) {
        throw FileWriteError("не удалось записать изменения в файл «" + filePath + "»");
    }
}

void RecordFile::append(const Achievement& record) {
    requireWriteMode();
    stream.clear();
    stream.seekp(positionOf(count));
    stream.write(reinterpret_cast<const char*>(&record), sizeof(record));
    stream.flush();
    if (!stream) {
        throw FileWriteError("не удалось дописать запись в файл «" + filePath + "»");
    }
    ++count;
}

std::uint32_t RecordFile::takeNextId() {
    const std::uint32_t newId = header.nextId;
    header.nextId = header.nextId + 1;
    writeHeader();
    return newId;
}
