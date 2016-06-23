#include "danmakumove.h"
#include "danmakucreate.h"
#include "danmakuwid.h"

#include <QPainter>
#include <QTimer>
#include <QTextDocument>

#define DM_UP_SPEED	2

DanmakuMove *DanmakuMove::inst_ = nullptr;
DanmakuMove *DanmakuMove::getInst_() {
	return inst_ ? inst_ : new DanmakuMove;
}
DanmakuMove::DanmakuMove(QObject *parent)
	: QObject(parent)
	, startPaintDMIndex_(1)
	, standByPix_(nullptr)
	, isNeedUpdateStandBy_(true) {

	inst_ = this;
	setObjectName("DanmakuMove");
	DanmakuWid::objects_["DanmakuMove"] = this;

	dmMoveUpTimer_ = new QTimer(this);
	dmMoveUpTimer_->setInterval(40);
	QObject::connect(dmMoveUpTimer_, &QTimer::timeout, [this](){
		dmUpPos_.setY(dmUpPos_.y() - DM_UP_SPEED);
		moveDM_();
	});
}

void DanmakuMove::drawStandBy_(){

	if (startPaintDMIndex_ <= dmCreater_->totalDMCount_) {
		return;
	}

	if (!isNeedUpdateStandBy_)
		return;
	else
		isNeedUpdateStandBy_ = false;

	QFont font;
	font.setBold(true);
	font.setFamily(QString("Microsoft YaHei"));
	font.setPixelSize(25);

	QString styleStr = QString(
		"<p>"
		"<span style = \" font-size:%1px; color:#FF9DCB; \">%2< / span>"
		"</p>");
	QString standByStr = styleStr.arg(font.pixelSize()).arg(tr("Stand By"));

	QTextDocument td;
	td.setDefaultFont(font);
	td.setDocumentMargin(10);

	QTextOption op = td.defaultTextOption();
	op.setAlignment(Qt::AlignHCenter);
	td.setDefaultTextOption( op );
	td.setHtml(standByStr);
	td.setTextWidth(dmSideWidSize_.width());
	QSize s = td.size().toSize();

	if (standByPix_)
		delete standByPix_;
	standByPix_ = new QPixmap(QSize(dmSideWidSize_.width(), 60));
	standByPix_->fill(Qt::transparent);
	QPainter p(standByPix_);
	p.setPen(QPen(QColor(0x49545A)));
	p.setBrush(QBrush(QColor(0x49545A), Qt::SolidPattern));
	p.setFont(font);

	int border = 1;
	QRect r(0, 0, standByPix_->width(), standByPix_->height());
	p.setOpacity(0.6);
	p.drawRoundedRect(r.adjusted(border, border, -border, -border), 3, 3);

	td.drawContents(&p);

	dmSideWidBKPixCache_->fill(Qt::transparent);
	QPainter pSideDM(dmSideWidBKPixCache_);
	pSideDM.drawPixmap(QPoint(0, dmSideWidBKPixCache_->height()-standByPix_->height()+border), *standByPix_);
}

DanmakuMove::~DanmakuMove() {
	dmMoveUpTimer_->stop();
	inst_ = nullptr;
}

