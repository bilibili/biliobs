#include "danmakuwid.h"
#include "flowlayout.h"

#include "danmakucreate.h"
#include "danmakumove.h"
#include "danmakurender.h"
#include "tasknamespace.h"

#if HAS_LOCKDM
#include "suspendlockwid.h"
#endif

#include <QPainter>
#include <QThread>
#include <QMouseEvent>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QToolTip>
#include <QSystemTrayIcon>
#include <windows.h>

#include <qpainter.h>
#include <qpen.h>
#include <qcolor.h>

QHash<QString, QObject *> DanmakuWid::objects_;
DanmakuWid::DanmakuWid(QWidget *parent)
	: QWidget(parent)
	, isPress_(false)
	, pressPoint_(QPoint(0, 0))
	, canDragScale_(false)
	, lockTrans_(false)
	, isTop_(true)
	, checkTransMouseTimer_(nullptr)
	, dmCreater_(nullptr)
	, dmMover_(nullptr)
	, dmRender_(nullptr)
	, bkPix_(nullptr)
	, bkPixCache_(nullptr) {

	ui.setupUi(this);
	setAttribute(Qt::WA_TranslucentBackground);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose);

	checkTransMouseTimer_ = new QTimer;
	checkTransMouseTimer_->setInterval(100);
	QObject::connect(checkTransMouseTimer_, &QTimer::timeout, [this](){
		QPoint curPos = QCursor::pos();
		QRect stateWidRect = ui.stateWid->geometry();
		QRect stateWidGlobalRect = QRect(pos().x(), pos().y()+stateWidRect.y(), stateWidRect.width(), stateWidRect.height());
		if ((stateWidGlobalRect.contains(curPos))||(!this->geometry().contains(curPos)) ){					//do not transparent mouseevent when cursor within stateWid
			this->checkTransMouseTimer_->stop();
			SetWindowLongPtr((HWND)winId(), GWL_EXSTYLE, GetWindowLongPtr((HWND)winId(), GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
			if (isTop_)
				SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	});

	setupStateWidLaout_();

	installEventFilter(this);
	dragScaleLab_->installEventFilter(this);
	dmWid_ = ui.dmWid;
	dmWid_->installEventFilter(this);

#if HAS_LOCKDM
	suspendLockWid_ = new SuspendLockWid;
	suspendLockWid_->setObjectName("SuspendLockWid");

	QObject::connect(suspendLockWid_, &SuspendLockWid::sglClicked, [this](){
		suspendLockWid_->setHidden(true);
		//lockBtn_->setHidden(false);
		lockBtn_->setEnabled(true);
		setLockTransState_();
	});
#endif

    setNetState_(-1);
    setNetUpSpeed_(-1);
}


DanmakuWid::~DanmakuWid() {

	releaseObjects_() ;

	checkTransMouseTimer_->stop();

#if HAS_LOCKDM
	suspendLockWid_->close();
#endif
}

bool DanmakuWid::eventFilter(QObject *o, QEvent *e) {

	if (qobject_cast<QWidget *>(o) == dmWid_){
		switch (e->type()) {
			case QEvent::Paint: {
				QPainter p(dmWid_);
				p.drawPixmap(0, dmWid_->height() - bkPix_->height(), *bkPix_);
				break;
			}
			case QEvent::Resize:{
				if (canDragScale_)
					setDMParams_();
				break;
			}
			case QEvent::Enter:{
				SetWindowLongPtr((HWND)winId(), GWL_EXSTYLE, GetWindowLongPtr((HWND)winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT);
				if (isTop_)
					SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				checkTransMouseTimer_->start();
				break;
			}
		}
	}
	else if (qobject_cast<DanmakuWid *>(o) == this){
		switch (e->type()) {
			case QEvent::MouseButtonDblClick:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseMove:
			case QEvent::Enter:
			case QEvent::Leave: {
				setDragCursorShape_(e);
				break;
			}
		}
	}
	else if (qobject_cast<QLabel *>(o) == dragScaleLab_){
		switch (e->type()){
			case QEvent::Enter: 
				canDragScale_ = true;
				setCursor(Qt::SizeFDiagCursor);
				break;
			case QEvent::Leave:{
				canDragScale_ = false;
				setCursor(Qt::ArrowCursor);
				break;
			}
		}
	}

	return QWidget::eventFilter(o, e);
}

void DanmakuWid::paintEvent(QPaintEvent *e)
{
    if (isPress_ && canDragScale_) {
        QPainter painter(this);
        QPen pen = painter.pen();
        pen.setWidth(2);
        pen.setColor(QColor(105, 165, 205, 255));
        painter.setPen(pen);
        painter.drawRect(1, 1, width() - 2, height() - 2);
    }
}

void DanmakuWid::initDMThreads_(){

	dmCreater_ = DanmakuCreate::getInst_();
	dmMover_ = DanmakuMove::getInst_();
	dmRender_ = DanmakuRender::getInst_();

	dmMover_->dmCreater_ = dmCreater_;
	dmRender_->dmCreater_ = dmCreater_;
	dmRender_->dmMover_ = dmMover_;

	dmCreater_->dmOpacity_ = 1.0;
	dmCreater_->danmakuFontSize_ = 20;
	dmRender_->staySecond_ = 5;
	dmRender_->setOpacityDecreaseSpeed_();

	dmMover_->dmUpPos_ = QPoint(0, dmWid_->height());

	auto setupDMConnect = [this](){
		QObject::connect(this, SIGNAL(sglSendDM(QString)), dmCreater_, SLOT(sltRecvDM(QString)), Qt::QueuedConnection);
		QObject::connect(dmCreater_, SIGNAL(sglPreparedDM(int)), dmMover_, SLOT(sltRecvPreparedDM(int)), Qt::QueuedConnection);
		QObject::connect(dmMover_, SIGNAL(sglUpdateDMPix()), this, SLOT(sltUpdateDMPix()), Qt::QueuedConnection);
		QObject::connect(dmMover_, SIGNAL(sglSendRenderDM(int)), dmRender_, SLOT(sltRecvToRenderDM(int)), Qt::QueuedConnection);
		QObject::connect(dmRender_, &DanmakuRender::sglTopDM, this, [this](){ 
			if (isTop_){
				raise();
				if (!suspendLockWid_->isHidden())
					suspendLockWid_->raise();
			}
		}, Qt::QueuedConnection);
		QObject::connect(this, &DanmakuWid::sglUpdateStandByPix, dmMover_, [this](){
			dmMover_->isNeedUpdateStandBy_ = true;
		}, Qt::QueuedConnection);
	};
	setupDMConnect();
}

void DanmakuWid::startDMThreads_() {

	auto startDMThread = [this](){
		auto dmCreaterT = new QThread(this);
		auto dmMoverT = new QThread(this);
		auto dmRenderT = new QThread(this);
		dmCreaterT->setObjectName("DMCreateThread");
		dmMoverT->setObjectName("DMMoveThread");
		dmRenderT->setObjectName("DMRenderThread");
		DanmakuWid::objects_["DMCreateThread"] = dmCreaterT;
		DanmakuWid::objects_["DMMoveThread"] = dmMoverT;
		DanmakuWid::objects_["DMRenderThread"] = dmRenderT;

		dmCreater_->moveToThread(dmCreaterT);
		dmMover_->moveToThread(dmMoverT);
		dmRender_->moveToThread(dmRenderT);

		QObject::connect(dmCreaterT, &QThread::finished, dmCreater_, &QObject::deleteLater);
		QObject::connect(dmMoverT, &QThread::finished, dmMover_, &QObject::deleteLater);
		QObject::connect(dmRenderT, &QThread::finished, dmRender_, &QObject::deleteLater);

		dmCreaterT->start();
		dmMoverT->start();
		dmRenderT->start();
	};
	startDMThread();
}

void DanmakuWid::releaseObjects_() {

	QThread *curT = qobject_cast<QThread *>(DanmakuWid::objects_["DMMoveThread"]);
	if (curT){
		curT->quit();
		curT->wait();
		curT = qobject_cast<QThread *>(DanmakuWid::objects_["DMRenderThread"]);
		curT->quit();
		curT->wait();
		curT = qobject_cast<QThread *>(DanmakuWid::objects_["DMCreateThread"]);
		curT->quit();
		curT->wait();
	}

	delete bkPix_;
	delete bkPixCache_;

	DanmakuWid::objects_.clear();
}

void DanmakuWid::changeOpacity_(int opacity) {

    double opa = double(opacity) / 100.0;

    if (opa < 0.05)
        opa = 0.05;
    else if (opa > 1)
        opa = 1;

	setWindowOpacity(opa);

	suspendLockWid_->changeOpacity(opa);
}

void DanmakuWid::changeStayTime_(int second) {

	QMutexLocker m(&dmCreater_->dmHashMutex_);
	dmRender_->changedStayTime_(second);
}

void DanmakuWid::changeFontSize_(QString fontSize) {

	QMutexLocker m(&dmCreater_->dmHashMutex_);
	bool isOk;
	int fSize = fontSize.toInt(&isOk);
	if (isOk)
		dmCreater_->danmakuFontSize_ = fSize;
	else
		dmCreater_->danmakuFontSize_ = 20;
}

void DanmakuWid::initBKPix_(){

	if (bkPix_){
		delete bkPix_;
		delete bkPixCache_;
	}
	QSize s = dmWid_->size();
	bkPix_ = new QPixmap(s);
	bkPix_->fill(Qt::transparent);

	bkPixCache_ = new QPixmap(s);
	bkPixCache_->fill(Qt::transparent);
}

void DanmakuWid::setDMParams_() {

	QMutexLocker m(&dmCreater_->dmHashMutex_);
	QSize s = dmWid_->size();

	*bkPix_ = bkPix_->scaled(s);
	*bkPixCache_ = bkPixCache_->scaled(s);

	dmCreater_->dmWidth_ = s.width();

	dmMover_->dmSideWidSize_ = s;
	dmMover_->dmSideWidBKPixCache_ = bkPixCache_;

	dmMover_->drawStandBy_();
	sltUpdateDMPix();
}

void DanmakuWid::sltUpdateDMPix() {

	bkPix_->fill(Qt::transparent);
	QPainter p(bkPix_);
	p.drawPixmap(0, 0, *bkPixCache_);

	update();
}


void DanmakuWid::mousePressEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton)
		isPress_ = true;

    if (isPress_ && canDragScale_)
        update();
    
	pressPoint_ = pos() - e->globalPos();
	QWidget::mousePressEvent(e);
}

void DanmakuWid::mouseMoveEvent(QMouseEvent *e) {

	if (isPress_){
		if (canDragScale_){
			QPoint gPoint = e->globalPos();
			QRect rect = this->rect();

			QPoint topLeft = mapToGlobal(rect.topLeft());
			QPoint bottomRight = mapToGlobal(rect.bottomRight());

			QRect moveRect(topLeft, bottomRight);
			moveRect.setWidth(gPoint.x() - topLeft.x());
			moveRect.setHeight(gPoint.y() - topLeft.y());
			setGeometry(moveRect);
			emit sglUpdateStandByPix();
		}else
			move( e->globalPos() + pressPoint_ );
	}
	QWidget::mouseMoveEvent(e);
}

void DanmakuWid::mouseReleaseEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton)
		isPress_ = false;

    update();
	QWidget::mouseReleaseEvent(e);
}

