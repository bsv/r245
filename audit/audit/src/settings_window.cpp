#include <QFileDialog>
#include <QDebug>
#include <QSortFilterProxyModel>
#include "settings_window.h"

SettingsWindow::SettingsWindow(SettingsObj * set, Monitor * monitor, QWidget *parent):
    QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    menu = new QMenu(this);
    menu->addAction("�������� ���������", this, SLOT(slotGetDevSettings()));
    menu->addAction("�������� �����", this, SLOT(slotUpdAddr()));
    menu->addAction("������� ����������", this, SLOT(slotDeleteDev()));

    set_menu_tab->setTabEnabled(1, false);

    set_obj = set;
    monitor_obj = monitor;

    block_alias_change = false;

    settings_le->setText(settings.value("/settings/settings_file", "").toString());
    log_le->setText(settings.value("/settings/log_file", "").toString());

    slotOpenSettings(false);
    slotOpenLog(false);

    react_list << "������ �� ������" << "�������� ������" << "�������� ���������";
    event_list << "��������� ����� ���" << "��� �������";
    chanell_list << "1" << "2";

    tag_view->setModel(set_obj->getModel(SettingsObj::TagTypeModelProxy));

    dev_view->setModel(set_obj->getModel(SettingsObj::DevTypeModel));
    dev_view->installEventFilter(this);
    dev_view->setObjectName("dev_view");

    event_view->setModel(set_obj->getModel(SettingsObj::EventTypeModelProxy));
    event_view->hideColumn(SettingsObj::EvIdDev);
    event_view->hideColumn(SettingsObj::EvIdTag);
    event_view->setItemDelegate(new EventDelegate(((DevModel*)dev_view->model())->getDevList(),
                                                  &tag_list, &event_list, &react_list,
                                                  &chanell_list, event_view));

    connect(settings_button, SIGNAL(clicked()), SLOT(slotOpenSettings()));
    connect(log_button, SIGNAL(clicked()), SLOT(slotOpenLog()));
    connect(add_button, SIGNAL(clicked()), SLOT(slotAdd()));
    connect(del_button, SIGNAL(clicked()), SLOT(slotDelete()));
    connect(save_button, SIGNAL(clicked()), SLOT(slotSaveSetings()));
    connect(find_dev_button, SIGNAL(clicked()), SLOT(slotReadDevInfo()));
    connect(cancel_button, SIGNAL(clicked()), SLOT(close()));
    connect(dev_view, SIGNAL(clicked(QModelIndex)), SLOT(slotDevClick(QModelIndex)));
    connect(dist1_dial, SIGNAL(valueChanged(int)), SLOT(slotDist1(int)));
    connect(time1_edt, SIGNAL(timeChanged(QTime)), SLOT(slotTime1()));
    connect(dist2_dial, SIGNAL(valueChanged(int)), SLOT(slotDist2(int)));
    connect(time2_edt, SIGNAL(timeChanged(QTime)), SLOT(slotTime2()));
    connect(ch1_button, SIGNAL(clicked()), SLOT(slotActChannel()));
    connect(ch2_button, SIGNAL(clicked()), SLOT(slotActChannel()));
    connect(find_tag_le, SIGNAL(textChanged(QString)), SLOT(slotFindTag()));
    connect(find_event_le, SIGNAL(textChanged(QString)), SLOT(slotFindEvent()));
    connect(synch_time_button, SIGNAL(clicked()), SLOT(slotSynchTime()));
    connect(new_log_btn, SIGNAL(clicked()), SLOT(slotNewLog()));
    connect(new_settings_btn, SIGNAL(clicked()), SLOT(slotNewSettings()));
    connect(dev_view, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotAddDev(QModelIndex)));

    QStandardItemModel * event_model = (QStandardItemModel*)set_obj->getModel(SettingsObj::EventTypeModel);
    connect(event_model, SIGNAL(itemChanged(QStandardItem*)), SLOT(slotEventDataChanged(QStandardItem*)));

    QStandardItemModel * tag_model = (QStandardItemModel*)set_obj->getModel(SettingsObj::TagTypeModel);
    QStandardItemModel * dev_model = (QStandardItemModel *)set_obj->getModel(SettingsObj::DevTypeModel);

    connect(tag_model, SIGNAL(itemChanged(QStandardItem*)), SLOT(slotAliasChanged(QStandardItem*)));
    connect(dev_model, SIGNAL(itemChanged(QStandardItem*)), SLOT(slotDevDataChanged(QStandardItem*)));
    connect(set_obj, SIGNAL(sigAddReader(QStandardItem*)), SLOT(slotAliasChanged(QStandardItem*)));
}

