#ifndef BILIUSERINFOWID_H
#define BILIUSERINFOWID_H

#include <QWidget>
#include "ui_BiLiUserInfoWid.h"
#include "rounded_widget.h"

class EffectWindowInterface;

class BiLiUserInfoWid : public RoundedWidget
{
	Q_OBJECT

public:
	BiLiUserInfoWid();
	~BiLiUserInfoWid();

	void mSetUserName(QString usrName);
	void mSetUserFace(QPixmap& usrFace);
	void mSetGuest(QString guestNumStr);
	void mSetScore(QString scoreStr);

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;

private:
	Ui::BiLiUserInfoWid ui;

	QString full_name_;

signals:
	void OnUserNameOrIconClickedSignal();
	void onRoomManageTriggered();
	void onInfoModifyTriggered();

private:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent*) override;

    void resizeEvent(QResizeEvent *) override;

    void moveEvent(QMoveEvent *) override;
private:

    EffectWindowInterface *effect_widget_;
};

#endif // BILIUSERINFOWID_H
