
#include "RightListToolbar.h"

#include <qpushbutton.h>
#include <qlayout.h>


RightListToolbar::RightListToolbar(QWidget *parent/* = 0*/) :
	QWidget(parent)
{
	setUpUi();
	establishConn();
}

void RightListToolbar::setUpUi()
{
	mv_up_btn_  = new QPushButton("");
	mv_up_btn_->setToolTip(tr("MoveUp"));
	mv_up_btn_->setObjectName("MoveUpBtn");
	mv_up_btn_->setFlat(true);
	mv_up_btn_->setFixedSize(22, 22);
	mv_up_btn_->setIconSize(QSize(20, 20));

	mv_dn_btn_  = new QPushButton("");
	mv_dn_btn_->setToolTip(tr("MoveDown"));
	mv_dn_btn_->setObjectName("MoveDnBtn");
	mv_dn_btn_->setFlat(true);
	mv_dn_btn_->setFixedSize(22, 22);
	mv_dn_btn_->setIconSize(QSize(20, 20));

	mv_top_btn_ = new QPushButton("");
	mv_top_btn_->setToolTip(tr("MoveTop"));
	mv_top_btn_->setObjectName("MoveTopBtn");
	mv_top_btn_->setFlat(true);
	mv_top_btn_->setFixedSize(22, 22);
	mv_top_btn_->setIconSize(QSize(20, 20));

	mv_btm_btn_ = new QPushButton("");
	mv_btm_btn_->setToolTip(tr("MoveBottom"));
	mv_btm_btn_->setObjectName("MoveBtmBtn");
	mv_btm_btn_->setFlat(true);
	mv_btm_btn_->setFixedSize(22, 22);
	mv_btm_btn_->setIconSize(QSize(20, 20));

	dlt_btn_    = new QPushButton("");
	dlt_btn_->setToolTip(tr("Delete"));
	dlt_btn_->setObjectName("DltBtn");
	dlt_btn_->setFlat(true);
	dlt_btn_->setFixedSize(22, 22);
	dlt_btn_->setIconSize(QSize(20, 20));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	QSpacerItem *spacer_h0 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	QSpacerItem *spacer_h1 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	QSpacerItem *spacer_h2 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	QSpacerItem *spacer_h3 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	QSpacerItem *spacer_h4 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	QSpacerItem *spacer_h5 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);

	layout->addItem(spacer_h0);

	layout->addWidget(mv_up_btn_);

	layout->addItem(spacer_h1);

	layout->addWidget(mv_dn_btn_);

	layout->addItem(spacer_h2);

	layout->addWidget(mv_top_btn_);

	layout->addItem(spacer_h3);

	layout->addWidget(mv_btm_btn_);

	layout->addItem(spacer_h4);

	layout->addWidget(dlt_btn_);

	layout->addItem(spacer_h5);

}

void RightListToolbar::establishConn()
{
	connect(mv_up_btn_,  &QPushButton::clicked, this, &RightListToolbar::mvUpSignal);
	connect(mv_dn_btn_,  &QPushButton::clicked, this, &RightListToolbar::mvDnSignal);
	connect(mv_top_btn_, &QPushButton::clicked, this, &RightListToolbar::mvTopSignal);
	connect(mv_btm_btn_, &QPushButton::clicked, this, &RightListToolbar::mvBtmSignal);
	connect(dlt_btn_,    &QPushButton::clicked, this, &RightListToolbar::dltSignal);
}