bool SettingsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->objectName() == "dev_view")
    {
        if (event->type() == QEvent::ContextMenu)
        {
            QModelIndex index = dev_view->selectionModel()->currentIndex();

            if(index.isValid() && isReaderDev(index))
            {
                QMouseEvent * mevent = static_cast<QMouseEvent *>(event);
                menu->exec(mevent->globalPos());
            }
            return true;
        } else
        {
            return false;
        }
    } else
    {
        return QDialog::eventFilter(obj, event);
    }
}

void SettingsWindow::slotUpdAddr()
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(index.isValid() && isReaderDev(index))
    {

        utils.showMessage(QMessageBox::Warning, "��������",
                          "��� ����� ������ �� ����� ������ ���� ��������� ������ ���� �����������");

        unsigned char dev_num = index.parent().row();
        unsigned char addr = index.parent().child(index.row(), 0).data().toInt();

        if(utils.R245_SetAddr(dev_num, addr))
        {
            utils.showMessage(QMessageBox::Warning, "����� ������", "������ ����� ������");
        }
    }
}

void SettingsWindow::slotGetDevSettings()
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(index.isValid() && isReaderDev(index))
    {
        unsigned char dev_num = index.parent().row();
        unsigned char addr = index.parent().child(index.row(), 0).data().toInt();
        ulong id = index.parent().data().toULongLong();


        DEV_INFO * dev = set_obj->getDevSettings(id, addr);

        if(dev != NULL)
        {
            if(set_obj->getReaderSettings(dev_num, dev))
            {
                updateSettings(dev);
            }
        }
    }
}

void SettingsWindow::slotDevDataChanged(QStandardItem * item)
{
    item->model()->blockSignals(true);

    if(item->column() == SettingsObj::Addr)
    {
        bool is_num = false;
        int addr = item->data(Qt::DisplayRole).toInt(&is_num);

        if(!is_num)
        {
            utils.showMessage(QMessageBox::Warning, "��������� ������", "����� ������ ���� ������");
            addr = set_obj->getFreeAddress(item->parent()->row());
            item->setText(QString().setNum(addr));
        } else
        {
            qDebug() << "ADDR" << addr;
            // ������� �������� ������ �� �����������
            /*if(!set_obj->isFreeAddress(item->parent()->row(), addr))
            {
                utils.showMessage(QMessageBox::Warning, "��������� ������", "����� ����� ��� �������.");
                addr = set_obj->getFreeAddress(item->parent()->row());
                item->setText(QString().setNum(addr));
            }*/
        }

    } else if(item->column() == SettingsObj::Name)
    {
        ulong id = item->parent()->data(Qt::DisplayRole).toULongLong();
        int addr = item->parent()->child(item->row())->data(Qt::DisplayRole).toUInt();

        DEV_INFO * dev = set_obj->getDevSettings(id, addr);
        if(dev != NULL)
        {
            dev->name = item->text();
            ((DevModel *)item->model())->changeReader(item->parent()->row(), item->row());
            slotAliasChanged(item);

            utils.changeAlias(item, (QStandardItemModel *) set_obj->getModel(SettingsObj::EventTypeModel), false);
        } else
        {
            item->setText("");
        }
    }

    item->model()->blockSignals(false);
}

/**
  * �������� ���������� �������� ���� dev_view �� ��� �������
  * ������: 2 �����, ������� ���� - ����� ��������� ��������,
  *     ������� ���� - ����� ��������� ��������
  *     (�� ����� 0 ���� index - ��� �������� �������)
  */
short int SettingsWindow::getDevCoord(QModelIndex index)
{
    short int dev_id = 0;

    if(index.parent().isValid())
    {
        dev_id |= index.row() + 1; // 0 - �������� �������
        dev_id |= index.parent().row() << 8;
    } else
    {
        dev_id |= index.row() << 8;
    }

    return dev_id;
}

bool SettingsWindow::isReaderDev(QModelIndex index)
{
    short int dev_id = getDevCoord(index);

    return (dev_id & 0x00FF);
}

