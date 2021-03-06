#include "settings_obj.h"
#include "global.h"

SettingsObj::SettingsObj()
{

    fsettings = NULL;
    flog = NULL;
    flog_backup = NULL;

    buf_logging = false;

    tag_model = new QStandardItemModel();
    tag_model->setObjectName("tag_model");

    event_model = new QStandardItemModel();

    initSetModels();

    tag_model_proxy = new QSortFilterProxyModel();
    tag_model_proxy->setSourceModel(tag_model);
    tag_model_proxy->setFilterKeyColumn(-1); // filter all column
    tag_model_proxy->setFilterWildcard("*");

    event_model_proxy = new QSortFilterProxyModel();
    event_model_proxy->setSourceModel(event_model);
    event_model_proxy->setFilterKeyColumn(-1);
    event_model_proxy->setFilterWildcard("*");

    dev_model = new DevModel();
    dev_model->setObjectName("dev_model");

    connect(&timer_save_log, SIGNAL(timeout()), SLOT(slotSaveBufLog()));
    timer_save_log.setInterval(1000);
}

/**
  * ������� ������������� �������, �������� ������ � ����������
  */
void SettingsObj::initSetModels()
{
    tag_model->clear();
    event_model->clear();

    QStringList tag_header;
    tag_header << "id" << "���";
    tag_model->setColumnCount(tag_header.count());
    tag_model->setHorizontalHeaderLabels(tag_header);

    QStringList event_header;
    event_header << "id/��� ����������" <<
            "���" << "�����" << "id/��� �����" << "�������" <<
            "�������" << "id ����������" << "id �����";

    event_model->setColumnCount(event_header.count());
    event_model->setHorizontalHeaderLabels(event_header);

    dev_settings.clear();
}

/**
  * ��������� ���������� ������ ��� �������
  *
  * @param ex - ���������� ��������� ��� ������
  * @param type_model - ��� ������, �� ������� ���������� ����������� �����
  */
void SettingsObj::setFilterWildCard(QString ex, TypeModel type_model)
{
    switch(type_model)
    {
        case TagTypeModelProxy:
            tag_model_proxy->setFilterWildcard(ex);
            break;
        case EventTypeModelProxy:
            event_model_proxy->setFilterWildcard(ex);
            break;
        default:
            break;
    }
}

/**
  * �������� ����� ��������
  * @param file_name - ���� � ����� ��������
  *
  * @return true � ������ ������� �������� ��������
  */
bool SettingsObj::openSettingFile(QString file_name)
{
    if(utils.closeFile(fsettings))
    {
        delete fsettings;
        fsettings = NULL;
    }
    qDebug("Open Settings");
    fsettings = new QFile(file_name);

    initSetModels();

    QDomDocument dom_doc;

    if(utils.openFile(fsettings, QIODevice::ReadOnly))
    {
        QTextStream ts(fsettings);
        ts.setCodec("Windows-1251");

        if(dom_doc.setContent(ts.readAll()/*fsettings*/))
        {
            QDomElement dom_el = dom_doc.documentElement();
            readSettingNodes(dom_el);
        }
        utils.closeFile(fsettings);
        return true;
    }

    return false;
}

/**
  * �������� ����� �������
  * @param file_name - ���� � ����� �������
  *
  * @return true � ������ ������� �������� ��������
  */
