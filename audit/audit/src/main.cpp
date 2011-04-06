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

QString key_word = "ANT";

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

    /*if(!QFile::exists("libr245.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "Ошибка", "В папке с программой не найдена библиотека libr245.dll");
    } else if(!QFile::exists("ftd2xx.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "Ошибка", "В папке с программой не найдена библиотека ftd2xx.dll");
    } else*/ if(utils.loadLibrary("libr245.dll"))
    {
        // Начал реализовывать механизм защиты от незаконного распространения
        if(!QFile::exists("key.txt"))
        {
            utils.showMessage(QMessageBox::Warning, "Ошибка", "В папке с программой не найден файл с ключом key.txt");
            utils.setDevCount(0);
        } else {

            QFile key_file("key.txt");
            unsigned char dev_count = 0;
            unsigned short crc = 0;

            utils.setDevCount(0);

            utils.openFile(&key_file, QIODevice::ReadOnly);

            QTextStream kstr(&key_file);
            QString key = kstr.readAll();

            for(; dev_count < 255 /*MAX DEV COUNT*/; dev_count++)
            {
                crc = utils.crc16(&dev_count, 1, POLYNOM, 0xFFFF);
                if(QString().setNum(crc, 16) == key)
                {
                    utils.setDevCount(dev_count); // установка ограничения на работу с dev_count устройствами
                    break;
                }
                qDebug() << dev_count << " " << QString().setNum(crc, 16);
            }

            qDebug() << "KEY = " << key << "DEV_COUNT = " << dev_count << key;
            utils.closeFile(&key_file);
        }
        //====================================================================
        MainWindow win;
        win.show();

        return app.exec();
    } else
    {
        utils.showMessage(QMessageBox::Warning, "Ошибка", "Не удается загрузить библиотеку libr245.dll");
        return 1;
    }

    return 0;
}
