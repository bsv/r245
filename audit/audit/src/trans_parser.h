#ifndef TRANS_PARSER_H
#define TRANS_PARSER_H

#include <QXmlDefaultHandler>
#include <QProgressDialog>
#include <QDebug>
#include "monitor.h"
#include "r245_types.h"
#include "settings_obj.h"

#define TRANS_LINE_COUNT 13 // количество строк, занимаемых одной транзакцией в журнале

class TransParser : public QXmlDefaultHandler
{
public:
    TransParser(qint64 lcount, Monitor *monitor, SettingsObj *set);
    ~TransParser();
    bool startDocument();
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool characters(const QString &ch);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool fatalError(const QXmlParseException &exception);
    bool endDocument();
    bool parseOK();

private:
    R245_TRANSACT trans;
    QString dev_num;
    QString el_name;
    Monitor * monitor_obj;
    SettingsObj * set_obj;
    bool transact;
    bool parse_ok;

    QProgressDialog * progress;
    qint64 line_ctr;
};

#endif // TRANS_PARSER_H
