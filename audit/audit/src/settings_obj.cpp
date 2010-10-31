#include "settings_obj.h"
#include "global.h"
#include "trans_parser.h"

SettingsObj::SettingsObj()
{

    fsettings = NULL;
    flog = NULL;
    log_stream = NULL;

    tag_model = new QStandardItemModel();
    tag_model->setObjectName("tag_model");

    dev_name_model = new QStandardItemModel();
    dev_name_model->setObjectName("dev_name_model");

    event_model = new QStandardItemModel();

    initSetModels();

    tag_model_proxy = new QSortFilterProxyModel();
    tag_model_proxy->setSourceModel(tag_model);
    tag_model_proxy->setFilterKeyColumn(-1); // filter all column
    tag_model_proxy->setFilterWildcard("*");

    dev_name_model_proxy = new QSortFilterProxyModel();
    dev_name_model_proxy->setSourceModel(dev_name_model);
    dev_name_model_proxy->setFilterKeyColumn(-1); // filter all column
    dev_name_model_proxy->setFilterWildcard("*");

    event_model_proxy = new QSortFilterProxyModel();
    event_model_proxy->setSourceModel(event_model);
    event_model_proxy->setFilterKeyColumn(-1);
    event_model_proxy->setFilterWildcard("*");

    dev_model = new QStandardItemModel();
}

/**
  * Функция инициализации моделей, хранящих данные о настройках
  */
void SettingsObj::initSetModels()
{
    tag_model->clear();
    dev_name_model->clear();
    event_model->clear();

    QStringList tag_header;
    tag_header << "id" << "имя";
    tag_model->setColumnCount(tag_header.count());
    tag_model->setHorizontalHeaderLabels(tag_header);

    dev_name_model->setColumnCount(tag_header.count());
    dev_name_model->setHorizontalHeaderLabels(tag_header);

    QStringList event_header;
    event_header << "id/имя устройства" <<
            "имя" << "канал" << "id/имя метки" << "событие" <<
            "реакция" << "id устройства" << "id метки";

    event_model->setColumnCount(event_header.count());
    event_model->setHorizontalHeaderLabels(event_header);

    dev_settings.clear();
}

/**
  * Установка параметров поиска для моделей
  *
  * @param ex - регулярное выражения для поиска
  * @param type_model - тип модели, по которой необходимо осуществить поиск
  */
void SettingsObj::setFilterWildCard(QString ex, TypeModel type_model)
{
    switch(type_model)
    {
        case TagModelProxy:
            tag_model_proxy->setFilterWildcard(ex);
            break;
        case DevNameModelProxy:
            dev_name_model_proxy->setFilterWildcard(ex);
            break;
        case EventModelProxy:
            event_model_proxy->setFilterWildcard(ex);
            break;
        default:
            break;
    }
}

/**
  * Открытие файла настроек
  * @param file_name - путь к файлу настроек
  *
  * @return true в случае удачной операции открытия
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
        if(dom_doc.setContent(fsettings))
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
  * Открытие файла журнала
  * @param file_name - путь к файлу журнала
  *
  * @return true в случае удачной операции открытия
  */
bool SettingsObj::openLogFile(QString file_name, Monitor *monitor)
{
    if(utils.closeFile(flog))
    {
        delete flog;
        flog = NULL;
    }
    qDebug("Open Log");

    flog = new QFile(file_name);

    if(utils.openFile(flog, QIODevice::ReadOnly))
    {
        qint64 line_ctr = 0;

        // считаем количество строк в файле
        while(!flog->atEnd())
        {
            flog->readLine();
            line_ctr++;
        }

        utils.closeFile(flog);

        TransParser tparser(line_ctr, monitor, this);
        QXmlInputSource source(flog);
        QXmlSimpleReader reader;

        reader.setContentHandler(&tparser);
        reader.parse(source);

        if(!tparser.parseOK())
        {
            utils.showMessage(QMessageBox::Warning,
                              "Открытие журнала",
                              "Файл журнала поврежден");
        }
    }

    if(utils.closeFile(flog))
    {
        qDebug("Close flog");
    }

    if(utils.openFile(flog, QIODevice::Append))
    {
        qDebug("Open flog OK");
        flog->seek(flog->size() - QString("</log>\n").size());
        if(log_stream != NULL)
        {
            delete log_stream;
            log_stream = NULL;
        }
        log_stream = new QTextStream(flog);
    }

    return true;
}

