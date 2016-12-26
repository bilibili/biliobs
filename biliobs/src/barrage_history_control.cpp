
#include "barrage_history_control.h"
#include "barrage_history_db_control.h"
#include "timer_on_qt.h"
#include <assert.h>
#include <qdatetime.h>


BarrageHistoryControl::BarrageHistoryControl() : db_control_(0), refresh_mode_(DEFAULT_MODE)
{
    BarrageHistoryDbControlInterface *db_c = new BarrageHistoryDbControl();
    db_c->init();
    db_c->setDbReadyCb(dbReadyCb, (void*)this);
    db_c->setDbSizeChangedCb(dbSizeCb, (void*)this);
    db_c->setQueryFinishedCb(dbQueryCb, (void*)this);
    db_control_ = db_c;

    timer_ = new TimerOnQt();
    timer_->init();

    history_pos_ = -1;

    is_query_last_ = false;

    ready_ = false;
}

BarrageHistoryControl::~BarrageHistoryControl()
{
    db_control_->release();
    delete db_control_;

    timer_->release();
    delete timer_;
}

void BarrageHistoryControl::parseMsg(const QDate &cur_date, const QString &msg, QString *ret)
{
    if (msg[0] == ':') {
        QString html;

        QStringList list = msg.split(":");

        if (list.size() < 6) {
            html = "msg null";
            *ret = msg;
            return;
        }

        QStringList sec_list = list[1].split("-");
        int y;
        int m;
        int d;

        int hh;
        int mm;
        int ss;

        y = sec_list[0].toInt();
        m = sec_list[1].toInt();
        d = sec_list[2].toInt();

        QDate date(y, m, d);

        sec_list = list[2].split("-");
        hh = sec_list[0].toInt();
        mm = sec_list[1].toInt();
        ss = sec_list[2].toInt();

        QTime time(hh, mm, ss);


        if (list[3] == "s") {
            html.append("<font color=\"#ff99ff\" style=\"font-size:14px; font-family:Microsoft YaHei \">");
            html.append(QStringLiteral("【主】"));
        }
        else {
            html.append("<font color=\"#4fc1e9\" style=\"font-size:14px; font-family:Microsoft YaHei \">");
            if (list[3] == "f")
                html.append(QStringLiteral("【房】"));
            else if (list[3] == "s")
                html.append(QStringLiteral("【爷】"));
        }

        html.append(' ');
        html.append(list[4]);

        html.append(' ');

        if (cur_date != date) {
            html.append(date.toString("yyyy/MM/dd"));

            html.append(' ');
        }
        html.append(time.toString("hh:mm:ss"));

        html.append("</font>");


        html.append("<br>");



        html.append("<font color=black style=\"font-size:14px; font-family:Microsoft YaHei \">");
        html.append("&nbsp;   ");
        html.append(list[5]);
        for (int i = 6; i < list.size(); i++) {
            html.append(":");
            html.append(list[i]);
        }

        html.append("</font>");

        *ret = html;
        return;

    }

    int i = 0;
    int size = msg.length();
    for (; i < size; i++)
        if (':' == msg[i])
            break;

    if (i != size) {
        QString left = msg.left(i + 1);
        QString right = msg.right(size - (i + 1));
        *ret = "<font color=red style=\"font-size:14px; font-family:Microsoft YaHei \">" + left + right + "</font>";
    }
    else
        *ret = msg;
}

void BarrageHistoryControl::setRefreshMode(BarrageHistoryControl::FreshMode mode_code, int interval)
{
    if (DEFAULT_MODE == refresh_mode_) {
        if (DEFAULT_MODE == mode_code)
            return;

        refresh_mode_ = mode_code;

        if (INTERVAL == mode_code) {
            assert(!timer_->isWorking());

            timer_->startTimer(timerCb, this, interval);
            refresh_interval_ = interval;
        }

        onModeChanged();

    } else if (mode_code == refresh_mode_) {
        if (INTERVAL == mode_code && refresh_interval_ != interval) {
            timer_->stopTimer();

            timer_->startTimer(timerCb, this, interval);
            refresh_interval_ = interval;
        }

        onModeChanged();

    } else {
        if (INTERVAL == refresh_mode_) {
            timer_->stopTimer();
        } else {
            if (INTERVAL == mode_code) {
                assert(!timer_->isWorking());

                timer_->startTimer(timerCb, this, interval);
                refresh_interval_ = interval;
            }
        }

        refresh_mode_ = mode_code;

        onModeChanged();
    }
}

void BarrageHistoryControl::onModeChanged()
{
    int p = data_cache_.ptr;
    if (data_cache_.size() == data_cache_.capa()) {
        rt_data_copy_.clear();
    }
    for (int i = 0; i < data_cache_.capa(); i++) {

        rt_data_copy_.push(data_cache_.arr[p]);
        if (data_cache_.size() - 1 == p)
            p = 0;
        else
            p++;
    }


    source_type_ = RAM_SOURCE;
    if (new_page_udate_cb_)
        new_page_udate_cb_(ui_);
}

void BarrageHistoryControl::doOnTimerToggled(void)
{
    if (INTERVAL != refresh_mode_)
        assert(false);

    if (RAM_SOURCE == source_type_)
        new_page_dealy_update_cb_(ui_);
}

