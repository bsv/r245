/**
  * @file main.cpp
  * @authors ��������� ������ (bsv.serg@gmail.com)
  * @authors �������� �������
  *
  * �������� ���� ��������� � ������ main.
  */

#include <QApplication>
#include <QTextCodec>
#include "main_window.h"
#include "utils.h"
#include "global.h"

/**
  * @mainpage ������� ����������� �� ���� ����������� PR-G07
  *
  * ������ ����������� ������� ������������� �����������
  * ��������� ������ � ������������ PR-G07. ��� ��������������
  * � ���������� ����������� ����������� ������������ ���������� libr245dll.dll.
  * @n
  * ����� ��������� �������� �������� ������ � ���������� �� ������ ���������
  * � ������������ �� ������������� ����������. � ������ ����������� �����������
  * ������� ��������������� �� �������������.
  */

/**
  * ���������� ������ ������ Utils.
  *
  * ������, ����������� ���������, ������������� ������ �
  * ������������ ������ Utils �� ����� ����� ���������.
  * @n
  * � ������� ������� ������ �������� ������ ��������� ��������������� ������
  * (��������� �������������� �������� ������, �������� ���������� libr245dll.dll � �.�).
  */
Utils utils;

QString key_word = "ANT";

/**
  * ����� ����� � ���������.
  *
  * ������� ������� ������ ���������� qt,
  * ����������� ���������, ������ ��� ��������� � �����������,
  * ������� ������� ���� � ��������� ���� ��������� ������� app.exec().
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
        utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������� ���������� libr245.dll");
    } else if(!QFile::exists("ftd2xx.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������� ���������� ftd2xx.dll");
    } else*/ if(utils.loadLibrary("libr245.dll"))
    {
        // ����� ������������� �������� ������ �� ����������� ���������������
        if(!QFile::exists("key.txt"))
        {
            utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������ ���� � ������ key.txt");
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
                    utils.setDevCount(dev_count); // ��������� ����������� �� ������ � dev_count ������������
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
        utils.showMessage(QMessageBox::Warning, "������", "�� ������� ��������� ���������� libr245.dll");
        return 1;
    }

    return 0;
}
