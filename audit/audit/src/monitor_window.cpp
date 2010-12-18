#include "monitor_window.h"
#include "global.h"

/**
  * @param set - указатель на объект, отвечающий за настройки приложения.
  * @param mon - указатель на объект монитора.
  * @param parent - указатель на объект предка.
  */
MonitorWindow::MonitorWindow(SettingsObj * set, Monitor * mon, QWidget *parent):
        QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

    set_obj = set;
    monitor = mon;

    connect(&future_watch, SIGNAL(finished()), SLOT(slotPrintOK()));

    printer_pdf.setOutputFormat(QPrinter::PdfFormat);

    monitor_view->setModel(monitor->getModel(true));
    monitor_view->hideColumn(Monitor::DevNumAttr);
    monitor_view->hideColumn(Monitor::TagIdAttr);
    monitor_view->hideColumn(Monitor::TransCodeAttr);
    monitor_view->verticalHeader()->hide();
    monitor_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    monitor_view->resizeColumnsToContents();

    connect(&timer, SIGNAL(timeout()), SLOT(slotUpdateTrans()));
    connect(resetFilterBtn, SIGNAL(clicked()), SLOT(slotResetFilter()));
    connect(mw_tabs, SIGNAL(currentChanged(int)), SLOT(slotTabChanged()));
    connect(tag_check, SIGNAL(clicked()), SLOT(slotTagInform()));
    connect(save_file_button, SIGNAL(clicked()), SLOT(slotSaveFile()));
    connect(clear_button, SIGNAL(clicked()), SLOT(slotClearMonitor()));

    connect(((QStandardItemModel *)monitor->getModel(false)), SIGNAL(itemChanged(QStandardItem*)), SLOT(slotMonitorChanged(QStandardItem *)));

    timer.start(REQ_PERIOD);
    slotResetFilter();
    setFilter();
}

void MonitorWindow::slotMonitorChanged(QStandardItem *)
{
    monitor_view->resizeColumnsToContents();
}

/**
  * Очищает модель монитора и вид monitor_view,
  * который отображает данные модели.
  */
void MonitorWindow::slotClearMonitor()
{
    monitor->clear();

    monitor_view->hideColumn(Monitor::DevNumAttr);
    monitor_view->hideColumn(Monitor::TagIdAttr);
    monitor_view->hideColumn(Monitor::TransCodeAttr);
    monitor_view->verticalHeader()->hide();
}

/**
  * Статическая функция печати текстового документа. Может выполняться в
  * отдельном потоке.
  *
  * @param qdoc - указатель на текстовый документ, приготовленный к печати.
  * @param printer - указатель на объект принтера.
  */
void MonitorWindow::printThreadFunc(QTextDocument * qdoc, QPrinter * printer)
{
    QTextDocument * doc = qdoc->clone();

    //QTime time;
    //time.start();
    doc->print(printer);
    //qDebug() << time.elapsed();
}

/**
  * Сохраняет данные монитора с учетом параметров фильтрации в
  * в файл csv.
  *
  * @param file_path - имя сохраняемого файла.
  */
void MonitorWindow::printMonitor(QString file_path)
{
    if(future_watch.isRunning())
    {
        utils.showMessage(QMessageBox::Warning, "Подождите пожалуйста",
                          "Предыдущая операция с файлом ещё не завершена.");
    } else
    {

        QFile file(file_path);

        if(!file.open(QIODevice::WriteOnly))
        {
            utils.showMessage(QMessageBox::Warning, "Внимание",
                              "Невозможно сохранить файл с именем: <br>" + file_path);
            return;
        }

        QAbstractItemModel * model = monitor->getModel(true);

        int count_row = model->rowCount();

        QProgressDialog progress("Обработка событий монитора", "&Cancel", 0, count_row-1);
        progress.setWindowTitle("Пожалуйста подождите...");
        progress.setMinimumDuration(0);
        progress.setAutoClose(true);
        progress.setModal(true);

        QString data = "";
        QString value = "";

        // write table header
        for(int column = 0; column <= Monitor::TypeEventAttr; column++)
        {
            data += model->headerData(column, Qt::Horizontal).toString() + ";";
        }

        data += "\n";

        // write table data
        for(int row = 0; row < count_row; row++)
        {
            progress.setValue(row);

            for(int column = 0; column <= Monitor::TypeEventAttr; column++)
            {
                qApp->processEvents();
                value = model->index(row, column).data().toString();

                // Удаляем служебные символы из строки
                // Могут встречаться при чтении из пустой ячейки
                value.remove("\n");
                value.remove("\r");

                data += value + ";";
            }
            data += "\n";
        }

        //save file
        QTextStream stream(&file);
        stream << data;
        file.close();
    }
}


/**
  * Отображает сообщение о завершении печати и активирует кнопку сохранения файла.
  */
void MonitorWindow::slotPrintOK()
{
    qDebug("SAVE PDF");
    utils.showMessage(QMessageBox::Information, "Сохранение/печать",
                      "Сохранение/печать завершена");
    save_file_button->setEnabled(true);
}

/**
  * Сохраняет данные отфильтрованного монитора в файл csv.
  */
