#include "danmakucreate.h"
#include "danmakuwid.h"

#include <QPainter>
#include <QRect>
#include <QTextDocument>

#if 1
#include <QThread>
#include <QDebug>
#include <QTime>
#endif

DanmakuCreate *DanmakuCreate::inst_ = nullptr;
DanmakuCreate *DanmakuCreate::getInst_(){
	return inst_ ? inst_ : new DanmakuCreate ;
}

DanmakuCreate::DanmakuCreate(QObject *parent)
	: QObject(parent)
	,totalDMCount_(0)
	,dmAverageHeight_(0)
	,totalDMHeight_(0)
	,danmakuFontSize_(20)
	,dmOpacity_(1.0){

	danmakuStyle_ = QString(
		"<p>"
		"<span style = \" font-size:%1px; color:#4FC1E9; \">%2: < / span>"
		"<span style = \" font-size:%3px; color:#FFFFFF; \">%4< / span> "
		"</p>");

	giftStyle_ = QString(
		"<p>"
		"<span style = \" font-size:%1px; color:#FF8F34; \">%2 < / span>"
		"<span style = \" font-size:%3px; color:#FFFFFF; \">%4x%5< / span>"
		"</p>");

	laoyeStyle_ = QString(
		"<p>"
		"<span style = \" font-size:%1px; color:#FF8F34; \">%2< / span>"
		"</p>");
	
	announceStyle_ = QString(
		"<p>"
		"<span style = \" font-size:%1px; color:#FF9DCB; \">%2< / span>"
		"</p>");

	//提前预留空间???
	//dmTaskHs_.reserve(200000);
	inst_ = this;
	inst_->setObjectName("DanmakuCreate");
	DanmakuWid::objects_["DanmakuCreate"] = this;

}

DanmakuCreate::~DanmakuCreate() { 
	dmTaskHs_.clear();
	inst_ = nullptr;
}

void DanmakuCreate::setDMPix_(TKNS::DMTask *dmTk) {

	QFont font;
	font.setBold(true);
	font.setFamily(QString("Microsoft YaHei"));
	font.setPixelSize(danmakuFontSize_);

	QString dm = dmTk->dmStr_;
	QStringList lines = dm.split("|:|");
	QString cmd = lines.first();
	if (!cmd.compare("DANMU_MSG", Qt::CaseInsensitive))
		dm = danmakuStyle_.arg(font.pixelSize()).arg(lines[1].trimmed()).arg(font.pixelSize()).arg(lines[2].trimmed());
	else if (!cmd.compare("SEND_GIFT", Qt::CaseInsensitive))
		dm = giftStyle_.arg(font.pixelSize()).arg(lines[1]).arg(font.pixelSize()).arg(lines[2]).arg(lines[3]);
	else if (!cmd.compare("WELCOME", Qt::CaseInsensitive))
		dm = laoyeStyle_.arg(font.pixelSize()).arg(lines[1]);
	else if (!cmd.compare("SYS_MSG", Qt::CaseInsensitive))
		dm = announceStyle_.arg(font.pixelSize()).arg(lines[1]);

	QTextDocument td;
	td.setDefaultFont(font);
	td.setDocumentMargin(10);
	td.setHtml(dm);

	td.setTextWidth(dmWidth_);
	QSize s = td.size().toSize();

	dmTk->dmPixSize_ = s;
	dmTk->dmForDrawRect_.setSize(s);
	dmTk->dmOpacity_ = dmOpacity_ + dmOpacityIdle_;

	dmTk->dmPix_ = new QPixmap(s);
	dmTk->dmPix_->fill(Qt::transparent);

	QPainter p(dmTk->dmPix_);
	p.setPen(QPen(QColor(0x49545A)));
	p.setBrush(QBrush(QColor(0x49545A), Qt::SolidPattern));
	p.setFont(font);

	int border = 1;
	QRect r(0, 0, s.width(), s.height());
	p.setOpacity(0.9);
	p.drawRoundedRect(r.adjusted(border, border, -border, -border), 6, 6);

	td.drawContents(&p);
	dmTk->dmPixBackup_ = *dmTk->dmPix_;

}

void DanmakuCreate::sltRecvDM(QString dm) {

	QMutexLocker m_(&dmHashMutex_);
	TKNS::DMTask *dmTk = new TKNS::DMTask;
	dmTk->dmIndex_ = totalDMCount_+1;
	dmTk->dmStr_ = dm;
	setDMPix_(dmTk);
	dmAverageHeight_ = dmAverageHeight_ ? (dmAverageHeight_ + dmTk->dmPixSize_.height()) / 2 : dmTk->dmPixSize_.height();
	dmTaskHs_[dmTk->dmIndex_] = dmTk;
	totalDMHeight_ += dmTk->dmPixSize_.height();

	++totalDMCount_;

	emit sglPreparedDM(dmTk->dmIndex_);
}