void DanmakuWid::setNetState_(int state){

	switch(state){
		case 0:{	//good
			colorLab_->setStyleSheet("background: rgb(73, 195, 32);");
			break;
		}
		case 1:{	//bad
			colorLab_->setStyleSheet("background: red;");
			break;
		}
		default:{	//broken
			colorLab_->setStyleSheet("background: gray;");
			break;
		}
	}
}

void DanmakuWid::setNetUpSpeed_(int speed){

	QString speedStr;
	if (speed < 0)
		speedStr = QString("--");
	else
		speedStr = QString::number(speed);
	speedStr += "kbps";
	uploadSpeedVolLab_->setText(speedStr);
}

void DanmakuWid::setFrameLostRate_(float rate){

	QString str;
	if (0 > rate)
		str = QString("--");
	else {
		if (rate > 100)
			rate = 100;
		int tmp;
		if ((int)rate) {
			str = QString::number((int)rate);
			str += QString(".");
			tmp = (int)((rate - (int)rate) * 100);
		}
		else {
			str = QString("0.");
			tmp = (int)(rate * 100);
		}
		if (tmp < 10)
			str += QString::number(0) + QString::number(tmp);
		else
			str += QString::number(tmp);
		str += "%";
	}
	frameLostRateVolLab_->setText(str);
}

