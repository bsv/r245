/**
  * @file monitor_filter.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса фильтра событий.
  */

#ifndef MONITOR_FILTER_H
#define MONITOR_FILTER_H

#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QDateTime>
#include <QApplication>
#include "global.h"

/**
  * Класс позволяет фильтровать содержимое вспомогательной модели монитора
  * monitor_proxy_model по времени, дате, идентификатору устройства,
  * идентификатору метки и номеру канала.
  *
  */
class MonitorFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MonitorFilter(QObject *parent = 0);

    /// Возвращает нижнюю границу даты
    QDate filterMinimumDate() const { return minDate; }
    void setFilterMinimumDate(const QDate date);

    /// Возвращает верхнюю границу даты
    QDate filterMaximumDate() const { return maxDate; }
    void setFilterMaximumDate(const QDate date);

    /// Возвращает нижнюю границу времени
    QTime filterMinimumTime() const { return minTime; }
    void setFilterMinimumTime( QTime time);

    /// Возвращает верхнюю границу времени
    QTime filterMaximumTime() const { return maxTime; }
    void setFilterMaximumTime(const QTime time);

    void setRegExp(QRegExp channel, QRegExp device, QRegExp tag, QRegExp event);
    void setTransCodeRE(QString reg_exp);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool dateInRange(const QDate &date) const;
    bool timeInRange(const QTime &time) const;

    /// Нижняя граница даты
    QDate minDate;

    /// Верхняя граница даты
    QDate maxDate;

    /// Нижняя граница времени
    QTime minTime;

    /// Верхняя граница времени
    QTime maxTime;

    /// Регулярное выражение для фильтрации по каналу
    QRegExp channelRegExp;

    /// Регулярное выражение для фильтрации по меткам
    QRegExp tagRegExp;

    /// Регулярное выражение для фильтрации по устройствам
    QRegExp deviceRegExp;

    /// Регулярное выражение для фильтрации по коду события
    QRegExp trans_code_rexp;

    /// Регулярное выражение для фильтрации по символьному имени типа события
    QRegExp event_reg_exp;
};

#endif // MONITOR_FILTER_H
