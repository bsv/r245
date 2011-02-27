/**
  * @file monitor.h
  * @authors ��������� ������ (bsv.serg@gmail.com)
  * @authors �������� �������
  *
  * �������� �������� ������ �������� �������, ����� ������ �������� ����� �����������������
  * � ������� �������� monitor_model.
  */

#ifndef MONITOR_H
#define MONITOR_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include "global.h"
#include <QDateTime>
#include "monitor_filter.h"
#include "monitor_model.h"

class SettingsObj;

class Monitor
{
public:
    /**
      * ������ ��� ������������ ������ ���������� �������������� ������� �������
      * ������ monitor_model.
      */
    enum MonitorAttr
    {
        /// ������������ ������ �������, �������� ����� �������.
        TimeAttr,
        /// ������������ ������ �������, �������� ���� �������.
        DateAttr,
        /// ������������ ������ �������, �������� ���������� ������������� ����������
        /// (����� ���� ���������� ��� ������ ���������, �� ��������).
        DevNameAttr,
        /// ������������ ������ �������, �������� ���������� �������� ������������ ����������.
        /// � ������� ��������� �������������� ����� ���������� ���������� ����������.
        DevNumAttr,
        /// ������������ ������ �������, �������� ����� ����� ������, ������� ������������ �������.
        ChAttr,
        /// ������������ ������ �������, �������� ���������� ������������� �����
        /// (�� ��������� � �������� ���������).
        TagNameAttr,
        /// ������������ ������ �������, �������� ���������� �������� ������������ �����.
        TagIdAttr,
        /// ������������ ������ �������, �������� �������� ���������������� �������.
        TypeEventAttr,
        /// ������������ ������ �������, �������� ��� ���������������� �������.
        /// (���������� ������������� �������)
        TransCodeAttr
    };

    /** @name ������������/�����������
      * @{
      */
    Monitor();
    ~Monitor();
    /** @} */

    void addTransToModel(QString dev_num, R245_TRANSACT * trans, const QString &tag_name, const QString &dev_name);
    void setFilter(QString channel, QString device, QString tag, QString event, QDate daten, QDate datem, QTime timen, QTime timem);
    void onlyTagInf(bool only = true);
    QAbstractItemModel * getModel(bool proxy);
    void clear();
    void update();
    void updateAlias(SettingsObj * set_obj);
    QMap <int, QString> * getState();


private:
    /** @name ��������������� �������
      * @{
      */
    void initMas();
    void initHeader();
    /** @} */

    /**
      * ������ ���������� � ����� ������� � ����
      * <��� �������> - <�������� �������> (���� - ��������).
      */
    QMap <int, QString> state;

    /**
      * ������ �� ������ ������ ��������. �������
      * ������ ������ � �������������� ��������.
      */
    MonitorModel * monitor_model;

    /**
      * ������ �� ������ ��������������� ������ ��������, ������� ��������
      * ������ � �������������� �������� � ������ ������������� �������� �������� setFilter(...).
      * ���������� ������ ������ ������������ � ���� �������� MonitorWindow.
      */
    MonitorFilter * monitor_model_proxy;

};

#endif // MONITOR_H
