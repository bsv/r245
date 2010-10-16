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
    MainWindow win;
    win.show();

    return app.exec();
}