/**
  * Добавление записи в файл журнала
  *
  * @param dev_num - идентификатор устройства
  * @param trans - ссылка на структуру,
  *   содержащую информацию о произошедшем событии, регистрируемое в журнале
  */
void SettingsObj::addLogNode(QString dev_num, R245_TRANSACT * trans)
{
    if(log_stream != NULL)
    {
        *log_stream << "    <transact>\n";
        *log_stream << "        <code>"    << trans->code    << "</code>\n";
        *log_stream << "        <channel>" << trans->channel << "</channel>\n";
        *log_stream << "        <tid>"     << trans->tid     << "</tid>\n";
        *log_stream << "        <day>"     << trans->day     << "</day>\n";
        *log_stream << "        <month>"   << trans->month   << "</month>\n";
        *log_stream << "        <year>"    << trans->year    << "</year>\n";
        *log_stream << "        <hour>"    << trans->hour    << "</hour>\n";
        *log_stream << "        <min>"     << trans->min     << "</min>\n";
        *log_stream << "        <sec>"     << trans->sec     << "</sec>\n";
        *log_stream << "        <dow>"     << trans->dow     << "</dow>\n";
        *log_stream << "        <dev_num>" << dev_num        << "</dev_num>\n";
        *log_stream << "    </transact>\n";
    }
}

/**
  * Функция получает информацию о подключенных устройствах.
  * Полученная информация помещается в модель dev_model
  */
void SettingsObj::readDevInfo()
{
    R245_DEV_INFO info;
    short int dev_ctr = 0;

    dev_model->clear();
    utils.R245_CloseAllDev();

    QStringList dev_header;
    dev_header << /*"номер" <<*/ "тип" << "id" << "locId" << "описание";
    dev_model->setColumnCount(dev_header.count());
    dev_model->setHorizontalHeaderLabels(dev_header);

    while(!utils.R245_GetDevInfo(dev_ctr, &info))
    {
        //QString num = info.serial_number;
        QString type = QString().setNum(info.type);
        QString id = QString().setNum(info.id);
        QString loc_id = QString().setNum(info.loc_id);
        QString desc = info.desc;

        addDevInfoToModel(/*num,*/ type, id, loc_id, desc);
        dev_ctr++;
    }
}

/**
  * Считывание настроек из файла. Информация о настройках помещается
  * в соответствующие модели tag_model, dev_name_model, event_model.
  * Информация о настройках устройств сохраняется в списке dev_settings.
  *
  * @param node - ссылка на элемент в структуре xml, содержащий настройки
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
                    qDebug()<< "set tag";
                } else if(dom_el.tagName() == "dev_name")
                {
                    QDomElement child_el = dom_node.firstChildElement();
                    addDevNameToModel(dom_el.attribute("id", ""), child_el.text());
                } else if(dom_el.tagName() == "dev")
                {
                    DEV_INFO dev;

                    dev.id = dom_el.attribute("id", "").toInt();

                    QDomElement child_el = dom_node.firstChildElement();
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
                        }
                        child_el = child_el.nextSiblingElement();
                    }

                    if(getDevSettings(dev.id) == NULL)
                        dev_settings.append(dev);

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
  * Получение ссылки на модель
  *
  * @param type_model - тип запрашиваемой модели
  *
  * @return возвращает указатель на запрашиваемую модель
  */
