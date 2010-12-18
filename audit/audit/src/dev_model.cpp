#include "dev_model.h"

DevModel::DevModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

void DevModel::addReader(unsigned char dev_num, QList<QStandardItem * > &items)
{
    if(dev_map.find(dev_num) == dev_map.end())
    {
        dev_map[dev_num] = new QList<QString>();
    }

    dev_map[dev_num]->append(items[1]->text());
    this->item(dev_num)->appendRow(items);
    updateDevList();
}

void DevModel::delReader(unsigned char dev_num, unsigned char reader_num)
{
    dev_map[dev_num]->removeAt(reader_num);

    this->item(dev_num)->removeRow(reader_num);
    updateDevList();
}

void DevModel::changeReader(unsigned char dev_num, unsigned char reader_num)
{
    (*dev_map[dev_num])[reader_num] = this->item(dev_num)->child(reader_num, 1)->text();
    updateDevList();
}

QList<QString> * DevModel::getDevList()
{
    return &dev_list;
}

void DevModel::updateDevList()
{
    dev_list.clear();

    QMap<uchar, QList<QString> *>::iterator dev_root = dev_map.begin();

    for(; dev_root != dev_map.end(); dev_root++)
    {
        QList<QString>::iterator dev = (*dev_root)->begin();

        for(; dev != (*dev_root)->end(); dev++)
        {
            dev_list.append(*dev);
        }
    }
}
