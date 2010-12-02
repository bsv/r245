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

/** @name ��������� ��� ��������� �������
  * ���� ��� ������ �������, �� ���� HEAD_DEV_INFO_CHANNEL
  * � dev_model ����� �������� (CHANNEL_ACT_1 | CHANNEL_ACT_2)
  * @{
  */
#define CHANNEL_ACT_1 1
#define CHANNEL_ACT_2 2
/** @}*/

/**
  * ����� �������� ����������� ��������� ���
  * �������������� � ����������� �  �����������������
  * ������� ���������
  */
class SettingsObj : public QObject
{
    Q_OBJECT
public:

    /**
      * �������� ������������ �������������
      * ��������������� �������, ������� �������� �����
      */
    enum TypeModel {
        TagModel,
        DevModel,
        EventModel,
        TagModelProxy,
        EventModelProxy
    };

    /**
      * �������� ������������ �������������
      * ������� ������� � ������� �������� ���������
      * ��� �������� ����� � ��������� (tag_model, dev_name_model)
      */
    enum AliasTable
    {
        AliasId,
        AliasName
    };

    /**
      * �������� ������������ �������������
      * ������� ������� � ������ dev_model
      */
    enum DevInfoAttr
    {
        Id,
        Desc
    };

    enum ReaderInfo
    {
        Addr,
        Name
    };

    /**
      * �������� ������������ �������������
      * ������� ������� � ������ event_model
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

    /** @name ������������/�����������
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
    void addDevInfoToModel(R245_DEV_INFO * info);
    short int setActiveDev(int row, bool active);
    short int setAuditEn(int row, unsigned char addr, bool active);
    short int setChannelDev(int row, unsigned char addr, short int channel);
    short int setTimeDev(int row, unsigned char addr, short int time, bool time1);
    short int setDistDev(int row, unsigned char addr, unsigned char dist, bool dist1);
    DEV_INFO * getDevSettings(ulong id, unsigned char addr);
    void readDevInfo();
    void saveSetings();
    void addLogNode(QString dev_num, R245_TRANSACT * trans);
    void addReaderToModel(unsigned char dev_num, unsigned char addr = 0, QString name = "��� �����");
    void deleteReaderFromModel(int dev_num, int reader_num);
    bool getReaderSettings(unsigned char dev_num, DEV_INFO * dev);
    unsigned char getFreeAddress(unsigned char dev_num);
    bool isFreeAddress(unsigned char dev_num, unsigned char addr);
private:

    /// ��������� �� ���� ��������
    QFile * fsettings;

    /// ��������� �� ���� �������
    QFile * flog;

    /// ��������� �� ������ ��������� �����
    QStandardItemModel * tag_model;

    /// ��������� �� ������ ����������� ����������� ������ �� ������
    QSortFilterProxyModel * tag_model_proxy;

    /// ��������� �� ������ ��������� ��� ���������
    QStandardItemModel * dev_name_model;

    /// ��������� �� ������ ����������� ����������� ������ �� �����������
    QSortFilterProxyModel * dev_name_model_proxy;

    /// ��������� �� ������, �������� ���������� � �����������
    QStandardItemModel * dev_model;

    /// ��������� �� ������, �������� ��������� �������
    QStandardItemModel * event_model;

    /// ��������� �� ������ ����������� ����������� ������ �� ������ �������� �������
    QSortFilterProxyModel * event_model_proxy;

    /** ������ �������� DEV_INFO, �������� ���������� � ������������ �����������
      * (� ��� ����� � ��������� ��� ���������, ����������� �� �����)
      */
    QMap <ulong, QList<DEV_INFO> *> dev_settings;

    /// ��������� ����� ��� �������� ������������� �������� ��������
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
    QDomElement addDevToDom(QDomDocument dom_doc, ulong id);
    QDomElement addEventToDom(QDomDocument dom_doc, int row);
    void initSetModels();

public slots:
signals:
        void sigAddReader(QStandardItem *);
};
#endif // SETTINGS_OBJ_H