QAbstractItemModel * SettingsObj::getModel(TypeModel type_model)
{
    switch(type_model)
    {
        case TagModel:
            return tag_model;
        case DevNameModel:
            return dev_name_model;
        case DevModel:
            return dev_model;
        case EventModel:
            return event_model;
        case TagModelProxy:
            return tag_model_proxy;
        case DevNameModelProxy:
            return dev_name_model_proxy;
        case EventModelProxy:
            return event_model_proxy;
    }
    return NULL;
}

/**
  * Добавление синонимов меток в структуру xml документа
  *
  * @param dom_doc - документ, куда следует поместить данные
  * @param id - ссылка на идентификатор метки
  * @param name - ссылка на синоним метки
  *
  * @return возвращает QDomElement, пригодный для вставки в структуру xml.
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
  * Добавление синонимов устройств в структуру xml документа
  *
  * @param dom_doc - документ, куда следует поместить данные
  * @param id - ссылка на идентификатор устройства
  * @param name - ссылка на синоним устройства
  *
  * @return возвращает QDomElement, пригодный для вставки в структуру xml.
  */
QDomElement SettingsObj::addDevNameToDom(QDomDocument dom_doc, const QString &id, const QString &name)
{
    QDomElement dom_element = makeElement(dom_doc, "dev_name", id, "");

    dom_element.appendChild(makeElement(dom_doc, "name", "", name));

    return dom_element;
}

/**
  * Добавление настроек устройств в структуру xml документа
  *
  * @param dom_doc - документ, куда следует поместить данные
  * @param dev - ссылка на структуру DEV_INFO.
  *
  * @return возвращает QDomElement, пригодный для вставки в структуру xml.
  */
QDomElement SettingsObj::addDevToDom(QDomDocument dom_doc, const DEV_INFO &dev)
{
    QDomElement dom_element = makeElement(dom_doc, "dev", QString().setNum(dev.id), "");

    dom_element.appendChild(makeElement(dom_doc, "channel", "", QString().setNum(dev.channel)));
    dom_element.appendChild(makeElement(dom_doc, "active", "", QString().setNum(dev.active)));
    dom_element.appendChild(makeElement(dom_doc, "dist1", "", QString().setNum(dev.dist1)));
    dom_element.appendChild(makeElement(dom_doc, "time1", "", QString().setNum(dev.time1)));
    dom_element.appendChild(makeElement(dom_doc, "dist2", "", QString().setNum(dev.dist2)));
    dom_element.appendChild(makeElement(dom_doc, "time2", "", QString().setNum(dev.time2)));

    return dom_element;
}

/**
  * Добавление информации о настроках событий в структуру xml документа
  *
  * @param dom_doc - документ, куда следует поместить данные
  * @param row - номер строки в модели event_model, откуда следует взять данные
  *
  * @return возвращает QDomElement, пригодный для вставки в структуру xml.
  */
QDomElement SettingsObj::addEventToDom(QDomDocument dom_doc, int row)
{
    QDomElement dom_element = makeElement(dom_doc, "event_node", "", "");

    dom_element.appendChild(makeElement(dom_doc, "id_dev", "", event_model->index(row, EvIdDev).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "name", "", event_model->index(row, EvName).data().toString().toUtf8()));
    dom_element.appendChild(makeElement(dom_doc, "id_tag", "", event_model->index(row, EvIdTag).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "channel", "", event_model->index(row, EvChanell).data().toString()));
    dom_element.appendChild(makeElement(dom_doc, "event", "", event_model->index(row, EvEvent).data().toString().toUtf8()));
    dom_element.appendChild(makeElement(dom_doc, "react", "", event_model->index(row, EvReact).data().toString().toUtf8()));

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
  * Делает активным или неактивным выбранное устройство.
  * С активным устройством возможно осуществлять обмен данными.
  *
  * @param row - номер строки из модели dev_model, который указывает какое устройство
  *     следует акивировать/деактивировать.
  * @param active - если true, то активируем устройство, если false, то даективируем
  *
  * @return возвращает код результата выполнения операции. R245_OK - если всё хорошо.
  */
short int SettingsObj::setActiveDev(int row, bool active)
{
    QBrush color;
    qint8 ft_status = 1;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt());

    if(dev != NULL)
    {
        if(active)
        {
            color = Qt::green;
            utils.R245_InitDev(row);
            ft_status = utils.R245_AuditEn(row, active);
        } else
        {
            color = Qt::white;
            ft_status = utils.R245_CloseDev(row);
        }

        if(!ft_status)
        {
            dev->active = active;
            qDebug("Active ok");
            for(int i = 0; i < dev_model->columnCount(); ++i)
            {
                dev_model->item(row, i)->setBackground(color);
            }
        } else
        {
            utils.R245_CloseDev(row);
        }
    }

    return ft_status;
}


