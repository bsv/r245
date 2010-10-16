#include "monitor_filter.h"
#include "monitor.h"

MonitorFilter::MonitorFilter(QObject *parent):QSortFilterProxyModel(parent)
{
    trans_code_rexp = QRegExp("");
}

/**
  * явл€етс€ перегруженной функцией класса QSortFilterProxyModel.
  * ќтвечает за проверку каждой строки модели на соответствие услови€м фильтрации.
  *
  * @return true, если строка удовлетвор€ет параметрам фильтрации, false, если не удовлетвор€ет.
  */
bool MonitorFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    qApp->processEvents(); // чтоб интерфейс не зависал

    QModelIndex dev_name_indx = sourceModel()->index(sourceRow, Monitor::DevNameAttr, sourceParent);
    QModelIndex dev_num_indx = sourceModel()->index(sourceRow, Monitor::DevNumAttr, sourceParent);
    QModelIndex ch_indx = sourceModel()->index(sourceRow, Monitor::ChAttr, sourceParent);
    QModelIndex tag_name_indx = sourceModel()->index(sourceRow, Monitor::TagNameAttr, sourceParent);
    QModelIndex tag_id_indx = sourceModel()->index(sourceRow, Monitor::TagIdAttr, sourceParent);
    QModelIndex date_indx = sourceModel()->index(sourceRow, Monitor::DateAttr, sourceParent);
    QModelIndex time_indx = sourceModel()->index(sourceRow, Monitor::TimeAttr, sourceParent);
    QModelIndex trans_code_indx = sourceModel()->index(sourceRow, Monitor::TransCodeAttr, sourceParent);
    QModelIndex event_indx = sourceModel()->index(sourceRow, Monitor::TypeEventAttr, sourceParent);


    return     (sourceModel()->data(dev_name_indx).toString().contains(deviceRegExp) ||
                sourceModel()->data(dev_num_indx).toString().contains(deviceRegExp))
            && (sourceModel()->data(trans_code_indx).toString().contains(trans_code_rexp))
            && sourceModel()->data(ch_indx).toString().contains(channelRegExp)
            && sourceModel()->data(event_indx).toString().contains(event_reg_exp)
            && (sourceModel()->data(tag_name_indx).toString().contains(tagRegExp) ||
                sourceModel()->data(tag_id_indx).toString().contains(tagRegExp))
            && dateInRange(QDate().fromString(sourceModel()->data(date_indx).toString(), Qt::LocalDate))
            && timeInRange(sourceModel()->data(time_indx).toTime());
}

/**
  * ”станавливает регул€рное выражение на фильтрацию кода событи€.
  *
  * @param reg_exp - регул€рное выражение дл€ фильтра.
  */
void MonitorFilter::setTransCodeRE(QString reg_exp)
{
    trans_code_rexp = QRegExp(reg_exp);
}

/**
  * ”станавливает нижнюю границу дл€ даты.
  *
  * @param date - нижн€€ граница даты.
  */
void MonitorFilter::setFilterMinimumDate(const QDate date)
{
    minDate = date;
}

/**
  * ”станавливает верхнюю границу дл€ даты.
  *
  * @param date - нижн€€ граница даты.
  */
void MonitorFilter::setFilterMaximumDate(const QDate date)
{
    maxDate = date;
}

/**
  * ”станавливает нижнюю границу времени.
  *
  * @param time - нижн€€ граница времени.
  */
void MonitorFilter::setFilterMinimumTime(const QTime time)
{
    minTime = time;
}

/**
  * ”станавливает верхнюю границу времени.
  *
  * @param time - нижн€€ граница времени.
  */
void MonitorFilter::setFilterMaximumTime(const QTime time)
{
    maxTime = time;
}

/**
  * ”станавливает регул€рные выражени€ дл€ фильтра канала, устройства и метки.
  *
  * @param channel - регул€рное выражение дл€ канала.
  * @param device - регул€рное выражение дл€ идентификатора устройства (числового или символьного).
  * @param tag - регул€рное выражение дл€ идентификатора метки (числового или символьного).
  */
void MonitorFilter::setRegExp(QRegExp channel, QRegExp device, QRegExp tag, QRegExp event)
{
    channelRegExp = channel;
    deviceRegExp = device;
    tagRegExp = tag;
    event_reg_exp = event;
}

/**
  * ѕровер€ет дату на соответствие заданному диапазону от
  * minDate до maxDate.
  *
  * @param date - значение провер€емого значени€ даты.
  * @return true, если значение date удовлетвор€ет
  *     заданному диапазону.
  */
bool MonitorFilter::dateInRange(const QDate &date) const
{
    return (!minDate.isValid() || date >= minDate)
           && (!maxDate.isValid() || date <= maxDate);
}

/**
  * ѕровер€ет врем€ на соответствие заданному диапазону от
  * minTime до maxTime.
  *
  * @param time - значение провер€емого значени€ времени.
  * @return true, если значение time удовлетвор€ет
  *     заданному диапазону.
  */
bool MonitorFilter::timeInRange(const QTime &time) const
{

    return (!minTime.isValid() || time >= minTime)
           && (!maxTime.isValid() || time <= maxTime);
}
