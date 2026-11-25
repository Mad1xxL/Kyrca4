// achievement.cpp — проверка, ввод и вывод записей о достижениях.
#include "achievement.h"

#include <cctype>
#include <cstring>
#include <ctime>
#include <iostream>

#include "console_io.h"
#include "errors.h"

// Текстовые поля 

void setText(char* destination, std::size_t destinationSize, const std::string& text) {
    std::memset(destination, 0, destinationSize);
    const std::size_t copyLength =
        text.size() < destinationSize - 1 ? text.size() : destinationSize - 1;
    std::memcpy(destination, text.data(), copyLength);
}

std::string getText(const char* source, std::size_t sourceSize) {
    std::size_t length = 0;
    while (length < sourceSize && source[length] != '\0') {
        ++length;
    }
    return std::string(source, length);
}

// Вспомогательные проверки 

static bool containsDigit(const std::string& text) {
    for (const char symbol : text) {
        if (std::isdigit(static_cast<unsigned char>(symbol))) {
            return true;
        }
    }
    return false;
}

static bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static int daysInMonth(int month, int year) {
    const int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year)) {
        return 29;
    }
    return daysPerMonth[month - 1];
}

static Date getToday() {
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#ifdef _MSC_VER
    localtime_s(&localTime, &now);  // в Visual Studio std::localtime помечен устаревшим
#else
    localTime = *std::localtime(&now);
#endif
    Date today{};
    today.year = static_cast<std::uint16_t>(localTime.tm_year + 1900);
    today.month = static_cast<std::uint8_t>(localTime.tm_mon + 1);
    today.day = static_cast<std::uint8_t>(localTime.tm_mday);
    return today;
}

// Дата в виде числа ГГГГММДД — так даты удобно сравнивать.
static int dateToNumber(const Date& date) {
    return date.year * 10000 + date.month * 100 + date.day;
}

// Проверка полей 

std::string checkStudentId(const std::string& text) {
    if (text.size() < STUDENT_ID_MIN_DIGITS || text.size() > STUDENT_ID_MAX_DIGITS) {
        return "номер зачётной книжки должен содержать от " +
               std::to_string(STUDENT_ID_MIN_DIGITS) + " до " +
               std::to_string(STUDENT_ID_MAX_DIGITS) + " цифр";
    }
    for (const char symbol : text) {
        if (!std::isdigit(static_cast<unsigned char>(symbol))) {
            return "допускаются только цифры";
        }
    }
    return "";
}

std::string checkFullName(const std::string& text) {
    const std::size_t length = con::textLength(text);
    if (length < 3 || length > FULL_NAME_MAX_LENGTH) {
        return "ФИО должно содержать от 3 до " + std::to_string(FULL_NAME_MAX_LENGTH) +
               " символов";
    }
    if (text.find(' ') == std::string::npos) {
        return "укажите фамилию и имя (или инициалы) через пробел";
    }
    if (containsDigit(text)) {
        return "ФИО не может содержать цифры";
    }
    return "";
}

std::string checkGroup(const std::string& text) {
    const std::size_t length = con::textLength(text);
    if (length < 3 || length > GROUP_MAX_LENGTH) {
        return "название группы должно содержать от 3 до " +
               std::to_string(GROUP_MAX_LENGTH) + " символов";
    }
    const std::size_t dashPosition = text.find('-');
    if (dashPosition == std::string::npos || dashPosition == 0 ||
        dashPosition == text.size() - 1) {
        return "ожидается формат «буквы-цифры», например АБс-524";
    }
    return "";
}

std::string checkTitle(const std::string& text) {
    const std::size_t length = con::textLength(text);
    if (length < TITLE_MIN_LENGTH || length > TITLE_MAX_LENGTH) {
        return "название должно содержать от " + std::to_string(TITLE_MIN_LENGTH) +
               " до " + std::to_string(TITLE_MAX_LENGTH) + " символов";
    }
    return "";
}

