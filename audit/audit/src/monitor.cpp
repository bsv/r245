#include "monitor.h"
#include "settings_obj.h"

Monitor::Monitor()
{

    monitor_model = new QStandardItemModel();
    monitor_model->setObjectName("monitor_model");

    initHeader();

    monitor_model_proxy = new MonitorFilter();
    monitor_model_proxy->setSourceModel(monitor_model);
    initMas();
}

/// ������������� �������������� ��������� ��������� ������ monitor_model.
void Monitor::initHeader()
{
    QStringList header;

    header << "�����" << "����" << "��� ����������" << "id ����������" << "�����" << "��� �����" << "id �����" << "��� �������" << "��� �������";
    monitor_model->setHorizontalHeaderLabels(header);
}

/**
  * ������� ������ ��������.
  */
void Monitor::clear()
{
    monitor_model->clear();
    initHeader();
}

/**
  * ������� ���������� ���������� � ����� ��������� ������� ���������.
  *
  * @return QMap <int, QString> * - ������ �� ����� state, ������� ������ �����������
  * ���� ������� � ��� �������� ��������.
  */
QMap <int, QString> * Monitor::getState()
{
    return &state;
}

/// �������������� ����� state.
void Monitor::initMas()
{
    state[0x01]  = "������� ����������� ��������";
    state[0x02]  = "������� ����������� ���������";
    state[0x10]  = "��������� ����� ���";
    state[0x11]  = "��� �������";
    state[0x20]  = "�������� �����������";
    state[0x21]  = "����������� ������������";
    state[0x22]  = "�������� ������ ������� ����";
    state[0x23]  = "������ ������� ���� ������������";
    state[0x24]  = "��������� ��������� � ����";
    state[0x25]  = "��������� ���� �������������";
    state[0x101] = "�������� ������ ����������� ����������";
    state[0x102] = "������ ����������� ���������� ������������";
    state[0x103] = "�������� ������ ����� (���������)";
    state[0x104] = "������ ����� (���������) ������������";
    state[0x121] = "������� ��� ������� �� ��������� � ������ ������";
    state[0x122] = "���������� ������������";
    state[0x123] = "���������� �������������";
    state[0x124] = "����� ���������� � ����������� �������";
    state[0x125] = "���� ������������ ������� ����������� ����������";
    state[0x126] = "���������� �������� � ���� �������";
    state[0x127] = "���������� �� �������� � ���� �������";
    state[0x128] = "����� �����, ������ �� ���������";
    state[0x129] = "������� ���������� �����";
    state[0x12A] = "���� ������� ��������, �������� ���������";
    state[0x12B] = "�������� ������� �� ����������� �������";
    state[0x12C] = "�������� ������� �������� �����";
    state[0x12D] = "������ ���������";
    state[0x12E] = "������ �� ��������� � ������������� �����";
    state[0x12F] = "�������� ����������� �� �������: ��� �������";
    state[0x130] = "�������� ����������� �� �������: ��������� ����� ���";
}

/**
  * ������� ��������� ���������� � ������� � ������ ��������.
  *
  * @param dev_num - �������� ������������� ����������
  * @param trans - ������ �� ���������, �������� ���������� � �������
  * @param tag_name - ���������� ������������� �����
  * @param dev_name - ���������� ������������� ����������
  */
void Monitor::addTransToModel(QString dev_num, R245_TRANSACT * trans, const QString &tag_name, const QString &dev_name)
{
        int row = 0/*monitor_model->rowCount()*/;

        monitor_model->insertRow(row);
        monitor_model->setItem(row, TypeEventAttr, new QStandardItem(QString("%1").arg(state[trans->code])));
        monitor_model->setItem(row, TransCodeAttr, new QStandardItem(QString().setNum(trans->code)));
        monitor_model->setItem(row, ChAttr, new QStandardItem(QString("%1").arg(trans->channel)));

        if(tag_name == "")
        {
            monitor_model->setItem(row, TagNameAttr, new QStandardItem(QString().setNum(trans->tid)));
        }
        else
        {
            monitor_model->setItem(row, TagNameAttr, new QStandardItem(tag_name));
        }
        monitor_model->setItem(row, TagIdAttr, new QStandardItem(QString().setNum(trans->tid)));

        if(dev_name != "")
        {
            monitor_model->setItem(row, DevNameAttr, new QStandardItem(dev_name));
        }
        else
        {
            monitor_model->setItem(row, DevNameAttr, new QStandardItem(dev_num));
        }
        monitor_model->setItem(row, DevNumAttr, new QStandardItem(dev_num));

        monitor_model->setItem(row, DateAttr, new QStandardItem(QDate(trans->year, trans->month, trans->day).toString(Qt::LocalDate)));
        monitor_model->setItem(row, TimeAttr, new QStandardItem(QTime(trans->hour, trans->min, trans->sec).toString()));
}

