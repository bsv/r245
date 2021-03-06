/**
  * @file monitor_filter.h
  * @authors ��������� ������ (bsv.serg@gmail.com)
  * @authors �������� �������
  *
  * �������� �������� ������ ������� �������.
  */

#ifndef MONITOR_FILTER_H
#define MONITOR_FILTER_H

#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QDateTime>
#include <QApplication>
#include "global.h"

/**
  * ����� ��������� ����������� ���������� ��������������� ������ ��������
  * monitor_proxy_model �� �������, ����, �������������� ����������,
  * �������������� ����� � ������ ������.
  *
  */
class MonitorFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MonitorFilter(QObject *parent = 0);

    /// ���������� ������ ������� ����
    QDate filterMinimumDate() const { return minDate; }
    void setFilterMinimumDate(const QDate date);

    /// ���������� ������� ������� ����
    QDate filterMaximumDate() const { return maxDate; }
    void setFilterMaximumDate(const QDate date);

    /// ���������� ������ ������� �������
    QTime filterMinimumTime() const { return minTime; }
    void setFilterMinimumTime( QTime time);

    /// ���������� ������� ������� �������
    QTime filterMaximumTime() const { return maxTime; }
    void setFilterMaximumTime(const QTime time);

    void setRegExp(QRegExp channel, QRegExp device, QRegExp tag, QRegExp event);
    void setTransCodeRE(QString reg_exp);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool dateInRange(const QDate &date) const;
    bool timeInRange(const QTime &time) const;

    /// ������ ������� ����
    QDate minDate;

    /// ������� ������� ����
    QDate maxDate;

    /// ������ ������� �������
    QTime minTime;

    /// ������� ������� �������
    QTime maxTime;

    /// ���������� ��������� ��� ���������� �� ������
    QRegExp channelRegExp;

    /// ���������� ��������� ��� ���������� �� ������
    QRegExp tagRegExp;

    /// ���������� ��������� ��� ���������� �� �����������
    QRegExp deviceRegExp;

    /// ���������� ��������� ��� ���������� �� ���� �������
    QRegExp trans_code_rexp;

    /// ���������� ��������� ��� ���������� �� ����������� ����� ���� �������
    QRegExp event_reg_exp;
};

#endif // MONITOR_FILTER_H
