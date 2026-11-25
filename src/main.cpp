// main.cpp — точка входа: заставка, вход в систему, главное меню.
#include <iostream>

#include "app_menu.h"
#include "auth.h"
#include "console_io.h"
#include "errors.h"
#include "record_file.h"

int main() {
    con::init();
    printBanner();

    try {
        ensureDataDirectory();

        // Первый запуск: учётных данных ещё нет — пользователь их задаёт.
        if (!auth::credentialsExist()) {
            auth::runFirstSetup();
        }
        // Без успешного входа меню не открывается.
        if (!auth::runLogin()) {
            return 1;
        }
        runMainMenu();
    } catch (const InputClosed&) {
        std::cout << "\nВвод завершён. Работа программы прекращена.\n";
    } catch (const AppError& error) {
        std::cout << "Ошибка: " << error.what() << ".\nРабота программы прекращена.\n";
        return 1;
    } catch (const std::exception& error) {
        std::cout << "Непредвиденная ошибка: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
