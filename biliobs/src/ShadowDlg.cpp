#include "ShadowDlg.h"
#include <QtMath>
#include <QPainter>
#include <QPalette>
#include <QMouseEvent>
#include <QMessageBox>
#include <QCloseEvent>
#include <QVBoxLayout >
#include <QGraphicsDropShadowEffect>
#include "BiLiOBSMainWid.h"

ShadowDlg::ShadowDlg(QWidget *parent)
	: QDialog(parent),
	mIsPressed(false){

	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
}

ShadowDlg::~ShadowDlg() { }

void ShadowDlg::mSetSolidWid(QWidget *sw, QSize swSize) {

	mSolidWid = sw;
	resize(swSize.width()+10, swSize.height()+10);

	QGraphicsDropShadowEffect *shadowEfct = new QGraphicsDropShadowEffect();
	shadowEfct->setBlurRadius(20.0);
	shadowEfct->setColor(QColor(0, 0, 0, 100));
	shadowEfct->setOffset(0, 0);
	mSolidWid->setGraphicsEffect(shadowEfct);

	QVBoxLayout * mainLayout = new QVBoxLayout();
	mainLayout->addWidget(mSolidWid, Qt::AlignCenter);
	mainLayout->addSpacing(0);
	mainLayout->setContentsMargins(5, 5, 5, 5);
	QWidget::setLayout(mainLayout);
}

void ShadowDlg::closeEvent(QCloseEvent *e) {

	if (mSolidWid->close()){
		delete mSolidWid;
		e->accept();
	}else
		e->ignore();
}

void ShadowDlg::mouseMoveEvent(QMouseEvent *e) {

	if (mIsPressed) {
		QPoint point = e->globalPos();
		move(point - mPoint);
		if (mSolidWid->objectName() == QString("BiLiOBSMainWid")){
			BiLiOBSMainWid  *sWid = static_cast<BiLiOBSMainWid *>(mSolidWid);
		}
	}
}

void ShadowDlg::mousePressEvent(QMouseEvent *e) {

	if (e->button() == Qt::LeftButton)
		mIsPressed = true;

	mPoint = e->globalPos() - pos();
}

void ShadowDlg::mouseReleaseEvent(QMouseEvent *e) {
	mIsPressed = false;
}