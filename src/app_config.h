// app_config.h — название программы и пути к файлам данных.
#pragma once

#include <string>

namespace cfg {

const std::string PROGRAM_NAME =
    "Система учёта студенческой активности с использованием технологии\n"
    "электронной подписи для верификации достижений";
const std::string PROGRAM_CODE = "СУСА-ЭП";
const std::string PROGRAM_VERSION = "0.2";

const std::string DATA_DIR = "data";
const std::string SOURCE_FILE = "data/achievements.dat";   // исходный файл
const std::string VERIFIED_FILE = "data/verified.dat";     // второй файл (позже)
const std::string AUTH_FILE = "data/auth.dat";             // логин и хеш пароля

const int MAX_LOGIN_ATTEMPTS = 3;

} 
