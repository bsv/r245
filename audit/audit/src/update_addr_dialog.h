#ifndef UPDATE_ADDR_DIALOG_H
#define UPDATE_ADDR_DIALOG_H

#include <QtGui/QDialog>
#include "ui_update_addr_dialog.h"

class UpdateAddrDialog : public QDialog, public Ui::UpdateAddrDialogClass
{
    Q_OBJECT

public:
    UpdateAddrDialog(QWidget *parent = 0, uchar cur_addr = 0);
    ~UpdateAddrDialog();

    uchar getCurAddr();
    uchar getTargAddr();

private slots:
    void slotUpdate();

};

#endif // UPDATE_ADDR_DIALOG_H
