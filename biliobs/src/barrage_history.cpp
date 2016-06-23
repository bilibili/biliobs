#include "barrage_history.h"
#include "ui_barrage_history.h"

#include "raw_data_manager.h"
#include "raw_data_manager_assitant.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlockFormat>

#include <qdatetime.h>
#include "restrict_text_area.h"
#include <qtextedit.h>
#include <qlayout.h>

#include <qdebug.h>
#include <assert.h>

#include "BiliOBSUtility.hpp"

#include <QWheelEvent>

#include "barrage_history_control.h"


BarrageHistory::BarrageHistory(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BarrageHistory)
{
    ui->setupUi(this);

    rt_display_wgt_ = new RestrictTextArea(this);


    QHBoxLayout *content_layout = new QHBoxLayout(ui->BarageHistoryDisplay);
    content_layout->addWidget(rt_display_wgt_);
    content_layout->setSpacing(0);


    connect(qApp, &QApplication::focusChanged, this, &BarrageHistory::onWindowFocusChanged);





    control_ = new BarrageHistoryControl();

    control_->setUiUpdateCb(newPageUpdateCb,
                            queryDataCb,
                            newPageDelayUpdateCb,
                            newPageAddCb, 
                            (void*)this);
}

BarrageHistory::~BarrageHistory()
{
    delete ui;
}

void BarrageHistory::initByConfig(config_t *config)
{
    int mode;
    int interval;

    if (config_has_user_value(config, "DanmakuHistory", "DanmakuRefreshMode"))
        mode = config_get_int(config, "DanmakuHistory", "DanmakuRefreshMode");
    else
        mode = config_get_default_int(config, "DanmakuHistory", "DanmakuRefreshMode");

    if (3 == mode) {
        if (config_has_user_value(config, "DanmakuHistory", "DanmakuRefreshInterval"))
            interval = config_get_int(config, "DanmakuHistory", "DanmakuRefreshInterval");
        else
            interval = config_get_default_int(config, "DanmakuHistory", "DanmakuRefreshInterval");


        setFreshMode(mode, interval);
    }
    else {
        setFreshMode(mode, 0);
    }

}

void BarrageHistory::updateData()
{
    control_->update();
}

void BarrageHistory::setFreshMode(int mode_code, int interval)
{
    control_->setRefreshMode((BarrageHistoryControl::FreshMode)mode_code, interval);
}


void BarrageHistory::insertData(const QString &msg)
{
    control_->addNewData(msg);
    
}


void BarrageHistory::wheelEvent(QWheelEvent *e)
{

    if (rt_display_wgt_->isVisible()) {
        QPointF relative_pos = rt_display_wgt_->mapFromGlobal(e->globalPos());
        QWheelEvent *copy = new QWheelEvent(relative_pos,
            e->globalPosF(),
            e->pixelDelta(),
            e->angleDelta(),
            e->delta(),
            e->orientation(),
            e->buttons(),
            e->modifiers());

        QCoreApplication::sendEvent(rt_display_wgt_, copy);
    }

    e->accept();
      
}

void BarrageHistory::onWindowFocusChanged(QWidget * old, QWidget * now)
{
    control_->onFocused();
    
}
void BarrageHistory::on_pageUpBtn_clicked()
{
    if (control_->pageUp())
        forbideReentry(true);
}

void BarrageHistory::on_pageDnBtn_clicked()
{
    //if (-1 == history_pos_)
    //    getLastPage();
    //else

    if (!control_->isReady())
        return;

    if (control_->pageDown())
        forbideReentry(true);


}


void BarrageHistory::on_firstPage_clicked()
{
    if (!control_->isReady())
        return;

	if (control_->getFirstPage())
        forbideReentry(true);
}

void BarrageHistory::on_lastPage_clicked()
{
    if (!control_->isReady())
        return;

    
    updateData();
    
}




void BarrageHistory::forbideReentry(bool forbide)
{
	ui->firstPage->setEnabled(!forbide);
	ui->pageDnBtn->setEnabled(!forbide);
	ui->pageUpBtn->setEnabled(!forbide);
	ui->lastPage->setEnabled(!forbide);
}


void BarrageHistory::doPageUpdate()
{
    rt_display_wgt_->beginAddTransaction();
    rt_display_wgt_->replace(0, 0);
    int p = control_->rt_data_copy_.ptr;
    for (int i = 0; i < control_->rt_data_copy_.capa(); i++) {
        rt_display_wgt_->addItem(control_->rt_data_copy_.arr[p]);

        if (control_->rt_data_copy_.size() - 1 == p)
            p = 0;
        else
            p++;
    }
    rt_display_wgt_->endAddTransaction();
}

void BarrageHistory::doQueryData(int ret_size)
{
    forbideReentry(false);

    QDate date = QDate::currentDate();
    QString retrive;

    QString const *thread_exchange_storage = control_->getDbExchangeData();

    rt_display_wgt_->beginAddTransaction();
    for (int i = 0; i < ret_size; i++) {

        BarrageHistoryControl::parseMsg(date, thread_exchange_storage[i], &retrive);
        rt_display_wgt_->addItem(retrive);

    }
    rt_display_wgt_->endAddTransaction();

}

void BarrageHistory::doNewDelayUpdate()
{
    int p = control_->data_cache_.ptr;
    rt_display_wgt_->beginAddTransaction();

    for (int i = 0; i < control_->data_cache_.capaticy; i++) {

        rt_display_wgt_->addItem(control_->data_cache_.arr[p]);
        control_->rt_data_copy_.push(control_->data_cache_.arr[p]);

        if (control_->data_cache_.size() - 1 == p)
            p = 0;
        else
            p++;
    }
    rt_display_wgt_->endAddTransaction();
    control_->data_cache_.clear();
}

void BarrageHistory::doNewPageAdd(const QString &item)
{
    rt_display_wgt_->addItem(item);
}