// record_ops.h — операции над записями.
// Каждая функция — один пункт меню.
#pragma once

namespace ops {

void createSourceFile();    //ввод записей и создание исходного файла
void viewSourceFile();      //просмотр исходного файла
void addRecord();           //добавление

// В разработке:
void editRecord();          //изменение
void deleteRecord();        //удаление
void searchByStudentId();   //поиск по номеру зачётной книжки
void sortRecords();         //сортировка
void filterRecords();       //фильтрация
void buildVerifiedFile();   //второй файл (подтверждённые достижения)
void viewVerifiedFile();    //просмотр второго файла
void showActivityRating();  //рейтинг активности

} 