void DanmakuMove::moveDM_(){

	QMutexLocker m(&dmCreater_->dmHashMutex_);
	int remaindDMCount = dmCreater_->totalDMCount_ - startPaintDMIndex_ + 1;
	int accelerator = dmCreater_->totalDMHeight_ / 25 / 3;		//25fps, one danmaku/3s
	dmUpPos_.setY(dmUpPos_.y() - accelerator - DM_UP_SPEED);

	if ((dmCreater_->totalDMHeight_ < (dmSideWidSize_.height() - dmUpPos_.y())) || (dmCreater_->totalDMHeight_ >(3 * dmSideWidSize_.height())))
		dmUpPos_.setY(dmSideWidSize_.height() - dmCreater_->totalDMHeight_);

	QPixmap tmpDMPix(dmSideWidSize_);
	tmpDMPix.fill(Qt::transparent);
	QPainter p(&tmpDMPix);

	//for: draw dm from toPaintPos to bottom of screen
	QPoint toPaintPos = dmUpPos_;					
	for (uint i = startPaintDMIndex_; i <= dmCreater_->totalDMCount_; i++){
		TKNS::DMTask *toPaintTk = dmCreater_->dmTaskHs_[i];

		if (toPaintTk->dmOpacity_ <= 0.0){
			//µ¯Ä»Ê±¼äÍÆ×§Îª999 ÔÙ½µµÍµ½×îµÍ, ·´¸´²Ù×÷ºó äÖÈ¾Ë³ÐòºÍÉ¾³ýË³Ðò
			//»á½»Ìæ, ±ÜÃâÄÚ´æÐ¹Â©¼°É¾³ý´íÂÒ×öÒÔÏÂÅÐ¶Ï
			if (toPaintTk->dmIndex_ > dmCreater_->dmTaskHs_[startPaintDMIndex_]->dmIndex_)
				continue;

			TKNS::DMTask *toDelTk = dmCreater_->dmTaskHs_.take(toPaintTk->dmIndex_);
			toDelTk->dmTState_ = TKNS::DMTaskState::HIDE_STATE;
			toPaintPos.setY(toPaintPos.y() + toDelTk->dmPixSize_.height());
			dmUpPos_ = toPaintPos;
			dmCreater_->totalDMHeight_ -= toDelTk->dmPixSize_.height();
			delete toDelTk->dmPix_;
			delete toDelTk;
			startPaintDMIndex_++;
			continue;
		}
		if (toPaintPos.y() >= 0) {
			int availableHeight = dmSideWidSize_.height() - toPaintPos.y();
			if (availableHeight >= toPaintTk->dmPixSize_.height()){
				toPaintTk->dmTState_ = TKNS::DMTaskState::SHOW_STATE;
				p.drawPixmap(toPaintPos, *(toPaintTk->dmPix_));
				toPaintPos.setY(toPaintPos.y() + toPaintTk->dmPixSize_.height());
			}
			else{
				//down to bottom of screen
				toPaintTk->dmTState_ = TKNS::DMTaskState::ADD_STATE;
				p.drawPixmap(toPaintPos, *(toPaintTk->dmPix_));
				toPaintTk->dmCurCutPos_.setY(availableHeight);
				toPaintTk->dmForDrawRect_.setTop(availableHeight);
				break;
			}
		}
		else{
			int availableHeight = qAbs(toPaintPos.y());
			if (availableHeight >= toPaintTk->dmPixSize_.height()){
				TKNS::DMTask *toDelTk = dmCreater_->dmTaskHs_.take(toPaintTk->dmIndex_);
				toDelTk->dmTState_ = TKNS::DMTaskState::HIDE_STATE;
				toPaintPos.setY(toPaintPos.y() + toDelTk->dmPixSize_.height());
				dmUpPos_ = toPaintPos;
				dmCreater_->totalDMHeight_ -= toDelTk->dmPixSize_.height();
				delete toDelTk->dmPix_;
				delete toDelTk;
				startPaintDMIndex_++;
			}
			else{
				int needPaintPixH = toPaintTk->dmPixSize_.height() - availableHeight;
				p.drawPixmap(0, 0
					, *(toPaintTk->dmPix_)
					, 0, availableHeight, toPaintTk->dmPixSize_.width(), needPaintPixH);
				toPaintTk->dmTState_ = TKNS::DMTaskState::SUB_STATE;
				toPaintTk->dmCurCutPos_.setY(availableHeight);
				toPaintTk->dmForDrawRect_.setTop(availableHeight);
				toPaintPos.setY(needPaintPixH);
			}
		}
	}

	if (startPaintDMIndex_ > dmCreater_->totalDMCount_) {
		drawStandBy_();
		p.drawPixmap(QPoint(0, tmpDMPix.height()-standByPix_->height()), *standByPix_);
	}

	dmSideWidBKPixCache_->fill(Qt::transparent);
	QPainter pSideDM(dmSideWidBKPixCache_);
	pSideDM.drawPixmap(0, 0, tmpDMPix);

	emit sglUpdateDMPix();
}

void DanmakuMove::sltRecvPreparedDM(int index) {

	if (index < startPaintDMIndex_)
		return;

	dmMoveUpTimer_->start();
	moveDM_();
	emit sglSendRenderDM(index);
}