void DanmakuWid::setNumOfAudience_(int num){
	audienceVolLab_->setText(QString::number(num));
}

void DanmakuWid::setNumOfFans_(int num){
	fansVolLab_->setText(QString::number(num));
}

void DanmakuWid::setupStateWidLaout_() {

	colorLab_ = new QLabel;
	colorLab_->setMinimumSize(16, 16);
	colorLab_->setStyleSheet("background: rgb(73, 195, 32);");
	uploadSpeedLab_ = new QLabel(tr("Upload Speed: "));
	uploadSpeedLab_->setObjectName("UploadSpeedLab");
	uploadSpeedVolLab_ = new QLabel("0kpbs");
	uploadSpeedVolLab_->setObjectName("UploadSpeedVolLab");
	QSpacerItem *item1 = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto ascendingLay = new QHBoxLayout;
	ascendingLay->addWidget(colorLab_);
	ascendingLay->addItem(item1);
	ascendingLay->addWidget(uploadSpeedLab_);
	ascendingLay->addWidget(uploadSpeedVolLab_);

	frameLostRateLab_ = new QLabel(tr("Frame Lost Rate: "));
	frameLostRateLab_->setObjectName("FrameLostRateLab");
	frameLostRateVolLab_ = new QLabel("0%");
	frameLostRateVolLab_->setObjectName("FrameLostRateVolLab");
	auto frameLostLay = new QHBoxLayout;
	frameLostLay->addWidget(frameLostRateLab_);
	frameLostLay->addWidget(frameLostRateVolLab_);

	audienceLab_ = new QLabel(tr("Audience: "));
	audienceLab_->setObjectName("AudienceLab");
	audienceVolLab_ = new QLabel("0");
	audienceVolLab_->setObjectName("AudienceVolLab");
	auto audienceLay = new QHBoxLayout;
	audienceLay->addWidget(audienceLab_);
	audienceLay->addWidget(audienceVolLab_);

	fansLab_ = new QLabel(tr("Fans: "));
	fansLab_->setObjectName("FansLab");
	fansVolLab_ = new QLabel("0");;
	fansVolLab_->setObjectName("FansVolLab");
	auto fansLay = new QHBoxLayout;
	fansLay->addWidget(fansLab_);
	fansLay->addWidget(fansVolLab_);

	switchLab_ = new QLabel(tr("Danmaku Switch: "));;
	switchLab_->setObjectName("SwitchLab");
	switchCB_ = new QCheckBox;
	switchCB_->setObjectName("DMOnOff");
	switchCB_->setFixedSize(QSize(44, 18));
	auto switchLay = new QHBoxLayout;
	switchLay->addWidget(switchLab_);
	switchLay->addWidget(switchCB_);

	settingBtn_ = new QPushButton;
	topBtn_     = new QPushButton;
	lockBtn_    = new QPushButton;
	minBtn_		= new QPushButton;

	settingBtn_->setFixedSize(QSize(21, 18));
	settingBtn_->setObjectName("DanmakuSettingBtn");
	settingBtn_->setToolTip(tr("Settings"));

	topBtn_->setFixedSize(QSize(21, 18));
	topBtn_->setObjectName("DanmakuTopBtn");
	topBtn_->setToolTip(tr("Cancel Top"));

	lockBtn_->setFixedSize(QSize(21, 18));
	lockBtn_->setObjectName("DanmakuLockBtn");
	lockBtn_->setToolTip(tr("Lock"));

	minBtn_->setFixedSize(QSize(21, 18));
	minBtn_->setObjectName("DanmakuMinBtn");
	minBtn_->setToolTip(tr("Minimize"));

	dragScaleLab_ = new QLabel;
	dragScaleLab_->setProperty("FixRightBottomConner", QVariant::fromValue(true));
	dragScaleLab_->setPixmap(QPixmap(QString::fromUtf8(":/FucBtn/DragCornerLab")));

	QSpacerItem *item2 = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding);
	QSpacerItem *item3 = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding);
	QSpacerItem *item4 = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding);
	QSpacerItem *item5 = new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding);
	//QSpacerItem *item6 = new QSpacerItem(5, 30, QSizePolicy::Fixed, QSizePolicy::Expanding);
	QSpacerItem *item7 = new QSpacerItem(5, 30, QSizePolicy::Fixed, QSizePolicy::Expanding);

	FlowLayout *lay = new FlowLayout;
	lay->addItem(ascendingLay);
	lay->addItem(item2);
	lay->addItem(frameLostLay);
	lay->addItem(item3);
	lay->addItem(audienceLay);
	lay->addItem(item4);
	lay->addItem(fansLay);
	lay->addItem(item5);
	lay->addItem(switchLay);
	//lay->addItem(item6);
	lay->addWidget(settingBtn_);
	lay->addWidget(topBtn_);
	lay->addWidget(minBtn_);
