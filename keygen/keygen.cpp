#include "keygen.h"

KeygenWindow::KeygenWindow(QWidget *parent)
    : QWidget(parent)
{
	setupUi(this);

	if(r245.loadLibrary())
	{
	    dev_list->setModel(&model);

        code_edit->setText(QString().setNum(POLYNOM));
        num_edit->setText("0");
        connect(save_button, SIGNAL(clicked()), SLOT(slotSave()));
        connect(get_id_btn, SIGNAL(clicked()), SLOT(slotGetId()));
        connect(add_dev_btn, SIGNAL(clicked()), SLOT(slotAddDev()));
	} else
	{
	    showMessage(QMessageBox::Warning, "Error", "Load library isn't OK: lib/libr245.dll!!!");
	    exit(1);
	}
}

void KeygenWindow::slotAddDev()
{
    if(dev_edit->text() != "")
    {
        if(str_list.indexOf(dev_edit->text()) == -1)
        {
            str_list << dev_edit->text();
            model.setStringList(str_list);
        }
    }
}

void KeygenWindow::slotGetId()
{
    R245_DEV_INFO info;
    bool ok = false;

    uchar dev_num = num_edit->text().toInt(&ok);

    if(!ok)
       dev_num = 0;


    if(!r245.R245_GetDevInfo(dev_num, &info))
    {
        QString snumber = "";

        for(int i = 0; i < 16; i++)
        {
            snumber += QString().setNum((uchar)info.serial_number[i], 16);
        }

        r245.R245_InitDev(dev_num);
        dev_edit->setText(snumber);
    }
}

void KeygenWindow::slotSave()
{
    QFile key_file("key.txt");
    ushort crc = 0;
    bool ok;
    ushort poly = code_edit->text().toInt(&ok);
    uchar dig = 0;

    if(!ok)
    {
        showMessage(QMessageBox::Warning, "Ошибка формата", "Кодовое число должно быть цифрой!!!");
        return;
    }

    openFile(&key_file, QIODevice::WriteOnly);
    QTextStream kstr(&key_file);

    for(int i = 0; i < str_list.size(); i++)
    {
        crc = 0xFFFF;

        for(int j = 0; j < str_list[i].size(); j++)
        {
            dig = (str_list[i])[j].toAscii();
            crc = crc16(&dig, 1, poly, crc);
        }

        kstr << QString().setNum(crc, 16) << endl;
    }
    showMessage(QMessageBox::Information, "Успех", "Ключ успешно создан!!!");

    closeFile(&key_file);
}

unsigned short int KeygenWindow::crc16(unsigned char *mes, int size, unsigned short int polinom, unsigned short int start_crc)
{
    unsigned short int crc = start_crc, i = 0;

    for(i = 0; i < size; i++){
        crc = calcTab(((crc >> 8) ^ mes[i]) & 0xFF, polinom) ^ (crc << 8);
    }
    return crc;
}


unsigned short int KeygenWindow::calcTab(int i, unsigned short polinom)
{
    unsigned short int r = i << 8;
    int bit = 0;

    for(bit = 0; bit < 8; bit++)
    {
        if(r & 0x8000)
            r = (r << 1) ^ polinom;
        else
            r <<= 1;
    }
    return r;
}


bool KeygenWindow::openFile(QFile * file, QFlags<QIODevice::OpenModeFlag> mode)
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

bool KeygenWindow::closeFile(QFile *file)
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


void KeygenWindow::showMessage(QMessageBox::Icon icon, QString header, QString msg)
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

KeygenWindow::~KeygenWindow()
{

}
