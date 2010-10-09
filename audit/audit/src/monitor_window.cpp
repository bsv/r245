#include "monitor_window.h"
#include "global.h"
#include <QObject>

MonitorWindow::MonitorWindow(SettingsObj * set, Monitor * mon, QWidget *parent):
        QDialog(parent)
{
    setupUi(this);

    set_obj = set;
    monitor = mon;

    connect(&future_watch, SIGNAL(finished()), SLOT(slotPrintOK()));

    initPdfPrinter(&printer_pdf);

    monitor_view->setModel(monitor->getModel(true));
    monitor_view->hideColumn(Monitor::DevNumAttr);
    monitor_view->hideColumn(Monitor::TagIdAttr);
    monitor_view->hideColumn(Monitor::TransCodeAttr);
    monitor_view->verticalHeader()->hide();
    monitor_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(&timer, SIGNAL(timeout()), SLOT(slotUpdateTrans()));
    connect(resetFilterBtn, SIGNAL(clicked()), SLOT(slotResetFilter()));
    connect(mw_tabs, SIGNAL(currentChanged(int)), SLOT(slotTabChanged()));
    connect(tag_check, SIGNAL(clicked()), SLOT(slotTagInform()));
    connect(save_file_button, SIGNAL(clicked()), SLOT(slotSaveFile()));
    connect(clear_button, SIGNAL(clicked()), SLOT(slotClearMonitor()));

    timer.start(1000);
    slotResetFilter();
    setFilter();
}

void MonitorWindow::initPdfPrinter(QPrinter * printer)
{
    printer->setOutputFormat(QPrinter::PdfFormat);
}

void MonitorWindow::slotClearMonitor()
{
    monitor->clear();

    monitor_view->hideColumn(Monitor::DevNumAttr);
    monitor_view->hideColumn(Monitor::TagIdAttr);
    monitor_view->hideColumn(Monitor::TransCodeAttr);
    monitor_view->verticalHeader()->hide();
}

void MonitorWindow::printThreadFunc(QTextDocument * qdoc, QPrinter * printer)
{
    QTextDocument * doc = qdoc->clone();

    QTime time;
    time.start();
    doc->print(printer);
    qDebug() << time.elapsed();
}

void MonitorWindow::setHtmlTh(QTextDocument * qdoc, QString text)
{
    QTextDocument * doc = qdoc->clone();
    doc->setHtml(text);
}

//print thread==========================
/*PrintThread::PrintThread()
{
    doc = NULL;
}

void PrintThread::start(QTextDocument *qdoc, QPrinter *pr, Priority priority)
{
    delete doc;

    doc = qdoc->clone();
    doc->moveToThread(this);
    printer = pr;
    QThread::start(priority);
}

void PrintThread::run()
{
    QTime time;
    time.start();
    doc->print(printer);
    qDebug() << time.elapsed();

    qDebug("Print finished");

    delete doc;
    doc = NULL;
}*/

//print thread==========================


void MonitorWindow::printMonitor(QPrinter * printer)
{
    if(future_watch.isRunning())
    {
        utils.showMessage(QMessageBox::Warning, "Подождите пожалуйста",
                          "Предыдущая операция с файлом ещё не завершена.");
    } else
    {
        QString text;
        QAbstractItemModel * model = monitor->getModel(true);

        int count_row = model->rowCount();

        text = "<table bgcolor='#000000' cellpadding=3 cellspacing=2>";

        text += "<tr bgcolor='#e5e5e5'><td>Время</td><td>Дата</td><td>Имя устройства</td>";
        text += "<td>id устройства</td><td>Канал</td><td>Имя метки</td>";
        text += "<td>id метки</td><td>Тип события</td><td>Код события</td></tr>";

        qApp->processEvents();
        QProgressDialog progress("Обработка событий монитора", "&Cancel", 0, count_row-1);
        progress.setWindowTitle("Пожалуйста подождите...");
        progress.setMinimumDuration(0);
        progress.setAutoClose(true);

        for(int row = 0; row < count_row; row++)
        {
            qApp->processEvents();
            progress.setValue(row);

            text += "<tr>";
            for(int column = 0; column <= Monitor::TransCodeAttr; column++)
            {
                text += "<td bgcolor='#ffffff'>" + model->index(row, column).data().toString() + "</td>";
            }

            text += "</td>";
        }

        text += "</table>";


        qdoc.setHtml(text);
        qApp->processEvents();
        utils.showMessage(QMessageBox::Information, "Сохранение/печать",
                          "Продолжайте работу с программой. По завершении операции будет показано сообщение");

        QFuture<void> future = QtConcurrent::run(printThreadFunc, &qdoc, printer);
        future_watch.setFuture(future);
        save_file_button->setEnabled(false);
        /*qApp->processEvents();
        print_thread.start(&qdoc, printer, QThread::TimeCriticalPriority);
        qApp->processEvents();*/
    }
}

void MonitorWindow::slotPrintOK()
{
    qDebug("SAVE PDF");
    utils.showMessage(QMessageBox::Information, "Сохранение/печать",
                      "Сохранение/печать завершена");
    save_file_button->setEnabled(true);
}