bool SettingsObj::openLogFile(QString file_name, Monitor *monitor)
{
    if(utils.closeFile(flog))
    {
        delete flog;
        flog = NULL;

        delete flog_backup;
        flog_backup = NULL;
    }
    qDebug("Open Log");

    flog = new QFile(file_name);
    flog_backup = new QFile(file_name + "~");

    if(!flog_backup->exists())
    {
        // ������� ������ ����, ���� ��� �� ������
        utils.openFile(flog_backup, QIODevice::WriteOnly);
        utils.closeFile(flog_backup);
    } else
    {
        if(flog->size() != flog_backup->size())
        {
            bool ok = false;
            ok = flog->remove() & flog_backup->copy(file_name);

            if(!ok)
            {
                QString mes = "���� ������� ���������.\n���������� ����������� � ������� ����:\n";
                mes += file_name + "~\n� ����:\n" + file_name;

                utils.showMessage(QMessageBox::Warning,
                                  "�������� �������", mes);
                return false;
            }
        }
    }

    if(utils.openFile(flog_backup, QIODevice::ReadOnly))
    {

        flog_backup->readLine(); // ������ ������ ������ � ������ �� ������
                          // ������ ������ - ��� ��������� ������

        QStringList str_list;
        QStandardItemModel * model = (QStandardItemModel *)monitor->getModel(false);

        while(!flog_backup->atEnd())
        {
            str_list = QString(flog_backup->readLine()).split(';');
            QList <QStandardItem *> items;
            //QString tag_name = "";
            //QString dev_name = "";

            for(int column = 0; column <= Monitor::TransCodeAttr; column++)
            {
                items << new QStandardItem(str_list[column]);
            }

            model->appendRow(items);
        }

        utils.closeFile(flog_backup);
    } else
    {
        utils.showMessage(QMessageBox::Warning,
                          "�������� �������",
                          "���� ������� ���������");
        return false;
    }

    return true;
}

void SettingsObj::findTagAlias(QString find_val, QString * alias)
{
    *alias = "";

    for(int i = 0; i < tag_model->rowCount(); ++i)
    {
        if(tag_model->index(i, SettingsObj::AliasId).data().toString() == find_val)
        {
            *alias = tag_model->index(i, SettingsObj::AliasName).data().toString();
            break;
        }
    }
}

QString SettingsObj::getTagId(QString name)
{
    for(int row = 0; row < tag_model->rowCount(); row++)
    {
        if(tag_model->item(row, AliasName)->text() == name)
        {
            return tag_model->item(row, AliasId)->text();
        }
    }

    return "";
}

QString SettingsObj::getDevId(QString name)
{
    for(int row_dev = 0; row_dev < dev_model->rowCount(); row_dev++)
    {
        for(int row_reader = 0; row_reader < dev_model->item(row_dev)->rowCount(); row_reader++)
        {
            if(dev_model->item(row_dev)->child(row_reader, AliasName)->text() == name)
            {
                return dev_model->item(row_dev)->text() + " " +
                        dev_model->item(row_dev)->child(row_reader, AliasId)->text();
            }
        }
    }

    return "";
}

void SettingsObj::findDevAlias(QString find_val, QString * alias)
{
    *alias = "";

    for(int i = 0; i < dev_model->rowCount(); ++i)
    {
        for(int j = 0; j < dev_model->item(i)->rowCount(); j++)
        {
            QString id = dev_model->item(i)->data(Qt::DisplayRole).toString() + " " +
                         dev_model->item(i)->child(j, SettingsObj::AliasId)->data(Qt::DisplayRole).toString();
            if(id == find_val)
            {
                *alias = dev_model->item(i)->child(j, SettingsObj::AliasName)->data(Qt::DisplayRole).toString();
                return;
            }
        }
    }
}

/**
  * ���������� ������ � ���� �������
  *
  * @param dev_num - ������������� ����������
  * @param trans - ������ �� ���������,
  *   ���������� ���������� � ������������ �������, �������������� � �������
  */
void SettingsObj::addLastTransToLog(QStandardItemModel * model)
{
    if((flog != NULL) && (flog_backup != NULL))
    {
        if(!utils.openFile(flog_backup, QIODevice::Append))
        {
            utils.showMessage(QMessageBox::Warning,
                    "���������� ������", "������ �������� ����� �������");
            return;
        }

        QTextStream log_stream(flog_backup);
        QString value = "";
        QString data = "";

        if(flog_backup->size() == 0)
        {
            // write table header
            for(int column = 0; column <= Monitor::TransCodeAttr; column++)
            {
                data += model->headerData(column, Qt::Horizontal).toString() + ";";
            }

            data += "\n";
        }

        log_stream << data;

        if(flog->size() == 0)
        {
            saveDataToLog2(data);
        }
        data = "";

        int row = 0; // ��� ��� ��������� ���������� ������������ ������

        for(int column = 0; column <= Monitor::TransCodeAttr; column++)
        {
            qApp->processEvents();
            value = model->index(row, column).data().toString();

            // ������� ��������� ������� �� ������
            // ����� ����������� ��� ������ �� ������ ������
            value.remove("\n");
            value.remove("\r");
            data += value + ";";
        }
        data += "\n";
        log_stream << data;
        saveDataToLog2(data);
        utils.closeFile(flog_backup);
    }
}

