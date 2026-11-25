// app_menu.cpp — главное меню и обработка ошибок операций.
#include "app_menu.h"

#include <iostream>
#include <string>
#include <vector>

#include "app_config.h"
#include "console_io.h"
#include "errors.h"
#include "record_ops.h"

// Пункт меню: номер команды, название и функция, которая её выполняет.
struct MenuItem {
    int command;
    std::string title;
    void (*action)();
};

void printBanner() {
    con::printRule(72, '=');
    std::cout << cfg::PROGRAM_CODE << "\n" << cfg::PROGRAM_NAME << "\n"
              << "Версия " << cfg::PROGRAM_VERSION << "\n";
    con::printRule(72, '=');
}

// Выполняет операцию. Любая ожидаемая ошибка выводится, после чего
// пользователь возвращается в меню.
static void runAction(const MenuItem& item) {
    try {
        item.action();
    } catch (const OperationCancelled&) {
        std::cout << "Операция отменена. Данные не изменены.\n";
    } catch (const InputClosed&) {
        throw;
    } catch (const AppError& error) {
        std::cout << "Ошибка: операция «" << item.title << "» не выполнена.\n"
                  << "Причина: " << error.what() << ".\n";
    }
}

static void signatureMenu() {

    std::cout << "Функция «Электронная подпись» находится в разработке.\n";
}

void runMainMenu() {
    const std::vector<MenuItem> items = {
        {1, "Создать исходный файл (ввод записей)", ops::createSourceFile},
        {2, "Просмотреть исходный файл", ops::viewSourceFile},
        {3, "Добавить запись", ops::addRecord},
        {4, "Изменить запись", ops::editRecord},
        {5, "Удалить запись", ops::deleteRecord},
        {6, "Поиск по номеру зачётной книжки", ops::searchByStudentId},
        {7, "Сортировка записей", ops::sortRecords},
        {8, "Фильтрация записей", ops::filterRecords},
        {9, "Сформировать файл подтверждённых достижений", ops::buildVerifiedFile},
        {10, "Просмотреть файл подтверждённых достижений", ops::viewVerifiedFile},
        {11, "Электронная подпись", signatureMenu},
        {12, "Рейтинг активности", ops::showActivityRating},
    };
    const int maxCommand = 12;

    while (true) {
        std::cout << "\n===== Главное меню =====\n";
        for (const MenuItem& item : items) {
            std::cout << (item.command < 10 ? " " : "") << item.command << " — " << item.title
                      << "\n";
        }
        std::cout << " 0 — Выход\n";

        const int command = con::readInt("Введите номер команды: ", 0, maxCommand,
                                         "Ошибка: неизвестная команда. Введите номер от 0 до 12.");
        if (command == 0) {
            break;
        }
        runAction(items[command - 1]);
    }
    std::cout << "Работа программы завершена.\n";
}
