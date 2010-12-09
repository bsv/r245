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
    dev_model->setObjectName("dev_model");
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
    dev_header << "id" << "описание";
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
        addDevInfoToModel(&info);
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
                    //qDebug()<< "set tag";
                } else if(dom_el.tagName() == "dev_name")
                {
                    QDomElement child_el = dom_node.firstChildElement();
                    addDevNameToModel(dom_el.attribute("id", ""), child_el.text());
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
        case DevModel:
            return dev_model;
        case EventModel:
            return event_model;
        case TagModelProxy:
            return tag_model_proxy;
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
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr);

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
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr);

    if(dev != NULL)
    {
        if(channel & CHANNEL_ACT_1)
            ft_status = utils.R245_SetChan(row, addr, 1, 1);
        else
            ft_status = utils.R245_SetChan(row, addr, 1, 0);

        if(!ft_status)
        {
            if(channel & CHANNEL_ACT_2)
                ft_status = utils.R245_SetChan(row, addr, 2, 1);
            else
                ft_status = utils.R245_SetChan(row, addr, 2, 0);

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
    DEV_INFO * dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr);
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
    dev = getDevSettings(dev_model->data(dev_model->index(row, Id)).toInt(), addr);
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

        // Сохраняем настройки имен меток
        QDomElement tag_dom = makeElement(doc, "tags", "", "");

        root_el.appendChild(tag_dom);

        for(int row = 0; row < tag_model->rowCount(); ++row)
        {
            id = tag_model->data(tag_model->index(row, 0)).toString();
            name = tag_model->data(tag_model->index(row, 1)).toString();

            tag_dom.appendChild(addTagToDom(doc, id, name));
        }

        // Сохраняем настройки имен устройств
        QDomElement dev_name_dom = makeElement(doc, "dev_names", "", "");

        root_el.appendChild(dev_name_dom);

        for(int row = 0; row < dev_name_model->rowCount(); ++row)
        {
            id = dev_name_model->data(dev_name_model->index(row, 0)).toString();
            name = dev_name_model->data(dev_name_model->index(row, 1)).toString();

            dev_name_dom.appendChild(addDevNameToDom(doc, id, name));
        }

        // Сохраняем настройки устройств
        QDomElement dev_dom = makeElement(doc, "devices", "", "");

        root_el.appendChild(dev_dom);

        QMap<ulong, QList<DEV_INFO> *>::const_iterator it = dev_settings.constBegin();

        for(; it != dev_settings.constEnd(); ++it)
        {
            dev_dom.appendChild(addDevToDom(doc, it.key()));
        }

        // Сохраняем настройки событий
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

    //utils.findAlias(tag_model, id_tag, &tag_name);
    //utils.findAlias(dev_name_model, id_dev, &dev_name);

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

DEV_INFO * SettingsObj::getDevSettings(ulong id, unsigned char addr)
{

    if(dev_settings.find(id) != dev_settings.end())
    {
        QList<DEV_INFO> * dev_root = dev_settings[id];
        QList<DEV_INFO>::iterator it = dev_root->begin();

        for(; it != dev_root->end(); ++it)
        {
            if(it->addr == addr)
            {
                return &(*it);
            }
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

    // Поиск свободного адреса для устройства
    if(addr == 0)
    {
        addr = getFreeAddress(dev_num);
        if(addr == 0)
        {
            // Если не нали свободного адреса, то выводим сообщение
            utils.showMessage(QMessageBox::Warning, "Поиск адреса", "Все свободные адреса заняты");
            return;
        }
    }

    DEV_INFO * dev = getDevSettings(id, addr);

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

    dev_model->item(dev_num)->appendRow(items);

    int row = dev_model->item(dev_num)->rowCount() - 1;
    emit sigAddReader(dev_model->item(dev_num)->child(row));

    /*if(dev == NULL)
    {
        DEV_INFO dev_new;
        dev_new.name = name;
        dev_new.addr = addr;

        if(utils.R245_GetChan(row, addr, &dev_new.channel) == R245_OK)
        {
            utils.R245_GetDamp(row, addr, 1, &dev_new.dist1);
            utils.R245_GetDamp(row, addr, 2, &dev_new.dist2);
            utils.R245_GetTime(row, addr, 1, &dev_new.time1);
            utils.R245_GetTime(row, addr, 2, &dev_new.time2);

            items.append(new QStandardItem(QString().setNum(addr)));
            items.append(new QStandardItem(name));
            item->appendRow(items);

            dev_settings[id].append(dev_new);
        } else
        {
            utils.showMessage(QMessageBox::Warning,
                              "Обнаружение устройств",
                              "Невозможно инициалицировать устройство " + QString().setNum(row)
                              );
        }

    } else
    {
        if(setChannelDev(row, dev->channel) == R245_OK)
        {
            setTimeDev(row, dev->time1, true);
            setTimeDev(row, dev->time2, false);
            setDistDev(row, dev->dist1, true);
            setDistDev(row, dev->dist2, false);

            items.append(new QStandardItem(QString().setNum(dev->addr)));
            items.append(new QStandardItem(dev->name));
            item->appendRow(items);
        } else
        {
            utils.showMessage(QMessageBox::Warning,
                              "Обнаружение устройств",
                              "Невозможно настроить подключенное устройство " + QString().setNum(row)
                              );
        }
    }*/
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
                      "Получение настроек",
                      "Невозможно получить настройки устройства");

    qDebug() << ft_status;

    return false;
}

void SettingsObj::deleteReaderFromModel(int dev_num, int reader_num)
{
    ulong id = dev_model->item(dev_num)->data(Qt::DisplayRole).toULongLong();

    dev_settings[id]->removeAt(reader_num);
    dev_model->item(dev_num)->removeRow(reader_num);
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

    /*DEV_INFO * dev = getDevSettings(id.toInt());

    if(dev == NULL)
    {
        DEV_INFO dev_new;

        dev_new.active  = 0;
        dev_new.id      = id.toInt();

        utils.R245_InitDev(row);

        if(utils.R245_GetChan(row, 1, &dev_new.channel) == R245_OK)
        {*/
            /* Если команда отправляется без ошибок, то
             * инициализация прошла успешно
             */

       /*     utils.R245_GetChan(row, 1, &dev_new.channel);
            utils.R245_GetDamp(row, 1, 1, &dev_new.dist1);
            utils.R245_GetDamp(row, 1, 2, &dev_new.dist2);
            utils.R245_GetTime(row, 1, 1, &dev_new.time1);
            utils.R245_GetTime(row, 1, 2, &dev_new.time2);

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
    {*/
        // Запрещаем автоматическую настройку устройств
        /*if(setActiveDev(row, dev->active) == R245_OK)
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
        }*/
  //  }
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
