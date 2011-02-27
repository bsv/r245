/**
  * @file monitor_window.h
  * @authors ��������� ������ (bsv.serg@gmail.com)
  * @authors �������� �������
  *
  * �������� �������� ������ ���� ��������.
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
  * ������������ ����� ����������, �����������
  * �� ���� ������ ��������� � �����������.
  */
#define MAX_TRANS_FOR_TIMER_INT 5

/**
  * ������ ��������� � ����������� � �������������.
  */
#define REQ_PERIOD 1000

class MonitorWindow : public QDialog, public Ui::MonitorWindow
{
    Q_OBJECT
public:
    /** @name ������������/�����������
      * @{
      */
    MonitorWindow(SettingsObj * set, Monitor * mon, QWidget *parent = 0);
    ~MonitorWindow();
    /** @} */

    Monitor * getMonitor();
    void eventHandler(QString dev_num, R245_TRANSACT * trans, QString tag_name = "", QString dev_name = "");
private:
    /**
      * ������ �������.
      * ������������ ��� ����������� ��������������
      * ������ ���������� �����������. ������ ������ ��������
      * �������� REQ_PERIOD.
      */
    QTimer timer;

    /// ��������� �� ������, ���������� �� ��������� ����������.
    SettingsObj * set_obj;

    /// ��������� �� ������ ��������.
    Monitor * monitor;

    /// ������ pdf ��������.
    QPrinter printer_pdf;

    /// ������ ���������� ���������. ������������ � ��������
    /// �������������� ��������� �������������� � ������ ������ � ������� html.
    QTextDocument qdoc;

    /// � ������� ����� ������� �������� ��������� �� �������,
    /// ��������������� � ������ QFuture<void> future.
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
