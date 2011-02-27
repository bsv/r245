/**
  * @file monitor.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса монитора событий, через объект которого можно взаимодействовать
  * с моделью монитора monitor_model.
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
      * Данный тип перечисления задает символьные идентификаторы номерам колонок
      * модели monitor_model.
      */
    enum MonitorAttr
    {
        /// соответсвует номеру колонки, хранящей время события.
        TimeAttr,
        /// соответсвует номеру колонки, хранящей дату события.
        DateAttr,
        /// соответсвует номеру колонки, хранящей символьный идентификатор устройства
        /// (может быть одинаковым для разный устройств, не уникален).
        DevNameAttr,
        /// соответсвует номеру колонки, хранящей уникальный числовой идентфикатор устройства.
        /// С помощью числового идентификатора можно однозначно определить устройство.
        DevNumAttr,
        /// соответсвует номеру колонки, хранящей номер радио канала, который зафиксировал событие.
        ChAttr,
        /// соответсвует номеру колонки, хранящей символьный идентификатор метки
        /// (не уникально в пределах программы).
        TagNameAttr,
        /// соответсвует номеру колонки, хранящей уникальный числовой идентфикатор метки.
        TagIdAttr,
        /// соответсвует номеру колонки, хранящей название зафиксированного события.
        TypeEventAttr,
        /// соответсвует номеру колонки, хранящей код зафиксированного события.
        /// (уникальный идентификатор события)
        TransCodeAttr
    };

    /** @name Конструкторы/деструкторы
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
    /** @name Вспомогательные функции
      * @{
      */
    void initMas();
    void initHeader();
    /** @} */

    /**
      * Хранит информацию о кодах событий в виде
      * <код события> - <название события> (ключ - значение).
      */
    QMap <int, QString> state;

    /**
      * Ссылка на объект модели монитора. Монитор
      * хранит данные о регистрируемых событиях.
      */
    MonitorModel * monitor_model;

    /**
      * Ссылка на объект вспомогательной модели монитора, которая содержит
      * данные о регистрируемых событиях с учетом установленных фильтров функцией setFilter(...).
      * Содержимое данной модели отображается в окне монитора MonitorWindow.
      */
    MonitorFilter * monitor_model_proxy;

};

#endif // MONITOR_H
