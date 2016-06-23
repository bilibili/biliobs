#ifndef BARRAGE_HISTORY_H
#define BARRAGE_HISTORY_H

#include <QWidget>
#include <QString>
#include "barrage_history_macro.h"
#include "barrage_history_control.h"

class QTextEdit;
class RestrictTextArea;

typedef struct config_data config_t;

#include <QTextEdit>
#include <QWheelEvent>
#include <qglobal.h>


namespace Ui {
class BarrageHistory;
}

class RawDataManagerAssitant;

class BarrageHistory : public QWidget
{
    Q_OBJECT

public:
    explicit BarrageHistory(QWidget *parent = 0);
    ~BarrageHistory();

    void initByConfig(config_t *config);

	void updateData();

public slots:
    void setFreshMode(int mode_code, int interval);


public slots:
	void insertData(const QString &html);


private:
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void onWindowFocusChanged(QWidget * old, QWidget * now);

private slots:
    void on_pageUpBtn_clicked();
    void on_pageDnBtn_clicked();
	void on_firstPage_clicked();
	void on_lastPage_clicked();


private:
	void getFirstPage();
	void pageDown();
	void pageUp();
	void getLastPage();


private:
	void forbideReentry(bool forbide);



private:
    Ui::BarrageHistory *ui;


private:
    static void newPageUpdateCb(void *obj) { ((BarrageHistory*)obj)->doPageUpdate(); }
    void doPageUpdate();

    static void queryDataCb(void *obj, int ret_size) { ((BarrageHistory*)obj)->doQueryData(ret_size); }
    void doQueryData(int ret_size);

    static void newPageDelayUpdateCb(void *obj) { ((BarrageHistory*)obj)->doNewDelayUpdate(); }
    void doNewDelayUpdate();

    static void newPageAddCb(void *obj, const QString &item) { ((BarrageHistory*)obj)->doNewPageAdd(item); }
    void doNewPageAdd(const QString &item);
private:
    BarrageHistoryControl *control_;

private:

    RestrictTextArea *rt_display_wgt_;

};

#endif // BARRAGE_HISTORY_H
