#include "utils.h"
#include "settings_obj.h"
#include <QDebug>
#include <QProgressDialog>

Utils::Utils()
{
    lib = NULL;
}

void Utils::setDevCount(short int count)
{
    dev_count = count;
}

short int Utils::getDevCount()
{
    return dev_count;
}

bool Utils::loadLibrary(QString file_name)
{
    
    lib = new QLibrary(file_name);
    
    if(lib->load())
    {
        qDebug("LOAD is OK");
    } else
    {
        qDebug("LOAD isn't ok");
        lib = NULL;
        return false;
    }

    R245_Init = (Init) lib->resolve("R245_Init");
    R245_CloseAllDev = (CloseAllDev) lib->resolve("R245_CloseAllDev");
    R245_GetNumDevs = (GetNumDevs) lib->resolve("R245_GetNumDevs");
    R245_GetDevInfo = (GetDevInfo) lib->resolve("R245_GetDevInfo");
    R245_CloseDev = (CloseDev) lib->resolve("R245_CloseDev");
    R245_InitDev = (InitDev) lib->resolve("R245_InitDev");
    R245_AuditEn = (AuditEn) lib->resolve("R245_AuditEn");
    R245_GetVersion = (GetVersion) lib->resolve("R245_GetVersion");
    R245_GetNumTrans = (GetNumTrans) lib->resolve("R245_GetNumTrans");
    R245_GetTransact = (GetTransact) lib->resolve("R245_GetTransact");
    R245_GetDamp = (GetDamp) lib->resolve("R245_GetDamp");
    R245_SetDamp = (SetDamp) lib->resolve("R245_SetDamp");
    R245_GetTime = (GetTime) lib->resolve("R245_GetTime");
    R245_SetTime = (SetTime) lib->resolve("R245_SetTime");
    R245_SetChan = (SetChan) lib->resolve("R245_SetChan");
    R245_GetChan = (GetChan) lib->resolve("R245_GetChan");
    R245_SetTimeRTC = (SetTimeRTC) lib->resolve("R245_SetTimeRTC");
    R245_SetDateRTC = (SetDateRTC) lib->resolve("R245_SetDateRTC");
    R245_SetAddr = (SetAddr) lib->resolve("R245_SetAddr");
    R245_ClearTrans = (ClearTrans) lib->resolve("R245_ClearTrans");
    
    return true;
}

void Utils::showMessage(QMessageBox::Icon icon, QString header, QString msg)
{
    QMessageBox* pmbx = new QMessageBox(
            icon,
            header,
            msg);

    QFont font;

    font.setPointSize(15);

    pmbx->setFont(font);
    pmbx->exec();
    delete pmbx;
}

bool Utils::unloadLibrary()
{
    if(lib != NULL)
    {
        if(lib->unload())
        {
            qDebug("UnLOAD is OK");
            delete lib;
            lib = NULL;
            
            return true;
        } else
        {
            qDebug("UnLOAD isn't OK");
        }
    }
    
    return false;
}

void Utils::changeAlias(QStandardItem * alias_item, QStandardItemModel * model, bool clear)
{
    QStandardItemModel * alias_model = alias_item->model();
    QStandardItem * item_model;
    QString alias_id = "";
    QString alias_name = "";

    int row_count = model->rowCount();
    int id_attr, name_attr;

    if(alias_model->objectName() == "tag_model")
    {
        alias_id = alias_model->index(alias_item->row(), SettingsObj::AliasId).data().toString();
        alias_name = alias_model->index(alias_item->row(), SettingsObj::AliasName).data().toString();
        if(model->objectName() == "monitor_model")
        {

            id_attr = Monitor::TagIdAttr;
            name_attr = Monitor::TagNameAttr;
        } else // event_model
        {

            id_attr = SettingsObj::EvIdTag;
            name_attr = SettingsObj::EvNameTag;
        }

    } else if(alias_model->objectName() == "dev_model")// dev_model
    {

        alias_id = alias_item->parent()->text() + " " +
                   alias_item->parent()->child(alias_item->row(), SettingsObj::AliasId)->text();
        alias_name = alias_item->parent()->child(alias_item->row(), SettingsObj::AliasName)->text();

        if(model->objectName() == "monitor_model")
        {
            qDebug() << "Dev_monitor_model";

            id_attr = Monitor::DevNumAttr;
            name_attr = Monitor::DevNameAttr;
        }
        else // event_model
        {
            id_attr = SettingsObj::EvIdDev;
            name_attr = SettingsObj::EvNameDev;
        }
    } else
    {
        showMessage(QMessageBox::Warning, "", "Смена псевдонимов не возможна");
        return;
    }


    QProgressDialog progress("Применение синонимов в зависимых таблицах", "&Cancel", 0, row_count-1);
    progress.setWindowTitle("Пожалуйста подождите...");
    progress.setMinimumDuration(0);
    progress.setAutoClose(true);
    progress.setModal(true);

    for(int row = 0; row < row_count; row++)
    {
        progress.setValue(row);
        qApp->processEvents();

        item_model = model->item(row, id_attr);

        if(item_model->text() == alias_id)
        {
            if(clear)
            {
                model->blockSignals(true); // чтобы не работали сигналы на изменение моделей (актуально для event_model)
                model->item(row, name_attr)->setText(alias_id);
                model->blockSignals(false);
            } else
            {
                // Очень много времени отнимает, если не активно окно монитора !!!
                model->item(row, name_attr)->setText(alias_name);
            }
        }
    }
    qDebug() << "Alias changed";
}

int Utils::timeToSec(QTime time)
{
    return (time.hour()*3600 +
           time.minute()*60 +
           time.second())*10;
}


QTime Utils::secToTime(int time_sec)
{
    int hour = time_sec / 36000;
    int min  = (time_sec/10%36000) / 60;
    int sec  = (time_sec/10%3600) % 60;

    return QTime(hour, min, sec);
}

bool Utils::openFile(QFile * file, QFlags<QIODevice::OpenModeFlag> mode)
{
    if(file == NULL)
        return false;

    if(!file->open(mode))
    {
        qDebug() << "Error: open file";
        return false;
    }

    return true;
}

bool Utils::closeFile(QFile *file)
{
    if(file != NULL)
    {
        if(file->isOpen())
        {
            file->close();
        }
        return true;
    }

    return false;
}
