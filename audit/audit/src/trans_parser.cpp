#include "trans_parser.h"

TransParser::TransParser(qint64 lcount, Monitor *monitor, SettingsObj *set):
    dev_num(""),
    el_name(""), transact(false), line_ctr(0),
    monitor_obj(monitor), set_obj(set)
{

    progress = new QProgressDialog("Загрузка файла журнала", "&Cancel", 0, lcount);
    progress->setWindowTitle("Пожалуйста подождите...");
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->setModal(true);
}

TransParser::~TransParser()
{
    delete progress;
}

bool TransParser::startDocument()
{
    parse_ok = false;

    progress->setValue(++line_ctr);
    qDebug("Start doc OK");



    return true;
}

bool TransParser::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    el_name = qName;
    //qDebug() << "start element " << qName;

    if(qName == "transact")
    {
        transact = true;
        dev_num = "";

        trans.channel = 0;
        trans.code = 0;
        trans.day = 0;
        trans.dow = 0;
        trans.hour = 0;
        trans.min = 0;
        trans.month = 0;
        trans.sec = 0;
        trans.tid = 0;
        trans.year = 0;

    } else if(transact){
        el_name = qName;
    }
    return true;
}

bool TransParser::characters(const QString &ch)
{
    //qDebug() << "Read ch |" << el_name << ": " << ch;
    if(transact)
    {
        if(el_name == "code")
        {
            trans.code = ch.toInt();
        } else if(el_name == "channel")
        {
            trans.channel = ch.toInt();
        } else if(el_name == "tid")
        {
            trans.tid = ch.toInt();
        } else if(el_name == "day")
        {
            trans.day = ch.toInt();
        } else if(el_name == "month")
        {
            trans.month = ch.toInt();
        } else if(el_name == "year")
        {
            trans.year = ch.toInt();
        } else if(el_name == "hour")
        {
            trans.hour = ch.toInt();
        } else if(el_name == "min")
        {
            trans.min = ch.toInt();
        } else if(el_name == "sec")
        {
            trans.sec = ch.toInt();
        } else if(el_name == "dow")
        {
            trans.dow = ch.toInt();
        } else if(el_name == "dev_num")
        {
            dev_num = ch;
        }
    }
    el_name = "";

    return true;
}

bool TransParser::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    if(qName == "transact")
    {
        transact = false;

        line_ctr += TRANS_LINE_COUNT;
        progress->setValue(line_ctr);
        //qDebug("READ TRANSACT");

        QString tag_name = "", dev_name = "";
        utils.findAlias(set_obj->getModel(SettingsObj::TagModel),  QString().setNum(trans.tid), &tag_name);
        utils.findAlias(set_obj->getModel(SettingsObj::DevNameModel), dev_num, &dev_name);

        monitor_obj->addTransToModel(dev_num, &trans, tag_name, dev_name);
    }
    return true;
}

bool TransParser::fatalError(const QXmlParseException &exception)
{
    qDebug() << "Line:" << exception.lineNumber()
            << ", Column:" << exception.columnNumber()
            << ", Message:" << exception.message();
    return false;
}

bool TransParser::endDocument()
{
    parse_ok = true;
    progress->setValue(++line_ctr);

    qDebug() << "end parse";
    return true;
}

bool TransParser::parseOK()
{
    return parse_ok;
}