#if HAS_LOCKDM
	lay->addWidget(lockBtn_);
	QObject::connect(lockBtn_, &QPushButton::clicked, [this](){
		//lockBtn_->setHidden(true);
		lockBtn_->setEnabled(false);
		setLockTransState_();
		suspendLockWid_->setGeometry(getLockBtnGeometry_());
		suspendLockWid_->show();
		suspendLockWid_->raise();
	});
#endif
	lay->addItem(item7);
	lay->addWidget(dragScaleLab_);

	ui.stateWid->setLayout(lay);

	QObject::connect(minBtn_, &QPushButton::clicked, [this](){
		trayicon_->show();
		trayicon_->showMessage(tr("Danmaku control panel minimize to systemicon"), tr("Click to set control panel visable"), QSystemTrayIcon::Information, 3000);
		ui.stateWid->setHidden(true);
	});

	QObject::connect(settingBtn_, SIGNAL(clicked()), this, SIGNAL(sglSettingsDM()));
	QObject::connect(topBtn_, &QPushButton::clicked, [this](){
		isTop_ = !isTop_;
		if (isTop_){
			topBtn_->setToolTip(tr("Cancel Top"));
			topBtn_->setStyleSheet( "QPushButton:hover{ \
										image: url(:/FucBtn/DMTopH); \
									} \
									QPushButton{ \
										border-style: outset; \
										image: url(:/FucBtn/DMTop); \
									} ");
			SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			raise();
		}
		else{
			topBtn_->setToolTip(tr("Top"));
			topBtn_->setStyleSheet( "QPushButton:hover{ \
										image: url(:/FucBtn/NoTopH); \
									} \
									QPushButton{ \
										border-style: outset; \
										image: url(:/FucBtn/NoTop); \
									} ");
			SetWindowPos((HWND)winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	});
	QObject::connect(switchCB_, &QCheckBox::stateChanged, [this](int state){
		if (state == Qt::Checked)
			ui.dmWid->setHidden(true);
		else
			ui.dmWid->setHidden(false);
	});

	uploadSpeedLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #9F9F9F; }");
	uploadSpeedVolLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #FFFFFF; }");
	frameLostRateLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #9F9F9F; }");
	frameLostRateVolLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #FFFFFF; }");
	audienceLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #9F9F9F; }");
	audienceVolLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #FFFFFF; }");
	fansLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #9F9F9F; }");
	fansVolLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #FFFFFF; }");
	switchLab_->setStyleSheet( "QLabel{ \
						font-family: \"Microsoft YaHei\"; \
						font-size: 12px; \
						color: #9F9F9F; }");

	trayicon_ = new QSystemTrayIcon(this);
	trayicon_->setIcon(QIcon(QString::fromUtf8(":/FucBtn/DMTryicon")));
	QObject::connect(trayicon_, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::ActivationReason reason){
		switch (reason){
			case QSystemTrayIcon::Trigger:
			case QSystemTrayIcon::DoubleClick: {
				trayicon_->hide();
				ui.stateWid->setHidden(false);
				break;
			}
			default:
				break;
		}
	});
}

