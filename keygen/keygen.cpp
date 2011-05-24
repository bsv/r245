#include "keygen.h"

KeygenWindow::KeygenWindow(QWidget *parent)
    : QWidget(parent)
{
	setupUi(this);

	if(r245.loadLibrary())
	{
	    dev_list->setModel(&model);

        code_edit->setText(QString().setNum(POLYNOM));
        connect(save_button, SIGNAL(clicked()), SLOT(slotSave()));
        connect(get_id_btn, SIGNAL(clicked()), SLOT(slotGetId()));
        connect(add_dev_btn, SIGNAL(clicked()), SLOT(slotAddDev()));
	} else
	{
	    showMessage(QMessageBox::Warning, "Error", "Load library isn't OK!!!");
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

    if(!r245.R245_GetDevInfo(0, &info))
    {
        //dev_edit->setText(QString().setNum(info.id));
        dev_edit->setText(QString().setNum(67363761));
    }
}

void KeygenWindow::slotSave()
{
    QFile key_file("key.txt");
    ushort crc = 0;
    bool ok;
    ushort poly = code_edit->text().toInt(&ok);
    ulong id = 0;
    uchar byte_mas[4];

    if(!ok)
    {
        showMessage(QMessageBox::Warning, "Ошибка формата", "Кодовое число должно быть цифрой!!!");
        return;
    }

    openFile(&key_file, QIODevice::WriteOnly);
    QTextStream kstr(&key_file);

    for(int i = 0; i < str_list.size(); i++)
    {
        id = str_list[i].toLong(&ok);

        if(!ok)
        {
            showMessage(QMessageBox::Warning, "Ошибка формата", "Идентификатор должен быть числом!!!");
            break;
        }

        byte_mas[0] = id;
        byte_mas[1] = id >> 8;
        byte_mas[2] = id >> 16;
        byte_mas[3] = id >> 24;

        crc = crc16(byte_mas, 4, poly, 0xFFFF);

        kstr << QString().setNum(crc, 16) << endl;
    }

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