void SettingsWindow::slotNewLog()
{

    QString file_path = QFileDialog::getSaveFileName(0, "�������� ����� �������", "", "*.xml");

    if(!file_path.isEmpty())
    {
        log_le->setText(file_path);
        QFile file(file_path);

        if(utils.openFile(&file, QIODevice::WriteOnly))
        {
            QTextStream text_stream(&file);
            text_stream << "<log>\n";
            text_stream << "</log>";
        }

        utils.closeFile(&file);
        set_obj->openLogFile(file_path, monitor_obj);

    } else
    {
        log_le->setText("");
    }
}

void SettingsWindow::slotNewSettings()
{
    QString file_path = QFileDialog::getSaveFileName(0, "�������� ����� ��������", "", "*.xml");

    if(!file_path.isEmpty())
    {
        settings_le->setText(file_path);
        QFile file(file_path);

        if(utils.openFile(&file, QIODevice::WriteOnly))
        {
            QTextStream text_stream(&file);
            text_stream << "<settings>\n";
            text_stream << "</settings>";
        }

        utils.closeFile(&file);
        set_obj->openSettingFile(file_path);
    } else
    {
        settings_le->setText("");
    }
}

void SettingsWindow::slotAliasChanged(QStandardItem *item)
{
    if(item->model()->objectName() == "tag_model")
    {
        if(item->column() == SettingsObj::AliasName)
        {
            tag_list[item->row()] = item->text();
        }
    }

    if(!block_alias_change)
    {
        utils.changeAlias(item, (QStandardItemModel *) monitor_obj->getModel(false), false);
        utils.changeAlias(item, (QStandardItemModel *) set_obj->getModel(SettingsObj::EventTypeModel), false);
    }

    event_view->resizeColumnsToContents();
    tag_view->resizeColumnsToContents();
}

void SettingsWindow::slotEventDataChanged(QStandardItem *item)
{

    if(item->column() == SettingsObj::EvNameDev || item->column() == SettingsObj::EvNameTag)
    {
        int id_attr;
        QString id;

        if(item->column() == SettingsObj::EvNameDev)
        {
            id = set_obj->getDevId(item->text());
            id_attr = SettingsObj::EvIdDev;
        } else
        {
            id = set_obj->getTagId(item->text());
            id_attr = SettingsObj::EvIdTag;
        }

        item->model()->blockSignals(true);
        item->model()->item(item->row(), id_attr)->setText(id);
        item->model()->blockSignals(false);


    }

    event_view->resizeColumnsToContents();
}

void SettingsWindow::slotFindEvent()
{
    set_obj->setFilterWildCard(find_event_le->text() + "*", SettingsObj::EventTypeModelProxy);
}

void SettingsWindow::slotFindTag()
{
    set_obj->setFilterWildCard(find_tag_le->text() + "*", SettingsObj::TagTypeModelProxy);
}

void SettingsWindow::slotSynchTime()
{
    /*R245_RTC rtc_data;
    QAbstractItemModel * model = set_obj->getModel(SettingsObj::DevModel);
    short int dev_count = model->rowCount();

    for(int dev_num = 0; dev_num < dev_count; dev_num++)
    {
        DEV_INFO * dev = set_obj->getDevSettings(model->data(model->index(dev_num, SettingsObj::Id)).toInt());
        QDateTime clock = QDateTime::currentDateTime();

        rtc_data.hour = clock.time().hour();
        rtc_data.min = clock.time().minute();
        rtc_data.sec = clock.time().second();
        rtc_data.dow = clock.date().dayOfWeek();
        rtc_data.year = clock.date().year();
        rtc_data.month = clock.date().month();
        rtc_data.day = clock.date().day();

        if(!dev->active)
            utils.R245_InitDev(dev_num);

        utils.R245_SetTimeRTC(dev_num, 1, &rtc_data);
        utils.R245_SetDateRTC(dev_num, 1, &rtc_data);

        if(!dev->active)
            utils.R245_CloseDev(dev_num);
    }*/
}

void SettingsWindow::slotActChannel()
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(!(index.isValid() && (isReaderDev(index))))
    {
        ch1_button->setChecked(false);
        ch2_button->setChecked(false);
    }
}

