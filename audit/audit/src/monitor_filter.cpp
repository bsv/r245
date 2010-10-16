#include "monitor_filter.h"
#include "monitor.h"

MonitorFilter::MonitorFilter(QObject *parent):QSortFilterProxyModel(parent)
{
    trans_code_rexp = QRegExp("");
}

/**
  * �������� ������������� �������� ������ QSortFilterProxyModel.
  * �������� �� �������� ������ ������ ������ �� ������������ �������� ����������.
  *
  * @return true, ���� ������ ������������� ���������� ����������, false, ���� �� �������������.
  */
bool MonitorFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    qApp->processEvents(); // ���� ��������� �� �������

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
  * ������������� ���������� ��������� �� ���������� ���� �������.
  *
  * @param reg_exp - ���������� ��������� ��� �������.
  */
void MonitorFilter::setTransCodeRE(QString reg_exp)
{
    trans_code_rexp = QRegExp(reg_exp);
}

/**
  * ������������� ������ ������� ��� ����.
  *
  * @param date - ������ ������� ����.
  */
void MonitorFilter::setFilterMinimumDate(const QDate date)
{
    minDate = date;
}

/**
  * ������������� ������� ������� ��� ����.
  *
  * @param date - ������ ������� ����.
  */
void MonitorFilter::setFilterMaximumDate(const QDate date)
{
    maxDate = date;
}

/**
  * ������������� ������ ������� �������.
  *
  * @param time - ������ ������� �������.
  */
void MonitorFilter::setFilterMinimumTime(const QTime time)
{
    minTime = time;
}

/**
  * ������������� ������� ������� �������.
  *
  * @param time - ������ ������� �������.
  */
void MonitorFilter::setFilterMaximumTime(const QTime time)
{
    maxTime = time;
}

/**
  * ������������� ���������� ��������� ��� ������� ������, ���������� � �����.
  *
  * @param channel - ���������� ��������� ��� ������.
  * @param device - ���������� ��������� ��� �������������� ���������� (��������� ��� �����������).
  * @param tag - ���������� ��������� ��� �������������� ����� (��������� ��� �����������).
  */
void MonitorFilter::setRegExp(QRegExp channel, QRegExp device, QRegExp tag, QRegExp event)
{
    channelRegExp = channel;
    deviceRegExp = device;
    tagRegExp = tag;
    event_reg_exp = event;
}

/**
  * ��������� ���� �� ������������ ��������� ��������� ��
  * minDate �� maxDate.
  *
  * @param date - �������� ������������ �������� ����.
  * @return true, ���� �������� date �������������
  *     ��������� ���������.
  */
bool MonitorFilter::dateInRange(const QDate &date) const
{
    return (!minDate.isValid() || date >= minDate)
           && (!maxDate.isValid() || date <= maxDate);
}

/**
  * ��������� ����� �� ������������ ��������� ��������� ��
  * minTime �� maxTime.
  *
  * @param time - �������� ������������ �������� �������.
  * @return true, ���� �������� time �������������
  *     ��������� ���������.
  */
bool MonitorFilter::timeInRange(const QTime &time) const
{

    return (!minTime.isValid() || time >= minTime)
           && (!maxTime.isValid() || time <= maxTime);
}
