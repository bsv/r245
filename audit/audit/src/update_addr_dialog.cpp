#include "update_addr_dialog.h"
#include "global.h"

UpdateAddrDialog::UpdateAddrDialog(QWidget *parent, uchar cur_addr)
    : QDialog(parent)
{
	setupUi(this);

	cur_addr_spin->setValue(cur_addr);

	connect(upd_btn, SIGNAL(clicked()), SLOT(slotUpdate()));
	connect(cancel_btn, SIGNAL(clicked()), SLOT(reject()));
}

void UpdateAddrDialog::slotUpdate()
{
    if(cur_addr_spin->value() == 3 ||
            targ_addr_spin->value() == 3)
    {
        utils.showMessage(QMessageBox::Warning, "Обновление адреса",
                "Адрес устройства должен быть в диапазоне от 1 до 254. Адреса 0, 3, 255 зарезервированы");
    } else
    {
        accept();
    }
}

uchar UpdateAddrDialog::getCurAddr()
{
    return cur_addr_spin->value();
}

uchar UpdateAddrDialog::getTargAddr()
{
    return targ_addr_spin->value();
}

UpdateAddrDialog::~UpdateAddrDialog()
{

}