void SettingsWindow::slotSaveSetings()
{
    if(dev_tab->isVisible())
    {
        QModelIndex index = dev_view->selectionModel()->currentIndex();

        if(index.isValid() && isReaderDev(index))
        {

            int row = index.parent().row();
            int addr = index.parent().child(index.row(), 0).data().toInt();

            unsigned char channel = 0;
            unsigned short time1 = utils.timeToSec(time1_edt->time());
            unsigned short time2 = utils.timeToSec(time2_edt->time());
            unsigned char dist1 = dist1_le->text().toInt();
            unsigned char dist2 = dist2_le->text().toInt();

            QStandardItemModel * model = (QStandardItemModel *)set_obj->getModel(SettingsObj::DevTypeModel);
            ulong id = model->item(row)->data(Qt::DisplayRole).toULongLong();

            DEV_INFO * dev = set_obj->getDevSettings(id, addr);

            if(dev == NULL)
            {
                qDebug() << "DevNULL";
                return;
            }

            if(ch1_button->isChecked())
                channel |= CHANNEL_ACT_1;
            if(ch2_button->isChecked())
                channel |= CHANNEL_ACT_2;
            qDebug() << "Channel" << channel;

            if(dev->channel != channel)
            {
                if(set_obj->setChannelDev(row, dev->addr, channel) != R245_OK)
                {
                    utils.showMessage(QMessageBox::Warning,
                                      "��������� �������",
                                      "���������� �������� ��������� �������");
                }
            }
            if(dev->time1 != time1)
            {
                if(set_obj->setTimeDev(row, dev->addr, time1, true) != R245_OK)
                {
                    utils.showMessage(QMessageBox::Warning,
                                      "��������� ������� �������",
                                      "���������� �������� ��������� ������� ������� ��� ������� ������");
                }
            }
            if(dev->time2 != time2)
            {
                if(set_obj->setTimeDev(row, dev->addr, time2, false) != R245_OK)
                {
                    utils.showMessage(QMessageBox::Warning,
                                      "��������� ������� �������",
                                      "���������� �������� ��������� ������� ������� ��� ������� ������");
                }
            }
            if(dev->dist1 != dist1)
            {
                if(set_obj->setDistDev(row, dev->addr, dist1, true) != R245_OK)
                {
                    utils.showMessage(QMessageBox::Warning,
                                      "��������� ��������� ����������",
                                      "���������� �������� ��������� ��������� ���������� ��� ������� ������");
                }
            }
            if(dev->dist2 != dist2)
            {
                if(set_obj->setDistDev(row, dev->addr, dist2, false) != R245_OK)
                {
                    utils.showMessage(QMessageBox::Warning,
                                      "��������� ��������� ����������",
                                      "���������� �������� ��������� ��������� ���������� ��� ������� ������");
                }
            }
        }
    } else
    {
        set_obj->saveSetings();
    }

}

void SettingsWindow::slotDist1(int value)
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(index.isValid() && isReaderDev(index))
    {
        dist1_le->setText(QString().setNum(value));
    } else
    {
        dist1_le->setText(QString().setNum(0));
        dist1_dial->setValue(0);
    }
}

void SettingsWindow::slotTime1()
{

    qDebug() << "slot time 1";
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(!(index.isValid() && isReaderDev(index)))
    {
        time1_edt->setTime(QTime().fromString(""));
    }
}

void SettingsWindow::slotDist2(int value)
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    if(index.isValid() && isReaderDev(index))
    {
        dist2_le->setText(QString().setNum(value));
    } else
    {
        dist2_le->setText(QString().setNum(0));
        dist2_dial->setValue(0);
    }
}

void SettingsWindow::slotTime2()
{
    QModelIndex index = dev_view->selectionModel()->currentIndex();;

    if(!(index.isValid() && isReaderDev(index)))
    {
        time2_edt->setTime(QTime().fromString("00:00:00"));
    }
}

void SettingsWindow::slotReadDevInfo()
{
    set_obj->readDevInfo();
}

void SettingsWindow::slotAddDev(QModelIndex index)
{
    int row = index.row();

    // ���� ������� �������� ������� ������ (���������� �����. �����������)
    if((row != -1) && !isReaderDev(index))
    {
        dev_view->collapse(index);
        set_obj->addReaderToModel(row);
    }
}

void SettingsWindow::slotDeleteDev()
{   
    QModelIndex index = dev_view->selectionModel()->currentIndex();

    // ���� ������ �������� ������� (���������� �����������)
    if(index.isValid() && isReaderDev(index))
    {
        set_obj->deleteReaderFromModel(index.parent().row(), index.row());
    }
}

/**
  ���������� �������� ����������� � ��������� ����������
*/

