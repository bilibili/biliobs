#ifndef RAWDATAMANAGER_H
#define RAWDATAMANAGER_H

#include <QObject>
class QThread;
class QString;
class QSqlQuery;
#include <QMutex>
#include <qdatetime.h>


/*control in the same thread*/

namespace std {
    template <class T> class allocator;

    template < class T, class Alloc = allocator<T> > class vector;

//    template < class T, class Alloc = allocator<T> > class deque;
//    template <class T, class Container = deque<T> > class queue;
}

class RawDataManagerInterface : public QObject {
	Q_OBJECT
protected:
	//void init();
    virtual ~RawDataManagerInterface() {}

	/*ensure argument 'datas' without access until op accomplish*/
	void getData(int index, QString *array, int size, void *receiver);
	void getLast(QString *array, int size, void *receiver);
	void addMsg(const QString&);

signals:
	void getFinished(void *receiver, int db_size, int ret_size);
	void dbRealSize(int size);

protected:
	virtual void getDataImpl(int index, QString *array, int size, void *receiver) = 0;
	virtual void getLastImpl(QString *array, int size, void *receiver) = 0;
	virtual void addMsgImpl(const QString &record) = 0;


private slots:
	void localGetData(int index, void *array, int size, void *receiver)  { getDataImpl(index, (QString*)array, size, receiver); }
	void localGetLast(void *array, int size, void *receiver)  { getLastImpl((QString*)array, size, receiver); }
	void localAddMsg(const QString &record) { addMsgImpl(record); }



	friend class RawDataManagerAssitant;
};

class RawDataManager : public RawDataManagerInterface
{
    Q_OBJECT

public:
    static RawDataManager* getUniqueInstance();
    static void destroyUniqueInstance();

private slots:
    void startWork();
    void stopWork();

    void onThreadFinish();

private:
	void getDataImpl(int index, QString *array, int size, void *receiver) override;
	void getLastImpl(QString *array, int size, void *receiver) override;
	void addMsgImpl(const QString&) override;



private slots:
    //void localAppendMsg(const QString&);

    //void taskInvoke(void *args, void *function);

    void beginTimer();


public slots:

private:
    explicit RawDataManager();
    ~RawDataManager();

    void timerEvent(QTimerEvent *) override;

private:
    static RawDataManager *s_singleton_;

private:
    QThread *thread_;
//    std::queue<QString> datas_;
    QSqlQuery *query_;
    QMutex mutex_;

    QDate head_date_;

    int timer_;
	int division_counter_;

	bool on_affair_;
	unsigned counter_;

    bool db_valid_;

    bool need_write_;
};

#endif // RAWDATAMANAGER_H
