// record_ops.cpp — операции над записями.
// Записи читаются из файла по одной, без загрузки в массив
#include "record_ops.h"

#include <iostream>
#include <string>

#include "achievement.h"
#include "app_config.h"
#include "console_io.h"
#include "errors.h"
#include "record_file.h"

namespace ops {

static void printNotReady(const std::string& operationName) {
    std::cout << "Функция «" << operationName << "» находится в разработке.\n";
}

// Вывод всех не удалённых записей файла таблицей.
static void showFile(const std::string& path, FileKind kind) {
    RecordFile file(path, kind, RecordFile::Mode::ReadOnly);
    std::cout << "\n" << fileKindName(kind) << " («" << path << "»)\n";
    std::size_t shownCount = 0;
    for (std::size_t position = 0; position < file.recordCount(); ++position) {
        const Achievement record = file.read(position);
        if (record.isDeleted) {
            continue;
        }
        if (shownCount == 0) {
            printTableHeader();
        }
        printTableRow(record);
        ++shownCount;
    }
    if (shownCount == 0) {
        std::cout << "Файл не содержит записей.\n";
    } else {
        con::printRule(100);
        std::cout << "Всего записей: " << shownCount << "\n";
    }
}

// Создание исходного файла

void createSourceFile() {
    ensureDataDirectory();

    if (fileExists(cfg::SOURCE_FILE)) {
        std::cout << "Исходный файл уже существует.\n";
        if (!con::confirm("Создать новый файл? Все прежние записи будут потеряны")) {
            throw OperationCancelled();
        }
    }

    // Записи пишутся во временный файл. Он заменяет исходный только после
    // подтверждения, поэтому при отмене старый файл не меняется.
    const std::string temporaryPath = cfg::SOURCE_FILE + ".tmp";
    writeHeaderOnlyFile(temporaryPath, FileKind::Source);
    std::size_t enteredCount = 0;
    {
        RecordFile newFile(temporaryPath, FileKind::Source, RecordFile::Mode::ReadWrite);
        while (true) {
            std::cout << "\n--- Запись " << enteredCount + 1 << " ---\n";
            try {
                Achievement record = inputAchievement();
                record.id = newFile.takeNextId();
                newFile.append(record);
                ++enteredCount;
                std::cout << "Запись принята, регистрационный номер: " << record.id << ".\n";
            } catch (const OperationCancelled&) {
                std::cout << "Ввод текущей записи прерван.\n";
                break;
            }
            if (!con::confirm("Ввести ещё одну запись?")) {
                break;
            }
        }
    }

    bool shouldSave = false;
    if (enteredCount == 0) {
        shouldSave = con::confirm("Записи не введены. Создать пустой исходный файл?");
    } else {
        shouldSave = con::confirm("Сохранить исходный файл (записей: " +
                                  std::to_string(enteredCount) + ")?");
    }
    if (!shouldSave) {
        removeFileQuietly(temporaryPath);
        throw OperationCancelled();
    }
    replaceFile(temporaryPath, cfg::SOURCE_FILE);
    std::cout << "Исходный файл «" << cfg::SOURCE_FILE << "» создан. Записей: " << enteredCount
              << ".\n";
}

// Просмотр

void viewSourceFile() {
    showFile(cfg::SOURCE_FILE, FileKind::Source);
}

// Добавление

void addRecord() {
    std::uint32_t newId = 0;
    {
        RecordFile file(cfg::SOURCE_FILE, FileKind::Source, RecordFile::Mode::ReadWrite);
        Achievement record = inputAchievement();
        std::cout << "\nПроверьте введённые данные:\n";
        printTableRow(record);
        if (!con::confirm("Сохранить запись?")) {
            throw OperationCancelled();
        }
        newId = file.takeNextId();
        record.id = newId;
        file.append(record);  // дописываем в конец, остальные записи не трогаем
    }
    std::cout << "Запись добавлена. Регистрационный номер: " << newId << ".\n";
    if (con::confirm("Показать содержимое исходного файла?")) {
        viewSourceFile();
    }
}

// В разработке

void editRecord() {

    printNotReady("Изменить запись");
}

void deleteRecord() {

    printNotReady("Удалить запись");
}

void searchByStudentId() {

    printNotReady("Поиск по номеру зачётной книжки");
}

void sortRecords() {
    printNotReady("Сортировка записей");
}

void filterRecords() {
    printNotReady("Фильтрация записей");
}

void buildVerifiedFile() {
    printNotReady("Сформировать файл подтверждённых достижений");
}

void viewVerifiedFile() {
    printNotReady("Просмотреть файл подтверждённых достижений");
}

void showActivityRating() {
    printNotReady("Рейтинг активности");
}

}
