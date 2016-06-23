
#ifndef BARRAGE_HISTORY_DB_CONTROL_H
#define BARRAGE_HISTORY_DB_CONTROL_H

#include "barrage_history_db_control_interface.h"
class RawDataManagerAssitant;

class BarrageHistoryDbControl : public BarrageHistoryDbControlInterface {
public:
    BarrageHistoryDbControl();
    ~BarrageHistoryDbControl();

public:
    bool getDataRequest(int index, int size) override;
    bool getLastRequest(int size) override;
    bool addMsg(const QString&record) override;

private:
    void onDbSizeChanged(int size);
    void onQueryFinshed(int ret_size);

private:
    RawDataManagerAssitant *db_manager_assistant_;
};

#endif