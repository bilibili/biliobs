
#ifndef RAWDATAMANAGERASSITANT_H
#define RAWDATAMANAGERASSITANT_H

#include <qobject.h>

/*the header file is nessary, don't omit this: the size of class-member-function-pointer may be 4、8 or 16 bytes*/
#include "barrage_history_db_control.h"

class RawDataManagerInterface;


class QString;
class BarrageHistoryDbControl;

namespace std {
	template <class T> class allocator;

	template < class T, class Alloc = allocator<T> > class vector;
}

/*construct in UI thread*/
class RawDataManagerAssitant : public  QObject{
	Q_OBJECT
public:
	RawDataManagerAssitant(RawDataManagerInterface *mgr);
    ~RawDataManagerAssitant();

	/*ensure argument 'datas' without access until op accomplish*/
	void getData(int index, QString *array, int size, void *receiver);
	void getLast(QString *array, int size, void *receiver);
	void addMsg(const QString &record);


signals:

public:
    void setCb(
               void (BarrageHistoryDbControl::*size_cb)(int size),
               void (BarrageHistoryDbControl::*query_cb)(int size),
               BarrageHistoryDbControl *obj)
    {
        size_cb_ = size_cb;
        query_cb_ = query_cb;
        receiver_ = obj;
    }
private:
    void (BarrageHistoryDbControl::*size_cb_)(int size);
    void (BarrageHistoryDbControl::*query_cb_)(int size);
    BarrageHistoryDbControl *receiver_;

private slots:
	void onDbSizeUpdate(int size);
    void onQueryfinish(void *receiver, int db_size, int ret_size);
private:
	RawDataManagerInterface *const manager_;

};

#endif