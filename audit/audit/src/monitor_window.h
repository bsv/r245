/**
  * @file monitor_window.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса окна монитора.
  */

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
#include "monitor.h"
#include "settings_obj.h"
#include "ui_monitor_window.h"

/**
  * Максимальное число транзакций, считываемых
  * за один период обращения к считывателю.
  */
#define MAX_TRANS_FOR_TIMER_INT 5

/**
  * Период обращения к считывателю в миллисекундах.
  */
#define REQ_PERIOD 1000

class MonitorWindow : public QDialog, public Ui::MonitorWindow
{
    Q_OBJECT
public:
    /** @name Конструкторы/деструкторы
      * @{
      */
    MonitorWindow(SettingsObj * set, Monitor * mon, QWidget *parent = 0);
    ~MonitorWindow();
    /** @} */

    Monitor * getMonitor();
    void eventHandler(QString dev_num, R245_TRANSACT * trans, QString tag_name = "", QString dev_name = "");
private:
    /**
      * Объект таймера.
      * Используется для организации периодического
      * опроса устройства считывателя. Период опроса задается
      * макросом REQ_PERIOD.
      */
    QTimer timer;

    /// Указатель на объект, отвечающий за настройки приложения.
    SettingsObj * set_obj;

    /// Указатель на объект монитора.
    Monitor * monitor;

    /// Объект pdf принтера.
    QPrinter printer_pdf;

    /// Объект текстового документа. Используется в качестве
    /// промежуточного хранилища подготовленных к печати данным в формате html.
    QTextDocument qdoc;

    /// С помощью этого объекта возможно наблюдать за потоком,
    /// функционирующим в рамках QFuture<void> future.
    QFutureWatcher<void> future_watch;

    static void printThreadFunc(QTextDocument * qdoc, QPrinter * printer);

    void printMonitor(QString file_path);
    void setFilter();
private slots:
    void slotUpdateTrans();
    void slotResetFilter();
    void slotTabChanged();
    void slotTagInform();
    void slotSaveFile();
    void slotClearMonitor();
    void slotResizeMonitorView();

    void slotPrintOK();
};

#endif // __MONITOR_WINDOW_H__