void MonitorWindow::slotSaveFile()
{
    QString file_path = QFileDialog::getSaveFileName(0, "Сохранение отчета", "", "*.pdf");

    qDebug("Dialog");
    if(!file_path.isEmpty())
    {
        printer_pdf.setOutputFileName(file_path);
        printMonitor(&printer_pdf);
    }
}

void MonitorWindow::slotTagInform()
{
    monitor->onlyTagInf(tag_check->isChecked());
}

void MonitorWindow::slotTabChanged()
{
    if(monitor_tab->isVisible())
    {
        qDebug("apply filter");
        setFilter();
    }
}

void MonitorWindow::setFilter()
{
    monitor->setFilter(numChanEdt->text(), numDevEdt->text(), numTagEdt->text(), sinceDateSpn->date(),
                       toDateSpn->date(), sinceTimeSpn->time(), toTimeSpn->time());
}

void MonitorWindow::slotResetFilter()
{
    qDebug("reset filter");
    numChanEdt->setText("");
    numDevEdt->setText("");
    numTagEdt->setText("");
    sinceDateSpn->setDate(QDate().fromString("01.09.2010", Qt::LocalDate));
    toDateSpn->setDate(QDate().currentDate());
    sinceTimeSpn->setTime(QTime().fromString("00:00:00"));
    toTimeSpn->setTime(QTime().fromString("23:59:59"));
}

void MonitorWindow::slotUpdateTrans()
{
    if(this->isActiveWindow())
    {
        R245_TRANSACT trans;
        short int status = 0;
        int dev_count = set_obj->getModel(SettingsObj::DevModel)->rowCount();
        short trans_ctr = 0;


        // !!! Исправить цикл идет по всем устройствам, надо только по подключенным и активным
        for(int dev_num = 0; dev_num < dev_count; dev_num++)
        {
            while(!(status = utils.R245_GetTransact(dev_num, &trans)))
            {
                QString tag_name = "", dev_name = "";
                QAbstractItemModel * tag_model = set_obj->getModel(SettingsObj::TagModel);
                QAbstractItemModel * dev_name_model = set_obj->getModel(SettingsObj::DevNameModel);

                utils.findAlias(tag_model, QString().setNum(trans.tid), &tag_name);
                utils.findAlias(dev_name_model, QString().setNum(dev_num), &dev_name);

                monitor->addTransToModel(QString().setNum(dev_num), &trans, tag_name, dev_name);
                set_obj->addLogNode(QString().setNum(dev_num), &trans); // add node to log file
                eventHandler(QString().setNum(dev_num), &trans);

                monitor->update(); // При каждой транзакции сортирует всю таблицу (это плохо)

                if(trans_ctr == MAX_TRANS_FOR_TIMER_INT)
                {
                    break; // выходит только из цикла while
                           // с каждого устройства считываем
                           // по MAX_TRANS_FOR_TIMER_INT транзакций
                }

                trans_ctr++;
            }
        }
        //trans_thread->run();
    } //else trans_thread->exit();
}

Monitor * MonitorWindow::getMonitor()
{
    return monitor;
}

void MonitorWindow::eventHandler(QString dev_num, R245_TRANSACT *trans)
{
    QStandardItemModel * event_model = (QStandardItemModel *)set_obj->getModel(SettingsObj::EventModel);

    int row_count = event_model->rowCount();
    QMap <int, QString> * state = monitor->getState();

    for(int row = 0; row < row_count; row++)
    {
        QString event_name = event_model->data(event_model->index(row, SettingsObj::EvEvent)).toString();
        if(event_name == QString("%1").arg((*state)[trans->code]))
        {
            QString id_dev = event_model->data(event_model->index(row, SettingsObj::EvIdDev)).toString();
            QString chanell = event_model->data(event_model->index(row, SettingsObj::EvChanell)).toString();
            QString id_tag = event_model->data(event_model->index(row, SettingsObj::EvIdTag)).toString();
            QString event_msg = event_model->data(event_model->index(row, SettingsObj::EvName)).toString();

            if(
                 (dev_num == id_dev) &&
                 (QString().setNum(trans->channel) == chanell) &&
                 (QString().setNum(trans->tid) == id_tag)
              )
            {
                QStandardItem * event_react = event_model->item(row, SettingsObj::EvReact);
                QString react = event_react->text();
                QStandardItemModel * monitor_model = (QStandardItemModel *) monitor->getModel(false);
                if(react == "выделить цветом")
                {
                    for(int i = 0; i < monitor_model->columnCount(); ++i)
                    {
                        monitor_model->item(0, i)->setBackground(event_react->background());
                    }
                } else if(react == "показать сообщение")
                {
                    QString msg = "<table><tr><td>Событие: </td><td>" + event_msg + "</td></tr>" +
                                  "<tr><td>Таг: </td><td>" + id_tag + "</td></tr>" +
                                  "<tr><td>Устройство: </td><td>" + id_dev + "</td></tr>" +
                                  "<tr><td>Канал: </td><td>" + chanell + "</td></tr></table>";


                    utils.showMessage(QMessageBox::Information, event_name, msg);
                    trans->code = 0x12F;
                    set_obj->addLogNode(0, trans);
                }
            }
        }
    }
}

MonitorWindow::~MonitorWindow()
{
    utils.R245_CloseAllDev();
}
