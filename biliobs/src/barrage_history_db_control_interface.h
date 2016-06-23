
#ifndef BARRAGE_HISTORY_DB_CONTROL_INTERFACE_H
#define BARRAGE_HISTORY_DB_CONTROL_INTERFACE_H

#include <qstring.h>
#include "barrage_history_macro.h"

class BarrageHistoryDbControlInterface {
public:
    BarrageHistoryDbControlInterface() : valid_(false), size_changed_cb_(0), query_cb_(0), ready_cb_(0), no_ready_(true), db_size_(0), on_query_(false) {}
    virtual ~BarrageHistoryDbControlInterface() {}
public:
    virtual bool init() { return true; }
    virtual bool release() { return true; }

    virtual bool getDataRequest(int index, int size) = 0;
    virtual bool getLastRequest(int size) = 0;
    virtual bool addMsg(const QString&record) = 0;

public:
    void setDbSizeChangedCb(void (*cb)(void*, int size), void *arg) { size_changed_cb_ = cb; size_changed_cb_arg_ = arg; }
protected:
    void (*size_changed_cb_)(void*, int);
    void *size_changed_cb_arg_;

public:
    void setDbReadyCb(void(*cb)(void*, int size), void *arg) { ready_cb_ = cb; ready_cb_arg_ = arg; }
protected:
    void(*ready_cb_)(void*, int);
    void *ready_cb_arg_;

    bool no_ready_;

public:
    void setQueryFinishedCb(void(*cb)(void *arg, int ret_size), void *arg) { query_cb_ = cb; query_cb_arg_ = arg; }

    QString const *getRetData() const 
    {
        if (on_query_)
            return 0;

        return db_exchange_storage_; 
    }
    bool isBusy() const { return on_query_; }
protected:
    QString db_exchange_storage_[LIMIT_PER_PAGE];
    void (*query_cb_)(void *, int);
    void *query_cb_arg_;
    bool on_query_;

public:
    bool isValid() const { return valid_; }
protected:
    bool valid_;
    

public:
    int getDbSize() const { return db_size_; }
protected:
    int db_size_;


};

#endif