void MonitorWindow::slotSaveFile()
{
    QString file_path = QFileDialog::getSaveFileName(0, "Сохранение отчета", "", "*.csv");

    qDebug("Dialog");
    if(!file_path.isEmpty())
    {
        printer_pdf.setOutputFileName(file_path);
        printMonitor(file_path);
    }
}

/**
  * В зависимости от выбора tag_check, в мониторе отображаются
  * либо все события, либо события, связанные с метками.
  */
void MonitorWindow::slotTagInform()
{
    monitor->onlyTagInf(tag_check->isChecked());
}

/**
  * При переключении на вкладку с монитором обнавляется значение фильтра.
  */
void MonitorWindow::slotTabChanged()
{
    if(monitor_tab->isVisible())
    {
        qDebug("apply filter");
        setFilter();
    }
}

/**
  * Устанавливает значение фильтра для событий монитора.
  */
void MonitorWindow::setFilter()
{
    monitor->setFilter(numChanEdt->text(), numDevEdt->text(),
                       numTagEdt->text(), event_edt->text(),
                       sinceDateSpn->date(), toDateSpn->date(),
                       sinceTimeSpn->time(), toTimeSpn->time());
}

/**
  * Сбрасывает значение фильтра событий.
  */
void MonitorWindow::slotResetFilter()
{
    qDebug("reset filter");
    numChanEdt->setText("");
    numDevEdt->setText("");
    numTagEdt->setText("");
    event_edt->setText("");
    sinceDateSpn->setDate(QDate().fromString("01.09.2010", Qt::LocalDate));
    toDateSpn->setDate(QDate().currentDate());
    sinceTimeSpn->setTime(QTime().fromString("00:00:00"));
    toTimeSpn->setTime(QTime().fromString("23:59:59"));
}

/**
  * Цикл считывания транзакиций из журнала считывателя
  */
void MonitorWindow::slotUpdateTrans()
{
    if(this->isActiveWindow())
    {
        R245_TRANSACT trans;
        short int status = 0;
        QStandardItemModel * model = (QStandardItemModel *) set_obj->getModel(SettingsObj::DevTypeModel);
        int dev_count = set_obj->getModel(SettingsObj::DevTypeModel)->rowCount();
        short trans_ctr = 0;

        for(int dev_num = 0; dev_num < dev_count; dev_num++)
        {
            for(int dev_row = 0; dev_row < model->item(dev_num)->rowCount(); dev_row++)
            {
                int addr = model->item(dev_num)->child(dev_row)->data(Qt::DisplayRole).toInt();
                trans_ctr = 0;
                while(!(status = utils.R245_GetTransact(dev_num, addr, &trans)))
                {
                    // Запрещаем очищать модель когда идет считывание транзакций
                    clear_button->setEnabled(false);

                    QString id = model->item(dev_num)->data(Qt::DisplayRole).toString() + " " + addr;

                    QString tag_name = "", dev_name = "";

                    set_obj->findTagAlias(QString().setNum(trans.tid), &tag_name);
                    set_obj->findDevAlias(id, &dev_name);

                    monitor->addTransToModel(id, &trans, "", ""/*tag_name, dev_name*/);
                    set_obj->addLogNode(id, &trans); // add node to log file
                    eventHandler(id, &trans, tag_name, dev_name);
                    monitor_view->resizeColumnsToContents();

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
        }
        clear_button->setEnabled(true);
    }
}

/**
  * Возвращает указатель на объект монитора.
  *
  * @return Monitor *
  */
Monitor * MonitorWindow::getMonitor()
{
    return monitor;
}

/**
  * Обрабатывает полученные от считывателя транзакции в соответствии с
  * установленными настройками событий.
  *
  * @param dev_num - числовой идентификатор устройства.
  * @param trans - указатель на структуру, хранящей информацию о принятой транзакции.
  * @param tag_name - синоним тега, если нет, то передается по умолчанию пустая строка
  * @param dev_name - синоним устройства, если нет, то передается по умолчанию пустая строка
  */
void MonitorWindow::eventHandler(QString dev_num, R245_TRANSACT *trans, QString tag_name, QString dev_name)
{
    QStandardItemModel * event_model = (QStandardItemModel *)set_obj->getModel(SettingsObj::EventTypeModel);

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
                                  "<tr><td>Таг: </td><td>" + id_tag + "</td></tr>";
                    if(tag_name != "")
                    {
                        msg += "<tr><td>Имя тага: </td><td>" + tag_name + "</td></tr>";
                    }
                    msg += "<tr><td>Устройство: </td><td>" + id_dev + "</td></tr>";
                    if(dev_name != "")
                    {
                        msg += "<tr><td>Имя устройства: </td><td>" + dev_name + "</td></tr>";
                    }
                    msg += "<tr><td>Канал: </td><td>" + chanell + "</td></tr></table>";


                    utils.showMessage(QMessageBox::Information, event_name, msg);

                    if(event_name == "таг потерян")
                    {
                        trans->code = 0x12F;
                    } else
                    {
                        trans->code = 0x130;
                    }

                    set_obj->addLogNode(dev_num, trans);
                    monitor->addTransToModel(dev_num, trans, tag_name, dev_name);
                }
            }
        }
    }
}

/**
  * Деструктор закрывает все открытые устройства при удалении
  * объекта окна монитора.
  */
MonitorWindow::~MonitorWindow()
{
    utils.R245_CloseAllDev();
}
