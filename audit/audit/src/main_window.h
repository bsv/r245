/**
  * @file main_window.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса главного окна.
  */

#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H

#include <QMainWindow>
#include "monitor_window.h"
#include "settings_window.h"
#include "settings_obj.h"
#include "monitor.h"
#include "ui_main_window.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
public:
    /** @name Конструкторы/деструкторы
      * @{
      */
	MainWindow( QWidget * parent = 0, Qt::WFlags f = 0 );
    ~MainWindow();
    /** @} */
private:
    /// Ссылка на объект окна настроек
    SettingsWindow * swindow;

    /// Ссылка на объект окна монитора
    MonitorWindow  * mwindow;

    /// Ссылка на объект, отвечающий за работу с настройками программы
    SettingsObj    * set_obj;

    /// Ссылка на объект, отвечающий за работу с моделью монитора
    Monitor        * monitor;
private slots:
};
#endif




