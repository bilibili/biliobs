#include <QPainter>
#include <functional>
#include "bili_area_tool.h"

#define WID_W	100
#define WID_H	100

bili_area_zoom::bili_area_zoom(std::function<const QPixmap&()>&& bkPix, QWidget *parent)
	:QWidget(parent)
	,mBKPix(bkPix){

	resize(WID_W, WID_W);
	mPaintPix = mBKPix();
}

bili_area_zoom::~bili_area_zoom() { 
}

void bili_area_zoom::mMove(QPoint pos, int w, int h){

	int spacing = 20;
	QPoint newPos = pos+QPoint(spacing, spacing);
	if ((w - newPos.x()) < WID_W)
		newPos.setX(pos.x()-WID_W-spacing);
	if ((h - newPos.y()) < WID_H)
		newPos.setY(pos.y()-WID_H-spacing);

	move(newPos);
}

void bili_area_zoom::mSetZoomPos(QPoint pos){

	QRect r(pos+QPoint(-WID_W/4, -WID_H/4), pos+QPoint(WID_H/4, WID_H/4));
	mPaintPix = mBKPix().copy(r).scaled(QSize(WID_W, WID_H));
}

void bili_area_zoom::paintEvent(QPaintEvent *e){

	QPainter p(this);
	p.drawPixmap(0, 0, mPaintPix);

	QPen penBorder;
	penBorder.setWidth(2);
	penBorder.setColor(QColor(105, 165, 205, 255));
	p.setPen(penBorder);
	QRect r = rect();
	p.drawRect(r);

	QPoint centerPos = r.center();
	int halfW = WID_W / 2;
	int halfH = WID_H / 2;
	p.drawLine(centerPos.x()-halfW, centerPos.y(), centerPos.x()+halfW, centerPos.y());
	p.drawLine(centerPos.x(), centerPos.y()-halfH, centerPos.x(), centerPos.y()+halfH);

	QWidget::paintEvent(e);
}

////////////////////////////////////////////////////////////////////////////////////

bili_area_tool::bili_area_tool(QWidget *parent)
	: QWidget(parent) {
	ui.setupUi(this);

	QObject::connect(ui.OkBtn, &QPushButton::clicked, std::bind(&bili_area_tool::mSglBtnClicked, this, 0));
	QObject::connect(ui.CancelBtn, &QPushButton::clicked, std::bind(&bili_area_tool::mSglBtnClicked, this, 1));
}

bili_area_tool::~bili_area_tool() {

}