/**
  * ������� ������������� ��������� ���������� ������ ������ ��������.
  * ��������� ���������� ��������������� ������ monitor_model_proxy.
  *
  * @param channel - ���������� ��������� ��� ���������� �������, �������������� �������.
  * @param device - ���������� ��������� ��� ���������� ��������� �� ����������� � ��������� ���������������.
  * @param tag - ���������� ��������� ��� ���������� ����� �� ����������� � ��������� ���������������
  * @param daten - ������ ������� ����.
  * @param datem - ������� ������� ����.
  * @param timen - ������ ������� �������.
  * @param timem - ������� ������� �������.
  */
void Monitor::setFilter(QString channel, QString device, QString tag, QString event, QDate daten, QDate datem,
                        QTime timen, QTime timem)
{
    monitor_model_proxy->setRegExp(QRegExp(channel), QRegExp(device), QRegExp(tag), QRegExp(event));
    monitor_model_proxy->setFilterMinimumDate(daten);
    monitor_model_proxy->setFilterMaximumDate(datem);
    monitor_model_proxy->setFilterMinimumTime(timen);
    monitor_model_proxy->setFilterMaximumTime(timem);

    update();
}

/**
  * ��������� ������ �������� � ������������ � ��������������
  * ����������� ����������.
  */
void Monitor::update()
{
    monitor_model_proxy->setFilterRegExp(QRegExp(""));
}

/**
  * ������� ��������� ������ ����� ������ ������� � ��������.
  *
  * @param only - ���� true, �� monitor_model_proxy ���������� �������, ��������� � �������,
  *     ���� false, �� ������������ ��� �������.
  */
void Monitor::onlyTagInf(bool only)
{
    if(only)
    {
        monitor_model_proxy->setTransCodeRE("16|17");
    } else
    {
        monitor_model_proxy->setTransCodeRE("");
    }
    update();
}

/**
  * ������� ���������� ��������� �� ������,
  * ������� �������� ��������� ������� ������.
  *
  * @param proxy - ���� true, �� ������������ ��������� �� ������ monitor_model_proxy,
  *     ���� false, �� ����������� ��������� �� monitor_model.
  * @return QAbstractItemModel * - ��������� �� ������.
  */
QAbstractItemModel * Monitor::getModel(bool proxy)
{
    if(proxy)
        return monitor_model_proxy;
    else
        return monitor_model;
}

/**
  * ��������� �������� ����������� ��� ����� � ���������.
  *
  * @param tag_model - ������ �� ������, �������� ������ � ����������� �����.
  * @param dev_name_model - ������ �� ������, �������� ������ � ����������� ���������.
  */
void Monitor::updateAlias(SettingsObj * set_obj)
{
    QStandardItem * dev_item, * tag_item;

    int row_count = monitor_model->rowCount();

    for(int row = 0; row < row_count; row++)
    {
        dev_item = monitor_model->item(row, DevNumAttr);
        tag_item = monitor_model->item(row, TagIdAttr);

        QString tag_name = "", dev_name = "";

        set_obj->findTagAlias(tag_item->text(), &tag_name);
        set_obj->findDevAlias(dev_item->text(), &dev_name);

        if(tag_name != "")
        {
            monitor_model->item(row, TagNameAttr)->setText(tag_name);
        } else
        {
            monitor_model->item(row, TagNameAttr)->setText(tag_item->text());
        }

        if(dev_name != "")
        {
            monitor_model->item(row, DevNameAttr)->setText(dev_name);
        } else
        {
            monitor_model->item(row, DevNameAttr)->setText(dev_item->text());
        }
    }
}

Monitor::~Monitor()
{
    delete monitor_model;
}
