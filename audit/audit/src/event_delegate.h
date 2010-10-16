/**
  * @file event_delegate.h
  * @authors Быковский Сергей (bsv.serg@gmail.com)
  * @authors Авдюшкин Василий
  *
  * Содержит описание класса делегата, который используется при редактировании
  * модели event_model в классе SettingsObj.
  */

#ifndef COMBO_BOX_DELEGATE_H
#define COMBO_BOX_DELEGATE_H

#include <QItemDelegate>
#include <QComboBox>
#include <QLineEdit>
#include <QColorDialog>
#include "settings_obj.h"

/**
  * Расширяет стандартные способы редактирования табличной модели event_model.
  * Ячейкам, содержащим данные о событиях, реакциях на события и номере канала,
  * предоставлятся интерфейс редактирования в виде выпадающего списка. Кроме этого,
  * при обработке реакции "выделить цветом", выскакивает окошко выбора цвета из
  * палитры. Выбор сохраняется путем изменения фона ячейки модели.
  * Таким образом запоминается дополнительный цветовой параметер.
  *
  */
class EventDelegate : public QItemDelegate
{
    Q_OBJECT
private:
    /** Список возможных событий */
    QList<QString> * event_list;

    /** Список способов реагирования на события*/
    QList<QString> * react_list;

    /** Список значений каналов считывателя */
    QList<QString> * chanell_list;

public:
    EventDelegate(QList<QString> * event, QList<QString> * react, QList<QString> * chanell, QObject *parent = 0);

    /**
      * @name Перегруженные функции класса QItemDelegate
      * @{
      */
    QWidget * createEditor(QWidget * parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    void setEditorData(QWidget * editor, const QModelIndex &index) const;
    void setModelData(QWidget * editor, QAbstractItemModel * model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    /**
      * @}
      */
};

#endif // COMBO_BOX_DELEGATE_H