void BarrageHistoryControl::addNewData(const QString &msg)
{
    if (!db_control_->isValid())
        return;

    if (data_cache_.capa() == data_cache_.size())
        if (RAM_SOURCE == source_type_)
            history_pos_++;

    db_size_++;

    need_update_ = true;

    if (msg.startsWith(':')) {
        QString retrive;
        QDate date = QDate::currentDate();

        QString compose(":");
        compose.append(QDate::currentDate().toString("yyyy-MM-dd"));
        compose.append(':');
        compose.append(QDateTime::currentDateTime().toString("HH-mm-ss"));
        compose.append(msg);


        parseMsg(date, compose, &retrive);
        db_control_->addMsg(compose);

        if (REAL_TIME == refresh_mode_ && RAM_SOURCE == source_type_) {
            new_page_add_cb_(ui_, retrive);
            rt_data_copy_.push(retrive);
        }
        else
            data_cache_.push(retrive);
    }
    else {
        db_control_->addMsg(msg);

        QString compose;

        compose = "<font color=red style=\"font-size:14px; font-family:Microsoft YaHei \">" + msg + "</font>";


        if (REAL_TIME == refresh_mode_ && RAM_SOURCE == source_type_) {
            new_page_add_cb_(ui_, compose);
            rt_data_copy_.push(compose);
        }
        else
            data_cache_.push(compose);
    }
}

void BarrageHistoryControl::update()
{
    if (db_size_ > data_cache_.size())
        history_pos_ = db_size_ - data_cache_.size();
    else
        history_pos_ = 0;

    source_type_ = RAM_SOURCE;

    if (INTERVAL != refresh_mode_)
        if (data_cache_.capa() > 0)
            new_page_dealy_update_cb_(ui_);
        
}

bool BarrageHistoryControl::getFirstPage()
{
    source_type_ = DB_SOURCE;

    if (db_control_->isBusy())
        return false;

    history_pos_ = 0;
    cur_page_size_ = data_cache_.size();

    db_control_->getDataRequest(history_pos_, data_cache_.size());

    return true;
}
bool BarrageHistoryControl::pageDown()
{
    if (history_pos_ + data_cache_.size() >= db_size_)
        return false;

    source_type_ = DB_SOURCE;


    if (db_control_->isBusy())
        return false;

    history_pos_ += cur_page_size_;

    db_control_->getDataRequest(history_pos_, data_cache_.size());

    return true;
}
bool BarrageHistoryControl::pageUp()
{
    if (history_pos_ == 0)
        return false;

    source_type_ = DB_SOURCE;

    if (db_control_->isBusy())
        return false;

    int search_size;


    if (history_pos_ < data_cache_.size()) {
        search_size = history_pos_;
        cur_page_size_ = search_size;
        history_pos_ = 0;
    }
    else {
        history_pos_ -= data_cache_.size();
        search_size = data_cache_.size();
        cur_page_size_ = data_cache_.size();
    }

    db_control_->getDataRequest(history_pos_, search_size);

    return true;
}
//bool BarrageHistoryControl::getLastPage()
//{
//    if (db_control_->isBusy())
//        return false;
//
//
//
//    db_control_->getLastRequest(data_cache_.size());
//    return true;
//}

QString const* BarrageHistoryControl::getDbExchangeData() const
{
    return db_control_->getRetData();
}
int BarrageHistoryControl::getDbSize() const
{
    return db_control_->getDbSize();
}
bool BarrageHistoryControl::dbIsValid() const
{
    return db_control_->isValid();
}

bool BarrageHistoryControl::isDbBusy() const
{
    return db_control_->isBusy();
}

bool BarrageHistoryControl::getDataRequest(int index, int size)
{
    if (dbIsValid())
        return db_control_->getDataRequest(index, size);

    return false;
}
bool BarrageHistoryControl::getLastRequest(int size)
{
    if (dbIsValid()) {
        is_query_last_ = true;
        return db_control_->getLastRequest(size);
    }
        

    return false;
}
bool BarrageHistoryControl::addMsg(const QString&record)
{
    if (dbIsValid())
        return db_control_->addMsg(record);

    return false;
}

void BarrageHistoryControl::dbReadyed(int size)
{
    db_size_ = size;
    if (db_control_->isValid()) {
        getLastRequest(LIMIT_PER_PAGE);
    }
    

}

void BarrageHistoryControl::dbSizeChanged(int size)
{

}

void BarrageHistoryControl::dbQueryFinished(int ret_size)
{
    QString const *thread_exchange_storage = getDbExchangeData();
    QString retrive;

    if (is_query_last_) {
        is_query_last_ = false;
        
        for (int i = 0; i < ret_size; i++) {

            QDate date = QDate::currentDate();
            QString retrive;

            parseMsg(date, thread_exchange_storage[i], &retrive);


            rt_data_copy_.push(retrive);

        }

        ready_ = true;
    }
    query_data_cb_(ui_, ret_size);
}

void BarrageHistoryControl::onFocused()
{
    if (FOCUS == refresh_mode_)
        if (RAM_SOURCE == source_type_)
            if (data_cache_.capa() > 0)
                new_page_dealy_update_cb_(ui_);
}