/**
  * @file main.cpp
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Основной файл программы с блоком main.
  */

#include <QApplication>
#include <QTextCodec>
#include "main_window.h"
#include "utils.h"
#include "global.h"

/**
  * @mainpage Система мониторинга на базе считывателя PR-G07
  *
  * Данный программный коплекс предоставляет графический
  * интерфейс работы с сичтывателем PR-G07. Для взаимодействия
  * с аппаратным устройством считывателя используется библиотека libr245dll.dll.
  * @n
  * Более подробное описание принципа работы с программой вы можете прочитать
  * в документации по практическому применению. В данном руководстве описываются
  * вопросы ориентированные на разработчиков.
  */

/**
  * Глобальный объект класса Utils.
  *
  * Объект, объявленный глобально, предоставляет доступ к
  * возможностям класса Utils из любой точки программы.
  * @n
  * С помощью функций класса возможно решать некоторые вспомогательные задачи
  * (различные преобразования форматов данных, загрузка библиотеки libr245dll.dll и т.п).
  */
Utils utils;

/**
  * Точка входа в программу.
  *
  * Функция создает объект приложения qt,
  * настраивает кодировку, задает имя программы и организации,
  * создает главное окно и запускает цикл обработки событий app.exec().
  */
int main(int argc, char ** argv)
{
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    QTextCodec::setCodecForCStrings(codec);

    QCoreApplication::setOrganizationName("ANT");
    QCoreApplication::setApplicationName("R245 monitor");

    QApplication app( argc, argv );
    MainWindow win;
    win.show();

    return app.exec();
}