void SettingsWindow::updateSettings(DEV_INFO * dev)
{
    time1_edt->setTime(utils.secToTime(dev->time1));
    time2_edt->setTime(utils.secToTime(dev->time2));
    dist1_dial->setValue(dev->dist1);
    dist2_dial->setValue(dev->dist2);

    if(dev->channel & CHANNEL_ACT_1)
        ch1_button->setChecked(true);
    else
        ch1_button->setChecked(false);

    if(dev->channel & CHANNEL_ACT_2)
        ch2_button->setChecked(true);
    else
        ch2_button->setChecked(false);
}

void SettingsWindow::slotDevClick(QModelIndex qmi)
{
    if(isReaderDev(qmi))
    {
        short int dev_coord = getDevCoord(qmi);
        int row = (dev_coord & 0xFF00) >> 8;
        int dev_row = (dev_coord & 0x00FF) - 1;

        QStandardItemModel * model = (QStandardItemModel *) set_obj->getModel(SettingsObj::DevTypeModel);

        uint addr = model->item(row)->child(dev_row)->data(Qt::DisplayRole).toUInt();

        DEV_INFO * dev = set_obj->getDevSettings(model->index(row, 0).data().toULongLong(), addr);

        if(dev != NULL)
        {
            updateSettings(dev);
        }
    } else
    {
        time1_edt->setTime(QTime().fromString("00:00:00"));
        time2_edt->setTime(QTime().fromString("00:00:00"));
        ch1_button->setChecked(false);
        ch2_button->setChecked(false);
        dist1_le->setText("0");
        dist1_dial->setValue(0);
        dist2_le->setText("0");
        dist2_dial->setValue(0);
    }
}

void SettingsWindow::slotAdd()
{
    if(tag_tab->isVisible())
    {
        tag_list.push_front("");
        find_tag_le->setText("");
        set_obj->addTagToModel();

    } else if(event_tab->isVisible())
    {
        find_event_le->setText("");
        set_obj->addEventToModel();
    }
}

void SettingsWindow::slotDelete()
{
    QTableView * table_view = NULL;
    SettingsObj::TypeModel type_model;

    if(tag_tab->isVisible())
    {
        table_view = tag_view;
        type_model = SettingsObj::TagTypeModel;
    } else if(event_tab->isVisible())
    {
        table_view = event_view;
        type_model = SettingsObj::EventTypeModel;
    }

    int row = table_view->selectionModel()->currentIndex().row();

    if(row > -1)
    {
        // column = 0 - ��� ������� ���� ��� �����. � utils.changeAlias �� ���� ������������ ��� ������ ������.
        QStandardItem * item = ((QStandardItemModel *)set_obj->getModel(type_model))->item(row, 0);

        if(type_model == SettingsObj::TagTypeModel)
        {
            tag_list.removeAt(item->row());
        }

        utils.changeAlias(item, (QStandardItemModel *) monitor_obj->getModel(false), true);
        utils.changeAlias(item, (QStandardItemModel *) set_obj->getModel(SettingsObj::EventTypeModel), true);

        set_obj->getModel(type_model)->removeRow(row);
    }
}

void SettingsWindow::slotOpenSettings(bool dialog)
{
    if(dialog)
        openFile(settings_le, "�������� ���� ��������");

    block_alias_change = true;
    if(settings_le->text() != "")
    {
        if(set_obj->openSettingFile(settings_le->text()))
        {
            event_view->hideColumn(SettingsObj::EvIdDev);
            event_view->hideColumn(SettingsObj::EvIdTag);
            set_menu_tab->setTabEnabled(1, true);

            monitor_obj->updateAlias(set_obj);
            block_alias_change = false;
            return;
        }
    }
    set_menu_tab->setTabEnabled(1, false);
    block_alias_change = false;

}

void SettingsWindow::slotOpenLog(bool dialog)
{
    if(dialog)
        openFile(log_le, "�������� ���� �������");
    if(log_le->text() != "")
    {
        set_obj->openLogFile(log_le->text(), monitor_obj);
        monitor_obj->update();
    }

    monitor_obj->updateAlias(set_obj);
}

void SettingsWindow::openFile(QLineEdit * le, QString caption)
{
    QString file_path = QFileDialog::getOpenFileName(0, caption);

    if(!file_path.isEmpty())
    {
        le->setText(file_path);
    } else
    {
        le->setText("");
    }
}

SettingsWindow::~SettingsWindow()
{
    settings.setValue("/settings/settings_file", settings_le->text());
    settings.setValue("/settings/log_file", log_le->text());
}
