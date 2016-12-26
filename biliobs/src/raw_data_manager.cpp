#include "raw_data_manager.h"
#include "BiliGlobalDatas.hpp"

#include <QThread>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QDebug>

#include <QFile>

#include <QVariant>

#include <assert.h>

#include "../common/biliobs_paths.h"
#include <qdatetime.h>

#define DB_NAME "MyDb.db"

static char const * const g_begin = "BEGIN TRANSACTION";
static char const * const g_end = "COMMIT TRANSACTION";

class MyQuery : public QSqlQuery {
public:
    MyQuery(QSqlDatabase db) : QSqlQuery(db) {}
    ~MyQuery()
    {
        qDebug() << " ************************ query released";
    }

};


void RawDataManagerInterface::getData(int index, QString *array, int size, void *receiver)
{


	QMetaObject::invokeMethod(this, 
				              "localGetData", 
							  Qt::QueuedConnection, 
		                      Q_ARG(int, index), 
							  Q_ARG(void*, array),
							  Q_ARG(int, size),
							  Q_ARG(void*, receiver));
}

void RawDataManagerInterface::getLast(QString *array, int size, void *receiver)
{
	QMetaObject::invokeMethod(this,
							  "localGetLast",
							  Qt::QueuedConnection,
							  Q_ARG(void*, array),
							  Q_ARG(int, size),
							  Q_ARG(void*, receiver));
}

void RawDataManagerInterface::addMsg(const QString &record)
{
	QMetaObject::invokeMethod(this,
		                      "localAddMsg",
		                      Qt::QueuedConnection,
							  Q_ARG(const QString&, record));
}

RawDataManager *RawDataManager::s_singleton_ = 0;
RawDataManager* RawDataManager::getUniqueInstance()
{
    if (!s_singleton_) {
        s_singleton_ = new RawDataManager;
    }

    return s_singleton_;
}

void RawDataManager::destroyUniqueInstance()
{
    if (s_singleton_) {
        //delete s_singleton_;
        if (s_singleton_->thread_) {
            qDebug() << "thread " << s_singleton_->thread_ << " will delete";
            QMetaObject::invokeMethod(s_singleton_, "stopWork");

            s_singleton_->thread_->wait();

            s_singleton_->deleteLater();
            //qDebug() << s_singleton_->thread_->isFinished();
            //qDebug() << s_singleton_->thread_->isRunning();
            //QMetaObject::invokeMethod(s_singleton_->thread_, "quit");
            //QMetaObject::invokeMethod(s_singleton_->thread_, "quit");
            
        }
        delete s_singleton_;
        //s_singleton_->deleteLater();
        s_singleton_ = 0;
    }
}