std::string checkDate(const std::string& text, Date& resultDate) {
    // Формат строго ДД.ММ.ГГГГ: 10 символов, точки на 3-й и 6-й позициях.
    if (text.size() != 10 || text[2] != '.' || text[5] != '.') {
        return "ожидается формат ДД.ММ.ГГГГ, например 15.10.2026";
    }
    for (std::size_t position = 0; position < text.size(); ++position) {
        if (position == 2 || position == 5) {
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(text[position]))) {
            return "ожидается формат ДД.ММ.ГГГГ, например 15.10.2026";
        }
    }
    const int day = std::stoi(text.substr(0, 2));
    const int month = std::stoi(text.substr(3, 2));
    const int year = std::stoi(text.substr(6, 4));

    if (month < 1 || month > 12) {
        return "месяц должен быть от 01 до 12";
    }
    if (day < 1 || day > daysInMonth(month, year)) {
        return "в этом месяце нет такого дня";
    }
    const Date today = getToday();
    if (year < MIN_YEAR) {
        return "год не может быть раньше " + std::to_string(MIN_YEAR);
    }
    Date enteredDate{};
    enteredDate.year = static_cast<std::uint16_t>(year);
    enteredDate.month = static_cast<std::uint8_t>(month);
    enteredDate.day = static_cast<std::uint8_t>(day);
    if (dateToNumber(enteredDate) > dateToNumber(today)) {
        return "дата не может быть позже сегодняшней (" + dateToString(today) + ")";
    }
    resultDate = enteredDate;
    return "";
}

static bool hasTerminatingZero(const char* field, std::size_t fieldSize) {
    return std::memchr(field, '\0', fieldSize) != nullptr;
}

bool isRecordIntact(const Achievement& record) {
    if (!hasTerminatingZero(record.studentId, STUDENT_ID_SIZE) ||
        !hasTerminatingZero(record.fullName, FULL_NAME_SIZE) ||
        !hasTerminatingZero(record.group, GROUP_SIZE) ||
        !hasTerminatingZero(record.title, TITLE_SIZE)) {
        return false;
    }
    if (record.category < 1 || record.category > CATEGORY_COUNT) return false;
    if (record.level < 1 || record.level > LEVEL_COUNT) return false;
    if (record.points < MIN_POINTS || record.points > MAX_POINTS) return false;
    if (record.date.month < 1 || record.date.month > 12) return false;
    if (record.isSigned > 1 || record.isDeleted > 1) return false;
    return true;
}

// Ввод 

Achievement makeEmptyAchievement() {
    Achievement record;
    std::memset(&record, 0, sizeof(record));
    return record;
}

// Общая схема ввода текстового поля: запрос → проверка → при ошибке
// сообщение с названием поля и повтор. «0» отменяет всю операцию.
static std::string readCheckedText(Field field, const std::string& prompt,
                                   std::string (*checkFunction)(const std::string&)) {
    while (true) {
        const std::string text = con::readLine(prompt);
        if (con::isCancel(text)) {
            throw OperationCancelled();
        }
        const std::string error = checkFunction(text);
        if (error.empty()) {
            return text;
        }
        std::cout << "Ошибка в поле «" << fieldName(field) << "»: " << error
                  << ".\nПовторите ввод или введите 0 для отмены операции.\n";
    }
}

// Ввод числа из ограниченного списка. «0» отменяет операцию.
static int readChoice(Field field, const std::string& prompt, int maxValue) {
    const std::string errorMessage = "Ошибка в поле «" + fieldName(field) +
                                     "»: выберите номер от 1 до " +
                                     std::to_string(maxValue) + " (0 — отмена).";
    const int choice = con::readInt(prompt, 0, maxValue, errorMessage);
    if (choice == 0) {
        throw OperationCancelled();
    }
    return choice;
}