short int SettingsObj::setChannelDev(int row, short int channel)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt());

    //qDebug() << "Channel change = " << channel;

    if(dev != NULL)
    {
        if(!dev->active)
            utils.R245_InitDev(row);

        if(channel & CHANNEL_ACT_1)
            ft_status = utils.R245_SetChan(row, 1, 1);
        else
            ft_status = utils.R245_SetChan(row, 1, 0);

        if(!ft_status)
        {
            if(channel & CHANNEL_ACT_2)
                ft_status = utils.R245_SetChan(row, 2, 1);
            else
                ft_status = utils.R245_SetChan(row, 2, 0);

            if(!ft_status)
            {
                dev->channel = channel;
            }
        }
        if(!dev->active)
            utils.R245_CloseDev(row);
    }

    return ft_status;
}

short int SettingsObj::setDistDev(int row, short int dist, bool dist1)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt());
    unsigned char channel = (dist1)? 1: 2;

    if(dev != NULL)
    {
        if(!dev->active)
            utils.R245_InitDev(row);
        ft_status = utils.R245_SetDamp(row, channel, 31 - dist);

        if(!ft_status)
        {
            if(dist1)
                dev->dist1 = dist;
            else
                dev->dist2 = dist;
        }
        if(!dev->active)
            utils.R245_CloseDev(row);
    } else
    {
        qDebug() << "link to dev is null";
    }

    return ft_status;
}

