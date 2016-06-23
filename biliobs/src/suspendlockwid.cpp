#include "suspendlockwid.h"
#include <windows.h>
#include <QToolTip>


SuspendLockWid::SuspendLockWid(QWidget *parent)
	: QLabel(parent) {

	ui.setupUi(this);

	//setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
	SetWindowPos((HWND)winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	installEventFilter(this);

	//setPixmap(QPixmap(QString::fromUtf8(":/FucBtn/DMUnlock")));
	setStyleSheet("border-image:url(:/FucBtn/DMUnlock); background: rgb(67, 67, 67);");
	bg_style_ = "background: rgb(67, 67, 67); ";
	icon_style_ = "border-image:url(:/FucBtn/DMUnlock); ";
}

SuspendLockWid::~SuspendLockWid() { }

void SuspendLockWid::changeOpacity(float d)
{
	int i = d * 100;
	if (i > 100)
		i = 100;

	setWindowOpacity(d);

	//setStyleSheet(icon_style_ + bg_style_);
}

bool SuspendLockWid::eventFilter(QObject *o, QEvent *e) {
	if (qobject_cast<SuspendLockWid *>(o) == this){
		switch (e->type()) {
//			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease: {
				emit sglClicked();
				break;
			}
			case QEvent::Enter: {
				setCursor(Qt::PointingHandCursor);
				//setPixmap(QPixmap(QString::fromUtf8(":/FucBtn/DMUnlockH")));
				icon_style_ = "border-image:url(:/FucBtn/DMUnlockH);";
				setStyleSheet(icon_style_ + bg_style_);
				QToolTip::showText(QCursor::pos(), tr("Unlock"), 0, QRect());
				break;
			}
			case QEvent::Leave: {
				setCursor(Qt::ArrowCursor);
				icon_style_ = "border-image:url(:/FucBtn/DMUnlock);";
				setStyleSheet(icon_style_ + bg_style_);
				//setPixmap(QPixmap(QString::fromUtf8(":/FucBtn/DMUnlock")));
				break;
			}
		}
	}
	return QWidget::eventFilter(o, e);
}