#ifndef __MONITOR_WINDOW_H__
#define __MONITOR_WINDOW_H__

#include <QDialog>
#include <QTimer>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <QFileDialog>
#include <QProgressDialog>
#include "settings_obj.h"
#include "monitor.h"
#include "ui_monitor_window.h"

#define MAX_TRANS_FOR_TIMER_INT 5

/*class PrintThread : QThread
{
public:
    PrintThread();
    void start(QTextDocument *qdoc, QPrinter *pr, Priority priority = InheritPriority);
    void run();

private:
    QTextDocument * doc;
    QPrinter * printer;
};*/


class MonitorWindow : public QDialog, public Ui::MonitorWindow
{
    Q_OBJECT
public:
    MonitorWindow(SettingsObj * set, Monitor * mon, QWidget *parent = 0);
    ~MonitorWindow();

    Monitor * getMonitor();
    void eventHandler(QString dev_num, R245_TRANSACT * trans);
private:
    QTimer timer;
    SettingsObj * set_obj;
    Monitor * monitor;
    //TransThread * trans_thread;
    QPrinter printer_pdf;
    QTextDocument qdoc;
    QFutureWatcher<void> future_watch;
    //PrintThread print_thread;

    static void printThreadFunc(QTextDocument * qdoc, QPrinter * printer);
    static void initPdfPrinter(QPrinter * printer);
    static void setHtmlTh(QTextDocument * qdoc, QString text);

    void initMas();
    void printMonitor(QPrinter * printer);
    void setFilter();
private slots:
    void slotUpdateTrans();
    void slotResetFilter();
    void slotTabChanged();
    void slotTagInform();
    void slotSaveFile();
    void slotClearMonitor();

    void slotPrintOK();
};

#endif // __MONITOR_WINDOW_H__
