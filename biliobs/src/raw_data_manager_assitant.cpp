
#include "raw_data_manager_assitant.h"
#include "raw_data_manager.h"
#include "barrage_history_db_control.h"

RawDataManagerAssitant::RawDataManagerAssitant(RawDataManagerInterface *mgr):
    size_cb_(0),
    query_cb_(0),
	manager_(mgr)

{
	connect(manager_, 
		    &RawDataManagerInterface::getFinished, 
			this, 
            &RawDataManagerAssitant::onQueryfinish,
			Qt::QueuedConnection);
	connect(manager_, 
		    &RawDataManagerInterface::dbRealSize, 
			this, 
			&RawDataManagerAssitant::onDbSizeUpdate,
			Qt::QueuedConnection);
}

RawDataManagerAssitant::~RawDataManagerAssitant()
{
    //manager_->deleteLater();
}

void RawDataManagerAssitant::getData(int index, QString *array, int size, void *receiver)
{
	manager_->getData(index, array, size, receiver);
}

void RawDataManagerAssitant::getLast(QString *array, int size, void *receiver)
{
	manager_->getLast(array, size, receiver);
}

void RawDataManagerAssitant::addMsg(const QString &record)
{
	manager_->addMsg(record);
}

void RawDataManagerAssitant::onDbSizeUpdate(int size)
{
    int s1 = sizeof(size_cb_);
    int s2 = sizeof(query_cb_);
    int s3 = sizeof(*this);
    if (size_cb_)
        (receiver_->*size_cb_)(size);
}

void RawDataManagerAssitant::onQueryfinish(void *receiver, int db_size, int ret_size)
{
    if (query_cb_)
        (receiver_->*query_cb_)(ret_size);
}

//void RawDataManagerAssitant::dbUpdateSize(int size)
//{
//
//}
//
//void RawDataManagerAssitant::onGetFinished()
//{
//
//}