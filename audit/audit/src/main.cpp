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

    if(!QFile::exists("r245.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������� ���������� r245.dll");
    } else if(!QFile::exists("ftd2xx.dll"))
    {
        utils.showMessage(QMessageBox::Warning, "������", "� ����� � ���������� �� ������� ���������� ftd2xx.dll");
    } else if(utils.loadLibrary("r245.dll"))
    {
        MainWindow win;
        win.show();

        return app.exec();
    } else
    {
        utils.showMessage(QMessageBox::Warning, "������", "�� ������� ��������� ���������� r245.dll");
        return 1;
    }

    return 0;
}
