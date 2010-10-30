#ifndef SETTINGS_OBJ_H
#define SETTINGS_OBJ_H

#include <QFile>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QtXml>
#include <QIODevice>
#include <QTextStream>
#include <QDebug>
#include "global.h"
#include "monitor.h"

/** @name Константы для активации каналов
  * Если оба канала активны, то поле HEAD_DEV_INFO_CHANNEL
  * в dev_model имеет значение (CHANNEL_ACT_1 | CHANNEL_ACT_2)
  * @{
  */
#define CHANNEL_ACT_1 1
#define CHANNEL_ACT_2 2
/** @}*/

/**
  * Класс содержит необходимый интерфейс для
  * взаимодействия с настройками и  конфигурационными
  * файлами программы
  */
class SettingsObj : public QObject
{
    Q_OBJECT
public:

    /**
      * Элементы перечисления соответствуют
      * идентификаторам моделей, которые содержит класс
      */
    enum TypeModel {
        TagModel,
        DevNameModel,
        DevModel,
        EventModel,
        TagModelProxy,
        DevNameModelProxy,
        EventModelProxy
    };

    /**
      * Элементы перечисления соответствуют
      * номерам колонок в моделях настроек синонимов
      * для названий меток и устройств (tag_model, dev_name_model)
      */
    enum AliasTable
    {
        AliasId,
        AliasName
    };

    /**
      * Элементы перечисления соответствуют
      * номерам колонок в модели dev_model
      */
    enum DevInfoAttr
    {
       // Num,
        Type,
        Id,
        LocId,
        Desc
    };

    /**
      * Элементы перечисления соответствуют
      * номерам колонок в модели event_model
      */
    enum EventAttr
    {
        EvNameDev,
        EvName,
        EvChanell,
        EvNameTag,
        EvEvent,
        EvReact,
        EvIdDev,
        EvIdTag
    };

    /** @name Конструкторы/деструкторы
      * @{
      */
    SettingsObj();
    ~SettingsObj();
    /** @} */

    bool openSettingFile(QString file_name);
    bool openLogFile(QString file_name, Monitor * monitor);
    QAbstractItemModel * getModel(TypeModel type_model);
    void setFilterWildCard(QString ex, TypeModel type_model);
    void addTagToModel(QString id = "", QString name = "");
    void addDevNameToModel(QString id = "", QString name = "");
    void addEventToModel(QString id_dev = "",
                         QString name = "",
                         QString chanell = "",
                         QString id_tag = "",
                         QString event = "",
                         QString react = "",
                         int red = 255, int green = 255, int blue = 255);
    void addDevInfoToModel(/*QString num = "",*/
                           QString type = "",
                           QString id = "",
                           QString loc_id = "",
                           QString desc = "");
    short int setActiveDev(int row, bool active);
    short int setChannelDev(int row, short int channel);
    short int setTimeDev(int row, short int time, bool time1);
    short int setDistDev(int row, short int dist, bool dist1);
    DEV_INFO * getDevSettings(unsigned int id);
    void readDevInfo();
    void saveSetings();
    void addLogNode(QString dev_num, R245_TRANSACT * trans);
private:

    /// Указатель на файл настроек
    QFile * fsettings;

    /// Указатель на файл журнала
    QFile * flog;

    /// Указатель на модель синонимов меток
    QStandardItemModel * tag_model;

    /// Указатель на модель отображения результатов поиска по меткам
    QSortFilterProxyModel * tag_model_proxy;

    /// Указатель на модель синонимов для устройств
    QStandardItemModel * dev_name_model;

    /// Указатель на модель отображения результатов поиска по устройствам
    QSortFilterProxyModel * dev_name_model_proxy;

    /// Указатель на модель, хранящую информацию о устройствах
    QStandardItemModel * dev_model;

    /// Указатель на модель, хранящую настройку событий
    QStandardItemModel * event_model;

    /// Указатель на модель отображения результатов поиска по модели настроек событий
    QSortFilterProxyModel * event_model_proxy;

    /** Список структур DEV_INFO, хранящий информацию о подключенных устройствах
      * (в том числе и настройки для устройств, загруженных их файла)
      */
    QList<DEV_INFO> dev_settings;

    /// Текстовый поток для удобства осуществления файловых операций
    QTextStream * log_stream;

    void readSettingNodes(const QDomNode &node);
    QDomElement makeElement(QDomDocument  & dom_doc,
                            const QString & name,
                            const QString & attr,
                            const QString & text);
    QDomElement addTagToDom(QDomDocument dom_doc,
                       const QString & id,
                       const QString & name);
    QDomElement addDevNameToDom(QDomDocument dom_doc,
                                const QString & id,
                                const QString & name);
    QDomElement addDevToDom(QDomDocument dom_doc, const DEV_INFO &id);
    QDomElement addEventToDom(QDomDocument dom_doc, int row);
    void initSetModels();
public slots:
};
#endif // SETTINGS_OBJ_H
