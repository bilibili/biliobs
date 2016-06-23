#include "barrage_history_db_control.h"
#include "raw_data_manager_assitant.h"
#include "raw_data_manager.h"

BarrageHistoryDbControl::BarrageHistoryDbControl()
{
    db_manager_assistant_ = new RawDataManagerAssitant(RawDataManager::getUniqueInstance());

    db_manager_assistant_->setCb(&BarrageHistoryDbControl::onDbSizeChanged,
                                 &BarrageHistoryDbControl::onQueryFinshed,
                                 this);
}

BarrageHistoryDbControl::~BarrageHistoryDbControl()
{
    RawDataManager::destroyUniqueInstance();
}

bool BarrageHistoryDbControl::getDataRequest(int index, int size)
{
    if (isValid()) {
        if (on_query_)
            return false;

        on_query_ = true;
        db_manager_assistant_->getData(index, db_exchange_storage_, size, 0);
        return true;
    }

    return false;

}
bool BarrageHistoryDbControl::getLastRequest(int size)
{
    if (isValid()) {
        if (on_query_)
            return false;

        on_query_ = true;
        db_manager_assistant_->getLast(db_exchange_storage_, size, 0);
        return true;
    }

    return false;
}
bool BarrageHistoryDbControl::addMsg(const QString&record)
{
    if (isValid()) {

        db_manager_assistant_->addMsg(record);
        return true;
    }

    return false;
}


void BarrageHistoryDbControl::onDbSizeChanged(int size)
{
    db_size_ = size;
    if (no_ready_) {
        no_ready_ = false;

        if (size < 0)
            valid_ = false;
        else
            valid_ = true;

        if (ready_cb_)
            ready_cb_(ready_cb_arg_, size);
    } else
        size_changed_cb_(size_changed_cb_arg_, size);

    
}
void BarrageHistoryDbControl::onQueryFinshed(int ret_size)
{
    on_query_ = false;
    if (query_cb_)
        query_cb_(query_cb_arg_, ret_size);

    
}