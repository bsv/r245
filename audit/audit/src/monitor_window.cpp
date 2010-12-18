#include "monitor_window.h"
#include "global.h"

/**
  * @param set - ��������� �� ������, ���������� �� ��������� ����������.
  * @param mon - ��������� �� ������ ��������.
  * @param parent - ��������� �� ������ ������.
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
  * ������� ������ �������� � ��� monitor_view,
  * ������� ���������� ������ ������.
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
  * ����������� ������� ������ ���������� ���������. ����� ����������� �
  * ��������� ������.
  *
  * @param qdoc - ��������� �� ��������� ��������, �������������� � ������.
  * @param printer - ��������� �� ������ ��������.
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
  * ��������� ������ �������� � ������ ���������� ���������� �
  * � ���� csv.
  *
  * @param file_path - ��� ������������ �����.
  */
void MonitorWindow::printMonitor(QString file_path)
{
    if(future_watch.isRunning())
    {
        utils.showMessage(QMessageBox::Warning, "��������� ����������",
                          "���������� �������� � ������ ��� �� ���������.");
    } else
    {

        QFile file(file_path);

        if(!file.open(QIODevice::WriteOnly))
        {
            utils.showMessage(QMessageBox::Warning, "��������",
                              "���������� ��������� ���� � ������: <br>" + file_path);
            return;
        }

        QAbstractItemModel * model = monitor->getModel(true);

        int count_row = model->rowCount();

        QProgressDialog progress("��������� ������� ��������", "&Cancel", 0, count_row-1);
        progress.setWindowTitle("���������� ���������...");
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

                // ������� ��������� ������� �� ������
                // ����� ����������� ��� ������ �� ������ ������
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
  * ���������� ��������� � ���������� ������ � ���������� ������ ���������� �����.
  */
void MonitorWindow::slotPrintOK()
{
    qDebug("SAVE PDF");
    utils.showMessage(QMessageBox::Information, "����������/������",
                      "����������/������ ���������");
    save_file_button->setEnabled(true);
}

/**
  * ��������� ������ ���������������� �������� � ���� csv.
  */
void MonitorWindow::slotSaveFile()
{
    QString file_path = QFileDialog::getSaveFileName(0, "���������� ������", "", "*.csv");

    qDebug("Dialog");
    if(!file_path.isEmpty())
    {
        printer_pdf.setOutputFileName(file_path);
        printMonitor(file_path);
    }
}

/**
  * � ����������� �� ������ tag_check, � �������� ������������
  * ���� ��� �������, ���� �������, ��������� � �������.
  */
void MonitorWindow::slotTagInform()
{
    monitor->onlyTagInf(tag_check->isChecked());
}

/**
  * ��� ������������ �� ������� � ��������� ����������� �������� �������.
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
  * ������������� �������� ������� ��� ������� ��������.
  */
void MonitorWindow::setFilter()
{
    monitor->setFilter(numChanEdt->text(), numDevEdt->text(),
                       numTagEdt->text(), event_edt->text(),
                       sinceDateSpn->date(), toDateSpn->date(),
                       sinceTimeSpn->time(), toTimeSpn->time());
}

/**
  * ���������� �������� ������� �������.
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
  * ���� ���������� ����������� �� ������� �����������
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
                    // ��������� ������� ������ ����� ���� ���������� ����������
                    clear_button->setEnabled(false);

                    QString id = model->item(dev_num)->data(Qt::DisplayRole).toString() + " " + addr;

                    QString tag_name = "", dev_name = "";

                    set_obj->findTagAlias(QString().setNum(trans.tid), &tag_name);
                    set_obj->findDevAlias(id, &dev_name);

                    monitor->addTransToModel(id, &trans, "", ""/*tag_name, dev_name*/);
                    set_obj->addLogNode(id, &trans); // add node to log file
                    eventHandler(id, &trans, tag_name, dev_name);
                    monitor_view->resizeColumnsToContents();

                    monitor->update(); // ��� ������ ���������� ��������� ��� ������� (��� �����)

                    if(trans_ctr == MAX_TRANS_FOR_TIMER_INT)
                    {
                        break; // ������� ������ �� ����� while
                               // � ������� ���������� ���������
                               // �� MAX_TRANS_FOR_TIMER_INT ����������
                    }
                    trans_ctr++;
                }
            }
        }
        clear_button->setEnabled(true);
    }
}

/**
  * ���������� ��������� �� ������ ��������.
  *
  * @return Monitor *
  */
Monitor * MonitorWindow::getMonitor()
{
    return monitor;
}

/**
  * ������������ ���������� �� ����������� ���������� � ������������ �
  * �������������� ����������� �������.
  *
  * @param dev_num - �������� ������������� ����������.
  * @param trans - ��������� �� ���������, �������� ���������� � �������� ����������.
  * @param tag_name - ������� ����, ���� ���, �� ���������� �� ��������� ������ ������
  * @param dev_name - ������� ����������, ���� ���, �� ���������� �� ��������� ������ ������
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
                if(react == "�������� ������")
                {
                    for(int i = 0; i < monitor_model->columnCount(); ++i)
                    {
                        monitor_model->item(0, i)->setBackground(event_react->background());
                    }
                } else if(react == "�������� ���������")
                {
                    QString msg = "<table><tr><td>�������: </td><td>" + event_msg + "</td></tr>" +
                                  "<tr><td>���: </td><td>" + id_tag + "</td></tr>";
                    if(tag_name != "")
                    {
                        msg += "<tr><td>��� ����: </td><td>" + tag_name + "</td></tr>";
                    }
                    msg += "<tr><td>����������: </td><td>" + id_dev + "</td></tr>";
                    if(dev_name != "")
                    {
                        msg += "<tr><td>��� ����������: </td><td>" + dev_name + "</td></tr>";
                    }
                    msg += "<tr><td>�����: </td><td>" + chanell + "</td></tr></table>";


                    utils.showMessage(QMessageBox::Information, event_name, msg);

                    if(event_name == "��� �������")
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
  * ���������� ��������� ��� �������� ���������� ��� ��������
  * ������� ���� ��������.
  */
MonitorWindow::~MonitorWindow()
{
    utils.R245_CloseAllDev();
}
