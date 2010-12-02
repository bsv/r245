/**
  * @file r245_types.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание типов, используемых в приложении.
  */

#ifndef R245_TYPES_H
#define R245_TYPES_H

#include <QString>

/// Признак успешного завершения функций работы со считывателем
#define R245_OK 0

/**
  * Структура, хранящая информацию о устройстве.
  * Аналогична структуре библиотеки libr245dll.dll
  * для работы с устройством считывателя.
  */
typedef struct struct_r245_dev_info
{
    unsigned long flags;
    unsigned long id;
    unsigned long loc_id;
    unsigned long type;
    char serial_number[16];
    char desc[64];
    void * ft_handle;

} R245_DEV_INFO;

/**
  * Структура, хранящая информацию о настройках устройства считывателя.
  */
typedef struct struct_dev_info_qt
{
    unsigned char addr;
    bool active;
    unsigned char channel;
    short int time1;
    unsigned char dist1;
    short int time2;
    unsigned char dist2;
    QString name;
} DEV_INFO;

/**
  * Структура, хранящая информацию о транзакции, полученной от считывателя.
  * Аналогична структуре библиотеки libr245dll.dll.
  */
typedef struct struct_transact
{
    short unsigned int code;
    unsigned char channel;
    unsigned long tid;
    unsigned char day;
    unsigned char month;
    short unsigned int year;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned char dow;
} R245_TRANSACT;

/**
  * Структура, хранящая информацию о настройках времени внутренних
  * часов считывателя.
  * Аналогична структуре библиотеки libr245dll.dll.
  */
typedef struct struct_rtc
{
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned char dow;
    unsigned short int year;
    unsigned char month;
    unsigned char day;
} R245_RTC;

/** @name Определение типов функций, соответствующих функциям библиотеки libr245dll.dll
  * @{
  */
typedef short int (*InitDev) (unsigned char);
typedef short int (*AuditEn) (unsigned char, unsigned char, unsigned char);
typedef short int (*GetVersion) (unsigned char, unsigned char, unsigned char *);
typedef short int (*Init)();
typedef short int (*GetDevInfo)(unsigned char, R245_DEV_INFO *);
typedef short int (*CloseDev) (unsigned char);
typedef short int (*CloseAllDev) ();
typedef short int (*GetNumDevs) ();
typedef short int (*GetNumTrans) (unsigned char, unsigned char, short unsigned int *);
typedef short int (*GetTransact) (unsigned char, unsigned char, R245_TRANSACT *);
typedef short int (*GetDamp) (unsigned char, unsigned char, unsigned char, unsigned char *);
typedef short int (*SetDamp) (unsigned char, unsigned char, unsigned char, unsigned char);
typedef short int (*GetTime) (unsigned char, unsigned char, unsigned char, short int *);
typedef short int (*SetTime) (unsigned char, unsigned char, unsigned char, short int);
typedef short int (*SetChan) (unsigned char, unsigned char, unsigned char, unsigned char);
typedef short int (*GetChan) (unsigned char, unsigned char, unsigned char *);
typedef short int (*SetTimeRTC) (unsigned char, unsigned char, R245_RTC *);
typedef short int (*SetDateRTC) (unsigned char, unsigned char, R245_RTC *);
typedef short int (*SetAddr) (unsigned char, unsigned char);
typedef short int (*ClearTrans) (unsigned char, unsigned char);
/** @} */

#endif // R245_TYPES_H
