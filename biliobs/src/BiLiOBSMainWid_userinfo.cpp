#include "BiLiOBSMainWid.h"
#include "../biliapi/IBiliApi.h"

#include "BiliGlobalDatas.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QDesktopServices>
#include <QUrl>

#include "BiLiUserInfoWid.h"
#include "BiliOBSUtility.hpp"
#include "qmessagebox.h"


void BiLiOBSMainWid::on_UserInfoBtn_clicked()
{
	//此功能只有在有直播间的时候能用
	if (onBroadcastRoomRequested(0) == false)
	{
		return;
	}

    QPoint pos = ui.UserInfoBtn->geometry().bottomLeft();
    QPoint pos_right = ui.UserInfoBtn->geometry().bottomRight();
    pos = (pos + pos_right) / 2;

	pos = mapToGlobal(pos);
    pos.setX(pos.x() - mBiLiUserInfoWid->width() / 2);
    pos.setY(pos.y() + 4);
    mBiLiUserInfoWid->show();
	mBiLiUserInfoWid->move(pos);

	mBiLiUserInfoWid->mSetUserName(gBili_userName.c_str());
	mBiLiUserInfoWid->mSetUserFace(mUserFace);
	mBiLiUserInfoWid->mSetGuest(lexical_cast<std::string>(gBili_audienceCount).c_str());
	mBiLiUserInfoWid->mSetScore(lexical_cast<std::string>(gBili_rcost).c_str());
}

void BiLiOBSMainWid::OnOpenRoomClicked()
{
	if (onBroadcastRoomRequested(0))
	{
		std::string url = biliApi->GetRoomAdminUrl();
		QDesktopServices::openUrl(QUrl(url.c_str()));
	}
}

void BiLiOBSMainWid::OnRoomMangerClicked()
{
	std::string url = biliApi->GetRoomAdminUrl("admin");
	QDesktopServices::openUrl(QUrl(url.c_str()));
}

void BiLiOBSMainWid::OnInfomodifyClicked()
{
	std::string url = biliApi->GetRoomAdminUrl("info");
	QDesktopServices::openUrl(QUrl(url.c_str()));
}