void RawDataManager::startWork()
{

    //connect(this, &RawDataManager::newMsg, this, &RawDataManager::localAppendMsg, Qt::QueuedConnection);

    QString path = QString::fromStdWString(biliobs::GetUserDataPath());
    path.append(QString::fromStdString(gBili_mid)).append("\\").append(DB_NAME);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QFile file(path);
    db.setDatabaseName(path);

    query_ = 0;


    if (file.exists()) {
        if (!db.open()) {
            qDebug() << db.lastError().text();

            bool ret = file.remove();
            
            goto FILE_BAD;
        }

        query_ = new MyQuery(db);
        /*query head_time*/
        QString select_sql = "select * from HEAD_TIME";
        if (!query_->exec(select_sql)) {
            qDebug() << query_->lastError().text();
            goto RE_BEGIN;
        }

        if (!query_->first()) {

            goto RE_BEGIN;
        }
        QString date;
        QVariant val = query_->value(0);
        if (val.isValid() && QVariant::String == val.type())
            date = val.toString();
        else {
            goto RE_BEGIN;
        }

        int y;
        int m;
        int d;

        QStringList sec_list = date.split('-');
        if (sec_list.size() < 3) {

            goto RE_BEGIN;
        }
        y = sec_list[0].toInt();
        m = sec_list[1].toInt();
        d = sec_list[2].toInt();

        head_date_ = QDate(y, m, d);
        if (head_date_ > QDate::currentDate()) {

            goto RE_BEGIN;
        }
        if (head_date_.addDays(7) <= QDate::currentDate()) {

            qDebug() << "delete old db";
            goto RE_BEGIN;
        }


        /*query counter*/
        select_sql = "select * from TIMES";
        if (!query_->exec(select_sql)) {
            qDebug() << query_->lastError().text();

            goto RE_BEGIN;
        }
        if (!query_->first()) {
            qDebug() << query_->lastError().text();

            goto RE_BEGIN;
        }

        QVariant c = query_->value(0);
        if (!c.isValid()) {

            qDebug() << c.toInt();

            qDebug() << query_->lastError().text();

            goto RE_BEGIN;

        }


        
        counter_ = c.toUInt();
        emit dbRealSize(counter_);
        
        on_affair_ = false;

goto SUCCESS;


    }


    if (false) {
RE_BEGIN:
        delete query_;
        query_ = 0;

        db.close();
        file.remove();
    }
FILE_BAD:
    if (!db.open()) {
        qDebug() << db.lastError().text();

        db_valid_ = false;

        emit dbRealSize(-1);
        return;
    }

    query_ = new MyQuery(db);

    //query_->exec(g_begin);

    /*时间*/

    if (!query_->exec(QString("CREATE TABLE HEAD_TIME (timetamp TEXT)")))
        goto ERROR_RELEASE;


    qDebug() << QString("INSERT INTO HEAD_TIME VALUES (")
        + QDate::currentDate().toString("yyyy-MM-dd")
        + ")";
    if (!query_->exec(QString("INSERT INTO HEAD_TIME VALUES (\"")
        + QDate::currentDate().toString("yyyy-MM-dd")
        + "\")")) {

        qDebug() << query_->lastError().text();

        goto ERROR_RELEASE;

    }


    if (!query_->exec(QString("CREATE TABLE TIMES (times INTEGER)"))) {
        qDebug() << query_->lastError().text();

        goto ERROR_RELEASE;
    }

    if (!query_->exec(QString("INSERT INTO TIMES VALUES (")
        + "0"
        + ")")) {
        qDebug() << query_->lastError().text();

        goto ERROR_RELEASE;

    }

    if (!query_->exec(QString("CREATE TABLE DATA (id INTEGER, msg TEXT)"))) {
        qDebug() << query_->lastError().text();

        counter_ = 0;

        goto ERROR_RELEASE;
    }
    //query_->exec(g_end);

    counter_ = 0;
    emit dbRealSize(0);
    goto SUCCESS;

ERROR_RELEASE:
    db_valid_ = false;
    delete query_;
    query_ = 0;

    emit dbRealSize(-1);
    return;

SUCCESS:
    head_date_ = QDate::currentDate();

    need_write_ = false;
    if (!query_->exec(g_begin))
        qDebug() << query_->lastError().text();
    else
        on_affair_ = true;

    thread_->start();
    db_valid_ = true;

    
    beginTimer();



}

void RawDataManager::stopWork()
{
    if (!db_valid_) {
        thread_->quit();
        return;
    }

    db_valid_ = false;
    //quit();
    killTimer(timer_);

    if (on_affair_) {
        QString upd_sql = "update TIMES set times = " + QString::number(counter_);
        if (!query_->exec(upd_sql))
            qDebug() << query_->lastError().text();

        if (!query_->exec(g_end)) {
            qDebug() << query_->lastError().text();
        }
    }
    else {
        QString upd_sql = "update TIMES set times = 0";
        query_->exec(upd_sql);
    }
    query_->finish();
    delete query_;
    query_ = 0;

    QSqlDatabase db = QSqlDatabase::database(DB_NAME, false);
    db.close();


    thread_->quit();
}

void RawDataManager::onThreadFinish()
{
    //deleteLater();
}

void RawDataManager::getDataImpl(int index, QString *array, int size, void *receiver)
{

    if (!db_valid_) {
        emit getFinished(receiver, 0, 0);

        return;
    }
//    mutex_.lock();
	//qDebug() << "******************************";
	//qDebug() << "search for:" << index;

//    qDebug() << "get data";

	//division_counter_ = 0;
	//on_affair_ = false;

 //   QString upd_sql = "update TIMES set times = " + QString::number(counter_);
 //   if (!query_->exec(upd_sql)) {
 //       qDebug() << query_->lastError().text();
 //   }
	//if (!query_->exec(g_end)) {
	//	qDebug() << query_->lastError().text();
	//}
	//else {

		//emit dbRealSize(counter_);

		QString select_sql = "select * from DATA WHERE id >= " + QString::number(index);
		if (!query_->exec(select_sql)) {
			qDebug() << query_->lastError().text();

		} else {

			bool end = false;
			end = !query_->first();
			int i = 0;
			for (; i < size; i++) {
				if (end)
					break;

			    QVariant val = query_->value(1);
				if (val.isValid() && QVariant::String == val.type())
					array[i] = val.toString();
			    else
					array[i] = QString("");

			    end = !query_->next();

			}

			emit getFinished(receiver, counter_, i);

		}


		//if (!query_->exec(g_begin)) {
		//	qDebug() << query_->lastError().text();;
		//}
		//else
		//	on_affair_ = true;

	//}


//    mutex_.unlock();
}