/**
 * ��������� ������ �� ������ (�����������) ���� �������
 */
void SettingsObj::saveDataToLog2(QString & data)
{
    buf_logging = true;
    if(flog != NULL)
    {
        if(utils.openFile(flog, QIODevice::Append))
        {
            if(buf_log.isEmpty())
            {
                timer_save_log.stop();

                QTextStream log(flog);
                log << data;
            } else
            {
                // ��������� ������� �����,
                // ������� ������������ � ����������� ���� �������
                // � �� �������, ����� ��� ���������� �� ������

                buf_log.append(data);
            }
            utils.closeFile(flog);
        } else
        {
            buf_log.append(data);

            if(!timer_save_log.isActive())
            {
                timer_save_log.start();
            }
        }

    }
    buf_logging = false;
}

void SettingsObj::slotSaveBufLog()
{
    if(!buf_logging)
    {
        if(flog != NULL)
        {
            if(utils.openFile(flog, QIODevice::Append))
            {
                QTextStream log(flog);

                if(!buf_log.isEmpty())
                {
                    for(int i = 0; i < buf_log.size(); i++)
                    {
                        log << buf_log[i];
                    }
                    buf_log.clear();
                }
                utils.closeFile(flog);
            }
        }
    }
}

/**
  * ������� �������� ���������� � ������������ �����������.
  * ���������� ���������� ���������� � ������ dev_model
  */
void SettingsObj::readDevInfo()
{
    R245_DEV_INFO info;
    short int dev_ctr = 0;
    ulong crc_id = 0;
    uchar dbyte;
    QString snumber;
    QStringList * dev_list = utils.getDevList();

    dev_model->clear();
    utils.R245_CloseAllDev();

    QStringList dev_header;
    dev_header << "id" << "��������";
    dev_model->setColumnCount(dev_header.count());
    dev_model->setHorizontalHeaderLabels(dev_header);

    //TEST
    /*short int static ctr = 0;
    info.desc[0] = 'T';
    info.desc[1] = '\0';
    info.id = ctr++;
    info.loc_id = 2;
    info.type = 5;
    addDevInfoToModel(&info);*/
    //TEST

    while(!utils.R245_GetDevInfo(dev_ctr, &info))
    {

        crc_id = 0xFFFF;

        for(int i = 0; i < 16; i++)
        {
            snumber = QString().setNum((uchar)info.serial_number[i], 16);

            for(int j = 0; j < snumber.size(); j++)
            {
                dbyte = snumber[j].toAscii();
                crc_id = utils.crc16(&dbyte, 1, POLYNOM, crc_id);
            }
        }

        if(dev_list->indexOf(QString().setNum(crc_id, 16)) != -1)
        {
            qDebug() << "OK CRC";
            info.id = crc_id; // info->id ����� ���� ��� ������ ������������ ����
                               // ����������, ������� �������� �� crc(info->serial_number)
            addDevInfoToModel(&info);
        }
        dev_ctr++;
    }
}

/**
  * ���������� �������� �� �����. ���������� � ���������� ����������
  * � ��������������� ������ tag_model, dev_name_model, event_model.
  * ���������� � ���������� ��������� ����������� � ������ dev_settings.
  *
  * @param node - ������ �� ������� � ��������� xml, ���������� ���������
  */