short int SettingsObj::setTimeDev(int row, short int time, bool time1)
{
    qint8 ft_status = 0;
    DEV_INFO * dev = NULL;
    dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt());
    unsigned char channel = (time1)? 1: 2;

    qDebug() << "Set time: " << time;

    if(dev != NULL)
    {

        if(!dev->active)
            utils.R245_InitDev(row);

        ft_status = utils.R245_SetTime(row, channel, time);

        if(!ft_status)
        {
            if(time1)
                dev->time1 = time;
            else
                dev->time2 = time;
        }

        if(!dev->active)
            utils.R245_CloseDev(row);
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

        QDomElement tag_dom = makeElement(doc, "tags", "", "");

        root_el.appendChild(tag_dom);

        for(int row = 0; row < tag_model->rowCount(); ++row)
        {
            id = tag_model->data(tag_model->index(row, 0)).toString();
            name = tag_model->data(tag_model->index(row, 1)).toString().toUtf8();

            tag_dom.appendChild(addTagToDom(doc, id, name));
        }

        QDomElement dev_name_dom = makeElement(doc, "dev_names", "", "");

        root_el.appendChild(dev_name_dom);

        for(int row = 0; row < dev_name_model->rowCount(); ++row)
        {
            id = dev_name_model->data(dev_name_model->index(row, 0)).toString();
            name = dev_name_model->data(dev_name_model->index(row, 1)).toString().toUtf8();

            dev_name_dom.appendChild(addDevNameToDom(doc, id, name));
        }

        QDomElement dev_dom = makeElement(doc, "devices", "", "");

        root_el.appendChild(dev_dom);

        QList<DEV_INFO>::iterator it = dev_settings.begin();

        for(; it != dev_settings.end(); ++it)
        {
            dev_dom.appendChild(addDevToDom(doc, *it));
        }

        QDomElement event_dom = makeElement(doc, "events", "", "");

        root_el.appendChild(event_dom);

        for(int row = 0; row < event_model->rowCount(); ++row)
        {
            event_dom.appendChild(addEventToDom(doc, row));
        }

        QTextStream(fsettings) << doc.toString();
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

void SettingsObj::addDevNameToModel(QString id, QString name)
{
    int row = 0/*dev_name_model->rowCount()*/;
    dev_name_model->insertRow(row);
    dev_name_model->setItem(row, 0, new QStandardItem(id));
    dev_name_model->setItem(row, 1, new QStandardItem(name));
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

    utils.findAlias(tag_model, id_tag, &tag_name);
    utils.findAlias(dev_name_model, id_dev, &dev_name);

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

DEV_INFO * SettingsObj::getDevSettings(unsigned int id)
{
    QList<DEV_INFO>::iterator it = dev_settings.begin();

    for(; it != dev_settings.end(); ++it)
    {
        if(it->id == id)
        {
            return &(*it);
        }
    }

    return NULL;
}

void SettingsObj::addDevInfoToModel(/*QString num,*/ QString type, QString id,
                       QString loc_id, QString desc)
{
    int row = dev_model->rowCount();

    dev_model->insertRow(row);
    //dev_model->setItem(row, Num, new QStandardItem(num));
    dev_model->setItem(row, Type, new QStandardItem(type));
    dev_model->setItem(row, Id, new QStandardItem(id));
    dev_model->setItem(row, LocId, new QStandardItem(loc_id));
    dev_model->setItem(row, Desc, new QStandardItem(desc));

    DEV_INFO * dev = getDevSettings(id.toInt());

    if(dev == NULL)
    {
        DEV_INFO dev_new;

        dev_new.active  = 0;
        dev_new.id      = id.toInt();

        utils.R245_InitDev(row);

        if(utils.R245_GetChan(row, &dev_new.channel) == R245_OK)
        {
            /* Если команда отправляется без ошибок, то
             * инициализация прошла успешно
             */

            utils.R245_GetChan(row, &dev_new.channel);
            utils.R245_GetDamp(row, 1, &dev_new.dist1);
            utils.R245_GetDamp(row, 2, &dev_new.dist2);
            utils.R245_GetTime(row, 1, &dev_new.time1);
            utils.R245_GetTime(row, 2, &dev_new.time2);

            utils.R245_CloseDev(row);
            dev_settings.append(dev_new);
        } else
        {
            qDebug() << "INIT ERROR";
            utils.showMessage(QMessageBox::Warning,
                              "Обнаружение устройств",
                              "Невозможно инициалицировать устройство " + QString().setNum(row)
                              );
        }


    } else
    {
        if(setActiveDev(row, dev->active) == R245_OK)
        {
            setChannelDev(row, dev->channel);
            setTimeDev(row, dev->time1, true);
            setTimeDev(row, dev->time2, false);
            setDistDev(row, dev->dist1, true);
            setDistDev(row, dev->dist2, false);
        } else
        {
            utils.showMessage(QMessageBox::Warning,
                              "Обнаружение устройств",
                              "Невозможно настроить подключенное устройство " + QString().setNum(row)
                              );
        }
    }
}

SettingsObj::~SettingsObj()
{
    if(log_stream != NULL)
    {
        *log_stream << "</log>\n";
    }

    if(utils.closeFile(fsettings))
        delete fsettings;
    if(utils.closeFile(flog))
        delete flog;

    delete log_stream;
    delete tag_model;
    delete tag_model_proxy;
    delete dev_name_model;
    delete dev_name_model_proxy;
    delete dev_model;
    delete event_model;
}
