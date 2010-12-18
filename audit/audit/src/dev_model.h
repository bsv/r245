#ifndef DEV_MODEL_H
#define DEV_MODEL_H

#include <QStandardItemModel>

class DevModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit DevModel(QObject *parent = 0);

    void addReader(unsigned char dev_num, QList<QStandardItem * > &items);
    void delReader(unsigned char dev_num, unsigned char reader_num);
    void changeReader(unsigned char dev_num, unsigned char reader_num);
    QList<QString> * getDevList();
    void updateDevList();

private:
    QMap <uchar, QList<QString> *> dev_map;
    QList<QString> dev_list;

signals:

public slots:

};

#endif // DEV_MODEL_H
