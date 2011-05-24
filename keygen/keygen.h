#ifndef KEYGEN_H
#define KEYGEN_H

#include <QtGui/QWidget>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QStringListModel>
#include <QStringList>
#include "ui_keygen.h"
#include "R245.h"

#define POLYNOM 0x1021

class KeygenWindow : public QWidget, public Ui::keygenClass
{
    Q_OBJECT

public:
    KeygenWindow(QWidget *parent = 0);
    ~KeygenWindow();

private:
    R245 r245;
    QStringListModel model;
    QStringList str_list;

    bool openFile(QFile * file, QFlags<QIODevice::OpenModeFlag> mode);
    bool closeFile(QFile *file);
    unsigned short int crc16(unsigned char *mes, int size, unsigned short int polinom, unsigned short int start_crc);
    unsigned short int calcTab(int i, unsigned short polinom);
    void showMessage(QMessageBox::Icon icon, QString header, QString msg);

private slots:
    void slotSave();
    void slotGetId();
    void slotAddDev();
};

#endif // KEYGEN_H
