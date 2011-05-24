/**
  * @file utils.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса утилит. Возможности класса используются
  * для решения вспомогательных задач приложения.
  */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <QString>
#include <QLibrary>
#include <QAbstractItemModel>
#include <QTime>
#include <QFile>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
#include "r245_types.h"

#define POLYNOM 0x1021

class Utils
{
private:
    QLibrary *lib;
    QStringList * dev_list; // указатель на список устройств, с которыми разрешена работа
                            // в рамках данной программы. Список получается из key.txt в
                            // зашифрованном виде

    unsigned short calcTab(int i, unsigned short polinom);

public:
    Utils();

    bool loadLibrary(QString file_name);
    bool unloadLibrary();

    int timeToSec(QTime time);
    QTime secToTime(int time_sec);
    void showMessage(QMessageBox::Icon icon, QString header, QString msg);
    bool openFile(QFile * file, QFlags<QIODevice::OpenModeFlag> mode);
    bool closeFile(QFile *file);
    void setDevList(QStringList * str_list);
    QStringList * getDevList();
    unsigned short crc16(unsigned char *mes, int size,
                    unsigned short int polinom, unsigned short int start_crc);

    // Import functions from libr245dll.dll
    /** @name Определение имен функций, импортируемых из библиотеки libr245dll.dll
     * @{
     */

    Init R245_Init;
    CloseAllDev R245_CloseAllDev;
    GetNumDevs R245_GetNumDevs;
    GetDevInfo R245_GetDevInfo;
    CloseDev R245_CloseDev;
    InitDev R245_InitDev;
    AuditEn R245_AuditEn;
    GetVersion R245_GetVersion;
    GetNumTrans R245_GetNumTrans;
    GetTransact R245_GetTransact;
    GetDamp R245_GetDamp;
    SetDamp R245_SetDamp;
    GetTime R245_GetTime;
    SetTime R245_SetTime;
    SetChan R245_SetChan;
    GetChan R245_GetChan;
    SetTimeRTC R245_SetTimeRTC;
    SetDateRTC R245_SetDateRTC;
    SetAddr R245_SetAddr;
    ClearTrans R245_ClearTrans;
    /** @} */
};

#endif // __UTILS_H__
