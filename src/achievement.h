// achievement.h — описание записи о достижении, проверка полей, ввод и вывод.
// Все записи имеют фиксированный размер, поэтому запись номер index лежит
// в файле по смещению «заголовок + index * sizeof(Achievement)».
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// Размеры текстовых полей в байтах (с учётом завершающего нуля).
// В UTF-8 русская буква занимает 2 байта, поэтому буфер взят с запасом.
const std::size_t STUDENT_ID_SIZE = 12;
const std::size_t FULL_NAME_SIZE = 128;
const std::size_t GROUP_SIZE = 24;
const std::size_t TITLE_SIZE = 208;

// Допустимая длина полей в символах.
const std::size_t STUDENT_ID_MIN_DIGITS = 6;
const std::size_t STUDENT_ID_MAX_DIGITS = 10;
const std::size_t FULL_NAME_MAX_LENGTH = 60;
const std::size_t GROUP_MAX_LENGTH = 12;
const std::size_t TITLE_MIN_LENGTH = 3;
const std::size_t TITLE_MAX_LENGTH = 100;

const int CATEGORY_COUNT = 5;  // учебная, научная, спортивная, общественная, творческая
const int LEVEL_COUNT = 5;     // вузовский ... международный
const int MIN_POINTS = 1;
const int MAX_POINTS = 100;
const int MIN_YEAR = 2000;

// Поля записи. Номера используются в меню изменения и сортировки.
enum class Field {
    StudentId = 1,
    FullName,
    Group,
    Category,
    Title,
    Level,
    Date,
    Points
};
const int FIELD_COUNT = 8;

// pack(1) отключает выравнивание: структура лежит в файле байт в байт,
// без пустых промежутков между полями.
#pragma pack(push, 1)

struct Date {
    std::uint16_t year;
    std::uint8_t month;
    std::uint8_t day;
};

struct Achievement {
    std::uint32_t id;                  // регистрационный номер (уникальный)
    char studentId[STUDENT_ID_SIZE];   // номер зачётной книжки — КЛЮЧ ПОИСКА
    char fullName[FULL_NAME_SIZE];     // ФИО студента
    char group[GROUP_SIZE];            // учебная группа, например АБс-524
    std::uint8_t category;             // категория достижения, 1..5
    std::uint8_t level;                // уровень мероприятия, 1..5
    char title[TITLE_SIZE];            // название достижения / мероприятия
    Date date;                         // дата получения
    std::uint16_t points;              // баллы для рейтинга, 1..100
    std::uint8_t isSigned;             // 1 — запись подписана ЭП
    std::uint8_t isDeleted;            // 1 — запись логически удалена
    std::uint32_t signature;           // значение электронной подписи
};

#pragma pack(pop)

static_assert(sizeof(Achievement) == 390, "Размер записи изменился: обновите формат файла");

// Работа с текстовыми полями 

// Копирует строку в поле фиксированного размера (всегда с нулём в конце).
void setText(char* destination, std::size_t destinationSize, const std::string& text);

// Читает строку из поля фиксированного размера.
std::string getText(const char* source, std::size_t sourceSize);

// Проверка значений полей
// Каждая функция возвращает пустую строку, если значение верно,
// иначе — текст ошибки для пользователя.

std::string checkStudentId(const std::string& text);
std::string checkFullName(const std::string& text);
std::string checkGroup(const std::string& text);
std::string checkTitle(const std::string& text);
std::string checkDate(const std::string& text, Date& resultDate);

// Проверка записи, прочитанной из файла: строки завершены нулём,
// числовые поля в допустимых диапазонах. false — запись повреждена.
bool isRecordIntact(const Achievement& record);

// Ввод с клавиатуры 

// Пустая запись (все байты нулевые).
Achievement makeEmptyAchievement();

// Ввод всех полей новой записи. Ввод «0» в любом поле отменяет операцию
// (бросается OperationCancelled). Регистрационный номер не заполняется.
Achievement inputAchievement();

// Ввод одного поля записи (используется при добавлении и изменении).
void inputField(Achievement& record, Field field);

// Вывод 

std::string fieldName(Field field);
std::string categoryName(int category);
std::string levelName(int level);
std::string dateToString(const Date& date);

void printTableHeader();
void printTableRow(const Achievement& record);
