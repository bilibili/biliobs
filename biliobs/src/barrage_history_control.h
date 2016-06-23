
#ifndef BARRAGE_HISTORY_CONTROL_H
#define BARRAGE_HISTORY_CONTROL_H

class BarrageHistoryDbControlInterface;
class QString;
class TimerInterFace;
class QDate;
#include <qstring.h>

#include "barrage_history_macro.h"

class BarrageHistoryControl {

public:
    BarrageHistoryControl();
    ~BarrageHistoryControl();


public:
    static void parseMsg(const QDate &cur_date, const QString &msg, QString *ret);
//=========================================================
public:
    enum FreshMode {
        MANUAL = 0,
        FOCUS,
        REAL_TIME,
        INTERVAL,
        DEFAULT_MODE
    } ;
public:
    void setRefreshMode(FreshMode mode_code, int interval);
private:
    void onModeChanged(void);
private:
    static void timerCb(void *arg) { ((BarrageHistoryControl*)arg)->doOnTimerToggled(); }
    void doOnTimerToggled(void);
private:
    FreshMode refresh_mode_;
    int refresh_interval_;
    TimerInterFace *timer_;


public:
    struct DataCircuitArray{

        QString arr[LIMIT_PER_PAGE];
        int ptr;
        int capaticy;

        DataCircuitArray()
        {
            capaticy = 0;
        }
        void push(const QString &msg)
        {
            if (LIMIT_PER_PAGE == capaticy) {
                if (LIMIT_PER_PAGE - 1 == ptr) {
                    ptr = 0;
                    arr[LIMIT_PER_PAGE - 1] = msg;
                }
                else {
                    arr[ptr] = msg;
                    ptr++;
                }

            }
            else if (0 == capaticy) {
                ptr = 0;
                arr[0] = msg;
                capaticy = 1;
            }
            else {
                arr[capaticy] = msg;
                capaticy++;
            }

        }

        void clear()
        {
            ptr = 0;
            capaticy = 0;
        }

        int size() const
        {
            return LIMIT_PER_PAGE;
        }

        int capa() const
        {
            return capaticy;
        }
    }data_cache_, rt_data_copy_;
public:
    void addNewData(const QString &msg);
    void update();


private:
    enum DataSourceType {
        DB_SOURCE = 0,  /*从数据库读取数据,针对最后一页*/
        RAM_SOURCE  /*从内存读取数据*/
    } source_type_;


private:
    int db_size_;
    int history_pos_;
    bool need_update_;
    int cur_page_size_;

public:
    void setUiUpdateCb(void (*new_page_udate_cb)(void *),
                       void (*query_data_cb)(void *, int),
                       void (*new_page_dealy_update_cb)(void*),
                       void (*new_page_add_cb)(void*, const QString &item),
                       void *ui)
    {
        new_page_udate_cb_ = new_page_udate_cb;
        query_data_cb_ = query_data_cb;
        new_page_dealy_update_cb_ = new_page_dealy_update_cb;
        new_page_add_cb_ = new_page_add_cb;

        ui_ = ui;
    }

public:
    bool getFirstPage();
    bool pageDown();
    bool pageUp();
    //bool getLastPage();

private:
    void (*new_page_udate_cb_)(void *);
    void (*query_data_cb_)(void *, int);
    void (*new_page_dealy_update_cb_)(void*);
    void (*new_page_add_cb_)(void*, const QString &item);
    void *ui_;


//=========================================================

public:
    QString const* getDbExchangeData() const;
    int getDbSize() const;
    bool dbIsValid() const;
    bool isDbBusy() const;
    bool isReady() const { return ready_; }

public:
    bool getDataRequest(int index, int size);
    bool getLastRequest(int size);
    bool addMsg(const QString&record);
private:
    BarrageHistoryDbControlInterface *db_control_;
    bool is_query_last_;
    bool ready_;


private:
    static void dbReadyCb(void *arg, int size) { ((BarrageHistoryControl*)arg)->dbReadyed(size); }
    void dbReadyed(int size);

private:
    static void dbSizeCb(void *arg, int size) { ((BarrageHistoryControl*)arg)->dbSizeChanged(size); }
    void dbSizeChanged(int size);

private:
    static void dbQueryCb(void *arg, int size) { ((BarrageHistoryControl*)arg)->dbQueryFinished(size); }
    void dbQueryFinished(int size);

public:
    void onFocused();

};

#endif