void SettingsObj::readSettingNodes(const QDomNode &node)
{
    QDomNode dom_node = node.firstChild();
    while(!dom_node.isNull())
    {
        if(dom_node.isElement())
        {
            QDomElement dom_el = dom_node.toElement();
            if(!dom_el.isNull())
            {
                if(dom_el.tagName() == "tag")
                {
                    QDomElement child_el = dom_node.firstChildElement();
                    addTagToModel(dom_el.attribute("id", ""), child_el.text());
                } else if(dom_el.tagName() == "root_dev")
                {
                    ulong id = dom_el.attribute("id", "").toULong();
                    QList<DEV_INFO> * dev_list = new QList<DEV_INFO>;

                    QDomElement dev_el = dom_node.firstChildElement();
                    while(!dev_el.isNull())
                    {
                        if(dev_el.tagName() == "dev")
                        {
                            DEV_INFO dev;

                            dev.addr = dev_el.attribute("id", "").toInt();

                            QDomElement child_el = dev_el.firstChildElement();
                            while(!child_el.isNull())
                            {
                                if(child_el.tagName() == "channel")
                                {
                                    dev.channel = child_el.text().toInt();
                                } else if(child_el.tagName() == "active")
                                {
                                    dev.active = child_el.text().toInt();
                                } else if(child_el.tagName() == "dist1")
                                {
                                    dev.dist1 = child_el.text().toInt();
                                } else if(child_el.tagName() == "time1")
                                {
                                    dev.time1 = child_el.text().toInt();
                                } else if(child_el.tagName() == "dist2")
                                {
                                    dev.dist2 = child_el.text().toInt();
                                } else if(child_el.tagName() == "time2")
                                {
                                    dev.time2 = child_el.text().toInt();
                                } else if(child_el.tagName() == "name")
                                {
                                    dev.name = child_el.text();
                                }
                                child_el = child_el.nextSiblingElement();
                            }
                            dev_list->append(dev);
                        }
                        dev_el = dev_el.nextSiblingElement();
                    }

                    dev_settings[id] = dev_list;

                } else if(dom_el.tagName() == "event_node")
                {
                    QString id_dev  = "";
                    QString name    = "";
                    QString channel = "";
                    QString id_tag  = "";
                    QString event   = "";
                    QString react   = "";
                    QString color   = "";

                    int r = 0, g = 0, b = 0;

                    QDomElement child_el = dom_node.firstChildElement();
                    while(!child_el.isNull())
                    {
                        if(child_el.tagName() == "id_dev")
                        {
                            id_dev = child_el.text();
                        } else if(child_el.tagName() == "name")
                        {
                            name = child_el.text();
                        } else if(child_el.tagName() == "channel")
                        {
                            channel = child_el.text();
                        } else if(child_el.tagName() == "id_tag")
                        {
                            id_tag = child_el.text();
                        } else if(child_el.tagName() == "event")
                        {
                            event = child_el.text();
                        } else if(child_el.tagName() == "react")
                        {
                            react = child_el.text();
                        } else if(child_el.tagName() == "red")
                        {
                            r = child_el.text().toInt();
                        } else if(child_el.tagName() == "green")
                        {
                            g = child_el.text().toInt();
                        } else if(child_el.tagName() == "blue")
                        {
                            b = child_el.text().toInt();
                        }

                        child_el = child_el.nextSiblingElement();
                    }
                    addEventToModel(id_dev, name, channel, id_tag, event, react, r, g, b);
                }
            }
        }
        readSettingNodes(dom_node);
        dom_node = dom_node.nextSibling();
    }
}

/**
  * ��������� ������ �� ������
  *
  * @param type_model - ��� ������������� ������
  *
  * @return ���������� ��������� �� ������������� ������
  */
QAbstractItemModel * SettingsObj::getModel(TypeModel type_model)
{
    switch(type_model)
    {
        case TagTypeModel:
            return tag_model;
        case DevTypeModel:
            return dev_model;
        case EventTypeModel:
            return event_model;
        case TagTypeModelProxy:
            return tag_model_proxy;
        case EventTypeModelProxy:
            return event_model_proxy;
    }
    return NULL;
}