void inputField(Achievement& record, Field field) {
    switch (field) {
        case Field::StudentId: {
            const std::string value = readCheckedText(
                field, "Номер зачётной книжки (6–10 цифр): ", checkStudentId);
            setText(record.studentId, STUDENT_ID_SIZE, value);
            break;
        }
        case Field::FullName: {
            const std::string value = readCheckedText(
                field, "ФИО студента (например, Иванов Иван Иванович): ", checkFullName);
            setText(record.fullName, FULL_NAME_SIZE, value);
            break;
        }
        case Field::Group: {
            const std::string value =
                readCheckedText(field, "Группа (например, АБс-524): ", checkGroup);
            setText(record.group, GROUP_SIZE, value);
            break;
        }
        case Field::Category: {
            std::cout << "Категории: ";
            for (int category = 1; category <= CATEGORY_COUNT; ++category) {
                std::cout << category << " — " << categoryName(category)
                          << (category < CATEGORY_COUNT ? "; " : "\n");
            }
            record.category =
                static_cast<std::uint8_t>(readChoice(field, "Категория (1–5): ", CATEGORY_COUNT));
            break;
        }
        case Field::Title: {
            const std::string value = readCheckedText(
                field, "Название достижения (3–100 символов): ", checkTitle);
            setText(record.title, TITLE_SIZE, value);
            break;
        }
        case Field::Level: {
            std::cout << "Уровни: ";
            for (int level = 1; level <= LEVEL_COUNT; ++level) {
                std::cout << level << " — " << levelName(level)
                          << (level < LEVEL_COUNT ? "; " : "\n");
            }
            record.level =
                static_cast<std::uint8_t>(readChoice(field, "Уровень (1–5): ", LEVEL_COUNT));
            break;
        }
        case Field::Date: {
            while (true) {
                const std::string text = con::readLine("Дата получения (ДД.ММ.ГГГГ): ");
                if (con::isCancel(text)) {
                    throw OperationCancelled();
                }
                Date enteredDate{};
                const std::string error = checkDate(text, enteredDate);
                if (error.empty()) {
                    record.date = enteredDate;
                    break;
                }
                std::cout << "Ошибка в поле «" << fieldName(field) << "»: " << error
                          << ".\nПовторите ввод или введите 0 для отмены операции.\n";
            }
            break;
        }
        case Field::Points: {
            record.points = static_cast<std::uint16_t>(
                readChoice(field, "Баллы за достижение (1–100): ", MAX_POINTS));
            break;
        }
    }
}

Achievement inputAchievement() {
    std::cout << "Введите данные достижения. Для отмены введите 0 в любом поле.\n";
    Achievement record = makeEmptyAchievement();
    for (int fieldNumber = 1; fieldNumber <= FIELD_COUNT; ++fieldNumber) {
        inputField(record, static_cast<Field>(fieldNumber));
    }
    return record;
}

// Вывод 

std::string fieldName(Field field) {
    switch (field) {
        case Field::StudentId: return "Номер зачётной книжки";
        case Field::FullName:  return "ФИО";
        case Field::Group:     return "Группа";
        case Field::Category:  return "Категория";
        case Field::Title:     return "Название достижения";
        case Field::Level:     return "Уровень";
        case Field::Date:      return "Дата";
        case Field::Points:    return "Баллы";
    }
    return "?";
}

std::string categoryName(int category) {
    switch (category) {
        case 1: return "Учебная";
        case 2: return "Научная";
        case 3: return "Спортивная";
        case 4: return "Общественная";
        case 5: return "Творческая";
    }
    return "?";
}

std::string levelName(int level) {
    switch (level) {
        case 1: return "Вузовский";
        case 2: return "Городской";
        case 3: return "Региональный";
        case 4: return "Всероссийский";
        case 5: return "Международный";
    }
    return "?";
}

std::string dateToString(const Date& date) {
    std::string dayText = std::to_string(date.day);
    std::string monthText = std::to_string(date.month);
    if (dayText.size() < 2) dayText = "0" + dayText;
    if (monthText.size() < 2) monthText = "0" + monthText;
    return dayText + "." + monthText + "." + std::to_string(date.year);
}

void printTableHeader() {
    con::printRule(100);
    std::cout << "Рег.№ | Зачётка | ФИО | Группа | Категория | Уровень | Дата | Баллы | ЭП\n";
    con::printRule(100);
}

void printTableRow(const Achievement& record) {
    if (record.id == 0) {
        std::cout << "(новая)";  // номер присваивается при сохранении
    } else {
        std::cout << record.id;
    }
    std::cout << " | " << getText(record.studentId, STUDENT_ID_SIZE) << " | "
              << getText(record.fullName, FULL_NAME_SIZE) << " | "
              << getText(record.group, GROUP_SIZE) << " | " << categoryName(record.category)
              << " | " << levelName(record.level) << " | " << dateToString(record.date) << " | "
              << record.points << " | " << (record.isSigned ? "да" : "нет") << "\n";
    std::cout << "      Достижение: " << getText(record.title, TITLE_SIZE) << "\n";
}
