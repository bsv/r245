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
#include <QStringList>

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

    if(!QFile::exists("lib/libr245.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "������", "� ����� lib �� ������� ���������� libr245.dll");    }
    else if(!QFile::exists("lib/ftd2xx.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "������", "� ����� lib �� ������� ���������� ftd2xx.dll");    }
    else if(utils.loadLibrary("lib/libr245.dll"))
    {
        QStringList * str_list = new QStringList();
        utils.setDevList(str_list);

        if(!QFile::exists("key.txt"))
        {
            utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������ ���� � ������ key.txt");
        } else {

            QFile key_file("key.txt");

            utils.openFile(&key_file, QIODevice::ReadOnly);

            QTextStream kstr(&key_file);

            while(!kstr.atEnd())
            {
                *str_list << kstr.readLine();
            }

            qDebug() << "KEY count = " << str_list->size();
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