void RawDataManager::getLastImpl(QString *array, int size, void *receiver)
{
    if (!db_valid_) {
        emit getFinished(receiver, 0, 0);

        return;
    }

	division_counter_ = 0;

    if (need_write_) {

        QString upd_sql = "update TIMES set times = " + QString::number(counter_);
        if (!query_->exec(upd_sql)) {
            qDebug() << query_->lastError().text();
        }

        if (!query_->exec(g_end)) {
            qDebug() << query_->lastError().text();
        } else
            on_affair_ = false;
    }


    emit dbRealSize(counter_);
    int pos;
    if (counter_ < size)
        pos = 0;
    else
        pos = counter_ - size;


    QString select_sql = "select * from DATA WHERE id >= " + QString::number(pos);
    if (!query_->exec(select_sql)) {
        qDebug() << query_->lastError().text();

        emit getFinished(receiver, counter_, -1);
    }
    else {

        bool end = false;
        end = !query_->first();
        int i = 0;
        for (; i < size; i++) {
            if (end)
                break;

            QVariant val = query_->value(1);
            if (val.isValid() && QVariant::String == val.type())
                array[i] = val.toString();
            else
                array[i] = QString("");

            end = !query_->next();

        }

        emit getFinished(receiver, counter_, i);

    }
    if (!on_affair_)
        if (!query_->exec(g_begin)) {
            qDebug() << query_->lastError().text();;
        }
        else
            on_affair_ = true;
       
}

void RawDataManager::addMsgImpl(const QString &data)
{
    assert(thread_->isRunning());

    if (!db_valid_) {


        return;
    }

//    mutex_.lock();

//    qDebug() << "write:";

    QString sql = "INSERT INTO DATA VALUES (" + QString::number(counter_) + ",'" + data + "')";

	//qDebug() << sql;

    if (!query_->exec(sql)) {
        qDebug() << query_->lastError().text();
    } else {
        counter_++;
        need_write_ = true;
    }

	
//    mutex_.unlock();
}

//
//void RawDataManager::taskInvoke(void *args, void *function)
//{
//    void (*f)(RawDataManager *, void*) = (void(*)(RawDataManager *, void*))function;
//    f(this, args);
//}

void RawDataManager::beginTimer()
{
    timer_ = startTimer(100);

	division_counter_ = 0;
}

RawDataManager::RawDataManager()
{
    db_valid_ = false;

    thread_ = new QThread();
    thread_->setObjectName("RawDatamMgrThread");
    connect(thread_, &QThread::finished, this, &RawDataManager::onThreadFinish);

    moveToThread(thread_);
    setParent(thread_);
    thread_->start();

    QMetaObject::invokeMethod(this, "startWork");

    
   
}

RawDataManager::~RawDataManager()
{
    qDebug() << "########################### manager released";
    delete thread_;
}

void RawDataManager::timerEvent(QTimerEvent *)
{
	if (100 >= division_counter_) {
		division_counter_++;
		return;
	}

	division_counter_ = 0;

    if (!need_write_)
        return;

	on_affair_ = false;

    //mutex_.lock();
    QString upd_sql = "update TIMES set times = " + QString::number(counter_);
    if (!query_->exec(upd_sql)) {
        qDebug() << query_->lastError().text();
    }
    if (!query_->exec(g_end)) {
        qDebug() << query_->lastError().text();
    } else {

		emit dbRealSize(counter_);

		//qDebug() << "size:" << counter_;

        if (!query_->exec(g_begin)) {
            qDebug() << query_->lastError().text();;
        } else
			on_affair_ = true;

    }

    //mutex_.unlock();
}