/**
  * ���������� ��������� ����� � ��������� xml ���������
  *
  * @param dom_doc - ��������, ���� ������� ��������� ������
  * @param id - ������ �� ������������� �����
  * @param name - ������ �� ������� �����
  *
  * @return ���������� QDomElement, ��������� ��� ������� � ��������� xml.
  */
QDomElement SettingsObj::addTagToDom(QDomDocument dom_doc,
                                const QString &id,
                                const QString &name)
{
    QDomElement dom_element = makeElement(dom_doc, "tag", id, "");

    dom_element.appendChild(makeElement(dom_doc, "name", "", name));

    return dom_element;
}

/**
  * ���������� �������� ��������� � ��������� xml ���������
  *
  * @param dom_doc - ��������, ���� ������� ��������� ������
  * @param dev - ������ �� ��������� DEV_INFO.
  *
  * @return ���������� QDomElement, ��������� ��� ������� � ��������� xml.
  */
QDomElement SettingsObj::addDevToDom(QDomDocument dom_doc, ulong id)
{
    QDomElement root_dev_element = makeElement(dom_doc, "root_dev", QString().setNum(id), "");

    QList<DEV_INFO> * dev_root = dev_settings[id];

    QList<DEV_INFO>::iterator it = dev_root->begin();

    for(; it != dev_root->end(); ++it)
    {
        DEV_INFO dev = *it;

        QDomElement dev_element = makeElement(dom_doc, "dev", QString().setNum(dev.addr), "");

        dev_element.appendChild(makeElement(dom_doc, "channel", "", QString().setNum(dev.channel)));
        dev_element.appendChild(makeElement(dom_doc, "active", "", QString().setNum(dev.active)));
        dev_element.appendChild(makeElement(dom_doc, "dist1", "", QString().setNum(dev.dist1)));
        dev_element.appendChild(makeElement(dom_doc, "time1", "", QString().setNum(dev.time1)));
        dev_element.appendChild(makeElement(dom_doc, "dist2", "", QString().setNum(dev.dist2)));
        dev_element.appendChild(makeElement(dom_doc, "time2", "", QString().setNum(dev.time2)));
        dev_element.appendChild(makeElement(dom_doc, "name", "", dev.name));

        root_dev_element.appendChild(dev_element);
    }

    return root_dev_element;
}

/**
  * ���������� ���������� � ��������� ������� � ��������� xml ���������
  *
  * @param dom_doc - ��������, ���� ������� ��������� ������
  * @param row - ����� ������ � ������ event_model, ������ ������� ����� ������
  *
  * @return ���������� QDomElement, ��������� ��� ������� � ��������� xml.
  */
QDomElement SettingsObj::addEventToDom(QDomDocument dom_doc, int row)
{
    QDomElement dom_element = makeElement(dom_doc, "event_node", "", "");

    dom_element.appendChild(makeElement(dom_doc, "id_dev", "", event_model->index(row, EvIdDev).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "name", "", event_model->index(row, EvName).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "id_tag", "", event_model->index(row, EvIdTag).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "channel", "", event_model->index(row, EvChanell).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "event", "", event_model->index(row, EvEvent).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "react", "", event_model->index(row, EvReact).data().toString()));

    int r = 0;
    int g = 0;
    int b = 0;

    event_model->item(row, EvReact)->background().color().getRgb(&r, &g, &b);;

    dom_element.appendChild(makeElement(dom_doc, "red", "", QString().setNum(r)));
    dom_element.appendChild(makeElement(dom_doc, "green", "", QString().setNum(g)));
    dom_element.appendChild(makeElement(dom_doc, "blue", "", QString().setNum(b)));

    return dom_element;
}

/**
  * ������ �������� ��� ���������� ��������� ����������.
  * � �������� ����������� �������� ������������ ����� �������.
  *
  * @param row - ����� ������ �� ������ dev_model, ������� ��������� ����� ����������
  *     ������� �����������/��������������.
  * @param active - ���� true, �� ���������� ����������, ���� false, �� ������������
  *
  * @return ���������� ��� ���������� ���������� ��������. R245_OK - ���� �� ������.
  */