#if HAS_LOCKDM
void DanmakuWid::setLockTransState_() {

	lockTrans_ = !lockTrans_;
	if (lockTrans_){
		SetWindowLongPtr((HWND)winId(), GWL_EXSTYLE, GetWindowLongPtr((HWND)winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else{
		SetWindowLongPtr((HWND)winId(), GWL_EXSTYLE, GetWindowLongPtr((HWND)winId(), GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
		SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

QRect DanmakuWid::getLockBtnGeometry_() {

	QPoint lockBtnPos = mapToGlobal(lockBtn_->mapTo(this, QPoint(0, 0)));
	QRect r = QRect(lockBtnPos-QPoint(-1, 1), lockBtnPos + QPoint(lockBtn_->width(), lockBtn_->height()));
	return r;
}

#endif

void DanmakuWid::setDragCursorShape_(QEvent *e) {

	QEvent::Type eType = e->type();
	if ((eType == QEvent::Enter) || (eType == QEvent::Leave)){

		QPoint dragPaddingWidth = QPoint(5, 5);
		QPoint curPos = QCursor::pos();

		QPoint bottomRight = mapToGlobal(rect().bottomRight());
		QRect dragRect = QRect(QPoint(bottomRight.x() - dragScaleLab_->width(), bottomRight.y() - dragScaleLab_->height()) - dragPaddingWidth,
			bottomRight + dragPaddingWidth);

		//QPoint dragLabPos = mapToGlobal( dragScaleLab_->mapTo(this, QPoint(0, 0)) );
		//QRect dragRect = QRect( dragLabPos - dragPaddingWidth, bottomRight + dragPaddingWidth);

		if (dragRect.contains(curPos)){
			canDragScale_ = true;
			setCursor(Qt::SizeFDiagCursor);
		}
		else{
			canDragScale_ = false;
			setCursor(Qt::ArrowCursor);
		}
	}
}

bool DanmakuWid::transMouse_() {

	QPoint curPos = QCursor::pos();
	QRect rect = dmWid_->geometry();
	QRect globalRect = QRect(pos().x(), pos().y() + rect.y(), rect.width(), rect.height());
	if (globalRect.contains(curPos)){					//transparent mouseevent when cursor within dmWid
		SetWindowLongPtr((HWND)winId(), GWL_EXSTYLE, GetWindowLongPtr((HWND)winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT);
		if (isTop_)
			SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		checkTransMouseTimer_->start();
		return true;
	}
	return false;
}
