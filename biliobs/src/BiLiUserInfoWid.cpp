#include "BiLiUserInfoWid.h"
#include <QMouseEvent>
#include <time.h>
#include <QPainter>

#include "../biliapi/IBiliApi.h"

#include "bili_effect_widget.h"

BiLiUserInfoWid::BiLiUserInfoWid()
    : RoundedWidget(Qt::white)
{
	ui.setupUi(this);

	QFont font = ui.roomManageLab->font();
	font.setLetterSpacing(QFont::AbsoluteSpacing, -1);
	//font.setStretch(100);
	ui.roomManageLab->setFont(font);
	ui.infoModifyLab->setFont(font);
	ui.roomManageLab->setFont(font);
	ui.roomManageLab->setFont(font);
	ui.RoomGuestDisplay->setFont(font);
	ui.ScoreDisplayLab->setFont(font);

	QFont name_font = ui.NameLab->font();
	name_font.setLetterSpacing(QFont::AbsoluteSpacing, -1);
	ui.NameLab->setFont(name_font);

	ui.NameLab->installEventFilter(this);
	ui.UserFaceLab->installEventFilter(this);
	ui.roomManageIcon->installEventFilter(this);
	ui.roomManageLab->installEventFilter(this);
	ui.infoModifyIcon->installEventFilter(this);
	ui.infoModifyLab->installEventFilter(this);

	ui.NameLab->setCursor(Qt::PointingHandCursor);
	ui.UserFaceLab->setCursor(Qt::PointingHandCursor);
	ui.roomManageIcon->setCursor(Qt::PointingHandCursor);
	ui.roomManageLab->setCursor(Qt::PointingHandCursor);
	ui.infoModifyIcon->setCursor(Qt::PointingHandCursor);
	ui.infoModifyLab->setCursor(Qt::PointingHandCursor);

    effect_widget_ = new BiliEffectWidget(this);

}

BiLiUserInfoWid::~BiLiUserInfoWid() {

}

void BiLiUserInfoWid::mSetUserName(QString usrName) {

	full_name_ = usrName;

	QFontMetrics font_metric(ui.NameLab->font());

	if (font_metric.width(full_name_, -1) > ui.NameLab->maximumWidth()) {
		QString end_terminator = QString("...");
		int limit = ui.NameLab->maximumWidth() - font_metric.width(end_terminator, -1);
		int char_len;
		for (char_len = 1; font_metric.width(full_name_, char_len) <= limit; char_len++);

		QString display_text = full_name_;
		display_text.resize(char_len);
		display_text.append(end_terminator);
		ui.NameLab->setText(display_text);
		
	} else {
		ui.NameLab->setText(full_name_);
	}

}
void BiLiUserInfoWid::mSetUserFace(QPixmap& usrFace) {


	QPainterPath path;
	path.addEllipse(0, 0, usrFace.width(), usrFace.height());

	QPixmap backPixmap(usrFace.width(), usrFace.height());
	backPixmap.fill();
	QPainter p(&backPixmap);
	p.setClipPath(path);
	p.drawPixmap(0, 0, usrFace.width(), usrFace.height(), usrFace);
	ui.UserFaceLab->setPixmap(backPixmap);
}
void BiLiUserInfoWid::mSetGuest(QString guestNumStr) {
	ui.RoomGuestLab->setText(guestNumStr);
}
void BiLiUserInfoWid::mSetScore(QString scoreStr) {
	ui.ScoreLab->setText(scoreStr);
}

bool BiLiUserInfoWid::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == static_cast<QObject*>(ui.NameLab) || watched == static_cast<QObject*>(ui.UserFaceLab))
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::MouseButton::LeftButton)
			{
				emit OnUserNameOrIconClickedSignal();
				return true;
			}
		}
	}
	else if (watched == static_cast<QObject*>(ui.roomManageIcon) || watched == static_cast<QObject*>(ui.roomManageLab))
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::MouseButton::LeftButton)
			{
				emit onRoomManageTriggered();
				return true;
			}
		}
	}
	else if (watched == static_cast<QObject*>(ui.infoModifyIcon) || watched == static_cast<QObject*>(ui.infoModifyLab))
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::MouseButton::LeftButton)
			{
				emit onInfoModifyTriggered();
				return true;
			}
		}
	}

	return false;
}

void BiLiUserInfoWid::showEvent(QShowEvent *)
{
    effect_widget_->doOnHostWidgetVisibleStateChanged(true);

    effect_widget_->doOnHostWidgetPositionChanged(frameGeometry().x(), frameGeometry().y());
}
void BiLiUserInfoWid::hideEvent(QHideEvent*)
{
    effect_widget_->doOnHostWidgetVisibleStateChanged(false);
}

void BiLiUserInfoWid::resizeEvent(QResizeEvent *)
{
    //if (first_resize_) {
    //    //if (pos() == QPoint(0, 0))
    effect_widget_->doOnHostWidgetResize(frameGeometry().x(), frameGeometry().y(), frameGeometry().width(), frameGeometry().height());
    //else


    //    first_resize_ = false;
    //}
    //else
    //    effect_widget_->doOnHostWidgetResize(frameGeometry().x(), frameGeometry().y(), frameGeometry().width(), frameGeometry().height());
}

void BiLiUserInfoWid::moveEvent(QMoveEvent *)
{
    effect_widget_->doOnHostWidgetPositionChanged(frameGeometry().x(), frameGeometry().y());

    //qDebug() << frameGeometry().topLeft();
}
