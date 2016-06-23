#include "danmakurender.h"
#include "danmakucreate.h"
#include "danmakumove.h"
#include "danmakuwid.h"

#include <QFont>
#include <QPainter>
#include <QRectF>
#include <QTimer>

DanmakuRender *DanmakuRender::inst_ = nullptr;
DanmakuRender *DanmakuRender::getInst_() {
	return inst_ ? inst_ : new DanmakuRender;
}

DanmakuRender::DanmakuRender(QObject *parent)
	: QObject(parent)
	, staySecond_(5){

	inst_ = this;
	inst_->setObjectName("DanmakuRender");
	DanmakuWid::objects_["DanmakuRender"] = this;

	dmOpacityTimer_ = new QTimer(this);
	dmOpacityTimer_->setInterval(40);
	QObject::connect(dmOpacityTimer_, &QTimer::timeout, [this](){
		renderDM_();
	});
	dmOpacityTimer_->start();

	dmSideTopTimer_ = new QTimer(this);
	dmSideTopTimer_->setInterval(3000);
	QObject::connect(dmSideTopTimer_, &QTimer::timeout, [this](){
		emit sglTopDM();
	});
	dmSideTopTimer_->start();
}

DanmakuRender::~DanmakuRender() {
	dmOpacityTimer_->stop();
	dmSideTopTimer_->stop();
	toRenderDMIndexL.clear();
	inst_ = nullptr;
}

void DanmakuRender::setOpacityDecreaseSpeed_(){

	opacityDecreaseSpeed_ = 1.0 / (float)25;
	dmCreater_->dmOpacityIdle_ = staySecond_;
}
void DanmakuRender::renderDM_() {

	QMutexLocker m(&dmCreater_->dmHashMutex_);
	if (toRenderDMIndexL.size()){
		for (int i = 0; i < toRenderDMIndexL.size(); ){
			if (toRenderDMIndexL[i] < dmMover_->startPaintDMIndex_) {
				toRenderDMIndexL.removeOne(toRenderDMIndexL[i]);
				continue;
			}
			else{
				TKNS::DMTask *toRenderTk = dmCreater_->dmTaskHs_[toRenderDMIndexL[i]];
                if ((!toRenderTk) ||(toRenderTk->dmOpacity_ <= 0.0)){
					toRenderDMIndexL.removeOne(toRenderDMIndexL[i]);
					continue;
				}
				else{
					toRenderTk->dmOpacity_ -= opacityDecreaseSpeed_;
					toRenderTk->dmPix_->fill(Qt::transparent);
					QPainter p(toRenderTk->dmPix_);
					p.setOpacity(toRenderTk->dmOpacity_);
					p.drawPixmap(0, 0, toRenderTk->dmPixBackup_);
				}
			}
			i++;
		}
	}
}

void DanmakuRender::sltRecvToRenderDM(int dmIndex) {

	if (dmIndex < dmMover_->startPaintDMIndex_)
		return;
	toRenderDMIndexL.append(dmIndex);
}

void DanmakuRender::changedStayTime_(int second) {

	staySecond_ = second;
	setOpacityDecreaseSpeed_();
}