short int SettingsObj::setActiveDev(int row, bool active)
{
    QBrush color;
    qint8 ft_status = 1;

    if(active)
    {
        ft_status = utils.R245_InitDev(row);
        if(!ft_status)
            color = Qt::green;
    } else
    {
        ft_status = utils.R245_CloseDev(row);
        if(!ft_status)
            color = Qt::white;
    }

    return ft_status;
}

short int SettingsObj::setAuditEn(int row, unsigned char addr, bool active)
{

    qint8 ft_status = 0;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr, true);

    if(dev != NULL)
    {
        ft_status = utils.R245_AuditEn(row, addr, active);

        if(!ft_status)
        {
            dev->active = active;
        }
    }

    return ft_status;
}


short int SettingsObj::setChannelDev(int row, unsigned char addr, short int channel)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr, true);

    if(dev != NULL)
    {
        if(channel & CHANNEL_ACT_1)
        {
            ft_status = utils.R245_SetChan(row, addr, 1, 1);
        }
        else
        {
            ft_status = utils.R245_SetChan(row, addr, 1, 0);
        }

        if(!ft_status)
        {
            if(channel & CHANNEL_ACT_2)
            {
                ft_status = utils.R245_SetChan(row, addr, 2, 1);
            }
            else
            {
                ft_status = utils.R245_SetChan(row, addr, 2, 0);
            }

            if(!ft_status)
            {
                dev->channel = channel;
            }
        }
    }

    return ft_status;
}

short int SettingsObj::setDistDev(int row, unsigned char addr, unsigned char dist, bool dist1)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr, true);
    unsigned char channel = (dist1)? 1: 2;

    if(dev != NULL)
    {
        ft_status = utils.R245_SetDamp(row, addr, channel, dist);

        if(!ft_status)
        {
            if(dist1)
                dev->dist1 = dist;
            else
                dev->dist2 = dist;
        }
    } else
    {
        qDebug() << "link to dev is null";
    }

    return ft_status;
}

short int SettingsObj::setTimeDev(int row, unsigned char addr, short int time, bool time1)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = NULL;
    dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr, true);
    unsigned char channel = (time1)? 1: 2;

    qDebug() << "Set time: " << time;

    if(dev != NULL)
    {

        ft_status = utils.R245_SetTime(row, addr, channel, time);

        if(!ft_status)
        {
            if(time1)
                dev->time1 = time;
            else
                dev->time2 = time;
        }
    }

    return ft_status;
}

void SettingsObj::saveSetings()
{
    if(utils.openFile(fsettings, QIODevice::WriteOnly))
    {
        QString id, name;

        QDomDocument doc("settings");


        QDomElement root_el = doc.createElement("settings");

        doc.appendChild(root_el);

        // ��������� ��������� ���� �����
        QDomElement tag_dom = makeElement(doc, "tags", "", "");

        root_el.appendChild(tag_dom);

        for(int row = 0; row < tag_model->rowCount(); ++row)
        {
            id = tag_model->data(tag_model->index(row, 0)).toString();
            name = tag_model->data(tag_model->index(row, 1)).toString();

            tag_dom.appendChild(addTagToDom(doc, id, name));
        }

        // ��������� ��������� ���������
        QDomElement dev_dom = makeElement(doc, "devices", "", "");

        root_el.appendChild(dev_dom);

        QMap<ulong, QList<DEV_INFO> *>::const_iterator it = dev_settings.constBegin();

        for(; it != dev_settings.constEnd(); ++it)
        {
            dev_dom.appendChild(addDevToDom(doc, it.key()));
        }

        // ��������� ��������� �������
        QDomElement event_dom = makeElement(doc, "events", "", "");

        root_el.appendChild(event_dom);

        for(int row = 0; row < event_model->rowCount(); ++row)
        {
            event_dom.appendChild(addEventToDom(doc, row));
        }

        QTextStream text_settings(fsettings);

        text_settings.setCodec("Windows-1251");

        text_settings<< doc.toString();
        utils.closeFile(fsettings);
    }

}

QDomElement SettingsObj::makeElement(QDomDocument &dom_doc,
                                     const QString &name,
                                     const QString &attr,
                                     const QString &text)
{
    QDomElement dom_element = dom_doc.createElement(name);

    if(!attr.isEmpty())
    {
        QDomAttr dom_attr = dom_doc.createAttribute("id");
        dom_attr.setValue(attr);
        dom_element.setAttributeNode(dom_attr);
    }

    if(!text.isEmpty())
    {
        QDomText dom_text = dom_doc.createTextNode(text);
        dom_element.appendChild(dom_text);
    }

    return dom_element;
}

void SettingsObj::addTagToModel(QString id, QString name)
{
    int row = 0/*tag_model->rowCount()*/;
    tag_model->insertRow(row);
    tag_model->setItem(row, 0, new QStandardItem(id));
    tag_model->setItem(row, 1, new QStandardItem(name));
}

void SettingsObj::addEventToModel(QString id_dev, QString name,
                     QString chanell,
                     QString id_tag, QString event, QString react,
                     int red, int green, int blue)
{
    int row = event_model->rowCount();

    event_model->insertRow(row);

    QString tag_name = "", dev_name = "";

    findTagAlias(id_tag, &tag_name);
    findDevAlias(id_dev, &dev_name);

    event_model->setItem(row, EvIdDev, new QStandardItem(id_dev));
    event_model->setItem(row, EvName, new QStandardItem(name));
    event_model->setItem(row, EvChanell, new QStandardItem(chanell));
    event_model->setItem(row, EvIdTag, new QStandardItem(id_tag));
    event_model->setItem(row, EvEvent, new QStandardItem(event));
    event_model->setItem(row, EvReact, new QStandardItem(react));

    event_model->item(row, EvReact)->setBackground(QColor(red, green, blue));

    if(tag_name == "")
    {
        event_model->setItem(row, EvNameTag, new QStandardItem(id_tag));
    }
    else
    {
        event_model->setItem(row, EvNameTag, new QStandardItem(tag_name));
    }

    if(dev_name == "")
    {
        event_model->setItem(row, EvNameDev, new QStandardItem(id_dev));
    }
    else
    {
        event_model->setItem(row, EvNameDev, new QStandardItem(dev_name));
    }
}

// addr = true => id_reader = addr, else id_reader = row
DEV_INFO * SettingsObj::getDevSettings(ulong id, unsigned char id_reader, bool addr)
{

    if(dev_settings.find(id) != dev_settings.end())
    {
        QList<DEV_INFO> * dev_root = dev_settings[id];

        if(addr)
        {
            QList<DEV_INFO>::iterator it = dev_root->begin();

            for(; it != dev_root->end(); ++it)
            {
                if(it->addr == id_reader)
                {
                    return &(*it);
                }
            }
        } else
        {
            return &((*dev_root)[id_reader]);
        }
    }

    return NULL;
}

bool SettingsObj::isFreeAddress(unsigned char dev_num, unsigned char addr)
{
    for(int i = 0; i < dev_model->item(dev_num)->rowCount(); i++)
    {
        unsigned char cur_addr = dev_model->item(dev_num)->child(i)->data(Qt::DisplayRole).toInt();
        if(cur_addr == addr)
        {
            return false;
        }
    }
    return true;
}

unsigned char SettingsObj::getFreeAddress(unsigned char dev_num)
{
    unsigned char addr = 1;

    for(addr = 1; addr < 255; addr++)
    {
        if(isFreeAddress(dev_num, addr))
        {
            return addr;
        }
    }
    return 0;
}

void SettingsObj::addReaderToModel(unsigned char dev_num, unsigned char addr, QString name)
{
    QModelIndex index = dev_model->index(dev_num, 0);

    QList<QStandardItem *> items;

    ulong id = index.data().toULongLong();

    // ����� ���������� ������ ��� ����������
    if(addr == 0)
    {
        addr = getFreeAddress(dev_num);
        if(addr == 0)
        {
            // ���� �� ���� ���������� ������, �� ������� ���������
            utils.showMessage(QMessageBox::Warning, "����� ������", "��� ��������� ������ ������");
            return;
        }
    }

    DEV_INFO * dev = getDevSettings(id, addr, true);

    if(dev == NULL)
    {

        qDebug() << "Dev NULL";
        DEV_INFO dev_new;

        dev_new.active = false;
        dev_new.addr = addr;
        dev_new.channel = 0;
        dev_new.dist1 = 7;
        dev_new.dist2 = 3;
        dev_new.name = name;
        dev_new.time1 = 0;
        dev_new.time2 = 0;

        dev_settings[id]->append(dev_new);

        items.append(new QStandardItem(QString().setNum(addr)));
        items.append(new QStandardItem(name));
    } else
    {
        qDebug() << "SETTINGS LOAD";
        items.append(new QStandardItem(QString().setNum(dev->addr)));
        items.append(new QStandardItem(dev->name));
    }

    dev_model->addReader(dev_num, items);
    if(setAuditEn(dev_num, items[0]->text().toInt(), true))
    {
        qDebug() << "Error acivate, dev_num = " << dev_num << " addr = " << addr;
    }
    else
    {
        qDebug() << "Reader is activated, dev_num = " << dev_num << " addr = " << addr;
    }

    int row = dev_model->item(dev_num)->rowCount() - 1;
    emit sigAddReader(dev_model->item(dev_num)->child(row));
}

bool SettingsObj::getReaderSettings(unsigned char dev_num, DEV_INFO * dev)
{
    qDebug() << "NUM = " << dev_num;
    qDebug() << "ADDR = " << dev->addr;
    int ft_status = utils.R245_GetChan(dev_num, dev->addr, &dev->channel);
    if(ft_status == R245_OK)
    {
        utils.R245_GetDamp(dev_num, dev->addr, 1, &dev->dist1);
        utils.R245_GetDamp(dev_num, dev->addr, 2, &dev->dist2);
        utils.R245_GetTime(dev_num, dev->addr, 1, &dev->time1);
        utils.R245_GetTime(dev_num, dev->addr, 2, &dev->time2);

        return true;
    }

    utils.showMessage(QMessageBox::Warning,
                          "��������� ��������",
                          "���������� �������� ��������� ����������");

    qDebug() << ft_status;

    return false;
}

void SettingsObj::deleteReaderFromModel(int dev_num, int reader_num)
{
    ulong id = dev_model->item(dev_num)->data(Qt::DisplayRole).toULongLong();

    dev_settings[id]->removeAt(reader_num);
    dev_model->delReader(dev_num, reader_num);
}

void SettingsObj::addDevInfoToModel(R245_DEV_INFO * info)
{
    int row = dev_model->rowCount();

    if(!setActiveDev(row, true))
    {
        QStandardItem * id_item = new QStandardItem(QString().setNum(info->id));
        QStandardItem * desc_item = new QStandardItem(info->desc);

        id_item->setEditable(false);
        desc_item->setEditable(false);

        QList<QStandardItem *> items;

        items.append(id_item);
        items.append(desc_item);

        dev_model->appendRow(items);

        if(dev_settings.find(info->id) == dev_settings.end())
        {
            dev_settings[info->id] = new QList<DEV_INFO>;
        } else
        {
            QList<DEV_INFO> * dev_root = dev_settings[info->id];
            QList<DEV_INFO>::iterator it = dev_root->begin();

            for(; it != dev_root->end(); ++it)
            {
                addReaderToModel(row, it->addr, it->name);
            }
        }
    }
}

SettingsObj::~SettingsObj()
{

    if(utils.closeFile(fsettings))
        delete fsettings;
    if(utils.closeFile(flog))
        delete flog;

    delete tag_model;
    delete tag_model_proxy;
    delete dev_model;
    delete event_model;
}
